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
#include <string>
#include <sstream>
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
      // 定数配列。
      /** 各マスのシンボルのテーブル。 */
      static const std::string SQUARE_SYMBOL[NUM_SQUARES];

      /** 各ファイルのシンボルテーブル。 */
      static const std::string FYLE_SYMBOL[NUM_FYLES];

      /** 各ランクのシンボルテーブル。 */
      static const std::string RANK_SYMBOL[NUM_RANKS];

      /** 各サイドのシンボルテーブル。 */
      static const std::string SIDE_SYMBOL[NUM_SIDES];

      /** 各駒のシンボルテーブル。 */
      static const std::string PIECE_SYMBOL[NUM_PIECE_TYPES];

      /**
       * 各キャスリングのシンボルテーブル。
       * - NO_CASTLING : 0
       * - WHITE_SHORT_CASTLING : 1
       * - WHITE_LONG_CASTLING : 2
       * - BLACK_SHORT_CASTLING : 3
       * - BLACK_LONG_CASTLING : 4
       */
      static const std::string CASTLING_SYMBOL[5];

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

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 関数オブジェクト。
       * @param self 自分自身。
       * @param caller 呼び出し元の関数オブジェクト。
       * @param list 呼び出しに使用されたリスト。
       * @return 戻り値オブジェクト。
       */
      LispObjectPtr operator()
      (LispObjectPtr self, const LispObject& caller, const LispObject& list);

      /**
       * 間違ったマスの指定のエラーを作成する。
       * @param func_name 関数名。
       * @param square 間違ったマス。
       * @return エラーオブジェクト。
       */
      static LispObjectPtr GenWrongSquareError
      (const std::string& func_name, Square square) {
        std::string message = "The value '" + std::to_string(square)
        + "' given to (" + func_name + ") does not exist on chess board.";

        return LispObject::GenError("@not-square", message);
      }

      /**
       * 間違ったファイルの指定のエラーを作成する。
       * @param func_name 関数名。
       * @param fyle 間違ったファイル。
       * @return エラーオブジェクト。
       */
      static LispObjectPtr GenWrongFyleError
      (const std::string& func_name, Fyle fyle) {
        std::string message = "The value '" + std::to_string(fyle)
        + "' given to (" + func_name + ") does not exist on chess board.";

        return LispObject::GenError("@not-fyle", message);
      }

      /**
       * 間違ったランクの指定のエラーを作成する。
       * @param func_name 関数名。
       * @param rank 間違ったファイル。
       * @return エラーオブジェクト。
       */
      static LispObjectPtr GenWrongRankError
      (const std::string& func_name, Rank rank) {
        std::string message = "The value '" + std::to_string(rank)
        + "' given to (" + func_name + ") does not exist on chess board.";

        return LispObject::GenError("@not-rank", message);
      }

      /**
       * 間違ったサイドの指定のエラーを作成する。
       * @param func_name 関数名。
       * @param side 間違ったサイド。
       * @return エラーオブジェクト。
       */
      static LispObjectPtr GenWrongSideError
      (const std::string& func_name, Side side) {
        std::string message = "The value '" + std::to_string(side)
        + "' given to (" + func_name + ") is not side.";

        return LispObject::GenError("@not-side", message);
      }

      /**
       * 間違ったキャスリングの指定のエラーを作成する。
       * @param func_name 関数名。
       * @param castling 間違ったサイド。
       * @return エラーオブジェクト。
       */
      static LispObjectPtr GenWrongCastlingError
      (const std::string& func_name, int castling) {
        std::string message = "The value '" + std::to_string(castling)
        + "' given to (" + func_name + ") does not indicate any castlings.";
        return LispObject::GenError("@not-castling", message);
      }

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
      // ========================== //
      // Lisp関数オブジェクト用関数 //
      // ========================== //
      // --- エンジンの状態にアクセス --- //
      /**
       * 白ポーンの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetWhitePawnPosition() const;
      /**
       * 白ナイトの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetWhiteKnightPosition() const;
      /**
       * 白ビショップの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetWhiteBishopPosition() const;
      /**
       * 白ルークの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetWhiteRookPosition() const;
      /**
       * 白クイーンの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetWhiteQueenPosition() const;
      /**
       * 白キングの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetWhiteKingPosition() const;
      /**
       * 黒ポーンの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetBlackPawnPosition() const;
      /**
       * 黒ナイトの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetBlackKnightPosition() const;
      /**
       * 黒ビショップの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetBlackBishopPosition() const;
      /**
       * 黒ルークの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetBlackRookPosition() const;
      /**
       * 黒クイーンの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetBlackQueenPosition() const;
      /**
       * 黒キングの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetBlackKingPosition() const;
      /**
       * 空のマスの配置を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetEmptySquarePosition() const;

      /**
       * その位置の駒を得る。
       * @param func_name 関数名。
       * @param square マスを表す定数。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetPiece
      (const std::string& func_name, Square square) const;

      /**
       * 手番を得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetToMove() const;

      /**
       * キャスリングの権利を得る。
       */
      LispObjectPtr GetCastlingRights() const;

      /**
       * アンパッサンのマスを得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetEnPassantSquare() const;

      /**
       * 手数を得る。
       */
      LispObjectPtr GetPly() const;

      /**
       * 50手ルールの手数を得る。
       */
      LispObjectPtr GetPly100() const;

      /**
       * 白がキャスリングしたかどうかのフラグを得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetWhiteHasCastled() const;
      /**
       * 黒がキャスリングしたかどうかのフラグを得る。
       * @return 戻り値のオブジェクト。
       */
      LispObjectPtr GetBlackHasCastled() const;

      /**
       * ボードを式状態にする。
       * @return trueのBooleanオブジェクト。
       */
      LispObjectPtr SetStartPosition();

      /**
       * FENの駒の配置にする。
       * @param fen_str_ptr FEN文字列の入ったStringオブジェクト。
       * @return trueのBooleanオブジェクト。
       */
      LispObjectPtr SetFEN(LispObjectPtr fen_str_ptr);

      // ========== //
      // メンバ変数 //
      // ========== //
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

  /** Sayulisp実行クラス。 */
  class Sayulisp {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      Sayulisp();
      /**
       * コピーコンストラクタ。
       * @param sayulisp コピー元。
       */
      Sayulisp(const Sayulisp& sayulisp);
      /**
       * ムーブコンストラクタ。
       * @param sayulisp ムーブ元。
       */
      Sayulisp(Sayulisp&& sayulisp);
      /**
       * コピー代入演算子。
       * @param sayulisp コピー元。
       */
      Sayulisp& operator=(const Sayulisp& sayulisp);
      /**
       * ムーブ代入演算子。
       * @param sayulisp ムーブ元。
       */
      Sayulisp& operator=(Sayulisp&& sayulisp);
      /** デストラクタ。 */
      virtual ~Sayulisp() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * Sayulispを開始する。
       * @param stream_ptr 入力に使うストリームのポインタ。
       */
      void Run(std::istream* stream_ptr);

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * ヘルプを作成する。
       */
      void SetHelp();

      // ========================== //
      // Lisp関数オブジェクト用関数 //
      // ========================== //
      /**
       * エンジン関数オブジェクトを生成する。
       * @return エンジン関数オブジェクト。
       */
      LispObjectPtr GenEngine();

      // ========== //
      // メンバ変数 //
      // ========== //
      /** ヘルプ辞書。 */
      std::shared_ptr<HelpDict> dict_ptr_;
      /** グローバル関数オブジェクト。 */
      LispObjectPtr global_ptr_;
  };
}  // namespace Sayuri

#endif
