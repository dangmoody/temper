/*
===========================================================================

Temper
v2.0.0

Distributed under MIT License:
Copyright (c) 2021:

	Dan Moody (daniel.guy.moody@gmail.com)
	TODO(MY): YOUR NAME HERE (email optional)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


TODO: documentation here

===========================================================================
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#elif defined( __GNUC__ )	// defined( __clang__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
// DM: only disabling this one on gcc to avoid a warning that gets generated when trying to convert function pointers to void*
// if anyone knows of a better way to get around that without disabling all pedantic warnings I'd love to hear about it
// submit an issue telling me how: https://github.com/dangmoody/Tantrum/issues
#pragma GCC diagnostic ignored "-Wpedantic"
#endif	// defined( __clang__ )

#if defined( __linux__ ) || defined( __APPLE__ )
#pragma push_macro( "_POSIX_C_SOURCE" )
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#if defined( _WIN32 )
#include <Windows.h>
#elif defined( __APPLE__ ) || defined( __linux__ )
#include <unistd.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/stat.h>
#include <linux/limits.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#if defined( _WIN32 )
#define TANTRUM_API	__declspec( dllexport )
#elif defined( __APPLE__ ) || defined( __linux__ )
#define TANTRUM_API	__attribute__( ( visibility( "default" ) ) )
#else
#error Uncrecognised platform.  It appears Tantrum does not support it.  If you think this is a bug, please submit an issue at https://github.com/dangmoody/Tantrum/issues
#endif

#ifdef __cplusplus
#define TANTRUM_EXTERN_C extern "C"
#else
#define TANTRUM_EXTERN_C
#endif

//==========================================================
// Public API
//==========================================================

// MY: I'd like to eventually add more security around this,
// such as ensuring it's only ever called/used once and thowing
// an error if it isn't. Maybe also (SOMEHOW) ensuring no test
// ever has a higher count.
#define TANTRUM_SETUP() \
do { \
	g_tantrumTestContext.totalTestsDeclared = __COUNTER__; \
\
	g_tantrumTestContext.timeUnit = TANTRUM_TIME_UNIT_MS; \
\
	QueryPerformanceFrequency( &g_tantrumTestContext.timestampFrequency ); \
} while ( 0 )

//----------------------------------------------------------

#define TANTRUM_RUN_ALL_TESTS()							TantrumExecuteAllTestsInternal()

//----------------------------------------------------------

#define TANTRUM_RUN_ALL_TESTS_WITH_ARGS( argc, argv )	TantrumExecuteAllTestsWithArgumentsInternal( argc, argv )

//==========================================================
// User-Overridable Preprocessor defines
//
// By default, Tantrum will use it's own internal implementations for things.
// But you can override them to help hook Tantrum into your codebase.
//==========================================================

#ifndef TANTRUM_EXIT_SUCCESS
#define TANTRUM_EXIT_SUCCESS			EXIT_SUCCESS
#endif

#ifndef TANTRUM_EXIT_FAILURE
#define TANTRUM_EXIT_FAILURE			EXIT_FAILURE
#endif

#ifndef TANTRUM_ASSERT_INTERNAL
#define TANTRUM_ASSERT_INTERNAL			assert
#endif

#ifndef TANTRUM_SNPRINTF
#define TANTRUM_SNPRINTF				snprintf
#endif

#ifndef TANTRUM_LOG
#define TANTRUM_LOG						TantrumLogInternal
#endif

#ifndef TANTRUM_LOG_WARNING
#define TANTRUM_LOG_WARNING				TantrumLogWarningInternal
#endif

#ifndef TANTRUM_LOG_ERROR
#define TANTRUM_LOG_ERROR				TantrumLogErrorInternal
#endif

#ifndef TANTRUM_FLOAT_EQUALS_INTERNAL
#define TANTRUM_FLOAT_EQUALS_INTERNAL	TantrumFloatEqualsInternal
#endif

#ifndef TANTRUM_STRING_EQUALS
#define TANTRUM_STRING_EQUALS			TantrumStringEqualsInternal
#endif

#ifndef TANTRUM_STRING_CONTAINS
#define TANTRUM_STRING_CONTAINS			TantrumStringContainsInternal
#endif

//==========================================================
// STRUCTS
//==========================================================

typedef uint32_t tantrumBool32;

//----------------------------------------------------------

typedef enum tantrumTestFlag_t {
	TANTRUM_TEST_FLAG_SHOULD_RUN		= 0,
	TANTRUM_TEST_FLAG_SHOULD_SKIP,
	TANTRUM_TEST_FLAG_DEPRECATED
} tantrumTestFlag_t;

typedef enum tantrumTimeUnit_t {
	TANTRUM_TIME_UNIT_CLOCKS			= 0,
	TANTRUM_TIME_UNIT_NS,
	TANTRUM_TIME_UNIT_US,
	TANTRUM_TIME_UNIT_MS,
	TANTRUM_TIME_UNIT_SECONDS
} tantrumTimeUnit_t;

//----------------------------------------------------------

typedef void( *testCallback_t )( void );

typedef tantrumBool32 ( *tantrumStringCompareFunc_t )( const char*, const char* );

typedef struct suiteTestInfo_t {
	testCallback_t		callback;
	tantrumTestFlag_t	testingFlag;
	bool				isExpectedToFail;
	double				testTimeTaken;
	// MY: TODO - fix padding
	const char*			testNameStr;
	const char*			suiteNameStr;
} suiteTestInfo_t;

typedef suiteTestInfo_t( *testInfoFetcherFunc_t )( void );

//----------------------------------------------------------

typedef struct tantrumTestContext_t {
#ifdef _WIN32
	LARGE_INTEGER		timestampFrequency;
#endif
	uint32_t			testsPassed;
	uint32_t			testsFailed;
	uint32_t			testsAborted;
	uint32_t			testsSkipped;
	uint32_t			totalTestsDeclared; // Gets set in the main function with a preprocessor
	uint32_t			totalTestsFoundWithFilters;
	uint32_t			totalTestsExecuted;
	uint32_t			totalErrorsInCurrentTests;
	tantrumBool32		partialFilter;
	tantrumBool32		isFilteringTests;
	tantrumTimeUnit_t	timeUnit;
	const char*			programName;
	const char*			suiteFilterPrevious;
	const char*			suiteFilter;
	const char*			testFilter;
} tantrumTestContext_t;

//----------------------------------------------------------

#if defined( _WIN32 )
#define TANTRUM_COLOR_DEFAULT			0x07
#define TANTRUM_COLOR_RED				0x0C
#define TANTRUM_COLOR_GREEN				0x02
#define TANTRUM_COLOR_YELLOW			0x0E

typedef uint32_t						tantrumTextColor_t;
#elif defined( __linux__ ) || defined( __APPLE__ )
#define TANTRUM_COLOR_DEFAULT			"\033[0m"//"\x1B[0m"
#define TANTRUM_COLOR_RED				"\033[0;31m"//"\x1B[31m"
#define TANTRUM_COLOR_GREEN				"\033[0;32m"//"\x1B[32m"
#define TANTRUM_COLOR_YELLOW			"\033[1;33m"

typedef const char*						tantrumTextColor_t;
#endif // defined( _WIN32 )

//==========================================================
// GLOBALS
//==========================================================

static tantrumTestContext_t				g_tantrumTestContext;

//==========================================================
// FUNCTIONS - BASE HELPER/UTILITY FUNCTIONS
//==========================================================

static void TantrumSetTextColorInternal( const tantrumTextColor_t color ) {
#if defined( _WIN32 )
	SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), (WORD) color );
#elif defined( __APPLE__ ) || defined( __linux__ )
	printf( "%s", color );
#else
#error Uncrecognised platform.  It appears Tantrum does not support it.  If you think this is a bug, please submit an issue at https://github.com/dangmoody/Tantrum/issues
#endif
}

//----------------------------------------------------------

static void TantrumLogInternal( const char* fmt, ... ) {
	TANTRUM_ASSERT_INTERNAL( fmt );

	va_list args;
	va_start( args, fmt );
	vprintf( fmt, args );
	va_end( args );
}

//----------------------------------------------------------

static void TantrumLogWarningInternal( const char* fmt, ... ) {
	TANTRUM_ASSERT_INTERNAL( fmt );

	va_list args;
	va_start( args, fmt );

	TantrumSetTextColorInternal( TANTRUM_COLOR_RED );

	printf( "WARNING: " );

	TantrumSetTextColorInternal( TANTRUM_COLOR_YELLOW );

	vprintf( fmt, args );

	TantrumSetTextColorInternal( TANTRUM_COLOR_DEFAULT );

	va_end( args );
}

//----------------------------------------------------------

static void TantrumLogErrorInternal( const char* fmt, ... ) {
	TANTRUM_ASSERT_INTERNAL( fmt );

	va_list args;
	va_start( args, fmt );

	TantrumSetTextColorInternal( TANTRUM_COLOR_RED );

	printf( "ERROR: " );

	TantrumSetTextColorInternal( TANTRUM_COLOR_YELLOW );

	vprintf( fmt, args );

	TantrumSetTextColorInternal( TANTRUM_COLOR_DEFAULT );

	va_end( args );
}

//----------------------------------------------------------

static uint32_t TantrumGetPercentInternal( uint32_t yourValue, uint32_t yourMax ) {
	return (uint32_t) ( ( ( (float) yourValue ) / ( (float) yourMax ) ) * 100 );
}

//----------------------------------------------------------

static bool TantrumFloatEqualsInternal( const float a, const float b, const float epsilon ) {
	return fabsf( a - b ) <= epsilon;
}

//----------------------------------------------------------

static const char* TantrumGetNextArgInternal( const int argIndex, const int argc, char** argv ) {
	TANTRUM_ASSERT_INTERNAL( argc );
	TANTRUM_ASSERT_INTERNAL( argv );

	return ( argIndex + 1 < argc ) ? argv[argIndex + 1] : NULL;
}

//----------------------------------------------------------

static tantrumBool32 TantrumStringEqualsInternal( const char* a, const char* b ) {
	if ( a && b ) {
		return strcmp( a, b ) == 0;
	} else if ( !a && !b ) {
		return true;
	}

	return false;
}

//----------------------------------------------------------

static tantrumBool32 TantrumStringContainsInternal( const char* str, const char* substring ) {
	return strstr( str, substring ) != NULL;
}

//==========================================================
// PREPROCESSORS - TANTRUM INTERNAL
//==========================================================

#define TANTRUM_DEFAULT_EPSILON					0.00001f

//----------------------------------------------------------

#define TANTRUM_CONCAT_INTERNAL_FINAL( a, b )	a ## b

//----------------------------------------------------------

#define TANTRUM_CONCAT_INTERNAL( a, b )			TANTRUM_CONCAT_INTERNAL_FINAL( a, b )

//----------------------------------------------------------

#define TANTRUM_DEFINE_TEST_INTERNAL( counter, suiteNameString, testName, testExpectedToFail, runFlag ) \
\
	/*1. Create a function with a name matching the test.*/ \
	void ( testName )( void ); \
