# CMakeLists.txt: CMake用設定ファイル。
#
#  The MIT License (MIT)
#
#  Copyright (c) 2015 Hironori Ishibashi
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to
#  deal in the Software without restriction, including without limitation the
#  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
#  sell copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
#  IN THE SOFTWARE.

cmake_minimum_required(VERSION 2.8)

# コンパイラの設定。
if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER clang)
endif(NOT CMAKE_C_COMPILER)
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif(NOT CMAKE_CXX_COMPILER)

# プロジェクト名。
project(SAYURI)
set(VERSION "2015.10.20")

# ソースとヘッダファイルのリスト。
file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# デフォルトでリリース用のコンパイルに設定する。
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# 基本オプション。
set(BASIC_FLAGS "-std=c++11 -Wall -Wsign-compare -fexceptions -fno-rtti -pthread -march=native")

# リリース用のコンパイラのオプション設定。
set(CMAKE_C_FLAGS_RELEASE "${BASIC_FLAGS} -Ofast")
set(CMAKE_CXX_FLAGS_RELEASE "${BASIC_FLAGS} -Ofast")

# デバッグ用のコンパイラオプションの設定
set(CMAKE_C_FLAGS_DEBUG "${BASIC_FLAGS} -Ofast -g")
set(CMAKE_CXX_FLAGS_DEBUG "${BASIC_FLAGS} -Ofast -g")

# プロファイル用コンパイラオプションの設定。
set(CMAKE_C_FLAGS_PROFILE "${BASIC_FLAGS} -pg")
set(CMAKE_CXX_FLAGS_PROFILE "${BASIC_FLAGS} -pg")

# ビルド設定。
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(sayuri ${SRCS})

# CPackの設定。
set(CPACK_SOURCE_PACKAGE_FILE_NAME "sayuri-${VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/\\.git/;/\\.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

# CPackを実行。
include(CPack)

# make distで作るように設定を変える。
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
