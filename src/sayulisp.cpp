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
 * @file sayulisp.cpp
 * @author Hironori Ishibashi
 * @brief Sayuri用Lispライブラリの実装。
 */

#include "sayulisp.h"

#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <climits>
#include <sstream>
#include "common.h"
#include "params.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "uci_shell.h"
#include "lisp_core.h"
#include "fen.h"
#include "position_record.h"
#include "pv_line.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ========== //
  // static定数 //
  // ========== //
  const std::string EngineSuite::SQUARE_SYMBOL[NUM_SQUARES] {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"
  };
  const std::string EngineSuite::FYLE_SYMBOL[NUM_FYLES] {
    "FYLE_A", "FYLE_B", "FYLE_C", "FYLE_D",
    "FYLE_E", "FYLE_F", "FYLE_G", "FYLE_H"
  };
  const std::string EngineSuite::RANK_SYMBOL[NUM_RANKS] {
    "RANK_1", "RANK_2", "RANK_3", "RANK_4",
    "RANK_5", "RANK_6", "RANK_7", "RANK_8"
  };
  const std::string EngineSuite::SIDE_SYMBOL[NUM_SIDES] {
    "NO_SIDE", "WHITE", "BLACK"
  };
  const std::string EngineSuite::PIECE_TYPE_SYMBOL[NUM_PIECE_TYPES] {
    "EMPTY", "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"
  };
  const std::string EngineSuite::CASTLING_SYMBOL[5] {
    "NO_CASTLING",
    "WHITE_SHORT_CASTLING", "WHITE_LONG_CASTLING",
    "BLACK_SHORT_CASTLING", "BLACK_LONG_CASTLING"
  };


  // =========== //
  // EngineSuite //
  // =========== //
  // コンストラクタ。
  EngineSuite::EngineSuite() :
  search_params_ptr_(new SearchParams()),
  eval_params_ptr_(new EvalParams()),
  engine_ptr_(new ChessEngine(*search_params_ptr_, *eval_params_ptr_)),
  table_ptr_(new TranspositionTable(UCI_DEFAULT_TABLE_SIZE)),
  shell_ptr_(new UCIShell(*engine_ptr_, *table_ptr_)) {
    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});
  }

  // コピーコンストラクタ。
  EngineSuite::EngineSuite(const EngineSuite& suite) :
  search_params_ptr_(new SearchParams(*(suite.search_params_ptr_))),
  eval_params_ptr_(new EvalParams(*(suite.eval_params_ptr_))),
  engine_ptr_(new ChessEngine(*search_params_ptr_, *eval_params_ptr_)),
  table_ptr_(new TranspositionTable(suite.table_ptr_->GetSizeBytes())),
  shell_ptr_(new UCIShell(*engine_ptr_, *table_ptr_)) {
    PositionRecord record(*(suite.engine_ptr_), 0);
    engine_ptr_->LoadRecord(record);

    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});
  }

  // ムーブコンストラクタ。
  EngineSuite::EngineSuite(EngineSuite&& suite) :
  search_params_ptr_(std::move(suite.search_params_ptr_)),
  eval_params_ptr_(std::move(suite.eval_params_ptr_)),
  engine_ptr_(std::move(suite.engine_ptr_)),
  table_ptr_(std::move(suite.table_ptr_)),
  shell_ptr_(std::move(suite.shell_ptr_)) {}

  // コピー代入演算子。
  EngineSuite& EngineSuite::operator=(const EngineSuite& suite) {
    search_params_ptr_.reset(new SearchParams(*(suite.search_params_ptr_)));
    eval_params_ptr_.reset(new EvalParams(*(suite.eval_params_ptr_)));
    engine_ptr_.reset(new ChessEngine(*search_params_ptr_, *eval_params_ptr_));
    table_ptr_.reset(new TranspositionTable(suite.table_ptr_->GetSizeBytes()));
    shell_ptr_.reset(new UCIShell(*engine_ptr_, *table_ptr_));

    PositionRecord record(*(suite.engine_ptr_), 0);
    engine_ptr_->LoadRecord(record);

    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});

    return *this;
  }

  // ムーブ代入演算子。
  EngineSuite& EngineSuite::operator=(EngineSuite&& suite) {
    search_params_ptr_ = std::move(suite.search_params_ptr_);
    eval_params_ptr_ = std::move(suite.eval_params_ptr_);
    engine_ptr_ = std::move(suite.engine_ptr_);
    table_ptr_ = std::move(suite.table_ptr_);
    shell_ptr_ = std::move(suite.shell_ptr_);
    return *this;
  }

  // ========================== //
  // Lisp関数オブジェクト用関数 //
  // ========================== //
  // 関数オブジェクト。
  LispObjectPtr EngineSuite::operator()
  (LispObjectPtr self, const LispObject& caller, const LispObject& list) {
    // 準備。
    LispIterator list_itr(&list);
    std::string func_name = (list_itr++)->ToString();
    int required_args = 1;

    // 引数チェック。
    if (!list_itr) {
      throw LispObject::GenInsufficientArgumentsError
      (func_name, required_args, true, list.Length() - 1);
    }

    // メッセージシンボルを得る。
    LispObjectPtr message_ptr = caller.Evaluate(*(list_itr++));
    if (!(message_ptr->IsSymbol())) {
      throw LispObject::GenWrongTypeError
      (func_name, "Symbol", std::vector<int> {1}, true);
    }
    std::string message_symbol = message_ptr->symbol_value();

    // メッセージシンボルに合わせて分岐する。
    if (message_symbol == "@get-white-pawn-position") {
      return GetWhitePawnPosition();

    } else if (message_symbol == "@get-white-knight-position") {
      return GetWhiteKnightPosition();

    } else if (message_symbol == "@get-white-bishop-position") {
      return GetWhiteBishopPosition();

    } else if (message_symbol == "@get-white-rook-position") {
      return GetWhiteRookPosition();

    } else if (message_symbol == "@get-white-queen-position") {
      return GetWhiteQueenPosition();

    } else if (message_symbol == "@get-white-king-position") {
      return GetWhiteKingPosition();

    } else if (message_symbol == "@get-black-pawn-position") {
      return GetBlackPawnPosition();

    } else if (message_symbol == "@get-black-knight-position") {
      return GetBlackKnightPosition();

    } else if (message_symbol == "@get-black-bishop-position") {
      return GetBlackBishopPosition();

    } else if (message_symbol == "@get-black-rook-position") {
      return GetBlackRookPosition();

    } else if (message_symbol == "@get-black-queen-position") {
      return GetBlackQueenPosition();

    } else if (message_symbol == "@get-black-king-position") {
      return GetBlackKingPosition();

    } else if (message_symbol == "@get-empty-square-position") {
      return GetEmptySquarePosition();

    } else if (message_symbol == "@get-piece") {
      // 駒を得る。
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr square_ptr = caller.Evaluate(*list_itr);
      if (!(square_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      Square square = square_ptr->number_value();

      return GetPiece(func_name, square);

    } else if (message_symbol == "@get-to-move") {
      return GetToMove();

    } else if (message_symbol == "@get-castling-rights") {
      return GetCastlingRights();

    } else if (message_symbol == "@get-en-passant-square") {
      return GetEnPassantSquare();

    } else if (message_symbol == "@get-ply") {
      return GetPly();

    } else if (message_symbol == "@get-clock") {
      return GetClock();

    } else if (message_symbol == "@get-white-has-castled") {
      return GetWhiteHasCastled();

    } else if (message_symbol == "@get-black-has-castled") {
      return GetBlackHasCastled();

    } else if (message_symbol == "@set-new-game") {
      return SetNewGame();

    } else if (message_symbol == "@set-fen") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr fen_str_ptr = caller.Evaluate(*list_itr);
      if (!(fen_str_ptr->IsString())) {
        throw LispObject::GenWrongTypeError
        (func_name, "String", std::vector<int> {2}, true);
      }
      return SetFEN(fen_str_ptr);

    } else if (message_symbol == "@get-candidate-moves") {
      return GetCandidateMoves();

    } else if (message_symbol == "@place-piece") {
      required_args = 4;
      // 駒の位置を得る。
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr square_ptr = caller.Evaluate(*(list_itr++));
      if (!(square_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      // 駒の種類を得る。
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr type_ptr = caller.Evaluate(*(list_itr++));
      if (!(type_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {3}, true);
      }

      // 駒のサイドを得る。
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr side_ptr = caller.Evaluate(*(list_itr));
      if (!(side_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {4}, true);
      }

      return PlacePiece(square_ptr, type_ptr, side_ptr);
    } else if (message_symbol == "@set-to-move") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr to_move_ptr = caller.Evaluate(*list_itr);
      if (!(to_move_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      return SetToMove(to_move_ptr);

    } else if (message_symbol == "@set-castling-rights") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr castling_rights_ptr = caller.Evaluate(*list_itr);
      if (!(castling_rights_ptr->IsList())) {
        throw LispObject::GenWrongTypeError
        (func_name, "List", std::vector<int> {2}, true);
      }

      return SetCastlingRights(castling_rights_ptr, func_name);

    } else if (message_symbol == "@set-en-passant-square") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr en_passant_square_ptr = caller.Evaluate(*list_itr);
      if (!((en_passant_square_ptr->IsNumber())
      || (en_passant_square_ptr->IsNil()))) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number or Nil", std::vector<int> {2}, true);
      }

      return SetEnPassantSquare(en_passant_square_ptr);

    } else if (message_symbol == "@set-ply") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr ply_ptr = caller.Evaluate(*list_itr);
      if (!(ply_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      return SetPly(ply_ptr);

    } else if (message_symbol == "@set-clock") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr clock_ptr = caller.Evaluate(*list_itr);
      if (!(clock_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      return SetClock(clock_ptr);

    } else if (message_symbol == "@correct-position?") {
      return IsCorrectPosition();

    } else if (message_symbol == "@white-checked?") {
      return IsWhiteChecked();

    } else if (message_symbol == "@black-checked?") {
      return IsBlackChecked();

    } else if (message_symbol == "@checkmated?") {
      return IsCheckmated();

    } else if (message_symbol == "@stalemated?") {
      return IsStalemated();

    } else if (message_symbol == "@play-move") {
      // 引数をチェック。
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr move_obj = caller.Evaluate(*list_itr);
      if (!(move_obj->IsList())) {
        throw LispObject::GenWrongTypeError
        (func_name, "List", std::vector<int> {2}, true);
      }
      return PlayMove(caller, func_name, move_obj);

    } else if (message_symbol == "@undo-move") {
      return UndoMove();

    } else if (message_symbol == "@input-uci-command") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr command_ptr = caller.Evaluate(*list_itr);
      if (!(command_ptr->IsString())) {
        throw LispObject::GenWrongTypeError
        (func_name, "String", std::vector<int> {2}, true);
      }
      return InputUCICommand(command_ptr);

    } else if (message_symbol == "@add-uci-output-listener") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr func_ptr = caller.Evaluate(*list_itr);
      if (!(func_ptr->IsFunction())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Function", std::vector<int> {2}, true);
      }
      int num_args = func_ptr->function().arg_name_vec_.size();
      if (num_args != 1) {
        throw LispObject::GenError("@engine_error",
        "The number of argument of callback must be 1. ("
        + list_itr->ToString() + ") requires "
        + std::to_string(num_args) + " arguments.");
      }

      return AddUCIOutputListener(caller, *list_itr);

    } else if (message_symbol == "@go-movetime") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr move_time_ptr = caller.Evaluate(*(list_itr++));
      if (!(move_time_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      LispObjectPtr move_list_ptr = LispObject::NewNil();
      if (list_itr) {
        move_list_ptr = caller.Evaluate(*list_itr);
      }
      return GoMoveTime(func_name, *move_time_ptr, *move_list_ptr);

    } else if (message_symbol == "@go-timelimit") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr time_limit_ptr = caller.Evaluate(*(list_itr++));
      if (!(time_limit_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      LispObjectPtr move_list_ptr = LispObject::NewNil();
      if (list_itr) {
        move_list_ptr = caller.Evaluate(*list_itr);
      }
      return GoTimeLimit(func_name, *time_limit_ptr, *move_list_ptr);

    } else if (message_symbol == "@go-depth") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr depth_ptr = caller.Evaluate(*(list_itr++));
      if (!(depth_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      LispObjectPtr move_list_ptr = LispObject::NewNil();
      if (list_itr) {
        move_list_ptr = caller.Evaluate(*list_itr);
      }
      return GoDepth(func_name, *depth_ptr, *move_list_ptr);

    } else if (message_symbol == "@go-nodes") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr nodes_ptr = caller.Evaluate(*(list_itr++));
      if (!(nodes_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }

      LispObjectPtr move_list_ptr = LispObject::NewNil();
      if (list_itr) {
        move_list_ptr = caller.Evaluate(*list_itr);
      }
      return GoNodes(func_name, *nodes_ptr, *move_list_ptr);

    } else if (message_symbol == "@set-hash-size") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr hash_size_ptr = caller.Evaluate(*(list_itr++));
      if (!(hash_size_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }
      return SetHashSize(*hash_size_ptr);

    } else if (message_symbol == "@set-threads") {
      required_args = 2;
      if (!list_itr) {
        throw LispObject::GenInsufficientArgumentsError
        (func_name, required_args, false, list.Length() - 1);
      }
      LispObjectPtr num_threads_ptr = caller.Evaluate(*(list_itr++));
      if (!(num_threads_ptr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2}, true);
      }
      return SetThreads(*num_threads_ptr);

    } else if (message_symbol == "@material") {
      LispObjectPtr material_list_ptr = LispObject::NewNil();
      if (list_itr) {
        material_list_ptr = caller.Evaluate(*list_itr);
        if (!(material_list_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      return SetMaterial(*material_list_ptr);

    } else if (message_symbol == "@enable-quiesce-search") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableQuiesceSearch(*enable_ptr);

    } else if (message_symbol == "@enable-repetition-check") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableRepetitionCheck(*enable_ptr);

    } else if (message_symbol == "@enable-check-extension") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableCheckExtension(*enable_ptr);

    } else if (message_symbol == "@ybwc-limit-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetYBWCLimitDepth(*depth_ptr);

    } else if (message_symbol == "@ybwc-invalid-moves") {
      LispObjectPtr num_moves_ptr = LispObject::NewNil();
      if (list_itr) {
        num_moves_ptr = caller.Evaluate(*list_itr);
        if (!(num_moves_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetYBWCInvalidMoves(*num_moves_ptr);

    } else if (message_symbol == "@enable-aspiration-windows") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableAspirationWindows(*enable_ptr);

    } else if (message_symbol == "@aspiration-windows-limit-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetAspirationWindowsLimitDepth(*depth_ptr);

    } else if (message_symbol == "@aspiration-windows-delta") {
      LispObjectPtr delta_ptr = LispObject::NewNil();
      if (list_itr) {
        delta_ptr = caller.Evaluate(*list_itr);
        if (!(delta_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetAspirationWindowsDelta(*delta_ptr);

    }

    throw LispObject::GenError("@engine-error", "(" + func_name
    + ") couldn't understand '" + message_symbol + "'.");
  }

  LispObjectPtr EngineSuite::GetBestMove(std::uint32_t depth,
  std::uint64_t nodes, int thinking_time, const std::vector<Move>& move_vec) {
    // ストッパーを登録。
    engine_ptr_->SetStopper(Util::GetMin(depth, MAX_PLYS),
    Util::GetMin(nodes, MAX_NODES),
    Chrono::milliseconds(thinking_time), false);

    // テーブルの年齢を上げる。
    table_ptr_->GrowOld();

    // 思考開始。
    PVLine pv_line = engine_ptr_->Calculate(shell_ptr_->num_threads(),
    *table_ptr_, move_vec, *shell_ptr_);
    Move best_move = pv_line.length() >= 1 ? pv_line[0] : 0;

    std::ostringstream stream;
    if (best_move) {
      stream << "bestmove "<< Util::MoveToString(best_move);

      // 2手目があるならponderで表示。
      if (pv_line.length() >= 2) {
        stream << " ponder " << Util::MoveToString(pv_line[1]);
      }
    }

    // UCIアウトプットリスナーに最善手を送る。
    for (auto& callback : callback_vec_) {
      callback(stream.str());
    }

    // 戻り値の最善手を作る。
    if (best_move) {
      LispObjectPtr ret_ptr = LispObject::NewList(3);
      ret_ptr->car
      (LispObject::NewSymbol(SQUARE_SYMBOL[GetFrom(best_move)]));
      ret_ptr->cdr()->car
      (LispObject::NewSymbol(SQUARE_SYMBOL[GetTo(best_move)]));
      ret_ptr->cdr()->cdr()->car
      (LispObject::NewSymbol(PIECE_TYPE_SYMBOL[GetPromotion(best_move)]));

      return ret_ptr;
    }

    return LispObject::NewNil();
  }

  // 手のリストから手のベクトルを作る。
  std::vector<Move> EngineSuite::MoveListToVec
  (const std::string& func_name, const LispObject& move_list) {
    if (!(move_list.IsList())) {
      throw LispObject::GenWrongTypeError
      (func_name, "List", std::vector<int> {3}, true);
    }

    std::vector<Move> ret;

    LispIterator list_itr(&move_list);
    for (int index = 1; list_itr; ++list_itr, ++index) {
      // リストかどうか。
      if (!(list_itr->IsList())) {
        throw LispObject::GenWrongTypeError
        (func_name, "List", std::vector<int> {3, index}, true);
      }

      // 長さは3?。
      if (list_itr->Length() != 3) {
        throw LispObject::GenError("@engine_error",
        "The " + std::to_string(index) + "th move of move list of ("
        + func_name + ") must be 3 elements. Given "
        + std::to_string(list_itr->Length()) + ".");
      }

      Move move = 0;
      // from。
      if (!(list_itr->car()->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {3, index, 1}, true);
      }
      int square = list_itr->car()->number_value();
      if ((square < static_cast<int>(A1))
      || (square > static_cast<int>(H8))) {
        throw GenWrongSquareError(func_name, square);
      }
      SetFrom(move, square);
      // to。
      if (!(list_itr->cdr()->car()->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {3, index, 2}, true);
      }
      square = list_itr->cdr()->car()->number_value();
      if ((square < static_cast<int>(A1))
      || (square > static_cast<int>(H8))) {
        throw GenWrongSquareError(func_name, square);
      }
      SetTo(move, square);
      // promotion。
      if (!(list_itr->cdr()->cdr()->car()->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {3, index, 3}, true);
      }
      int piece_type = list_itr->cdr()->cdr()->car()->number_value();
      if ((piece_type < static_cast<int>(EMPTY))
      || (piece_type > static_cast<int>(KING))) {
        throw GenWrongPieceTypeError(func_name, piece_type);
      }
      SetPromotion(move, piece_type);

      // ベクトルにプッシュ。
      ret.push_back(move);
    }

    return ret;
  }

  // 白ポーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhitePawnPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][PAWN];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白ナイトの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteKnightPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][KNIGHT];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白ビショップの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteBishopPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][BISHOP];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白ルークの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteRookPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][ROOK];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白クイーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteQueenPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][QUEEN];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白キングの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteKingPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][KING];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ポーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackPawnPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][PAWN];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ナイトの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackKnightPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][KNIGHT];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ビショップの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackBishopPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][BISHOP];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ルークの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackRookPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][ROOK];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒クイーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackQueenPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][QUEEN];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒キングの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackKingPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][KING];
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 空のマスの配置にアクセス。
  LispObjectPtr EngineSuite::GetEmptySquarePosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = ~(engine_ptr_->blocker_0());
    bb; NEXT_BITBOARD(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(SQUARE_SYMBOL[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }

  // 駒を得る。
  LispObjectPtr EngineSuite::GetPiece(const std::string& func_name,
  Square square) const {
    LispObjectPtr ret_ptr = LispObject::NewList(2);
    if (square >= NUM_SQUARES) {
      throw GenWrongSquareError(func_name, square);
    }

    ret_ptr->car(LispObject::NewSymbol
    (SIDE_SYMBOL[engine_ptr_->side_board()[square]]));

    ret_ptr->cdr()->car(LispObject::NewSymbol
    (PIECE_TYPE_SYMBOL[engine_ptr_->piece_board()[square]]));

    return ret_ptr;
  }

  // 手番にアクセス。
  LispObjectPtr EngineSuite::GetToMove() const {
    return LispObject::NewSymbol(SIDE_SYMBOL[engine_ptr_->to_move()]);
  }

  // キャスリングの権利にアクセス。
  LispObjectPtr EngineSuite::GetCastlingRights() const {
    Castling rights = engine_ptr_->castling_rights();

    LispObjectPtr ret_ptr = LispObject::NewNil();
    if ((rights & WHITE_SHORT_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[1]),
      LispObject::NewNil()));
    }
    if ((rights & WHITE_LONG_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[2]),
      LispObject::NewNil()));
    }
    if ((rights & BLACK_SHORT_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[3]),
      LispObject::NewNil()));
    }
    if ((rights & BLACK_LONG_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[4]),
      LispObject::NewNil()));
    }

    return ret_ptr;
  }

  // アンパッサンのマスにアクセス。
  LispObjectPtr EngineSuite::GetEnPassantSquare() const {
    Square en_passant_square = engine_ptr_->en_passant_square();

    if (en_passant_square) {
      return LispObject::NewSymbol(SQUARE_SYMBOL[en_passant_square]);
    }

    return LispObject::NewNil();
  }

  // 手数にアクセス。
  LispObjectPtr EngineSuite::GetPly() const {
    return LispObject::NewNumber(engine_ptr_->ply());
  }

  // 50手ルールの手数にアクセス。
  LispObjectPtr EngineSuite::GetClock() const {
    return LispObject::NewNumber(engine_ptr_->clock());
  }

  // 白がキャスリングしたかどうかのフラグにアクセス。
  LispObjectPtr EngineSuite::GetWhiteHasCastled() const {
    return LispObject::NewBoolean(engine_ptr_->has_castled()[WHITE]);
  }
  // 黒がキャスリングしたかどうかのフラグにアクセス。
  LispObjectPtr EngineSuite::GetBlackHasCastled() const {
    return LispObject::NewBoolean(engine_ptr_->has_castled()[BLACK]);
  }

  // ボードを初期状態にする。
  LispObjectPtr EngineSuite::SetNewGame() {
    engine_ptr_->SetNewGame();
    return LispObject::NewBoolean(true);
  }

  // FENの配置にする。
  LispObjectPtr EngineSuite::SetFEN(LispObjectPtr fen_str_ptr) {
    FEN fen;
    try {
      fen = FEN(fen_str_ptr->string_value());
    } catch (SayuriError error) {
      throw LispObject::GenError("@engine-error", "Couldn't parse FEN.");
    }

    // 必ずそれぞれキングが1つずつでなくてはならない。
    // そうでなければ配置失敗。
    int num_white_king = Util::CountBits(fen.position()[WHITE][KING]);
    int num_black_king = Util::CountBits(fen.position()[BLACK][KING]);
    if ((num_white_king != 1) || (num_black_king != 1)) {
      throw LispObject::GenError("@engine-error",
      "This FEN indicates invalid position.");
    }

    engine_ptr_->LoadFEN(fen);
    return LispObject::NewBoolean(true);
  }

  // 候補手のリストを得る。
  LispObjectPtr EngineSuite::GetCandidateMoves() {
    LispObjectPtr ret_ptr = LispObject::NewNil();

    // 手の生成。
    std::vector<Move> move_vec = engine_ptr_->GetLegalMoves();

    // 手をリストに追加していく。
    for (auto move : move_vec) {
      ret_ptr->Append
      (LispObject::NewPair(MoveToList(move), LispObject::NewNil()));
    }

    return ret_ptr;
  }

  // 駒を置く。
  LispObjectPtr EngineSuite::PlacePiece(LispObjectPtr square_ptr,
  LispObjectPtr type_ptr, LispObjectPtr side_ptr) {
    // 準備。
    Square square = square_ptr->number_value();
    PieceType piece_type = type_ptr->number_value();
    Side side = side_ptr->number_value();

    // 引数チェック。
    if (square >= NUM_SQUARES) {
      throw LispObject::GenError("@engine_error",
      "The square value '" + std::to_string(square)
      + "' doesn't indicate any square.");
    }
    if (piece_type >= NUM_PIECE_TYPES) {
      throw LispObject::GenError("@engine_error",
      "The piece type value '" + std::to_string(piece_type)
      +  "' doesn't indicate any piece type.");
    }
    if (side >= NUM_SIDES) {
      throw LispObject::GenError("@engine_error",
      "The side value '" + std::to_string(side)
      + "' doesn't indicate any side.");
    }
    if ((piece_type && !side) || (!piece_type && side)) {
      throw LispObject::GenError("@engine_error",
      "'" + SIDE_SYMBOL[side] + " " + PIECE_TYPE_SYMBOL[piece_type]
      + "' doesn't exist in the world.");
    }

    // 元の駒の種類とサイドを得る。
    PieceType origin_type = engine_ptr_->piece_board()[square];
    Side origin_side = engine_ptr_->side_board()[square];

    // もし置き換える前の駒がキングなら置き換えられない。
    if (origin_type == KING) {
      throw LispObject::GenError("@engine_error",
      "Couldn't place the piece, because " + SIDE_SYMBOL[origin_side]
      + " " + PIECE_TYPE_SYMBOL[origin_type] + " is placed there."
      " Each side must have just one King.");
    }

    // チェック終了したので置き換える。
    engine_ptr_->PlacePiece(square, piece_type, side);

    // 戻り値を作る。
    LispObjectPtr ret_ptr = LispObject::NewList(2);
    ret_ptr->car(LispObject::NewSymbol(SIDE_SYMBOL[origin_side]));
    ret_ptr->cdr()->car
    (LispObject::NewSymbol(PIECE_TYPE_SYMBOL[origin_type]));

    return ret_ptr;
  }

  // 手番をセットする。
  LispObjectPtr EngineSuite::SetToMove(LispObjectPtr to_move_ptr) {
    Side to_move = to_move_ptr->number_value();
    if (to_move >= NUM_SIDES) {
      throw LispObject::GenError("@engine_error",
      "The side value '" + std::to_string(to_move)
      + "' doesn't indicate any side.");
    }
    if (!to_move) {
      throw LispObject::GenError("@engine_error", "'NO_SIDE' is not allowed.");
    }

    Side origin = engine_ptr_->to_move();
    engine_ptr_->to_move(to_move);

    return LispObject::NewSymbol(SIDE_SYMBOL[origin]);
  }

  // キャスリングの権利をセットする。
  LispObjectPtr EngineSuite::SetCastlingRights
  (LispObjectPtr castling_rights_ptr, const std::string& func_name) {
    Castling rights = 0;
    int index = 1;
    for (LispIterator itr(castling_rights_ptr.get()); itr; ++itr, ++index) {
      if (!(itr->IsNumber())) {
        throw LispObject::GenWrongTypeError
        (func_name, "Number", std::vector<int> {2, index}, true);
      }
      int num = itr->number_value();
      if (num == 1) rights |= WHITE_SHORT_CASTLING;
      else if (num == 2) rights |= WHITE_LONG_CASTLING;
      else if (num == 3) rights |= BLACK_SHORT_CASTLING;
      else if (num == 4) rights |= BLACK_LONG_CASTLING;
    }

    Castling origin = engine_ptr_->castling_rights();
    LispObjectPtr ret_ptr = LispObject::NewNil();
    if ((origin & WHITE_SHORT_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[1]), LispObject::NewNil()));
    }
    if ((origin & WHITE_LONG_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[2]), LispObject::NewNil()));
    }
    if ((origin & BLACK_SHORT_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[3]), LispObject::NewNil()));
    }
    if ((origin & BLACK_LONG_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(CASTLING_SYMBOL[4]), LispObject::NewNil()));
    }

    engine_ptr_->castling_rights(rights);
    return ret_ptr;
  }

  // アンパッサンの位置をセットする。
  LispObjectPtr EngineSuite::SetEnPassantSquare
  (LispObjectPtr en_passant_square_ptr) {
    // 引数がNilだった場合、アンパッサンの位置を無効にする。
    if (en_passant_square_ptr->IsNil()) {
      Square origin = engine_ptr_->en_passant_square();
      LispObjectPtr ret_ptr = LispObject::NewNil();
      if (origin) {
        ret_ptr = LispObject::NewSymbol(SQUARE_SYMBOL[origin]);
      }
      engine_ptr_->en_passant_square(0);

      return ret_ptr;
    }

    Square square = en_passant_square_ptr->number_value();

    // 引数をチェック。
    if (square >= NUM_SQUARES) {
      throw LispObject::GenError("@engine_error", "The square value '"
      + std::to_string(square)
      + "' doesn't indicate any square.");
    }

    // 位置が0でなければフィルタリング。
    if ((engine_ptr_->blocker_0() & Util::SQUARE[square])) {
      throw LispObject::GenError("@engine_error",
      "'" + SQUARE_SYMBOL[square] + "' is not empty.");
    }
    Rank rank = Util::SQUARE_TO_RANK[square];
    if (!((rank == RANK_3) || (rank == RANK_6))) {
      throw LispObject::GenError("@engine_error",
      "The rank of square must be 'RANK_3' or 'RANK_6'. you indicated '"
      + RANK_SYMBOL[rank] + "'.");
    }
    if (rank == RANK_3) {
      Square target = square + 8;
      if (!(engine_ptr_->position()[WHITE][PAWN] & Util::SQUARE[target])) {
        throw LispObject::GenError("@engine_error",
        "White Pawn doesn't exist on '" + SQUARE_SYMBOL[target] + "' .");
      }
    } else if (rank == RANK_6) {
      Square target = square - 8;
      if (!(engine_ptr_->position()[BLACK][PAWN] & Util::SQUARE[target])) {
        throw LispObject::GenError("@engine_error",
        "Black Pawn doesn't exist on '" + SQUARE_SYMBOL[target] + "' .");
      }
    }

    Square origin = engine_ptr_->en_passant_square();
    LispObjectPtr ret_ptr = LispObject::NewNil();
    if (origin) {
      ret_ptr = LispObject::NewSymbol(SQUARE_SYMBOL[origin]);
    }
    engine_ptr_->en_passant_square(square);

    return ret_ptr;
  }

  // 1手指す。
  LispObjectPtr EngineSuite::PlayMove(const LispObject& caller,
  const std::string& func_name, LispObjectPtr move_ptr) {
    LispIterator itr(move_ptr.get());

    // 引数をチェック。
    // fromをチェック。
    if (!itr) {
      throw LispObject::GenError
      ("@engine_error", "Couldn't find 'From' value.");
    }
    LispObjectPtr from_ptr = caller.Evaluate(*(itr++));
    if (!(from_ptr->IsNumber())) {
      throw LispObject::GenWrongTypeError
      (func_name, "Number", std::vector<int> {2, 1}, true);
    }
    Square from = from_ptr->number_value();
    if (from >= NUM_SQUARES) {
      throw LispObject::GenError("@engine_error", "The 'From' value '"
      + std::to_string(from) + "' doesn't indicate any square.");
    }

    // toをチェック。
    if (!itr) {
      throw LispObject::GenError
      ("@engine_error", "Couldn't find 'To' value.");
    }
    LispObjectPtr to_ptr = caller.Evaluate(*(itr++));
    if (!(to_ptr->IsNumber())) {
      throw LispObject::GenWrongTypeError
      (func_name, "Number", std::vector<int> {2, 2}, true);
    }
    Square to = to_ptr->number_value();
    if (to >= NUM_SQUARES) {
      throw LispObject::GenError("@engine_error", "The 'To' value '"
      + std::to_string(to) + "' doesn't indicate any square.");
    }

    // promotionをチェック。
    if (!itr) {
      throw LispObject::GenError
      ("@engine_error", "Couldn't find 'Promotion' value.");
    }
    LispObjectPtr promotion_ptr = caller.Evaluate(*itr);
    if (!(promotion_ptr->IsNumber())) {
      throw LispObject::GenWrongTypeError
      (func_name, "Number", std::vector<int> {2, 3}, true);
    }
    PieceType promotion = promotion_ptr->number_value();
    if (promotion >= NUM_PIECE_TYPES) {
      throw LispObject::GenError("@engine_error", "The 'Promotion' value '"
      + std::to_string(promotion) + "' doesn't indicate any piece type.");
    }

    Move move = 0;
    SetFrom(move, from);
    SetTo(move, to);
    SetPromotion(move, promotion);

    try {
      engine_ptr_->PlayMove(move);
    } catch (SayuriError error) {
      throw LispObject::GenError("@engine_error",
      "'(" + SQUARE_SYMBOL[from] + " " + SQUARE_SYMBOL[to] + " "
      + PIECE_TYPE_SYMBOL[promotion] + ")' is not legal move.");
    }

    return LispObject::NewBoolean(true);
  }

  // 手を戻す。
  LispObjectPtr EngineSuite::UndoMove() {
    Move move = 0;
    try {
      move = engine_ptr_->UndoMove();
    } catch (SayuriError error) {
      throw LispObject::GenError("@engine_error", "Couldn't undo,"
      " because there are no moves in the engine's move history table.");
    }

    LispObjectPtr ret_ptr = LispObject::NewList(3);
    ret_ptr->car(LispObject::NewSymbol(SQUARE_SYMBOL[GetFrom(move)]));
    ret_ptr->cdr()->car(LispObject::NewSymbol(SQUARE_SYMBOL[GetTo(move)]));
    ret_ptr->cdr()->cdr()->car
    (LispObject::NewSymbol(PIECE_TYPE_SYMBOL[GetPromotion(move)]));

    return ret_ptr;
  }

  // UCIコマンドを入力する。
  LispObjectPtr EngineSuite::InputUCICommand(LispObjectPtr command_ptr) {
    return LispObject::NewBoolean
    (shell_ptr_->InputCommand(command_ptr->string_value()));
  }

  // UCIアウトプットリスナーを登録する。
  LispObjectPtr EngineSuite::AddUCIOutputListener(const LispObject& caller,
  const LispObject& symbol) {
    // コールバック用S式を作成。
    LispObjectPtr s_expr = LispObject::NewList(2);
    s_expr->car(symbol.Clone());
    s_expr->cdr()->car(LispObject::NewString(""));

    // 呼び出し元のポインタ。
    LispObjectPtr caller_ptr = caller.Clone();

    // リスナーを作成。
    std::function<void(const std::string&)> callback =
    [this, s_expr, caller_ptr](const std::string& message) {
      s_expr->cdr()->car()->string_value(message);
      caller_ptr->Evaluate(*s_expr);
    };

    callback_vec_.push_back(callback);

    return LispObject::NewBoolean(true);
  }

  // move_timeミリ秒間思考する。 最善手が見つかるまで戻らない。
  LispObjectPtr EngineSuite::GoMoveTime(const std::string& func_name,
  const LispObject& move_time, const LispObject& move_list) {
    int move_time_2 = move_time.number_value();
    if (move_time_2 < 0) {
      throw LispObject::GenError("@engine_error",
      "Move time must be 0 milliseconds and more. Given "
      + std::to_string(move_time_2) + " milliseconds.");
    }

    return GetBestMove(MAX_PLYS, MAX_NODES, move_time_2,
    MoveListToVec(func_name, move_list));
  }

  // 持ち時間time(ミリ秒)で思考する。 最善手が見つかるまで戻らない。
  LispObjectPtr EngineSuite::GoTimeLimit(const std::string& func_name,
  const LispObject& time, const LispObject& move_list) {
    int time_2 = time.number_value();
    if (time_2 < 0) {
      throw LispObject::GenError("@engine_error",
      "Time limit must be 0 milliseconds and more. Given "
      + std::to_string(time_2) + " milliseconds.");
    }

    return GetBestMove(MAX_PLYS, MAX_NODES, TimeLimitToMoveTime(time_2),
    MoveListToVec(func_name, move_list));
  }

  // 深さdepthまで思考する。 最善手が見つかるまで戻らない。
  LispObjectPtr EngineSuite::GoDepth(const std::string& func_name,
  const LispObject& depth, const LispObject& move_list) {
    int depth_2 = depth.number_value();
    if (depth_2 < 0) {
      throw LispObject::GenError("@engine_error",
      "Depth must be 0 and more. Given "
      + std::to_string(depth_2) + ".");
    }

    return GetBestMove(depth_2, MAX_NODES, INT_MAX,
    MoveListToVec(func_name, move_list));
  }

  // nodesのノード数まで思考する。 最善手が見つかるまで戻らない。
  LispObjectPtr EngineSuite::GoNodes(const std::string& func_name,
  const LispObject& nodes, const LispObject& move_list) {
    long long nodes_2 = nodes.number_value();
    if (nodes_2 < 0) {
      throw LispObject::GenError("@engine_error",
      "Nodes must be 0 and more. Given "
      + std::to_string(nodes_2) + ".");
    }

    return GetBestMove(MAX_PLYS, nodes_2, INT_MAX,
    MoveListToVec(func_name, move_list));
  }

  // SearchParams - マテリアル。
  LispObjectPtr EngineSuite::SetMaterial(const LispObject& material_list) {
    // 長さをチェック。
    unsigned int len = material_list.Length();
    if (len > 0) {
      if (len < 7) {
        throw LispObject::GenError("@engine_error",
        "Not enough length of material list. Needs 7. Given "
        + std::to_string(len) + ".");
      }
    }

    // 返すオブジェクトを作成しながら配列にセットする。
    int material[NUM_PIECE_TYPES] {0, 0, 0, 0, 0, 0, 0};

    LispObjectPtr ret_ptr = LispObject::NewList(7);
    LispObject* ptr = ret_ptr.get();
    ptr->car(LispObject::NewNumber(0));  // Emptyの分。
    ptr = ptr->cdr().get();

    LispIterator itr(&material_list);
    ++itr;  // Emptyの分を飛ばす。
    for (PieceType piece_type = PAWN; piece_type < NUM_PIECE_TYPES;
    ++piece_type) {
      ptr->car
      (LispObject::NewNumber(search_params_ptr_->material()[piece_type]));

      ptr = ptr->cdr().get();

      if (len != 0) {
        material[piece_type] = (itr++)->number_value();
      }
    }

    // セットする。
    if (len != 0) {
      search_params_ptr_->material(material);
    }

    return ret_ptr;
  }

  // ======== //
  // Sayulisp //
  // ======== //
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  Sayulisp::Sayulisp() :
  dict_ptr_(new HelpDict()),
  global_ptr_(LispObject::GenGlobal(dict_ptr_)) {
    // EngineSuiteを作成する関数を作成。
    LispObject::SetBasicFunctions(global_ptr_, dict_ptr_);

    LispObjectPtr func_ptr = LispObject::NewNativeFunction();
    func_ptr->scope_chain(global_ptr_->scope_chain());
    func_ptr->native_function([this]
    (LispObjectPtr self, const LispObject& caller, const LispObject& list)
    -> LispObjectPtr {
      return this->GenEngine();
    });

    global_ptr_->BindSymbol("gen-engine", func_ptr);

    // 定数をバインドしていく。
    // マスの定数をバインド。
    FOR_SQUARES(square) {
      global_ptr_->BindSymbol(EngineSuite::SQUARE_SYMBOL[square],
      LispObject::NewNumber(square));
    }

    // ファイルの定数をバインド。
    FOR_FYLES(fyle) {
      global_ptr_->BindSymbol(EngineSuite::FYLE_SYMBOL[fyle],
      LispObject::NewNumber(fyle));
    }

    // ランクの定数をバインド。
    FOR_RANKS(rank) {
      global_ptr_->BindSymbol(EngineSuite::RANK_SYMBOL[rank],
      LispObject::NewNumber(rank));
    }

    // サイドの定数をバインド。
    FOR_SIDES(side) {
      global_ptr_->BindSymbol(EngineSuite::SIDE_SYMBOL[side],
      LispObject::NewNumber(side));
    }

    // 駒の定数をバインド。
    FOR_PIECE_TYPES(piece_type) {
      global_ptr_->BindSymbol(EngineSuite::PIECE_TYPE_SYMBOL[piece_type],
      LispObject::NewNumber(piece_type));
    }

    // キャスリングの権利の定数をバインド。
    for (int i = 0; i < 5; ++i) {
      global_ptr_->BindSymbol(EngineSuite::CASTLING_SYMBOL[i],
      LispObject::NewNumber(i));
    }

    // ヘルプ辞書を作成。
    SetHelp();
  }
  // コピーコンストラクタ。
  Sayulisp::Sayulisp(const Sayulisp& sayulisp) :
  dict_ptr_(sayulisp.dict_ptr_), global_ptr_(sayulisp.global_ptr_) {}
  // ムーブコンストラクタ。
  Sayulisp::Sayulisp(Sayulisp&& sayulisp) :
  dict_ptr_(std::move(sayulisp.dict_ptr_)),
  global_ptr_(std::move(sayulisp.global_ptr_)) {}
  // コピー代入演算子。
  Sayulisp& Sayulisp::operator=(const Sayulisp& sayulisp) {
    dict_ptr_ = sayulisp.dict_ptr_;
    global_ptr_ = sayulisp.global_ptr_;
    return *this;
  }
  // ムーブ代入演算子。
  Sayulisp& Sayulisp::operator=(Sayulisp&& sayulisp) {
    dict_ptr_ = std::move(sayulisp.dict_ptr_);
    global_ptr_ = std::move(sayulisp.global_ptr_);
    return *this;
  }

  // Sayulispを開始する。
  int Sayulisp::Run(std::istream* stream_ptr) {
    // 終了ステータス。
    int status = 0;

    // (exit)関数を作成。
    bool loop = true;
    LispObjectPtr exit_func_ptr = LispObject::NewNativeFunction();
    exit_func_ptr->scope_chain(global_ptr_->scope_chain());
    exit_func_ptr->native_function([&status, &loop]
    (LispObjectPtr self, const LispObject& caller, const LispObject& list)
    -> LispObjectPtr {
      // 準備。
      LispIterator list_itr(&list);
      std::string func_name = (list_itr++)->ToString();

      // ループをセット。
      loop = false;

      // 引数があった場合は終了ステータスあり。
      if (list_itr) {
        LispObjectPtr status_ptr = caller.Evaluate(*list_itr);
        if (!(status_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        status = status_ptr->number_value();
      }

      return LispObject::NewNumber(status);
    });
    global_ptr_->BindSymbol("exit", exit_func_ptr);

    LispTokenizer tokenizer;

    try {
      std::string input;
      while (std::getline(*(stream_ptr), input)) {
        input += "\n";
        int parentheses = tokenizer.Analyse(input);
        if (parentheses == 0) {
          std::queue<std::string> token_queue = tokenizer.token_queue();
          std::vector<LispObjectPtr> ptr_vec = LispObject::Parse(token_queue);

          for (auto& ptr : ptr_vec) {
            global_ptr_->Evaluate(*ptr);
            if (!loop) break;
          }

          tokenizer.Reset();
        } else if (parentheses < 0) {
          throw LispObject::GenError("@parse-error", "Too many parentheses.");
        }

        if (!loop) break;
      }
    } catch (LispObjectPtr error) {
      if (error->IsList() && (error->Length() == 2)
      && (error->car()->IsSymbol()) && (error->cdr()->car()->IsString())) {
        std::cerr << "Error: " << error->car()->symbol_value() << std::endl;
        std::cerr << error->cdr()->car()->string_value() << std::endl;
        throw error;
      } else {
        throw error;
      }
    }

    return status;
  }

  // エンジンを生成する。
  LispObjectPtr Sayulisp::GenEngine() {
    // スイートを作成。
    std::shared_ptr<EngineSuite> suite_ptr(new EngineSuite());

    // ネイティブ関数オブジェクトを作成。
    LispObjectPtr ret_ptr = LispObject::NewNativeFunction();
    ret_ptr->scope_chain(global_ptr_->scope_chain());
    ret_ptr->native_function([suite_ptr]
    (LispObjectPtr self, const LispObject& caller, const LispObject& list)
    -> LispObjectPtr {
      return (*suite_ptr)(self, caller, list);
    });

    return ret_ptr;
  }

  // ヘルプを作成する。
  void Sayulisp::SetHelp() {
    // --- 定数 --- //
    (*dict_ptr_)["A1"] =
R"...(### A1 ###

__Description__

* Constant value of Number that indicates A1 square.
* Value is '0'.)...";

    (*dict_ptr_)["B1"] =
R"...(### B1 ###

__Description__

* Constant value of Number that indicates B1 square.
* Value is '1'.)...";

    (*dict_ptr_)["C1"] =
R"...(### C1 ###

__Description__

* Constant value of Number that indicates C1 square.
* Value is '2'.)...";

    (*dict_ptr_)["D1"] =
