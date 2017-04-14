#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include "anyode/anyode_blas_lapack.hpp"
#include "anyode/anyode_decomposition.hpp"

TEST_CASE( "solve", "[SVD]" ) {
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
    AnyODE::DenseMatrixView<double> dmv {&data[0], n, n, ld, colmaj};
    std::array<double, n> xref {{-7, 13, 9, -4, -0.7, 42}};
    std::array<double, n> x;
    std::array<double, n> b;
    dmv.dot_vec(&xref[0], &b[0]);
    auto decomp = AnyODE::SVD<double>(&dmv);
    REQUIRE( decomp.m_info == 0 );
    int flag = decomp.solve(&b[0], &x[0]);
    REQUIRE( flag == 0 );
    for (int idx=0; idx<n; ++idx){
        REQUIRE( std::abs((x[idx] - xref[idx])/2e-13) < 1 );
    }
    REQUIRE( decomp.m_condition_number < 10.0 );

}
