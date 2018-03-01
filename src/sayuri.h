/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Hironori Ishibashi
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
 * @file sayuri.h
 * @author Hironori Ishibashi
 * @brief Sayuriへのインターフェイス。
 */

#ifndef SAYURI_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define SAYURI_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#ifdef __cplusplus
namespace Sayuri {
  extern "C" {
#endif

/** 
 * Sayulispのコードを実行する。
 * @param code Sayulispのコード。
 * @return 結果のS式。
 */
const char* ExecuteSayulisp(const char* code);

#ifdef __cplusplus
  }  // extgern "C"
}  // namespace Sayuri
#endif

#endif