R"...(### D1 ###

__Description__

* Constant value of Number that indicates D1 square.
* Value is '3'.)...";

    (*dict_ptr_)["E1"] =
R"...(### E1 ###

__Description__

* Constant value of Number that indicates E1 square.
* Value is '4'.)...";

    (*dict_ptr_)["F1"] =
R"...(### F1 ###

__Description__

* Constant value of Number that indicates F1 square.
* Value is '5'.)...";

    (*dict_ptr_)["G1"] =
R"...(### G1 ###

__Description__

* Constant value of Number that indicates G1 square.
* Value is '6'.)...";

    (*dict_ptr_)["H1"] =
R"...(### H1 ###

__Description__

* Constant value of Number that indicates H1 square.
* Value is '7'.)...";
    (*dict_ptr_)["A2"] =
R"...(### A2 ###

__Description__

* Constant value of Number that indicates A2 square.
* Value is '8'.)...";

    (*dict_ptr_)["B2"] =
R"...(### B2 ###

__Description__

* Constant value of Number that indicates B2 square.
* Value is '9'.)...";

    (*dict_ptr_)["C2"] =
R"...(### C2 ###

__Description__

* Constant value of Number that indicates C2 square.
* Value is '10'.)...";

    (*dict_ptr_)["D2"] =
