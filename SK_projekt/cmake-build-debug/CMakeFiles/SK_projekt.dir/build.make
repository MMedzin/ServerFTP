# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /home/michal/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/203.6682.181/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/michal/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/203.6682.181/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/SK_projekt.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/SK_projekt.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/SK_projekt.dir/flags.make

CMakeFiles/SK_projekt.dir/main.c.o: CMakeFiles/SK_projekt.dir/flags.make
CMakeFiles/SK_projekt.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/SK_projekt.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/SK_projekt.dir/main.c.o   -c /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/main.c

CMakeFiles/SK_projekt.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SK_projekt.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/main.c > CMakeFiles/SK_projekt.dir/main.c.i

CMakeFiles/SK_projekt.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SK_projekt.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/main.c -o CMakeFiles/SK_projekt.dir/main.c.s

CMakeFiles/SK_projekt.dir/utils.c.o: CMakeFiles/SK_projekt.dir/flags.make
CMakeFiles/SK_projekt.dir/utils.c.o: ../utils.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/SK_projekt.dir/utils.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/SK_projekt.dir/utils.c.o   -c /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/utils.c

CMakeFiles/SK_projekt.dir/utils.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SK_projekt.dir/utils.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/utils.c > CMakeFiles/SK_projekt.dir/utils.c.i

CMakeFiles/SK_projekt.dir/utils.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SK_projekt.dir/utils.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/utils.c -o CMakeFiles/SK_projekt.dir/utils.c.s

CMakeFiles/SK_projekt.dir/command_parser.c.o: CMakeFiles/SK_projekt.dir/flags.make
CMakeFiles/SK_projekt.dir/command_parser.c.o: ../command_parser.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/SK_projekt.dir/command_parser.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/SK_projekt.dir/command_parser.c.o   -c /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/command_parser.c

CMakeFiles/SK_projekt.dir/command_parser.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SK_projekt.dir/command_parser.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/command_parser.c > CMakeFiles/SK_projekt.dir/command_parser.c.i

CMakeFiles/SK_projekt.dir/command_parser.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SK_projekt.dir/command_parser.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/command_parser.c -o CMakeFiles/SK_projekt.dir/command_parser.c.s

CMakeFiles/SK_projekt.dir/commands.c.o: CMakeFiles/SK_projekt.dir/flags.make
CMakeFiles/SK_projekt.dir/commands.c.o: ../commands.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/SK_projekt.dir/commands.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/SK_projekt.dir/commands.c.o   -c /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/commands.c

CMakeFiles/SK_projekt.dir/commands.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SK_projekt.dir/commands.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/commands.c > CMakeFiles/SK_projekt.dir/commands.c.i

CMakeFiles/SK_projekt.dir/commands.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SK_projekt.dir/commands.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/commands.c -o CMakeFiles/SK_projekt.dir/commands.c.s

CMakeFiles/SK_projekt.dir/hashmap_threads.c.o: CMakeFiles/SK_projekt.dir/flags.make
CMakeFiles/SK_projekt.dir/hashmap_threads.c.o: ../hashmap_threads.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/SK_projekt.dir/hashmap_threads.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/SK_projekt.dir/hashmap_threads.c.o   -c /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/hashmap_threads.c

CMakeFiles/SK_projekt.dir/hashmap_threads.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/SK_projekt.dir/hashmap_threads.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/hashmap_threads.c > CMakeFiles/SK_projekt.dir/hashmap_threads.c.i

CMakeFiles/SK_projekt.dir/hashmap_threads.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/SK_projekt.dir/hashmap_threads.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/hashmap_threads.c -o CMakeFiles/SK_projekt.dir/hashmap_threads.c.s

# Object files for target SK_projekt
SK_projekt_OBJECTS = \
"CMakeFiles/SK_projekt.dir/main.c.o" \
"CMakeFiles/SK_projekt.dir/utils.c.o" \
"CMakeFiles/SK_projekt.dir/command_parser.c.o" \
"CMakeFiles/SK_projekt.dir/commands.c.o" \
"CMakeFiles/SK_projekt.dir/hashmap_threads.c.o"

# External object files for target SK_projekt
SK_projekt_EXTERNAL_OBJECTS =

SK_projekt: CMakeFiles/SK_projekt.dir/main.c.o
SK_projekt: CMakeFiles/SK_projekt.dir/utils.c.o
SK_projekt: CMakeFiles/SK_projekt.dir/command_parser.c.o
SK_projekt: CMakeFiles/SK_projekt.dir/commands.c.o
SK_projekt: CMakeFiles/SK_projekt.dir/hashmap_threads.c.o
SK_projekt: CMakeFiles/SK_projekt.dir/build.make
SK_projekt: CMakeFiles/SK_projekt.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking C executable SK_projekt"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SK_projekt.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/SK_projekt.dir/build: SK_projekt

.PHONY : CMakeFiles/SK_projekt.dir/build

CMakeFiles/SK_projekt.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/SK_projekt.dir/cmake_clean.cmake
.PHONY : CMakeFiles/SK_projekt.dir/clean

CMakeFiles/SK_projekt.dir/depend:
	cd /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug /home/michal/Documents/studia/semestr_5/SK2/projekt/ServerFTP/SK_projekt/cmake-build-debug/CMakeFiles/SK_projekt.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/SK_projekt.dir/depend

