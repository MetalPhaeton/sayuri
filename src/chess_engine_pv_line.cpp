/*
   chess_engine_pv_line.cpp: PVのラインを格納するクラスの実装。

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

#include "chess_engine.h"

#include <iostream>
#include <cstddef>

namespace Sayuri {
  /********************/
  /* コンストラクタ。 */
  /********************/
  // コンストラクタ。
  ChessEngine::PVLine::PVLine() : length_(0) {}

  // コピーコンストラクタ。
  ChessEngine::PVLine::PVLine(const PVLine& pv_line) :
  length_(pv_line.length_) {
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_;
    }
  }

  // ムーブコンストラクタ。
  ChessEngine::PVLine::PVLine(PVLine&& pv_line) :
  length_(pv_line.length_) {
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_;
    }
  }

  // コピー代入。
  ChessEngine::PVLine& ChessEngine::PVLine::operator=(const PVLine& pv_lien) {
    length_ = pv_line.length_;
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_;
    }
  }

  // ムーブ代入。
  ChessEngine::PVLine& ChessEngine::PVLine::operator=(PVLine&& pv_lien) {
    length_ = pv_line.length_;
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_;
    }
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  // 最初の要素に手をセットする。
  void ChessEngine::PVLine::SetFirst(Move move) {
    line_[0] = move;
    if (length <= 0) length = 1;
  }

  // PVラインを2番目以降の要素にコピーする。
  void ChessEngine::PVLine::Insert(const PVLine& pv_line) {
    length_ = pv_line.length_ + 1;
    for (std::size_t i = 1, i < length_; i++) {
      line_[i] = pv_line.line_[i - 1];
    }
  }
}  // namespace Sayuri