R"...(### D2 ###

__Description__

* Constant value of Number that indicates D2 square.
* Value is '11'.)...";

    (*dict_ptr_)["E2"] =
R"...(### E2 ###

__Description__

* Constant value of Number that indicates E2 square.
* Value is '12'.)...";

    (*dict_ptr_)["F2"] =
R"...(### F2 ###

__Description__

* Constant value of Number that indicates F2 square.
* Value is '13'.)...";

    (*dict_ptr_)["G2"] =
R"...(### G2 ###

__Description__

* Constant value of Number that indicates G2 square.
* Value is '14'.)...";

    (*dict_ptr_)["H2"] =
R"...(### H2 ###

__Description__

* Constant value of Number that indicates H2 square.
* Value is '15'.)...";

    (*dict_ptr_)["A3"] =
R"...(### A3 ###

__Description__

* Constant value of Number that indicates A3 square.
* Value is '16'.)...";

    (*dict_ptr_)["B3"] =
R"...(### B3 ###

__Description__

* Constant value of Number that indicates B3 square.
* Value is '17'.)...";

    (*dict_ptr_)["C3"] =
R"...(### C3 ###

__Description__

* Constant value of Number that indicates C3 square.
* Value is '18'.)...";

    (*dict_ptr_)["D3"] =
