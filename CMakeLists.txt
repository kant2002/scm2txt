project (scm2txt)
cmake_minimum_required (VERSION 3.0)

if (NOT EXISTS "${scm2txt_SOURCE_DIR}/dependencies/StormLib/CMakeLists.txt")
    message(FATAL_ERROR
"Missing dependency: StormLib
scm2txt requires the StormLib library.
It is provided as a GIT submodule of this repository.
Did you forgot to execute the following commands?
   git submodule init
   git submodule update")
endif()

add_subdirectory(dependencies/StormLib)
add_subdirectory (src)
