# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/shankar/CSCI6525/project/Moby

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/shankar/CSCI6525/project/Moby/build

# Include any dependencies generated for this target.
include CMakeFiles/moby-compare-trajs.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/moby-compare-trajs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/moby-compare-trajs.dir/flags.make

CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o: CMakeFiles/moby-compare-trajs.dir/flags.make
CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o: ../example/compare-trajs.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/shankar/CSCI6525/project/Moby/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o -c /home/shankar/CSCI6525/project/Moby/example/compare-trajs.cpp

CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/shankar/CSCI6525/project/Moby/example/compare-trajs.cpp > CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.i

CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/shankar/CSCI6525/project/Moby/example/compare-trajs.cpp -o CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.s

CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.requires:
.PHONY : CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.requires

CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.provides: CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.requires
	$(MAKE) -f CMakeFiles/moby-compare-trajs.dir/build.make CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.provides.build
.PHONY : CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.provides

CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.provides.build: CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o

# Object files for target moby-compare-trajs
moby__compare__trajs_OBJECTS = \
"CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o"

# External object files for target moby-compare-trajs
moby__compare__trajs_EXTERNAL_OBJECTS =

moby-compare-trajs: CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o
moby-compare-trajs: CMakeFiles/moby-compare-trajs.dir/build.make
moby-compare-trajs: libMoby.so
moby-compare-trajs: /usr/lib/x86_64-linux-gnu/libqhull.so
moby-compare-trajs: /usr/lib/x86_64-linux-gnu/libxml2.so
moby-compare-trajs: /usr/lib/libosg.so
moby-compare-trajs: /usr/lib/libosgViewer.so
moby-compare-trajs: /usr/lib/libosgDB.so
moby-compare-trajs: /usr/lib/libosgGA.so
moby-compare-trajs: /usr/lib/libOpenThreads.so
moby-compare-trajs: /usr/lib/libosgUtil.so
moby-compare-trajs: CMakeFiles/moby-compare-trajs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable moby-compare-trajs"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/moby-compare-trajs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/moby-compare-trajs.dir/build: moby-compare-trajs
.PHONY : CMakeFiles/moby-compare-trajs.dir/build

CMakeFiles/moby-compare-trajs.dir/requires: CMakeFiles/moby-compare-trajs.dir/example/compare-trajs.cpp.o.requires
.PHONY : CMakeFiles/moby-compare-trajs.dir/requires

CMakeFiles/moby-compare-trajs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/moby-compare-trajs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/moby-compare-trajs.dir/clean

CMakeFiles/moby-compare-trajs.dir/depend:
	cd /home/shankar/CSCI6525/project/Moby/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/shankar/CSCI6525/project/Moby /home/shankar/CSCI6525/project/Moby /home/shankar/CSCI6525/project/Moby/build /home/shankar/CSCI6525/project/Moby/build /home/shankar/CSCI6525/project/Moby/build/CMakeFiles/moby-compare-trajs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/moby-compare-trajs.dir/depend