\
	/*2. Create a testName_TestInfo struct that will just wrap the test information meta data.*/ \
	typedef struct TANTRUM_CONCAT_INTERNAL( testName, _TestInfo ) { \
		suiteTestInfo_t testInformation; \
	} TANTRUM_CONCAT_INTERNAL( testName, _TestInfo ); \
\
	/*3. Create a global instance of this new struct for us to access and write data about this test to.*/ \
	extern TANTRUM_CONCAT_INTERNAL( testName, _TestInfo ) TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ); \
	TANTRUM_CONCAT_INTERNAL( testName, _TestInfo ) TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ); \
\
	/*4. Create our invoker_n function. This is what the runner will loop over to grab the test function as well as all the information concerning it*/ \
	/* HACK(DM): I shouldn't have to add extern "C" before each declaration here to make this work for c++ compiled binaries.  I already did that at the top of the header! How is that NOT a compiler bug!? */ \
	TANTRUM_EXTERN_C suiteTestInfo_t TANTRUM_API TANTRUM_CONCAT_INTERNAL( tantrum_test_info_fetcher_, counter )( void ); \
	suiteTestInfo_t TANTRUM_CONCAT_INTERNAL( tantrum_test_info_fetcher_, counter )( void ) { \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.callback = testName; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.suiteNameStr = suiteNameString; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.isExpectedToFail = testExpectedToFail; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.testNameStr = #testName; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.testingFlag = runFlag; \
		return TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation; \
	} \
