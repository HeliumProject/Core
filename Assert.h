#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"

//
// Setup state
//

// this is for cross-platform code that uses gcc's unused attribute to remove locals
#define HELIUM_ASSERT_ONLY

// this sets the master flag if we are going to compile in asserts or not
#if !defined( NDEBUG ) && !defined( HELIUM_ASSERT_ENABLED )
#define HELIUM_ASSERT_ENABLED 1
#endif


//
// Define the main macros
//

#if HELIUM_CC_CL
# ifdef _MANAGED
#  define HELIUM_ISSUE_BREAK() System::Diagnostics::Debugger::Break()
# else //_MANAGED
#  define HELIUM_ISSUE_BREAK() __debugbreak()
# endif //_MANAGED
# define HELIUM_FUNCTION_NAME __FUNCSIG__
#else
# if HELIUM_CC_GCC
#  define HELIUM_ISSUE_BREAK() __builtin_trap()
# elif HELIUM_CC_CLANG
#  define HELIUM_ISSUE_BREAK() __builtin_trap()
# elif HELIUM_CC_SNC
#  define HELIUM_ISSUE_BREAK() __builtin_snpause()
# else
#  error Platform has no break intrinsic!
# endif
# define HELIUM_FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#if HELIUM_CC_CLANG
# pragma clang diagnostic ignored "-Wunused-value"
#endif

namespace Helium
{
	HELIUM_PLATFORM_API void FatalExit( int exitCode );
}

#if HELIUM_ASSERT_ENABLED

namespace Helium
{
	/// Assert utility functions.
	class HELIUM_PLATFORM_API Assert
	{
	public:
		/// @name Static Utility Functions
		//@{
		static bool Trigger( const char* pExpression, const char* pFunction, const char* pFile, int line, const char* pMessage, ... );
		//@}

	private:
		/// Non-zero if the assert handler is currently active, zero if not.
		static volatile int32_t sm_active;

		/// @name Private Static Utility Functions
		//@{
		static bool TriggerImplementation( const char* pMessageText );
		//@}
	};
}

/// Trigger the assert handler function and trigger a break if necessary
///
/// @param[in] EXP      Expression string.
/// @param[in] MESSAGE  Message string.
#define HELIUM_TRIGGER_ASSERT_HANDLER( EXP, ... ) \
	( Helium::Assert::Trigger( EXP, TXT(HELIUM_FUNCTION_NAME), TXT(__FILE__), __LINE__, __VA_ARGS__ ) ? ( HELIUM_ISSUE_BREAK(), true ) : false )

/// Trigger a debug breakpoint unconditionally in non-release builds.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK_MSG(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_BREAK() HELIUM_TRIGGER_ASSERT_HANDLER( NULL, NULL );

/// Trigger a debug breakpoint with a customized message unconditionally in non-release builds.
///
/// @param[in] ...  Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_BREAK_MSG( ... ) HELIUM_TRIGGER_ASSERT_HANDLER( NULL, __VA_ARGS__ );

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds.
///
/// @param[in] EXP  Expression to evaluate.
///
/// @see HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_ASSERT( EXP ) { if( !( EXP ) ) HELIUM_TRIGGER_ASSERT_HANDLER( TXT( #EXP ), NULL ); }

/// Trigger a debug breakpoint with a customized message if the result of an expression is false in non-release builds.
///
/// @param[in] EXP      Expression to evaluate.
/// @param[in] ...      Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_BREAK(), HELIUM_BREAK_MSG() HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_ASSERT_MSG( EXP, ... ) { if( !( EXP ) ) HELIUM_TRIGGER_ASSERT_HANDLER( TXT( #EXP ), __VA_ARGS__ ); }

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds while still evaluating the
/// expression in release builds, and evaluating to the result of the expression.
///
/// @param[in] EXP  Expression to evaluate.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY_MSG()
#define HELIUM_VERIFY( EXP ) ( ( EXP ) ? true : ( HELIUM_TRIGGER_ASSERT_HANDLER( TXT( #EXP ), NULL ), false ) )

/// Trigger a debug breakpoint with a customized message if the result of an expression is false in non-release builds
/// while still evaluating the expression in release builds, and evaluating to the result of the expression.
///
/// @param[in] EXP      Expression to evaluate.
/// @param[in] ...      Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY()
#define HELIUM_VERIFY_MSG( EXP, ... ) ( ( EXP ) ? true : ( HELIUM_TRIGGER_ASSERT_HANDLER( TXT( #EXP ), NULL ), false ) )

#else  // HELIUM_ASSERT_ENABLED

/// Trigger a debug breakpoint with a customized message unconditionally in non-release builds.
///
/// @param[in] ...  Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_BREAK()

/// Trigger a debug breakpoint with a customized message unconditionally in non-release builds.
///
/// @param[in] ...  Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_BREAK_MSG( ... )

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds.
///
/// @param[in] EXP  Expression to evaluate.
///
/// @see HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_ASSERT( EXP )

/// Trigger a debug breakpoint with a customized message if the result of an expression is false in non-release builds.
///
/// @param[in] EXP      Expression to evaluate.
/// @param[in] ...      Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_BREAK(), HELIUM_BREAK_MSG() HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_ASSERT_MSG( EXP, ... )

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds while still evaluating the
/// expression in release builds, and evaluating to the result of the expression.
///
/// @param[in] EXP      Expression to evaluate.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY_MSG()
#define HELIUM_VERIFY( EXP ) ( ( EXP ) ? true : false )

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds while still evaluating the
/// expression in release builds, and evaluating to the result of the expression.
///
/// @param[in] EXP      Expression to evaluate.
/// @param[in] ...      Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY()
#define HELIUM_VERIFY_MSG( EXP, ... ) ( ( EXP ) ? true : false )

#endif

//
// Compile time
//

#if HELIUM_CC_CL
# if _MSC_VER >= 1600
#  define HELIUM_COMPILE_ASSERT(exp) static_assert(exp, #exp)
# endif
#endif

#ifndef HELIUM_COMPILE_ASSERT
# define HELIUM_COMPILE_ASSERT_JOIN(X, Y) HELIUM_COMPILE_ASSERT_DO_JOIN(X, Y)
# define HELIUM_COMPILE_ASSERT_DO_JOIN(X, Y) HELIUM_COMPILE_ASSERT_DO_JOIN2(X, Y)
# define HELIUM_COMPILE_ASSERT_DO_JOIN2(X, Y) X##Y
# define HELIUM_COMPILE_ASSERT(exp) typedef char HELIUM_COMPILE_ASSERT_JOIN(__HELIUM_COMPILE_ASSERT__,__LINE__)[(exp)?1:-1]
#endif

//
// #pragma TODO("Do something!")
//

#define TODO_STRING2(x) #x
#define TODO_STRING(x) TODO_STRING2(x)
#define TODO(msg) message (__FILE__ "(" TODO_STRING(__LINE__) ") : TODO: " msg)
#define NYI(msg) TODO(__FUNCTION__ " is not yet implemented... " msg)