R"...(### D3 ###

__Description__

* Constant value of Number that indicates D3 square.
* Value is '19'.)...";

    (*dict_ptr_)["E3"] =
R"...(### E3 ###

__Description__

* Constant value of Number that indicates E3 square.
* Value is '20'.)...";

    (*dict_ptr_)["F3"] =
R"...(### F3 ###

__Description__

* Constant value of Number that indicates F3 square.
* Value is '21'.)...";

    (*dict_ptr_)["G3"] =
R"...(### G3 ###

__Description__

* Constant value of Number that indicates G3 square.
* Value is '22'.)...";

    (*dict_ptr_)["H3"] =
R"...(### H3 ###

__Description__

* Constant value of Number that indicates H3 square.
* Value is '23'.)...";

    (*dict_ptr_)["A4"] =
R"...(### A4 ###

__Description__

* Constant value of Number that indicates A4 square.
* Value is '24'.)...";

    (*dict_ptr_)["B4"] =
R"...(### B4 ###

__Description__

* Constant value of Number that indicates B4 square.
* Value is '25'.)...";

    (*dict_ptr_)["C4"] =
R"...(### C4 ###

__Description__

* Constant value of Number that indicates C4 square.
* Value is '26'.)...";

    (*dict_ptr_)["D4"] =
