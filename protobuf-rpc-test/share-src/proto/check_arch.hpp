#ifndef CHECK_ARCH_HPP__
#define CHECK_ARCH_HPP__

#if defined(BOOST_VERSION)
#if BOOST_VERSION >= 105500
#include <boost/predef.h>

#ifndef BOOST_ARCH_X86
#error "this library support X86/X86_64 only"		
#endif 

#endif
#endif



#endif// !CHECK_ARCH_HPP__
