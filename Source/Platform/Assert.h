#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"

//
// Setup state
//

// this is for cross-platform code that uses gcc's unused attribute to remove locals
#define HELIUM_ASSERT_ONLY

// this sets the master flag if we are going to compile in asserts or not
#if !defined( HELIUM_ASSERT_ENABLED )
# if !defined( HELIUM_RELEASE ) && !defined( HELIUM_PROFILE )
#  define HELIUM_ASSERT_ENABLED 1
# else
#  define HELIUM_ASSERT_ENABLED 0
# endif
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
#  define HELIUM_ISSUE_BREAK() __builtin_debugtrap()
# elif HELIUM_CC_SNC
#  define HELIUM_ISSUE_BREAK() __builtin_snpause()
# else
#  error Platform has no break intrinsic!
# endif
# define HELIUM_FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#if HELIUM_CC_CLANG

# define HELIUM_CC_CLANG_PRAGMA( PRAGMA ) _Pragma( #PRAGMA )
# define HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	HELIUM_CC_CLANG_PRAGMA( clang diagnostic push ) \
	HELIUM_CC_CLANG_PRAGMA( clang diagnostic ignored "-Wunused-value" )
# define HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	HELIUM_CC_CLANG_PRAGMA( clang diagnostic pop )

#else // HELIUM_CC_CLANG

# define HELIUM_CC_CLANG_PRAGMA( PRAGMA )
# define HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING
# define HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING

#endif // HELIUM_CC_CLANG

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
		static void SetFatal(bool value);
		//@}

	private:
		/// Non-zero if the assert handler is currently active, zero if not.
		static int32_t sm_active;

		/// call abort when this is true and an assertion fails 
		static bool sm_fatal;

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
	( Helium::Assert::Trigger( EXP, HELIUM_FUNCTION_NAME, __FILE__, __LINE__, __VA_ARGS__ ) ? ( HELIUM_ISSUE_BREAK(), true ) : false )

/// Trigger a debug breakpoint unconditionally in non-release builds.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK_MSG(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_BREAK() \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	HELIUM_TRIGGER_ASSERT_HANDLER( NULL, NULL ); \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING

/// Trigger a debug breakpoint with a customized message unconditionally in non-release builds.
///
/// @param[in] ...  Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_BREAK_MSG( ... ) \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	HELIUM_TRIGGER_ASSERT_HANDLER( NULL, __VA_ARGS__ ); \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds.
///
/// @param[in] EXP  Expression to evaluate.
///
/// @see HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_ASSERT( EXP ) \
	{ \
		if( !( EXP ) ) \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
			HELIUM_TRIGGER_ASSERT_HANDLER( #EXP, NULL ); \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	}

/// Trigger a debug breakpoint with a customized message if the result of an expression is false in non-release builds.
///
/// @param[in] EXP      Expression to evaluate.
/// @param[in] ...      Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_BREAK(), HELIUM_BREAK_MSG() HELIUM_VERIFY(), HELIUM_VERIFY_MSG()
#define HELIUM_ASSERT_MSG( EXP, ... ) \
	{ \
		if( !( EXP ) ) \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
			HELIUM_TRIGGER_ASSERT_HANDLER( #EXP, __VA_ARGS__ ); \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	}

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds while still evaluating the
/// expression in release builds, and evaluating to the result of the expression.
///
/// @param[in] EXP  Expression to evaluate.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY_MSG()
#define HELIUM_VERIFY( EXP ) \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	( ( EXP ) ? true : ( HELIUM_TRIGGER_ASSERT_HANDLER( #EXP, NULL ), false ) ) \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING

/// Trigger a debug breakpoint with a customized message if the result of an expression is false in non-release builds
/// while still evaluating the expression in release builds, and evaluating to the result of the expression.
///
/// @param[in] EXP      Expression to evaluate.
/// @param[in] ...      Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY()
#define HELIUM_VERIFY_MSG( EXP, ... ) \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	( ( EXP ) ? true : ( HELIUM_TRIGGER_ASSERT_HANDLER( #EXP, NULL ), false ) ) \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING

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
#define HELIUM_VERIFY( EXP ) \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	( ( EXP ) ? true : false ) \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING

/// Trigger a debug breakpoint if the result of an expression is false in non-release builds while still evaluating the
/// expression in release builds, and evaluating to the result of the expression.
///
/// @param[in] EXP      Expression to evaluate.
/// @param[in] ...      Message to display if the assertion is triggered.
///
/// @see HELIUM_ASSERT(), HELIUM_ASSERT_MSG(), HELIUM_BREAK(), HELIUM_BREAK_MSG(), HELIUM_VERIFY()
#define HELIUM_VERIFY_MSG( EXP, ... ) \
HELIUM_CC_PUSH_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING \
	( ( EXP ) ? true : false ) \
HELIUM_CC_POP_SILENCE_UNUSED_EXPRESSION_RESULT_WARNING

#endif

//
// Compile time
//

#define HELIUM_COMPILE_ASSERT(exp) static_assert(exp, #exp)
