/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
#include <map>
#include <set>
#include "common.h"
#include "params.h"
#include "chess_engine.h"
#include "board.h"
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
  class EngineSuite;

  /** Sayulisp実行クラス。 */
  class Sayulisp : public Lisp {
    public:
      /**
       * メッセージシンボル関数型。
       * LC_Functionにシンボル名の引数を追加したもの。
       */
      using MessageFunction = std::function
      <LPointer(const std::string&, LPointer, LObject*, const LObject&)>;

      /** メッセージシンボル関数宣言マクロ。 */
#define DEF_MESSAGE_FUNCTION(func_name) \
      LPointer func_name(const std::string& symbol, LPointer self, \
      LObject* caller, const LObject& args)

      /** メッセージシンボル関数オブジェクト登録マクロ。 */
#define INSERT_MESSAGE_FUNCTION(func_name) \
    [this](const std::string& symbol, LPointer self, LObject* caller, \
    const LObject& args) -> LPointer {\
      return this->func_name(symbol, self, caller, args);\
    }

      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param argv コマンド引数。
       */
      Sayulisp(const std::vector<std::string>& argv) : Lisp(argv) {
        func_id_ = "Sayulisp";
        SetSayulispFunction();
      }
      /** コンストラクタ。 */
      Sayulisp() {
        func_id_ = "Sayulisp";
        SetSayulispFunction();
      }
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
      // パブリック定数 //
      // ============== //
      /** マスの定数のマップ。 */
      static const std::map<std::string, Square> SQUARE_MAP;
      /** ファイルの定数のマップ。 */
      static const std::map<std::string, Fyle> FYLE_MAP;
      /** ランクの定数のマップ。 */
      static const std::map<std::string, Rank> RANK_MAP;
      /** サイドの定数のマップ。 */
      static const std::map<std::string, Side> SIDE_MAP;
      /** 駒の種類の定数のマップ。 */
      static const std::map<std::string, PieceType> PIECE_MAP;
      /** キャスリングの定数のマップ。 */
      static const std::map<std::string, int> CASTLING_MAP;
      /** マスの定数の逆マップ。 */
      static const std::string SQUARE_MAP_INV[NUM_SQUARES];
      /** ファイルの定数の逆マップ。 */
      static const std::string FYLE_MAP_INV[NUM_FYLES];
      /** ランクの定数の逆マップ。 */
      static const std::string RANK_MAP_INV[NUM_RANKS];
      /** サイドの定数の逆マップ。 */
      static const std::string SIDE_MAP_INV[NUM_SIDES];
      /** 駒の種類の定数の逆マップ。 */
      static const std::string PIECE_MAP_INV[NUM_PIECE_TYPES];
      /** キャスリングの定数の逆マップ。 */
      static const std::string CASTLING_MAP_INV[5];

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * Sayulispの関数をセットする。
       */
      void SetSayulispFunction();

      /**
       * Sayulispを開始する。
       * @param stream_ptr 入力に使うストリームのポインタ。
       * @return 終了ステータス。
       */
      int Run(std::istream* stream_ptr);

      /**
       * 指し手をリストに変換する。
       */
      static LPointer MoveToList(Move move) {
        LPointer ret_ptr = NewList(3);

        ret_ptr->car(NewSymbol(SQUARE_MAP_INV[Get<FROM>(move)]));
        ret_ptr->cdr()->car(NewSymbol(SQUARE_MAP_INV[Get<TO>(move)]));
        ret_ptr->cdr()->cdr()->car
        (NewSymbol(PIECE_MAP_INV[Get<PROMOTION>(move)]));

        return ret_ptr;
      }

      /**
       * 指し手のリストを指し手に変換する。
       * @param obj 指し手のリスト。
       * @return 変換後の指し手。
       */
      static Move ListToMove(const LObject& obj) {
        CheckMove(obj);

        Move move = 0;
        Sayuri::Set<FROM>(move, obj.car()->number());
        Sayuri::Set<TO>(move, obj.cdr()->car()->number());
        Sayuri::Set<PROMOTION>(move, obj.cdr()->cdr()->car()->number());

        return move;
      }

      /**
       * マスを表しているかどうかをチェックする。
       * @param obj チェックするマスのオブジェクト。
       */
      static void CheckSquare(const LObject& obj) {
        CheckType(obj, LType::NUMBER);
        Square square = obj.number();

        if (square >= NUM_SQUARES) {
          throw GenError("@not-square", "'" + std::to_string(square)
          + "' doesn't indicate any square. Square is from '0' to '63'.");
        }
      }
      /**
       * 駒の種類を表しているかどうかをチェックする。
       * @param obj チェックする駒の種類のオブジェクト。
       */
      static void CheckPieceType(const LObject& obj) {
        CheckType(obj, LType::NUMBER);
        PieceType piece_type = obj.number();

        if (piece_type >= NUM_PIECE_TYPES) {
          throw GenError("@not-piece-type", "'" + std::to_string(piece_type)
          + "' doesn't indicate any piece type. "
          "Pieced type is from '0' to '6'.");
        }
      }
      /**
       * 駒を表しているかどうかをチェックする。
       * @param obj チェックする駒のオブジェクト。
       */
      static void CheckPiece(const LObject& obj) {
        // リストをチェック。
        CheckList(obj);
        if (CountList(obj) < 2) {
          throw GenError("@not-piece", "'" + obj.ToString()
          + "' doesn't indicate any piece. "
          "Format of piece must be `(<Side> <Piece type>)`.");
        }

        // サイドと駒の種類チェック。
        CheckSide(*(obj.car()));
        CheckPieceType(*(obj.cdr()->car()));
      }
      /**
       * ファイルを表しているかどうかをチェックする。
       * @param obj チェックするファイルのオブジェクト。
       */
      static void CheckFyle(const LObject& obj) {
        CheckType(obj, LType::NUMBER);
        Fyle fyle = obj.number();

        if (fyle >= NUM_FYLES) {
          throw GenError("@not-fyle", "'" + std::to_string(fyle)
          + "' doesn't indicate any fyle. Fyle is from '0' to '7'.");
        }
      }
      /**
       * ランクを表しているかどうかをチェックする。
       * @param obj チェックするランクのオブジェクト。
       */
      static void CheckRank(const LObject& obj) {
        CheckType(obj, LType::NUMBER);
        Rank rank = obj.number();

        if (rank >= NUM_RANKS) {
          throw GenError("@not-rank", "'" + std::to_string(rank)
          + "' doesn't indicate any rank. Rank is from '0' to '7'.");
        }
      }
      /**
       * サイドを表しているかどうかをチェックする。
       * @param obj チェックするサイドのオブジェクト。
       */
      static void CheckSide(const LObject& obj) {
        CheckType(obj, LType::NUMBER);
        Side side = obj.number();

        if (side >= NUM_SIDES) {
          throw GenError("@not-side", "'" + std::to_string(side)
          + "' doesn't indicate any side. Side is from '0' to '2'.");
        }
      }
      /**
       * キャスリングを表しているかどうかをチェックする。
       * @param obj チェックするキャスリングのオブジェクト。
       */
      static void CheckCastling(const LObject& obj) {
        CheckType(obj, LType::NUMBER);
        int castling = obj.number();

        if (castling >= 5) {
          throw GenError("@not-castling", "'" + std::to_string(castling)
          + "' doesn't indicate any castling right. "
          "Castling right is from '0' to '4'.");
        }
      }
      /**
       * 指し手を表しているかどうかをチェックする。
       * @param obj チェックする指し手のオブジェクト。
       */
      static void CheckMove(const LObject& obj) {
        CheckList(obj);
        if (CountList(obj) >= 3) {
          CheckSquare(*(obj.car()));
          CheckSquare(*(obj.cdr()->car()));
          CheckPieceType(*(obj.cdr()->cdr()->car()));
          return;
        }

        throw GenError("@not-move", "Move must be (<From> <To> <Promotion>).");
      }

      /**
       * メッセージシンボル関数の準備をする。
       * @param symbol メッセージシンボル。
       * @param args 引数リスト。 (関数名を含む)
       * @param required_args 要求される引数の数。
       * @param args_ptr_ptr 引数リストへのポインタのポインタ。
       */
      static void GetReadyForMessageFunction(const std::string& symbol,
      const LObject& args, int required_args, LObject** args_ptr_ptr);

      // ========================== //
      // Lisp関数オブジェクト用関数 //
      // ========================== //
      /** エンジン関数オブジェクトを生成する。 */
      DEF_LC_FUNCTION(GenEngine);

      /** ライセンスを表示する。 */
      DEF_LC_FUNCTION(SayuriLicense) {
        return NewString(LICENSE);
      }

      /** マスのシンボルを数値に変換する。 */
      DEF_LC_FUNCTION(SquareToNumber) ;

      /** ファイルのシンボルを数値に変換する。 */
      DEF_LC_FUNCTION(FyleToNumber) ;

      /** ランクのシンボルを数値に変換する。 */
      DEF_LC_FUNCTION(RankToNumber) ;

      /** サイドのシンボルを数値に変換する。 */
      DEF_LC_FUNCTION(SideToNumber) ;

      /** 駒の種類のシンボルを数値に変換する。 */
      DEF_LC_FUNCTION(PieceToNumber) ;

      /** キャスリングのシンボルを数値に変換する。 */
      DEF_LC_FUNCTION(CastlingToNumber) ;

      /** 数値をマスのシンボルに変換する。 */
      DEF_LC_FUNCTION(NumberToSquare) ;

      /** 数値をファイルのシンボルに変換する。 */
      DEF_LC_FUNCTION(NumberToFyle) ;

      /** 数値をランクのシンボルに変換する。 */
      DEF_LC_FUNCTION(NumberToRank) ;

      /** 数値をサイドのシンボルに変換する。 */
      DEF_LC_FUNCTION(NumberToSide) ;

      /** 数値を駒の種類のシンボルに変換する。 */
      DEF_LC_FUNCTION(NumberToPiece) ;

      /** 数値をキャスリングのシンボルに変換する。 */
      DEF_LC_FUNCTION(NumberToCastling) ;

      /** PGNオブジェクトを作成する。 */
      DEF_LC_FUNCTION(GenPGN) ;

      /** FEN/EPDをパースする。 */
      DEF_LC_FUNCTION(ParseFENEPD) ;

      /** 駒の配列リストをFENの駒の配置文字列に変換する。 */
      DEF_LC_FUNCTION(ToFENPosition) ;

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * ヘルプを作成する。
       */
      void SetHelp();
  };

  /** Sayulisp用エンジンセット。 */
  class EngineSuite : public LN_Function {
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

      // ============== //
      // パブリック関数 //
      // ============== //
      // ========== //
      // Lisp用関数 //
      // ========== //
      /** 関数オブジェクト。 */
      DEF_LC_FUNCTION(operator());

      // ====================== //
      // メッセージシンボル関数 //
      // ====================== //
      /** 駒の配置を得る。 */
      template<Side SIDE, PieceType PIECE_TYPE>
      DEF_MESSAGE_FUNCTION(GetPosition);

      /** 駒を得る。 */
      DEF_MESSAGE_FUNCTION(GetPiece);

      /** すべての駒を得る。 */
      DEF_MESSAGE_FUNCTION(GetAllPieces);

      // %%% @get-to-move
      /** 現在のサイドを得る。 */
      DEF_MESSAGE_FUNCTION(GetToMove) {
        return Lisp::NewSymbol(Sayulisp::SIDE_MAP_INV[board_ptr_->to_move_]);
      }

      // %%% @get-castling-rights
      /** 現在のキャスリングの権利を得る。 */
      DEF_MESSAGE_FUNCTION(GetCastlingRights) {
        LPointerVec ret_vec;
        Castling rights = board_ptr_->castling_rights_;

        if ((rights & WHITE_SHORT_CASTLING)) {
          ret_vec.push_back(Lisp::NewSymbol(Sayulisp::CASTLING_MAP_INV[1]));
        }
        if ((rights & WHITE_LONG_CASTLING)) {
          ret_vec.push_back(Lisp::NewSymbol(Sayulisp::CASTLING_MAP_INV[2]));
        }
        if ((rights & BLACK_SHORT_CASTLING)) {
          ret_vec.push_back(Lisp::NewSymbol(Sayulisp::CASTLING_MAP_INV[3]));
        }
        if ((rights & BLACK_LONG_CASTLING)) {
          ret_vec.push_back(Lisp::NewSymbol(Sayulisp::CASTLING_MAP_INV[4]));
        }

        return Lisp::LPointerVecToList(ret_vec);
      }

      // %%% @get-en-passant-square
      /** 現在のアンパッサンのマスを得る。 */
      DEF_MESSAGE_FUNCTION(GetEnPassantSquare) {
        if (board_ptr_->en_passant_square_) {
          return Lisp::NewSymbol
          (Sayulisp::SQUARE_MAP_INV[board_ptr_->en_passant_square_]);
        }
        return Lisp::NewNil();
      }

      // %%% @get-ply
      /** 現在の手数を得る。 */
      DEF_MESSAGE_FUNCTION(GetPly) {
        return Lisp::NewNumber(board_ptr_->ply_);
      }

      // %%% @get-clock
      /** 現在のクロックを得る。 */
      DEF_MESSAGE_FUNCTION(GetClock) {
        return Lisp::NewNumber(board_ptr_->clock_);
      }

      // %%% @get-white-has-castled
      // %%% @get-black-has-castled
      /** 現在のキャスリングしたかどうかを得る。 */
      template<Side SIDE>
      DEF_MESSAGE_FUNCTION(GetHasCastled) {
        return Lisp::NewBoolean(board_ptr_->has_castled_[SIDE]);
      }

      // %%% @get-fen
      /** 現在の状態のFENを得る。 */
      DEF_MESSAGE_FUNCTION(GetFEN) {
        return Lisp::NewString(engine_ptr_->GetFENString());
      }

      // %%% @to-string
      /** 現在の状態の文字列を得る。 */
      DEF_MESSAGE_FUNCTION(BoardToString) {
        return Lisp::NewString(Board::ToString(*board_ptr_));
      }

      // %%% @set-new-game
      /** ボードの状態を初期状態にする。 */
      DEF_MESSAGE_FUNCTION(SetNewGame) {
        engine_ptr_->SetNewGame();
        return Lisp::NewBoolean(true);
      }

      /** FENをセットする。 */
      DEF_MESSAGE_FUNCTION(SetFEN);

      /** 駒を置く。 */
      DEF_MESSAGE_FUNCTION(PlacePiece);

      /** 候補手を得る。 */
      DEF_MESSAGE_FUNCTION(GetCandidateMoves);

      /** 手番をセットする。 */
      DEF_MESSAGE_FUNCTION(SetToMove);

      /** キャスリングの権利をセットする。 */
      DEF_MESSAGE_FUNCTION(SetCastlingRights);

      /** アンパッサンのマスをセットする。 */
      DEF_MESSAGE_FUNCTION(SetEnPassantSquare);

      /** 手数をセットする。 */
      DEF_MESSAGE_FUNCTION(SetPly);

      /** クロックをセットする。 */
      DEF_MESSAGE_FUNCTION(SetClock);

      // %%% @correct-position?
      /** 正しい配置かどうか。 */
      DEF_MESSAGE_FUNCTION(IsCorrectPosition) {
        return Lisp::NewBoolean(engine_ptr_->IsCorrectPosition());
      }

      // %%% @white-checked?
      // %%% @black-checked?
      /** チェックがかかっているかどうか。 */
      template<Side SIDE>
      DEF_MESSAGE_FUNCTION(IsChecked) {
        constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
        return Lisp::NewBoolean
        (engine_ptr_->IsAttacked(board_ptr_->king_[SIDE], ENEMY_SIDE));
      }

      // %%% @checkmated?
      /** 正しい配置かどうか。 */
      DEF_MESSAGE_FUNCTION(IsCheckmated) {
        Side to_move = board_ptr_->to_move_;

        if (engine_ptr_->GetLegalMoves().size() == 0) {
          if (engine_ptr_->IsAttacked(board_ptr_->king_[to_move],
          Util::GetOppositeSide(to_move))) {
            return Lisp::NewBoolean(true);
          }
        }

        return Lisp::NewBoolean(false);
      }

      // %%% @stalemated?
      /** 正しい配置かどうか。 */
      DEF_MESSAGE_FUNCTION(IsStalemated) {
        Side to_move = board_ptr_->to_move_;

        if (engine_ptr_->GetLegalMoves().size() == 0) {
          if (!(engine_ptr_->IsAttacked(board_ptr_->king_[to_move],
          Util::GetOppositeSide(to_move)))) {
            return Lisp::NewBoolean(true);
          }
        }

        return Lisp::NewBoolean(false);
      }

      /** 手を指す。 */
      DEF_MESSAGE_FUNCTION(PlayMoveOrNote);

      /** 指し手を戻す。 */
      DEF_MESSAGE_FUNCTION(UndoMove);

      /** 指し手をPGNの指し手の文字列に変換する。 */
      DEF_MESSAGE_FUNCTION(MoveToNote);

      /** UCIコマンドを入力する。 */
      DEF_MESSAGE_FUNCTION(InputUCICommand);

      /** UCIコマンドの出力リスナーを登録する。 */
      DEF_MESSAGE_FUNCTION(AddUCIOutputListener);

      /** エンジンとして実行する。 */
      DEF_MESSAGE_FUNCTION(RunEngine);

      /**
       * Go...()で使う関数
       * @param depth 探索深さ。
       * @param nodes 探索するノード数。
       * @param thinking_time 思考するミリ秒。
       * @param candidate_list 探索する候補手のリスト。 (Nilなら全て。)
       * @return PVラインのリスト。
       */
      LPointer GoFunc(std::uint32_t depth, std::uint64_t nodes,
      int thinking_time, const LObject& candidate_list);

      /** ミリ秒で思考する。 */
      DEF_MESSAGE_FUNCTION(GoMoveTime);

      /** 持ち時間ミリ秒以内で思考する。 */
      DEF_MESSAGE_FUNCTION(GoTimeLimit);

      /** 指定深さで思考する。 */
      DEF_MESSAGE_FUNCTION(GoDepth);

      /** 指定ノード数で思考する。 */
      DEF_MESSAGE_FUNCTION(GoNodes);

      /** ハッシュテーブルのサイズを設定する。 */
      DEF_MESSAGE_FUNCTION(SetHashSize);

      /** スレッド数を設定する。 */
      DEF_MESSAGE_FUNCTION(SetThreads);

      /** マテリアルを設定する。 */
      DEF_MESSAGE_FUNCTION(SetMaterial);

      /** Search ParamsのBoolean型パラメータの設定マクロ。 */
#define SET_BOOLEAN_PARAM(accessor) \
      LPointer ret_ptr =\
      Lisp::NewBoolean(search_params_ptr_->accessor());\
      \
      LObject* args_ptr = args.cdr()->cdr().get();\
      if (args_ptr->IsPair()) {\
        LPointer result = caller->Evaluate(*(args_ptr->car()));\
        Lisp::CheckType(*result, LType::BOOLEAN);\
        \
        search_params_ptr_->accessor(result->boolean());\
      };\
      \
      return ret_ptr

      /** Search ParamsのNumber型パラメータの設定マクロ。 */
#define SET_NUMBER_PARAM(accessor) \
      LPointer ret_ptr =\
      Lisp::NewNumber(search_params_ptr_->accessor());\
      \
      LObject* args_ptr = args.cdr()->cdr().get();\
      if (args_ptr->IsPair()) {\
        LPointer result = caller->Evaluate(*(args_ptr->car()));\
        Lisp::CheckType(*result, LType::NUMBER);\
        \
        search_params_ptr_->accessor(result->number());\
      };\
      \
      return ret_ptr

      // %%% @enable-quiesce-search
      /** Search Params - enable-quiesce-search */
      DEF_MESSAGE_FUNCTION(SetEnabelQuiesceSearch) {
        SET_BOOLEAN_PARAM(enable_quiesce_search);
      }

      // %%% @enable-repetition-check
      /** SearchParams - enable-repetition-check */
      DEF_MESSAGE_FUNCTION(SetEnabelRepetitionCheck) {
        SET_BOOLEAN_PARAM(enable_repetition_check);
      }

      // %%% @enable-check-extension
      /** SearchParams - enable-check-extension */
      DEF_MESSAGE_FUNCTION(SetEnableCheckExtension) {
        SET_BOOLEAN_PARAM(enable_check_extension);
      }

      // %%% @ybwc-limit-depth
      /** SearchParams - ybwc-limit-depth */
      DEF_MESSAGE_FUNCTION(SetYBWCLimitDepth) {
        SET_NUMBER_PARAM(ybwc_limit_depth);
      }

      // %%% @ybwc-invalid-moves
      /** SearchParams - ybwc-invalid-moves */
      DEF_MESSAGE_FUNCTION(SetYBWCInvalidMoves) {
        SET_NUMBER_PARAM(ybwc_invalid_moves);
      }

      // %%% @enable-aspiration-windows
      /** SearchParams - enable-aspiration-windows */
      DEF_MESSAGE_FUNCTION(SetEnableAspirationWindows) {
        SET_BOOLEAN_PARAM(enable_aspiration_windows);
      }

      // %%% @aspiration-windows-limit-depth
      /** SearchParams - aspiration-windows-limit-depth */
      DEF_MESSAGE_FUNCTION(SetAspirationWindowsLimitDepth) {
        SET_NUMBER_PARAM(aspiration_windows_limit_depth);
      }

      // %%% @aspiration-windows-delta
      /** SearchParams - aspiration-windows-delta */
      DEF_MESSAGE_FUNCTION(SetAspirationWindowsDelta) {
        SET_NUMBER_PARAM(aspiration_windows_delta);
      }

      // %%% @enable-see
      /** SearchParams - enable-see */
      DEF_MESSAGE_FUNCTION(SetEnableSEE) {
        SET_BOOLEAN_PARAM(enable_see);
      }

      // %%% @enable-history
      /** SearchParams - enable-history */
      DEF_MESSAGE_FUNCTION(SetEnableHistory) {
        SET_BOOLEAN_PARAM(enable_history);
      }

      // %%% @enable-killer
      /** SearchParams - enable-killer */
      DEF_MESSAGE_FUNCTION(SetEnableKiller) {
        SET_BOOLEAN_PARAM(enable_killer);
      }

      // %%% @enable-hash-table
      /** SearchParams - enable-hash-table */
      DEF_MESSAGE_FUNCTION(SetEnableHashTable) {
        SET_BOOLEAN_PARAM(enable_ttable);
      }

      // %%% @enable-iid
      /** SearchParams - enable-iid */
      DEF_MESSAGE_FUNCTION(SetEnableIID) {
        SET_BOOLEAN_PARAM(enable_iid);
      }

      // %%% @iid-limit-depth
      /** SearchParams - iid-limit-depth */
      DEF_MESSAGE_FUNCTION(SetIIDLimitDepth) {
        SET_NUMBER_PARAM(iid_limit_depth);
      }

      // %%% @iid-search-depth
      /** SearchParams - iid-search-depth */
      DEF_MESSAGE_FUNCTION(SetIIDSearchDepth) {
        SET_NUMBER_PARAM(iid_search_depth);
      }

      // %%% @enable-iid
      /** SearchParams - enable-iid */
      DEF_MESSAGE_FUNCTION(SetEnableNMR) {
        SET_BOOLEAN_PARAM(enable_nmr);
      }

      // %%% @nmr-limit-depth
      /** SearchParams - nmr-limit-depth */
      DEF_MESSAGE_FUNCTION(SetNMRLimitDepth) {
        SET_NUMBER_PARAM(nmr_limit_depth);
      }

      // %%% @nmr-search-reduction
      /** SearchParams - nmr-search-reduction */
      DEF_MESSAGE_FUNCTION(SetNMRSearchReduction) {
        SET_NUMBER_PARAM(nmr_search_reduction);
      }

      // %%% @nmr-reduction
      /** SearchParams - nmr-reduction */
      DEF_MESSAGE_FUNCTION(SetNMRReduction) {
        SET_NUMBER_PARAM(nmr_reduction);
      }

      // %%% @enable-probcut
      /** SearchParams - enable-probcut */
      DEF_MESSAGE_FUNCTION(SetEnableProbCut) {
        SET_BOOLEAN_PARAM(enable_probcut);
      }

      // %%% @probcut-limit-depth
      /** SearchParams - probcut-limit-depth */
      DEF_MESSAGE_FUNCTION(SetProbCutLimitDepth) {
        SET_NUMBER_PARAM(probcut_limit_depth);
      }

      // %%% @probcut-margin
      /** SearchParams - probcut-margin */
      DEF_MESSAGE_FUNCTION(SetProbCutMargin) {
        SET_NUMBER_PARAM(probcut_margin);
      }

      // %%% @probcut-search-reduction
      /** SearchParams - probcut-search-reduction */
      DEF_MESSAGE_FUNCTION(SetProbCutSearchReduction) {
        SET_NUMBER_PARAM(probcut_search_reduction);
      }

      // %%% @enable-history-pruning
      /** SearchParams - enable-history-pruning */
      DEF_MESSAGE_FUNCTION(SetEnableHistoryPruning) {
        SET_BOOLEAN_PARAM(enable_history_pruning);
      }

      // %%% @history-pruning-limit-depth
      /** SearchParams - history-pruning-limit-depth */
      DEF_MESSAGE_FUNCTION(SetHistoryPruningLimitDepth) {
        SET_NUMBER_PARAM(history_pruning_limit_depth);
      }

      // %%% @history-pruning-move-threshold
      /** SearchParams - history-pruning-move-threshold */
      DEF_MESSAGE_FUNCTION(SetHistoryPruningMoveThreshold) {
        SET_NUMBER_PARAM(history_pruning_move_threshold);
      }

      // %%% @history-pruning-invalid-moves
      /** SearchParams - history-pruning-invalid-moves */
      DEF_MESSAGE_FUNCTION(SetHistoryPruningInvalidMoves) {
        SET_NUMBER_PARAM(history_pruning_invalid_moves);
      }

      // %%% @history-pruning-threshold
      /** SearchParams - history-pruning-threshold */
      DEF_MESSAGE_FUNCTION(SetHistoryPruningThreshold) {
        SET_NUMBER_PARAM(history_pruning_threshold);
      }

      // %%% @history-pruning-reduction
      /** SearchParams - history-pruning-reduction */
      DEF_MESSAGE_FUNCTION(SetHistoryPruningReduction) {
        SET_NUMBER_PARAM(history_pruning_reduction);
      }

      // %%% @enable-lmr
      /** SearchParams - enable-lmr */
      DEF_MESSAGE_FUNCTION(SetEnableLMR) {
        SET_BOOLEAN_PARAM(enable_lmr);
      }

      // %%% @lmr-limit-depth
      /** SearchParams - lmr-limit-depth */
      DEF_MESSAGE_FUNCTION(SetLMRLimitDepth) {
        SET_NUMBER_PARAM(lmr_limit_depth);
      }

      // %%% @lmr-move-threshold
      /** SearchParams - lmr-move-threshold */
      DEF_MESSAGE_FUNCTION(SetLMRMoveThreshold) {
        SET_NUMBER_PARAM(lmr_move_threshold);
      }

      // %%% @lmr-invalid-moves
      /** SearchParams - lmr-invalid-moves */
      DEF_MESSAGE_FUNCTION(SetLMRInvalidMoves) {
        SET_NUMBER_PARAM(lmr_invalid_moves);
      }

      // %%% @lmr-search-reduction
      /** SearchParams - lmr-search-reduction */
      DEF_MESSAGE_FUNCTION(SetLMRSearchReduction) {
        SET_NUMBER_PARAM(lmr_search_reduction);
      }

      // %%% @enable-futility-pruning
      /** SearchParams - enable-futility-pruning */
      DEF_MESSAGE_FUNCTION(SetEnableFutilityPruning) {
        SET_BOOLEAN_PARAM(enable_futility_pruning);
      }

      // %%% @futility-pruning-depth
      /** SearchParams - futility-pruning-depth */
      DEF_MESSAGE_FUNCTION(SetFutilityPruningDepth) {
        SET_NUMBER_PARAM(futility_pruning_depth);
      }

      // %%% @futility-pruning-margin
      /** SearchParams - futility-pruning-margin */
      DEF_MESSAGE_FUNCTION(SetFutilityPruningMargin) {
        SET_NUMBER_PARAM(futility_pruning_margin);
      }

      /** 駒の配置の価値テーブル。 オープニング。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetPieceSquareTableOpening);

      /** 駒の配置の価値テーブル。 エンディング。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetPieceSquareTableEnding);

      /** 攻撃の価値テーブル。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetAttackTable);

      /** 防御の価値テーブル。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetDefenseTable);

      /** ピンの価値テーブル。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetPinTable);

      /** ポーンの盾の価値テーブル。 */
      DEF_MESSAGE_FUNCTION(SetPawnShieldTable);

      /** オープニングの配置のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightOpeningPosition);

      /** エンディングの配置のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightEndingPosition);

      /** 機動力のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightMobility);

      /** センター支配のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightCenterControll);

      /** スウィートセンター支配のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightSweetCenterControll);

      /** 展開のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightDevelopment);

      /** 攻撃のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightAttack);

      /** 防御のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightDefense);

      /** ピンのウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightPin);

      /** キング周辺への攻撃のウェイト。 */
      template<PieceType TYPE>
      DEF_MESSAGE_FUNCTION(SetWeightAttackAroundKing);

      /** パスポーンのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightPassPawn);

      /** 守られたパスポーンのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightProtectedPassPawn);

      /** ダブルポーンのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightDoublePawn);

      /** 孤立ポーンのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightIsoPawn);

      /** ポーンの盾のウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightPawnShield);

      /** ビショップペアのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightBishopPair);

      /** バッドビショップのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightBadBishop);

      /** ルークペアのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightRookPair);

      /** セミオープンファイルのルークのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightRookSemiopenFyle);

      /** オープンファイルのルークのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightRookOpenFyle);

      /** 早すぎるクイーンの始動のウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightEarlyQueenStarting);

      /** キング周りの弱いマスのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightWeakSquare);

      /** キャスリングのウェイト。 */
      DEF_MESSAGE_FUNCTION(SetWeightCastling);

      /** キャスリングの放棄。 */
      DEF_MESSAGE_FUNCTION(SetWeightAbandonedCastling);

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
       * メッセージシンボル関数を設定する。
       */
      void SetMessageFunctions();

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
      /** エンジンのチェスボードのポインタ。 */
      const Board* board_ptr_;
      /** UCIShellのポインタ。 */
      std::unique_ptr<UCIShell> shell_ptr_;

      /** UCIのアウトプットリスナー。 */
      std::vector<std::function<void(const std::string&)>> callback_vec_;

      /** 各メッセージシンボル関数オブジェクトのマップ。 */
      std::map<std::string, Sayulisp::MessageFunction> message_func_map_;
  };
}  // namespace Sayuri

#endif
