#pragma once

#include <Python.h>
#include <numpy/arrayobject.h>
#include <anyode/anyode_iterative.hpp>
#include <anyode/anyode_matrix.hpp> // DenseMatrix
#include <anyode/anyode_decomposition.hpp>  // DenseLU
#include <anyode/anyode_numpy_types.hpp>


BEGIN_NAMESPACE(AnyODE)
template<typename Real_t = double, typename Index_t = int>
struct PyOdeSys: public AnyODE::OdeSysIterativeBase<Real_t, Index_t, DenseMatrix<Real_t>, DenseLU<Real_t>> {
    Index_t ny;
    PyObject *py_rhs, *py_jac, *py_jtimes, *py_quads, *py_roots, *py_kwargs, *py_dx0cb, *py_dx_max_cb,
        *py_prec_setup, *py_prec_solve_left;
    int mlower, mupper, nquads, nroots;
    Index_t nnz;
    PyArray_Descr * real_type_descr = PyArray_DescrFromType(PyOdeSys::real_type_tag);
    PyOdeSys(Index_t ny, PyObject * py_rhs, PyObject * py_jac=nullptr, PyObject * py_jtimes=nullptr,
             PyObject * py_quads=nullptr,
             PyObject * py_roots=nullptr, PyObject * py_kwargs=nullptr, int mlower=-1,
             int mupper=-1, int nquads=0, int nroots=0, PyObject * py_dx0cb=nullptr,
             PyObject * py_dx_max_cb=nullptr, Index_t nnz=-1,
             PyObject * py_prec_setup=nullptr, PyObject * py_prec_solve_left=nullptr) :
        ny(ny), py_rhs(py_rhs), py_jac(py_jac), py_jtimes(py_jtimes),
        py_quads(py_quads), py_roots(py_roots),
        py_kwargs(py_kwargs), py_dx0cb(py_dx0cb), py_dx_max_cb(py_dx_max_cb),
        mlower(mlower), mupper(mupper), nquads(nquads), nroots(nroots),
        nnz(nnz), py_prec_setup(py_prec_setup), py_prec_solve_left(py_prec_solve_left)
    {
        if (py_rhs == nullptr){
            throw std::runtime_error("py_rhs must not be nullptr");
        }
        if ((py_dx_max_cb != nullptr) && (py_dx_max_cb != Py_None)) {
            this->use_get_dx_max = true;
        }
        Py_INCREF(py_rhs);
        Py_XINCREF(py_jac);
        Py_XINCREF(py_jtimes);
        Py_XINCREF(py_quads);
        Py_XINCREF(py_roots);
        if (py_kwargs == Py_None){
            Py_DECREF(Py_None);
            this->py_kwargs = nullptr;
        } else {
            Py_XINCREF(py_kwargs);
        }
        Py_XINCREF(py_prec_setup);
        Py_XINCREF(py_prec_solve_left);
    }
    virtual ~PyOdeSys() {
        Py_DECREF(py_rhs);
        Py_XDECREF(py_jac);
        Py_XDECREF(py_jtimes);
        Py_XDECREF(py_quads);
        Py_XDECREF(py_roots);
        Py_XDECREF(py_kwargs);
        Py_DECREF(real_type_descr);
        Py_XDECREF(py_prec_setup);
        Py_XDECREF(py_prec_solve_left);
    }

    const static NPY_TYPES index_type_tag = npy_index_type<Index_t>::type_tag;
	const static NPY_TYPES real_type_tag = npy_real_type<Real_t>::type_tag;

