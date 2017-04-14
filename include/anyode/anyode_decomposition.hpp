#pragma once

#include <cmath>

#include "anyode/anyode_matrix.hpp"
#include "anyode/anyode_buffer.hpp"

namespace AnyODE {

    template<typename Real_t>
    struct DecompositionBase {
        virtual int factorize() = 0;
        virtual int solve(const Real_t * const, Real_t * const) = 0;
    };

    template<typename Real_t = double>
    struct SVD : public DecompositionBase<Real_t> {
        auto m_gesvd = std::conditional<std::is_same<Real_t, float>, &sgesvd_, &dgesvd_>;
        auto m_gemv = std::conditional<std::is_same<Real_t, float>, &sgemv_, &dgemv_>;
        DenseMatrixView<Real_t> * m_view;
        buffer_t<Real_t> m_s;
        int m_ldu;
        buffer_t<Real_t> m_u;
        int m_ldvt;
        buffer_t<Real_t> m_vt;
        buffer_t<Real_t> m_work;
        int m_lwork = -1; // Query
        int m_info;
        Real_t m_condition_number = -1;

        SVD(DenseMatrixView * view) :
            m_view(view), m_s(buffer_factory<Real_t>(std::min(view->m_nr, view->m_nc))),
            m_ldu(view->m_nr), m_u(buffer_factory<Real_t>(m_ldu*(view->m_nr))),
            m_ldvt(view->m_nc), m_vt(buffer_factory<Real_t>(m_ldvt*(view->m_nc)))
        {
            int info;
            Real_t optim_work_size;
            char mode = 'A';
            m_gesvd(&mode, &mode, &(m_view->m_nr), &(m_view->m_nc), m_view->m_data, &(m_view->m_ld),
               buffer_get_raw_ptr(m_s), buffer_get_raw_ptr(m_u), &m_ldu,
               buffer_get_raw_ptr(m_vt), &m_ldvt, &optim_work_size, &m_lwork, &info);
            m_lwork = static_cast<int>(optim_work_size);
            m_work = buffer_factory<Real_t>(m_lwork);
            m_info = factorize();
        }
        int factorize() override{
            int info;
            char mode = 'A';
            m_gesvd(&mode, &mode, &(m_view->m_nr), &(m_view->m_nc), m_view->m_data, &(m_view->m_ld),
                    buffer_get_raw_ptr(m_s), buffer_get_raw_ptr(m_u), &m_ldu,
                    buffer_get_raw_ptr(m_vt), &m_ldvt, buffer_get_raw_ptr(m_work), &m_lwork, &info);

            m_condition_number = std::fabs(m_s[0]/m_s[std::min(m_view->m_nr, m_view->m_nc) - 1]);
            return info;
        }
        int solve(const Real_t* const b, Real_t * const x) override{
            Real_t alpha=1, beta=0;
            int incx=1, incy=1;
            char trans = 'T';
            int sundials_dummy = 0;
            auto y1 = buffer_factory<Real_t>(m_view->m_nr);
            m_gemv(&trans, &(m_view->m_nr), &(m_view->m_nr), &alpha, buffer_get_raw_ptr(m_u), &(m_ldu),
                   const_cast<Real_t*>(b), &incx, &beta, buffer_get_raw_ptr(y1), &incy, sundials_dummy);
            for (int i=0; i < m_view->m_nr; ++i)
                y1[i] /= m_s[i];
            m_gemv(&trans, &(m_view->m_nc), &(m_view->m_nc), &alpha, buffer_get_raw_ptr(m_vt), &m_ldvt,
                   buffer_get_raw_ptr(y1), &incx, &beta, x, &incy, sundials_dummy);
            return 0;
        }

    };
}
