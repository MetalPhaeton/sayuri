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
#include <vector>
#include <sstream>
#include <functional>
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
      static const std::string PIECE_TYPE_SYMBOL[NUM_PIECE_TYPES];

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
       * 間違った駒の種類の指定のエラーを作成する。
       * @param func_name 関数名。
       * @param piece_type 間違ったマス。
       * @return エラーオブジェクト。
       */
      static LispObjectPtr GenWrongPieceTypeError
      (const std::string& func_name, PieceType piece_type) {
        std::string message = "The value '" + std::to_string(piece_type)
        + "' given to (" + func_name + ") is not a piece type.";

        return LispObject::GenError("@not-piece-type", message);
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

      /**
       * Moveをリストに変換する。
       * @param move 変換したいMove。
       * @return 変換結果のリスト。
       */
      static LispObjectPtr MoveToList(Move move) {
        LispObjectPtr ret_ptr = LispObject::NewList(3);

        ret_ptr->car(LispObject::NewSymbol(SQUARE_SYMBOL[GetFrom(move)]));

        ret_ptr->cdr()->car
        (LispObject::NewSymbol(SQUARE_SYMBOL[GetTo(move)]));

        ret_ptr->cdr()->cdr()->car
        (LispObject::NewSymbol(PIECE_TYPE_SYMBOL[GetPromotion(move)]));

        return ret_ptr;
      }

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
      LispObjectPtr GetClock() const;

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
       * ボードを初期状態にする。
       * @return trueのBooleanオブジェクト。
       */
      LispObjectPtr SetNewGame();

      /**
       * FENの駒の配置にする。
       * @param fen_str_ptr FEN文字列の入ったStringオブジェクト。
       * @return trueのBooleanオブジェクト。
       */
      LispObjectPtr SetFEN(LispObjectPtr fen_str_ptr);

      /**
       * 現在のボードの候補手のリストを得る。
       * @return 候補手のリスト。
       */
      LispObjectPtr GetCandidateMoves();

      /**
       * 駒を置く。
       * @param square_ptr 駒を置くマスのNumberオブジェクト。
       * @param type_ptr 置く駒の種類のNumberオブジェクト。
       * @param side_ptr 置く駒のサイドのNumberオブジェクト。
       * @return 元の位置にあった駒の種類とサイドのリスト。
       */
      LispObjectPtr PlacePiece(LispObjectPtr square_ptr,
      LispObjectPtr type_ptr, LispObjectPtr side_ptr);

      /**
       * 手番をセットする。
       * @param to_move_ptr 手番。
       * @return 変更前の値。
       */
      LispObjectPtr SetToMove(LispObjectPtr to_move_ptr);
      /**
       * キャスリングの権利をセットする。
       * @param castling_rights_ptr キャスリングの権利のリスト。
       * @param func_name 例外で使う関数名。
       * @return 変更前の値。
       */
      LispObjectPtr SetCastlingRights(LispObjectPtr castling_rights_ptr,
      const std::string& func_name);
      /**
       * アンパッサンの位置をセットする。
       * @param en_passant_square_ptr アンパッサンの位置。
       * @return 変更前の値。
       */
      LispObjectPtr SetEnPassantSquare(LispObjectPtr castling_rights_ptr);
      /**
       * 手数をセットする。
       * @param ply_ptr 手数。
       * @return 変更前の値。
       */
      LispObjectPtr SetPly(LispObjectPtr ply_ptr) {
        int ply = ply_ptr->number_value();
        if (ply < 1) {
          throw LispObject::GenError("@engine_error",
          "Minimum ply number is '1'. Given '"
          + DoubleToString(ply_ptr->number_value()) + "'.");
        }

        int origin = engine_ptr_->ply();
        engine_ptr_->ply(ply);
        return LispObject::NewNumber(origin);
      }
      /**
       * 50手ルールの手数をセットする。
       * @param clock_ptr 50手ルールの手数。
       * @return 変更前の値。
       */
      LispObjectPtr SetClock(LispObjectPtr clock_ptr) {
        int clock = clock_ptr->number_value();
        if (clock < 0) {
          throw LispObject::GenError("@engine_error",
          "Minimum clock number is '0'. Given '"
          + DoubleToString(clock_ptr->number_value()) + "'.");
        }

        int origin = engine_ptr_->clock();
        engine_ptr_->clock(clock);
        return LispObject::NewNumber(origin);
      }
      /**
       * 正しい駒の配置かどうか。
       * @return 正しければ#t。
       */
      LispObjectPtr IsCorrectPosition() const {
        return LispObject::NewBoolean(engine_ptr_->IsCorrectPosition());
      }
      /**
       * 白キングがチェックされているかどうか。
       * @return チェックされていれば#t。
       */
      LispObjectPtr IsWhiteChecked() const {
        return LispObject::NewBoolean
        (engine_ptr_->IsAttacked(engine_ptr_->king()[WHITE], BLACK));
      }
      /**
       * 黒キングがチェックされているかどうか。
       * @return チェックされていれば#t。
       */
      LispObjectPtr IsBlackChecked() const {
        return LispObject::NewBoolean
        (engine_ptr_->IsAttacked(engine_ptr_->king()[BLACK], WHITE));
      }
      /**
       * チェックメイトかどうか。
       * @return チェックメイトなら#t。
       */
      LispObjectPtr IsCheckmated() {
        Side side = engine_ptr_->to_move();
        if (engine_ptr_->IsAttacked
        (engine_ptr_->king()[side], Util::GetOppositeSide(side))) {
          std::vector<Move> move = engine_ptr_->GetLegalMoves();
          if (move.size() == 0) return LispObject::NewBoolean(true);
        }
        return LispObject::NewBoolean(false);
      }
      /**
       * ステールメイトかどうか。
       * @return ステールメイトなら#t。
       */
      LispObjectPtr IsStalemated() {
        Side side = engine_ptr_->to_move();
        if (!(engine_ptr_->IsAttacked
        (engine_ptr_->king()[side], Util::GetOppositeSide(side)))) {
          std::vector<Move> move = engine_ptr_->GetLegalMoves();
          if (move.size() == 0) return LispObject::NewBoolean(true);
        }
        return LispObject::NewBoolean(false);
      }

      /**
       * 1手指す。
       * @param caller 呼び出し元の関数。
       * @param func_name 呼びだされた関数名。
       * @param move_ptr (From To Promotion) のリストのポインタ。
       * @return #t。
       */
      LispObjectPtr PlayMove(const LispObject& caller,
      const std::string& func_name, LispObjectPtr move_ptr);
      /**
       * 手を戻す。
       * @return 戻された手のリスト。
       */
      LispObjectPtr UndoMove();

      /**
       * UCIコマンドを入力する。
       * @param command_ptr UCIコマンドの文字列。
       * @return コマンドが実行されれば#t。
       */
      LispObjectPtr InputUCICommand(LispObjectPtr command_ptr);

      /**
       * UCIのアウトプットリスナーを登録する。
       * @param caller 関数呼び出し元。
       * @param symbol リスナーのシンボル。
       * @return #t。
       */
      LispObjectPtr AddUCIOutputListener(const LispObject& caller,
      const LispObject& symbol);

      /**
       * move_timeミリ秒間思考する。 最善手が見つかるまで戻らない。
       * 思考中の出力はAddUCIOutputListener()で登録した関数で得られる。
       * @param func_name 関数名。
       * @param move_time 思考時間。 (ミリ秒)
       * @param move_list 探索したい候補手。
       * @return 最善手。
       */
      LispObjectPtr GoMoveTime(const std::string& func_name,
      const LispObject& move_time, const LispObject& move_list);
      /**
       * 持ち時間time(ミリ秒)で思考する。 最善手が見つかるまで戻らない。
       * 思考中の出力はAddUCIOutputListener()で登録した関数で得られる。
       * @param func_name 関数名。
       * @param time 持ち時間。 (ミリ秒)
       * @param move_list 探索したい候補手。
       * @return 最善手。
       */
      LispObjectPtr GoTimeLimit(const std::string& func_name,
      const LispObject& time, const LispObject& move_list);
      /**
       * 深さdepthまで思考する。 最善手が見つかるまで戻らない。
       * 思考中の出力はAddUCIOutputListener()で登録した関数で得られる。
       * @param func_name 関数名。
       * @param depth 深さ。
       * @param move_list 探索したい候補手。
       * @return 最善手。
       */
      LispObjectPtr GoDepth(const std::string& func_name,
      const LispObject& depth, const LispObject& move_list);
      /**
       * nodesのノード数まで思考する。 最善手が見つかるまで戻らない。
       * 思考中の出力はAddUCIOutputListener()で登録した関数で得られる。
       * @param func_name 関数名。
       * @param nodes ノード数。
       * @param move_list 探索したい候補手。
       * @return 最善手。
       */
      LispObjectPtr GoNodes(const std::string& func_name,
      const LispObject& nodes, const LispObject& move_list);

      /**
       * トランスポジションテーブルのサイズを変更する。
       * @param hash_size テーブルのサイズ。
       * @return 変更前のテーブルのサイズ。
       */
      LispObjectPtr SetHashSize(const LispObject& hash_size) {
        std::size_t old_size = table_ptr_->GetSizeBytes();

        table_ptr_->SetSize(hash_size.number_value());

        return LispObject::NewNumber(old_size);
      }

      /**
       * スレッドの数を変更する。
       * @param hash_size スレッドの数。
       * @return 変更前のスレッドの数。
       */
      LispObjectPtr SetThreads(const LispObject& num_threads) {
        int old_threads = shell_ptr_->num_threads();

        shell_ptr_->num_threads(num_threads.number_value());

        return LispObject::NewNumber(old_threads);
      }

      /**
       * SearchParams - マテリアル。
       * @param material_list マテリアルが記されたリスト。
       * @return セットされていたマテリアルのリスト。
       */
      LispObjectPtr SetMaterial(const LispObject& material_list);

      /**
       * SearchParams - クイース探索の有効無効。
       * @param enable クイース探索の有効無効。
       * @return セットされていたクイース探索の有効無効。
       */
      LispObjectPtr SetEnableQuiesceSearch(const LispObject& enable) {
        LispObjectPtr ret_ptr =
        LispObject::NewBoolean(search_params_ptr_->enable_quiesce_search());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_quiesce_search(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - 繰り返しチェックの有効無効。
       * @param enable 繰り返しチェックの有効無効。
       * @return セットされていた繰り返しチェックの有効無効。
       */
      LispObjectPtr SetEnableRepetitionCheck(const LispObject& enable) {
        LispObjectPtr ret_ptr =
        LispObject::NewBoolean(search_params_ptr_->enable_repetition_check());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_repetition_check(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - Check Extensionの有効無効。
       * @param enable Check Extensionの有効無効。
       * @return セットされていたCheck Extensionの有効無効。
       */
      LispObjectPtr SetEnableCheckExtension(const LispObject& enable) {
        LispObjectPtr ret_ptr =
        LispObject::NewBoolean(search_params_ptr_->enable_check_extension());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_check_extension(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - YBWCの深さ制限。
       * @param depth 深さ。
       * @return セットされていたYBWCの深さ制限。
       */
      LispObjectPtr SetYBWCLimitDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr =
        LispObject::NewNumber(search_params_ptr_->ybwc_limit_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->ybwc_limit_depth(depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - YBWCを無効にする最初の候補手の数。
       * @param num_moves 候補手の数。
       * @return セットされていたYBWCを無効にする最初の候補手の数。
       */
      LispObjectPtr SetYBWCInvalidMoves(const LispObject& num_moves) {
        LispObjectPtr ret_ptr =
        LispObject::NewNumber(search_params_ptr_->ybwc_invalid_moves());

        if (num_moves.IsNumber()) {
          search_params_ptr_->ybwc_invalid_moves(num_moves.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - Aspiration Windowsの有効無効。
       * @param enable Aspiration Windowsの有効無効。
       * @return セットされていたAspiration Windowsの有効無効。
       */
      LispObjectPtr SetEnableAspirationWindows(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_aspiration_windows());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_aspiration_windows
          (enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - Aspiration Windowsの深さ制限。
       * @param depth 深さ。
       * @return セットされていたAspiration Windowsの深さ制限。
       */
      LispObjectPtr SetAspirationWindowsLimitDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->aspiration_windows_limit_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->aspiration_windows_limit_depth
          (depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - Aspiration Windowsのデルタ値。
       * @param delta デルタ値。
       * @return セットされていたAspiration Windowsのデルタ値。
       */
      LispObjectPtr SetAspirationWindowsDelta(const LispObject& delta) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->aspiration_windows_delta());

        if (delta.IsNumber()) {
          search_params_ptr_->aspiration_windows_delta
          (delta.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - SEEの有効無効。
       * @param enable SEEの有効無効。
       * @return セットされていたSEEの有効無効。
       */
      LispObjectPtr SetEnableSEE(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_see());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_see(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - ヒストリーの有効無効。
       * @param enable ヒストリーの有効無効。
       * @return セットされていたヒストリーの有効無効。
       */
      LispObjectPtr SetEnableHistory(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_history());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_history(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - キラームーブの有効無効。
       * @param enable キラームーブの有効無効。
       * @return セットされていたキラームーブの有効無効。
       */
      LispObjectPtr SetEnableKiller(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_killer());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_killer(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - トランスポジションテーブルの有効無効。
       * @param enable トランスポジションテーブルの有効無効。
       * @return セットされていたトランスポジションテーブルの有効無効。
       */
      LispObjectPtr SetEnableHashTable(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_ttable());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_ttable(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - IIDの有効無効。
       * @param enable IIDの有効無効。
       * @return セットされていたIIDの有効無効。
       */
      LispObjectPtr SetEnableIID(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_iid());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_iid(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - IIDの深さ制限。
       * @param depth 深さ。
       * @return セットされていたIIDの深さ制限。
       */
      LispObjectPtr SetIIDLimitDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->iid_limit_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->iid_limit_depth(depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - IIDの探索深さ。
       * @param depth 深さ。
       * @return セットされていたIIDの探索深さ。
       */
      LispObjectPtr SetIIDSearchDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->iid_search_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->iid_search_depth(depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - NMRの有効無効。
       * @param enable NMRの有効無効。
       * @return セットされていたNMRの有効無効。
       */
      LispObjectPtr SetEnableNMR(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_nmr());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_nmr(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - NMRの深さ制限。
       * @param depth 深さ。
       * @return セットされていたNMRの深さ制限。
       */
      LispObjectPtr SetNMRLimitDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->nmr_limit_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->nmr_limit_depth(depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - NMRの探索時のリダクション。
       * @param reduction リダクション。
       * @return セットされていたNMRの探索時のリダクション。
       */
      LispObjectPtr SetNMRSearchReduction(const LispObject& reduction) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->nmr_search_reduction());

        if (reduction.IsNumber()) {
          search_params_ptr_->nmr_search_reduction(reduction.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - NMRの結果のリダクション。
       * @param reduction リダクション。
       * @return セットされていたNMRの結果のリダクション。
       */
      LispObjectPtr SetNMRReduction(const LispObject& reduction) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->nmr_reduction());

        if (reduction.IsNumber()) {
          search_params_ptr_->nmr_reduction(reduction.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - ProbCutの有効無効。
       * @param enable ProbCutの有効無効。
       * @return セットされていたProbCutの有効無効。
       */
      LispObjectPtr SetEnableProbCut(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_probcut());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_probcut(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - ProbCutの深さ制限。
       * @param depth 深さ。
       * @return セットされていたProbCutの深さ制限。
       */
      LispObjectPtr SetProbCutLimitDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->probcut_limit_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->probcut_limit_depth(depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - ProbCutのベータ値のマージン。
       * @param margin マージン。
       * @return セットされていたProbCutのベータ値のマージン。
       */
      LispObjectPtr SetProbCutMargin(const LispObject& margin) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->probcut_margin());

        if (margin.IsNumber()) {
          search_params_ptr_->probcut_margin(margin.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - ProbCutの探索時のリダクション。
       * @param reduction リダクション。
       * @return セットされていたProbCutの探索時のリダクション。
       */
      LispObjectPtr SetProbCutSearchReduction(const LispObject& reduction) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->probcut_search_reduction());

        if (reduction.IsNumber()) {
          search_params_ptr_->probcut_search_reduction
          (reduction.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - History Pruningの有効無効。
       * @param enable History Pruningの有効無効。
       * @return セットされていたHistory Pruningの有効無効。
       */
      LispObjectPtr SetEnableHistoryPruning(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_history_pruning());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_history_pruning(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - History Pruningの深さ制限。
       * @param depth 深さ。
       * @return セットされていたHistory Pruningの深さ制限。
       */
      LispObjectPtr SetHistoryPruningLimitDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->history_pruning_limit_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->history_pruning_limit_depth
          (depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - History Pruningの候補手の閾値。
       * @param threshold 閾値。
       * @return セットされていたHistory Pruningの候補手の閾値。
       */
      LispObjectPtr SetHistoryPruningMoveThreshold
      (const LispObject& threshold) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->history_pruning_move_threshold());

        if (threshold.IsNumber()) {
          search_params_ptr_->history_pruning_move_threshold
          (threshold.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - History Pruningを無効にする最初の候補手の数。
       * @param num_moves 候補手の数。
       * @return セットされていたHistory Pruningを無効にする最初の候補手の数。
       */
      LispObjectPtr SetHistoryPruningInvalidMoves
      (const LispObject& num_moves) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->history_pruning_invalid_moves());

        if (num_moves.IsNumber()) {
          search_params_ptr_->history_pruning_invalid_moves
          (num_moves.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - History Pruningのヒストリー値の閾値。
       * @param threshold 閾値。
       * @return セットされていたHistory Pruningのヒストリー値の閾値。
       */
      LispObjectPtr SetHistoryPruningThreshold(const LispObject& threshold) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->history_pruning_threshold());

        if (threshold.IsNumber()) {
          search_params_ptr_->history_pruning_threshold
          (threshold.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - History Pruningのリダクション。
       * @param reduction リダクション。
       * @return セットされていたHistory Pruningのリダクション。
       */
      LispObjectPtr SetHistoryPruningReduction(const LispObject& reduction) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->history_pruning_reduction());

        if (reduction.IsNumber()) {
          search_params_ptr_->history_pruning_reduction
          (reduction.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - LMRの有効無効。
       * @param enable LMRの有効無効。
       * @return セットされていたLMRの有効無効。
       */
      LispObjectPtr SetEnableLMR(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_lmr());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_lmr(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - LMRの深さ制限。
       * @param depth 深さ。
       * @return セットされていたLMRの深さ制限。
       */
      LispObjectPtr SetLMRLimitDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->lmr_limit_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->lmr_limit_depth(depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - LMRの候補手の閾値。
       * @param threshold 閾値。
       * @return セットされていたLMRの候補手の閾値。
       */
      LispObjectPtr SetLMRMoveThreshold(const LispObject& threshold) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->lmr_move_threshold());

        if (threshold.IsNumber()) {
          search_params_ptr_->lmr_move_threshold(threshold.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - LMRを無効にする最初の候補手の数。
       * @param num_moves 候補手の数。
       * @return セットされていたLMRを無効にする最初の候補手の数。
       */
      LispObjectPtr SetLMRInvalidMoves(const LispObject& num_moves) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->lmr_invalid_moves());

        if (num_moves.IsNumber()) {
          search_params_ptr_->lmr_invalid_moves(num_moves.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - LMRの探索時のリダクション。
       * @param reduction リダクション。
       * @return セットされていたLMRの探索時のリダクション。
       */
      LispObjectPtr SetLMRSearchReduction(const LispObject& reduction) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->lmr_search_reduction());

        if (reduction.IsNumber()) {
          search_params_ptr_->lmr_search_reduction(reduction.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - Futility Pruningの有効無効。
       * @param enable Futility Pruningの有効無効。
       * @return セットされていたFutility Pruningの有効無効。
       */
      LispObjectPtr SetEnableFutilityPruning(const LispObject& enable) {
        LispObjectPtr ret_ptr = LispObject::NewBoolean
        (search_params_ptr_->enable_futility_pruning());

        if (enable.IsBoolean()) {
          search_params_ptr_->enable_futility_pruning(enable.boolean_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - Futility Pruningを実行する深さ。
       * @param depth 深さ。
       * @return セットされていたFutility Pruningを実行する深さ。
       */
      LispObjectPtr SetFutilityPruningDepth(const LispObject& depth) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->futility_pruning_depth());

        if (depth.IsNumber()) {
          search_params_ptr_->futility_pruning_depth(depth.number_value());
        }

        return ret_ptr;
      }

      /**
       * SearchParams - Futility Pruningのマージン。
       * @param margin マージン。
       * @return セットされていたFutility Pruningのマージン。
       */
      LispObjectPtr SetFutilityPruningMargin(const LispObject& margin) {
        LispObjectPtr ret_ptr = LispObject::NewNumber
        (search_params_ptr_->futility_pruning_margin());

        if (margin.IsNumber()) {
          search_params_ptr_->futility_pruning_margin(margin.number_value());
        }

        return ret_ptr;
      }

      /**
       * EvalParams - ポーンのオープニング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetPawnSquareTableOpening(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - ナイトのオープニング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetKnightSquareTableOpening(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - ビショップのオープニング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetBishopSquareTableOpening(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - ルークのオープニング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetRookSquareTableOpening(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - クイーンのオープニング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetQueenSquareTableOpening(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - キングのオープニング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetKingSquareTableOpening(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - ポーンのエンディング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetPawnSquareTableEnding(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - ナイトのエンディング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetKnightSquareTableEnding(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - ビショップのエンディング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetBishopSquareTableEnding(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - ルークのエンディング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetRookSquareTableEnding(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - クイーンのエンディング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetQueenSquareTableEnding(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - キングのエンディング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetKingSquareTableEnding(const std::string& func_name,
      const LispObject& square_list);

      /**
       * EvalParams - 攻撃の価値テーブル。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param value_list テーブル。
       * @return セットされていたテーブル。
       */
      template<PieceType TYPE>
      LispObjectPtr SetAttackValueTable(const std::string& func_name,
      const std::string& symbol_name, const LispObject& value_list) {
        const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
        eval_params_ptr_->attack_value_table();
        LispObjectPtr ret_ptr = LispObject::NewList(NUM_PIECE_TYPES);
        LispObject* ptr = ret_ptr.get();
        FOR_PIECE_TYPES(piece_type) {
          ptr->car(LispObject::NewNumber(table[TYPE][piece_type]));

          ptr = ptr->cdr().get();
        }

        if (value_list.IsList() && !(value_list.IsNil())) {
          unsigned int len = value_list.Length();
          if (len < NUM_PIECE_TYPES) {
            throw LispObject::GenError("@engine_error",
            symbol_name + " needs List of "
            + std::to_string(NUM_PIECE_TYPES) + " parameters. Given "
            + std::to_string(len) + ".");
          }

          double temp[NUM_PIECE_TYPES][NUM_PIECE_TYPES];
          COPY_ARRAY(temp, table);
          LispIterator itr(&value_list);
          FOR_PIECE_TYPES(piece_type) {
            if (!(itr->IsNumber())) {
              throw LispObject::GenWrongTypeError(func_name, "Number",
              std::vector<int> {2, static_cast<int>(piece_type + 1)}, false);
            }
            if (piece_type == EMPTY) {
              temp[TYPE][piece_type] = 0.0;
            } else {
              temp[TYPE][piece_type] = itr->number_value();
            }
            ++itr;
          }

          eval_params_ptr_->attack_value_table(temp);
        }

        return ret_ptr;
      }

      /**
       * EvalParams - 攻撃のウェイト。
       * @param func_name 関数名。
       * @param weight_params ウェイトのパラメータリスト。
       * @return セットされていたウェイト。
       */
      template<PieceType TYPE>
      LispObjectPtr SetWeightAttack(const std::string& func_name,
      const LispObject& weight_params) {
        const Weight& weight = eval_params_ptr_->weight_attack()[TYPE];

        LispObjectPtr ret_ptr = LispObject::NewList(2);
        ret_ptr->car(LispObject::NewNumber(weight.opening_weight()));
        ret_ptr->cdr()->car(LispObject::NewNumber(weight.opening_weight()));

        if (weight_params.IsList() && !(weight_params.IsNil())) {
          int len = weight_params.Length();
          if (len < 2) {
            throw LispObject::GenError("@engine_error",
            "Weight needs 2 parameters. Given " + std::to_string(len) + ".");
          }

          if (!(weight_params.car()->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {2, 1}, false);
          }
          if (!(weight_params.cdr()->car()->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {2, 2}, false);
          }

          Weight temp[NUM_PIECE_TYPES];
          FOR_PIECE_TYPES(piece_type) {
            temp[piece_type] = eval_params_ptr_->weight_attack()[piece_type];
          }
          temp[TYPE].opening_weight(weight_params.car()->number_value());
          temp[TYPE].ending_weight(weight_params.cdr()->car()->number_value());
        }

        return ret_ptr;
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
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * UCIのアウトプットリスナー。
       * @param message アウトプット。
       */
      void ListenUCIOutput(const std::string& message) {
        for (auto& callback : callback_vec_) {
          callback(message);
        }
      }

      /**
       * 最善手を得る。
       * @param depth 探索する深さ。
       * @param nodes 探索するノード数。
       * @param thinking_time 思考時間。 (ミリ秒)
       * @param move_vec 探索したい候補手。
       * @retrun 最善手のリスト。
       */
      LispObjectPtr GetBestMove(std::uint32_t depth, std::uint64_t nodes,
      int thinking_time, const std::vector<Move>& move_vec);

      /**
       * 手のリストから手のベクトルを作る。
       * @param func_name 関数名。
       * @param move_list 手のリスト。
       * @return 手のベクトル。
       */
      static std::vector<Move> MoveListToVec(const std::string& func_name,
      const LispObject& move_list);

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
      /** UCIのアウトプットリスナー。 */
      std::vector<std::function<void(const std::string&)>> callback_vec_;
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
       * @return 終了ステータス。
       */
      int Run(std::istream* stream_ptr);

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
