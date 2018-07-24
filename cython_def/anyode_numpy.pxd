# -*- coding: utf-8; mode: cython -*-

from cpython.ref cimport PyObject
from libcpp cimport bool
from anyode cimport Info

cdef extern from "numpy/arrayobject.h":
    ctypedef int NPY_TYPES

cdef extern from "anyode/anyode_numpy.hpp" namespace "AnyODE":
    cdef cppclass PyOdeSys[Real_t, Index_t]:
        PyOdeSys(Index_t, PyObject*, PyObject*, PyObject*, PyObject*, PyObject*, PyObject*, int, int, int, int,
                 PyObject*, PyObject*, Index_t)
        Index_t get_ny()
        Index_t get_nnz()
        NPY_TYPES int_type_tag
        NPY_TYPES float_type_tag
        int get_nquads()
        int get_nroots()
        Real_t get_dx0(Real_t, Real_t *) except +
        Real_t get_dx_max(Real_t, Real_t *) except +
        bool autonomous_exprs
        bool use_get_dx_max
        bool record_rhs_xvals
        bool record_jac_xvals
        bool record_order
        bool record_fpe
        bool record_steps
        int mlower, mupper, nroots
        Info current_info
        int nfev, njev, njvev
        Index_t nnz
        void * integrator