    Index_t get_ny() const override { return ny; }
    int get_mlower() const override { return mlower; }
    int get_mupper() const override { return mupper; }
    Index_t get_nnz() const override { return nnz; }
    int get_nquads() const override { return nquads; }
    int get_nroots() const override { return nroots; }
    Real_t get_dx0(Real_t t, const Real_t * const y) override {
        if (py_dx0cb == nullptr || py_dx0cb == Py_None) {
			return this->default_dx0;
        }
        npy_intp dims[1] { static_cast<npy_intp>(this->ny) } ;
        PyObject * py_yarr = PyArray_SimpleNewFromData(
            1, dims, this->real_type_tag, static_cast<void*>(const_cast<Real_t*>(y)));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        PyObject * py_arglist = Py_BuildValue("(OO)", t_scalar, py_yarr);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_dx0cb, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_yarr);
        Py_DECREF(t_scalar);
        if (py_result == nullptr) {
            throw std::runtime_error("get_dx0 failed (dx0cb failed)");
        }
        double res = PyFloat_AsDouble(py_result);
        Py_DECREF(py_result);
        if ((PyErr_Occurred()) && (res == -1.0)) {
            throw std::runtime_error("get_dx0 failed (value returned by dx0cb could not be converted to float)");
        }
        return res;
    }
    Real_t get_dx_max(Real_t t, const Real_t * const y) override {
        if (py_dx_max_cb == nullptr || py_dx_max_cb == Py_None) {
            return INFINITY;
        }
        npy_intp dims[1] { static_cast<npy_intp>(this->ny) } ;
        PyObject * py_yarr = PyArray_SimpleNewFromData(
            1, dims, this->real_type_tag, static_cast<void*>(const_cast<Real_t*>(y)));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        PyObject * py_arglist = Py_BuildValue("(OO)", t_scalar, py_yarr);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_dx_max_cb, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_yarr);
        Py_DECREF(t_scalar);
        if (py_result == nullptr) {
            throw std::runtime_error("get_dx_max failed (dx_max_cb failed)");
        }
        double res = PyFloat_AsDouble(py_result);
        Py_DECREF(py_result);
        if (PyErr_Occurred() && (res == -1.0)) {
            throw std::runtime_error("get_dx_max failed (value returned by dx_max_cb could not be converted to float)");
        }
        return res;
    }
    Status handle_status_(PyObject * py_result, const std::string what_arg){
        if (py_result == nullptr){
            throw std::runtime_error(what_arg + " failed");
        } else if (py_result == Py_None){
            Py_DECREF(py_result);
            return AnyODE::Status::success;
        }
        long result = PyInt_AsLong(py_result);
        Py_DECREF(py_result);

        if ((PyErr_Occurred() && (result == -1)) ||
            (result == static_cast<long int>(AnyODE::Status::unrecoverable_error))) {
            return AnyODE::Status::unrecoverable_error;
        } else if (result == static_cast<long int>(AnyODE::Status::recoverable_error)) {
            return AnyODE::Status::recoverable_error;
        } else if (result == static_cast<long int>(AnyODE::Status::success)) {
            return AnyODE::Status::success;
        }
        throw std::runtime_error(what_arg + " did not return None, -1, 0 or 1");
    }
    Status rhs(Real_t t, const Real_t * const y, Real_t * const dydt) override {
        npy_intp dims[1] { static_cast<npy_intp>(this->ny) } ;
        PyObject * py_yarr = PyArray_SimpleNewFromData(
            1, dims, this->real_type_tag, static_cast<void*>(const_cast<Real_t*>(y)));
        PyObject * py_dydt = PyArray_SimpleNewFromData(
            1, dims, this->real_type_tag, static_cast<void*>(dydt));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        PyObject * py_arglist = Py_BuildValue("(OOO)", t_scalar, py_yarr, py_dydt);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_rhs, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_dydt);
        Py_DECREF(py_yarr);
        Py_DECREF(t_scalar);
        this->nfev++;
        return handle_status_(py_result, "rhs");
    }
    AnyODE::Status quads(Real_t t, const Real_t * const y, Real_t * const out) override {
        npy_intp ydims[1] { static_cast<npy_intp>(this->ny) };
        npy_intp rdims[1] { static_cast<npy_intp>(this->get_nquads()) };
        PyObject * py_yarr = PyArray_SimpleNewFromData(
            1, ydims, this->real_type_tag, static_cast<void*>(const_cast<Real_t*>(y)));
        PyObject * py_out = PyArray_SimpleNewFromData(
            1, rdims, this->real_type_tag, static_cast<void*>(out));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        PyObject * py_arglist = Py_BuildValue("(OOO)", t_scalar, py_yarr, py_out);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_quads, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_out);
        Py_DECREF(py_yarr);
        Py_DECREF(t_scalar);
        return handle_status_(py_result, "quads");
    }
    AnyODE::Status roots(Real_t t, const Real_t * const y, Real_t * const out) override {
        npy_intp ydims[1] { static_cast<npy_intp>(this->ny) };
        npy_intp rdims[1] { static_cast<npy_intp>(this->get_nroots()) };
        PyObject * py_yarr = PyArray_SimpleNewFromData(
            1, ydims, this->real_type_tag, static_cast<void*>(const_cast<Real_t*>(y)));
        PyObject * py_out = PyArray_SimpleNewFromData(
            1, rdims, this->real_type_tag, static_cast<void*>(out));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        PyObject * py_arglist = Py_BuildValue("(OOO)", t_scalar, py_yarr, py_out);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_roots, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_out);
        Py_DECREF(py_yarr);
        Py_DECREF(t_scalar);
        return handle_status_(py_result, "roots");
    }
    AnyODE::Status call_py_jac(Real_t t, const Real_t * const y, const Real_t * const fy,
                               PyObject * py_jmat, Real_t * const dfdt){
        npy_intp ydims[1] { static_cast<npy_intp>(this->ny) };
        PyObject * py_yarr = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(y));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * py_dfdt = (dfdt == nullptr) ? Py_BuildValue("") : PyArray_SimpleNewFromData(
            1, ydims, this->real_type_tag, static_cast<void*>(dfdt));
        PyObject * py_fy;
        if (fy) {
            py_fy = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(fy));
            PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_fy), NPY_ARRAY_WRITEABLE);  // make fy read-only
        } else {
            py_fy = Py_BuildValue(""); // Py_None with incref
        }
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        // Call jac with signature: (t, y[:], Jmat[:, :], dfdt[:]=None, fy[:]=None)
        // (NumPy takes cares of row vs. column major ordering. User responsible for dense/banded.)
        PyObject * py_arglist = Py_BuildValue("(OOOOO)", t_scalar, py_yarr, py_jmat, py_dfdt, py_fy);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_jac, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_fy);
        Py_DECREF(py_dfdt);
        Py_DECREF(py_yarr);
        Py_DECREF(t_scalar);
        this->njev++;
        return handle_status_(py_result, "jac");
    }
    AnyODE::Status jtimes(const Real_t * const v, Real_t * const Jv,
                          Real_t t, const Real_t * const y, const Real_t * const fy) override {
        npy_intp ydims[1] { static_cast<npy_intp>(this->ny) };
        PyObject * py_yarr = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(y));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * py_varr = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(v));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_varr), NPY_ARRAY_WRITEABLE);  // make varr read-only
        PyObject * py_Jv = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *> (Jv));
        PyObject * py_fy;
        if (fy) {
            py_fy = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(fy));
            PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_fy), NPY_ARRAY_WRITEABLE);  // make fy read-only
        } else {
            py_fy = Py_BuildValue(""); // Py_None with incref
        }
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        // Call jtimes with signature: (v[:], Jv[:], t, y[:], fy[:])
        PyObject * py_arglist = Py_BuildValue("(OOOOO)", py_varr, py_Jv, t_scalar, py_yarr, py_fy);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_jtimes, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_Jv);
        Py_DECREF(py_fy);
        Py_DECREF(py_yarr);
        Py_DECREF(py_varr);
        Py_DECREF(t_scalar);
        this->njvev++;
        return handle_status_(py_result, "jtimes");
    }
    AnyODE::Status prec_setup(Real_t t,
                              const Real_t * const ANYODE_RESTRICT y,
                              const Real_t * const ANYODE_RESTRICT fy,
                              bool jok,
                              bool& jac_recomputed,
                              Real_t gamma) override {
        npy_intp ydims[1] { static_cast<npy_intp>(this->ny) };
        PyObject * py_yarr = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(y));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * py_fy;
        if (fy) {
            py_fy = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(fy));
            PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_fy), NPY_ARRAY_WRITEABLE);  // make fy read-only
        } else {
            py_fy = Py_BuildValue(""); // Py_None with incref
        }
        PyObject * py_jok;
        if (jok) {
            Py_INCREF(Py_True);
            py_jok = Py_True;
        } else {
            Py_DECREF(Py_False);
            py_jok = Py_False;
        }
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        PyObject * py_gamma = PyArray_Scalar(&gamma, this->real_type_descr, NULL);
        // Call prec_setup with signature: (t, y[:], fy[:], jok, gamma) -> (status, jac_recomputed)
        PyObject * py_arglist = Py_BuildValue("(OOOOO)", t_scalar, py_yarr, py_fy, py_jok, py_gamma);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_prec_setup, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_jok);
        Py_DECREF(py_fy);
        Py_DECREF(py_yarr);
        Py_DECREF(py_gamma);
        Py_DECREF(t_scalar);
        if (py_result == nullptr){
            throw std::runtime_error("prec_setup failed");
        }
        AnyODE::Status status;
        PyObject * zero = PyLong_FromLong(0);
        PyObject * one = PyLong_FromLong(1);
        PyObject * py_status = PyObject_GetItem(py_result, zero);
        if (py_status == nullptr) {
            throw std::runtime_error("prec_setup needs to return 2 integers as a tuple");
        } else {
            long st = PyLong_AsLong(py_status);
            if (st == -1 && PyErr_Occurred()) {
                throw std::runtime_error("status (first returned item) needs to be an integer");
            } else {
                if (st == static_cast<int>(AnyODE::Status::success)) {
                    status = AnyODE::Status::success;
                } else if (st == static_cast<int>(AnyODE::Status::recoverable_error)) {
                    status = AnyODE::Status::recoverable_error;
                } else if (st == static_cast<int>(AnyODE::Status::unrecoverable_error)) {
                    status = AnyODE::Status::unrecoverable_error;
                } else {
                    throw std::runtime_error("Unknown status code (first returned item) returned from prec_setup");
                }
            }
        }
        Py_DECREF(py_status);
        Py_DECREF(zero);
        PyObject * py_jac_recomputed = PyObject_GetItem(py_result, one);
        if (py_jac_recomputed == nullptr) {
            throw std::runtime_error("prec_setup needs to return 2 integers as a tuple");
        } else {
            jac_recomputed = PyObject_IsTrue(py_jac_recomputed);
        }
        Py_DECREF(py_jac_recomputed);
        Py_DECREF(one);
        Py_DECREF(py_result);
        return status;
    }
    AnyODE::Status prec_solve_left(const Real_t t,
                                   const Real_t * const ANYODE_RESTRICT y,
                                   const Real_t * const ANYODE_RESTRICT fy,
                                   const Real_t * const ANYODE_RESTRICT r,
                                   Real_t * const ANYODE_RESTRICT z,
                                   Real_t gamma,
                                   Real_t delta,
                                   const Real_t * const ANYODE_RESTRICT ewt)
    {
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);
        PyObject * py_gamma = PyArray_Scalar(&gamma, this->real_type_descr, NULL);
        PyObject * py_delta = PyArray_Scalar(&gamma, this->real_type_descr, NULL);
        npy_intp ydims[1] { static_cast<npy_intp>(this->ny) };
        PyObject * py_y_arr = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(y));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_y_arr), NPY_ARRAY_WRITEABLE);  // make read-only
        PyObject * py_fy;
        if (fy) {
            py_fy = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(fy));
            PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_fy), NPY_ARRAY_WRITEABLE);  // make read-only
        } else {
            py_fy = Py_BuildValue(""); // Py_None with incref
        }
        PyObject * py_r_arr = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(y));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_r_arr), NPY_ARRAY_WRITEABLE);  // make read-only
        PyObject * py_z_arr = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(y));
        PyObject * py_ewt;
        if (ewt) {
            py_ewt = PyArray_SimpleNewFromData(1, ydims, this->real_type_tag, const_cast<Real_t *>(ewt));
            PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_ewt), NPY_ARRAY_WRITEABLE);  // make read-only
        } else {
            py_ewt = Py_BuildValue(""); // Py_None with incref
        }
        PyObject * py_arglist = Py_BuildValue("(OOOOOOOO)", t_scalar, py_y_arr, py_fy, py_r_arr, py_z_arr, py_gamma, py_delta, py_ewt);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_prec_setup, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(t_scalar);
        Py_DECREF(py_gamma);
        Py_DECREF(py_delta);
        Py_DECREF(py_y_arr);
        Py_DECREF(py_fy);
        Py_DECREF(py_r_arr);
        Py_DECREF(py_z_arr);
        Py_DECREF(py_ewt);
        return status;
    }
    AnyODE::Status dense_jac_cmaj(Real_t t, const Real_t * const y, const Real_t * const fy,
                                  Real_t * const jac, long int ldim, Real_t * const dfdt=nullptr) override {
        npy_intp Jdims[2] { static_cast<npy_intp>(this->ny), static_cast<npy_intp>(this->ny) };
        npy_intp strides[2] { sizeof(Real_t), static_cast<npy_intp>(ldim*sizeof(Real_t)) };
        int flags = NPY_ARRAY_ALIGNED | NPY_ARRAY_WRITEABLE;
        if (ldim == Jdims[0]) {
            flags |= NPY_ARRAY_F_CONTIGUOUS;
        }
        PyObject * py_jmat = PyArray_New(
            &PyArray_Type, 2, Jdims, this->real_type_tag, strides,
            static_cast<void *>(const_cast<Real_t *>(jac)), sizeof(Real_t),
            flags, nullptr);
        AnyODE::Status status = call_py_jac(t, y, fy, py_jmat, dfdt);
        Py_DECREF(py_jmat);
        return status;
    }
    AnyODE::Status dense_jac_rmaj(Real_t t, const Real_t * const y, const Real_t * const fy,
                                  Real_t * const jac, long int ldim, Real_t * const dfdt=nullptr) override {
        npy_intp Jdims[2] { static_cast<npy_intp>(this->ny), static_cast<npy_intp>(this->ny) };
        npy_intp strides[2] { static_cast<npy_intp>(ldim*sizeof(Real_t)), sizeof(Real_t) };
        int flags = NPY_ARRAY_ALIGNED| NPY_ARRAY_WRITEABLE;
        if (ldim == Jdims[1]) {
            flags |= NPY_ARRAY_C_CONTIGUOUS;
        }
        PyObject * py_jmat = PyArray_New(
            &PyArray_Type, 2, Jdims, this->real_type_tag, strides,
            static_cast<void *>(const_cast<Real_t *>(jac)), sizeof(Real_t), flags, nullptr);
        AnyODE::Status status = call_py_jac(t, y, fy, py_jmat, dfdt);
        Py_DECREF(py_jmat);
        return status;
    }
    AnyODE::Status sparse_jac_csc(Real_t t, const Real_t * const y, const Real_t * const fy,
                                  Real_t * const data, Index_t * const colptrs, Index_t * const rowvals) override {
        npy_intp y_dims[1] { static_cast<npy_intp>(this->ny) };
        npy_intp data_dims[1] { static_cast<npy_intp>(this->nnz) };
        npy_intp colptrs_dims[1] { static_cast<npy_intp>(this->ny + 1) };

        PyObject * py_yarr = PyArray_SimpleNewFromData(1, y_dims, this->real_type_tag, const_cast<Real_t *>(y));
        PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_yarr), NPY_ARRAY_WRITEABLE);  // make yarr read-only
        PyObject * py_fy;
        if (fy) {
            py_fy = PyArray_SimpleNewFromData(1, y_dims, this->real_type_tag, const_cast<Real_t *>(fy));
            PyArray_CLEARFLAGS(reinterpret_cast<PyArrayObject*>(py_fy), NPY_ARRAY_WRITEABLE);  // make fy read-only
        } else {
            py_fy = Py_BuildValue(""); // Py_None with incref
        }
        PyObject * py_data = PyArray_SimpleNewFromData(1, data_dims, this->real_type_tag, static_cast<Real_t *>(data));
        PyObject * py_colptrs = PyArray_SimpleNewFromData(1, colptrs_dims, this->index_type_tag, static_cast<Index_t *>(colptrs));
        PyObject * py_rowvals = PyArray_SimpleNewFromData(1, data_dims, this->index_type_tag, static_cast<Index_t *>(rowvals));
        PyObject * t_scalar = PyArray_Scalar(&t, this->real_type_descr, NULL);

        // Call sparse jac with signature: (t, y[:], data[:], colptrs[:], rowvals[:]
        PyObject * py_arglist = Py_BuildValue("(OOOOO)", t_scalar, py_yarr, py_data, py_colptrs, py_rowvals);
        PyObject * py_result = PyEval_CallObjectWithKeywords(this->py_jac, py_arglist, this->py_kwargs);
        Py_DECREF(py_arglist);
        Py_DECREF(py_fy);
        Py_DECREF(py_yarr);
        Py_DECREF(py_data);
        Py_DECREF(py_colptrs);
        Py_DECREF(py_rowvals);
        Py_DECREF(t_scalar);
        this->njev++;
        return handle_status_(py_result, "jac");
    }
    AnyODE::Status banded_jac_cmaj(Real_t t, const Real_t * const y, const Real_t * const fy,
                                   Real_t * const jac, long int ldim) override {
        npy_intp Jdims[2] { 1 + this->mlower + this->mupper, static_cast<npy_intp>(this->ny) };
        npy_intp strides[2] { sizeof(Real_t), static_cast<npy_intp>(ldim*sizeof(Real_t)) };
        int flags = NPY_ARRAY_ALIGNED | NPY_ARRAY_WRITEABLE;
        if (ldim == Jdims[0] ) {
            flags |= NPY_ARRAY_F_CONTIGUOUS;
        }
        PyObject * py_jmat = PyArray_New(
            &PyArray_Type, 2, Jdims, this->real_type_tag, strides,
            static_cast<void *>(const_cast<Real_t *>(jac)), sizeof(Real_t), flags, nullptr);
        AnyODE::Status status = call_py_jac(t, y, fy, py_jmat, nullptr);
        Py_DECREF(py_jmat);
        return status;
    }
};
END_NAMESPACE(AnyODE)
