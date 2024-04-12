
from anyode cimport OdeSysBase
from cpython.object cimport PyObject

cdef extern from "midpoint_numpy.hpp" namespace "midpoint":
    cppclass MidpointPyOdeSys:
        MidpointPyOdeSys(long int, PyObject *, PyObject *)

    double solve_predefined(double *yout, double *tout, long int nt, MidpointPyOdeSys * odesys) except +
