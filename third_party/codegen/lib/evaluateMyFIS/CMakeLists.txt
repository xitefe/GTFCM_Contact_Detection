###########################################################################
# CMakeLists.txt generated for component evaluateMyFIS
# Product type: STATIC library
###########################################################################
cmake_minimum_required(VERSION 3.12)
project(evaluateMyFIS)

# Propagate the CMAKE_EXPORT_COMPILE_COMMANDS variable from the 
# environment if it is defined as an environment variable, but not as a 
# CMake variable. This is to work around a bug in CMake 3.19 when the 
# "NMake Makefiles" generator is selected.
if(DEFINED ENV{CMAKE_EXPORT_COMPILE_COMMANDS} AND NOT DEFINED CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS $ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
endif()


###########################################################################
## Path variables
###########################################################################
# Derive an absolute path to the code generation anchor folder.
get_filename_component(START_DIR ../../.. ABSOLUTE)

# Special directories defined by using CACHE variables can be overridden 
# by setting the variable from the command line, e.g.,
# 
# 	cmake . -DMATLAB_ROOT=/path/to/another/matlab/root
if(WIN32)
    # Windows路径
    set(MATLAB_ROOT "E:/Acdemic App/Matlab/MATLAB/R2024a" CACHE PATH "")
else()
    # 在Linux上不需要MATLAB路径，设置为空即可
    set(MATLAB_ROOT "" CACHE PATH "")
endif()


# Additional variables that are defined conditionally.
if("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(BINARY_START_DIR "${START_DIR}")
else()
    set(BINARY_START_DIR "${CMAKE_BINARY_DIR}")
endif()

###########################################################################
## System Libraries
###########################################################################
find_library(FOUND_LIBM m NO_SYSTEM_ENVIRONMENT_PATH PATHS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})

###########################################################################
## Target definition and commands
###########################################################################

# Definition of target "evaluateMyFIS".
add_library(evaluateMyFIS ${START_DIR}/codegen/lib/evaluateMyFIS/evaluateMyFIS_initialize.cpp
    ${START_DIR}/codegen/lib/evaluateMyFIS/evaluateMyFIS_terminate.cpp
    ${START_DIR}/codegen/lib/evaluateMyFIS/evaluateMyFIS.cpp
    ${START_DIR}/codegen/lib/evaluateMyFIS/trimf.cpp
    ${START_DIR}/codegen/lib/evaluateMyFIS/evaluateAndMethod.cpp
    ${START_DIR}/codegen/lib/evaluateMyFIS/centroid.cpp)

# Set properties for target "evaluateMyFIS".
set_target_properties(evaluateMyFIS PROPERTIES PREFIX ""
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${BINARY_START_DIR}/codegen/lib/evaluateMyFIS/$<0:>"
    LIBRARY_OUTPUT_DIRECTORY "${BINARY_START_DIR}/codegen/lib/evaluateMyFIS/$<0:>"
    ARCHIVE_OUTPUT_DIRECTORY "${BINARY_START_DIR}/codegen/lib/evaluateMyFIS/$<0:>")

# Specify language features required for target "evaluateMyFIS".
target_compile_features(evaluateMyFIS PUBLIC cxx_std_11)

# Specify compiler preprocessor definitions for target "evaluateMyFIS".
target_compile_definitions(evaluateMyFIS PRIVATE -DMODEL=evaluateMyFIS)

# Specify include directories for target "evaluateMyFIS".
target_include_directories(evaluateMyFIS PUBLIC 
    $<BUILD_INTERFACE:${START_DIR}/codegen/lib/evaluateMyFIS>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/codegen/lib/evaluateMyFIS>
    $<BUILD_INTERFACE:${START_DIR}>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>>
    # 添加自定义包含目录
    ${START_DIR}/third_party/codegen/include
    # 仅当MATLAB_ROOT存在时才添加MATLAB包含路径
    $<$<BOOL:${MATLAB_ROOT}>:$<BUILD_INTERFACE:${MATLAB_ROOT}/extern/include>>
)

# Specify library link dependencies for target "evaluateMyFIS". CMake 
# generator expressions are used to create a CMakeLists.txt file that 
# supports multiple platforms with differently named system library 
# dependencies.
target_link_libraries(evaluateMyFIS PUBLIC $<$<BOOL:${FOUND_LIBM}>:m>)


###########################################################################
## Build success message
###########################################################################
add_custom_command(TARGET evaluateMyFIS POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "\\#\\#\\# Created STATIC library: $<TARGET_FILE:evaluateMyFIS>")

###########################################################################
## Call toolchain hook function if defined
###########################################################################
if(COMMAND toolchain_target_hook)
    toolchain_target_hook(evaluateMyFIS)
endif()
