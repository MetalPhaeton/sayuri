/*
   pv_line.cpp: PVのラインを格納するクラスの実装。

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

#include "pv_line.h"

#include <iostream>
#include <cstddef>
#include "chess_def.h"

namespace Sayuri {
  /**********/
  /* PVLine */
  /**********/
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  PVLine::PVLine() : length_(0), score_(0) {}

  // コピーコンストラクタ。
  PVLine::PVLine(const PVLine& pv_line) :
  length_(pv_line.length_), score_(pv_line.score_) {
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_[i];
    }
  }

  // ムーブコンストラクタ。
  PVLine::PVLine(PVLine&& pv_line) :
  length_(pv_line.length_), score_(pv_line.score_) {
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_[i];
    }
  }

  // コピー代入。
  PVLine& PVLine::operator=(const PVLine& pv_line) {
    length_ = pv_line.length_;
    score_ = pv_line.score_;
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_[i];
    }
    return *this;
  }

  // ムーブ代入。
  PVLine& PVLine::operator=(PVLine&& pv_line) {
    length_ = pv_line.length_;
    score_ = pv_line.score_;
    for (std::size_t i = 0; i < length_; i++) {
      line_[i] = pv_line.line_[i];
    }
    return *this;
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  // 最初の要素に手をセットする。
  void PVLine::SetMove(Move move) {
    line_[0].move_ = move;
    length_ = 1;
  }

  // PVラインを2番目以降の要素にコピーする。
  void PVLine::Insert(const PVLine& pv_line) {
    length_ = pv_line.length_ + 1;
    if (length_ > static_cast<std::size_t>(MAX_PLYS + 1))
      length_ = MAX_PLYS + 1;

    for (std::size_t i = 1; i < length_; i++) {
      line_[i] = pv_line.line_[i - 1];
    }
  }

  // 最初の要素にチェックメイトされたことを記録する。
  void PVLine::MarkCheckmated() {
    line_[0].has_checkmated_ = true;
    length_ = 1;
  }

  // ソート用比較関数。
  bool PVLine::Compare(const PVLine& first, const PVLine& second) {
    return first.score_ > second.score_;
  }

  /**********/
  /* PVSlot */
  /**********/
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  PVSlot::PVSlot() : has_checkmated_(false) {
    move_.all_ = 0;
  }

  // コピーコンストラクタ。
  PVSlot::PVSlot(const PVSlot& slot) : has_checkmated_(slot.has_checkmated_) {
    move_ = slot.move_;
  }

  // ムーブコンストラクタ。
  PVSlot::PVSlot(PVSlot&& slot) : has_checkmated_(slot.has_checkmated_) {
    move_ = slot.move_;
  }

  // コピー代入。
  PVSlot& PVSlot::operator=(const PVSlot& slot) {
    has_checkmated_ = slot.has_checkmated_;
    move_ = slot.move_;
    return *this;
  }

  // コピー代入。
  PVSlot& PVSlot::operator=(PVSlot&& slot) {
    has_checkmated_ = slot.has_checkmated_;
    move_ = slot.move_;
    return *this;
  }
}  // namespace Sayuri
