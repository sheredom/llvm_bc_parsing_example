# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

project(llvm_bc_parsing_example)
cmake_minimum_required(VERSION 3.4.3)

# pption to allow a user to specify where an LLVM install is on the system
set(LLVM_INSTALL_DIR "" CACHE STRING "An LLVM install directory.")

if("${LLVM_INSTALL_DIR}" STREQUAL "")
  message(FATAL_ERROR "LLVM_INSTALL_DIR not set! Set it to the location of an LLVM install.")
endif()

# fixup paths to only use the Linux convention
string(REPLACE "\\" "/" LLVM_INSTALL_DIR ${LLVM_INSTALL_DIR})

# tell CMake where LLVM's module is
list(APPEND CMAKE_MODULE_PATH ${LLVM_INSTALL_DIR}/lib/cmake/llvm)

# include LLVM
include(LLVMConfig)

add_executable(llvm_bc_parsing_example main.c)

target_include_directories(llvm_bc_parsing_example PUBLIC ${LLVM_INCLUDE_DIRS})

target_link_libraries(llvm_bc_parsing_example PUBLIC LLVMBitReader LLVMBitWriter)

target_compile_options(llvm_bc_parsing_example PUBLIC
  $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -Werror -std=gnu89>
  $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -Werror -std=gnu89>
  $<$<C_COMPILER_ID:MSVC>:/Wall /WX /wd4514 /wd4710 /wd4820>)
