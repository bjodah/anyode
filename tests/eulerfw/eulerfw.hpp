#pragma once
#include <chrono>

namespace eulerfw {

    struct Integrator {
        AnyODE::OdeSysBase<double, int> * m_sys;
        const int m_ny;
        double * const m_buffer;
        Integrator(AnyODE::OdeSysBase<double, int> * sys) : m_sys(sys), m_ny(m_sys->get_ny()), m_buffer(new double[m_ny]) {}
        ~Integrator() { delete []m_buffer; }
        void integrate(double * const tout, int n_tout, double * const yout) {
            auto t_start = std::chrono::high_resolution_clock::now();
            for (int i=1; i<n_tout; ++i){
                m_sys->rhs(tout[i-1], &yout[m_ny*(i-1)], m_buffer);
                for (int j=0; j<m_ny; ++j)
                    yout[m_ny*i + j] = yout[m_ny*(i-1) + j] + (tout[i] - tout[i-1])*m_buffer[j];
            }
            m_sys->current_info.nfo_dbl["time_wall"] = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now() - t_start).count();
        }
    };
}