\
	/*5. The test function declared at Step1 is now declared here by the user*/ \
	void ( testName )( void )

//----------------------------------------------------------

#ifdef TANTRUM_SELF_TEST_ENABLED
#define TANTRUM_TEST( testName, testExpectedToFail, runFlag ) \
	TANTRUM_DEFINE_TEST_INTERNAL( __COUNTER__, NULL, testName, testExpectedToFail, runFlag )
#else
#define TANTRUM_TEST( testName, runFlag ) \
	TANTRUM_DEFINE_TEST_INTERNAL( __COUNTER__, NULL, testName, false, runFlag )
#endif //TANTRUM_SELF_TEST_ENABLED

//----------------------------------------------------------

#ifdef TANTRUM_SELF_TEST_ENABLED
#define TANTRUM_SUITE_TEST( suiteName, testName, testExpectedToFail, runFlag ) \
	TANTRUM_DEFINE_TEST_INTERNAL( __COUNTER__, #suiteName, testName, testExpectedToFail, runFlag )
#else
#define TANTRUM_SUITE_TEST( suiteName, testName, runFlag ) \
	TANTRUM_DEFINE_TEST_INTERNAL( __COUNTER__, #suiteName, testName, false, runFlag )
#endif //TANTRUM_SELF_TEST_ENABLED

//----------------------------------------------------------

#define TANTRUM_DECLARE_PARAMETRIC_SUITE_TEST_INTERNAL( suiteName, testName, testExpectedToFail, runFlag, ... )\
\
	/*1. Create a function with a name matching the test with the provided parameters.*/\
	void ( testName )( __VA_ARGS__ ); \
\
	/*2. Typedef this function type.*/ \
	typedef void ( *TANTRUM_CONCAT_INTERNAL( testName, _FuncType ) )( __VA_ARGS__ ); \
\
	/*3. Stash this function and run info in a struct unique to this.*/ \
	typedef struct TANTRUM_CONCAT_INTERNAL( testName, _ParametricTestInfo ) { \
		TANTRUM_CONCAT_INTERNAL( testName, _FuncType ) Callback; \
		tantrumTestFlag_t	testingFlag; \
		bool				isExpectedToFail; \
		const char*			testNameStr; \
		const char*			suiteNameStr; \
	} TANTRUM_CONCAT_INTERNAL( testName, _ParametricTestInfo ); \
\
	/*4. Create a global instance of this new struct for us to access and write data about this test to.*/ \
	extern TANTRUM_CONCAT_INTERNAL( testName, _ParametricTestInfo ) TANTRUM_CONCAT_INTERNAL( testName, _GlobalParametricInfo ); \
	TANTRUM_CONCAT_INTERNAL( testName, _ParametricTestInfo ) TANTRUM_CONCAT_INTERNAL( testName, _GlobalParametricInfo ); \
\
	/*5. Define an info binding function to tie all this information into the struct*/ \
	/* HACK(DM): I shouldn't have to add extern "C" before each declaration here to make this work for c++ compiled binaries.  I already did that at the top of the header! How is that NOT a compiler bug!? */ \
	TANTRUM_EXTERN_C void TANTRUM_API TANTRUM_CONCAT_INTERNAL( testName, _ParametricTestInfoBinder )( void ); \
	void TANTRUM_CONCAT_INTERNAL( testName, _ParametricTestInfoBinder )( void ) { \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalParametricInfo ).Callback = testName; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalParametricInfo ).testingFlag = runFlag; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalParametricInfo ).isExpectedToFail = testExpectedToFail; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalParametricInfo ).testNameStr = #testName; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalParametricInfo ).suiteNameStr = suiteName; \
	}\
\
	/*6. The test function declared at Step1 is now declared here by the user*/\
	void ( testName )( __VA_ARGS__ )

//----------------------------------------------------------

#ifdef TANTRUM_SELF_TEST_ENABLED
#define TANTRUM_DECLARE_PARAMETRIC_TEST( testName, testExpectedToFail, runFlag, ... )\
	TANTRUM_DECLARE_PARAMETRIC_SUITE_TEST_INTERNAL( NULL, testName, testExpectedToFail, runFlag, __VA_ARGS__ )
#else
#define TANTRUM_DECLARE_PARAMETRIC_TEST( testName, runFlag, ... )\
	TANTRUM_DECLARE_PARAMETRIC_SUITE_TEST_INTERNAL( NULL, testName, false, runFlag, __VA_ARGS__ )
#endif //TANTRUM_SELF_TEST_ENABLED

//----------------------------------------------------------

