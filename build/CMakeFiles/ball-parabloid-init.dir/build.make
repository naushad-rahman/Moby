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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/shankar/CSCI6525/project/Moby

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/shankar/CSCI6525/project/Moby/build

# Include any dependencies generated for this target.
include CMakeFiles/ball-parabloid-init.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ball-parabloid-init.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ball-parabloid-init.dir/flags.make

CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o: CMakeFiles/ball-parabloid-init.dir/flags.make
CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o: ../example/ball-in-parabloid/init.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/shankar/CSCI6525/project/Moby/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o -c /home/shankar/CSCI6525/project/Moby/example/ball-in-parabloid/init.cpp

CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/shankar/CSCI6525/project/Moby/example/ball-in-parabloid/init.cpp > CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.i

CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/shankar/CSCI6525/project/Moby/example/ball-in-parabloid/init.cpp -o CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.s

CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.requires:
.PHONY : CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.requires

CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.provides: CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.requires
	$(MAKE) -f CMakeFiles/ball-parabloid-init.dir/build.make CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.provides.build
.PHONY : CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.provides

CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.provides.build: CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o

# Object files for target ball-parabloid-init
ball__parabloid__init_OBJECTS = \
"CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o"

# External object files for target ball-parabloid-init
ball__parabloid__init_EXTERNAL_OBJECTS =

libball-parabloid-init.so: CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o
libball-parabloid-init.so: CMakeFiles/ball-parabloid-init.dir/build.make
libball-parabloid-init.so: libMoby.so
libball-parabloid-init.so: /usr/lib/x86_64-linux-gnu/libqhull.so
libball-parabloid-init.so: /usr/lib/x86_64-linux-gnu/libxml2.so
libball-parabloid-init.so: /usr/lib/libosg.so
libball-parabloid-init.so: /usr/lib/libosgViewer.so
libball-parabloid-init.so: /usr/lib/libosgDB.so
libball-parabloid-init.so: /usr/lib/libosgGA.so
libball-parabloid-init.so: /usr/lib/libOpenThreads.so
libball-parabloid-init.so: /usr/lib/libosgUtil.so
libball-parabloid-init.so: CMakeFiles/ball-parabloid-init.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX shared module libball-parabloid-init.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ball-parabloid-init.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ball-parabloid-init.dir/build: libball-parabloid-init.so
.PHONY : CMakeFiles/ball-parabloid-init.dir/build

CMakeFiles/ball-parabloid-init.dir/requires: CMakeFiles/ball-parabloid-init.dir/example/ball-in-parabloid/init.cpp.o.requires
.PHONY : CMakeFiles/ball-parabloid-init.dir/requires

CMakeFiles/ball-parabloid-init.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ball-parabloid-init.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ball-parabloid-init.dir/clean

CMakeFiles/ball-parabloid-init.dir/depend:
	cd /home/shankar/CSCI6525/project/Moby/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/shankar/CSCI6525/project/Moby /home/shankar/CSCI6525/project/Moby /home/shankar/CSCI6525/project/Moby/build /home/shankar/CSCI6525/project/Moby/build /home/shankar/CSCI6525/project/Moby/build/CMakeFiles/ball-parabloid-init.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ball-parabloid-init.dir/depend
