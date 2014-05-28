/*
   pv_line.cpp: PVのラインを格納するクラスの実装。

   The MIT License (MIT)

   Copyright (c) 2014 Hironori Ishibashi

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
#include "common.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  PVLine::PVLine() : begin_(line_), end_(&(line_[MAX_PLYS + 1])),
  last_(begin_), score_(0), ply_mate_(-1) {}

  // コピーコンストラクタ。
  PVLine::PVLine(const PVLine& pv_line) :
  begin_(line_),
  end_(&(line_[MAX_PLYS + 1])),
  last_(begin_ + (pv_line.last_ - pv_line.begin_)),
  score_(pv_line.score_),
  ply_mate_(pv_line.ply_mate_) {
    for (int i = 0; i < (end_ - begin_); i++) {
      line_[i] = pv_line.line_[i];
    }
  }

  // ムーブコンストラクタ。
  PVLine::PVLine(PVLine&& pv_line) :
  begin_(line_),
  end_(&(line_[MAX_PLYS + 1])),
  last_(begin_ + (pv_line.last_ - pv_line.begin_)),
  score_(pv_line.score_),
  ply_mate_(pv_line.ply_mate_) {
    for (int i = 0; i < (end_ - begin_); i++) {
      line_[i] = pv_line.line_[i];
    }
  }

  // コピー代入。
  PVLine& PVLine::operator=(const PVLine& pv_line) {
    score_ = pv_line.score_;
    ply_mate_ = pv_line.ply_mate_;
    last_ = begin_ + (pv_line.last_ - pv_line.begin_);
    for (int i = 0; i < (end_ - begin_); i++) {
      line_[i] = pv_line.line_[i];
    }

    return *this;
  }

  // ムーブ代入。
  PVLine& PVLine::operator=(PVLine&& pv_line) {
    score_ = pv_line.score_;
    ply_mate_ = pv_line.ply_mate_;
    last_ = begin_ + (pv_line.last_ - pv_line.begin_);
    for (int i = 0; i < (end_ - begin_); i++) {
      line_[i] = pv_line.line_[i];
    }

    return *this;
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  // PVラインを2番目以降の要素にコピーする。
  void PVLine::Insert(const PVLine& pv_line) {
    // PVラインをコピー。
    last_ = (begin_ + 1) + (pv_line.last_ - pv_line.begin_);
    for (int i = 1; i < (last_ - begin_); i++) {
      line_[i] = pv_line.line_[i - 1];
    }

    // メイトまでのプライをコピー。
    ply_mate_ = pv_line.ply_mate_;
  }
}  // namespace Sayuri
