#ifndef ANYODE_NUMPY_HPP_363685EE8DA911E6879C8F3B030A9DA7
#define ANYODE_NUMPY_HPP_363685EE8DA911E6879C8F3B030A9DA7

#include <Python.h>
#include <anyode/anyode.hpp>
#include <numpy/arrayobject.h>

namespace AnyODE {
    struct PyOdeSys : public AnyODE::OdeSysBase {
        int ny, mlower=-1, mupper=-1, nroots=0;
        PyObject *py_rhs, *py_dense_jac_cmaj, *py_kwargs;
        unsigned nfev, njev;
        PyOdeSys(int ny, PyObject * py_rhs, PyObject * py_kwargs=nullptr, PyObject * py_dense_jac_cmaj=nullptr) :
            ny(ny), py_rhs(py_rhs), py_kwargs(py_kwargs), py_dense_jac_cmaj(py_dense_jac_cmaj) {
            if (py_rhs == nullptr)
                throw std::runtime_error("py_rhs must not be nullptr");
            Py_INCREF(py_rhs);
            Py_XINCREF(py_kwargs);
            Py_XINCREF(py_dense_jac_cmaj);
        }
        virtual ~PyOdeSys() {
            Py_DECREF(py_rhs);
            Py_XDECREF(py_kwargs);
            Py_XDECREF(py_dense_jac_cmaj);
        }

        virtual int get_ny() const override { return ny; }
        virtual int get_mlower() const override { return mlower; }
        virtual int get_mupper() const override { return mupper; }
        virtual int get_nroots() const override { return nroots; }
        Status handle_status_(PyObject * py_result, const std::string what_arg){
            if (py_result == nullptr){
                throw std::runtime_error(what_arg + " failed");
            } else if (py_result == Py_None){
                Py_DECREF(py_result);
                return AnyODE::Status::success;
            }
            long result = PyInt_AsLong(py_result);
            Py_DECREF(py_result);
            if ((PyErr_Occurred() and result == -1) or
                result == static_cast<long int>(AnyODE::Status::unrecoverable_error)) {
                return AnyODE::Status::unrecoverable_error;
            } else if (result == static_cast<long int>(AnyODE::Status::recoverable_error)) {
                return AnyODE::Status::recoverable_error;
            } else if (result == static_cast<long int>(AnyODE::Status::success)) {
                return AnyODE::Status::success;
            }
            throw std::runtime_error(what_arg + " did not return None, -1, 0 or 1");
        }
        virtual Status rhs(double xval, const double * const y, double * const dydx) override {
            npy_intp dims[1] { static_cast<npy_intp>(this->ny) } ;
            const auto type_tag = NPY_DOUBLE;
            PyObject * py_yarr = PyArray_SimpleNewFromData(
                1, dims, type_tag, static_cast<void*>(const_cast<double*>(y)));
            PyObject * py_dydx = PyArray_SimpleNewFromData(
                1, dims, type_tag, static_cast<void*>(dydx));
            PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
            PyObject * py_arglist = Py_BuildValue("(dOO)", (double)(xval), py_yarr, py_dydx);
            PyObject * py_result = (this->py_kwargs and this->py_kwargs != Py_None) ?
                PyEval_CallObjectWithKeywords(this->py_rhs, py_arglist, this->py_kwargs) :
                PyEval_CallObject(this->py_rhs, py_arglist);
            Py_DECREF(py_arglist);
            Py_DECREF(py_dydx);
            Py_DECREF(py_yarr);
            nfev++;
            return handle_status_(py_result, "rhs");
        }
    };
}


#endif /* ANYODE_NUMPY_HPP_363685EE8DA911E6879C8F3B030A9DA7 */
