#pragma once
#include <vector>

#if !defined(BEGIN_NAMESPACE)
#define BEGIN_NAMESPACE(s) namespace s{
#endif
#if !defined(END_NAMESPACE)
#define END_NAMESPACE(s) }
#endif

#ifndef ANYODE_RESTRICT
  #if defined(__GNUC__)
    #define ANYODE_RESTRICT __restrict__
  #elif defined(_MSC_VER) && _MSC_VER >= 1400
    #define ANYODE_RESTRICT __restrict
  // #elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
  //   #define ANYODE_RESTRICT restrict
  #else
    #define ANYODE_RESTRICT
  #endif
#endif

BEGIN_NAMESPACE(AnyODE)
template<class T> void ignore( const T& ) { } // ignore unused parameter warnings
END_NAMESPACE(AnyODE)
