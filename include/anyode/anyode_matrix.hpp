#pragma once
#include <stdlib.h>  // aligned_alloc & free
#include <stdint.h> // uintptr_t
namespace AnyODE {

    template<typename Real_t>
    class MatrixView {
        void * m_array_ = nullptr;
        static constexpr int alignment_bytes_ = 64;
        static constexpr int alignment_items_ = alignment_bytes_/sizeof(Real_t);
        static_assert(sizeof(Real_t) <= alignment_bytes_, "unhandled situation");
        Real_t * alloc_array_(int n){
            m_array_ = aligned_alloc(alignment_bytes_, sizeof(Real_t)*n);
            return static_cast<Real_t *>(m_array_);
        }
    public:
        Real_t * m_data;
        int m_nr, m_nc, m_ld, m_ndata;
        bool m_own_data;
        MatrixView(Real_t * const data, int nr, int nc, int ld, int ndata) :
            m_data(data ? data : alloc_array_(ndata)), m_nr(nr), m_nc(nc), m_ld(ld), m_ndata(ndata), m_own_data(data == nullptr) {}
        MatrixView(const MatrixView<Real_t>& ori) : MatrixView(nullptr, ori.m_nr, ori.m_nc, ori.m_ld, ori.m_ndata) {
            std::copy(ori.m_data, ori.m_data + m_ndata, m_data);
        }
        virtual ~MatrixView(){
            if (m_own_data and m_array_)
                free(m_array_);
        }
        virtual Real_t& operator()(int ri, int ci) noexcept = 0;
        const Real_t& operator()(int ri, int ci) const noexcept { return (*const_cast<MatrixView<Real_t>* >(this))(ri, ci); }
        virtual void dot_vec(const Real_t * const, Real_t * const) = 0;
        virtual void set_to_eye_plus_scaled_mtx(Real_t, const MatrixView&) = 0;
    };

    template<typename Real_t = double>
    struct DenseMatrixView : public MatrixView<Real_t> {
        bool m_colmaj;
        DenseMatrixView(Real_t * const data, int nr, int nc, int ld, bool colmaj=true) :
            MatrixView<Real_t>(data, nr, nc, ld, ld*(colmaj ? nc : nr)),
            m_colmaj(colmaj)
        {}
        DenseMatrixView(const DenseMatrixView<Real_t>& ori) : MatrixView<Real_t>(ori), m_colmaj(ori.m_colmaj)
        {}
        Real_t& operator()(int ri, int ci) noexcept override final {
            const int imaj = m_colmaj ? ci : ri;
            const int imin = m_colmaj ? ri : ci;
            return this->m_data[imaj*this->m_ld + imin];
        }
        void dot_vec(const Real_t * const vec, Real_t * const out) override final {
            Real_t alpha=1, beta=0;
            int inc=1;
            char trans= m_colmaj ? 'N' : 'T';
            int sundials_dummy = 0;
            constexpr gemv_callback<Real_t> gemv{};
            gemv(&trans, &(this->m_nr), &(this->m_nc), &alpha, this->m_data, &(this->m_ld),
                 const_cast<Real_t *>(vec), &inc, &beta, out, &inc, sundials_dummy);
        }
        void set_to_eye_plus_scaled_mtx(Real_t scale, const MatrixView<Real_t>& source) override final {
            for (int imaj = 0; imaj < (m_colmaj ? this->m_nc : this->m_nr); ++imaj){
                for (int imin = 0; imin < (m_colmaj ? this->m_nr : this->m_nc); ++imin){
                    const int ri = m_colmaj ? imin : imaj;
                    const int ci = m_colmaj ? imaj : imin;
                    this->m_data[this->m_ld*imaj + imin] = scale*source(ri, ci) + ((imaj == imin) ? 1 : 0);
                }
            }
        }
    };

    constexpr int banded_padded_ld_(int kl, int ku) { return 2*kl+ku+1; }

    template<typename Real_t = double>
    struct BandedPaddedMatrixView : public MatrixView<Real_t> {
        int m_kl, m_ku;
        static constexpr bool m_colmaj = true;  // dgbmv takes a trans arg, but not used at the moment.
#define LD (ld ? ld : banded_padded_ld_(kl, ku))
        BandedPaddedMatrixView(Real_t * const data, int nr, int nc, int kl, int ku, int ld=0) :
            MatrixView<Real_t>(data, nr, nc, LD, LD*nc),
            m_kl(kl), m_ku(ku)
        {}
#undef LD
        BandedPaddedMatrixView(const BandedPaddedMatrixView<Real_t>& ori) : MatrixView<Real_t>(ori), m_kl(ori.m_kl), m_ku(ori.m_ku)
        {}
        Real_t& operator()(int ri, int ci) noexcept override final {
            return this->m_data[m_kl + m_ku + ri - ci + ci*this->m_ld]; // m_kl paddding
        }
        void dot_vec(const Real_t * const vec, Real_t * const out) override final {
            Real_t alpha=1, beta=0;
            int inc=1;
            const char trans='N';
            int sundials_dummy = 0;
            constexpr gbmv_callback<Real_t> gbmv{};
            gbmv(&trans, &(this->m_nr), &(this->m_nc), &(m_kl), &(m_ku), &alpha, this->m_data+m_kl,  // m_kl padding
                 &(this->m_ld), const_cast<Real_t *>(vec), &inc, &beta, out, &inc, sundials_dummy);
        }
        void set_to_eye_plus_scaled_mtx(Real_t scale, const MatrixView<Real_t>& source) override final {
            for (int ci = 0; ci < this->m_nc; ++ci){
                for (int ri = std::max(0, ci-m_ku); ri < std::min(this->m_nr, ci+m_kl+1); ++ri){
                    (*this)(ri, ci) = scale*source(ri, ci) + ((ri == ci) ? 1 : 0);
                }
            }
        }

    };
}
