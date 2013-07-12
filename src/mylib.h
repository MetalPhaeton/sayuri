/*
   mylib.h: 自分用便利ライブラリのヘッダ。
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

/****************************/
/* 自分用ライブラリのヘッダ */
/****************************/

#ifndef MYLIB_H
#define MYLIB_H

#include <iostream>
#include <utility>
#include <string>
#include <vector>

namespace MyLib {
  // 文字列を切り分ける。
  // [引数]
  // str: 切り分ける文字列。
  // delim: 区切り文字。(複数可。)
  // delim_left: 残す区切り文字。(複数可。)
  // [戻り値]
  // 切り分けられた文字列のベクトル。
  std::vector<std::string> Split(std::string str, std::string delim,
  std::string delim_left);
}  // namespace MyLib

#endif
