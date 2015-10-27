// Predefined symbols and macros -*- C++ -*-

// Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
// Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#ifndef _CXXCONFIG
#define _CXXCONFIG 1

// Pick up any OS-specific definitions.
#include <bits/os_defines.h>

// Pick up any CPU-specific definitions.
#include <bits/cpu_defines.h>

// The current version of the C++ library in compressed ISO date format.
#define __GLIBCXX__ 20051216

// Macros for various namespace association schemes and modes.
// _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
#ifdef _GLIBCXX_DEBUG
# define _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG 1
#endif

#define _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION 1


// Macros for namespaces.
// _GLIBCXX_BEGIN_NAMESPACE
// _GLIBCXX_END_NAMESPACE
// _GLIBCXX_BEGIN_NESTED_NAMESPACE
// _GLIBCXX_END_NESTED_NAMESPACE
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
# define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y) namespace X { namespace Y {
# define _GLIBCXX_END_NESTED_NAMESPACE } }
# define _GLIBCXX_BEGIN_NAMESPACE(X) _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, _6)
# define _GLIBCXX_END_NAMESPACE _GLIBCXX_END_NESTED_NAMESPACE
#else
# define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X { 
# define _GLIBCXX_END_NAMESPACE } 
# if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
#  define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y) namespace X { namespace Y {
#  define _GLIBCXX_END_NESTED_NAMESPACE  } }
# else
#  define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y) _GLIBCXX_BEGIN_NAMESPACE(X)
#  define _GLIBCXX_END_NESTED_NAMESPACE _GLIBCXX_END_NAMESPACE
# endif
#endif

// Namespace associations for versioning mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
namespace std
{
  namespace _6 { }
  using namespace _6 __attribute__ ((strong));
}

// In addition, other supported namespace configurations.
namespace __gnu_cxx 
{ 
  namespace _6 { }
  using namespace _6 __attribute__ ((strong));
}

namespace __gnu_ext
{ 
  namespace _6 { }
  using namespace _6 __attribute__ ((strong));
}

namespace std
{
  namespace tr1 
  { 
    namespace _6 { }
    using namespace _6 __attribute__ ((strong));
  }
}
#endif

// Namespace associations for debug mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
namespace std
{ 
  namespace __gnu_norm { }

#if 1
  namespace __gnu_debug_def { }
  namespace __gnu_debug { using namespace __gnu_debug_def; } 
  using namespace __gnu_debug_def __attribute__ ((strong));
#else
  namespace __gnu_debug { namespace detail { } }
  using namespace __gnu_debug __attribute__ ((strong));
#endif
}

# define _GLIBCXX_STD __gnu_norm
# define _GLIBCXX_EXTERN_TEMPLATE 0
# if __NO_INLINE__ && !__GXX_WEAK__
#  warning debug mode without inlining may fail due to lack of weak symbols
# endif
#else
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
# define _GLIBCXX_STD _6
#else
# define _GLIBCXX_STD std
#endif
#endif


// Allow use of "export template." This is currently not a feature
// that g++ supports.
// #define _GLIBCXX_EXPORT_TEMPLATE 1

// Allow use of the GNU syntax extension, "extern template." This
// extension is fully documented in the g++ manual, but in a nutshell,
// it inhibits all implicit instantiations and is used throughout the
// library to avoid multiple weak definitions for required types that
// are already explicitly instantiated in the library binary. This
// substantially reduces the binary size of resulting executables.
#ifndef _GLIBCXX_EXTERN_TEMPLATE
# define _GLIBCXX_EXTERN_TEMPLATE 1
#endif


// Certain function definitions that are meant to be overridable from
// user code are decorated with this macro.  For some targets, this
// macro causes these definitions to be weak.
#ifndef _GLIBCXX_WEAK_DEFINITION
# define _GLIBCXX_WEAK_DEFINITION
#endif

// The remainder of the prewritten config is automatic; all the
// user hooks are listed above.

// Create a boolean flag to be used to determine if --fast-math is set.
#ifdef __FAST_MATH__
# define _GLIBCXX_FAST_MATH 1
#else
# define _GLIBCXX_FAST_MATH 0
#endif

// This marks string literals in header files to be extracted for eventual
// translation.  It is primarily used for messages in thrown exceptions; see
// src/functexcept.cc.  We use __N because the more traditional _N is used
// for something else under certain OSes (see BADNAMES).
#define __N(msgid)     (msgid)

// End of prewritten config; the discovered settings follow.