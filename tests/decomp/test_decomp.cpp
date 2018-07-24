#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#ifndef ANYODE_NO_LAPACK
#define ANYODE_NO_LAPACK 0
#endif

#if ANYODE_NO_LAPACK == 1
#include "anyode/anyode_decomposition.hpp"
#else
#include "anyode/anyode_decomposition_lapack.hpp"
#endif

#if ANYODE_NO_LAPACK != 1
TEST_CASE( "SVD_solve", "[SVD]" ) {
    constexpr int n = 6;
    constexpr int ld = 8;
    std::array<double, n*ld> data {{  // column major
        5,5,1,0,0,0,0,0,
        3,8,0,2,0,0,0,0,
        2,0,8,4,3,0,0,0,
        0,3,4,4,0,4,0,0,
        0,0,4,0,6,2,0,0,
        0,0,0,5,9,7,0,0
    }};
    bool colmaj = true;
    AnyODE::DenseMatrix<double> dmv {&data[0], n, n, ld, colmaj};
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> x;
    std::array<double, n> b;
    dmv.dot_vec(&xref[0], &b[0]);
    auto decomp = AnyODE::SVD<double>(&dmv);
    int info = decomp.factorize();
    REQUIRE( info == 0 );
    int flag = decomp.solve(&b[0], &x[0]);
    REQUIRE( flag == 0 );
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((x[idx] - xref[idx])/2e-13) < 1 );
    }
    REQUIRE( decomp.m_condition_number < 10.0 );

}
#endif

TEST_CASE( "DenseLU_solve", "[DenseLU]" ) {
    constexpr int n = 6;
    constexpr int ld = 8;
    std::array<double, n*ld> data {{  // column major
        5,5,1,0,0,0,0,0,
        3,8,0,2,0,0,0,0,
        2,0,8,4,3,0,0,0,
        0,3,4,4,0,4,0,0,
        0,0,4,0,6,2,0,0,
        0,0,0,5,9,7,0,0
    }};
    bool colmaj = true;
    AnyODE::DenseMatrix<double> dmv {&data[0], n, n, ld, colmaj};
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> x;
    std::array<double, n> b;
    dmv.dot_vec(&xref[0], &b[0]);
    auto decomp = AnyODE::DenseLU<double>(&dmv);
    int info = decomp.factorize();
    REQUIRE( info == 0 );
    int flag = decomp.solve(&b[0], &x[0]);
    REQUIRE( flag == 0 );
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((x[idx] - xref[idx])/2e-13) < 1 );
    }
}

#if ANYODE_NO_LAPACK != 1
TEST_CASE( "BandedLU_solve", "[BandedLU]" ) {
    constexpr int n = 6;
    constexpr int ld = 8;
    constexpr int nd = 2;
    std::array<double, n*ld> data {{  // column major
        5,5,1,0,0,0,0,0,
        3,8,0,2,0,0,0,0,
        2,0,8,4,3,0,0,0,
        0,3,4,4,0,4,0,0,
        0,0,4,0,6,2,0,0,
        0,0,0,5,9,7,0,0
    }};
    AnyODE::BandedMatrix<double> bpmv {nullptr, n, n, nd, nd};
    REQUIRE(bpmv.m_ld == 3*nd+1);
    REQUIRE(bpmv.m_kl == nd);
    REQUIRE(bpmv.m_ku == nd);
    REQUIRE(bpmv.m_nr == n);
    REQUIRE(bpmv.m_nc == n);
    for (int ri=0; ri < n; ++ri){
        for (int ci = std::max(0, ri-nd); ci < std::min(n, ri+nd+1); ++ci){
            bpmv(ri, ci) = data[ci*ld + ri];
        }
    }
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> bref {{22, 57, 46.2, 256, 400.8, 276.6}};
    std::array<double, n> x;
    std::array<double, n> b;
    bpmv.dot_vec(&xref[0], &b[0]);
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );

    }
    auto decomp = AnyODE::BandedLU<double>(&bpmv);
    int info = decomp.factorize();
    REQUIRE( info == 0 );
    int flag = decomp.solve(&b[0], &x[0]);
    REQUIRE( flag == 0 );
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((x[idx] - xref[idx])/2e-13) < 1 );
    }
}
#endif

TEST_CASE( "DiagInv_solve", "[Diaginv]" ) {
    constexpr int n = 6;
    constexpr int ld = 1;
    std::array<double, n*ld> data {{2,4,8,5,7,13}};
    AnyODE::DiagonalMatrix<double> dm {nullptr, n, n, 1};
    REQUIRE(dm.m_ld == 1);
    REQUIRE(dm.m_nr == n);
    REQUIRE(dm.m_nc == n);
    for (int i=0; i < n; ++i){
        dm(i, i) = data[i];
    }
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> bref {{-14. ,   52. ,   72. ,  -20. ,   -4.9,  546.}};
    std::array<double, n> x;
    std::array<double, n> b;
    dm.dot_vec(&xref[0], &b[0]);
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );
    }
    auto decomp = AnyODE::DiagonalInv<double>(&dm);
    int info = decomp.factorize();
    REQUIRE( info == 0 );
    int flag = decomp.solve(&b[0], &x[0]);
    REQUIRE( flag == 0 );
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((x[idx] - xref[idx])/2e-13) < 1 );
    }
}
