/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
 * @file pv_line.cpp
 * @author Hironori Ishibashi
 * @brief PVラインを記録するクラスの実装。
 */

#include "pv_line.h"

#include <iostream>
#include <cstddef>
#include <cstring>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  PVLine::PVLine() : last_(0), score_(0), mate_in_(-1) {
    INIT_ARRAY(line_);
  }

  // コピーコンストラクタ。
  PVLine::PVLine(const PVLine& pv_line) :
  last_(pv_line.last_),
  score_(pv_line.score_),
  mate_in_(pv_line.mate_in_) {
    COPY_ARRAY(line_, pv_line.line_);
  }

  // ムーブコンストラクタ。
  PVLine::PVLine(PVLine&& pv_line) :
  last_(pv_line.last_),
  score_(pv_line.score_),
  mate_in_(pv_line.mate_in_) {
    COPY_ARRAY(line_, pv_line.line_);
  }

  // コピー代入演算子。
  PVLine& PVLine::operator=(const PVLine& pv_line) {
    score_ = pv_line.score_;
    mate_in_ = pv_line.mate_in_;
    last_ = pv_line.last_;
    COPY_ARRAY(line_, pv_line.line_);

    return *this;
  }

  // ムーブ代入演算子。
  PVLine& PVLine::operator=(PVLine&& pv_line) {
    score_ = pv_line.score_;
    mate_in_ = pv_line.mate_in_;
    last_ = pv_line.last_;
    COPY_ARRAY(line_, pv_line.line_);

    return *this;
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // 2手目以降を挿入する。
  void PVLine::Insert(const PVLine& pv_line) {
    // PVラインをコピー。
    last_ = pv_line.last_ + 1;
    std::memcpy(&line_[1], pv_line.line_, sizeof(Move) * pv_line.last_);

    // メイトまでのプライをコピー。
    mate_in_ = pv_line.mate_in_;
  }
}  // namespace Sayuri
