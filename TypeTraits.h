#pragma once

#include "Platform/System.h"


//
// Windows
//

#if HELIUM_OS_WIN


# if HELIUM_CPP11
#  include <type_traits>
# else // HELIUM_CPP11
#  include <type_traits>

namespace std
{
	namespace tr1 {}
	using namespace tr1;
}

# endif // HELIUM_CPP11


//
// Linux
//

#elif HELIUM_OS_LINUX // HELIUM_OS_WIN

# if HELIUM_CPP11
#  include <type_traits>
#   if __GLIBCXX__ >= 20131008 // 4.8 or later

namespace std
{
	template< class T > struct has_trivial_assign : has_trivial_copy_assign< T > {};
	template< class T > struct has_trivial_constructor : has_trivial_default_constructor< T > {};
	template< class T > struct has_trivial_destructor : is_trivially_destructible< T > {};
	template< class T > struct has_trivial_copy : has_trivial_copy_assign< T > {};
}

#  else // earlier than 4.8

namespace std
{
	template< class T > struct has_trivial_assign : has_trivial_copy_assign< T > {};
	template< class T > struct has_trivial_constructor : has_trivial_default_constructor< T > {};
	template< class T > struct has_trivial_copy : has_trivial_copy_assign< T > {};
}

#  endif // glib version check

# else // HELIUM_CPP11

#  include <tr1/type_traits>

namespace std
{
	namespace tr1 {}
	using tr1::true_type;
	using tr1::false_type;
	using tr1::integral_constant;
	using tr1::is_base_of;
	using tr1::is_pointer;
	using tr1::is_reference;
	using tr1::is_signed;
	using tr1::is_array;
	using tr1::remove_cv;
	using tr1::remove_extent;
	using tr1::remove_reference;
	using tr1::alignment_of;
	using tr1::has_trivial_assign;
	using tr1::has_trivial_constructor;
	using tr1::has_trivial_destructor;
	using tr1::has_trivial_copy;
}

# endif // HELIUM_CPP11


//
// Mac
//

#elif HELIUM_OS_MAC // HELIUM_OS_LINUX

# if HELIUM_CC_GCC

#  include <tr1/type_traits>

namespace std
{
	namespace tr1 {}
	using tr1::true_type;
	using tr1::false_type;
	using tr1::integral_constant;
	using tr1::is_base_of;
	using tr1::is_pointer;
	using tr1::is_reference;
	using tr1::is_signed;
	using tr1::is_array;
	using tr1::remove_cv;
	using tr1::remove_extent;
	using tr1::remove_reference;
	using tr1::alignment_of;
	using tr1::has_trivial_assign;
	using tr1::has_trivial_constructor;
	using tr1::has_trivial_destructor;
	using tr1::has_trivial_copy;
}

# elif HELIUM_CC_CLANG // HELIUM_CC_GCC

#  include <type_traits>

namespace std
{
	template< class T > struct has_trivial_assign : is_trivially_assignable< T, T > {};
	template< class T > struct has_trivial_constructor : is_trivially_constructible< T > {};
	template< class T > struct has_trivial_destructor : is_trivially_destructible< T > {};
	template< class T > struct has_trivial_copy : is_trivially_copyable< T > {};
}

# endif // HELIUM_CC_CLANG

#endif // HELIUM_OS_MAC
