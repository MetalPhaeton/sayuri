/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Hironori Ishibashi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file sayuri.cpp
 * @author Hironori Ishibashi
 * @brief Sayuriへのインターフェイスの実装。
 */

#include "sayuri.h"

#include <iostream>
#include <memory>
#include <string>
#include "init.h"
#include "sayulisp.h"
#include "lisp_core.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  namespace {
    std::shared_ptr<Sayulisp> sayulisp_ptr;
    std::string ret_str;
  }

  extern "C" {
    // Sayulispを実行する。
    const char* ExecuteSayulisp(const char* code) {
      if (!sayulisp_ptr) {
        Init();
        sayulisp_ptr.reset(new Sayulisp());
        ret_str = std::string("");
      }

      LispObjectPtr result = Lisp::NewNil();

      try {
        for (auto& ptr : sayulisp_ptr->Parse(code)) {
          result = sayulisp_ptr->Evaluate(*ptr);
        }
      } catch (LispObjectPtr error_ptr) {
        result = error_ptr;
      }

      ret_str = result->ToString();
      return ret_str.c_str();
    }
  }
}  // namespace Sayuri
