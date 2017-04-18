#pragma once

#include <memory>

#include <anyode/anyode.hpp>
#include <anyode/anyode_blas_lapack.hpp>  // dgemv, dgesvd
#include <anyode/anyode_matrix.hpp> // DenseMatrixView
#include <anyode/anyode_buffer.hpp>  // make_unique
#include <anyode/anyode_decomposition.hpp>  // SVD

namespace AnyODE {

    template <typename Real_t=double, typename JacMat_t=DenseMatrixView, typename Decomp_t=SVD>
    struct OdeSysIterativeBase : public OdeSysBase<Real_t> {
        int m_njacvec_dot=0, m_nprec_setup=0, m_nprec_solve=0;
        std::unique_ptr<JacMat_t> m_jac_cache {nullptr};
        std::unique_ptr<JacMat_t> m_prec_cache {nullptr};
        bool m_update_prec_cache = false;
        Real_t m_old_gamma;

        virtual Status jac_times_vec(const Real_t * const __restrict__ vec,
                                     Real_t * const __restrict__ out,
                                     Real_t t,
                                     const Real_t * const __restrict__ y,
                                     const Real_t * const __restrict__ fy
                                     ) override
        {
            // See "Jacobian information (matrix-vector product)"
            //     (4.6.8 in cvs_guide.pdf for sundials 2.7.0)
            auto status = AnyODE::Status::success;
            const int ny = this->get_ny();
            if (m_jac_cache == nullptr){
                m_jac_cache = make_unique<JacMat_t>(nullptr, ny, ny, ny, true);
                status = this->dense_jac_cmaj(t, y, fy, m_jac_cache->m_data, m_jac_cache->m_ld);
            }
            m_jac_cache->dot_vec(vec, out);
            m_njacvec_dot++;
            return status;
        }

        virtual Status prec_setup(Real_t t,
                                  const Real_t * const __restrict__ y,
                                  const Real_t * const __restrict__ fy,
                                  bool jac_ok,
                                  bool& jac_recomputed,
                                  Real_t gamma) override
        {
            const int ny = this->get_ny();
            auto status = AnyODE::Status::success;
            ignore(gamma);
            // See "Preconditioning (Jacobian data)" in cvs_guide.pdf (4.6.10 for 2.7.0)
            if (m_jac_cache == nullptr)
                m_jac_cache = make_unique<JacMat_t>(nullptr, ny, ny, ny, true);

            if (jac_ok){
                jac_recomputed = false;
            } else {
                status = this->dense_jac_cmaj(t, y, fy, m_jac_cache->m_data, m_jac_cache->m_ld);
                m_update_prec_cache = true;
                jac_recomputed = true;
            }
            m_nprec_setup++;
            return status;
        }

        virtual Status prec_solve_left(const Real_t t,
                                       const Real_t * const __restrict__ y,
                                       const Real_t * const __restrict__ fy,
                                       const Real_t * const __restrict__ r,
                                       Real_t * const __restrict__ z,
                                       Real_t gamma,
                                       Real_t delta,
                                       const Real_t * const __restrict__ ewt
                                       ) override
        {
            // See 4.6.9 on page 75 in cvs_guide.pdf (Sundials 2.6.2)
            // Solves P*z = r, where P ~= I - gamma*J
            ignore(delta);
            const int ny = this->get_ny();
            if (ewt)
                throw std::runtime_error("Not implemented.");
            m_nprec_solve++;

            ignore(t); ignore(fy); ignore(y);
            bool recompute = false;
            if (m_prec_cache == nullptr){
                m_prec_cache = make_unique<JacMat_t>(nullptr, ny, ny, ny, true);
                recompute = true;
            } else {
                if (m_update_prec_cache or (m_old_gamma != gamma))
                    recompute = true;
            }
            if (recompute){
                m_old_gamma = gamma;
                m_prec_cache->set_to_eye_plus_scaled_mtx(-gamma, *m_jac_cache);
            }
            int info;
            auto decomp = Decomp_t((JacMat_t*)(m_prec_cache.get()));
            info = decomp.solve(r, z);
            if (info == 0)
                return AnyODE::Status::success;
            return AnyODE::Status::recoverable_error;
        }


    };
}
