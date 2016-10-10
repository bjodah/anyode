#pragma once

namespace eulerfw {

    struct Integrator {
        AnyODE::OdeSysBase * m_sys;
        double * const buffer;
        const int ny;
        Integrator(AnyODE::OdeSysBase * sys) : m_sys(sys), ny(m_sys->get_ny()), buffer(new double[ny]) {}
        ~Integrator() { delete []buffer; }
        void integrate(double * const tout, int n_tout, double * const yout) {
            for (int i=1; i<n_tout; ++i){
                m_sys->rhs(tout[i-1], &yout[ny*(i-1)], buffer);
                for (int j=0; j<ny; ++j)
                    yout[ny*i + j] = yout[ny*(i-1) + j] + (tout[i] - tout[i-1])*buffer[j];
            }
        }
    };
}
