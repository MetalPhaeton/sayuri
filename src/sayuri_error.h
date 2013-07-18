/* sayuri_error.h: Sayuriのエラーのヘッダ。

   The MIT License (MIT)

   Copyright (c) 2013 Ishibashi Hironori

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
 */
#ifndef SAYURI_ERROR_H
#define SAYURI_ERROR_H

#include <iostream>
#include <stdexcept>
#include <string>

namespace Sayuri {
  // Sayuriのエラークラス。
  class SayuriError : public std::logic_error {
    public:
      SayuriError(const char* message) : std::logic_error(message) {}
      SayuriError(const std::string message) : std::logic_error(message) {}
  };

  // 論理テスト。
  // [引数]
  // expr: 条件式。
  // [例外]
  // exprがfalseなら例外発生。
  void Assert(bool expr);
}  // namespace Sayuri

#endif
