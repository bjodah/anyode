# -*- coding: utf-8; mode: cython -*-
# cython: language_level = 3str
# distutils: language = c++
# distutils: extra_compile_args = -std=c++20

from cpython.object cimport PyObject
from libc.stdlib cimport malloc
cimport numpy as cnp
from midpoint cimport MidpointPyOdeSys, solve_predefined

cdef extern from "midpoint_numpy.hpp" namespace "midpoint":
    ctypedef long int midpoint_index
    ctypedef double midpoint_real

cnp.import_array()  # Numpy C-API initialization

import numpy as np
import copy

def predefined(rhs, double[::1] y0, double[::1] xout, params=None):
    cdef:
        midpoint_index ny = y0.shape[y0.ndim-1]
        MidpointPyOdeSys * odesys
        midpoint_real * yout = <midpoint_real *>malloc(xout.size*ny*sizeof(midpoint_real))
        midpoint_real [:, ::1] yout_view
        cnp.npy_intp yout_dims[2]

    for i, yval in enumerate(y0):
        yout[i] = yval
    odesys = new MidpointPyOdeSys(ny, <PyObject*>rhs, <PyObject*>params)

    try:
        t_wall = solve_predefined(yout, &xout[0], xout.size, odesys)
        yout_dims[0] = xout.size
        yout_dims[1] = ny
        yout_view = <midpoint_real [:yout_dims[0], :yout_dims[1]]> yout
        return np.asarray(yout_view), {'time_wall': t_wall}
    finally:
        del odesys
