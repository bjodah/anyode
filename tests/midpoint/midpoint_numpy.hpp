#pragma once
#include <anyode/anyode_numpy.hpp>
#include <chrono>
#include <memory>

BEGIN_NAMESPACE(midpoint)
using midpoint_real = double;
using midpoint_index = long int;

struct MidpointPyOdeSys : public AnyODE::PyOdeSys<midpoint_real, midpoint_index> {
    MidpointPyOdeSys(midpoint_index ny, PyObject *rhs, PyObject *py_kwargs)
        : AnyODE::PyOdeSys<midpoint_real, midpoint_index>::PyOdeSys(ny, rhs, nullptr, nullptr, nullptr, nullptr, py_kwargs) {}
};

double solve_predefined(midpoint_real *yout, midpoint_real *tout, midpoint_index nt, MidpointPyOdeSys * odesys) {
    const auto& start = std::chrono::high_resolution_clock::now();
    const auto ny {odesys->ny};
    std::unique_ptr<midpoint_real[]> tmp = std::make_unique<midpoint_real[]>(ny);
    for (midpoint_index i=1; i<nt; ++i) {
        midpoint_real h = tout[i] - tout[i-1];
        /*auto status =*/ odesys->rhs(tout[i-1], &yout[ny*(i-1)], tmp.get());
        for (midpoint_index j=0; j<ny; ++j) {
            tmp[j] = tmp[j]*h*0.5 + yout[ny*(i-1)+j];
        }
        /*status =*/ odesys->rhs(tout[i-1], tmp.get(), &yout[ny*i]);
        for (midpoint_index j=0; j<ny; ++j) {
            yout[ny*i + j] = yout[ny*i + j]*h + yout[ny*(i-1)+j];
        }
    }
    return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
}
END_NAMESPACE(midpoint)