R"...(### D4 ###

__Description__

* Constant value of Number that indicates D4 square.
* Value is '27'.)...";

    (*dict_ptr_)["E4"] =
R"...(### E4 ###

__Description__

* Constant value of Number that indicates E4 square.
* Value is '28'.)...";

    (*dict_ptr_)["F4"] =
R"...(### F4 ###

__Description__

* Constant value of Number that indicates F4 square.
* Value is '29'.)...";

    (*dict_ptr_)["G4"] =
R"...(### G4 ###

__Description__

* Constant value of Number that indicates G4 square.
* Value is '30'.)...";

    (*dict_ptr_)["H4"] =
R"...(### H4 ###

__Description__

* Constant value of Number that indicates H4 square.
* Value is '31'.)...";

    (*dict_ptr_)["A5"] =
R"...(### A5 ###

__Description__

* Constant value of Number that indicates A5 square.
* Value is '32'.)...";

    (*dict_ptr_)["B5"] =
R"...(### B5 ###

__Description__

* Constant value of Number that indicates B5 square.
* Value is '33'.)...";

    (*dict_ptr_)["C5"] =
R"...(### C5 ###

__Description__

* Constant value of Number that indicates C5 square.
* Value is '34'.)...";

    (*dict_ptr_)["D5"] =
