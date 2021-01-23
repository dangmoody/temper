# Temper

![CI](https://github.com/dangmoody/Tantrum/workflows/CI/badge.svg)

The new and improved C99, single-header-only, unit testing framework.

Distributed under the MIT license.  See [LICENSE](https://github.com/dangmoody/Tantrum/blob/master/LICENSE) file for details.

## Features:

- Automatic test registration at compile time, simply write your test and it will get called for you.
- Parametric tests.
- Early exits for tests and a bunch of new condition macros.
- Handling errors from your program's log output (requires a bit of extra work on your part). `// DM: I'm thinking about this`
- Low friction, easily overridable functions to help hook Temper into your codebase.
- Support for Clang, GCC, and MSVC across Windows, Mac OS, and Linux on x64 ISAs (support for ARM in progress).

`// DM: rework this maybe?`
`// DM: I like the marketing - be more human`
It's not a new feature but worth stressing. It's still just the one header file.  Drop it into your project, tell it to run all tests somewhere in code and you're good to go! And once again; it's all written in C99 compliant code.

## Installation

Download the latest release from the [releases](https://github.com/dangmoody/Tantrum/releases/latest) tab and include `temper.h`.

## Quick start guide:

```c
#include <temper.h>

// write some tests

int main( int argc, char** argv )
{
	TEMPER_SETUP(); // You MUST call this first for Temper to set itself up correctly.
	return TEMPER_RUN_ALL_TESTS_WITH_ARGS( argc, argv ); // This actually executes the tests.
}
```

#### Compiling

On Windows and Mac OS you shouldn't need to do anything extra on your part to get Temper to compile correctly.  There should be no other dependencies that are required.  If you find that there are, please submit a [bug report](https://github.com/dangmoody/Tantrum/issues).

If you are compiling Temper on Linux and you are NOT overriding the default internal functions then you will need to make sure you pass the following arguments to your compiler/linker:
- `-lm` - required if you're not overriding `TEMPER_FABSF`.
- `-ldl` - required if you're not overriding the `LoadEXEHandle()` and `UnloadEXEHandle()` functions.
- `-lpthread` - required if you're not overriding the `RunTestThread()` function.
- `--export-dynamic` - or some other equivalent, required to allow the compiler to export the test functions so they can be called dynamically by Temper at runtime.

When compiled, this will then produce an executable that will run all tests you have defined and return `EXIT_SUCCESS` (by default) if there were no errors.  If there were errors then the program will return `EXIT_FAILURE` (by default).

#### Writing tests

A test can be defined either as a part of or not a part of a suite and with or without parameters.

```c
// Define a simple test with a Name only
TEMPER_TEST( TestName, TEMPER_TEST_FLAG_SHOULD_RUN )
{
	TEMPER_TEST_TRUE( 69 == 105 );
}

// Define a test as a part of a suite
TEMPER_SUITE_TEST( SuiteName, TestName2, TEMPER_TEST_FLAG_SHOULD_RUN )
{
	TEMPER_TEST_TRUE( 69 == 420 );
}

// Define a parametric test (can take whatever parameters you like)
TEMPER_DECLARE_PARAMETRIC_TEST( TestName3, TEMPER_TEST_FLAG_SHOULD_RUN, int param1, int param2 )
{
	TEMPER_TEST_EQUAL( param1, param2 );
}

// Declare invokations to the parametric test. Must have matching arguments filled out.
TEMPER_INVOKE_PARAMETRIC_TEST( TestName3, Check5And5, 5, 5 ); // will work
TEMPER_INVOKE_PARAMETRIC_TEST( TestName3, Check5And6, 5, 6 ); // will fail
TEMPER_INVOKE_PARAMETRIC_TEST( TestName3, Check7And7, 7, 7 ); // will work

// Define a parametric test as a part of a suite
TEMPER_DECLARE_PARAMETRIC_SUITE_TEST( SuiteName, TestName4, TEMPER_TEST_FLAG_SHOULD_RUN, int param1, int param2 )
{
	TEMPER_TEST_EQUAL( param1, param2 );
}

// You invoke a suite parametric test in the same way you would a non-suite parametric test.
TEMPER_INVOKE_PARAMETRIC_TEST( TestName4, Check5And5, 5, 5 ); // will work
TEMPER_INVOKE_PARAMETRIC_TEST( TestName4, Check5And6, 5, 6 ); // will fail
TEMPER_INVOKE_PARAMETRIC_TEST( TestName4, Check7And7, 7, 7 ); // will work
```

#### Test cases / Conditional macros

|                        Name                        |                                               Description                                                |
| -------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| `TEST_TRUE( 5 == 5 )`                              | Passes if the condition is true, otherwise it'll log an error                                            |
| `TEMPER_TEST_FALSE( 5 == 6 )`                      | Passes if the condition is false, otheriwse it'll log an error                                           |
| `TEMPER_TEST_EQUAL( 5, 5 )`                        | Passes if the two parameters are the same, otheriwse it'll log an error                                  |
| `TEMPER_TEST_NOT_EQUAL( 5, 6 )`                    | Passes if two parameters are NOT the same, otheriwse it'll log an error                                  |
| `TEMPER_TEST_FLOAT_EQUAL( 5.5f, 5.5f )`            | Passes if the two parameters are the same or within a predefined tolerance, otheriwse it'll log an error |
| `TEMPER_TEST_ALMOST_EQUAL( 5.5f, 6.7f, 2.0f )`     | Passes if the two parameters are the same or within a specified tolerance, otheriwse it'll log an error  |
| `TEMPER_TEST_NOT_ALMOST_EQUAL( 5.5f, 6.7f, 1.0f )` | Passes if the two parameters are different beyond a specified tolerance, otheriwse it'll log an error    |

It's worth pointing out that all of these conditions have optional suffixes which you can apply for more options:

#### Suffixes for test macros

|    Suffix     |                                                        Description                                                         |
| ------------- | -------------------------------------------------------------------------------------------------------------------------- |
| `_M`          | Allows you to append a string parameter that will be output if the condition fails.                                        |
| `_OR_ABORT`   | If this condition fails the whole test will exit early                                                                     |
| `_OR_ABORT_M` | Allows you to append a string parameter that will be output if the condition fails, it will also have the test exit early. |

#### Examples of Suffixes:

```c
TEMPER_TEST_TRUE( 5 == 6 ); // Will just log a standard error when it fails
TEMPER_TEST_TRUE_M( 5 == 6, "Maths is broken." ); // Will print your message as well when it fails.
TEMPER_TEST_TRUE_OR_ABORT( 5 == 6 ); // Will just log a standard error and make the test exit early when it fails.
TEMPER_TEST_TRUE_OR_ABORT_M( 5 == 6, "Maths is broken." ); // Will make your test exit early and print your messsage.
```

## Overriding internal functions

The `g_temperTestContext` contains a member called `callbacks` which, in turn, contains a set of functions that you can use to override Temper's default functionality.  This can be used to help hook Temper into your codebase.

If you are running on a platform that **ISN'T** Windows, Mac OS, or Linux then you will definitely want to override these functions as some of Temper's internal functions make calls to their respective OS-level API.

## Command line arguments

```
[-h|--help]
	Shows this help and then exits.

-t <test>
	Only run the test with the given name.

-s <suite>
	Only run tests in the suite with the given name.

-p
	Enable partial filtering, which will only run tests and suites that contain the text specified in the filters.

--time-unit [seconds|ms|us|ns|clocks]
	Set the units to measure test times in.
	The default is microseconds.
```

## Contributing

Yes!

If you want to submit an idea/bug then use the [GitHub issue tracker](https://github.com/dangmoody/Tantrum/issues).

If you want to submit code then we are open to pull requests.  See [contributing.md](https://github.com/dangmoody/Tantrum/blob/master/doc/how_to_contribute.md) for details.

## Special thanks to:

* Zack Dutton - Bug reports and testing.