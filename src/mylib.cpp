/*
   mylib.cpp: 自分用便利ライブラリのソース。
   Copyright (c) 2013 Ishibashi Hironori

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom
   the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
*/

/**************************/
/* 自分用ライブラリの実装 */
/**************************/

#include "mylib.h"

#include <iostream>
#include <utility>
#include <string>
#include <vector>

namespace MyLib {
  // 文字を切り分ける。
  std::vector<std::string> Split(std::string str, std::string delim,
  std::string delim_left) {
    std::vector<std::string> ret;  // 結果。

    // 一文字づつ処理する。
    std::vector<char> c_vec;  // その他の文字を格納する。
    for (char& str_c : str) {
      // 区切り文字か検査。
      bool is_delim = false;
      for (char& delim_c : delim) {
        if (str_c == delim_c) {
          is_delim = true;
          break;
        }
      }

      // 残す区切り文字か検査。
      bool is_delim_left = false;
      for (char& delim_left_c : delim_left) {
        if (str_c == delim_left_c) {
          is_delim_left = true;
          break;
        }
      }

      if (is_delim) {  // 区切り文字。
        if (c_vec.size() > 0) {
          c_vec.push_back('\0');  // ヌル文字を追加。
          ret.push_back(std::string(c_vec.data()));
          c_vec.clear();
        }
      } else if (is_delim_left) {  // 残す区切り文字。
        if (c_vec.size() > 0) {
          c_vec.push_back('\0');  // ヌル文字を追加。
          ret.push_back(std::string(c_vec.data()));
          c_vec.clear();
        }
        c_vec.push_back(str_c);
        c_vec.push_back('\0');  // ヌル文字を追加。
        ret.push_back(std::string(c_vec.data()));
        c_vec.clear();
      } else {  // その他の文字。
        c_vec.push_back(str_c);
      }
    }
    if (c_vec.size() > 0) {
      c_vec.push_back('\0');  // ヌル文字を追加。
      ret.push_back(std::string(c_vec.data()));
    }

    return std::move(ret);
  }
}  // namespace MyLib