R"...(### D5 ###

__Description__

* Constant value of Number that indicates D5 square.
* Value is '35'.)...";

    (*dict_ptr_)["E5"] =
R"...(### E5 ###

__Description__

* Constant value of Number that indicates E5 square.
* Value is '36'.)...";

    (*dict_ptr_)["F5"] =
R"...(### F5 ###

__Description__

* Constant value of Number that indicates F5 square.
* Value is '37'.)...";

    (*dict_ptr_)["G5"] =
R"...(### G5 ###

__Description__

* Constant value of Number that indicates G5 square.
* Value is '38'.)...";

    (*dict_ptr_)["H5"] =
R"...(### H5 ###

__Description__

* Constant value of Number that indicates H5 square.
* Value is '39'.)...";

    (*dict_ptr_)["A6"] =
R"...(### A6 ###

__Description__

* Constant value of Number that indicates A6 square.
* Value is '40'.)...";

    (*dict_ptr_)["B6"] =
R"...(### B6 ###

__Description__

* Constant value of Number that indicates B6 square.
* Value is '41'.)...";

    (*dict_ptr_)["C6"] =
R"...(### C6 ###

__Description__

* Constant value of Number that indicates C6 square.
* Value is '42'.)...";

    (*dict_ptr_)["D6"] =
R"...(### D6 ###

__Description__

* Constant value of Number that indicates D6 square.
* Value is '43'.)...";

    (*dict_ptr_)["E6"] =
R"...(### E6 ###

__Description__

* Constant value of Number that indicates E6 square.
* Value is '44'.)...";

    (*dict_ptr_)["F6"] =
R"...(### F6 ###

__Description__

* Constant value of Number that indicates F6 square.
* Value is '45'.)...";

    (*dict_ptr_)["G6"] =
R"...(### G6 ###

__Description__

* Constant value of Number that indicates G6 square.
* Value is '46'.)...";

    (*dict_ptr_)["H6"] =
R"...(### H6 ###

__Description__

* Constant value of Number that indicates H6 square.
* Value is '47'.)...";

    (*dict_ptr_)["A7"] =
R"...(### A7 ###

__Description__

* Constant value of Number that indicates A7 square.
* Value is '48'.)...";

    (*dict_ptr_)["B7"] =
R"...(### B7 ###

__Description__

* Constant value of Number that indicates B7 square.
* Value is '49'.)...";

    (*dict_ptr_)["C7"] =
R"...(### C7 ###

__Description__

* Constant value of Number that indicates C7 square.
* Value is '50'.)...";

    (*dict_ptr_)["D7"] =
R"...(### D7 ###

__Description__

* Constant value of Number that indicates D7 square.
* Value is '51'.)...";

    (*dict_ptr_)["E7"] =
R"...(### E7 ###

__Description__

* Constant value of Number that indicates E7 square.
* Value is '52'.)...";

    (*dict_ptr_)["F7"] =
R"...(### F7 ###

__Description__

* Constant value of Number that indicates F7 square.
* Value is '53'.)...";

    (*dict_ptr_)["G7"] =
R"...(### G7 ###

__Description__

* Constant value of Number that indicates G7 square.
* Value is '54'.)...";

    (*dict_ptr_)["H7"] =
R"...(### H7 ###

__Description__

* Constant value of Number that indicates H7 square.
* Value is '55'.)...";

    (*dict_ptr_)["A8"] =
R"...(### A8 ###

__Description__

* Constant value of Number that indicates A8 square.
* Value is '56'.)...";

    (*dict_ptr_)["B8"] =
R"...(### B8 ###

__Description__

* Constant value of Number that indicates B8 square.
* Value is '57'.)...";

    (*dict_ptr_)["C8"] =
R"...(### C8 ###

