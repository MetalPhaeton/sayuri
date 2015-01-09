/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Hironori Ishibashi
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
 * @file sayulisp.h
 * @author Hironori Ishibashi
 * @brief Sayuri用Lispライブラリ。
 */
#ifndef SAYULISP_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define SAYULISP_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include "common.h"
#include "params.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "uci_shell.h"
#include "lisp_core.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class SearchParams;
  class EvalParams;
  class ChessEngine;
  class TranspositionTable;
  class UCIShell;

  /** Sayulisp用エンジンセット。 */
  class EngineSuite {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      EngineSuite();
      /**
       * コピーコンストラクタ。
       * @param suite コピー元。
       */
      EngineSuite(const EngineSuite& suite);
      /**
       * ムーブコンストラクタ。
       * @param suite ムーブ元。
       */
      EngineSuite(EngineSuite&& suite);
      /**
       * コピー代入演算子。
       * @param suite コピー元。
       */
      EngineSuite& operator=(const EngineSuite& suite);
      /**
       * ムーブ代入演算子。
       * @param suite ムーブ元。
       */
      EngineSuite& operator=(EngineSuite&& suite);
      /** デストラクタ。 */
      virtual ~EngineSuite() {}

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 探索関数用パラメータ。
       * @return 探索関数用パラメータ。
       */
      const SearchParams& search_params() const {return *search_params_ptr_;}
      /**
       * アクセサ - 評価関数用パラメータ。
       * @return 評価関数用パラメータ。
       */
      const EvalParams& eval_params() const {return *eval_params_ptr_;}
      /**
       * アクセサ - チェスエンジン。
       * @return チェスエンジン。
       */
      const ChessEngine& engine() const {return *engine_ptr_;}
      /**
       * アクセサ - トランスポジションテーブル。
       * @return トランスポジションテーブル。
       */
      const TranspositionTable& table() const {return *table_ptr_;}
      /**
       * アクセサ - UCIShell。
       * @return UCIShell。
       */
      const UCIShell& shell() const {return *shell_ptr_;}

    private:
      /** 探索関数用パラメータのポインタ。 */
      std::unique_ptr<SearchParams> search_params_ptr_;
      /** 評価関数用パラメータのポインタ。 */
      std::unique_ptr<EvalParams> eval_params_ptr_;
      /** チェスエンジンのポインタ。 */
      std::unique_ptr<ChessEngine> engine_ptr_;
      /** トランスポジションテーブルのポインタ。 */
      std::unique_ptr<TranspositionTable> table_ptr_;
      /** UCIShellのポインタ。 */
      std::unique_ptr<UCIShell> shell_ptr_;
  };
}  // namespace Sayuri

#endif
