# Contributing

## House Rules
1. We track bugs etc. via the GitHub issue tracker, so use that.
2. Always assign a PR reviewer and wait for them to green-light your PR before merging.


## Building The Project
If on Windows and using Clang or GCC then you will need to make sure that the compiler is added to your `PATH`.

1. Clone the git repo.
2. If on Windows:
	* If you want to use Visual Studio:
		* Make sure you have Visual Studio 2019 installed.
		* Run `.\scripts\generate_vs2019.bat`.
	* If you do NOT want to use Visual Studio: 
		* Run `.\scripts\build_clang_gcc.bat` to build a demo for Clang/GCC.
		* Run `.\scripts\build_msvc.bat` to build a demo for MSVC.
3. If on MacOS/Linux:
	* Run `.\scripts\build_clang_gcc.sh` to build a demo for Clang/GCC.


## Pull Requests
Your PR has a higher chance of being accepted if it fulfills at least one of the following criteria:
* It fixes a bug listed in the [issue tracker](https://github.com/dangmoody/Tantrum/issues).
* You have fixed an un-named `TODO` for us in the code somewhere.
* It does something completely awesome that will definitely help Temper in some way.

If your change doesn't do any of these things then it doesn't mean it won't get accepted, it just means your PR stands less of a chance of being accepted.

We don't bite and we're open to ideas.

## Recommended Reading
For Temper we like to follow the [Handmade Network's excellent written piece](https://handmade.network/wiki/7138-how_to_write_better_game_libraries) on how to write better game libraries for guiding principles.  We don't follow it super strictly, but we do refer to it often.