__Description__

* Constant value of Number that indicates C8 square.
* Value is '58'.)...";

    (*dict_ptr_)["D8"] =
R"...(### D8 ###

__Description__

* Constant value of Number that indicates D8 square.
* Value is '59'.)...";

    (*dict_ptr_)["E8"] =
R"...(### E8 ###

__Description__

* Constant value of Number that indicates E8 square.
* Value is '60'.)...";

    (*dict_ptr_)["F8"] =
R"...(### F8 ###

__Description__

* Constant value of Number that indicates F8 square.
* Value is '61'.)...";

    (*dict_ptr_)["G8"] =
R"...(### G8 ###

__Description__

* Constant value of Number that indicates G8 square.
* Value is '62'.)...";

    (*dict_ptr_)["H8"] =
R"...(### H8 ###

__Description__

* Constant value of Number that indicates H8 square.
* Value is '63'.)...";

    (*dict_ptr_)["FYLE_A"] =
R"...(### FYLE_A ###

__Description__

* Constant value of Number that indicates A-fyle.
* Value is '0'.)...";

    (*dict_ptr_)["FYLE_B"] =
R"...(### FYLE_B ###

__Description__

* Constant value of Number that indicates B-fyle.
* Value is '1'.)...";

    (*dict_ptr_)["FYLE_C"] =
R"...(### FYLE_C ###

__Description__

* Constant value of Number that indicates C-fyle.
* Value is '2'.)...";

    (*dict_ptr_)["FYLE_D"] =
R"...(### FYLE_D ###

__Description__

* Constant value of Number that indicates D-fyle.
* Value is '3'.)...";

    (*dict_ptr_)["FYLE_E"] =
R"...(### FYLE_E ###

__Description__

* Constant value of Number that indicates E-fyle.
* Value is '4'.)...";

    (*dict_ptr_)["FYLE_F"] =
R"...(### FYLE_F ###

__Description__

* Constant value of Number that indicates F-fyle.
* Value is '5'.)...";

    (*dict_ptr_)["FYLE_G"] =
R"...(### FYLE_G ###

__Description__

* Constant value of Number that indicates G-fyle.
* Value is '6'.)...";

    (*dict_ptr_)["FYLE_H"] =
R"...(### FYLE_H ###

__Description__

* Constant value of Number that indicates H-fyle.
* Value is '7'.)...";

    (*dict_ptr_)["RANK_1"] =
R"...(### RANK_1 ###

__Description__

* Constant value of Number that indicates the 1st rank.
* Value is '0'.)...";

    (*dict_ptr_)["RANK_2"] =
R"...(### RANK_2 ###

__Description__

* Constant value of Number that indicates the 2nd rank.
* Value is '1'.)...";

    (*dict_ptr_)["RANK_3"] =
R"...(### RANK_3 ###

__Description__

* Constant value of Number that indicates the 3rd rank.
* Value is '2'.)...";

    (*dict_ptr_)["RANK_4"] =
R"...(### RANK_4 ###

__Description__

* Constant value of Number that indicates the 4th rank.
* Value is '3'.)...";

    (*dict_ptr_)["RANK_5"] =
R"...(### RANK_5 ###

__Description__

* Constant value of Number that indicates the 5th rank.
* Value is '4'.)...";

    (*dict_ptr_)["RANK_6"] =
R"...(### RANK_6 ###

__Description__

* Constant value of Number that indicates the 6th rank.
* Value is '5'.)...";

    (*dict_ptr_)["RANK_7"] =
R"...(### RANK_7 ###

__Description__

* Constant value of Number that indicates the 7th rank.
* Value is '6'.)...";

    (*dict_ptr_)["RANK_8"] =
R"...(### RANK_8 ###

__Description__

* Constant value of Number that indicates the 8th rank.
* Value is '7'.)...";

    (*dict_ptr_)["NO_SIDE"] =
R"...(### NO_SIDE ###

__Description__

* Constant value of Number that indicates neither of sides.
* Value is '0'.)...";

    (*dict_ptr_)["WHITE"] =
R"...(### WHITE ###

__Description__

* Constant value of Number that indicates White.
* Value is '1'.)...";

    (*dict_ptr_)["BLACK"] =
R"...(### BLACK ###

__Description__

* Constant value of Number that indicates Black.
* Value is '2'.)...";

    (*dict_ptr_)["EMPTY"] =
R"...(### EMPTY ###

__Description__

* Constant value of Number that indicates no piece.
* Value is '0'.)...";

    (*dict_ptr_)["PAWN"] =
R"...(### PAWN ###

__Description__

* Constant value of Number that indicates Pawn.
* Value is '1'.)...";

    (*dict_ptr_)["KNIGHT"] =
R"...(### KNIGHT ###

__Description__

* Constant value of Number that indicates Knight.
* Value is '2'.)...";

    (*dict_ptr_)["BISHOP"] =
R"...(### BISHOP ###

__Description__

* Constant value of Number that indicates Bishop.
* Value is '3'.)...";

    (*dict_ptr_)["ROOK"] =
R"...(### ROOK ###

__Description__

* Constant value of Number that indicates Rook.
* Value is '4'.)...";

    (*dict_ptr_)["QUEEN"] =
R"...(### QUEEN ###

__Description__

* Constant value of Number that indicates Queen.
* Value is '5'.)...";

    (*dict_ptr_)["KING"] =
R"...(### KING ###

__Description__

* Constant value of Number that indicates King.
* Value is '6'.)...";

    (*dict_ptr_)["NO_CASTLING"] =
R"...(### NO_CASTLING ###

__Description__

* Constant value of Number that indicates no one to castle.
* Value is '0'.)...";

    (*dict_ptr_)["WHITE_SHORT_CASTLING"] =
R"...(### WHITE_SHORT_CASTLING ###

__Description__

* Constant value of Number that indicates White's Short Castling.
* Value is '1'.)...";

    (*dict_ptr_)["WHITE_LONG_CASTLING"] =
R"...(### WHITE_LONG_CASTLING ###

__Description__

* Constant value of Number that indicates White's Long Castling.
* Value is '2'.)...";

    (*dict_ptr_)["BLACK_SHORT_CASTLING"] =
R"...(### BLACK_SHORT_CASTLING ###

__Description__

* Constant value of Number that indicates Black's Short Castling.
* Value is '3'.)...";

    (*dict_ptr_)["BLACK_LONG_CASTLING"] =
R"...(### BLACK_LONG_CASTLING ###

__Description__

* Constant value of Number that indicates Black's Long Castling.
* Value is '4'.)...";

    // %%% exit
    (*dict_ptr_)["exit"] =
R"...(### exit ###

__Usage__

* `(exit [<Status : Number>])`

__Description__

* Exit from Sayulisp.
* `<Status>` is Exit Status. Default is '0'.

__Example__

    ;; Exit from Sayulisp.
    (exit)
    
    ;; Exit with EXIT_FAILURE.
    (exit 1))...";

    // %%% gen-engine
    (*dict_ptr_)["gen-engine"] =
R"...(### gen-engine ###

__Usage__

1. `(gen-engine)`
2. `((gen-engine) <Message Symbol> [<Arguments>...])`

__Description__

