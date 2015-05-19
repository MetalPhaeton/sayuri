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

    SetWeightFunctions();
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

    SetWeightFunctions();
  }

  // ムーブコンストラクタ。
  EngineSuite::EngineSuite(EngineSuite&& suite) :
  search_params_ptr_(std::move(suite.search_params_ptr_)),
  eval_params_ptr_(std::move(suite.eval_params_ptr_)),
  engine_ptr_(std::move(suite.engine_ptr_)),
  table_ptr_(std::move(suite.table_ptr_)),
  shell_ptr_(std::move(suite.shell_ptr_)) {
    SetWeightFunctions();
  }

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

    SetWeightFunctions();

    return *this;
  }

  // ムーブ代入演算子。
  EngineSuite& EngineSuite::operator=(EngineSuite&& suite) {
    search_params_ptr_ = std::move(suite.search_params_ptr_);
    eval_params_ptr_ = std::move(suite.eval_params_ptr_);
    engine_ptr_ = std::move(suite.engine_ptr_);
    table_ptr_ = std::move(suite.table_ptr_);
    shell_ptr_ = std::move(suite.shell_ptr_);

    SetWeightFunctions();

    return *this;
  }

  // ウェイト関数オブジェクトをセット。
  void EngineSuite::SetWeightFunctions() {
    weight_1_accessor_[WEIGHT_OPENING_POSITION] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_opening_position();
    };

    weight_1_accessor_[WEIGHT_ENDING_POSITION] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_ending_position();
    };

    weight_1_accessor_[WEIGHT_MOBILITY] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_mobility();
    };

    weight_1_accessor_[WEIGHT_CENTER_CONTROL] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_center_control();
    };

    weight_1_accessor_[WEIGHT_SWEET_CENTER_CONTROL] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_sweet_center_control();
    };

    weight_1_accessor_[WEIGHT_DEVELOPMENT] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_development();
    };

    weight_1_accessor_[WEIGHT_ATTACK] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_attack();
    };

    weight_1_accessor_[WEIGHT_DEFENSE] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_defense();
    };

    weight_1_accessor_[WEIGHT_PIN] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_pin();
    };

    weight_1_accessor_[WEIGHT_ATTACK_AROUND_KING] =
    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
      return this->eval_params_ptr_->weight_attack_around_king();
    };

    weight_2_accessor_[WEIGHT_PASS_PAWN] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_pass_pawn();
    };

    weight_2_accessor_[WEIGHT_PROTECTED_PASS_PAWN] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_protected_pass_pawn();
    };

    weight_2_accessor_[WEIGHT_DOUBLE_PAWN] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_double_pawn();
    };

    weight_2_accessor_[WEIGHT_ISO_PAWN] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_iso_pawn();
    };

    weight_2_accessor_[WEIGHT_PAWN_SHIELD] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_pawn_shield();
    };

    weight_2_accessor_[WEIGHT_BISHOP_PAIR] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_bishop_pair();
    };

    weight_2_accessor_[WEIGHT_BAD_BISHOP] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_bad_bishop();
    };

    weight_2_accessor_[WEIGHT_ROOK_PAIR] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_rook_pair();
    };

    weight_2_accessor_[WEIGHT_ROOK_SEMIOPEN_FYLE] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_rook_semiopen_fyle();
    };

    weight_2_accessor_[WEIGHT_ROOK_OPEN_FYLE] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_rook_open_fyle();
    };

    weight_2_accessor_[WEIGHT_EARLY_QUEEN_STARTING] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_early_queen_starting();
    };

    weight_2_accessor_[WEIGHT_WEAK_SQUARE] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_weak_square();
    };

    weight_2_accessor_[WEIGHT_CASTLING] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_castling();
    };

    weight_2_accessor_[WEIGHT_ABANDONED_CASTLING] =
    [this]() -> const Weight& {
      return this->eval_params_ptr_->weight_abandoned_castling();
    };

    weight_1_mutator_[WEIGHT_OPENING_POSITION] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_opening_position(weights);
    };

    weight_1_mutator_[WEIGHT_ENDING_POSITION] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_ending_position(weights);
    };

    weight_1_mutator_[WEIGHT_MOBILITY] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_mobility(weights);
    };

    weight_1_mutator_[WEIGHT_CENTER_CONTROL] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_center_control(weights);
    };

    weight_1_mutator_[WEIGHT_SWEET_CENTER_CONTROL] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_sweet_center_control(weights);
    };

    weight_1_mutator_[WEIGHT_DEVELOPMENT] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_development(weights);
    };

    weight_1_mutator_[WEIGHT_ATTACK] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_attack(weights);
    };

    weight_1_mutator_[WEIGHT_DEFENSE] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_defense(weights);
    };

    weight_1_mutator_[WEIGHT_PIN] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_pin(weights);
    };

    weight_1_mutator_[WEIGHT_ATTACK_AROUND_KING] =
    [this](const Weight (& weights)[NUM_PIECE_TYPES]) {
      this->eval_params_ptr_->weight_attack_around_king(weights);
    };

    weight_2_mutator_[WEIGHT_PASS_PAWN] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_pass_pawn(weight);
    };

    weight_2_mutator_[WEIGHT_PROTECTED_PASS_PAWN] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_protected_pass_pawn(weight);
    };

    weight_2_mutator_[WEIGHT_DOUBLE_PAWN] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_double_pawn(weight);
    };

    weight_2_mutator_[WEIGHT_ISO_PAWN] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_iso_pawn(weight);
    };

    weight_2_mutator_[WEIGHT_PAWN_SHIELD] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_pawn_shield(weight);
    };

    weight_2_mutator_[WEIGHT_BISHOP_PAIR] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_bishop_pair(weight);
    };

    weight_2_mutator_[WEIGHT_BAD_BISHOP] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_bad_bishop(weight);
    };

    weight_2_mutator_[WEIGHT_ROOK_PAIR] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_rook_pair(weight);
    };

    weight_2_mutator_[WEIGHT_ROOK_SEMIOPEN_FYLE] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_rook_semiopen_fyle(weight);
    };

    weight_2_mutator_[WEIGHT_ROOK_OPEN_FYLE] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_rook_open_fyle(weight);
    };

    weight_2_mutator_[WEIGHT_EARLY_QUEEN_STARTING] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_early_queen_starting(weight);
    };

    weight_2_mutator_[WEIGHT_WEAK_SQUARE] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_weak_square(weight);
    };

    weight_2_mutator_[WEIGHT_CASTLING] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_castling(weight);
    };

    weight_2_mutator_[WEIGHT_ABANDONED_CASTLING] =
    [this](const Weight& weight) {
      this->eval_params_ptr_->weight_abandoned_castling(weight);
    };
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
      return GetPosition<WHITE, PAWN>();

    } else if (message_symbol == "@get-white-knight-position") {
      return GetPosition<WHITE, KNIGHT>();

    } else if (message_symbol == "@get-white-bishop-position") {
      return GetPosition<WHITE, BISHOP>();

    } else if (message_symbol == "@get-white-rook-position") {
      return GetPosition<WHITE, ROOK>();

    } else if (message_symbol == "@get-white-queen-position") {
      return GetPosition<WHITE, QUEEN>();

    } else if (message_symbol == "@get-white-king-position") {
      return GetPosition<WHITE, KING>();

    } else if (message_symbol == "@get-black-pawn-position") {
      return GetPosition<BLACK, PAWN>();

    } else if (message_symbol == "@get-black-knight-position") {
      return GetPosition<BLACK, KNIGHT>();

    } else if (message_symbol == "@get-black-bishop-position") {
      return GetPosition<BLACK, BISHOP>();

    } else if (message_symbol == "@get-black-rook-position") {
      return GetPosition<BLACK, ROOK>();

    } else if (message_symbol == "@get-black-queen-position") {
      return GetPosition<BLACK, QUEEN>();

    } else if (message_symbol == "@get-black-king-position") {
      return GetPosition<BLACK, KING>();

    } else if (message_symbol == "@get-empty-square-position") {
      return GetPosition<NO_SIDE, EMPTY>();

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
        throw LispObject::GenError("@engine-error",
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

    } else if (message_symbol == "@enable-see") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableSEE(*enable_ptr);

    } else if (message_symbol == "@enable-history") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableHistory(*enable_ptr);

    } else if (message_symbol == "@enable-killer") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableKiller(*enable_ptr);

    } else if (message_symbol == "@enable-hash-table") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableHashTable(*enable_ptr);

    } else if (message_symbol == "@enable-iid") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableIID(*enable_ptr);

    } else if (message_symbol == "@iid-limit-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetIIDLimitDepth(*depth_ptr);

    } else if (message_symbol == "@iid-search-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetIIDSearchDepth(*depth_ptr);

    } else if (message_symbol == "@enable-nmr") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableNMR(*enable_ptr);

    } else if (message_symbol == "@nmr-limit-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetNMRLimitDepth(*depth_ptr);

    } else if (message_symbol == "@nmr-search-reduction") {
      LispObjectPtr reduction_ptr = LispObject::NewNil();
      if (list_itr) {
        reduction_ptr = caller.Evaluate(*list_itr);
        if (!(reduction_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetNMRSearchReduction(*reduction_ptr);

    } else if (message_symbol == "@nmr-reduction") {
      LispObjectPtr reduction_ptr = LispObject::NewNil();
      if (list_itr) {
        reduction_ptr = caller.Evaluate(*list_itr);
        if (!(reduction_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetNMRReduction(*reduction_ptr);

    } else if (message_symbol == "@enable-probcut") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableProbCut(*enable_ptr);

    } else if (message_symbol == "@probcut-limit-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetProbCutLimitDepth(*depth_ptr);

    } else if (message_symbol == "@probcut-margin") {
      LispObjectPtr margin_ptr = LispObject::NewNil();
      if (list_itr) {
        margin_ptr = caller.Evaluate(*list_itr);
        if (!(margin_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetProbCutMargin(*margin_ptr);

    } else if (message_symbol == "@probcut-search-reduction") {
      LispObjectPtr reduction_ptr = LispObject::NewNil();
      if (list_itr) {
        reduction_ptr = caller.Evaluate(*list_itr);
        if (!(reduction_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetProbCutSearchReduction(*reduction_ptr);

    } else if (message_symbol == "@enable-history-pruning") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableHistoryPruning(*enable_ptr);

    } else if (message_symbol == "@history-pruning-limit-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetHistoryPruningLimitDepth(*depth_ptr);

    } else if (message_symbol == "@history-pruning-move-threshold") {
      LispObjectPtr threshold_ptr = LispObject::NewNil();
      if (list_itr) {
        threshold_ptr = caller.Evaluate(*list_itr);
        if (!(threshold_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetHistoryPruningMoveThreshold(*threshold_ptr);

    } else if (message_symbol == "@history-pruning-invalid-moves") {
      LispObjectPtr num_moves_ptr = LispObject::NewNil();
      if (list_itr) {
        num_moves_ptr = caller.Evaluate(*list_itr);
        if (!(num_moves_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetHistoryPruningInvalidMoves(*num_moves_ptr);

    } else if (message_symbol == "@history-pruning-threshold") {
      LispObjectPtr threshold_ptr = LispObject::NewNil();
      if (list_itr) {
        threshold_ptr = caller.Evaluate(*list_itr);
        if (!(threshold_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetHistoryPruningThreshold(*threshold_ptr);

    } else if (message_symbol == "@history-pruning-reduction") {
      LispObjectPtr reduction_ptr = LispObject::NewNil();
      if (list_itr) {
        reduction_ptr = caller.Evaluate(*list_itr);
        if (!(reduction_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetHistoryPruningReduction(*reduction_ptr);

    } else if (message_symbol == "@enable-lmr") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableLMR(*enable_ptr);

    } else if (message_symbol == "@lmr-limit-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetLMRLimitDepth(*depth_ptr);

    } else if (message_symbol == "@lmr-move-threshold") {
      LispObjectPtr threshold_ptr = LispObject::NewNil();
      if (list_itr) {
        threshold_ptr = caller.Evaluate(*list_itr);
        if (!(threshold_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetLMRMoveThreshold(*threshold_ptr);

    } else if (message_symbol == "@lmr-invalid-moves") {
      LispObjectPtr num_moves_ptr = LispObject::NewNil();
      if (list_itr) {
        num_moves_ptr = caller.Evaluate(*list_itr);
        if (!(num_moves_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetLMRInvalidMoves(*num_moves_ptr);

    } else if (message_symbol == "@lmr-search-reduction") {
      LispObjectPtr reduction_ptr = LispObject::NewNil();
      if (list_itr) {
        reduction_ptr = caller.Evaluate(*list_itr);
        if (!(reduction_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetLMRSearchReduction(*reduction_ptr);

    } else if (message_symbol == "@enable-futility-pruning") {
      LispObjectPtr enable_ptr = LispObject::NewNil();
      if (list_itr) {
        enable_ptr = caller.Evaluate(*list_itr);
        if (!(enable_ptr->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {2}, true);
        }
      }

      return SetEnableFutilityPruning(*enable_ptr);

    } else if (message_symbol == "@futility-pruning-depth") {
      LispObjectPtr depth_ptr = LispObject::NewNil();
      if (list_itr) {
        depth_ptr = caller.Evaluate(*list_itr);
        if (!(depth_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetFutilityPruningDepth(*depth_ptr);

    } else if (message_symbol == "@futility-pruning-margin") {
      LispObjectPtr margin_ptr = LispObject::NewNil();
      if (list_itr) {
        margin_ptr = caller.Evaluate(*list_itr);
        if (!(margin_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }
      }

      return SetFutilityPruningMargin(*margin_ptr);

    } else if ((message_symbol == "@pawn-square-table-opening")
    || (message_symbol == "@knight-square-table-opening")
    || (message_symbol == "@bishop-square-table-opening")
    || (message_symbol == "@rook-square-table-opening")
    || (message_symbol == "@queen-square-table-opening")
    || (message_symbol == "@king-square-table-opening")) {
      LispObjectPtr table_ptr = LispObject::NewNil();
      if (list_itr) {
        table_ptr = caller.Evaluate(*list_itr);
        if (!(table_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      if (message_symbol == "@pawn-square-table-opening") {
        return SetPieceSquareTableOpening<PAWN>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@knight-square-table-opening") {
        return SetPieceSquareTableOpening<KNIGHT>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@bishop-square-table-opening") {
        return SetPieceSquareTableOpening<BISHOP>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@rook-square-table-opening") {
        return SetPieceSquareTableOpening<ROOK>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@queen-square-table-opening") {
        return SetPieceSquareTableOpening<QUEEN>
        (func_name, message_symbol, *table_ptr);
      } else {
        return SetPieceSquareTableOpening<KING>
        (func_name, message_symbol, *table_ptr);
      }

    } else if ((message_symbol == "@pawn-square-table-ending")
    || (message_symbol == "@knight-square-table-ending")
    || (message_symbol == "@bishop-square-table-ending")
    || (message_symbol == "@rook-square-table-ending")
    || (message_symbol == "@queen-square-table-ending")
    || (message_symbol == "@king-square-table-ending")) {
      LispObjectPtr table_ptr = LispObject::NewNil();
      if (list_itr) {
        table_ptr = caller.Evaluate(*list_itr);
        if (!(table_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      if (message_symbol == "@pawn-square-table-ending") {
        return SetPieceSquareTableEnding<PAWN>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@knight-square-table-ending") {
        return SetPieceSquareTableEnding<KNIGHT>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@bishop-square-table-ending") {
        return SetPieceSquareTableEnding<BISHOP>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@rook-square-table-ending") {
        return SetPieceSquareTableEnding<ROOK>
        (func_name, message_symbol, *table_ptr);
      } else if (message_symbol == "@queen-square-table-ending") {
        return SetPieceSquareTableEnding<QUEEN>
        (func_name, message_symbol, *table_ptr);
      } else {
        return SetPieceSquareTableEnding<KING>
        (func_name, message_symbol, *table_ptr);
      }

    } else if ((message_symbol == "@pawn-attack-table")
    || (message_symbol == "@knight-attack-table")
    || (message_symbol == "@bishop-attack-table")
    || (message_symbol == "@rook-attack-table")
    || (message_symbol == "@queen-attack-table")
    || (message_symbol == "@king-attack-table")) {
      LispObjectPtr value_list_ptr = LispObject::NewNil();
      if (list_itr) {
        value_list_ptr = caller.Evaluate(*list_itr);
        if (!(value_list_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      if (message_symbol == "@pawn-attack-table") {
        return SetAttackValueTable<PAWN>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@knight-attack-table") {
        return SetAttackValueTable<KNIGHT>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@bishop-attack-table") {
        return SetAttackValueTable<BISHOP>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@rook-attack-table") {
        return SetAttackValueTable<ROOK>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@queen-attack-table") {
        return SetAttackValueTable<QUEEN>
        (func_name, message_symbol, *value_list_ptr);
      } else {
        return SetAttackValueTable<KING>
        (func_name, message_symbol, *value_list_ptr);
      }

    } else if ((message_symbol == "@pawn-defense-table")
    || (message_symbol == "@knight-defense-table")
    || (message_symbol == "@bishop-defense-table")
    || (message_symbol == "@rook-defense-table")
    || (message_symbol == "@queen-defense-table")
    || (message_symbol == "@king-defense-table")) {
      LispObjectPtr value_list_ptr = LispObject::NewNil();
      if (list_itr) {
        value_list_ptr = caller.Evaluate(*list_itr);
        if (!(value_list_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      if (message_symbol == "@pawn-defense-table") {
        return SetDefenseValueTable<PAWN>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@knight-defense-table") {
        return SetDefenseValueTable<KNIGHT>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@bishop-defense-table") {
        return SetDefenseValueTable<BISHOP>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@rook-defense-table") {
        return SetDefenseValueTable<ROOK>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@queen-defense-table") {
        return SetDefenseValueTable<QUEEN>
        (func_name, message_symbol, *value_list_ptr);
      } else {
        return SetDefenseValueTable<KING>
        (func_name, message_symbol, *value_list_ptr);
      }

    } else if ((message_symbol == "@bishop-pin-table")
    || (message_symbol == "@rook-pin-table")
    || (message_symbol == "@queen-pin-table")) {
      LispObjectPtr value_list_ptr = LispObject::NewNil();
      if (list_itr) {
        value_list_ptr = caller.Evaluate(*list_itr);
        if (!(value_list_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      if (message_symbol == "@bishop-pin-table") {
        return SetPinValueTable<BISHOP>
        (func_name, message_symbol, *value_list_ptr);
      } else if (message_symbol == "@rook-pin-table") {
        return SetPinValueTable<ROOK>
        (func_name, message_symbol, *value_list_ptr);
      } else {
        return SetPinValueTable<QUEEN>
        (func_name, message_symbol, *value_list_ptr);
      }

    } else if (message_symbol == "@pawn-shield-table") {
      LispObjectPtr table_ptr = LispObject::NewNil();
      if (list_itr) {
        table_ptr = caller.Evaluate(*list_itr);
        if (!(table_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      return SetPawnShieldValueTable
      (func_name, message_symbol, *table_ptr);

    } else if ((message_symbol == "@weight-pawn-mobility")
    || (message_symbol == "@weight-knight-mobility")
    || (message_symbol == "@weight-bishop-mobility")
    || (message_symbol == "@weight-rook-mobility")
    || (message_symbol == "@weight-queen-mobility")
    || (message_symbol == "@weight-king-mobility")
    || (message_symbol == "@weight-pawn-center-control")
    || (message_symbol == "@weight-knight-center-control")
    || (message_symbol == "@weight-bishop-center-control")
    || (message_symbol == "@weight-rook-center-control")
    || (message_symbol == "@weight-queen-center-control")
    || (message_symbol == "@weight-king-center-control")
    || (message_symbol == "@weight-pawn-sweet-center-control")
    || (message_symbol == "@weight-knight-sweet-center-control")
    || (message_symbol == "@weight-bishop-sweet-center-control")
    || (message_symbol == "@weight-rook-sweet-center-control")
    || (message_symbol == "@weight-queen-sweet-center-control")
    || (message_symbol == "@weight-king-sweet-center-control")
    || (message_symbol == "@weight-pawn-development")
    || (message_symbol == "@weight-knight-development")
    || (message_symbol == "@weight-bishop-development")
    || (message_symbol == "@weight-rook-development")
    || (message_symbol == "@weight-queen-development")
    || (message_symbol == "@weight-king-development")
    || (message_symbol == "@weight-pawn-attack")
    || (message_symbol == "@weight-knight-attack")
    || (message_symbol == "@weight-bishop-attack")
    || (message_symbol == "@weight-rook-attack")
    || (message_symbol == "@weight-queen-attack")
    || (message_symbol == "@weight-king-attack")
    || (message_symbol == "@weight-pawn-defense")
    || (message_symbol == "@weight-knight-defense")
    || (message_symbol == "@weight-bishop-defense")
    || (message_symbol == "@weight-rook-defense")
    || (message_symbol == "@weight-queen-defense")
    || (message_symbol == "@weight-king-defense")
    || (message_symbol == "@weight-bishop-pin")
    || (message_symbol == "@weight-rook-pin")
    || (message_symbol == "@weight-queen-pin")
    || (message_symbol == "@weight-pawn-attack-around-king")
    || (message_symbol == "@weight-knight-attack-around-king")
    || (message_symbol == "@weight-bishop-attack-around-king")
    || (message_symbol == "@weight-rook-attack-around-king")
    || (message_symbol == "@weight-queen-attack-around-king")
    || (message_symbol == "@weight-king-attack-around-king")
    || (message_symbol == "@weight-pass-pawn")
    || (message_symbol == "@weight-protected-pass-pawn")
    || (message_symbol == "@weight-double-pawn")
    || (message_symbol == "@weight-iso-pawn")
    || (message_symbol == "@weight-pawn-shield")
    || (message_symbol == "@weight-bishop-pair")
    || (message_symbol == "@weight-bad-bishop")
    || (message_symbol == "@weight-rook-pair")
    || (message_symbol == "@weight-rook-semiopen-fyle")
    || (message_symbol == "@weight-rook-open-fyle")
    || (message_symbol == "@weight-early-queen-starting")
    || (message_symbol == "@weight-weak-square")
    || (message_symbol == "@weight-castling")
    || (message_symbol == "@weight-abandoned-castling")
    ) {
      LispObjectPtr weight_params_ptr = LispObject::NewNil();
      if (list_itr) {
        weight_params_ptr = caller.Evaluate(*list_itr);
        if (!(weight_params_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }
      }

      if (message_symbol == "@weight-pawn-mobility") {
        return SetWeight1<WEIGHT_MOBILITY, PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-knight-mobility") {
        return SetWeight1<WEIGHT_MOBILITY, KNIGHT>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-mobility") {
        return SetWeight1<WEIGHT_MOBILITY, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-mobility") {
        return SetWeight1<WEIGHT_MOBILITY, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-mobility") {
        return SetWeight1<WEIGHT_MOBILITY, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-king-mobility") {
        return SetWeight1<WEIGHT_MOBILITY, KING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pawn-center-control") {
        return SetWeight1<WEIGHT_CENTER_CONTROL, PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-knight-center-control") {
        return SetWeight1<WEIGHT_CENTER_CONTROL, KNIGHT>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-center-control") {
        return SetWeight1<WEIGHT_CENTER_CONTROL, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-center-control") {
        return SetWeight1<WEIGHT_CENTER_CONTROL, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-center-control") {
        return SetWeight1<WEIGHT_CENTER_CONTROL, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-king-center-control") {
        return SetWeight1<WEIGHT_CENTER_CONTROL, KING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pawn-sweet-center-control") {
        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-knight-sweet-center-control") {
        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, KNIGHT>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-sweet-center-control") {
        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-sweet-center-control") {
        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-sweet-center-control") {
        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-king-sweet-center-control") {
        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, KING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pawn-development") {
        return SetWeight1<WEIGHT_DEVELOPMENT, PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-knight-development") {
        return SetWeight1<WEIGHT_DEVELOPMENT, KNIGHT>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-development") {
        return SetWeight1<WEIGHT_DEVELOPMENT, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-development") {
        return SetWeight1<WEIGHT_DEVELOPMENT, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-development") {
        return SetWeight1<WEIGHT_DEVELOPMENT, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-king-development") {
        return SetWeight1<WEIGHT_DEVELOPMENT, KING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pawn-attack") {
        return SetWeight1<WEIGHT_ATTACK, PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-knight-attack") {
        return SetWeight1<WEIGHT_ATTACK, KNIGHT>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-attack") {
        return SetWeight1<WEIGHT_ATTACK, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-attack") {
        return SetWeight1<WEIGHT_ATTACK, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-attack") {
        return SetWeight1<WEIGHT_ATTACK, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-king-attack") {
        return SetWeight1<WEIGHT_ATTACK, KING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pawn-defense") {
        return SetWeight1<WEIGHT_DEFENSE, PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-knight-defense") {
        return SetWeight1<WEIGHT_DEFENSE, KNIGHT>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-defense") {
        return SetWeight1<WEIGHT_DEFENSE, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-defense") {
        return SetWeight1<WEIGHT_DEFENSE, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-defense") {
        return SetWeight1<WEIGHT_DEFENSE, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-king-defense"){
        return SetWeight1<WEIGHT_DEFENSE, KING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-pin") {
        return SetWeight1<WEIGHT_PIN, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-pin") {
        return SetWeight1<WEIGHT_PIN, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-pin") {
        return SetWeight1<WEIGHT_PIN, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pawn-attack-around-king") {
        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-knight-attack-around-king") {
        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, KNIGHT>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-attack-around-king") {
        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-attack-around-king") {
        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, ROOK>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-queen-attack-around-king") {
        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, QUEEN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-king-attack-around-king"){
        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, KING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pass-pawn"){
        return SetWeight2<WEIGHT_PASS_PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-protected-pass-pawn"){
        return SetWeight2<WEIGHT_PROTECTED_PASS_PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-double-pawn"){
        return SetWeight2<WEIGHT_DOUBLE_PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-iso-pawn"){
        return SetWeight2<WEIGHT_ISO_PAWN>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-pawn-shield"){
        return SetWeight2<WEIGHT_PAWN_SHIELD>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bishop-pair"){
        return SetWeight2<WEIGHT_BISHOP_PAIR>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-bad-bishop"){
        return SetWeight2<WEIGHT_BAD_BISHOP>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-pair"){
        return SetWeight2<WEIGHT_ROOK_PAIR>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-semiopen-fyle"){
        return SetWeight2<WEIGHT_ROOK_SEMIOPEN_FYLE>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-rook-open-fyle"){
        return SetWeight2<WEIGHT_ROOK_OPEN_FYLE>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-early-queen-starting"){
        return SetWeight2<WEIGHT_EARLY_QUEEN_STARTING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-weak-square"){
        return SetWeight2<WEIGHT_WEAK_SQUARE>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-castling"){
        return SetWeight2<WEIGHT_CASTLING>
        (func_name, message_symbol, *weight_params_ptr);
      } else if (message_symbol == "@weight-abandoned-castling"){
        return SetWeight2<WEIGHT_ABANDONED_CASTLING>
        (func_name, message_symbol, *weight_params_ptr);
      }
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
        throw LispObject::GenError("@engine-error",
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
      throw LispObject::GenError("@engine-error",
      "The square value '" + std::to_string(square)
      + "' doesn't indicate any square.");
    }
    if (piece_type >= NUM_PIECE_TYPES) {
      throw LispObject::GenError("@engine-error",
      "The piece type value '" + std::to_string(piece_type)
      +  "' doesn't indicate any piece type.");
    }
    if (side >= NUM_SIDES) {
      throw LispObject::GenError("@engine-error",
      "The side value '" + std::to_string(side)
      + "' doesn't indicate any side.");
    }
    if ((piece_type && !side) || (!piece_type && side)) {
      throw LispObject::GenError("@engine-error",
      "'" + SIDE_SYMBOL[side] + " " + PIECE_TYPE_SYMBOL[piece_type]
      + "' doesn't exist in the world.");
    }

    // 元の駒の種類とサイドを得る。
    PieceType origin_type = engine_ptr_->piece_board()[square];
    Side origin_side = engine_ptr_->side_board()[square];

    // もし置き換える前の駒がキングなら置き換えられない。
    if (origin_type == KING) {
      throw LispObject::GenError("@engine-error",
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
      throw LispObject::GenError("@engine-error",
      "The side value '" + std::to_string(to_move)
      + "' doesn't indicate any side.");
    }
    if (!to_move) {
      throw LispObject::GenError("@engine-error", "'NO_SIDE' is not allowed.");
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
      throw LispObject::GenError("@engine-error", "The square value '"
      + std::to_string(square)
      + "' doesn't indicate any square.");
    }

    // 位置が0でなければフィルタリング。
    if ((engine_ptr_->blocker_0() & Util::SQUARE[square])) {
      throw LispObject::GenError("@engine-error",
      "'" + SQUARE_SYMBOL[square] + "' is not empty.");
    }
    Rank rank = Util::SQUARE_TO_RANK[square];
    if (!((rank == RANK_3) || (rank == RANK_6))) {
      throw LispObject::GenError("@engine-error",
      "The rank of square must be 'RANK_3' or 'RANK_6'. you indicated '"
      + RANK_SYMBOL[rank] + "'.");
    }
    if (rank == RANK_3) {
      Square target = square + 8;
      if (!(engine_ptr_->position()[WHITE][PAWN] & Util::SQUARE[target])) {
        throw LispObject::GenError("@engine-error",
        "White Pawn doesn't exist on '" + SQUARE_SYMBOL[target] + "' .");
      }
    } else if (rank == RANK_6) {
      Square target = square - 8;
      if (!(engine_ptr_->position()[BLACK][PAWN] & Util::SQUARE[target])) {
        throw LispObject::GenError("@engine-error",
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
      ("@engine-error", "Couldn't find 'From' value.");
    }
    LispObjectPtr from_ptr = caller.Evaluate(*(itr++));
    if (!(from_ptr->IsNumber())) {
      throw LispObject::GenWrongTypeError
      (func_name, "Number", std::vector<int> {2, 1}, true);
    }
    Square from = from_ptr->number_value();
    if (from >= NUM_SQUARES) {
      throw LispObject::GenError("@engine-error", "The 'From' value '"
      + std::to_string(from) + "' doesn't indicate any square.");
    }

    // toをチェック。
    if (!itr) {
      throw LispObject::GenError
      ("@engine-error", "Couldn't find 'To' value.");
    }
    LispObjectPtr to_ptr = caller.Evaluate(*(itr++));
    if (!(to_ptr->IsNumber())) {
      throw LispObject::GenWrongTypeError
      (func_name, "Number", std::vector<int> {2, 2}, true);
    }
    Square to = to_ptr->number_value();
    if (to >= NUM_SQUARES) {
      throw LispObject::GenError("@engine-error", "The 'To' value '"
      + std::to_string(to) + "' doesn't indicate any square.");
    }

    // promotionをチェック。
    if (!itr) {
      throw LispObject::GenError
      ("@engine-error", "Couldn't find 'Promotion' value.");
    }
    LispObjectPtr promotion_ptr = caller.Evaluate(*itr);
    if (!(promotion_ptr->IsNumber())) {
      throw LispObject::GenWrongTypeError
      (func_name, "Number", std::vector<int> {2, 3}, true);
    }
    PieceType promotion = promotion_ptr->number_value();
    if (promotion >= NUM_PIECE_TYPES) {
      throw LispObject::GenError("@engine-error", "The 'Promotion' value '"
      + std::to_string(promotion) + "' doesn't indicate any piece type.");
    }

    Move move = 0;
    SetFrom(move, from);
    SetTo(move, to);
    SetPromotion(move, promotion);

    try {
      engine_ptr_->PlayMove(move);
    } catch (SayuriError error) {
      throw LispObject::GenError("@engine-error",
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
      throw LispObject::GenError("@engine-error", "Couldn't undo,"
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
      throw LispObject::GenError("@engine-error",
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
      throw LispObject::GenError("@engine-error",
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
      throw LispObject::GenError("@engine-error",
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
      throw LispObject::GenError("@engine-error",
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
        throw LispObject::GenError("@engine-error",
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

    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        return this->GenEngine();
      });
      global_ptr_->BindSymbol("gen-engine", func_ptr);
    }


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

    // --- 便利関数 --- //
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->SquareToNumber(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("square->number", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->FyleToNumber(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("fyle->number", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->RankToNumber(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("rank->number", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->SideToNumber(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("side->number", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->PieceTypeToNumber(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("piece->number", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->CastlingToNumber(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("castling->number", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->NumberToSquare(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("number->square", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->NumberToFyle(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("number->fyle", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->NumberToRank(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("number->rank", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->NumberToSide(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("number->side", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->NumberToPiece(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("number->piece", func_ptr);
    }
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain(global_ptr_->scope_chain());
      func_ptr->native_function([this]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 引数チェック。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return this->NumberToCastling(*(caller.Evaluate(*list_itr)));
      });
      global_ptr_->BindSymbol("number->castling", func_ptr);
    }
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

    (*dict_ptr_)["square->number"] =
R"...(### square->number ###

__Usage__

* `(square->number <Object>)`

__Description__

* If `<Object>` is Square Symbol, it returns Number indicating to Square.
* If `<Object>` is List, it returns List changed Square Symbol into Number. 

__Example__

    (define symbol-list
      '(A1 B1 C1 (WHITE D3 E4 (F5 PAWN G6) H7 BLACK_LONG_CASTLING)))
    
    (display (square->number symbol-list))
    ;; Output
    ;; > (0 1 2 (WHITE 19 28 (37 PAWN 46) 55 BLACK_LONG_CASTLING)))...";

    (*dict_ptr_)["fyle->number"] =
R"...(### fyle->number ###

__Usage__

* `(fyle->number <Object>)`

__Description__

* If `<Object>` is Fyle Symbol, it returns Number indicating to Fyle.
* If `<Object>` is List, it returns List changed Fyle Symbol into Number. 

__Example__

    (define symbol-list
      '(FYLE_A FYLE_B (WHITE FYLE_D E4 (PAWN G6) FYLE_H BLACK_LONG_CASTLING)))
    
    (display (fyle->number symbol-list))
    ;; Output
    ;; > (0 1 (WHITE 3 E4 (PAWN G6) 7 BLACK_LONG_CASTLING)))...";

    (*dict_ptr_)["rank->number"] =
R"...(### rank->number ###

__Usage__

* `(rank->number <Object>)`

__Description__

* If `<Object>` is Rank Symbol, it returns Number indicating to Rank.
* If `<Object>` is List, it returns List changed Rank Symbol into Number. 

__Example__

    (define symbol-list
      '(RANK_1 RANK_2 (WHITE RANK_4 E4 (PAWN G6) RANK_8 BLACK_LONG_CASTLING)))
    
    (display (rank->number symbol-list))
    ;; Output
    ;; > (0 1 (WHITE 3 E4 (PAWN G6) 7 BLACK_LONG_CASTLING)))...";

    (*dict_ptr_)["side->number"] =
R"...(### side->number ### {#side-to-number}

__Usage__

* `(side->number <Object>)`

__Description__

* If `<Object>` is Side Symbol, it returns Number indicating to Side.
* If `<Object>` is List, it returns List changed Side Symbol into Number. 

__Example__

    (define symbol-list
      '(NO_SIDE WHITE (FYLE_A BLACK E4 (PAWN G6) BLACK_LONG_CASTLING)))
    
    (display (side->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A 2 E4 (PAWN G6) BLACK_LONG_CASTLING)))...";

    (*dict_ptr_)["piece->number"] =
R"...(### piece->number ###

__Usage__

* `(piece->number <Object>)`

__Description__

* If `<Object>` is Piece Type Symbol, it returns Number indicating
  to Piece Type.
* If `<Object>` is List, it returns List changed Piece Type Symbol into Number. 

__Example__

    (define symbol-list
      '(EMPTY PAWN (FYLE_A QUEEN E4 (RANK_4 G6) KING BLACK_LONG_CASTLING)))
    
    (display (piece->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A 5 E4 (RANK_4 G6) 6 BLACK_LONG_CASTLING)))...";

    (*dict_ptr_)["castling->number"] =
R"...(### castling->number ###

__Usage__

* `(castling->number <Object>)`

__Description__

* If `<Object>` is Castling Right Symbol, it returns Number indicating
  to Piece Type.
* If `<Object>` is List, it returns List changed Castling Right Symbol
  into Number. 

__Example__

    (define symbol-list
      '(NO_CASTLING WHITE_SHORT_CASTLING (FYLE_A E4 (RANK_4 G6) KING)))
    
    (display (castling->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A E4 (RANK_4 G6) KING)))...";

    (*dict_ptr_)["number->square"] =
R"...(### number->square ### {#number-to-square}

__Usage__

* `(number->square <Object>)`

__Description__

* If `<Object>` is Number, it returns Square Symbol.
* If `<Object>` is List, it returns List changed Number into Square Symbol.

__Example__

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->square number-list))
    ;; Output
    ;; > (A1 B1 (C1 (D1 E1 "Hello") F1) 100))...";

    (*dict_ptr_)["number->fyle"] =
R"...(### number->fyle ### {#number-to-fyle}

__Usage__

* `(number->fyle <Object>)`

__Description__

* If `<Object>` is Number, it returns Fyle Symbol.
* If `<Object>` is List, it returns List changed Number into Fyle Symbol.

__Example__

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->fyle number-list))
    ;; Output
    ;; > (FYLE_A FYLE_B (FYLE_C (FYLE_D FYLE_E "Hello") FYLE_F) 100))...";

    (*dict_ptr_)["number->rank"] =
R"...(### number->rank ### {#number-to-rank}

__Usage__

* `(number->rank <Object>)`

__Description__

* If `<Object>` is Number, it returns Rank Symbol.
* If `<Object>` is List, it returns List changed Number into Rank Symbol.

__Example__

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->rank number-list))
    ;; Output
    ;; > (RANK_1 RANK_2 (RANK_3 (RANK_4 RANK_5 "Hello") RANK_6) 100))...";

    (*dict_ptr_)["number->side"] =
R"...(### number->side ### {#number-to-side}

__Usage__

* `(number->side <Object>)`

__Description__

* If `<Object>` is Number, it returns Side Symbol.
* If `<Object>` is List, it returns List changed Number into Side Symbol.

__Example__

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->side number-list))
    ;; Output
    ;; > (NO_SIDE WHITE (BLACK (3 4 "Hello") 5) 100))...";

    (*dict_ptr_)["number->piece"] =
R"...(### number->piece ### {#number-to-piece}

__Usage__

* `(number->piece <Object>)`

__Description__

* If `<Object>` is Number, it returns Piece Type Symbol.
* If `<Object>` is List, it returns List changed Number into Piece Type Symbol.

__Example__

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->piece number-list))
    ;; Output
    ;; > (EMPTY PAWN (KNIGHT (BISHOP ROOK "Hello") QUEEN) 100))...";

    (*dict_ptr_)["number->castling"] =
R"...(### number->castling ### {#number-to-castling}

__Usage__

* `(number->castling <Object>)`

__Description__

* If `<Object>` is Number, it returns Castling Rights Symbol.
* If `<Object>` is List, it returns List changed Number
  into CAstling Rights Symbol.

__Example__

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->castling number-list))
    ;; Output
    ;; > (NO_CASTLING WHITE_SHORT_CASTLING (WHITE_LONG_CASTLING
    ;; > (BLACK_SHORT_CASTLING BLACK_LONG_CASTLING "Hello") 5) 100))...";

    // %%% gen-engine
    (*dict_ptr_)["gen-engine"] =
R"...(### gen-engine ###

__Usage__

1. `(gen-engine)`
2. `((gen-engine) <Message Symbol> [<Arguments>...])`

__Description__

* 1: Generates chess engine.
* 2: The engine executes something according to `<Message Symbol>`.
* 2: Some `<Message Symbol>` require `<Argument>...`.

__Description of Message Symbols__

* `@get-white-pawn-position`
    + Returns List of position of White Pawns as Symbol.
* `@get-white-knight-position`
    + Returns List of position of White Knights as Symbol.
* `@get-white-bishop-position`
    + Returns List of position of White Bishops as Symbol.
* `@get-white-rook-position`
    + Returns List of position of White Rooks as Symbol.
* `@get-white-queen-position`
    + Returns List of position of White Queens as Symbol.
* `@get-white-king-position`
    + Returns List of position of White King as Symbol.
* `@get-black-pawn-position`
    + Returns List of position of White Pawns as Symbol.
* `@get-black-knight-position`
    + Returns List of position of White Knights as Symbol.
* `@get-black-bishop-position`
    + Returns List of position of White Bishops as Symbol.
* `@get-black-rook-position`
    + Returns List of position of White Rooks as Symbol.
* `@get-black-queen-position`
    + Returns List of position of White Queens as Symbol.
* `@get-black-king-position`
    + Returns List of position of White King as Symbol.
* `@get-empty-square-position`
    + Returns List of position of Empty Squares as Symbol.

__Example__

    (define my-engine (gen-engine))
    (display (my-engine '@get-white-pawn-position))
    
    ;; Output
    ;; > (A2 B2 C2 D2 E2 F2 G2 H2)

* `@get-piece <Square : Number>`
    + Returns a side and type of the piece as List that is
      `(<Side : Symbol>, <Type : Symbol>)`.

__Example__

    (define my-engine (gen-engine))
    (display (my-engine '@get-piece D1))
    
    ;; Output
    ;; > (WHITE QUEEN)

* `@get-to-move`
    + Returns turn to move as Symbol.
* `@get-castling-rights`
    + Returns List of Symbols indicates castling rights.
* `@get-en-passant-square`
    + Returns en passant square as Symbol if it exists now.
* `@get-ply`
    + Returns plies of moves from starting of the game.
    + 1 move = 2 plies.
* `@get-clock`
    + Returns Clock(plies for 50 Moves Rule).
        - If Pawn has moved or a piece has been captured,
          Clock is rewound to zero.
* `@get-white-has-castled`
    + Returns Boolean whether White King has castled or not.
* `@get-black-has-castled`
    + Returns Boolean whether Black King has castled or not.

__Example__

    (define my-engine (gen-engine))
    
    ;; Move pieces by UCI command.
    ;; 1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 4.O-O d5
    ;; +---------------+
    ;; |r . b q k . n r|
    ;; |p p p . . p p p|
    ;; |. . n . . . . .|
    ;; |. . b p p . . .|
    ;; |. . B . P . . .|
    ;; |. . . . . N . .|
    ;; |P P P P . P P P|
    ;; |R N B Q . R K .|
    ;; +---------------+
    (my-engine '@input-uci-command
        "position startpos moves e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 e1g1 d7d5")
    
    (display (my-engine '@get-to-move))
    ;; Output
    ;; > Symbol: WHITE
    
    (display (my-engine '@get-castling-rights))
    ;; Output
    ;; > (BLACK_SHORT_CASTLING BLACK_LONG_CASTLING)
    
    (display (my-engine '@get-en-passant-square))
    ;; Output
    ;; > Symbol: D6
    
    (display (my-engine '@get-ply))
    ;; Output
    ;; > 9
    
    (display (my-engine '@get-clock))
    ;; Output
    ;; > 0
    
    (display (my-engine '@get-white-has-castled))
    ;; Output
    ;; > #t
    
    (display (my-engine '@get-black-has-castled))
    ;; Output
    ;; > #f

* `@set-to-move <Side : Number>`
    + Sets turn to move.
    + Returns previous setting.
* `@set-castling_rights <Castling rights : List>`
    + Sets castling rights.
    + Returns previous setting.
* `@set-en-passant-square <<Square : Number> or <Nil>>`
    + Sets en passant square.
    + Returns previous setting.
* `@set-ply <Ply : Number>`
    + Sets plies(a half of one move).
    + Returns previous setting.
* `@set-clock <Ply : Number>`
    + Sets clock(plies for 50 moves rule).
    + Returns previous setting.

__Example__

    (define my-engine (gen-engine))
    (my-engine '@place-piece E4 PAWN WHITE)
    
    (display (my-engine '@set-to-move BLACK))
    ;; Output
    ;; > Symbol: WHITE
    
    (display (my-engine '@set-castling-rights
        (list WHITE_LONG_CASTLING BLACK_LONG_CASTLING)))
    ;; Output
    ;; > (WHITE_SHORT_CASTLING WHITE_LONG_CASTLING
    ;; > BLACK_SHORT_CASTLING BLACK_LONG_CASTLING)
    
    (display (my-engine '@set-en-passant-square E3))
    ;; Output
    ;; > ()
    
    (display (my-engine '@set-ply 111))
    ;; Output
    ;; > 1
    
    (display (my-engine '@set-clock 22))
    ;; Output
    ;; > 0
    
    ;; ---------------- ;;
    ;; Current Settings ;;
    ;; ---------------- ;;
    
    (display (my-engine '@get-to-move))
    ;; Output
    ;; > Symbol: BLACK
    
    (display (my-engine '@get-castling-rights))
    ;; Output
    ;; > (WHITE_LONG_CASTLING BLACK_LONG_CASTLING)
    
    (display (my-engine '@get-en-passant-square))
    ;; Output
    ;; > Symbol: E3
    
    (display (my-engine '@get-ply))
    ;; Output
    ;; > 111
    
    (display (my-engine '@get-clock))
    ;; Output
    ;; > 22

* `@set-new-game`
    + Sets starting position to the chess engine object.
    + Returns #t.
* `@set-fen <FEN : String>`
    + Sets FEN position to the chess engine object.
    + Returns #t.
* `@place-piece <Square : Number> <Piece type : Number> <Piece side : Number>`
    + Sets a piece on `<Square>`
      and returns the previous piece placed on `<Square>`.
    + `<Piece type>` is piece type.
    + `<Piece side>` is a color of the piece.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@set-new-game))
    ;; Output
    ;; > #t
    
    (display (my-engine '@set-fen
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1"))
    ;; Output
    ;; > #t

* `@get-candidate-moves`
    + Generates and returns List of candidate moves.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@get-candidate-moves))
    ;; Output
    ;; >  ((H2 H4 EMPTY) (H2 H3 EMPTY) (G2 G4 EMPTY) (G2 G3 EMPTY)
    ;; > (F2 F4 EMPTY) (F2 F3 EMPTY) (E2 E4 EMPTY) (E2 E3 EMPTY) (D2 D4 EMPTY)
    ;; > (D2 D3 EMPTY) (C2 C4 EMPTY) (C2 C3 EMPTY) (B2 B4 EMPTY) (B2 B3 EMPTY)
    ;; > (A2 A4 EMPTY) (A2 A3 EMPTY) (G1 H3 EMPTY) (G1 F3 EMPTY) (B1 C3 EMPTY)
    ;; > (B1 A3 EMPTY))

* `@correct-position?`
    + Judges whether it is position or not.
        - If Pawn is on 1st or 8th rank, it returns #f.
        - When turn to move is White, if Black King is checked,
          then it returns #f.
        - When turn to move is Black, if White King is checked,
          then it returns #f.
        - Otherwise, returns #t.

__Example__

    (define my-engine (gen-engine))
    
    ;; Put Pawn on 1st rank.
    (my-engine '@place-piece D1 PAWN WHITE)
    
    (display (my-engine '@correct-position?))
    ;; Output
    ;; > #f

* `@white-checked?`
    + Judges whether White King is checked or not.
* `@black-checked?`
    + Judges whether Black King is checked or not.

__Example__

    (define my-engine (gen-engine))
    
    ;; Move pieces by UCI command.
    ;; 1.d4 e6 2.Nf3 Bb4+
    ;; +---------------+
    ;; |r n b q k . n r|
    ;; |p p p p . p p p|
    ;; |. . . . p . . .|
    ;; |. . . . . . . .|
    ;; |. b . P . . . .|
    ;; |. . . . . N . .|
    ;; |P P P . P P P P|
    ;; |R N B Q K B . R|
    ;; +---------------+
    (my-engine '@input-uci-command
        "position startpos moves d2d4 e7e6 g1f3 f8b4")
    
    (display (my-engine '@white-checked?))
    ;; Output
    ;; > #t
    
    (display (my-engine '@black-checked?))
    ;; Output
    ;; > #f

* `@checkmated?`
    + Judges whether either King is checkmated or not.
* `@stalemated?`
    + Judges whether either King is stalemated or not.

__Example__

    (define my-engine (gen-engine))
    
    ;; Move pieces by UCI command.
    ;; 1.f3 e5 2.g4 Qh4#
    ;; +---------------+
    ;; |r n b . k b n r|
    ;; |p p p p . p p p|
    ;; |. . . . . . . .|
    ;; |. . . . p . . .|
    ;; |. . . . . . P q|
    ;; |. . . . . P . .|
    ;; |P P P P P . . P|
    ;; |R N B Q K B N R|
    ;; +---------------+
    (my-engine '@input-uci-command
        "position startpos moves f2f3 e7e5 g2g4 d8h4")
    
    (display (my-engine '@checkmated?))
    ;; Output
    ;; > #t
    
    (display (my-engine '@stalemated?))
    ;; Output
    ;; > #f

* `@play-move <One move : List>`
    + Moves one piece legally.
    + `<One move>` is `(<From : Number> <To : Number> <Promotion : Number>)`
        - `<From>` is a square which a piece to move is placed on.
        - `<To>` is a square where you want to move the piece to.
        - `<Promotion>` is a piece type which you want to promote Pawn into.
            - If it can't promote Pawn, `<Promotion>` is EMPTY.
        - If move is illegal, it throws exception.
        - Returns #t.
* `@undo-move`
    - Undoes previous move.
    - Returns previous move.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@play-move (list E2 E4 EMPTY)))
    ;; Output
    ;; > #t
    
    (display (my-engine '@get-white-pawn-position))
    ;; Output
    ;; > (A2 B2 C2 D2 F2 G2 H2 E4)
    
    (display (my-engine '@undo-move))
    ;; Output
    ;; > (E2 E4 EMPTY)
    
    (display (my-engine '@get-white-pawn-position))
    ;; Output
    ;; > (A2 B2 C2 D2 E2 F2 G2 H2)

* `@input-uci-command <UCI command : String>`
    + Executes `<UCI command>`.
    + If success, returns #t. Otherwise, returns #f.
    + If you have input "go" command,
      the engine starts to think the best move in background.
      So control will come back soon.
* `@add-uci-output-listener <Listener : Function>`
    + Registers Function to receive UCI output from the engine.
    + `<Listener>` is Function that has one argument(UCI output).

__Example__

    (define my-engine (gen-engine))
    
    ;; Create a listener.
    (define (listener message)
        (display "I'm Listener : " message))
    
    ;; Register the listener.
    (my-engine '@add-uci-output-listener listener)
    
    (display (my-engine '@input-uci-command "uci"))
    ;; Output
    ;; > I'm Listener : id name Sayuri 2015.03.27 devel
    ;; > I'm Listener : id author Hironori Ishibashi
    ;; > I'm Listener : option name Hash type spin default 32 min 8 max 8192
    ;; > I'm Listener : option name Clear Hash type button
    ;; > I'm Listener : option name Ponder type check default true
    ;; > I'm Listener : option name Threads type spin default 1 min 1 max 64
    ;; > I'm Listener : option name UCI_AnalyseMode type check default false
    ;; > I'm Listener : uciok
    ;; > #t

* `@go-movetime <Milliseconds : Number> [<Candidate move list : List>]`
    + Thinks for `<Milliseconds>` and returns the best move.
    + Different from "go" command, until the engine have found the best move,
      the control won't come back.
* `@go-timelimit <Milliseconds : Number> [<Candidate move list : List>]`
    + Thinks on the basis of `<Milliseconds>` and returns the best move.
        - If `<Milliseconds>` is more than 600000,
          the engine thinks for 60000 milliseconds.
        - If `<Milliseconds>` is less than 600000,
          the engine thinks for "`<Milliseconds>` / 10" milliseconds.
    + Different from "go" command, until the engine have found the best move,
      the control won't come back.
* `@go-depth <Ply : Number> [<Candidate move list : List>]`
    + Thinks until to reach `<Ply>`th depth and returns the best move.
    + Different from "go" command, until the engine have found the best move,
      the control won't come back.
* `@go-nodes <Nodes : Number> [<Candidate move list : List>]`
    + Thinks until to search `<Nodes>` nodes and returns the best move.
    + Different from "go" command, until the engine have found the best move,
      the control won't come back.

__Example__

    (define my-engine (gen-engine))
    
    ;; Register a listener.
    (define (listener message) (display "Engine > " message))
    (my-engine '@add-uci-output-listener listener)
    
    (display (my-engine '@go-movetime 10000))
    ;; Output
    ;; > Engine > info depth 1
    ;; > Engine > info currmove h2h4 currmovenumber 1
    ;; > Engine > info depth 1 seldepth 1 score cp 12 time 0 nodes 2 pv h2h4
    ;; > Engine > info currmove h2h3 currmovenumber 2
    ;; > Engine > info depth 1 seldepth 1 score cp 22 time 1 nodes 4 pv h2h3
    ;; > Engine > info currmove g2g4 currmovenumber 3
    ;; > Engine > info depth 1 seldepth 1 score cp 23 time 1 nodes 6 pv g2g4
    ;; > Engine > info currmove g2g3 currmovenumber 4
    ;; > Engine > info depth 1 seldepth 1 score cp 33 time 1 nodes 8 pv g2g3
    ;; > Engine > info currmove f2f4 currmovenumber 5
    ;; > Engine > info currmove f2f3 currmovenumber 6
    ;; > Engine > info depth 1 seldepth 1 score cp 36 time 1 nodes 11 pv f2f3
    ;; > Engine > info currmove e2e4 currmovenumber 7
    ;; > Engine > info depth 1 seldepth 1 score cp 45 time 1 nodes 13 pv e2e4
    ;; > Engine > info currmove e2e3 currmovenumber 8
    ;; > Engine > info currmove d2d4 currmovenumber 9
    ;; > Engine > info depth 1 seldepth 1 score cp 50 time 1 nodes 16 pv d2d4
    ;; > Engine > info currmove d2d3 currmovenumber 10
    ;; > Engine > info currmove c2c4 currmovenumber 11
    ;; > Engine > info currmove c2c3 currmovenumber 12
    ;; > Engine > info currmove b2b4 currmovenumber 13
    ;; > Engine > info currmove b2b3 currmovenumber 14
    ;; > Engine > info currmove a2a4 currmovenumber 15
    ;; > Engine > info currmove a2a3 currmovenumber 16
    ;; > Engine > info currmove g1h3 currmovenumber 17
    ;; > Engine > info currmove g1f3 currmovenumber 18
    ;; > Engine > info depth 1 seldepth 1 score cp 68 time 1 nodes 26 pv g1f3
    ;; > Engine > info currmove b1c3 currmovenumber 19
    ;; > Engine > info currmove b1a3 currmovenumber 20
    ;;
    ;; (Omitted)
    ;;
    ;; > Engine > info depth 11
    ;; > Engine > info currmove e2e4 currmovenumber 1
    ;; > Engine > info time 10000 nodes 5599214 hashfull 390 nps 559921
    ;; > score cp 45 pv e2e4 b8c6 g1f3 g8f6 e4e5 f6g4 d2d4 e7e6 h2h3 f8b4
    ;; > Engine > bestmove e2e4 ponder b8c6
    ;; > (E2 E4 EMPTY)

* `@set-hash-size <Size : Number>`
    + Sets size of Hash Table(Transposition Table)
      and returns the previous size.
    + The unit of size is "byte".
* `@set-threads <Number of threads : Number>`
    + Sets `<Number of threads>` and returns the previous number.

__Example__

    (define my-engine (gen-engine))
    
    ;; Set size of Hash Table to 128 MB.
    (my-engine '@input-uci-command "setoption name hash value 128")
    
    (display (my-engine '@set-hash-size (* 256 1024 1024)))
    ;; Set size of Hash Table to 256 MB and return 128 * 1024 * 1024 bytes.
    ;; Output
    ;; > 1.34218e+08
    
    ;; Set number of threads to 3.
    (my-engine '@input-uci-command "setoption name threads value 3")
    
    (display (my-engine '@set-threads 4))
    ;; Set number of threads to 4 and return 3.
    ;; Output
    ;; > 3

* `@material [<New materal : List>]`
    + Returns List of material.
        - 1st : Empty (It is always 0)
        - 2nd : Pawn
        - 3rd : Knight
        - 4th : Bishop
        - 5th : Rook
        - 6th : Queen
        - 7th : King
    + If you specify `<New materal>`, the material is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@material (list 111 222 333 444 555 666 777)))
    ;; Output
    ;; > (0 100 400 400 600 1200 1e+06)
    
    (display (my-engine '@material))
    ;; Output
    ;; > (0 222 333 444 555 666 777)

* `@enable-quiesce-search [<New setting : Boolean>]`
    + Returns whether Quiescence Search is enabled or not.
    + If you specify #t to `<New setting>`,
      Quiescence Search is set to be enabled.
      Otherwise, it is set to be disabled.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-quiesce-search #f))
    ;; Output
    :: > #t
    
    (display (my-engine '@enable-quiesce-search))
    ;; Output
    :: > #f

* `@enable-repetition-check [<New setting : Boolean>]`
    + Returns whether Repetition Check is enabled or not.
    + If you specify #t to `<New setting>`,
      Repetition Check is set to be enabled.
      Otherwise, it is set to be disabled.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-repetition-check #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-repetition-check))
    ;; Output
    ;; > #f

* `@enable-check-extension [<New setting : Boolean>]`
    + Returns whether Check Extension is enabled or not.
    + If you specify #t to `<New setting>`,
      Check Extension is set to be enabled.
      Otherwise, it is set to be disabled.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-check-extension #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-check-extension))
    ;; Output
    ;; > #f

* `@ybwc-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter, YBWC is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.

* `@ybwc-invalid-moves [<New number of moves : Number>]`
    + YBWC searches with one thread during this parameter of candidate moves.
    + Return this parameter.
    + If you specify `<New number of moves>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@ybwc-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@ybwc-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@ybwc-invalid-moves 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@ybwc-invalid-moves))
    ;; Output
    ;; > 10

* `@enable-aspiration-windows [<New setting : Boolean>]`
    + Returns whether Aspiration Windows is enabled or not.
    + If you specify #t to `<New setting>`,
      Aspiration Windows is set to be enabled.
      Otherwise, it is set to be disabled.
* `@aspiration-windows-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter at the root node,
      Aspiration Windows is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@aspiration-windows-delta [<New delta : Number>]`
    + Return Delta.
    + If you specify `<New delta>`, Delta is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-aspiration-windows #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-aspiration-windows))
    ;; Output
    ;; > #f
    
    (display (my-engine '@aspiration-windows-limit-depth 10))
    ;; Output
    ;; > 5
    
    (display (my-engine '@aspiration-windows-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@aspiration-windows-delta 20))
    ;; Output
    ;; > 15
    
    (display (my-engine '@aspiration-windows-delta))
    ;; Output
    ;; > 20

* `@enable-see [<New setting : Boolean>]`
    + Returns whether SEE is enabled or not.
    + If you specify #t to `<New setting>`,
      SEE is set to be enabled. Otherwise, it is set to be disabled.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-see #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-see))
    ;; Output
    ;; > #f

* `@enable-history [<New setting : Boolean>]`
    + Returns whether History Heuristics is enabled or not.
    + If you specify #t to `<New setting>`,
      History Heuristics is set to be enabled.
      Otherwise, it is set to be disabled.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-history #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-history))
    ;; Output
    ;; > #f

* `@enable-killer [<New setting : Boolean>]`
    + Returns whether Killer Move Heuristics is enabled or not.
    + If you specify #t to `<New setting>`,
      Killer Move Hiuristics is set to be enabled.
      Otherwise, it is set to be disabled.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-killer #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-killer))
    ;; Output
    ;; > #f

* `@enable-hash-table [<New setting : Boolean>]`
    + Returns whether Transposition Table is enabled or not.
    + If you specify #t to `<New setting>`,
      Transposition Table is set to be enabled.
      Otherwise, it is set to be disabled.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-hash-table #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-hash-table))
    ;; Output
    ;; > #t
* `@enable-iid [<New setting : Boolean>]`
    + Returns whether Internal Iterative Deepening is enabled or not.
    + If you specify #t to `<New setting>`,
      Internal Iterative Deepening is set to be enabled.
      Otherwise, it is set to be disabled.
* `@iid-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      Internal Iterative Deepening is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@iid-search-depth [<New depth : Number>]`
    + Internal Iterative Deepening searches until depth of this parameter.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-iid #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-iid))
    ;; Output
    ;; > #f
    
    (display (my-engine '@iid-limit-depth 10))
    ;; Output
    ;; > 5
    
    (display (my-engine '@iid-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@iid-search-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@iid-search-depth))
    ;; Output
    ;; > 10

* `@enable-nmr [<New setting : Boolean>]`
    + Returns whether Null Move Reduction is enabled or not.
    + If you specify #t to `<New setting>`,
      Null Move Reduction is set to be enabled.
      Otherwise, it is set to be disabled.
* `@nmr-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      Null Move Reduction is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@nmr-search-reduction [<New reduction : Number>]`
    + When searching shallowly, the depth is the actual depth
      minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.
* `@nmr-reduction [<New reduction : Number>]`
    + If the score is greater than or equals to Beta,
      the remaining depth is reduced by this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-nmr #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-nmr))
    ;; Output
    ;; > #f
    
    (display (my-engine '@nmr-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@nmr-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@nmr-search-reduction 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@nmr-search-reduction))
    ;; Output
    ;; > 10
    
    (display (my-engine '@nmr-reduction 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@nmr-reduction))
    ;; Output
    ;; > 10

* `@enable-probcut [<New setting : Boolean>]`
    + Returns whether ProbCut is enabled or not.
    + If you specify #t to `<New setting>`, ProbCut is set to be enabled.
      Otherwise, it is set to be disabled.
* `@probcut-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter, ProbCut is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@probcut-margin [<New margin : Number>]`
    + When Zero Window Search,
      ProbCut uses the current Beta plus this parameter as temporary Beta.
* `@probcut-search-reduction [<New reduction : Number>]`
    + When Zero Window Search, the depth is the actual depth
      minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-probcut #t))
    ;; Output
    ;; > #f
    
    (display (my-engine '@enable-probcut))
    ;; Output
    ;; > #t
    
    (display (my-engine '@probcut-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@probcut-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@probcut-margin 1200))
    ;; Output
    ;; > 400
    
    (display (my-engine '@probcut-margin))
    ;; Output
    ;; > 1200
    
    (display (my-engine '@probcut-search-reduction 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@probcut-search-reduction))
    ;; Output
    ;; > 10
* `@enable-history-pruning [<New setting : Boolean>]`
    + Returns whether History Pruning is enabled or not.
    + If you specify #t to `<New setting>`,
      History Pruning is set to be enabled.
      Otherwise, it is set to be disabled.
* `@history-pruning-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      History Pruning is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@history-pruning-move-threshold [<New threshold : Number>]`
    + If the number of the candidate move is less
      than the number of all moves times this parameter,
      History Pruning is invalidated.
    + This parameter is between 0.0 and 1.0.
    + Return this parameter.
    + If you specify `<New threshold>`, this parameter is updated.
* `@history-pruning-invalid-moves [<New number of moves : Number>]`
    + If the number of the candidate moves is less than this parameter,
      History Pruning is invalidated.
    + This parameter is given priority to `@history-pruning-move-threshold`.
    + Return this parameter.
    + If you specify `<New number of moves>`, this parameter is updated.
* `@history-pruning-threshold [<New threshold : Number>]`
    + If the history value of the current candidate move is lower
      than the max history value times this parameter,
      History Pruning temporarily reduces the remaining depth.
    + Return this parameter.
    + If you specify `<New threshold>`, this parameter is updated.
* `@history-pruning-reduction [<New reduction : Number>]`
    + When History Pruning reduces the remaining depth,
      a new depth is the current depth minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

__Example__

(define my-engine (gen-engine))

(display (my-engine '@enable-history-pruning #t))
;; Output
;; > #f

(display (my-engine '@enable-history-pruning))
;; Output
;; > #t

(display (my-engine '@history-pruning-limit-depth 10))
;; Output
;; > 4

(display (my-engine '@history-pruning-limit-depth))
;; Output
;; > 10

(display (my-engine '@history-pruning-move-threshold 0.8))
;; Output
;; > 0.6

(display (my-engine '@history-pruning-move-threshold))
;; Output
;; > 0.8

(display (my-engine '@history-pruning-invalid-moves 20))
;; Output
;; > 10

(display (my-engine '@history-pruning-invalid-moves))
;; Output
;; > 20

(display (my-engine '@history-pruning-threshold 0.8))
;; Output
;; > 0.5

(display (my-engine '@history-pruning-threshold))
;; Output
;; > 0.8

(display (my-engine '@history-pruning-reduction 10))
;; Output
;; > 1

(display (my-engine '@history-pruning-reduction))
;; Output
;; > 10

* `@enable-lmr [<New setting : Boolean>]`
    + Returns whether Late Move Reduction is enabled or not.
    + If you specify #t to `<New setting>`,
      Late Move Reduction is set to be enabled.
      Otherwise, it is set to be disabled.
* `@lmr-limit-depth [<New depth : Number>]`
    + If remaining depth is less than this parameter,
      Late Move Reduction is invalidated.
    + Return this parameter.
    + If you specify `<New depth>`, this parameter is updated.
* `@lmr-move-threshold [<New threshold : Number>]`
    + If the number of the candidate move is less
      than the number of all moves times this parameter,
      Late Move Reduction is invalidated.
    + This parameter is between 0.0 and 1.0.
    + Return this parameter.
    + If you specify `<New threshold>`, this parameter is updated.
* `@lmr-invalid-moves [<New number of moves : Number>]`
    + If the number of the candidate moves is less than this parameter,
      Late Move Reduction is invalidated.
    + This parameter is given priority to `@lmr-move-threshold`.
    + Return this parameter.
    + If you specify `<New number of moves>`, this parameter is updated.
* `@lmr-search-reduction [<New reduction : Number>]`
    + When searching shallowly, the depth is the actual depth
      minus this parameter.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-lmr #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-lmr))
    ;; Output
    ;; > #f
    
    (display (my-engine '@lmr-limit-depth 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@lmr-limit-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@lmr-move-threshold 0.8))
    ;; Output
    ;; > 0.3
    
    (display (my-engine '@lmr-move-threshold))
    ;; Output
    ;; > 0.8
    
    (display (my-engine '@lmr-invalid-moves 10))
    ;; Output
    ;; > 4
    
    (display (my-engine '@lmr-invalid-moves))
    ;; Output
    ;; > 10
    
    (display (my-engine '@lmr-search-reduction 5))
    ;; Output
    ;; > 1
    
    (display (my-engine '@lmr-search-reduction))
    ;; Output
    ;; > 5

* `@enable-futility-pruning [<New setting : Boolean>]`
    + Returns whether Futility Pruning is enabled or not.
    + If you specify #t to `<New setting>`,
      Futility Pruning is set to be enabled.
      Otherwise, it is set to be disabled.
* `@futility-pruning-depth [<New depth : Number>]`
    + If the remaining depth is less than or equals to this parameter,
      Futility Pruning is executed.
    + Return this parameter.
    + If you specify `<New reduction>`, this parameter is updated.
* `@futility-pruning-margin [<New margin : Number>]`
    + If the material after the move is lower than Alpha minus this parameter,
      the move is not evaluated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@enable-futility-pruning #f))
    ;; Output
    ;; > #t
    
    (display (my-engine '@enable-futility-pruning))
    ;; Output
    ;; > #f
    
    (display (my-engine '@futility-pruning-depth 10))
    ;; Output
    ;; > 3
    
    (display (my-engine '@futility-pruning-depth))
    ;; Output
    ;; > 10
    
    (display (my-engine '@futility-pruning-margin 1200))
    ;; Output
    ;; > 400
    
    (display (my-engine '@futility-pruning-margin))
    ;; Output
    ;; > 1200

* `@pawn-square-table-opening [<New table : List>]`
    + Returns Piece Square Table for Pawn at Opening as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@knight-square-table-opening [<New table : List>]`
    + Returns Piece Square Table for Knight at Opening as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@bishop-square-table-opening [<New table : List>]`
    + Returns Piece Square Table for Bishop at Opening as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@rook-square-table-opening [<New table : List>]`
    + Returns Piece Square Table for Rook at Opening as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@queen-square-table-opening [<New table : List>]`
    + Returns Piece Square Table for Queen at Opening as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@king-square-table-opening [<New table : List>]`
    + Returns Piece Square Table for King at Opening as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@pawn-square-table-ending [<New table : List>]`
    + Returns Piece Square Table for Pawn at Ending as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@knight-square-table-ending [<New table : List>]`
    + Returns Piece Square Table for Knight at Ending as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@bishop-square-table-ending [<New table : List>]`
    + Returns Piece Square Table for Bishop at Ending as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@rook-square-table-ending [<New table : List>]`
    + Returns Piece Square Table for Rook at Ending as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@queen-square-table-ending [<New table : List>]`
    + Returns Piece Square Table for Queen at Ending as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.
* `@king-square-table-ending [<New table : List>]`
    + Returns Piece Square Table for King at Ending as List composed
      of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@pawn-square-table-opening
    (list
      1 2 3 4 5 6 7 8
      11 22 33 44 55 66 77 88
      111 222 333 444 555 666 777 888
      1111 2222 3333 4444 5555 6666 7777 8888
      11111 22222 33333 44444 55555 66666 77777 88888
      111111 222222 333333 444444 555555 666666 777777 888888
      1111111 2222222 3333333 4444444 5555555 6666666 7777777 8888888
      11111111 22222222 33333333 44444444 55555555 66666666 77777777 88888888
    )))
    ;; Output
    ;; > (0 0 0 0 0 0 0 0
    ;; > 0 0 0 0 0 0 0 0
    ;; > 5 10 15 20 20 15 10 5
    ;; > 10 20 30 40 40 30 20 10
    ;; > 15 30 45 60 60 45 30 15
    ;; > 20 40 60 80 80 60 40 20
    ;; > 25 50 75 100 100 75 50 25
    ;; > 30 60 90 120 120 90 60 30)
    
    (display (my-engine '@pawn-square-table-opening))
    ;; Output
    ;; > (1 2 3 4 5 6 7 8
    ;; > 11 22 33 44 55 66 77 88
    ;; > 111 222 333 444 555 666 777 888
    ;; > 1111 2222 3333 4444 5555 6666 7777 8888
    ;; > 11111 22222 33333 44444 55555 66666 77777 88888
    ;; > 111111 222222 333333 444444 555555 666666 777777 888888
    ;; > 1111111 2222222 3333333 4444444 5555555 6666666 7777777 8888888
    ;; > 11111111 22222222 33333333 44444444 55555555 66666666 77777777
    ;; > 88888888)

* `@pawn-attack-table [<New table : List>]`
    + Returns a value table of Attacking Score for Pawn
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of attacking Pawn.
        - 3rd : Value of attacking Knight.
        - 4th : Value of attacking Bishop.
        - 5th : Value of attacking Rook.
        - 6th : Value of attacking Queen.
        - 7th : Value of attacking King.
    + If you specify `<New table>`, this parameter is updated.
* `@knight-attack-table [<New table : List>]`
    + Returns a value table of Attacking Score for Knight
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of attacking Pawn.
        - 3rd : Value of attacking Knight.
        - 4th : Value of attacking Bishop.
        - 5th : Value of attacking Rook.
        - 6th : Value of attacking Queen.
        - 7th : Value of attacking King.
    + If you specify `<New table>`, this parameter is updated.
* `@bishop-attack-table [<New table : List>]`
    + Returns a value table of Attacking Score for Bishop
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of attacking Pawn.
        - 3rd : Value of attacking Knight.
        - 4th : Value of attacking Bishop.
        - 5th : Value of attacking Rook.
        - 6th : Value of attacking Queen.
        - 7th : Value of attacking King.
    + If you specify `<New table>`, this parameter is updated.
* `@rook-attack-table [<New table : List>]`
    + Returns a value table of Attacking Score for Rook
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of attacking Pawn.
        - 3rd : Value of attacking Knight.
        - 4th : Value of attacking Bishop.
        - 5th : Value of attacking Rook.
        - 6th : Value of attacking Queen.
        - 7th : Value of attacking King.
    + If you specify `<New table>`, this parameter is updated.
* `@queen-attack-table [<New table : List>]`
    + Returns a value table of Attacking Score for Queen
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of attacking Pawn.
        - 3rd : Value of attacking Knight.
        - 4th : Value of attacking Bishop.
        - 5th : Value of attacking Rook.
        - 6th : Value of attacking Queen.
        - 7th : Value of attacking King.
    + If you specify `<New table>`, this parameter is updated.
* `@king-attack-table [<New table : List>]`
    + Returns a value table of Attacking Score for King
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of attacking Pawn.
        - 3rd : Value of attacking Knight.
        - 4th : Value of attacking Bishop.
        - 5th : Value of attacking Rook.
        - 6th : Value of attacking Queen.
        - 7th : Value of attacking King.
    + If you specify `<New table>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@pawn-attack-table (list 1 2 3 4 5 6 7)))
    ;; Output
    ;; > (0 10 12 14 16 18 20)
    
    (display (my-engine '@pawn-attack-table))
    ;; Output
    ;; > (0 2 3 4 5 6 7)

* `@weight-pawn-attack [<New weight : List>]`
    + Return Weight for Attacking Score for Pawn as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-knight-attack [<New weight : List>]`
    + Return Weight for Attacking Score for Knight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-attack [<New weight : List>]`
    + Return Weight for Attacking Score for Bishop as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-attack [<New weight : List>]`
    + Return Weight for Attacking Score for Rook as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-attack [<New weight : List>]`
    + Return Weight for Attacking Score for Queen as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-king-attack [<New weight : List>]`
    + Return Weight for Attacking Score for King as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@weight-pawn-attack (list 111 222)))
    ;; Output
    ;; > (1 0.3)
    
    (display (my-engine '@weight-pawn-attack))
    ;; Output
    ;; > (111 222)

* `@pawn-defense-table [<New table : List>]`
    + Returns a value table of Defense Score for Pawn
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of defense Pawn.
        - 3rd : Value of defense Knight.
        - 4th : Value of defense Bishop.
        - 5th : Value of defense Rook.
        - 6th : Value of defense Queen.
        - 7th : Value of defense King.
    + If you specify `<New table>`, this parameter is updated.
* `@knight-defense-table [<New table : List>]`
    + Returns a value table of Defense Score for Knight
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of defense Pawn.
        - 3rd : Value of defense Knight.
        - 4th : Value of defense Bishop.
        - 5th : Value of defense Rook.
        - 6th : Value of defense Queen.
        - 7th : Value of defense King.
    + If you specify `<New table>`, this parameter is updated.
* `@bishop-defense-table [<New table : List>]`
    + Returns a value table of Defense Score for Bishop
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of defense Pawn.
        - 3rd : Value of defense Knight.
        - 4th : Value of defense Bishop.
        - 5th : Value of defense Rook.
        - 6th : Value of defense Queen.
        - 7th : Value of defense King.
    + If you specify `<New table>`, this parameter is updated.
* `@rook-defense-table [<New table : List>]`
    + Returns a value table of Defense Score for Rook
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of defense Pawn.
        - 3rd : Value of defense Knight.
        - 4th : Value of defense Bishop.
        - 5th : Value of defense Rook.
        - 6th : Value of defense Queen.
        - 7th : Value of defense King.
    + If you specify `<New table>`, this parameter is updated.
* `@queen-defense-table [<New table : List>]`
    + Returns a value table of Defense Score for Queen
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of defense Pawn.
        - 3rd : Value of defense Knight.
        - 4th : Value of defense Bishop.
        - 5th : Value of defense Rook.
        - 6th : Value of defense Queen.
        - 7th : Value of defense King.
    + If you specify `<New table>`, this parameter is updated.
* `@king-defense-table [<New table : List>]`
    + Returns a value table of Defense Score for King
      as List composed of 7 values.
        - 1st : Not used. This is always '0'. (For EMPTY)
        - 2nd : Value of defense Pawn.
        - 3rd : Value of defense Knight.
        - 4th : Value of defense Bishop.
        - 5th : Value of defense Rook.
        - 6th : Value of defense Queen.
        - 7th : Value of defense King.
    + If you specify `<New table>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@pawn-defense-table (list 1 2 3 4 5 6 7)))
    ;; Output
    ;; > (0 10 0 0 0 0 0)
    
    (display (my-engine '@pawn-defense-table))
    ;; Output
    ;; > (0 2 3 4 5 6 7)

* `@weight-pawn-defense [<New weight : List>]`
    + Return Weight for Defense Score for Pawn as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-knight-defense [<New weight : List>]`
    + Return Weight for Defense Score for Knight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-defense [<New weight : List>]`
    + Return Weight for Defense Score for Bishop as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-defense [<New weight : List>]`
    + Return Weight for Defense Score for Rook as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-defense [<New weight : List>]`
    + Return Weight for Defense Score for Queen as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-king-defense [<New weight : List>]`
    + Return Weight for Defense Score for King as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@weight-pawn-defense (list 111 222)))
    ;; Output
    ;; > (1 0.5)
    
    (display (my-engine '@weight-pawn-defense))
    ;; Output
    ;; > (111 222)

* `@bishop-pin-table [<New value table : List>]`
    + Return a value table composed of 7 lists.
      Each list is composed of 7 values.
        - 1st : Futile list. (For EMPTY)
        - 2nd : a value list When a target piece is Pawn.
            - 1st : Futile value. (For EMPTY)
            - 2nd : a value when pin-board piece is Pawn.
            - 3rd : a value when pin-board piece is Knight.
            - 4rd : a value when pin-board piece is Bishop.
            - 5rd : a value when pin-board piece is Rook.
            - 6rd : a value when pin-board piece is Queen.
            - 7rd : a value when pin-board piece is King.
        - 3rd : a value list When a target piece is Knight.
            - 1st : Futile value. (For EMPTY)
            - 2nd : a value when pin-board piece is Pawn.
            - 3rd : a value when pin-board piece is Knight.
            - 4rd : a value when pin-board piece is Bishop.
            - 5rd : a value when pin-board piece is Rook.
            - 6rd : a value when pin-board piece is Queen.
            - 7rd : a value when pin-board piece is King.
        - 4th : a value list When a target piece is Bishop.
            - 1st : Futile value. (For EMPTY)
            - 2nd : a value when pin-board piece is Pawn.
            - 3rd : a value when pin-board piece is Knight.
            - 4rd : a value when pin-board piece is Bishop.
            - 5rd : a value when pin-board piece is Rook.
            - 6rd : a value when pin-board piece is Queen.
            - 7rd : a value when pin-board piece is King.
        - 5th : a value list When a target piece is Rook.
            - 1st : Futile value. (For EMPTY)
            - 2nd : a value when pin-board piece is Pawn.
            - 3rd : a value when pin-board piece is Knight.
            - 4rd : a value when pin-board piece is Bishop.
            - 5rd : a value when pin-board piece is Rook.
            - 6rd : a value when pin-board piece is Queen.
            - 7rd : a value when pin-board piece is King.
        - 6th : a value list When a target piece is Queen.
            - 1st : Futile value. (For EMPTY)
            - 2nd : a value when pin-board piece is Pawn.
            - 3rd : a value when pin-board piece is Knight.
            - 4rd : a value when pin-board piece is Bishop.
            - 5rd : a value when pin-board piece is Rook.
            - 6rd : a value when pin-board piece is Queen.
            - 7rd : a value when pin-board piece is King.
        - 7th : a value list When a target piece is King.
            - 1st : Futile value. (For EMPTY)
            - 2nd : a value when pin-board piece is Pawn.
            - 3rd : a value when pin-board piece is Knight.
            - 4rd : a value when pin-board piece is Bishop.
            - 5rd : a value when pin-board piece is Rook.
            - 6rd : a value when pin-board piece is Queen.
            - 7rd : a value when pin-board piece is King.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@bishop-pin-table
      (list (list 1 2 3 4 5 6 7)
            (list 8 9 10 11 12 13 14)
            (list 15 16 17 18 19 20 21)
            (list 22 23 24 25 26 27 28)
            (list 29 30 31 32 33 34 35)
            (list 36 37 38 39 40 41 42)
            (list 43 44 45 46 47 48 49))))
    ;; Output
    ;; > ((0 0 0 0 0 0 0)
    ;; > (0 0 0 0 5 5 5)
    ;; > (0 0 0 0 10 10 10)
    ;; > (0 0 0 0 0 0 0)
    ;; > (0 0 0 0 20 30 40)
    ;; > (0 0 0 0 30 40 50)
    ;; > (0 0 0 0 40 50 0))
    
    (display (my-engine '@bishop-pin-table))
    ;; Output
    ;; >  ((0 0 0 0 0 0 0)
    ;; > (0 9 10 11 12 13 14)
    ;; > (0 16 17 18 19 20 21)
    ;; > (0 23 24 25 26 27 28)
    ;; > (0 30 31 32 33 34 35)
    ;; > (0 37 38 39 40 41 42)
    ;; > (0 44 45 46 47 48 49))

* `@weight-bishop-pin [<New weight : List>]`
    + Return Weight for Pin Score for King as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-pin [<New weight : List>]`
    + Return Weight for Pin Score for Rook as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-pin [<New weight : List>]`
    + Return Weight for Pin Score for Queen as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@weight-bishop-pin (list 111 222)))
    ;; Output
    ;; > (1 1)
    
    (display (my-engine '@weight-bishop-pin))
    ;; Output
    ;; > (111 222)

* `@pawn-shield-table [<New table : List>]`
    + Returns Piece Square Table for Pawn Shield
      as List composed of 64 numbers. "()" is a square when evaluating Black.
        - From 1st to 8th : From A1(A8) to H1(H8)
        - From 9th to 16th : From A2(A7) to H2(H7)
        - From 17th to 24th : From A3(A6) to H3(H6)
        - From 25th to 32nd : From A4(A5) to H4(H5)
        - From 33rd to 40th : From A5(A4) to H5(H4)
        - From 41st to 48th : From A6(A3) to H6(H3)
        - From 49th to 56th : From A7(A2) to H7(H2)
        - From 57th to 64th : From A8(A1) to H8(H1)
    + If you specify `<New table>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (define table
      (list 1 2 3 4 5 6 7 8
            9 10 11 12 13 14 15 16
            17 18 19 20 21 22 23 24
            25 26 27 28 29 30 31 32
            33 34 35 36 37 38 39 40
            41 42 43 44 45 46 47 48
            49 50 51 52 53 54 55 56
            57 58 59 60 61 62 63 64))
    
    (display (my-engine '@pawn-shield-table table))
    ;; Output
    ;; > (0 0 0 0 0 0 0 0
    ;; > 30 30 30 30 30 30 30 30
    ;; > 0 0 0 0 0 0 0 0
    ;; > -30 -30 -30 -30 -30 -30 -30 -30
    ;; > -60 -60 -60 -60 -60 -60 -60 -60
    ;; > -90 -90 -90 -90 -90 -90 -90 -90
    ;; > -60 -60 -60 -60 -60 -60 -60 -60
    ;; > -30 -30 -30 -30 -30 -30 -30 -30)
    
    (display (my-engine '@pawn-shield-table))
    ;; Output
    ;; > (1 2 3 4 5 6 7 8
    ;; > 9 10 11 12 13 14 15 16
    ;; > 17 18 19 20 21 22 23 24
    ;; > 25 26 27 28 29 30 31 32
    ;; > 33 34 35 36 37 38 39 40
    ;; > 41 42 43 44 45 46 47 48
    ;; > 49 50 51 52 53 54 55 56
    ;; > 57 58 59 60 61 62 63 64)

* `@weight-pawn-shield [<New weight : List>]`
    + Return Weight for Pawn Shield as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@weight-pawn-shield (list 111 222)))
    ;; Output
    ;; > (1 0)
    
    (display (my-engine '@weight-pawn-shield))
    ;; Output
    ;; > (111 222)

* `@weight-pawn-mobility [<New weight : List>]`
    + Weight for Mobility of Pawn.
        - Score is Weight times number of squares where it can go to.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-knight-mobility [<New weight : List>]`
        - Score is Weight times number of squares where it can go to.
    + Weight for Mobility of Knight.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-mobility [<New weight : List>]`
    + Weight for Mobility of Bishop.
        - Score is Weight times number of squares where it can go to.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-mobility [<New weight : List>]`
    + Weight for Mobility of Rook.
        - Score is Weight times number of squares where it can go to.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-mobility [<New weight : List>]`
    + Weight for Mobility of Queen.
        - Score is Weight times number of squares where it can go to.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-king-mobility [<New weight : List>]`
    + Weight for Mobility of King.
        - Score is Weight times number of squares where it can go to.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-pawn-center-control [<New weight : List>]`
    + Weight for Controlling Center by Pawn.
        - Score is Weight times number of Center where it attacks.
        - "Center" is squares of
          C3 C4 C5 C6 D3 D4 D5 D6 E3 E4 E5 E6 F3 F4 F5 F6.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-knight-center-control [<New weight : List>]`
    + Weight for Controlling Center by Knight.
        - Score is Weight times number of Center where it attacks.
        - "Center" is squares of
          C3 C4 C5 C6 D3 D4 D5 D6 E3 E4 E5 E6 F3 F4 F5 F6.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-center-control [<New weight : List>]`
    + Weight for Controlling Center by Bishop.
        - Score is Weight times number of Center where it attacks.
        - "Center" is squares of
          C3 C4 C5 C6 D3 D4 D5 D6 E3 E4 E5 E6 F3 F4 F5 F6.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-center-control [<New weight : List>]`
    + Weight for Controlling Center by Rook.
        - Score is Weight times number of Center where it attacks.
        - "Center" is squares of
          C3 C4 C5 C6 D3 D4 D5 D6 E3 E4 E5 E6 F3 F4 F5 F6.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-center-control [<New weight : List>]`
    + Weight for Controlling Center by Queen.
        - Score is Weight times number of Center where it attacks.
        - "Center" is squares of
          C3 C4 C5 C6 D3 D4 D5 D6 E3 E4 E5 E6 F3 F4 F5 F6.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-king-center-control [<New weight : List>]`
    + Weight for Controlling Center by King.
        - Score is Weight times number of Center where it attacks.
        - "Center" is squares of
          C3 C4 C5 C6 D3 D4 D5 D6 E3 E4 E5 E6 F3 F4 F5 F6.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-pawn-sweet-center-control [<New weight : List>]`
    + Weight for Controlling Sweet Center by Pawn.
        - Score is Weight times number of Sweet Center where it attacks.
        - "Sweet Center" is squares of D4 D5 E4 E5.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-knight-sweet-center-control [<New weight : List>]`
    + Weight for Controlling Sweet Center by Knight.
        - Score is Weight times number of Sweet Center where it attacks.
        - "Sweet Center" is squares of D4 D5 E4 E5.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-sweet-center-control [<New weight : List>]`
    + Weight for Controlling Sweet Center by Bishop.
        - Score is Weight times number of Sweet Center where it attacks.
        - "Sweet Center" is squares of D4 D5 E4 E5.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-sweet-center-control [<New weight : List>]`
    + Weight for Controlling Sweet Center by Rook.
        - Score is Weight times number of Sweet Center where it attacks.
        - "Sweet Center" is squares of D4 D5 E4 E5.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-sweet-center-control [<New weight : List>]`
    + Weight for Controlling Sweet Center by Queen.
        - Score is Weight times number of Sweet Center where it attacks.
        - "Sweet Center" is squares of D4 D5 E4 E5.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-king-sweet-center-control [<New weight : List>]`
    + Weight for Controlling Sweet Center by King.
        - Score is Weight times number of Sweet Center where it attacks.
        - "Sweet Center" is squares of D4 D5 E4 E5.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-pawn-development [<New weight : List>]`
    + Weight for Development of Pawn.
        - Score is Weight times number of Pawns not on starting position.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-knight-development [<New weight : List>]`
    + Weight for Development of Knight.
        - Score is Weight times number of Knights not on starting position.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-development [<New weight : List>]`
    + Weight for Development of Bishop.
        - Score is Weight times number of Bishops not on starting position.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-development [<New weight : List>]`
    + Weight for Development of Rook.
        - Score is Weight times number of Rooks not on starting position.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-development [<New weight : List>]`
    + Weight for Development of Queen.
        - Score is Weight times number of Queens not on starting position.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-king-development [<New weight : List>]`
    + Weight for Development of King.
        - Score is Weight times number of King not on starting position.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-pawn-attack-around-king [<New weight : List>]`
    + Weight for Pawn attacking squares around opponent's King.
        - Score is Weight times number of attacked squares
          around opponent's King.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-knight-attack-around-king [<New weight : List>]`
    + Weight for Knight attacking squares around opponent's King.
        - Score is Weight times number of attacked squares
          around opponent's King.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-attack-around-king [<New weight : List>]`
    + Weight for Bishop attacking squares around opponent's King.
        - Score is Weight times number of attacked squares
          around opponent's King.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-attack-around-king [<New weight : List>]`
    + Weight for Rook attacking squares around opponent's King.
        - Score is Weight times number of attacked squares
          around opponent's King.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-queen-attack-around-king [<New weight : List>]`
    + Weight for Queen attacking squares around opponent's King.
        - Score is Weight times number of attacked squares
          around opponent's King.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-king-attack-around-king [<New weight : List>]`
    + Weight for King attacking squares around opponent's King.
        - Score is Weight times number of attacked squares
          around opponent's King.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-pass-pawn [<New weight : List>]`
    + Weight for Pass Pawn.
        - Score is Weight times number of Pass Pawns.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-protected-pass-pawn [<New weight : List>]`
    + Weight for Pass Pawn protected by friend Pawns.
        - Score is Weight times number of Pass Pawns protected by friend Pawns.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-double-pawn [<New weight : List>]`
    + Weight for Double Pawn.
        - Score is Weight times number of Double Pawn.
            - If 2 pawns are on same fyle, the number of Double Pawn is '2'.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-iso-pawn [<New weight : List>]`
    + Weight for Isolated Pawn.
        - Score is Weight times number of Isolated Pawn.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bishop-pair [<New weight : List>]`
    + Weight for Bishop Pair.
        - Score is Weight if 2 or more Bishops exists
          on different colored square.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-bad-bishop [<New weight : List>]`
    + Weight for Bad Bishop.
        - Score is Weight times number of Pawns on the same colored square
          where Bishop is placed on.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-pair [<New weight : List>]`
    + Weight for Rook Pair.
        - Score is Weight if 2 or more Rooks exists on the chess board.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-semiopen-fyle [<New weight : List>]`
    + Weight for Rook on semi-open fyle.
        - Score is Weight times number of Rooks on semi-open fyle.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-rook-open-fyle [<New weight : List>]`
    + Weight for Rook on open fyle.
        - Score is Weight times number of Rooks on open fyle.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-early-queen-starting [<New weight : List>]`
    + Weight for Queen that has moved too early in the game.
        - Score is Weight times number of Minor Pieces
          on its starting position.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-weak-square [<New weight : List>]`
    + Weight for Weak Square.
        - When King is on A1(A8) or A2(A7) or B1(B8) or B2(B7) or C1(C8)
          or C2(C7), "Weak Square" is A2(A7) or A3(A6) or B2(B7) or B3(B6)
          or C2(C7) or C3(C6) squares where Pawn is NOT placed on.  
          When King is on F1(F8) or F2(F7) or G1(G8) or G2(G7) or H1(H8)
          or H2(H7), "Weak Square" is F2(F7) or F3(F6) or G2(G7) or G3(G6)
          or H2(H7) or H3(H6) squares where Pawn is NOT placed on.
        - Score is Weight times number of Weak Square.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-castling [<New weight : List>]`
    + Weight for Castling.
        - Score is Weight if King has castled.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.
* `@weight-abandoned-castling [<New weight : List>]`
    + Weight for King abandoned all Castling rights.
        - Score is Weight if King has abandoned all castling rights.
    + Return Weight as List of 2 elements.
        - 1st : Weight on Opening.
        - 2nd : Weight on Ending.
    + If you specify `<New weight>`, this parameter is updated.

__Example__

    (define my-engine (gen-engine))
    
    (display (my-engine '@weight-pawn-mobility (list 111 222)))
    ;; Output
    ;; > (0 0)
    
    (display (my-engine '@weight-pawn-mobility))
    ;; Output
    ;; > (111 222))...";
  }
}  // namespace Sayuri
