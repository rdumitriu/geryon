///
/// \file platform.hpp
///
///  Created on: Jun 16, 2014
///  Author: rdumitriu at gmail.com
///

#ifndef GERYON_PLATFORM_HPP_
#define GERYON_PLATFORM_HPP_

#define G_PLATFORM_LIN 1
#define G_PLATFORM_WIN 2


#if defined(__WIN32__) || defined(_WIN32)
  #define G_HAS_WIN G_PLATFORM_WIN
  #define G_PLATFORM G_PLATFORM_WIN
#else
  #define G_HAS_LIN G_PLATFORM_LIN
  #define G_PLATFORM G_PLATFORM_LIN
#endif


#if G_PLATFORM == G_PLATFORM_WIN
  #define G_FUNCTION_EXPORT __declspec(dllexport)
#elif G_PLATFORM == G_PLATFORM_LIN
  #if defined(__GNUC__) && __GNUC__ >= 4
    #define G_FUNCTION_EXPORT __attribute__ ((visibility("default")))
  # else
    #define G_FUNCTION_EXPORT
  #endif
#endif

#define G_MODULE_EXPORT extern "C" G_FUNCTION_EXPORT
#define G_CLASS_EXPORT G_FUNCTION_EXPORT

#endif
