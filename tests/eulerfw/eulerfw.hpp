#pragma once
#include <vector>

namespace eulerfw {

    struct Integrator {
        AnyODE::OdeSysBase * m_sys;
        Integrator(AnyODE::OdeSysBase * sys) : m_sys(sys) {}
        void integrate(double * const tout, int n_tout, double * const yout) {
            const int ny = m_sys->get_ny();
            std::vector<double> buffer(ny);
            for (int i=1; i<n_tout; ++i){
                m_sys->rhs(tout[i-1], &yout[ny*(i-1)], &buffer[0]);
                const double h = tout[i] - tout[i-1];
                for (int j=0; j<ny; ++j)
                    yout[ny*i + j] = yout[ny*(i-1) + j] + h*buffer[j];
            }
        }
    };
}
