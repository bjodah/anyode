# -*- coding: utf-8; mode: cython -*-
# distutils: language = c++
# distutils: extra_compile_args = -std=c++11

from cpython.ref cimport PyObject
cimport numpy as cnp
cnp.import_array()  # Numpy C-API initialization

import numpy as np

from eulerfw cimport Integrator
from anyode cimport OdeSysBase
from anyode_numpy cimport PyOdeSys


cdef class EulerForward:

    cdef Integrator * thisptr
    cdef PyOdeSys * odesys

    def __cinit__(self, int ny, f, cb_kwargs=None, dense_jac_cmaj=None):
        self.odesys = new PyOdeSys(ny, <PyObject *>f, <PyObject *>cb_kwargs, <PyObject *>dense_jac_cmaj)
        self.thisptr = new Integrator(<OdeSysBase*>self.odesys)

    def __dealloc__(self):
        del self.thisptr
        del self.odesys

    def integrate(self,
                  cnp.ndarray[cnp.float64_t, ndim=1] tout,
                  cnp.ndarray[cnp.float64_t, ndim=1] y0):
        cdef cnp.ndarray[cnp.float64_t, ndim=2] yout
        if y0.size != self.odesys.get_ny():
            raise ValueError("y0 of incorrect size")
        yout = np.empty((tout.size, y0.size))
        yout[0, :] = y0
        self.thisptr.integrate(&tout[0], tout.size, &yout[0, 0])
        return yout
