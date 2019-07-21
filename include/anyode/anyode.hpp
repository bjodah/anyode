#pragma once

#include <memory>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include <anyode/anyode_util.hpp>

BEGIN_NAMESPACE(AnyODE)

enum class Status : int {success = 0, recoverable_error = 1, unrecoverable_error = -1};

template<typename Real_t=double, typename Index_t=int>
struct OdeSysBase {
    void * custom_data {nullptr};  // can be used from e.g. C (where you can't subclass)
    virtual ~OdeSysBase() {}

    virtual Index_t get_ny() const = 0;
    virtual int get_mlower() const { return -1; } // -1 denotes "not banded"
    virtual int get_mupper() const { return -1; } // -1 denotes "not banded"
    virtual Index_t get_nnz() const { return -1; } // -1 denotes "not sparse"
    virtual int get_nquads() const { return 0; } // Do not track quadratures by default;
    virtual int get_nroots() const { return 0; } // Do not look for roots by default;
    virtual Real_t get_dt0(Real_t /* t */, const Real_t * const /* y */) { return 0.0; }
    virtual Real_t get_dt_max(Real_t /* t */, const Real_t * const /* y */) { return 0.0; }

    virtual Status rhs(Real_t t, const Real_t * const y, Real_t * const f) = 0;
    virtual Status quads(Real_t /* t */, const Real_t * const /* y */, Real_t * const /* out */) {
        return Status::unrecoverable_error;
    }
    virtual Status roots(Real_t /* t */, const Real_t * const /* y */, Real_t * const /* out */) {
        return Status::unrecoverable_error;
    }
    virtual Status dense_jac_cmaj(Real_t /* t */,
                                  const Real_t * const ANYODE_RESTRICT /* y */,
                                  const Real_t * const ANYODE_RESTRICT /* fy */,
                                  Real_t * const ANYODE_RESTRICT /* jac */,
                                  long int /* ldim */,
                                  Real_t * const ANYODE_RESTRICT dfdt=nullptr){
        ignore(dfdt);
        return Status::unrecoverable_error;
    }
    virtual Status dense_jac_rmaj(Real_t /* t */,
                                  const Real_t * const ANYODE_RESTRICT /* y */,
                                  const Real_t * const ANYODE_RESTRICT /* fy */,
                                  Real_t * const ANYODE_RESTRICT /* jac */,
                                  long int /* ldim */,
                                  Real_t * const ANYODE_RESTRICT dfdt=nullptr){
        ignore(dfdt);
        return Status::unrecoverable_error;
    }
    virtual Status banded_jac_cmaj(Real_t /* t */,
                                   const Real_t * const ANYODE_RESTRICT /* y */,
                                   const Real_t * const ANYODE_RESTRICT /* fy */,
                                   Real_t * const ANYODE_RESTRICT /* jac */,
                                   long int /* ldim */){
        throw std::runtime_error("banded_jac_cmaj not implemented.");
        return Status::unrecoverable_error;
    }
    virtual Status sparse_jac_csc(Real_t /* t */,
                                  const Real_t * const ANYODE_RESTRICT /* y */,
                                  const Real_t * const ANYODE_RESTRICT /* fy */,
                                  Real_t * const ANYODE_RESTRICT /* data */,
                                  Index_t * const /* colptrs */,
                                  Index_t * const /* rowvals */
                                  ) {
        return Status::unrecoverable_error;
    }
    virtual Status sparse_jac_csr(Real_t /* t */,
                                  const Real_t * const ANYODE_RESTRICT /* y */,
                                  const Real_t * const ANYODE_RESTRICT /* fy */,
                                  Real_t * const ANYODE_RESTRICT /* data */,
                                  Index_t * const /* rowptrs */,
                                  Index_t * const /* colvals */
                                  ) {
        return Status::unrecoverable_error;
    }
    virtual Status jtimes(const Real_t * const ANYODE_RESTRICT /* vec */,
                          Real_t * const ANYODE_RESTRICT /* out */,
                          Real_t /* t */,
                          const Real_t * const ANYODE_RESTRICT /* y */,
                          const Real_t * const ANYODE_RESTRICT /* fy */
                          ) {
        return Status::unrecoverable_error;
    }
    virtual Status prec_setup(Real_t /* t */,
                              const Real_t * const ANYODE_RESTRICT /* y */,
                              const Real_t * const ANYODE_RESTRICT /* fy */,
                              bool /* jok */,
                              bool& /* jac_recomputed */,
                              Real_t /* gamma */)
    {
        return Status::unrecoverable_error;
    }
    virtual Status prec_solve_left(const Real_t /* t */,
                                   const Real_t * const ANYODE_RESTRICT /* y */,
                                   const Real_t * const ANYODE_RESTRICT /* fy */,
                                   const Real_t * const ANYODE_RESTRICT /* r */,
                                   Real_t * const ANYODE_RESTRICT /* z */,
                                   Real_t /* gamma */,
                                   Real_t /* delta */,
                                   const Real_t * const ANYODE_RESTRICT /* ewt */)
    {
        return Status::unrecoverable_error;
    }
};

END_NAMESPACE(AnyODE)
