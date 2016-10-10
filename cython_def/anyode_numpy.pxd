# -*- coding: utf-8; mode: cython -*-

from cpython.ref cimport PyObject

cdef extern from "anyode/anyode_numpy.hpp" namespace "AnyODE":
     cdef cppclass PyOdeSys:
         PyOdeSys(int, PyObject*, PyObject*, PyObject*)
         int get_ny()
         int get_mlower()
         int get_mupper()
         int get_nroots()
