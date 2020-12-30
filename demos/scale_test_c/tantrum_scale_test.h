#ifndef TANTRUM_SCALE_TEST_HEADER
#define TANTRUM_SCALE_TEST_HEADER

#include <Windows.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#if defined( __GNUC__ ) || defined( __clang__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wcomment"
#endif

//==========================================================
// STRUCTS
//==========================================================

typedef enum tantrumTestFlag_t
{
	TANTRUM_TEST_SHOULD_RUN = 0,
	TANTRUM_TEST_SHOULD_SKIP,
	TANTRUM_TEST_DEPRECATED
} tantrumTestFlag_t;

//----------------------------------------------------------

typedef void( *testCallback_t )( void );

typedef struct suiteTestInfo_t
{
	testCallback_t		callback;
	tantrumTestFlag_t	testingFlag;
	const char* testNameStr;
	const char* suiteNameStr;
} suiteTestInfo_t;

typedef suiteTestInfo_t( *testInvoker_t )( void );

//----------------------------------------------------------

typedef struct tantrumTestContext_t
{
	unsigned int testsPassed;
	unsigned int testsFailed;
	unsigned int totalTestsDeclared; // Gets set in the main function with a preprocessor
	unsigned int totalErrorsInCurrentTests;
	const char** inputArgv;
	const char* suiteFilter;
	const char* testFilter;
} tantrumTestContext_t;

//==========================================================
// GLOBALS
//==========================================================

static tantrumTestContext_t tantrumGlobalTestContext;

//==========================================================
// PREPROCESSORS - TANTRUM INTERNAL
//==========================================================

#define TANTRUM_CONCAT_INTERNAL_FINAL( a, b )	a ## b

//----------------------------------------------------------

#define TANTRUM_CONCAT_INTERNAL( a, b )		TANTRUM_CONCAT_INTERNAL_FINAL( a, b )

//----------------------------------------------------------

#define TANTRUM_DEFINE_TEST_INTERNAL( suiteNameString, testName, runFlag ) \
\
	/*1. Create a function with a name matching the test.*/\
	void ( testName )( void ); \
\
	/*2. Create a testName_TestInfo struct that will just wrap the test information meta data.*/\
	typedef struct TANTRUM_CONCAT_INTERNAL( testName, _TestInfo ) { \
		suiteTestInfo_t testInformation; \
	} TANTRUM_CONCAT_INTERNAL( testName, _TestInfo ); \
\
	/*3. Create a static/global instance of this new struct for us to access and write data about this test to.*/\
	static TANTRUM_CONCAT_INTERNAL( testName, _TestInfo ) TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ); \
\
	/*4. Create our invoker_n function. This is what the runner will loop over to grab the test function as well as all the information concerning it*/\
	__declspec( dllexport ) suiteTestInfo_t TANTRUM_CONCAT_INTERNAL( tantrum_test_func_info_fetcher_, __COUNTER__ )( void ) { \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.callback = testName; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.suiteNameStr = suiteNameString; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.testNameStr = #testName; \
		TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation.testingFlag = runFlag; \
		return TANTRUM_CONCAT_INTERNAL( testName, _GlobalInfo ).testInformation; \
	} \
\
	/*5. The test function declared at Step1 is now declared here by the user*/\
	void ( testName )( void )

//----------------------------------------------------------

#define TANTRUM_TEST( testName, runFlag ) \
	TANTRUM_DEFINE_TEST_INTERNAL( NULL, testName, runFlag )

//----------------------------------------------------------

