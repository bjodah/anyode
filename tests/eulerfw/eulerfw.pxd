# -*- coding: utf-8; mode: cython -*-

from anyode cimport OdeSysBase

cdef extern from "eulerfw.hpp" namespace "eulerfw":
    cppclass Integrator:
        Integrator(OdeSysBase *)
        void integrate(double *, int, double *) except +
