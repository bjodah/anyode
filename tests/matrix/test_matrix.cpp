#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include <memory>  // std::unique_ptr
#include "anyode/anyode_matrix.hpp"

#ifndef ANYODE_NO_LAPACK
#define ANYODE_NO_LAPACK 0
#endif

#if ANYODE_NO_LAPACK == 1
#include "anyode/anyode_blasless.hpp"
#else
#include "anyode/anyode_blas_lapack.hpp"
#endif


AnyODE::DenseMatrix<double> * mk_dm(bool own_data=false){
    constexpr int n = 6;
    constexpr int ld = 8;
    double * data = static_cast<double *>(malloc(sizeof(double)*n*ld));
    std::array<double, n*ld> data_ {{
        5,5,1,0,0,0,0,0,
        3,8,0,2,0,0,0,0,
        2,0,8,4,3,0,0,0,
        0,3,4,4,0,4,0,0,
        0,0,4,0,6,2,0,0,
        0,0,0,5,9,7,0,0
            }};
    std::copy(data_.data(), data_.end(), data);
    bool colmaj = true;
    return new AnyODE::DenseMatrix<double> {data, n, n, ld, colmaj, own_data};
}

TEST_CASE( "DenseMatrix.copy", "[DenseMatrix]" ) {
    const auto ori = mk_dm();
    const int n = 6;
    const int ld = 8;
    REQUIRE( ori->m_nr == n );
    REQUIRE( ori->m_nc == n );
    REQUIRE( ori->m_ld == ld );
    REQUIRE( ori->m_ndata == n*ld );
    REQUIRE( ! ori->m_own_data );
    std::array<double, n> b;
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> bref {{22, 57, 46.2, 256, 400.8, 276.6}};
    ori->dot_vec(&xref[0], &b[0]);
    auto dmv = *ori;
    free(ori->m_data);
    delete ori;
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );
    }
    REQUIRE( dmv.m_data != nullptr);
    REQUIRE( dmv.m_nr == n );
    REQUIRE( dmv.m_nc == n );
    REQUIRE( dmv.m_ld == ld );
    REQUIRE( dmv.m_ndata == ld*n );
    dmv.dot_vec(&xref[0], &b[0]);
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );
    }
}

#if ANYODE_NO_LAPACK != 1
TEST_CASE( "banded_padded_from_dense", "[BandedMatrix]" ) {
    REQUIRE( AnyODE::banded_padded_ld(3, 5) == 3*2 + 5 + 1);
    const int n = 6;
    const auto dense = mk_dm(true);
    REQUIRE( dense->m_own_data );
    auto banded = AnyODE::BandedMatrix<double>(*dense, 2, 2);
    delete dense;
    REQUIRE( banded.m_kl == 2 );
    REQUIRE( banded.m_ku == 2 );
    REQUIRE( banded.m_nr == n );
    REQUIRE( banded.m_nc == n );
    REQUIRE( banded.m_ld == 7 );
    REQUIRE( std::abs(banded.m_data[4] - 5) < 1e-15 );
    REQUIRE( std::abs(banded.m_data[5] - 5) < 1e-15 );
    REQUIRE( std::abs(banded.m_data[6] - 1) < 1e-15 );
}
#endif

AnyODE::DiagonalMatrix<double> * mk_dg(bool own_data=false){
    constexpr int n = 6;
    constexpr int ld = 1;
    double * data = static_cast<double *>(malloc(sizeof(double)*n*ld));
    std::array<double, n> data_ {{2,4,8,5,7,13}};
    std::copy(data_.data(), data_.end(), data);
    return new AnyODE::DiagonalMatrix<double> {data, n, n, 1, own_data};
}

TEST_CASE( "DiagonalMatrix methods", "[DiagonalMatrix]" ) {
    const auto ori = mk_dg();
    const int n = 6;
    const int ld = 1;
    REQUIRE( ori->m_nr == n );
    REQUIRE( ori->m_nc == n );
    REQUIRE( ori->m_ld == ld );
    REQUIRE( ori->m_ndata == n*ld );
    REQUIRE( ! ori->m_own_data );
    std::array<double, n> b;
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> bref {{-14. ,   52. ,   72. ,  -20. ,   -4.9,  546.}};
    ori->dot_vec(&xref[0], &b[0]);
    auto dmv = *ori;
    free(ori->m_data);
    delete ori;
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );
    }
    REQUIRE( dmv.m_data != nullptr);
    REQUIRE( dmv.m_nr == n );
    REQUIRE( dmv.m_nc == n );
    REQUIRE( dmv.m_ld == ld );
    REQUIRE( dmv.m_ndata == ld*n );
    dmv.dot_vec(&xref[0], &b[0]);
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );
    }
}
