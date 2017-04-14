#ifndef ANYODE_MATRIX_HPP_07E5C3BE95F911E68060EB14276541AC
#define ANYODE_MATRIX_HPP_07E5C3BE95F911E68060EB14276541AC

namespace AnyODE {

    template<typename Real_t>
    struct MatrixView {
        Real_t * m_data;
        int m_nr, m_nc, m_ld;
        bool m_own_data;
        MatrixView (Real_t * const data, int nr, int nc, int ld, bool own_data) :
            m_data(data), m_nr(nr), m_nc(nc), m_ld(ld), m_own_data(own_data) {}
        virtual ~MatrixView(){
            if (m_own_data and m_data)
                delete []m_data;
        }
        virtual Real_t& operator()(int ri, int ci) = 0;
        virtual void dot_vec(const Real_t * const, Real_t * const) = 0;
        virtual void set_to_eye_plus_scaled_mtx(Real_t, MatrixView&) = 0;
    };

    template<typename Real_t>
    struct DenseMatrixView : MatrixView<Real_t> {
        bool m_colmaj;
        DenseMatrixView(Real_t * const data, int nr, int nc, int ld, bool colmaj) :
         MatrixView(data ? data : new Real_t[ld*(colmaj ? nc : nr)], nr, nc, ld, data == nullptr),
         m_colmaj(colmaj) {}
        virtual Real_t& operator()(int ri, int ci) override {
            const int imaj = m_colmaj ? ci : ri;
            const int imin = m_colmaj ? ri : ci;
            return m_data[imaj*m_ld + imin];
        }
        virtual void dot_vec(const Real_t * const vec, Real_t * const out) override {
            Real_t alpha=1, beta=0;
            int inc=1;
            char trans= m_colmaj ? 'N' : 'T';
            int sundials_dummy = 0;
            dgemv_(&trans, &(m_nr), &(m_nc), &alpha, m_data, &m_ld, const_cast<Real_t *>(vec),
                   &inc, &beta, out, &inc, sundials_dummy);
        }
        virtual void set_to_eye_plus_scaled_mtx(Real_t scale, MatrixView& other) override {
            for (int imaj = 0; imaj < (m_colmaj ? m_nc : m_nr); ++imaj){
                for (int imin = 0; imin < (m_colmaj ? m_nr : m_nc); ++imin){
                    const int ri = m_colmaj ? imin : imaj;
                    const int ci = m_colmaj ? imaj : imin;
                    m_data[m_ld*imaj + imin] = scale*other(ri, ci) + ((imaj == imin) ? 1 : 0);
                }
            }
        }
    };

}

#endif /* ANYODE_MATRIX_HPP_07E5C3BE95F911E68060EB14276541AC */
