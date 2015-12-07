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

        return Lisp::GenError("@not-square", message);
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

        return Lisp::GenError("@not-piece-type", message);
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

        return Lisp::GenError("@not-fyle", message);
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

        return Lisp::GenError("@not-rank", message);
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

        return Lisp::GenError("@not-side", message);
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
        return Lisp::GenError("@not-castling", message);
      }

      /**
       * Moveをリストに変換する。
       * @param move 変換したいMove。
       * @return 変換結果のリスト。
       */
      static LispObjectPtr MoveToList(Move move) {
        LispObjectPtr ret_ptr = Lisp::NewList(3);

        ret_ptr->car(Lisp::NewSymbol(SQUARE_SYMBOL[Get<FROM>(move)]));

        ret_ptr->cdr()->car
        (Lisp::NewSymbol(SQUARE_SYMBOL[Get<TO>(move)]));

        ret_ptr->cdr()->cdr()->car
        (Lisp::NewSymbol(PIECE_TYPE_SYMBOL[Get<PROMOTION>(move)]));

        return ret_ptr;
      }

      // ========================== //
      // Lisp関数オブジェクト用関数 //
      // ========================== //
      // --- エンジンの状態にアクセス --- //
      /**
       * 駒の配置を得る。
       * @return 戻り値のオブジェクト。
       */
      template<Side SIDE, PieceType TYPE>
      LispObjectPtr GetPosition() const {
        Bitboard bb = 0;
        if (TYPE == EMPTY) {
          bb = ~(engine_ptr_->blocker()[R0]);
        } else {
          bb = engine_ptr_->position()[SIDE][TYPE];
        }

        LispObjectPtr ret_ptr = Lisp::NewList(Util::CountBits(bb));
        LispObject* ptr = ret_ptr.get();
        for (; bb; NEXT_BITBOARD(bb)) {
          ptr->car(Lisp::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]));
          ptr = ptr->cdr().get();
        }

        return ret_ptr;
      }

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
       * @param square 駒を置くマス。
       * @param side 駒のサイド。
       * @param piece_type 駒のタイプ。
       * @return 元の位置にあった駒の種類とサイドのリスト。
       */
      LispObjectPtr PlacePiece(Square square, Side side, PieceType piece_type);

      /**
       * 手番をセットする。
       * @param to_move 手番。
       * @return 変更前の値。
       */
      LispObjectPtr SetToMove(Side to_move);
      /**
       * キャスリングの権利をセットする。
       * @param caller 呼び出し元。
       * @param castling_rights_ptr キャスリングの権利のリスト。
       * @param func_name 例外で使う関数名。
       * @return 変更前の値。
       */
      LispObjectPtr SetCastlingRights(const LispObject& caller,
      LispObjectPtr castling_rights_ptr, const std::string& func_name);
      /**
       * アンパッサンの位置をセットする。
       * @param caller 呼び出し元。
       * @param en_passant_square_ptr アンパッサンの位置。
       * @return 変更前の値。
       */
      LispObjectPtr SetEnPassantSquare(const LispObject& caller,
      LispObjectPtr castling_rights_ptr);
      /**
       * 手数をセットする。
       * @param ply_ptr 手数。
       * @return 変更前の値。
       */
      LispObjectPtr SetPly(LispObjectPtr ply_ptr) {
        int ply = ply_ptr->number_value();
        if (ply < 1) {
          throw Lisp::GenError("@engine-error",
          "Minimum ply number is '1'. Given '"
          + DoubleToString(ply_ptr->number_value()) + "'.");
        }

        int origin = engine_ptr_->ply();
        engine_ptr_->ply(ply);
        return Lisp::NewNumber(origin);
      }
      /**
       * 50手ルールの手数をセットする。
       * @param clock_ptr 50手ルールの手数。
       * @return 変更前の値。
       */
      LispObjectPtr SetClock(LispObjectPtr clock_ptr) {
        int clock = clock_ptr->number_value();
        if (clock < 0) {
          throw Lisp::GenError("@engine-error",
          "Minimum clock number is '0'. Given '"
          + DoubleToString(clock_ptr->number_value()) + "'.");
        }

        int origin = engine_ptr_->clock();
        engine_ptr_->clock(clock);
        return Lisp::NewNumber(origin);
      }
      /**
       * 正しい駒の配置かどうか。
       * @return 正しければ#t。
       */
      LispObjectPtr IsCorrectPosition() const {
        return Lisp::NewBoolean(engine_ptr_->IsCorrectPosition());
      }
      /**
       * 白キングがチェックされているかどうか。
       * @return チェックされていれば#t。
       */
      LispObjectPtr IsWhiteChecked() const {
        return Lisp::NewBoolean
        (engine_ptr_->IsAttacked(engine_ptr_->king()[WHITE], BLACK));
      }
      /**
       * 黒キングがチェックされているかどうか。
       * @return チェックされていれば#t。
       */
      LispObjectPtr IsBlackChecked() const {
        return Lisp::NewBoolean
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
          if (move.size() == 0) return Lisp::NewBoolean(true);
        }
        return Lisp::NewBoolean(false);
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
          if (move.size() == 0) return Lisp::NewBoolean(true);
        }
        return Lisp::NewBoolean(false);
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
       * UCIエンジンとして実行する。
       * @return #t。
       */
      LispObjectPtr RunEngine();

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

        return Lisp::NewNumber(old_size);
      }

      /**
       * スレッドの数を変更する。
       * @param hash_size スレッドの数。
       * @return 変更前のスレッドの数。
       */
      LispObjectPtr SetThreads(const LispObject& num_threads) {
        int old_threads = shell_ptr_->num_threads();

        shell_ptr_->num_threads(num_threads.number_value());

        return Lisp::NewNumber(old_threads);
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
        Lisp::NewBoolean(search_params_ptr_->enable_quiesce_search());

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
        Lisp::NewBoolean(search_params_ptr_->enable_repetition_check());

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
        Lisp::NewBoolean(search_params_ptr_->enable_check_extension());

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
        Lisp::NewNumber(search_params_ptr_->ybwc_limit_depth());

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
        Lisp::NewNumber(search_params_ptr_->ybwc_invalid_moves());

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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewBoolean
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
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
        LispObjectPtr ret_ptr = Lisp::NewNumber
        (search_params_ptr_->futility_pruning_margin());

        if (margin.IsNumber()) {
          search_params_ptr_->futility_pruning_margin(margin.number_value());
        }

        return ret_ptr;
      }

      /**
       * EvalParams - ポジションの価値テーブル。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      template<PieceType TYPE>
      LispObjectPtr SetPieceSquareTableOpening(const std::string& func_name,
      const std::string& symbol_name, const LispObject& square_list) {
        // 先ず返すリストを作る。
        LispObjectPtr ret_ptr = Lisp::NewList(64);
        LispIterator<false> itr {ret_ptr.get()};
        FOR_SQUARES(square) {
          itr->type(LispObjectType::NUMBER);
          itr->number_value
          (eval_params_ptr_->opening_position_value_table()[TYPE][square]);

          ++itr;
        }

        // セットする。
        if (!(square_list.IsNil())) {
          if (square_list.Length() != 64) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs List of 64 parameters. Given "
            + std::to_string(square_list.Length()) + ".");
          }
          LispIterator<false> list_itr {&square_list};
          FOR_SQUARES(square) {
            if (!(list_itr->IsNumber())) {
              throw Lisp::GenWrongTypeError(func_name, "Number",
              std::vector<int> {2, static_cast<int>(square + 1)}, false);
            }
            eval_params_ptr_->opening_position_value_table
            (TYPE, square, list_itr->number_value());

            ++list_itr;
          }
        }

        return ret_ptr;
      }

      /**
       * EvalParams - エンディング時のポジションの価値テーブル。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      template<PieceType TYPE>
      LispObjectPtr SetPieceSquareTableEnding(const std::string& func_name,
      const std::string& symbol_name, const LispObject& square_list) {
        // 先ず返すリストを作る。
        LispObjectPtr ret_ptr = Lisp::NewList(64);
        LispIterator<false> itr {ret_ptr.get()};
        FOR_SQUARES(square) {
          itr->type(LispObjectType::NUMBER);
          itr->number_value
          (eval_params_ptr_->ending_position_value_table()[TYPE][square]);

          ++itr;
        }

        // セットする。
        if (!(square_list.IsNil())) {
          if (square_list.Length() != 64) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs List of 64 parameters. Given "
            + std::to_string(square_list.Length()) + ".");
          }

          // 値を変更。
          LispIterator<false> list_itr {&square_list};
          FOR_SQUARES(square) {
            if (!(list_itr->IsNumber())) {
              throw Lisp::GenWrongTypeError(func_name, "Number",
              std::vector<int> {2, static_cast<int>(square + 1)}, false);
            }
            eval_params_ptr_->ending_position_value_table
            (TYPE, square, list_itr->number_value());

            ++list_itr;
          }
        }

        return ret_ptr;
      }

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
        LispObjectPtr ret_ptr = Lisp::NewList(NUM_PIECE_TYPES);
        LispObject* ptr = ret_ptr.get();
        FOR_PIECE_TYPES(piece_type) {
          ptr->car(Lisp::NewNumber
          (eval_params_ptr_->attack_value_table()[TYPE][piece_type]));

          ptr = ptr->cdr().get();
        }

        if (value_list.IsList() && !(value_list.IsNil())) {
          unsigned int len = value_list.Length();
          if (len < NUM_PIECE_TYPES) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs List of "
            + std::to_string(NUM_PIECE_TYPES) + " parameters. Given "
            + std::to_string(len) + ".");
          }

          LispIterator<false> itr {&value_list};
          FOR_PIECE_TYPES(piece_type) {
            if (!(itr->IsNumber())) {
              throw Lisp::GenWrongTypeError(func_name, "Number",
              std::vector<int> {2, static_cast<int>(piece_type + 1)}, false);
            }
            if (piece_type == EMPTY) {
              eval_params_ptr_->attack_value_table(TYPE, piece_type, 0.0);
            } else {
              eval_params_ptr_->attack_value_table
              (TYPE, piece_type, itr->number_value());
            }
            ++itr;
          }
        }

        return ret_ptr;
      }

      /**
       * EvalParams - 防御の価値テーブル。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param value_list テーブル。
       * @return セットされていたテーブル。
       */
      template<PieceType TYPE>
      LispObjectPtr SetDefenseValueTable(const std::string& func_name,
      const std::string& symbol_name, const LispObject& value_list) {
        LispObjectPtr ret_ptr = Lisp::NewList(NUM_PIECE_TYPES);
        LispObject* ptr = ret_ptr.get();
        FOR_PIECE_TYPES(piece_type) {
          ptr->car(Lisp::NewNumber
          (eval_params_ptr_->defense_value_table()[TYPE][piece_type]));

          ptr = ptr->cdr().get();
        }

        if (value_list.IsList() && !(value_list.IsNil())) {
          unsigned int len = value_list.Length();
          if (len < NUM_PIECE_TYPES) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs List of "
            + std::to_string(NUM_PIECE_TYPES) + " parameters. Given "
            + std::to_string(len) + ".");
          }

          LispIterator<false> itr {&value_list};
          FOR_PIECE_TYPES(piece_type) {
            if (!(itr->IsNumber())) {
              throw Lisp::GenWrongTypeError(func_name, "Number",
              std::vector<int> {2, static_cast<int>(piece_type + 1)}, false);
            }
            if (piece_type == EMPTY) {
              eval_params_ptr_->defense_value_table(TYPE, piece_type, 0.0);
            } else {
              eval_params_ptr_->defense_value_table
              (TYPE, piece_type, itr->number_value());
            }
            ++itr;
          }
        }

        return ret_ptr;
      }

      /**
       * EvalParams - ピンの価値テーブル。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param value_list テーブル。
       * @return セットされていたテーブル。
       */
      template<PieceType TYPE>
      LispObjectPtr SetPinValueTable(const std::string& func_name,
      const std::string& symbol_name, const LispObject& value_list) {
        LispObjectPtr ret_ptr = Lisp::NewList(NUM_PIECE_TYPES);
        LispObject* ptr = ret_ptr.get();
        FOR_PIECE_TYPES(piece_type_1) {
          ptr->car(Lisp::NewList(NUM_PIECE_TYPES));

          LispObject* ptr_2 = ptr->car().get();
          FOR_PIECE_TYPES(piece_type_2) {
            if ((piece_type_1 == EMPTY) || (piece_type_2 == EMPTY)) {
              ptr_2->car(Lisp::NewNumber(0));
            } else {
              ptr_2->car(Lisp::NewNumber
              (eval_params_ptr_->pin_value_table()
              [TYPE][piece_type_1][piece_type_2]));
            }

            ptr_2 = ptr_2->cdr().get();
          }

          ptr = ptr->cdr().get();
        }

        // セットする。
        if (value_list.IsList() && !(value_list.IsNil())) {
          unsigned int len = value_list.Length();
          if (len < NUM_PIECE_TYPES) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs List of "
            + std::to_string(NUM_PIECE_TYPES) + " parameters. Given "
            + std::to_string(len) + ".");
          }

          // ループ 1。
          LispIterator<false> itr_1 {&value_list};
          FOR_PIECE_TYPES(piece_type_1) {
            if (!(itr_1->IsList())) {
              throw Lisp::GenWrongTypeError(func_name, "List",
              std::vector<int> {2, static_cast<int>(piece_type_1 + 1)}, false);
            }

            unsigned int len = itr_1->Length();
            if (len < NUM_PIECE_TYPES) {
              throw Lisp::GenError("@engine-error",
              "The " + std::to_string(piece_type_1 + 1) + "th parameter of "
              + symbol_name + " needs List of "
              + std::to_string(NUM_PIECE_TYPES) + " parameters. Given "
              + std::to_string(len) + ".");
            }

            // ループ 2。
            LispIterator<false> itr_2 {&(*itr_1)};
            FOR_PIECE_TYPES(piece_type_2) {
              if (!(itr_2->IsNumber())) {
                throw Lisp::GenWrongTypeError(func_name, "Number",
                std::vector<int> {2, static_cast<int>(piece_type_1 + 1),
                static_cast<int>(piece_type_2 + 1)}, false);
              }

              if ((piece_type_1 == EMPTY) || (piece_type_2 == EMPTY)) {
                eval_params_ptr_->pin_value_table
                (TYPE, piece_type_1, piece_type_2, 0.0);
              } else {
                eval_params_ptr_->pin_value_table
                (TYPE, piece_type_1, piece_type_2, itr_2->number_value());
              }

              ++itr_2;
            }

            ++itr_1;
          }
        }

        return ret_ptr;
      }

      /**
       * EvalParams - ポーンシールドの価値テーブル。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param square_list テーブル。
       * @return セットされていたテーブル。
       */
      LispObjectPtr SetPawnShieldValueTable(const std::string& func_name,
      const std::string& symbol_name, const LispObject& square_list) {
        // 先ず返すリストを作る。
        LispObjectPtr ret_ptr = Lisp::NewList(64);
        LispIterator<false> itr {ret_ptr.get()};
        FOR_SQUARES(square) {
          itr->type(LispObjectType::NUMBER);
          itr->number_value
          (eval_params_ptr_->pawn_shield_value_table()[square]);

          ++itr;
        }

        // セットする。
        if (!(square_list.IsNil())) {
          if (square_list.Length() != 64) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs List of 64 parameters. Given "
            + std::to_string(square_list.Length()) + ".");
          }

          // 値を変更。
          LispIterator<false> list_itr {&square_list};
          FOR_SQUARES(square) {
            if (!(list_itr->IsNumber())) {
              throw Lisp::GenWrongTypeError(func_name, "Number",
              std::vector<int> {2, static_cast<int>(square + 1)}, false);
            }

            eval_params_ptr_->pawn_shield_value_table
            (square, list_itr->number_value());

            ++list_itr;
          }
        }

        return ret_ptr;
      }

      /**
       * EvalParams - タイプ1のウェイト。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param weight_params ウェイトのパラメータリスト。
       * @return セットされていたウェイト。
       */
      template<int WEIGHT_TYPE, PieceType PIECE_TYPE>
      LispObjectPtr SetWeight1(const std::string& func_name,
      const std::string& symbol_name, const LispObject& weight_params) {
        const Weight& weight = weight_1_accessor_[WEIGHT_TYPE]()[PIECE_TYPE];

        LispObjectPtr ret_ptr = Lisp::NewList(2);
        ret_ptr->car(Lisp::NewNumber(weight.opening_weight()));
        ret_ptr->cdr()->car(Lisp::NewNumber(weight.ending_weight()));

        if (weight_params.IsList() && !(weight_params.IsNil())) {
          int len = weight_params.Length();
          if (len < 2) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs 2 parameters. Given " + std::to_string(len)
            + ".");
          }

          if (!(weight_params.car()->IsNumber())) {
            throw Lisp::GenWrongTypeError
            (func_name, "Number", std::vector<int> {2, 1}, false);
          }
          if (!(weight_params.cdr()->car()->IsNumber())) {
            throw Lisp::GenWrongTypeError
            (func_name, "Number", std::vector<int> {2, 2}, false);
          }

          weight_1_mutator_[WEIGHT_TYPE](PIECE_TYPE,
          weight_params.car()->number_value(),
          weight_params.cdr()->car()->number_value());
        }

        return ret_ptr;
      }

      /**
       * EvalParams - タイプ2のウェイト。
       * @param func_name 関数名。
       * @param symbol_name シンボル名。
       * @param weight_params ウェイトのパラメータリスト。
       * @return セットされていたウェイト。
       */
      template<int WEIGHT_TYPE>
      LispObjectPtr SetWeight2(const std::string& func_name,
      const std::string& symbol_name, const LispObject& weight_params) {
        const Weight& weight = weight_2_accessor_[WEIGHT_TYPE]();

        LispObjectPtr ret_ptr = Lisp::NewList(2);
        ret_ptr->car(Lisp::NewNumber(weight.opening_weight()));
        ret_ptr->cdr()->car(Lisp::NewNumber(weight.ending_weight()));

        if (weight_params.IsList() && !(weight_params.IsNil())) {
          int len = weight_params.Length();
          if (len < 2) {
            throw Lisp::GenError("@engine-error",
            symbol_name + " needs 2 parameters. Given " + std::to_string(len)
            + ".");
          }

          if (!(weight_params.car()->IsNumber())) {
            throw Lisp::GenWrongTypeError
            (func_name, "Number", std::vector<int> {2, 1}, false);
          }
          if (!(weight_params.cdr()->car()->IsNumber())) {
            throw Lisp::GenWrongTypeError
            (func_name, "Number", std::vector<int> {2, 2}, false);
          }

          weight_2_mutator_[WEIGHT_TYPE](weight_params.car()->number_value(),
          weight_params.cdr()->car()->number_value());
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
       * アクセサ - トランスポジションテーブル。
       * @return トランスポジションテーブル。
       */
      const TranspositionTable& table() const {return *table_ptr_;}
      /**
       * アクセサ - チェスエンジン。
       * @return チェスエンジン。
       */
      const ChessEngine& engine() const {return *engine_ptr_;}
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

      /** 
       * LispObjectのシンボルから数字を得る。
       * @param caller 呼び出し元。
       * @param obj 数字を得たいオブジェクト。
       * @return 数字。
       */
      static int ToInt(const LispObject& caller, const LispObject& obj) {
        if (obj.IsSymbol()) {
          LispObjectPtr result = caller.Evaluate(obj);
          return result->number_value();
        }
        return obj.number_value();
      }

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 探索関数用パラメータのポインタ。 */
      std::unique_ptr<SearchParams> search_params_ptr_;
      /** 評価関数用パラメータのポインタ。 */
      std::unique_ptr<EvalParams> eval_params_ptr_;
      /** トランスポジションテーブルのポインタ。 */
      std::unique_ptr<TranspositionTable> table_ptr_;
      /** チェスエンジンのポインタ。 */
      std::unique_ptr<ChessEngine> engine_ptr_;
      /** UCIShellのポインタ。 */
      std::unique_ptr<UCIShell> shell_ptr_;
      /** UCIのアウトプットリスナー。 */
      std::vector<std::function<void(const std::string&)>> callback_vec_;
      /** ウェイト用定数。 その1。 */
      enum {
        /** ウェイト用定数 - オープニング時のポジション。 */
        WEIGHT_OPENING_POSITION,
        /** ウェイト用定数 - エンディング時のポジション。 */
        WEIGHT_ENDING_POSITION,
        /** ウェイト用定数 - 機動力。 */
        WEIGHT_MOBILITY,
        /** ウェイト用定数 - センターコントロール。 */
        WEIGHT_CENTER_CONTROL,
        /** ウェイト用定数 - スウィートセンターコントロール。 */
        WEIGHT_SWEET_CENTER_CONTROL,
        /** ウェイト用定数 - 駒の展開。 */
        WEIGHT_DEVELOPMENT,
        /** ウェイト用定数 - 攻撃。 */
        WEIGHT_ATTACK,
        /** ウェイト用定数 - 防御。 */
        WEIGHT_DEFENSE,
        /** ウェイト用定数 - ピン。 */
        WEIGHT_PIN,
        /** ウェイト用定数 - キング周辺への攻撃。 */
        WEIGHT_ATTACK_AROUND_KING
      };
      /** ウェイト用定数。 その2。 */
      enum {
        /** ウェイト用定数 - パスポーン。 */
        WEIGHT_PASS_PAWN,
        /** ウェイト用定数 - 守られたパスポーン。 */
        WEIGHT_PROTECTED_PASS_PAWN,
        /** ウェイト用定数 - ダブルポーン。 */
        WEIGHT_DOUBLE_PAWN,
        /** ウェイト用定数 - 孤立ポーン。 */
        WEIGHT_ISO_PAWN,
        /** ウェイト用定数 - ポーンシールド。 */
        WEIGHT_PAWN_SHIELD,
        /** ウェイト用定数 - ビショップペア。 */
        WEIGHT_BISHOP_PAIR,
        /** ウェイト用定数 - バッドビショップ。 */
        WEIGHT_BAD_BISHOP,
        /** ウェイト用定数 - ルークペア。 */
        WEIGHT_ROOK_PAIR,
        /** ウェイト用定数 - セミオープンファイルのルーク。 */
        WEIGHT_ROOK_SEMIOPEN_FYLE,
        /** ウェイト用定数 - オープンファイルのルーク。 */
        WEIGHT_ROOK_OPEN_FYLE,
        /** ウェイト用定数 - 早すぎるクイーンの始動。 */
        WEIGHT_EARLY_QUEEN_STARTING,
        /** ウェイト用定数 - キング周りの弱いマス。 */
        WEIGHT_WEAK_SQUARE,
        /** ウェイト用定数 - キャスリング。 */
        WEIGHT_CASTLING,
        /** ウェイト用定数 - キャスリングの放棄。 */
        WEIGHT_ABANDONED_CASTLING
      };
      /** ウェイトアクセサオブジェクト。 その1。 */
      std::function<const Weight (&())[NUM_PIECE_TYPES]>
      weight_1_accessor_[WEIGHT_ATTACK_AROUND_KING + 1];
      /** ウェイトアクセサオブジェクト。 その2。 */
      std::function<const Weight&()>
      weight_2_accessor_[WEIGHT_ABANDONED_CASTLING + 1];
      /** ウェイトミューテータオブジェクト。 その1。 */
      std::function<void(PieceType, double, double)>
      weight_1_mutator_[WEIGHT_ATTACK_AROUND_KING + 1];
      /** ウェイトミューテータオブジェクト。 その2。 */
      std::function<void(double, double)>
      weight_2_mutator_[WEIGHT_ABANDONED_CASTLING + 1];
      /** ウェイト関数オブジェクトをセットする。 */
      void SetWeightFunctions();
  };

  /** Sayulisp実行クラス。 */
  class Sayulisp : public Lisp {
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
      Sayulisp(const Sayulisp& sayulisp) : Lisp(sayulisp) {}
      /**
       * ムーブコンストラクタ。
       * @param sayulisp ムーブ元。
       */
      Sayulisp(Sayulisp&& sayulisp) : Lisp(sayulisp) {}
      /**
       * コピー代入演算子。
       * @param sayulisp コピー元。
       */
      Sayulisp& operator=(const Sayulisp& sayulisp) {
        Lisp::operator=(sayulisp);
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param sayulisp ムーブ元。
       */
      Sayulisp& operator=(Sayulisp&& sayulisp) {
        Lisp::operator=(sayulisp);
        return *this;
      }
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

      /**
       * マスのシンボルを数値に変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr SquareToNumber(const LispObject& obj);

      /**
       * ファイルのシンボルを数値に変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr FyleToNumber(const LispObject& obj);

      /**
       * ランクのシンボルを数値に変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr RankToNumber(const LispObject& obj);

      /**
       * サイドのシンボルを数値に変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr SideToNumber(const LispObject& obj);

      /**
       * 駒の種類のシンボルを数値に変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr PieceTypeToNumber(const LispObject& obj);

      /**
       * キャスリングのシンボルを数値に変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr CastlingToNumber(const LispObject& obj);

      /**
       * 数値をマスのシンボルに変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr NumberToSquare(const LispObject& obj);

      /**
       * 数値をファイルのシンボルに変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr NumberToFyle(const LispObject& obj);

      /**
       * 数値をランクのシンボルに変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr NumberToRank(const LispObject& obj);

      /**
       * 数値をサイドのシンボルに変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr NumberToSide(const LispObject& obj);

      /**
       * 数値を駒の種類のシンボルに変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr NumberToPiece(const LispObject& obj);

      /**
       * 数値をキャスリングのシンボルに変換する。
       * @param obj 変換したいオブジェクト。
       * @return 変換後のオブジェクト。
       */
      LispObjectPtr NumberToCastling(const LispObject& obj);

      /**
       * PGNオブジェクトを作成する。
       * @param pgn_str PGN文字列。
       * @param caller_scope 呼び出し元のスコープチェイン。。
       * @return PGNオブジェクト。
       */
      LispObjectPtr GenPGN(const std::string& pgn_str,
      const ScopeChain& caller_scope);
  };
}  // namespace Sayuri

#endif
