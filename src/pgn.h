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
 * @file pgn.h
 * @author Hironori Ishibashi
 * @brief PGN。
 */

#ifndef PGN_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define PGN_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <string>
#include <queue>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** PGNパーサ。 */
  class PGN {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      PGN();
      /**
       * コピーコンストラクタ。
       * @param pgn コピー元。
       */
      PGN(const PGN& pgn);
      /**
       * ムーブコンストラクタ。
       * @param pgn ムーブ元。
       */
      PGN(PGN&& pgn);
      /**
       * コピー代入演算子。
       * @param pgn コピー元。
       */
      PGN& operator=(const PGN& pgn);
      /**
       * ムーブ代入演算子。
       * @param pgn ムーブ元。
       */
      PGN& operator=(PGN&& pgn);
      /** デストラクタ。 */
      virtual ~PGN() {}

    private:
      /** フレンド。 */
      friend int DebugMain(int, char**);

      /**
       * 字句解析する。
       * @param str 字句解析したい文字列。
       * @return トークンのキュー。
       */
      static std::queue<std::string> Tokenize(const std::string& str);
  };
}  // namespace Sayuri

#endif
