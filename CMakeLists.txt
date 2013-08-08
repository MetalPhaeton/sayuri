# CMakeLists.txt: CMake用設定ファイル。
# Copyright (c) 2013 Ishibashi Hironori
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

cmake_minimum_required(VERSION 2.8)

# プロジェクト名。
project(SAYURI)

# ソースとヘッダファイルのリスト。
file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# デフォルトでリリース用のコンパイルに設定する。
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# リリース用のコンパイラのオプション設定。
set(CMAKE_C_FLAGSE_RELEASE "-std=c++11 -w -g -O2 -march=k8")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -w -g -O2 -march=k8")

# デバッグ用のコンパイラオプションの設定
set(CMAKE_C_FLAGS_DEBUG "-std=c++11 -Wall -O0 -g -march=k8")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -Wall -O0 -g -march=k8")

# プロファイル用オプションの設定。
set(CMAKE_C_FLAGS_PROFILE "-std=c++11 -w -pg -march=k8")
set(CMAKE_CXX_FLAGS_PROFILE "-std=c++11 -w -pg -march=k8")

# ビルド設定。
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(sayuri ${SRCS})
target_link_libraries(sayuri pthread)
add_dependencies(sayuri ${HEADERS})

# CPackの設定。
set(CPACK_SOURCE_PACKAGE_FILE_NAME "sayuri.0000.00.00")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")

# CPackを実行。
include(CPack)

# make distで作るように設定を変える。
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
