#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include "anyode/anyode_blas_lapack.hpp"
#include "anyode/anyode_matrix.hpp"


TEST_CASE( "DenseView.copy", "[DenseView]" ) {
    constexpr int n = 6;
    constexpr int ld = 8;
    const auto data = new std::array<double, n*ld> {{
        5,5,1,0,0,0,0,0,
        3,8,0,2,0,0,0,0,
        2,0,8,4,3,0,0,0,
        0,3,4,4,0,4,0,0,
        0,0,4,0,6,2,0,0,
        0,0,0,5,9,7,0,0
        }};
    bool colmaj = true;
    const auto ori = new AnyODE::DenseMatrixView<double> {&(*data)[0], n, n, ld, colmaj};
    REQUIRE( ori->m_data == &(*data)[0]);
    REQUIRE( ori->m_nr == n );
    REQUIRE( ori->m_nc == n );
    REQUIRE( ori->m_ld == ld );
    REQUIRE( ori->m_ndata == ld*n );
    REQUIRE( ! ori->m_own_data );
    std::array<double, n> b;
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> bref {{22, 57, 46.2, 256, 400.8, 276.6}};
    ori->dot_vec(&xref[0], &b[0]);
    auto dmv = *ori;
    delete data;
    delete ori;
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );
    }
    REQUIRE( dmv.m_data != nullptr);
    REQUIRE( dmv.m_nr == n );
    REQUIRE( dmv.m_nc == n );
    REQUIRE( dmv.m_ld == ld );
    REQUIRE( dmv.m_ndata == ld*n );
    REQUIRE( dmv.m_own_data );
    dmv.dot_vec(&xref[0], &b[0]);
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((b[idx] - bref[idx])/2e-13) < 1 );
    }
}
