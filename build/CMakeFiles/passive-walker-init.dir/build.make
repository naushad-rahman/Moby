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
include CMakeFiles/passive-walker-init.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/passive-walker-init.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/passive-walker-init.dir/flags.make

CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o: CMakeFiles/passive-walker-init.dir/flags.make
CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o: ../example/passive-walker/init.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/shankar/CSCI6525/project/Moby/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o -c /home/shankar/CSCI6525/project/Moby/example/passive-walker/init.cpp

CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/shankar/CSCI6525/project/Moby/example/passive-walker/init.cpp > CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.i

CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/shankar/CSCI6525/project/Moby/example/passive-walker/init.cpp -o CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.s

CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.requires:
.PHONY : CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.requires

CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.provides: CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.requires
	$(MAKE) -f CMakeFiles/passive-walker-init.dir/build.make CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.provides.build
.PHONY : CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.provides

CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.provides.build: CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o

# Object files for target passive-walker-init
passive__walker__init_OBJECTS = \
"CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o"

# External object files for target passive-walker-init
passive__walker__init_EXTERNAL_OBJECTS =

libpassive-walker-init.so: CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o
libpassive-walker-init.so: CMakeFiles/passive-walker-init.dir/build.make
libpassive-walker-init.so: libMoby.so
libpassive-walker-init.so: /usr/lib/x86_64-linux-gnu/libqhull.so
libpassive-walker-init.so: /usr/lib/x86_64-linux-gnu/libxml2.so
libpassive-walker-init.so: /usr/lib/libosg.so
libpassive-walker-init.so: /usr/lib/libosgViewer.so
libpassive-walker-init.so: /usr/lib/libosgDB.so
libpassive-walker-init.so: /usr/lib/libosgGA.so
libpassive-walker-init.so: /usr/lib/libOpenThreads.so
libpassive-walker-init.so: /usr/lib/libosgUtil.so
libpassive-walker-init.so: CMakeFiles/passive-walker-init.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX shared module libpassive-walker-init.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/passive-walker-init.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/passive-walker-init.dir/build: libpassive-walker-init.so
.PHONY : CMakeFiles/passive-walker-init.dir/build

CMakeFiles/passive-walker-init.dir/requires: CMakeFiles/passive-walker-init.dir/example/passive-walker/init.cpp.o.requires
.PHONY : CMakeFiles/passive-walker-init.dir/requires

CMakeFiles/passive-walker-init.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/passive-walker-init.dir/cmake_clean.cmake
.PHONY : CMakeFiles/passive-walker-init.dir/clean

CMakeFiles/passive-walker-init.dir/depend:
	cd /home/shankar/CSCI6525/project/Moby/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/shankar/CSCI6525/project/Moby /home/shankar/CSCI6525/project/Moby /home/shankar/CSCI6525/project/Moby/build /home/shankar/CSCI6525/project/Moby/build /home/shankar/CSCI6525/project/Moby/build/CMakeFiles/passive-walker-init.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/passive-walker-init.dir/depend

