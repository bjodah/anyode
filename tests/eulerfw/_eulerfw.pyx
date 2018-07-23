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
    cdef PyOdeSys[double, int] * odesys

    def __cinit__(self, int ny, f, cb_kwargs=None, jtimes=None, quads=None, roots=None, jac=None, dx0cb=None, dx_max_cb=None):
        cdef int mlower=-1, mupper=-1, nroots=0, nquads=0, nnz=-1
        self.odesys = new PyOdeSys[double, int](
            ny, <PyObject *>f, <PyObject *>jac, <PyObject *>jtimes, <PyObject *>quads, <PyObject *>roots, <PyObject *>cb_kwargs,
            mlower, mupper, nquads, nroots, <PyObject *>dx0cb, <PyObject *>dx_max_cb, nnz)
        self.thisptr = new Integrator(<OdeSysBase[double, int]*>self.odesys)

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
        return yout, self.odesys.current_info.nfo_dbl[b'time_wall']

    def get_ny(self):
        return self.odesys.get_ny()

    def get_dx0(self, double t, cnp.ndarray[cnp.float64_t, ndim=1, mode='c'] y):
        return self.odesys.get_dx0(t, &y[0])

    def get_dx_max(self, double t, cnp.ndarray[cnp.float64_t, ndim=1, mode='c'] y):
        return self.odesys.get_dx_max(t, &y[0])