#ifdef TANTRUM_SELF_TEST_ENABLED
#define TANTRUM_DECLARE_PARAMETRIC_SUITE_TEST( suiteName, testName, testExpectedToFail, runFlag, ... )\
	TANTRUM_DECLARE_PARAMETRIC_SUITE_TEST_INTERNAL( #suiteName, testName, testExpectedToFail, runFlag, __VA_ARGS__ )
#else
#define TANTRUM_DECLARE_PARAMETRIC_SUITE_TEST( suiteName, testName, runFlag, ... )\
	TANTRUM_DECLARE_PARAMETRIC_SUITE_TEST_INTERNAL( #suiteName, testName, false, runFlag, __VA_ARGS__ )
#endif //TANTRUM_SELF_TEST_ENABLED

//----------------------------------------------------------

// TODO: make this macro end with a semicolon
#define TANTRUM_INVOKE_PARAMETRIC_TEST_INTERNAL( counter, nameOfTestToCall, parametricInvokationName, testExpectedToFail, ... ) \
\
	/*1. Create a function with a name matching the test.*/ \
	void ( parametricInvokationName )( void ); \
\
	/*2. Define this test body immediately*/ \
	void ( parametricInvokationName )( void ) { \
		TANTRUM_CONCAT_INTERNAL( nameOfTestToCall, _GlobalParametricInfo ).Callback( __VA_ARGS__ ); \
	} \
\
	/*3. Create a testName_TestInfo struct that will just wrap the test information meta data.*/ \
	typedef struct TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _TestInfo ) { \
		suiteTestInfo_t testInformation; \
	} TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _TestInfo ); \
\
	/*4. Create a global instance of this new struct for us to access and write data about this test to.*/ \
	extern TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _TestInfo ) TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ); \
	TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _TestInfo ) TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ); \
\
	/*5. Create our invoker_n function. This is what the runner will loop over to grab the test function as well as all the information concerning it*/ \
	/* HACK(DM): I shouldn't have to add extern "C" before each declaration here to make this work for c++ compiled binaries.  I already did that at the top of the header! How is that NOT a compiler bug!? */ \
	TANTRUM_EXTERN_C suiteTestInfo_t TANTRUM_API TANTRUM_CONCAT_INTERNAL( tantrum_test_info_fetcher_, counter )( void ); \
	suiteTestInfo_t TANTRUM_CONCAT_INTERNAL( tantrum_test_info_fetcher_, counter )( void ) { \
		TANTRUM_CONCAT_INTERNAL( nameOfTestToCall, _ParametricTestInfoBinder )();/*Make it so we can grab the needed information out of the test function's global info*/\
		TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ).testInformation.callback = parametricInvokationName; \
		TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ).testInformation.suiteNameStr = TANTRUM_CONCAT_INTERNAL( nameOfTestToCall, _GlobalParametricInfo ).suiteNameStr; \
		TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ).testInformation.isExpectedToFail = testExpectedToFail; \
		TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ).testInformation.testNameStr = #parametricInvokationName; \
		TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ).testInformation.testingFlag = TANTRUM_CONCAT_INTERNAL( nameOfTestToCall, _GlobalParametricInfo ).testingFlag; \
		return TANTRUM_CONCAT_INTERNAL( parametricInvokationName, _GlobalInfo ).testInformation; \
	} \
\
	/* leave this at the end so the macro can end with a semicolon */ \
	suiteTestInfo_t TANTRUM_API TANTRUM_CONCAT_INTERNAL( tantrum_test_info_fetcher_, counter )( void )

//----------------------------------------------------------

#ifdef TANTRUM_SELF_TEST_ENABLED
#define TANTRUM_INVOKE_PARAMETRIC_TEST( nameOfTestToCall, parametricInvokationName, testExpectedToFail, ... ) \
	TANTRUM_INVOKE_PARAMETRIC_TEST_INTERNAL( __COUNTER__, nameOfTestToCall, parametricInvokationName, testExpectedToFail, __VA_ARGS__ )
#else
#define TANTRUM_INVOKE_PARAMETRIC_TEST( nameOfTestToCall, parametricInvokationName, ... ) \
	TANTRUM_INVOKE_PARAMETRIC_TEST_INTERNAL( __COUNTER__, nameOfTestToCall, parametricInvokationName, false, __VA_ARGS__ )
#endif // TANTRUM_SELF_TEST_ENABLED

//==========================================================
// Public API - Condition Testing
//==========================================================