#define TANTRUM_SUITE_TEST( suiteName, testName, runFlag ) \
	TANTRUM_DEFINE_TEST_INTERNAL( #suiteName, testName, runFlag )

//----------------------------------------------------------

#define TANTRUM_SETUP()\
	tantrumGlobalTestContext.totalTestsDeclared = __COUNTER__

//==========================================================
// PREPROCESSORS - CONDITION TESTING
//==========================================================

#define TEST_TRUE( condition, message )\
{\
	if( condition )\
	{\
		printf( "TEST_TRUE(%s) has failed\n", #condition );\
		printf( "%s\n", #message );\
	}\
}

//----------------------------------------------------------

#define TEST_EQUAL( conditionA, conditionB, message )\
{\
	if( conditionA != conditionB )\
	{\
		printf( "TEST_EQUAL( %f, %f ) has failed\n", ( double )conditionA, ( double )conditionB );\
		printf( "%s\n", #message );\
	}\
}

//----------------------------------------------------------

#define TEST_NOT_EQUAL(conditionA, conditionB, message)\
{\
	if(conditionA == conditionB)\
	{\
		printf( "TEST_NOT_EQUAL( %f,%f ) has failed\n", ( double )conditionA, ( double )conditionB );\
		printf( "%s\n", #message );\
	}\
}

//----------------------------------------------------------

#define TEST_ALMOST_EQUAL( conditionA, conditionB, tolerance, message )\
{\
	double epsilon = Absolute( ( ( double )conditionA ) - ( ( double )conditionB ) );\
	double absoluteTolerance = Absolute( tolerance );\
	if( epsilon > absoluteTolerance )\
	{\
		printf( "TEST_ALMOST_EQUAL( %f, %f, %f ) has failed\n", conditionA, conditionB, tolerance );\
		printf( "%s\n", #message ); \
	}\
}

//----------------------------------------------------------

#define TEST_NOT_ALMOST_EQUAL( conditionA, conditionB, tolerance, message )\
{\
	double epsilon = Absolute( ( ( double )conditionA ) - ( ( double )conditionB ) );\
	double absoluteTolerance = Absolute( tolerance );\
	if( epsilon <= absoluteTolerance )\
	{\
		printf( "ASSER_NOT_ALMOST_EQUAL( %f, %f, %f ) has failed\n", conditionA, conditionB, tolerance );\
		printf( "%s\n", #message ); \
	}\
}

//----------------------------------------------------------

#define TEST_GREATER_THAN(conditionA, conditionB, message)\
{\
	if(conditionA > conditionB)\
	{\
		printf( "TEST_GREATER_THAN(%f,%f) has failed\n", ( double )conditionA, ( double )conditionB );\
		printf( "%s\n", #message );\
	}\
}

//----------------------------------------------------------

#define TEST_LESS_THAN(conditionA, conditionB, message)\
{\
	if( conditionA < conditionB )\
	{\
		printf( "TEST_LESS_THAN(%f,%f) has failed\n", ( double )conditionA, ( double )conditionB );\
		printf( "%s\n", #message );\
	}\
}

//==========================================================
// FUNCTIONS
//==========================================================

static const char* TantrumGetNextArgInternal( const int argIndex, const int argc, char** argv )
{
	assert( argc );
	assert( argv );

	return ( argIndex + 1 < argc ) ? argv[ argIndex + 1 ] : NULL;
}

//----------------------------------------------------------

static void TantrumHandleCommandLineArguments( int argc, char** argv )
{
	// parse command line args
	for( int argIndex = 0; argIndex < argc; argIndex++ )
	{
		const char* arg = argv[ argIndex ];

		if( strcmp( arg, "-s" ) == 0 )
		{
			const char* nextArg = TantrumGetNextArgInternal( argIndex, argc, argv );
			// TODO(DM): if nextArg == NULL then error that the suite filter wasnt set and show usage to help user
			tantrumGlobalTestContext.suiteFilter = nextArg;

			continue;
		}

		if( strcmp( arg, "-t" ) == 0 )
		{
			const char* nextArg = TantrumGetNextArgInternal( argIndex, argc, argv );
			// TODO(DM): if nextArg == NULL then error that the test filter wasnt set and show usage to help user
			tantrumGlobalTestContext.testFilter = nextArg;

			continue;
		}
	}
}

//----------------------------------------------------------

static int TantrumExecuteAllTests()
{
	// MY: Honestly I'd like to be able to use this without needing
	// argc/argv, ideally anyone should be able to use this weather
	// their using main(), main(argc, argv) or WinMain(...).
	// temporarily using global data to make this function work.
	HANDLE handle = LoadLibrary( tantrumGlobalTestContext.inputArgv[ 0 ] );
	assert( handle );

	// DM: yeah yeah yeah, I know: fixed-length string arrays bad
	// I'll write a tprintf at some point
	char testFuncNames[ 1024 ];
	testInvoker_t testInfoGrabberFunc = NULL;

	for( uint32_t i = 0; i < __COUNTER__; i++ )
	{
		snprintf( testFuncNames, 1024, "tantrum_test_func_info_fetcher_%d", i );
		testInfoGrabberFunc = ( testInvoker_t ) GetProcAddress( handle, testFuncNames );
		assert( testInfoGrabberFunc );

		suiteTestInfo_t information = testInfoGrabberFunc();

		bool isFilteredSuite = tantrumGlobalTestContext.suiteFilter && information.suiteNameStr && strcmp( information.suiteNameStr, tantrumGlobalTestContext.suiteFilter ) == 0;

		if( isFilteredSuite || !tantrumGlobalTestContext.suiteFilter )
		{
			bool isFilteredTest = tantrumGlobalTestContext.testFilter && strcmp( information.testNameStr, tantrumGlobalTestContext.testFilter ) == 0;

			if( isFilteredTest || !tantrumGlobalTestContext.testFilter )
			{
				// MY : I'm not checking the flag first as it'd still be helpful for search queries to see if the test even appears
				if( information.testingFlag == TANTRUM_TEST_SHOULD_RUN )
				{
					// DM: given this is now where information.testResults gets set, we can maybe remove that member?
					information.callback();

					if( information.suiteNameStr )
					{
						printf( "%s: ", information.suiteNameStr );
					}

					if( tantrumGlobalTestContext.totalErrorsInCurrentTests > 0 )
					{
						printf( "%s - FAILED\n", information.testNameStr );
					}
					else
					{
						printf( "%s - SUCCEEDED\n", information.testNameStr );
						break;
					}
				}
				else
				{
					const char* dodgeReason = information.testingFlag == TANTRUM_TEST_DEPRECATED ? "DEPRICATED" : "SHOULD_SKIP";
					printf( "NOT RUNNING \"%s\" AS IT WAS FLAGGED With \"%s\"\n", information.testNameStr, dodgeReason );
				}
			}
		}
	}

	FreeLibrary( handle );
	handle = NULL;
	return tantrumGlobalTestContext.testsFailed == 0 ? 0 : -1;
}

//----------------------------------------------------------

static int TantrumExecuteAllTestsWithArguments( int argc, char** argv )
{
	TantrumHandleCommandLineArguments( argc, argv );
	return TantrumExecuteAllTests();
}

//----------------------------------------------------------

#if defined( __GNUC__ ) || defined( __clang__ )
#pragma GCC diagnostic pop
#endif
#endif //TANTRUM_SCALE_TEST_HEADER