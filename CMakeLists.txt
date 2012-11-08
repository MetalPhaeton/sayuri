# CMakeLists.txt: CMake用設定ファイル。
# Copyright (c) 2012 Ishibashi Hironori
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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# プロジェクト名。
PROJECT(MISAKI)

# Boostの確認と設定。
FIND_PACKAGE(Boost REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# ソースとヘッダファイルのリスト。
FILE(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
FILE(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# デフォルトでリリース用のコンパイルに設定する。
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT CMAKE_BUILD_TYPE)

# リリース用のコンパイラのオプション設定。
IF(CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -march=native")
    SET(CMAKE_C_FLAGSE_RELEASE "-DNDEBUG -O3 -march=native")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# デバッグ用のコンパイラオプションの設定
IF(CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")  # デバッグ用のフラグ。
    SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")  # デバッグ用のフラグ。
    SET(CMAKE_CXX_FLAGS_PROFILE "-pg")  # プロファイル作成用のフラグ。
    SET(CMAKE_C_FLAGS_PROFILE "-pg")  # プロファイル作成用のフラグ。
    SET(CMAKE_LD_FLAGS_PROFILE "-lgmon")  # プロファイル作成用のフラグ。
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# ビルド設定。
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src)
ADD_LIBRARY(misaki STATIC ${SRC})
TARGET_LINK_LIBRARIES(misaki boost_thread)
ADD_DEPENDENCIES(misaki ${HEADERS})
ADD_CUSTOM_COMMAND(TARGET misaki POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${CMAKE_CURRENT_BINARY_DIR}/include/misaki)
FOREACH(H_FILE ${HEADERS})
    ADD_CUSTOM_COMMAND(TARGET misaki POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${H_FILE} ${CMAKE_CURRENT_BINARY_DIR}/include/misaki/)
ENDFOREACH(H_FILE ${HEADERS})
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include)

# CPackの設定。
SET(CPACK_PACKAGE_VERSION_MAJOR 1)
SET(CPACK_PACKAGE_VERSION_MINOR 2)
SET(CPACK_PACKAGE_VERSION_PATCH 6)
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "misaki-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")

# CPackを実行。
INCLUDE(CPack)

# make distで作るように設定を変える。
ADD_CUSTOM_TARGET(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