* 1: Generate chess engine.
* 2: The engine executes something according to `<Message Symbol>`.
* 2: Some `<Message Symbol>` require `<Argument>...`.
* 2: `<Message Symbol>` are...
    + `@get-white-pawn-position`
        - Return List of Symbols of square placed White Pawn.

    + `@get-white-knight-position`
        - Return List of Symbols of square placed White Knight.

    + `@get-white-bishop-position`
        - Return List of Symbols of square placed White Bishop.

    + `@get-white-rook-position`
        - Return List of Symbols of square placed White Rook.

    + `@get-white-queen-position`
        - Return List of Symbols of square placed White Queen.

    + `@get-white-king-position`
        - Return List of Symbols of square placed White King.

    + `@get-black-pawn-position`
        - Return List of Symbols of square placed Black Pawn.

    + `@get-black-knight-position`
        - Return List of Symbols of square placed Black Knight.

    + `@get-black-bishop-position`
        - Return List of Symbols of square placed Black Bishop.

    + `@get-black-rook-position`
        - Return List of Symbols of square placed Black Rook.

    + `@get-black-queen-position`
        - Return List of Symbols of square placed Black Queen.

    + `@get-black-king-position`
        - Return List of Symbols of square placed Black King.

    + `@get-empty-square-position`
        - Return List of Symbols of empty square.

    + `@get-piece`
        - This needs one argument.
            - A constant value that indicates one square.
        - Return what kind of piece is placed at the square
          as `(<Side : Symbol> <PieceType : Symbol>)`.

    + `@get-to-move`
        - Return Symbol of side which has its turn to move.

    + `@get-castling-rights`
        - Return what castling rights exist as List of Symbols.

    + `@get-en-passant-square`
        - Return Symbol of En Passant Square.

    + `@get-ply`
        - Return how many moves have played as plies. (1 move = 2 plies)

    + `@get-clock`
        - Return moves for 50 Moves Rule as plies. (1 move = 2 plies)

    + `@get-white-has-castled`
        - Return #t if White has castled. If not, it returns #f.

    + `@get-black-has-castled`
        - Return #t if Black has castled. If not, it returns #f.

    + `@set-new-game`
        - Set starting position and state.
        - Return #t.

    + `@set-fen <FEN : String>`
        - Set position and state into indicated position by `<FEN>`.
        - Return #t.

    + `@get-candidate-moves`
        - Return List that contains the candidate moves.

    + `@place-piece <Square : Number> <Piece type : Number>
      <Piece side : Number>`
        - Place a `<Piece side>` `<Piece type>` onto `<Square>`.
        - If `<Piece type>` is `EMPTY` and `<Piece side>` is `NO_SIDE`,
          a piece on `<Square>` is deleted.
        - Each side must have just one King,
            - If you try to place a piece onto a square where King is placed,
              Sayulisp will throw error.
            - If you place a new King, the old King will be deleted.

    + `@set-to-move <Side : Number>`
        - Change turn to move into `<Side>`.
        - Return the previous value.
        - `<Side>` must be White or Black.

    + `@set-castling-rights <Castling rights : List>`
        - Change castling rights into `<List>`.
        - Return the previous value.
        - `<List>` is consist of constant values that are
          'WHITE_SHORT_CASTLING' or 'WHITE_LONG_CASTLING'
          or 'BLACK_SHORT_CASTLING' or 'BLACK_LONG_CASTLING'.
        - After calling this function,
          castling rights is updated by position of King or Rook.
            - If King is not on starting square,
              its castling rights are deleted.
            - If Rook is not on starting square,
              its King Side or Queen Side of castling rights is deleted.

    + `@set-en-passant-square <<Square : Number> or <Nil>>`
        - Change en passant square into `<Square>`.
        - If argument is `<Nil>`, it changes into no en passant square.
        - Return the previous value.
        - But it doesn't change by following conditions.
            - `<Square>` is not on 'RANK_3' or on 'RANK_6'.
            - `<Square>` is not empty.
            - `<Square>` is on 'RANK_3' and
              White Pawn is not on directly above.
            - `<Square>` is on 'RANK_6' and
              Black Pawn is not on directly below.

    + `@set-ply <Ply : Number>`
        - Change ply(0.5 moves) into `<Ply>`.
        - Return the previous value.
        - `<Ply>` must be '1' and more.
      
    + `@set-clock <Ply : Number>`
        - Change ply(0.5 moves) of 50 moves rule into `<Ply>`.
        - Return the previous value.
        - `<Ply>` must be '0' and more.

    + `@correct-position?`
        - Judge if the position is correct or not.
            - If Pawn is on RANK_1 or RANK_8, then it returns `#f`.
            - If White's turn and Black King is checked, then it returns `#f`.
            - If Black's turn and White King is checked, then it returns `#f`.
            - Otherwise it returns `#t`.

    + `@white-checked?`
        - Judge if White King is checked or not.
      
    + `@black-checked?`
        - Judge if Black King is checked or not.
      
    + `@checkmated?`
        - Judge if either King is checkmated or not.
      
    + `@stalemated?`
        - Judge if either King is stalemated or not.

    + `@play-move <Move : List>`
        - Play one move.
        - `<Move>` is `(<From : Number> <To : Number> <Promotion : Number>)`.
            - If it doesn't promote the piece, `<Promotion>` is 'Empty'.
        - If `<Move>` is illegal, it throws an exception.

    + `@undo-move`
        - Undo the previous move.
        - If theres no previous move in the engine's move history table,
          it throws an exception.

    + `@input-uci-command <UCI command : String>`
        - Execute UCI command.
        - If the command has succeeded, it returns '#t'. Otherwise '#f'.
        - The output from engine is put into the callback registered
          by `@add-uci-output-listener`.
        - If you input "go" command, the thinking process runs on other thread.
          (It means the engine returns your control immediately.)
      
    + `@add-uci-output-listener <Callback : Function>`
        - Register a callback to listen the UCI engine's output.
        - `<Callback>` must recieve 1 argument that is UCI output as String.

    + `@go-movetime <Milliseconds : Number> [<Candidate moves list : List>]`
        - Think for `<Milliseconds>` and return the best move.
        - Different from "go movetime ..." with `@input-uci-command`,
          the thinking process waits until the engine to return best move.
        - `<Candidate moves list>` is a list composed of
          `(<From : Number> <To : Number> <Promotion : Number>)`.
            - If it doesn't promote the piece, `<Promotion>` is 'Empty'.

    + `@go-timelimit <Milliseconds : Number> [<Candidate moves list : List>]`
        - Think by `<Milliseconds>` and return the best move.
            - If `<Milliseconds>` is less than '600000' (10 minutes.),
              the engine thinks for '`<Milliseconds>` / 10'.
              Otherwise '60000' milliseconds (1 minute).
        - Different from "go wtime (or btime) ..." with `@input-uci-command`,
          the thinking process waits until the engine to return best move.
        - `<Candidate moves list>` is a list composed of
          `(<From : Number> <To : Number> <Promotion : Number>)`.
            - If it doesn't promote the piece, `<Promotion>` is 'Empty'.

    + `@go-depth <Ply : Number> [<Candidate moves list : List>]`
        - Think until to reach `<Ply>`th depth and return the best move.
        - Different from "go depth ..." with `@input-uci-command`,
          the thinking process waits until the engine to return best move.
        - `<Candidate moves list>` is a list composed of
          `(<From : Number> <To : Number> <Promotion : Number>)`.
            - If it doesn't promote the piece, `<Promotion>` is 'Empty'.

    + `@go-nodes <Nodes : Number> [<Candidate moves list : List>]`
        - Think for `<Nodes>` nodes and return the best move.
        - Different from "go nodes ..." with `@input-uci-command`,
          the thinking process waits until the engine to return best move.
        - `<Candidate moves list>` is a list composed of
          `(<From : Number> <To : Number> <Promotion : Number>)`.
            - If it doesn't promote the piece, `<Promotion>` is 'Empty'.

    + `@set-hash-size <Size : Number>`
        - Set size of Transposition Table(Hash Table) and return previous size.
      
    + `@set-threads <Number of Threads : Number>`
        - Set number of threads and return previous number.

    + `@material [<New material : List>]`
        - Return score list of material.
            - 1st : Empty. (It's always 0)
            - 2nd : Pawn.
            - 3rd : Knight.
            - 4th : Bishop.
            - 5th : Rook.
            - 6th : Queen.
            - 7th : King.
        - If you have specified `<New material>`,
          it sets new score of material.

    + `@enable-quiesce-search [<New setting : Boolean>]`
        - Return setting that Quiesce Search is enabled or disabled.
            - If it returns '#t', it is enabled. Otherwise disabled.
        - If you have specified `<New setting>`,
          it sets new setting to enable or disable Quiesce Search.

    + `@enable-repetition-check [<New setting : Boolean>]`
        - Return setting that Repetition Check is enabled or disabled.
            - If it returns '#t', it is enabled. Otherwise disabled.
        - If you have specified `<New setting>`,
          it sets new setting to enable or disable Repetition Check.

    + `@enable-check-extension [<New setting : Boolean>]`
        - Return setting that Check Extension is enabled or disabled.
            - If it returns '#t', it is enabled. Otherwise disabled.
        - If you have specified `<New setting>`,
          it sets new setting to enable or disable Check Extension.

    + `@ybwc-limit-depth [<New depth : Number>]`
        - Return depth (ply) that
          if the search function reaches a node on the remaining depth,
          it doesn't do YBWC in the deeper nodes.
        - If you have specified `<New depth>`, it sets new depth.

    + `@ybwc-invalid-moves [<New number of moves : Number>]`
        - Return number of moves that the search function invalidates YBWC
          while the number of candidate moves from 1st move in a node.
        - If you have specified `<New number of moves>`,
          it sets new number of of moves.

    + `@enable-aspiration-windows [<New setting : Boolean>]`
        - Return setting that Aspiration Windows is enabled or disabled.
            - If it returns '#t', it is enabled. Otherwise disabled.
        - If you have specified `<New setting>`,
          it sets new setting to enable or disable Aspiration Windows.

    + `@aspiration-windows-limit-depth [<New depth : Number>]`
        - Return depth (ply) that
          if depth of Iterative Deepening is less than it,
          the search function doesn't do Aspiration Windows on root node.
        - If you have specified `<New depth>`, it sets new depth.

    + `@aspiration-windows-delta [<New delta : Number>]`
        - Return a value that
          if the search function have occurred fail-hight on root node,
          the function adds delta value to beta value and searches again.
        - If you have specified `<New delta>`, it sets new delta value.
      
__Example__

    ;; Generate chess engine and bind to 'my-engine'.
    (define my-engine (gen-engine))
    
    ;; List squares placed White Pawn.
    (display (my-engine '@get-white-pawn-position))
    
    ;; Output
    ;;
    ;; > (A2 B2 C2 D2 E2 F2 G2 H2))...";
  }
}  // namespace Sayuri