#define TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ) \
do { \
	if ( abortOnFail ) { \
		TANTRUM_LOG( "=== THIS TEST IS BEING ABORTED ===\n" ); \
		g_tantrumTestContext.testsAborted += 1; \
		return; \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_TRUE_INTERNAL( condition, abortOnFail, message, file, line ) \
do { \
	if ( !( condition ) ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG_ERROR( "TANTRUM_TEST_TRUE( %s ) has failed at %s line %d:\n%s\n", #condition, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_TRUE( condition, message ) \
	TANTRUM_TEST_TRUE_INTERNAL( condition, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_TRUE_OR_ABORT( condition, message ) \
	TANTRUM_TEST_TRUE_INTERNAL( condition, true, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_FALSE_INTERNAL( condition, abortOnFail, message, file, line ) \
do { \
	if ( ( condition ) ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG_ERROR( "TANTRUM_TEST_FALSE( %s ) has failed at %s line %d\n%s\n", #condition, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_FALSE( condition, message ) \
	TANTRUM_TEST_FALSE_INTERNAL( condition, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_FALSE_OR_ABORT( condition, message ) \
	TANTRUM_TEST_FALSE_INTERNAL( condition, true, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_EQUAL_INTERNAL( conditionA, conditionB, abortOnFail, message, file, line ) \
do { \
	if ( !TANTRUM_FLOAT_EQUALS_INTERNAL( conditionA, conditionB, TANTRUM_DEFAULT_EPSILON ) ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG_ERROR( "TANTRUM_TEST_EQUAL( %f, %f ) has failed at %s line %d\n%s\n", (double) conditionA, (double) conditionB, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_EQUAL( conditionA, conditionB, message ) \
	TANTRUM_TEST_EQUAL_INTERNAL( conditionA, conditionB, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_EQUAL_OR_ABORT( conditionA, conditionB, message ) \
	TANTRUM_TEST_EQUAL_INTERNAL( conditionA, conditionB, true, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_NOT_EQUAL_INTERNAL( conditionA, conditionB, abortOnFail, message, file, line ) \
do { \
	if ( !TANTRUM_FLOAT_EQUALS_INTERNAL( conditionA, conditionB, TANTRUM_DEFAULT_EPSILON ) ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG( "TANTRUM_TEST_NOT_EQUAL( %f, %f ) has failed at %s line %d\n%s\n", (double) conditionA, (double) conditionB, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_NOT_EQUAL( conditionA, conditionB, message ) \
	TANTRUM_TEST_NOT_EQUAL_INTERNAL( conditionA, conditionB, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_NOT_EQUAL_OR_ABORT( conditionA, conditionB, message ) \
	TANTRUM_TEST_NOT_EQUAL_INTERNAL( conditionA, conditionB, true, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_ALMOST_EQUAL_INTERNAL( conditionA, conditionB, tolerance, abortOnFail, message, file, line ) \
do { \
	if ( !TANTRUM_FLOAT_EQUALS_INTERNAL( conditionA, conditionB, tolerance ) ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG_ERROR( "TANTRUM_TEST_ALMOST_EQUAL( %f, %f, %f ) has failed at %s line %d\n%s\n", (double) conditionA, (double) conditionB, (double) tolerance, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_ALMOST_EQUAL( conditionA, conditionB, tolerance, message ) \
	TANTRUM_TEST_ALMOST_EQUAL_INTERNAL( conditionA, conditionB, tolerance, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_ALMOST_EQUAL_OR_ABORT( conditionA, conditionB, tolerance, message ) \
	TANTRUM_TEST_ALMOST_EQUAL_INTERNAL( conditionA, conditionB, tolerance, true, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_NOT_ALMOST_EQUAL_INTERNAL( conditionA, conditionB, tolerance, abortOnFail, message, file, line ) \
do { \
	if ( !TANTRUM_FLOAT_EQUALS( conditionA, conditionB, tolerance ) ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG( "TANTRUM_TEST_NOT_ALMOST_EQUAL( %f, %f, %f ) has failed at %s line %d\n%s\n", (double) conditionA, (double) conditionB, (double) tolerance, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_NOT_ALMOST_EQUAL( conditionA, conditionB, tolerance, message ) \
	TANTRUM_TEST_NOT_ALMOST_EQUAL_INTERNAL( conditionA, conditionB, tolerance, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_NOT_ALMOST_EQUAL_OR_ABORT( conditionA, conditionB, tolerance, message ) \
	TANTRUM_TEST_NOT_ALMOST_EQUAL_INTERNAL( conditionA, conditionB, tolerance, true, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_GREATER_THAN_INTERNAL( conditionA, conditionB, abortOnFail, message, file, line ) \
do { \
	if ( conditionA > conditionB ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG( "TANTRUM_TEST_GREATER_THAN( %f, %f ) has failed at %s line %d\n%s\n", (double) conditionA, (double) conditionB, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_GREATER_THAN( conditionA, conditionB, message ) \
	TANTRUM_TEST_GREATER_THAN_INTERNAL( conditionA, conditionB, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_GREATER_THAN_OR_ABORT( conditionA, conditionB, message ) \
	TANTRUM_TEST_GREATER_THAN_INTERNAL( conditionA, conditionB, true, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_LESS_THAN_INTERNAL( conditionA, conditionB, abortOnFail, message, file, line ) \
do { \
	if ( conditionA < conditionB ) { \
		g_tantrumTestContext.totalErrorsInCurrentTests += 1; \
\
		TANTRUM_LOG( "TANTRUM_TEST_LESS_THAN( %f, %f ) has failed at %s line %d\n%s\n", (double) conditionA, (double) conditionB, file, line, message ); \
\
		TANTRUM_ABORT_TEST_ON_FAIL( abortOnFail ); \
	} \
} while( 0 )

//----------------------------------------------------------

#define TANTRUM_TEST_LESS_THAN( conditionA, conditionB, message ) \
	TANTRUM_TEST_LESS_THAN_INTERNAL( conditionA, conditionB, false, message, __FILE__, __LINE__ )

//----------------------------------------------------------

#define TANTRUM_TEST_LESS_THAN_OR_ABORT( conditionA, conditionB, message ) \
	TANTRUM_TEST_LESS_THAN_INTERNAL( conditionA, conditionB, true, message, __FILE__, __LINE__ )

//==========================================================
// FUNCTIONS - USER MODDING WELCOME
//==========================================================

static void TantrumPrintDivider_UserModdable() {
	TANTRUM_LOG( "------------------------------------------------------------\n\n" );
}

//----------------------------------------------------------

static void TantrumPrintTestExecutionInformation_UserModdable() {
	TantrumPrintDivider_UserModdable();

	TANTRUM_LOG( "\n=== TANTRUM TESTING REPORT ===\n" );
	TANTRUM_LOG( "Total tests defined: %d\n", g_tantrumTestContext.totalTestsDeclared );

	if ( g_tantrumTestContext.isFilteringTests ) {
		TANTRUM_LOG( "\t- Total tests matching filters: %d\n\t- Suite filter: %s\n\t- Test filter: %s\n\t- Partial results %s\n",
				g_tantrumTestContext.totalTestsFoundWithFilters,
				g_tantrumTestContext.suiteFilter,
				g_tantrumTestContext.testFilter,
				g_tantrumTestContext.partialFilter ? "PERMITTED" : "DISCARDED" );
	}

	uint32_t totalFound = g_tantrumTestContext.totalTestsFoundWithFilters;
	TANTRUM_LOG(
		"Passed:   %d ( %d%% )\n"
		"Failed:   %d ( %d%% )\n"
		"Aborted:  %d ( %d%% )\n"
		"Skipped:  %d ( %d%% )\n",
		g_tantrumTestContext.testsPassed,  TantrumGetPercentInternal( g_tantrumTestContext.testsPassed, totalFound  ),
		g_tantrumTestContext.testsFailed,  TantrumGetPercentInternal( g_tantrumTestContext.testsFailed, totalFound  ),
		g_tantrumTestContext.testsAborted, TantrumGetPercentInternal( g_tantrumTestContext.testsAborted, totalFound ),
		g_tantrumTestContext.testsSkipped, TantrumGetPercentInternal( g_tantrumTestContext.testsSkipped, totalFound  )
	);
}

//----------------------------------------------------------

static void TantrumOnBeforeTest_UserModdable( const suiteTestInfo_t information ) {
	if ( !TANTRUM_STRING_EQUALS( g_tantrumTestContext.suiteFilterPrevious, information.suiteNameStr ) ) {
		TantrumPrintDivider_UserModdable();
		g_tantrumTestContext.suiteFilterPrevious = information.suiteNameStr;
	}

	if ( information.suiteNameStr ) {
		TANTRUM_LOG( "TEST \t- \"%s\" : \"%s\"\n", information.suiteNameStr, information.testNameStr );
	} else {
		TANTRUM_LOG( "TEST \t- \"%s\"\n", information.testNameStr );
	}
}

//----------------------------------------------------------

static const char* TantrumGetTimeUnitStringInternal( void ) {
	switch ( g_tantrumTestContext.timeUnit ) {
		case TANTRUM_TIME_UNIT_CLOCKS:	return "clocks";
		case TANTRUM_TIME_UNIT_NS:		return "ns";
		case TANTRUM_TIME_UNIT_US:		return "us";
		case TANTRUM_TIME_UNIT_MS:		return "ms";
		case TANTRUM_TIME_UNIT_SECONDS:	return "seconds";

		default:
			TANTRUM_ASSERT_INTERNAL( false && "Tantrum test context time unit was invalid somehow!?" );
			return NULL;
	}
}

//----------------------------------------------------------

static void TantrumOnAfterTest_UserModdable( const suiteTestInfo_t information ) {
	if ( information.testingFlag == TANTRUM_TEST_FLAG_SHOULD_RUN ) {
		const char* timeUnitStr = TantrumGetTimeUnitStringInternal();

		if ( g_tantrumTestContext.totalErrorsInCurrentTests > 0 ) {
			TantrumSetTextColorInternal( TANTRUM_COLOR_RED );
			TANTRUM_LOG( "TEST FAILED (%f %s)\n\n", information.testTimeTaken, timeUnitStr );
			TantrumSetTextColorInternal( TANTRUM_COLOR_DEFAULT );
		} else {
			TantrumSetTextColorInternal( TANTRUM_COLOR_GREEN );
			TANTRUM_LOG( "TEST SUCCEEDED (%f %s)\n\n", information.testTimeTaken, timeUnitStr );
			TantrumSetTextColorInternal( TANTRUM_COLOR_DEFAULT );
		}
	} else {
		const char* skipReason = information.testingFlag == TANTRUM_TEST_FLAG_DEPRECATED ? "DEPRECATED" : "SHOULD_SKIP";
		TantrumSetTextColorInternal( TANTRUM_COLOR_YELLOW );
		TANTRUM_LOG( "TEST FLAGGED \"%s\"\n\n", skipReason );
		TantrumSetTextColorInternal( TANTRUM_COLOR_DEFAULT );
	}
}

//==========================================================
// Internal Functions
//
// You as the user probably don't want to be directly touching these.
//==========================================================

static bool TantrumHandleCommandLineArgumentsInternal( int argc, char** argv ) {
#if defined( _WIN32 )
	char fullExePath[MAX_PATH];
	DWORD fullExePathLength = GetModuleFileName( NULL, fullExePath, MAX_PATH );
	if ( fullExePathLength == 0 ) {
		TANTRUM_LOG_ERROR( "WinAPI call GetModuleFileName() failed: 0x%lX\n", GetLastError() );
		return false;
	}

	g_tantrumTestContext.programName = fullExePath;
#elif defined( __APPLE__ ) || defined( __linux__ ) // _WIN32
	int err = 0;

	const char* exeFilenameVirtual = "/proc/self/exe";
	struct stat exeFileInfo = { 0 };
	if ( lstat( exeFilenameVirtual, &exeFileInfo ) == -1 ) {
		err = errno;
		TANTRUM_LOG_ERROR( "lstat() failed: %s", strerror( err ) );
		return false;
	}

	char fullExePath[PATH_MAX];
	ssize_t fullExePathLength = readlink( exeFilenameVirtual, fullExePath, (size_t) exeFileInfo.st_size + 1 );
	err = errno;
	if ( fullExePathLength == -1 ) {
		TANTRUM_LOG_ERROR( "readlink() failed: %s", strerror( err ) );
		return false;
	}

	fullExePath[exeFileInfo.st_size] = 0;

	g_tantrumTestContext.programName = fullExePath;
#else
#error Uncrecognised platform.  It appears Tantrum does not support it.  If you think this is a bug, please submit an issue at https://github.com/dangmoody/Tantrum/issues
#endif // _WIN32

	// parse command line args
	for ( int argIndex = 0; argIndex < argc; argIndex++ ) {
		const char* arg = argv[argIndex];

		if ( TANTRUM_STRING_EQUALS( arg, "-s" ) ) {
			const char* nextArg = TantrumGetNextArgInternal( argIndex, argc, argv );
			// TODO(DM): if nextArg == NULL then error that the suite filter wasnt set and show usage to help user

			g_tantrumTestContext.suiteFilter = nextArg;
			g_tantrumTestContext.isFilteringTests = true;

			continue;
		}

		if ( TANTRUM_STRING_EQUALS( arg, "-t" ) ) {
			const char* nextArg = TantrumGetNextArgInternal( argIndex, argc, argv );
			// TODO(DM): if nextArg == NULL then error that the test filter wasnt set and show usage to help user

			g_tantrumTestContext.testFilter = nextArg;
			g_tantrumTestContext.isFilteringTests = true;

			continue;
		}

		if ( TANTRUM_STRING_EQUALS( arg, "-p" ) ) {
			g_tantrumTestContext.partialFilter = true;
			continue;
		}

		if ( TANTRUM_STRING_EQUALS( arg, "--time-unit" ) ) {
			const char* nextArg = TantrumGetNextArgInternal( argIndex, argc, argv );
			// TODO(DM): if nextArg == NULL then error that the time unit wasnt set and show usage to help user

			if ( TANTRUM_STRING_EQUALS( nextArg, "seconds" ) ) {
				g_tantrumTestContext.timeUnit = TANTRUM_TIME_UNIT_SECONDS;
			} else if ( TANTRUM_STRING_EQUALS( nextArg, "ms" ) ) {
				g_tantrumTestContext.timeUnit = TANTRUM_TIME_UNIT_MS;
			} else if ( TANTRUM_STRING_EQUALS( nextArg, "us" ) ) {
				g_tantrumTestContext.timeUnit = TANTRUM_TIME_UNIT_US;
			} else if ( TANTRUM_STRING_EQUALS( nextArg, "ns" ) ) {
				g_tantrumTestContext.timeUnit = TANTRUM_TIME_UNIT_NS;
			} else if ( TANTRUM_STRING_EQUALS( nextArg, "clocks" ) ) {
				g_tantrumTestContext.timeUnit = TANTRUM_TIME_UNIT_CLOCKS;
			} else {
				TANTRUM_LOG_ERROR(
					"Invalid time unit \"%s\" specified.  Please select from one of the following:\n"
					"\t- seconds\n"
					"\t- ms\n"
					"\t- us\n"
					"\t- ns\n"
					"\t- clocks\n"
					"\n",
					nextArg
				);
				// TODO(DM): TantrumShowUsageInternal() again...
				return false;
			}

			continue;
		}
	}

	// if partial filtering was enabled but the user did not then specify a suite or test filter then they need to know about incorrect usage
	if ( g_tantrumTestContext.partialFilter ) {
		if ( !g_tantrumTestContext.suiteFilter && !g_tantrumTestContext.testFilter ) {
			TANTRUM_LOG_ERROR( "Partial filtering (-p) was enabled but suite or test filtering (-s, -t) was not.\n\n" );
			// TODO(DM): TantrumShowUsageInternal() again...
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------

static void* TantrumLoadEXEHandleInternal( void ) {
#if defined( _WIN32 )
	HMODULE handle = LoadLibrary( g_tantrumTestContext.programName );
	TANTRUM_ASSERT_INTERNAL( handle );
	return handle;
#elif defined( __APPLE__ ) || defined( __linux__ )	// defined( _WIN32 )
	void* handle = dlopen( NULL, RTLD_LAZY );
	TANTRUM_ASSERT_INTERNAL( handle );
	return handle;
#else	// defined( _WIN32 )
#error Uncrecognised platform.  It appears Tantrum does not support it.  If you think this is a bug, please submit an issue at https://github.com/dangmoody/Tantrum/issues
#endif	// defined( _WIN32 )
}

//----------------------------------------------------------

static void* TantrumGetProcAddressInternal( void* handle, const char* funcName ) {
	TANTRUM_ASSERT_INTERNAL( handle );
	TANTRUM_ASSERT_INTERNAL( funcName );

#ifdef _WIN32
	void* proc = (void*) GetProcAddress( (HMODULE) handle, funcName );
	TANTRUM_ASSERT_INTERNAL( proc );
	return proc;
#elif defined( __APPLE__ ) || defined( __linux__ )	// defined( _WIN32 )
	void* proc = dlsym( handle, funcName );
	TANTRUM_ASSERT_INTERNAL( proc );
	return proc;
#else	// defined( _WIN32 )
#error Uncrecognised platform.  It appears Tantrum does not support it.  If you think this is a bug, please submit an issue at https://github.com/dangmoody/Tantrum/issues
#endif	// defined( _WIN32 )
}

//----------------------------------------------------------

static void TantrumCloseEXEHandleInternal( void* handle ) {
	TANTRUM_ASSERT_INTERNAL( handle );

#if defined( _WIN32 )
	FreeLibrary( (HMODULE) handle );
	handle = NULL;
#elif defined( __APPLE__ ) || defined( __linux__ )	// _WIN32
	int closeError = dlclose( handle );
	if ( closeError ) {
		TANTRUM_LOG_ERROR( "%s.\n", dlerror() );
	}

	handle = NULL;
#else	// _WIN32
#error Uncrecognised platform.  It appears Tantrum does not support it.  If you think this is a bug, please submit an issue at https://github.com/dangmoody/Tantrum/issues
#endif	// _WIN32
}

//----------------------------------------------------------

static double TantrumGetTimestampInternal( void ) {
#if defined( _WIN32 )
	LARGE_INTEGER now;
	QueryPerformanceCounter( &now );

	switch ( g_tantrumTestContext.timeUnit ) {
		case TANTRUM_TIME_UNIT_CLOCKS:	return (double) ( now.QuadPart );
		case TANTRUM_TIME_UNIT_NS:		return (double) ( ( now.QuadPart * 1000000000 ) / g_tantrumTestContext.timestampFrequency.QuadPart );
		case TANTRUM_TIME_UNIT_US:		return (double) ( ( now.QuadPart * 1000000 ) / g_tantrumTestContext.timestampFrequency.QuadPart );
		case TANTRUM_TIME_UNIT_MS:		return (double) ( ( now.QuadPart * 1000 ) / g_tantrumTestContext.timestampFrequency.QuadPart );
		case TANTRUM_TIME_UNIT_SECONDS:	return (double) ( ( now.QuadPart ) / g_tantrumTestContext.timestampFrequency.QuadPart );
	}
#elif defined( __APPLE__ ) || defined( __linux__ )	// defined( _WIN32 )
	struct timespec now;
	clock_gettime( CLOCK_MONOTONIC, &now );

	int64_t clocks = (int64_t) ( now.tv_sec * 1000000000 + now.tv_nsec );

	switch ( g_tantrumTestContext.timeUnit ) {
		case TANTRUM_TIME_UNIT_CLOCKS:	return (double) clocks;
		case TANTRUM_TIME_UNIT_NS:		return (double) clocks;
		case TANTRUM_TIME_UNIT_US:		return (double) clocks / 1000.0;
		case TANTRUM_TIME_UNIT_MS:		return (double) clocks / 1000000.0;
		case TANTRUM_TIME_UNIT_SECONDS:	return (double) clocks / 1000000000.0;
	}
#else	// defined( _WIN32 )
#error Uncrecognised platform.  It appears Tantrum does not support it.  If you think this is a bug, please submit an issue at https://github.com/dangmoody/Tantrum/issues
#endif	// defined( _WIN32 )

	// should never get here
	TANTRUM_ASSERT_INTERNAL( false && "Unrecognised time unit passed into TemperGetTimestampInternal().\n" );

	return 0.0;
}

//----------------------------------------------------------

static int TantrumExecuteAllTestsInternal() {
	// make the exe load itself
	void* handle = TantrumLoadEXEHandleInternal();

	// DM: I have never seen a function name exceed 64 characters, let alone 1024
	// so this shouldn't be a problem
	// I wonder if it's possible we could perhaps make this string length constant an overridable #define ?
	char testFuncName[1024];
	testInfoFetcherFunc_t testInfoGrabberFunc = NULL;

	for ( uint32_t i = 0; i < g_tantrumTestContext.totalTestsDeclared; i++ ) {
		TANTRUM_SNPRINTF( testFuncName, 1024, "tantrum_test_info_fetcher_%d", i );

		//TANTRUM_LOG( "Loading test func: %s\n", testFuncName );

		// get the test grabber functions out of the binary
		testInfoGrabberFunc = (testInfoFetcherFunc_t) TantrumGetProcAddressInternal( handle, testFuncName );

		if ( !testInfoGrabberFunc ) {
			const char* platformErrorMsg = "";
#if defined( __APPLE__ ) || defined( __linux__ )
			platformErrorMsg = "\nOn MacOS/Linux you need to explicitly allow dynamic symbol exporting (E.G.: on Clang use: \"-Wl,--export-dynamic\").\n";
#endif

			TANTRUM_LOG_ERROR( "Failed to find function %s.%s\n", testFuncName, platformErrorMsg );
			return TANTRUM_EXIT_FAILURE;
		}

		suiteTestInfo_t information = testInfoGrabberFunc();

		// if partial suite/test filtering is enabled then we want to check if the queried filter is in a part of the name they specified
		// otherwise partial filtering is off so we want to check for an exact string match
		tantrumStringCompareFunc_t stringCompareFunc = g_tantrumTestContext.partialFilter ? TANTRUM_STRING_CONTAINS : TANTRUM_STRING_EQUALS;

		bool isFilteredSuite = g_tantrumTestContext.suiteFilter && information.suiteNameStr && stringCompareFunc( information.suiteNameStr, g_tantrumTestContext.suiteFilter );

		if ( isFilteredSuite || !g_tantrumTestContext.suiteFilter ) {
			bool isFilteredTest = g_tantrumTestContext.testFilter && stringCompareFunc( information.testNameStr, g_tantrumTestContext.testFilter );

			if ( isFilteredTest || !g_tantrumTestContext.testFilter ) {
				g_tantrumTestContext.totalTestsFoundWithFilters += 1;

				TantrumOnBeforeTest_UserModdable( information );

				// MY : I'm not checking the flag first as it'd still be helpful for search queries to see if the test even appears
				if ( information.testingFlag == TANTRUM_TEST_FLAG_SHOULD_RUN ) {
					g_tantrumTestContext.totalErrorsInCurrentTests = 0;

					double start = TantrumGetTimestampInternal();
					information.callback();
					double end = TantrumGetTimestampInternal();

					information.testTimeTaken = end - start;

					g_tantrumTestContext.totalTestsExecuted += 1;

					if ( g_tantrumTestContext.totalErrorsInCurrentTests > 0 && !information.isExpectedToFail ) {
						g_tantrumTestContext.testsFailed += 1;
					} else {
						g_tantrumTestContext.testsPassed += 1;
					}
				} else {
					g_tantrumTestContext.testsSkipped += 1;
				}

				TantrumOnAfterTest_UserModdable( information );
			}
		}
	}

	TantrumPrintTestExecutionInformation_UserModdable();

	// cleanup
	TantrumCloseEXEHandleInternal( handle );

	return g_tantrumTestContext.testsFailed == 0 ? TANTRUM_EXIT_SUCCESS : TANTRUM_EXIT_FAILURE;
}

//----------------------------------------------------------

static int TantrumExecuteAllTestsWithArgumentsInternal( int argc, char** argv ) {
	if ( !TantrumHandleCommandLineArgumentsInternal( argc, argv ) ) {
		return TANTRUM_EXIT_FAILURE;
	}

	return TantrumExecuteAllTestsInternal();
}

//----------------------------------------------------------

#if defined( __linux__ ) || defined( __APPLE__ )
#pragma pop_macro( "_POSIX_C_SOURCE" )
#endif

#if defined( __clang__ )
#pragma clang diagnostic pop
#elif defined( __GNUC__ )
#pragma GCC diagnostic pop
#endif // defined( __GNUC__ ) || defined( __clang__ )

#ifdef __cplusplus
}
#endif
