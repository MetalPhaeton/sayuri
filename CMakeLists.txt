# CMakeLists.txt: CMake用設定ファイル。
#
#  The MIT License (MIT)
#
#  Copyright (c) 2014 Hironori Ishibashi
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

# コンパイラの設定。
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

cmake_minimum_required(VERSION 2.8)

# プロジェクト名。
project(SAYURI)
set(VERSION "2014.03.05")

# ソースとヘッダファイルのリスト。
file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
# ライブラリ作成用ソースファイルのリスト。
file(GLOB LIBSRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM LIBSRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

# デフォルトでリリース用のコンパイルに設定する。
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# リリース用のコンパイラのオプション設定。
set(CMAKE_C_FLAGSE_RELEASE "-std=c++11 -Wall -O2 -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -Wall -O2 -march=native")

# デバッグ用のコンパイラオプションの設定
set(CMAKE_C_FLAGS_DEBUG "-std=c++11 -Wall -O0 -g -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -Wall -O0 -g -march=native")

# プロファイル用コンパイラオプションの設定。
set(CMAKE_C_FLAGS_PROFILE "-std=c++11 -Wall -pg -march=native")
set(CMAKE_CXX_FLAGS_PROFILE "-std=c++11 -Wall -pg -march=native")

# ビルド設定。
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable(sayuri ${SRCS})
target_link_libraries(sayuri pthread)
add_dependencies(sayuri ${HEADERS})

# ライブラリのビルド設定。
add_library(sayurilib STATIC ${LIBSRCS})
target_link_libraries(sayurilib pthread)
add_dependencies(sayurilib ${HEADERS})
# インクルードファイル作成設定。
add_custom_command(TARGET sayurilib POST_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include)
foreach(HFILE ${HEADERS})
    add_custom_command(TARGET sayurilib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy ${HFILE} ${CMAKE_CURRENT_BINARY_DIR}/include/)
endforeach(HFILE ${HEADERS})

# CPackの設定。
set(CPACK_SOURCE_PACKAGE_FILE_NAME "sayuri-${VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/\\.git/;/\\.gitignore;~$;${CPACK_SOURCE_IGNORE_FILES}")

# CPackを実行。
include(CPack)

# make distで作るように設定を変える。
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
