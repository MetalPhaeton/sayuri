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
#include <map>
#include "common.h"
#include "params.h"
#include "chess_engine.h"
#include "board.h"
#include "transposition_table.h"
#include "uci_shell.h"
#include "lisp_core.h"
#include "fen.h"
#include "position_record.h"
#include "pv_line.h"
#include "pgn.h"

/** Sayuri 名前空間。 */
namespace Sayuri {

//
//  // ウェイト関数オブジェクトをセット。
//  void EngineSuite::SetWeightFunctions() {
//    weight_1_accessor_[WEIGHT_OPENING_POSITION] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_opening_position();
//    };
//
//    weight_1_accessor_[WEIGHT_ENDING_POSITION] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_ending_position();
//    };
//
//    weight_1_accessor_[WEIGHT_MOBILITY] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_mobility();
//    };
//
//    weight_1_accessor_[WEIGHT_CENTER_CONTROL] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_center_control();
//    };
//
//    weight_1_accessor_[WEIGHT_SWEET_CENTER_CONTROL] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_sweet_center_control();
//    };
//
//    weight_1_accessor_[WEIGHT_DEVELOPMENT] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_development();
//    };
//
//    weight_1_accessor_[WEIGHT_ATTACK] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_attack();
//    };
//
//    weight_1_accessor_[WEIGHT_DEFENSE] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_defense();
//    };
//
//    weight_1_accessor_[WEIGHT_PIN] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_pin();
//    };
//
//    weight_1_accessor_[WEIGHT_ATTACK_AROUND_KING] =
//    [this]() -> const Weight (&)[NUM_PIECE_TYPES] {
//      return this->eval_params_ptr_->weight_attack_around_king();
//    };
//
//    weight_2_accessor_[WEIGHT_PASS_PAWN] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_pass_pawn();
//    };
//
//    weight_2_accessor_[WEIGHT_PROTECTED_PASS_PAWN] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_protected_pass_pawn();
//    };
//
//    weight_2_accessor_[WEIGHT_DOUBLE_PAWN] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_double_pawn();
//    };
//
//    weight_2_accessor_[WEIGHT_ISO_PAWN] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_iso_pawn();
//    };
//
//    weight_2_accessor_[WEIGHT_PAWN_SHIELD] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_pawn_shield();
//    };
//
//    weight_2_accessor_[WEIGHT_BISHOP_PAIR] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_bishop_pair();
//    };
//
//    weight_2_accessor_[WEIGHT_BAD_BISHOP] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_bad_bishop();
//    };
//
//    weight_2_accessor_[WEIGHT_ROOK_PAIR] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_rook_pair();
//    };
//
//    weight_2_accessor_[WEIGHT_ROOK_SEMIOPEN_FYLE] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_rook_semiopen_fyle();
//    };
//
//    weight_2_accessor_[WEIGHT_ROOK_OPEN_FYLE] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_rook_open_fyle();
//    };
//
//    weight_2_accessor_[WEIGHT_EARLY_QUEEN_STARTING] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_early_queen_starting();
//    };
//
//    weight_2_accessor_[WEIGHT_WEAK_SQUARE] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_weak_square();
//    };
//
//    weight_2_accessor_[WEIGHT_CASTLING] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_castling();
//    };
//
//    weight_2_accessor_[WEIGHT_ABANDONED_CASTLING] =
//    [this]() -> const Weight& {
//      return this->eval_params_ptr_->weight_abandoned_castling();
//    };
//
//    weight_1_mutator_[WEIGHT_OPENING_POSITION] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_opening_position
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_ENDING_POSITION] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_ending_position
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_MOBILITY] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_mobility
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_CENTER_CONTROL] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_center_control
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_SWEET_CENTER_CONTROL] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_sweet_center_control
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_DEVELOPMENT] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_development
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_ATTACK] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_attack
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_DEFENSE] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_defense
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_PIN] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_pin
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_1_mutator_[WEIGHT_ATTACK_AROUND_KING] =
//    [this](PieceType piece_type, double opening_weight,
//    double ending_weight) {
//      this->eval_params_ptr_->weight_attack_around_king
//      (piece_type, opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_PASS_PAWN] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_pass_pawn
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_PROTECTED_PASS_PAWN] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_protected_pass_pawn
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_DOUBLE_PAWN] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_double_pawn
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_ISO_PAWN] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_iso_pawn
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_PAWN_SHIELD] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_pawn_shield
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_BISHOP_PAIR] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_bishop_pair
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_BAD_BISHOP] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_bad_bishop
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_ROOK_PAIR] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_rook_pair
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_ROOK_SEMIOPEN_FYLE] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_rook_semiopen_fyle
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_ROOK_OPEN_FYLE] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_rook_open_fyle
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_EARLY_QUEEN_STARTING] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_early_queen_starting
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_WEAK_SQUARE] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_weak_square
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_CASTLING] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_castling
//      (opening_weight, ending_weight);
//    };
//
//    weight_2_mutator_[WEIGHT_ABANDONED_CASTLING] =
//    [this](double opening_weight, double ending_weight) {
//      this->eval_params_ptr_->weight_abandoned_castling
//      (opening_weight, ending_weight);
//    };
//  }
//
//  // ========================== //
//  // Lisp関数オブジェクト用関数 //
//  // ========================== //
//  // 関数オブジェクト。
//  LispObjectPtr EngineSuite::operator()
//  (LispObjectPtr self, const LispObject& caller, const LispObject& list) {
//    // 準備。
//    LispIterator<false> list_itr {&list};
//    std::string func_name = (list_itr++)->ToString();
//    int required_args = 1;
//
//    // 引数チェック。
//    if (!list_itr) {
//      throw Lisp::GenInsufficientArgumentsError
//      (func_name, required_args, true, list.Length() - 1);
//    }
//
//    // メッセージシンボルを得る。
//    LispObjectPtr message_ptr = caller.Evaluate(*(list_itr++));
//    if (!(message_ptr->IsSymbol())) {
//      throw Lisp::GenWrongTypeError
//      (func_name, "Symbol", std::vector<int> {1}, true);
//    }
//    std::string message_symbol = message_ptr->symbol_value();
//
//    if ((message_symbol == "@weight-pawn-mobility")
//    || (message_symbol == "@weight-knight-mobility")
//    || (message_symbol == "@weight-bishop-mobility")
//    || (message_symbol == "@weight-rook-mobility")
//    || (message_symbol == "@weight-queen-mobility")
//    || (message_symbol == "@weight-king-mobility")
//    || (message_symbol == "@weight-pawn-center-control")
//    || (message_symbol == "@weight-knight-center-control")
//    || (message_symbol == "@weight-bishop-center-control")
//    || (message_symbol == "@weight-rook-center-control")
//    || (message_symbol == "@weight-queen-center-control")
//    || (message_symbol == "@weight-king-center-control")
//    || (message_symbol == "@weight-pawn-sweet-center-control")
//    || (message_symbol == "@weight-knight-sweet-center-control")
//    || (message_symbol == "@weight-bishop-sweet-center-control")
//    || (message_symbol == "@weight-rook-sweet-center-control")
//    || (message_symbol == "@weight-queen-sweet-center-control")
//    || (message_symbol == "@weight-king-sweet-center-control")
//    || (message_symbol == "@weight-pawn-development")
//    || (message_symbol == "@weight-knight-development")
//    || (message_symbol == "@weight-bishop-development")
//    || (message_symbol == "@weight-rook-development")
//    || (message_symbol == "@weight-queen-development")
//    || (message_symbol == "@weight-king-development")
//    || (message_symbol == "@weight-pawn-attack")
//    || (message_symbol == "@weight-knight-attack")
//    || (message_symbol == "@weight-bishop-attack")
//    || (message_symbol == "@weight-rook-attack")
//    || (message_symbol == "@weight-queen-attack")
//    || (message_symbol == "@weight-king-attack")
//    || (message_symbol == "@weight-pawn-defense")
//    || (message_symbol == "@weight-knight-defense")
//    || (message_symbol == "@weight-bishop-defense")
//    || (message_symbol == "@weight-rook-defense")
//    || (message_symbol == "@weight-queen-defense")
//    || (message_symbol == "@weight-king-defense")
//    || (message_symbol == "@weight-bishop-pin")
//    || (message_symbol == "@weight-rook-pin")
//    || (message_symbol == "@weight-queen-pin")
//    || (message_symbol == "@weight-pawn-attack-around-king")
//    || (message_symbol == "@weight-knight-attack-around-king")
//    || (message_symbol == "@weight-bishop-attack-around-king")
//    || (message_symbol == "@weight-rook-attack-around-king")
//    || (message_symbol == "@weight-queen-attack-around-king")
//    || (message_symbol == "@weight-king-attack-around-king")
//    || (message_symbol == "@weight-pass-pawn")
//    || (message_symbol == "@weight-protected-pass-pawn")
//    || (message_symbol == "@weight-double-pawn")
//    || (message_symbol == "@weight-iso-pawn")
//    || (message_symbol == "@weight-pawn-shield")
//    || (message_symbol == "@weight-bishop-pair")
//    || (message_symbol == "@weight-bad-bishop")
//    || (message_symbol == "@weight-rook-pair")
//    || (message_symbol == "@weight-rook-semiopen-fyle")
//    || (message_symbol == "@weight-rook-open-fyle")
//    || (message_symbol == "@weight-early-queen-starting")
//    || (message_symbol == "@weight-weak-square")
//    || (message_symbol == "@weight-castling")
//    || (message_symbol == "@weight-abandoned-castling")
//    ) {
//      LispObjectPtr weight_params_ptr = Lisp::NewNil();
//      if (list_itr) {
//        weight_params_ptr = caller.Evaluate(*list_itr);
//        if (!(weight_params_ptr->IsList())) {
//          throw Lisp::GenWrongTypeError
//          (func_name, "List", std::vector<int> {2}, true);
//        }
//      }
//
//      if (message_symbol == "@weight-pawn-mobility") {
//        return SetWeight1<WEIGHT_MOBILITY, PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-knight-mobility") {
//        return SetWeight1<WEIGHT_MOBILITY, KNIGHT>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-mobility") {
//        return SetWeight1<WEIGHT_MOBILITY, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-mobility") {
//        return SetWeight1<WEIGHT_MOBILITY, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-mobility") {
//        return SetWeight1<WEIGHT_MOBILITY, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-king-mobility") {
//        return SetWeight1<WEIGHT_MOBILITY, KING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pawn-center-control") {
//        return SetWeight1<WEIGHT_CENTER_CONTROL, PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-knight-center-control") {
//        return SetWeight1<WEIGHT_CENTER_CONTROL, KNIGHT>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-center-control") {
//        return SetWeight1<WEIGHT_CENTER_CONTROL, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-center-control") {
//        return SetWeight1<WEIGHT_CENTER_CONTROL, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-center-control") {
//        return SetWeight1<WEIGHT_CENTER_CONTROL, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-king-center-control") {
//        return SetWeight1<WEIGHT_CENTER_CONTROL, KING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pawn-sweet-center-control") {
//        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-knight-sweet-center-control") {
//        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, KNIGHT>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-sweet-center-control") {
//        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-sweet-center-control") {
//        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-sweet-center-control") {
//        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-king-sweet-center-control") {
//        return SetWeight1<WEIGHT_SWEET_CENTER_CONTROL, KING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pawn-development") {
//        return SetWeight1<WEIGHT_DEVELOPMENT, PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-knight-development") {
//        return SetWeight1<WEIGHT_DEVELOPMENT, KNIGHT>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-development") {
//        return SetWeight1<WEIGHT_DEVELOPMENT, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-development") {
//        return SetWeight1<WEIGHT_DEVELOPMENT, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-development") {
//        return SetWeight1<WEIGHT_DEVELOPMENT, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-king-development") {
//        return SetWeight1<WEIGHT_DEVELOPMENT, KING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pawn-attack") {
//        return SetWeight1<WEIGHT_ATTACK, PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-knight-attack") {
//        return SetWeight1<WEIGHT_ATTACK, KNIGHT>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-attack") {
//        return SetWeight1<WEIGHT_ATTACK, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-attack") {
//        return SetWeight1<WEIGHT_ATTACK, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-attack") {
//        return SetWeight1<WEIGHT_ATTACK, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-king-attack") {
//        return SetWeight1<WEIGHT_ATTACK, KING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pawn-defense") {
//        return SetWeight1<WEIGHT_DEFENSE, PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-knight-defense") {
//        return SetWeight1<WEIGHT_DEFENSE, KNIGHT>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-defense") {
//        return SetWeight1<WEIGHT_DEFENSE, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-defense") {
//        return SetWeight1<WEIGHT_DEFENSE, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-defense") {
//        return SetWeight1<WEIGHT_DEFENSE, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-king-defense"){
//        return SetWeight1<WEIGHT_DEFENSE, KING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-pin") {
//        return SetWeight1<WEIGHT_PIN, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-pin") {
//        return SetWeight1<WEIGHT_PIN, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-pin") {
//        return SetWeight1<WEIGHT_PIN, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pawn-attack-around-king") {
//        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-knight-attack-around-king") {
//        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, KNIGHT>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-attack-around-king") {
//        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-attack-around-king") {
//        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, ROOK>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-queen-attack-around-king") {
//        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, QUEEN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-king-attack-around-king"){
//        return SetWeight1<WEIGHT_ATTACK_AROUND_KING, KING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pass-pawn"){
//        return SetWeight2<WEIGHT_PASS_PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-protected-pass-pawn"){
//        return SetWeight2<WEIGHT_PROTECTED_PASS_PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-double-pawn"){
//        return SetWeight2<WEIGHT_DOUBLE_PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-iso-pawn"){
//        return SetWeight2<WEIGHT_ISO_PAWN>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-pawn-shield"){
//        return SetWeight2<WEIGHT_PAWN_SHIELD>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bishop-pair"){
//        return SetWeight2<WEIGHT_BISHOP_PAIR>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-bad-bishop"){
//        return SetWeight2<WEIGHT_BAD_BISHOP>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-pair"){
//        return SetWeight2<WEIGHT_ROOK_PAIR>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-semiopen-fyle"){
//        return SetWeight2<WEIGHT_ROOK_SEMIOPEN_FYLE>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-rook-open-fyle"){
//        return SetWeight2<WEIGHT_ROOK_OPEN_FYLE>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-early-queen-starting"){
//        return SetWeight2<WEIGHT_EARLY_QUEEN_STARTING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-weak-square"){
//        return SetWeight2<WEIGHT_WEAK_SQUARE>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-castling"){
//        return SetWeight2<WEIGHT_CASTLING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//      if (message_symbol == "@weight-abandoned-castling"){
//        return SetWeight2<WEIGHT_ABANDONED_CASTLING>
//        (func_name, message_symbol, *weight_params_ptr);
//      }
//    }
//
//    throw Lisp::GenError("@engine-error", "(" + func_name
//    + ") couldn't understand '" + message_symbol + "'.");
//  }
//
  // ======== //
  // Sayulisp //
  // ======== //
  // ========== //
  // static定数 //
  // ========== //
  const std::map<std::string, Square> Sayulisp::SQUARE_MAP {
    {"A1", A1}, {"B1", B1}, {"C1", C1}, {"D1", D1},
    {"E1", E1}, {"F1", F1}, {"G1", G1}, {"H1", H1},
    {"A2", A2}, {"B2", B2}, {"C2", C2}, {"D2", D2},
    {"E2", E2}, {"F2", F2}, {"G2", G2}, {"H2", H2},
    {"A3", A3}, {"B3", B3}, {"C3", C3}, {"D3", D3},
    {"E3", E3}, {"F3", F3}, {"G3", G3}, {"H3", H3},
    {"A4", A4}, {"B4", B4}, {"C4", C4}, {"D4", D4},
    {"E4", E4}, {"F4", F4}, {"G4", G4}, {"H4", H4},
    {"A5", A5}, {"B5", B5}, {"C5", C5}, {"D5", D5},
    {"E5", E5}, {"F5", F5}, {"G5", G5}, {"H5", H5},
    {"A6", A6}, {"B6", B6}, {"C6", C6}, {"D6", D6},
    {"E6", E6}, {"F6", F6}, {"G6", G6}, {"H6", H6},
    {"A7", A7}, {"B7", B7}, {"C7", C7}, {"D7", D7},
    {"E7", E7}, {"F7", F7}, {"G7", G7}, {"H7", H7},
    {"A8", A8}, {"B8", B8}, {"C8", C8}, {"D8", D8},
    {"E8", E8}, {"F8", F8}, {"G8", G8}, {"H8", H8}
  };
  const std::map<std::string, Fyle> Sayulisp::FYLE_MAP {
    {"FYLE_A", FYLE_A}, {"FYLE_B", FYLE_B},
    {"FYLE_C", FYLE_C}, {"FYLE_D", FYLE_D},
    {"FYLE_E", FYLE_E}, {"FYLE_F", FYLE_F},
    {"FYLE_G", FYLE_G}, {"FYLE_H", FYLE_H},
  };
  const std::map<std::string, Rank> Sayulisp::RANK_MAP {
    {"RANK_1", RANK_1}, {"RANK_2", RANK_2},
    {"RANK_3", RANK_3}, {"RANK_4", RANK_4},
    {"RANK_5", RANK_5}, {"RANK_6", RANK_6},
    {"RANK_7", RANK_7}, {"RANK_8", RANK_8},
  };
  const std::map<std::string, Side> Sayulisp::SIDE_MAP {
    {"NO_SIDE", NO_SIDE}, {"WHITE", WHITE}, {"BLACK", BLACK}
  };
  const std::map<std::string, PieceType> Sayulisp::PIECE_MAP {
    {"EMPTY", EMPTY},
    {"PAWN", PAWN}, {"KNIGHT", KNIGHT}, {"BISHOP", BISHOP},
    {"ROOK", ROOK}, {"QUEEN", QUEEN}, {"KING", KING}
  };
  const std::map<std::string, int> Sayulisp::CASTLING_MAP {
    {"NO_CASTLING", 0},
    {"WHITE_SHORT_CASTLING", 1}, {"WHITE_LONG_CASTLING", 2},
    {"BLACK_SHORT_CASTLING", 3}, {"BLACK_LONG_CASTLING", 4}
  };
  const std::string Sayulisp::SQUARE_MAP_INV[NUM_SQUARES] {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"
  };
  const std::string Sayulisp::FYLE_MAP_INV[NUM_FYLES] {
    "FYLE_A", "FYLE_B", "FYLE_C", "FYLE_D",
    "FYLE_E", "FYLE_F", "FYLE_G", "FYLE_H"
  };
  const std::string Sayulisp::RANK_MAP_INV[NUM_RANKS] {
    "RANK_1", "RANK_2", "RANK_3", "RANK_4",
    "RANK_5", "RANK_6", "RANK_7", "RANK_8"
  };
  const std::string Sayulisp::SIDE_MAP_INV[NUM_SIDES] {
    "NO_SIDE", "WHITE", "BLACK"
  };
  const std::string Sayulisp::PIECE_MAP_INV[NUM_PIECE_TYPES] {
    "EMPTY", "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"
  };
  const std::string Sayulisp::CASTLING_MAP_INV[5] {
    "NO_CASTLING", "WHITE_SHORT_CASTLING", "WHITE_LONG_CASTLING",
    "BLACK_SHORT_CASTLING", "BLACK_LONG_CASTLING"
  };

//  // ==================== //
//  // コンストラクタと代入 //
//  // ==================== //
//  // コンストラクタ。
//  Sayulisp::Sayulisp() : Lisp() {
//    // EngineSuiteを作成する関数を作成。
//    {
//      auto func = [this]
//      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
//      -> LispObjectPtr {
//        return this->GenEngine();
//      };
//      AddNativeFunction(func, "gen-engine");
//    }
//
//
//    // 定数をバインドしていく。
//    // マスの定数をバインド。
//    FOR_SQUARES(square) {
//      BindSymbol(EngineSuite::SQUARE_SYMBOL[square],
//      Lisp::NewNumber(square));
//    }
//
//    // ファイルの定数をバインド。
//    FOR_FYLES(fyle) {
//      BindSymbol(EngineSuite::FYLE_SYMBOL[fyle],
//      Lisp::NewNumber(fyle));
//    }
//
//    // ランクの定数をバインド。
//    FOR_RANKS(rank) {
//      BindSymbol(EngineSuite::RANK_SYMBOL[rank],
//      Lisp::NewNumber(rank));
//    }
//
//    // サイドの定数をバインド。
//    FOR_SIDES(side) {
//      BindSymbol(EngineSuite::SIDE_SYMBOL[side],
//      Lisp::NewNumber(side));
//    }
//
//    // 駒の定数をバインド。
//    FOR_PIECE_TYPES(piece_type) {
//      BindSymbol(EngineSuite::PIECE_TYPE_SYMBOL[piece_type],
//      Lisp::NewNumber(piece_type));
//    }
//
//    // キャスリングの権利の定数をバインド。
//    for (int i = 0; i < 5; ++i) {
//      BindSymbol(EngineSuite::CASTLING_SYMBOL[i],
//      Lisp::NewNumber(i));
//    }
//
//    // ヘルプ辞書を作成。
//    SetHelp();
//
//    // --- 便利関数 --- //
//    {
//      auto func = [this](LispObjectPtr self, const LispObject& caller,
//      const LispObject& list) -> LispObjectPtr {
//        // 引数チェック。
//        LispIterator<false> list_itr {&list};
//        std::string func_name = (list_itr++)->ToString();
//        int required_args = 1;
//
//        if (!list_itr) {
//          throw Lisp::GenInsufficientArgumentsError
//          (func_name, required_args, false, list.Length() - 1);
//        }
//        LispObjectPtr result = caller.Evaluate(*list_itr);
//        if (!(result->IsString())) {
//          throw GenWrongTypeError
//          (func_name, "String", std::vector<int> {1}, true);
//        }
//
//        return this->GenPGN(result->string_value(), caller.scope_chain());
//      };
//      AddNativeFunction(func, "gen-pgn");
//    }
//    {
//      auto func = [this](LispObjectPtr self, const LispObject& caller,
//      const LispObject& list) -> LispObjectPtr {
//        // 引数チェック。
//        LispIterator<false> list_itr {&list};
//        std::string func_name = (list_itr++)->ToString();
//        int required_args = 1;
//
//        if (!list_itr) {
//          throw Lisp::GenInsufficientArgumentsError
//          (func_name, required_args, false, list.Length() - 1);
//        }
//        LispObjectPtr fen_ptr = caller.Evaluate(*list_itr);
//        if (!(fen_ptr->IsString())) {
//          throw GenWrongTypeError
//          (func_name, "String", std::vector<int> {1}, true);
//        }
//
//        return this->ParseFENEPD(fen_ptr->string_value());
//      };
//      AddNativeFunction(func, "parse-fen/epd");
//    }
//    {
//      auto func = [this](LispObjectPtr self, const LispObject& caller,
//      const LispObject& list) -> LispObjectPtr {
//        // 引数チェック。
//        LispIterator<false> list_itr {&list};
//        std::string func_name = (list_itr++)->ToString();
//        int required_args = 1;
//
//        if (!list_itr) {
//          throw Lisp::GenInsufficientArgumentsError
//          (func_name, required_args, false, list.Length() - 1);
//        }
//        LispObjectPtr position_ptr = caller.Evaluate(*list_itr);
//        if (!(position_ptr->IsList())) {
//          throw GenWrongTypeError
//          (func_name, "List", std::vector<int> {1}, true);
//        }
//
//        return this->ToFENPosition(func_name, *position_ptr);
//      };
//      AddNativeFunction(func, "to-fen-position");
//    }
//  }
//
  // Sayulispの関数を設定する。
  void Sayulisp::SetSayulispFunction() {
    // 定数を登録。
    for (auto& pair : SQUARE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : FYLE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : RANK_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : SIDE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : PIECE_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }
    for (auto& pair : CASTLING_MAP) {
      scope_chain_.InsertSymbol(pair.first, NewNumber(pair.second));
    }

    // 関数を登録。
    LC_Function func;
    std::string help;

    func = LC_FUNCTION_OBJ(SayuriLicense);
    INSERT_LC_FUNCTION(func, "sayuri-license", "Sayulisp:sayuri-license");
    help =
R"...(### sayuri-license ###

<h6> Usage </h6>

* `(sayuri-license)`

<h6> Description </h6>

* Returns String of license terms of Sayuri.

<h6> Example </h6>

    (display (sayuri-license))
    
    ;; Output
    ;; > Copyright (c) 2013-2016 Hironori Ishibashi
    ;; > 
    ;; > Permission is hereby granted, free of charge, to any person obtaining
    :: > a copy
    ;; > of this software and associated documentation files (the "Software"),
    ;; > to
    ;; > deal in the Software without restriction, including without limitation
    ;; > the
    ;; > rights to use, copy, modify, merge, publish, distribute, sublicense,
    ;; > and/or
    ;; > sell copies of the Software, and to permit persons to whom the
    ;; > Software is
    ;; > furnished to do so, subject to the following conditions:
    ;; > 
    ;; > The above copyright notice and this permission notice shall be
    ;; > included in
    ;; > all copies or substantial portions of the Software.
    ;; > 
    ;; > THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    ;; > EXPRESS OR
    ;; > IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    ;; > MERCHANTABILITY,
    ;; > FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
    ;; > SHALL THE
    ;; > AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    ;; > LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ;; > ARISING
    ;; > FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    ;; > DEALINGS IN THE SOFTWARE.)...";
    help_dict_.emplace("sayuri-license", help);

    func = LC_FUNCTION_OBJ(SquareToNumber);
    INSERT_LC_FUNCTION(func, "square->number", "Sayulisp:square->number");
    help =
R"...(### square->number ###

<h6> Usage </h6>

* `(square->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Square Symbol, it returns Number indicating to Square.
* If `<Object>` is List, it returns List changed Square Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(A1 B1 C1 (WHITE D3 E4 (F5 PAWN G6) H7 BLACK_LONG_CASTLING)))
    
    (display (square->number symbol-list))
    ;; Output
    ;; > (0 1 2 (WHITE 19 28 (37 PAWN 46) 55 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("square->number", help);

    func = LC_FUNCTION_OBJ(FyleToNumber);
    INSERT_LC_FUNCTION(func, "fyle->number", "Sayulisp:fyle->number");
    help =
R"...(### fyle->number ###

<h6> Usage </h6>

* `(fyle->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Fyle Symbol, it returns Number indicating to Fyle.
* If `<Object>` is List, it returns List changed Fyle Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(FYLE_A FYLE_B (WHITE FYLE_D E4 (PAWN G6) FYLE_H BLACK_LONG_CASTLING)))
    
    (display (fyle->number symbol-list))
    ;; Output
    ;; > (0 1 (WHITE 3 E4 (PAWN G6) 7 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("fyle->number", help);

    func = LC_FUNCTION_OBJ(RankToNumber);
    INSERT_LC_FUNCTION(func, "rank->number", "Sayulisp:rank->number");
    help =
R"...(### rank->number ###

<h6> Usage </h6>

* `(rank->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Rank Symbol, it returns Number indicating to Rank.
* If `<Object>` is List, it returns List changed Rank Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(RANK_1 RANK_2 (WHITE RANK_4 E4 (PAWN G6) RANK_8 BLACK_LONG_CASTLING)))
    
    (display (rank->number symbol-list))
    ;; Output
    ;; > (0 1 (WHITE 3 E4 (PAWN G6) 7 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("rank->number", help);

    func = LC_FUNCTION_OBJ(SideToNumber);
    INSERT_LC_FUNCTION(func, "side->number", "Sayulisp:side->number");
    help =
R"...(### side->number ### {#side-to-number}

<h6> Usage </h6>

* `(side->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Side Symbol, it returns Number indicating to Side.
* If `<Object>` is List, it returns List changed Side Symbol into Number. 

<h6> Example </h6>

    (define symbol-list
      '(NO_SIDE WHITE (FYLE_A BLACK E4 (PAWN G6) BLACK_LONG_CASTLING)))
    
    (display (side->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A 2 E4 (PAWN G6) BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("side->number", help);

    func = LC_FUNCTION_OBJ(PieceToNumber);
    INSERT_LC_FUNCTION(func, "piece->number", "Sayulisp:piece->number");
    help =
R"...(### piece->number ###

<h6> Usage </h6>

* `(piece->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Piece Type Symbol, it returns Number indicating
  to Piece Type.
* If `<Object>` is List, it returns List changed Piece Type Symbol into Number.

<h6> Example </h6>

    (define symbol-list
      '(EMPTY PAWN (FYLE_A QUEEN E4 (RANK_4 G6) KING BLACK_LONG_CASTLING)))
    
    (display (piece->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A 5 E4 (RANK_4 G6) 6 BLACK_LONG_CASTLING)))...";
    help_dict_.emplace("piece->number", help);

    func = LC_FUNCTION_OBJ(CastlingToNumber);
    INSERT_LC_FUNCTION(func, "castling->number", "Sayulisp:castling->number");
    help =
R"...(### castling->number ###

<h6> Usage </h6>

* `(castling->number <Object>)`

<h6> Description </h6>

* If `<Object>` is Castling Right Symbol, it returns Number indicating
  to Piece Type.
* If `<Object>` is List, it returns List changed Castling Right Symbol
  into Number. 

<h6> Example </h6>

    (define symbol-list
      '(NO_CASTLING WHITE_SHORT_CASTLING (FYLE_A E4 (RANK_4 G6) KING)))
    
    (display (castling->number symbol-list))
    ;; Output
    ;; > (0 1 (FYLE_A E4 (RANK_4 G6) KING)))...";
    help_dict_.emplace("castling->number", help);

    func = LC_FUNCTION_OBJ(NumberToSquare);
    INSERT_LC_FUNCTION(func, "number->square", "Sayulisp:number->square");
    help =
R"...(### number->square ### {#number-to-square}

<h6> Usage </h6>

* `(number->square <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Square Symbol.
* If `<Object>` is List, it returns List changed Number into Square Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->square number-list))
    ;; Output
    ;; > (A1 B1 (C1 (D1 E1 "Hello") F1) 100))...";
    help_dict_.emplace("number->square", help);

    func = LC_FUNCTION_OBJ(NumberToFyle);
    INSERT_LC_FUNCTION(func, "number->fyle", "Sayulisp:number->fyle");
    help =
R"...(### number->fyle ### {#number-to-fyle}

<h6> Usage </h6>

* `(number->fyle <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Fyle Symbol.
* If `<Object>` is List, it returns List changed Number into Fyle Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->fyle number-list))
    ;; Output
    ;; > (FYLE_A FYLE_B (FYLE_C (FYLE_D FYLE_E "Hello") FYLE_F) 100))...";
    help_dict_.emplace("number->fyle", help);

    func = LC_FUNCTION_OBJ(NumberToRank);
    INSERT_LC_FUNCTION(func, "number->rank", "Sayulisp:number->rank");
    help =
R"...(### number->rank ### {#number-to-rank}

<h6> Usage </h6>

* `(number->rank <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Rank Symbol.
* If `<Object>` is List, it returns List changed Number into Rank Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->rank number-list))
    ;; Output
    ;; > (RANK_1 RANK_2 (RANK_3 (RANK_4 RANK_5 "Hello") RANK_6) 100))...";
    help_dict_.emplace("number->rank", help);

    func = LC_FUNCTION_OBJ(NumberToSide);
    INSERT_LC_FUNCTION(func, "number->side", "Sayulisp:number->side");
    help =
R"...(### number->side ### {#number-to-side}

<h6> Usage </h6>

* `(number->side <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Side Symbol.
* If `<Object>` is List, it returns List changed Number into Side Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->side number-list))
    ;; Output
    ;; > (NO_SIDE WHITE (BLACK (3 4 "Hello") 5) 100))...";
    help_dict_.emplace("number->side", help);

    func = LC_FUNCTION_OBJ(NumberToPiece);
    INSERT_LC_FUNCTION(func, "number->piece", "Sayulisp:number->piece");
    help =
R"...(### number->piece ### {#number-to-piece}

<h6> Usage </h6>

* `(number->piece <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Piece Type Symbol.
* If `<Object>` is List, it returns List changed Number into Piece Type Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->piece number-list))
    ;; Output
    ;; > (EMPTY PAWN (KNIGHT (BISHOP ROOK "Hello") QUEEN) 100))...";
    help_dict_.emplace("number->piece", help);

    func = LC_FUNCTION_OBJ(NumberToCastling);
    INSERT_LC_FUNCTION(func, "number->castling", "Sayulisp:number->castling");
    help =
R"...(### number->castling ### {#number-to-castling}

<h6> Usage </h6>

* `(number->castling <Object>)`

<h6> Description </h6>

* If `<Object>` is Number, it returns Castling Rights Symbol.
* If `<Object>` is List, it returns List changed Number
  into CAstling Rights Symbol.

<h6> Example </h6>

    (define number-list '(0 1 (2 (3 4 "Hello") 5) 100))
    
    (display (number->castling number-list))
    ;; Output
    ;; > (NO_CASTLING WHITE_SHORT_CASTLING (WHITE_LONG_CASTLING
    ;; > (BLACK_SHORT_CASTLING BLACK_LONG_CASTLING "Hello") 5) 100))...";
    help_dict_.emplace("number->castling", help);

    func = LC_FUNCTION_OBJ(GenEngine);
    INSERT_LC_FUNCTION(func, "gen-engine", "Sayulisp:gen-engine");
    help =
R"...(### gen-engine ###

<h6> Usage </h6>

1. `(gen-engine)`
2. `((gen-engine) <Message Symbol> [<Arguments>...])`

<h6> Description </h6>

* 1: Generates chess engine.
* 2: The engine executes something according to `<Message Symbol>`.
* 2: Some `<Message Symbol>` require `<Argument>...`.
* `(help "engine <MessageSymbol>")`
    + Returns description for each message symbol.

<h6> Example </h6>

    (define my-engine (gen-engine))
    (display (my-engine '@get-white-pawn-position))
    ;; Output
    ;; > (A2 B2 C2 D2 E2 F2 G2 H2)
    
    (display (help "engine @get-white-pawn-position"))
    ;; Output
    ;; > ### Getting squares ###
    ;; > Returns List of Symbols of squares where specific pieces are on.
    ;; > 
    ;; > * `@get-white-pawn-position`
    ;; > * `@get-white-knight-position`
    ;; > * `@get-white-bishop-position`
    ;; > * `@get-white-rook-position`
    ;; > * `@get-white-queen-position`
    ;; > * `@get-white-king-position`
    ;; > * `@get-black-pawn-position`
    ;; > * `@get-black-knight-position`
    ;; > * `@get-black-bishop-position`
    ;; > * `@get-black-rook-position`
    ;; > * `@get-black-queen-position`
    ;; > * `@get-black-king-position`
    ;; > * `@get-empty-square-position`
    ;; > 
    ;; > <h6> Example </h6>
    ;; > 
    ;; >     (define my-engine (gen-engine))
    ;; >     (display (my-engine '@get-white-pawn-position))
    ;; >     
    ;; >     ;; Output
    ;; >     ;; > (A2 B2 C2 D2 E2 F2 G2 H2))...";
    help_dict_.emplace("gen-engine", help);

    func = LC_FUNCTION_OBJ(GenPGN);
    INSERT_LC_FUNCTION(func, "gen-pgn", "Sayulisp:gen-pgn");
    help =
R"...(### gen-pgn ###

<h6> Usage </h6>

* `(gen-pgn <PGN string : String>)`

<h6> Description </h6>

* Generates and returns PGN object from `<PGN string>`.
* PGN object is operated by Message Symbol.
* PGN object has 2 states.
    + Current game.
        - This can be changed by `@set-current-game`.
    + Current move.
        - This can be changed by `@next-move`, `@prev-move`, `@alt-move`,
          `@orig-move`, `@rewind-move`.

<h6> Description of Message Symbols </h6>

* `@get-pgn-comments`
    + Returns Lists of comments about PGN.

* `@get-current-game-comments.`
    + Returns List of comments about the current game.

* `@get-current-move-comments`
    + Returns List of comments about the current move.

* `@length`
    + Returns the number of games that PGN has.

* `@set-current-game <Index : Number>`
    + Sets a current game into the `<Index>`th game.

* `@get-current-game-headers`
    + Returns List of Lists composed with headers of the current game.
        - The format is "`((<Name 1> <value 1>) (<Name 2> <Value 2>)...)`".

* `@current-move`
    + Returns the current move text.

* `@next-move`
    + Change the current move into the next move
      and returns the move text.

* `@prev-move`
    + Change the current move into the previous move
      and returns the move text.

* `@alt-move`
    + Change the current move into the alternative move
      and returns the move text.

* `@orig-move`
    + If the current move is an alternative move,
      then change a current move into the original move
      and returns the move text.

* `@rewind-move`
    + Change a current move into the first move
      and returns the move text.

<h6> Example </h6>

    ;; Open PGN File.
    (define pgn-file (input-stream "/path/to/pgnfile.pgn"))
    
    ;; Reads the file and generates PGN object.
    (define my-pgn (gen-pgn (pgn-file '@read)))
    
    ;; Displays the current game headers.
    (display (my-pgn '@get-current-game-headers))
    
    ;; Output
    ;; > (("Black" "Hanako Yamada") ("Site" "Japan")
    ;; > ("White" "Hironori Ishibashi")))...";
    help_dict_.emplace("gen-pgn", help);

    func = LC_FUNCTION_OBJ(ParseFENEPD);
    INSERT_LC_FUNCTION(func, "parse-fen/epd", "Sayulisp:parse-fen/epd");
    help =
R"...(### parse-fen/epd ###

<h6> Usage </h6>

* `(parse-fen/epd <FEN or EPD : String>)`

<h6> Description </h6>

* Parses `<FEN or EPD>` and returns result value.
    +  A result value is `((<Tag 1 : String> <Object 1>)...)`.

<h6> Example </h6>

    (display (parse-fen/epd
        "rnbqkbnr/pp2pppp/3p4/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3"))
    ;; Output
    ;; > (("fen castling" (WHITE_SHORT_CASTLING
    ;; > WHITE_LONG_CASTLING BLACK_SHORT_CASTLING BLACK_LONG_CASTLING))
    ;; > ("fen clock" 0)
    ;; > ("fen en_passant" D3)
    ;; > ("fen ply" 5)
    ;; > ("fen position" ((WHITE ROOK) (WHITE KNIGHT) (WHITE BISHOP)
    ;; > (WHITE QUEEN) (WHITE KING) (WHITE BISHOP) (NO_SIDE EMPTY)
    ;; > (WHITE ROOK) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (WHITE KNIGHT) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (WHITE PAWN)
    ;; > (WHITE PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN)
    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (BLACK PAWN) (BLACK PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK ROOK)
    ;; > (BLACK KNIGHT) (BLACK BISHOP) (BLACK QUEEN) (BLACK KING)
    ;; > (BLACK BISHOP) (BLACK KNIGHT) (BLACK ROOK)))
    ;; > ("fen to_move" BLACK)))...";
    help_dict_.emplace("parse-fen/epd", help);

    func = LC_FUNCTION_OBJ(ToFENPosition);
    INSERT_LC_FUNCTION(func, "to-fen-position", "Sayulisp:to-fen-position");
    help =
R"...(### to-fen-position ###

<h6> Usage </h6>

* `(to-fen-position <Pieces list : List>)`

<h6> Description </h6>

* Analyses `<Pieces list>` and returns FEN position.

<h6> Example </h6>

    (display (to-fen-position
        '((WHITE KING) (WHITE KING)(WHITE KING) (WHITE KING)
        (WHITE QUEEN) (WHITE QUEEN)(WHITE QUEEN) (WHITE QUEEN)
        (WHITE ROOK) (WHITE ROOK)(WHITE ROOK) (WHITE ROOK)
        (WHITE BISHOP) (WHITE BISHOP)(WHITE BISHOP) (WHITE BISHOP)
        (WHITE KNIGHT) (WHITE KNIGHT)(WHITE KNIGHT) (WHITE KNIGHT)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
        (BLACK KNIGHT) (BLACK KNIGHT)(BLACK KNIGHT) (BLACK KNIGHT)
        (BLACK BISHOP) (BLACK BISHOP)(BLACK BISHOP) (BLACK BISHOP)
        (BLACK ROOK) (BLACK ROOK)(BLACK ROOK) (BLACK ROOK)
        (BLACK QUEEN) (BLACK QUEEN)(BLACK QUEEN) (BLACK QUEEN)
        (BLACK KING) (BLACK KING)(BLACK KING) (BLACK KING))))
    ;; Output
    ;; > qqqqkkkk/bbbbrrrr/4nnnn/8/8/NNNN4/RRRRBBBB/KKKKQQQQ)...";
    help_dict_.emplace("to-fen-position", help);
  }

  // Sayulispを開始する。
  int Sayulisp::Run(std::istream* stream_ptr) {
    // 終了ステータス。
    int status = 0;

    // (exit)関数を作成。
    bool loop = true;
    auto func = [&status, &loop](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = args.cdr().get();

      // ループをセット。
      loop = false;

      // 引数があった場合は終了ステータスあり。
      if (args_ptr->IsPair()) {
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        status = result->number();
      }

      return Lisp::NewNumber(status);
    };
    scope_chain_.InsertSymbol("exit",
    NewN_Function(func, "Sayulisp:exit", scope_chain_));

    try {
      std::string input;
      while (std::getline(*(stream_ptr), input)) {
        input += "\n";
        Tokenize(input);
        LPointerVec s_tree = Parse();

        for (auto& s : s_tree) {
          Evaluate(*s);
        }

        if (!loop) break;
      }
    } catch (LPointer error) {
      PrintError(error);
    }

    return status;
  }

  // メッセージシンボル関数の準備をする。
  void Sayulisp::GetReadyForMessageFunction(const std::string& symbol,
  const LObject& args, int required_args, LObject** args_ptr_ptr) {
    const LPointer& message_args_ptr = args.cdr()->cdr();

    int ret = Lisp::CountList(*message_args_ptr);
    if (ret < required_args) {
      throw Lisp::GenError("@engine-error",
      "'" + symbol + "' requires "
      + std::to_string(required_args) + " arguments and more. Not "
      + std::to_string(ret) + ".");
    }

    *args_ptr_ptr = message_args_ptr.get();
  }

  // エンジンを生成する。
  DEF_LC_FUNCTION(Sayulisp::GenEngine) {
    // スイートを作成。
    std::shared_ptr<EngineSuite> suite_ptr(new EngineSuite());

    // ネイティブ関数オブジェクトを作成。
    auto func = [suite_ptr](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return (*suite_ptr)(self, caller, args);
    };

    return NewN_Function(func, "Sayulisp:gen-engine:"
    + std::to_string(reinterpret_cast<std::size_t>(suite_ptr.get())),
    caller->scope_chain());
  }

  // マスのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::SquareToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (SQUARE_MAP.find(ptr->symbol()) != SQUARE_MAP.end()) {
        return NewNumber(SQUARE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // ファイルのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::FyleToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (FYLE_MAP.find(ptr->symbol()) != FYLE_MAP.end()) {
        return NewNumber(FYLE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // ランクのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::RankToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (RANK_MAP.find(ptr->symbol()) != RANK_MAP.end()) {
        return NewNumber(RANK_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // サイドのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::SideToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (SIDE_MAP.find(ptr->symbol()) != SIDE_MAP.end()) {
        return NewNumber(SIDE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 駒の種類のシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::PieceToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (PIECE_MAP.find(ptr->symbol()) != PIECE_MAP.end()) {
        return NewNumber(PIECE_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // キャスリングのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::CastlingToNumber) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }
      if (CASTLING_MAP.find(ptr->symbol()) != CASTLING_MAP.end()) {
        return NewNumber(CASTLING_MAP.at(ptr->symbol()));
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をマスのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToSquare) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_SQUARES))) {
          return NewSymbol(SQUARE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をファイルのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToFyle) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_FYLES))) {
          return NewSymbol(FYLE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をランクのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToRank) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_RANKS))) {
          return NewSymbol(RANK_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をサイドのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToSide) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_SIDES))) {
          return NewSymbol(SIDE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値を駒の種類のシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToPiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < static_cast<int>(NUM_PIECE_TYPES))) {
          return NewSymbol(PIECE_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }

  // 数値をキャスリングのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToCastling) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<LPointer(LPointer)> func;
    func = [&func](LPointer ptr) -> LPointer {
      if (ptr->IsPair()) {
        ptr->car(func(ptr->car()));
        ptr->cdr(func(ptr->cdr()));
        return ptr;
      }

      if (ptr->IsNumber()) {
        int number = ptr->number();
        if ((number >= 0) && (number < 5)) {
          return NewSymbol(CASTLING_MAP_INV[number]);
        }
      }

      return ptr;
    };

    return func(caller->Evaluate(*(args_ptr->car())));
  }


  DEF_LC_FUNCTION(Sayulisp::GenPGN) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // PGN文字列を得る。
    LPointer pgn_str_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*pgn_str_ptr, LType::STRING);
    std::shared_ptr<PGN> pgn_ptr(new PGN());
    pgn_ptr->Parse(pgn_str_ptr->string());

    // 現在のゲームのインデックス。
    std::shared_ptr<int> current_index_ptr(new int(0));

    // メッセージシンボル用オブジェクトを作る。
    std::map<std::string, MessageFunction> message_func_map;

    message_func_map["@get-pgn-comments"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      const std::vector<std::string>& comment_vec = pgn_ptr->comment_vec();
      std::size_t len = comment_vec.size();

      LPointerVec ret_vec(len);

      // コメントをコピーする。
      for (unsigned int i = 0; i < len; ++i) {
        ret_vec[i] = NewString(comment_vec[i]);
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@get-current-game-comments"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在のゲームのコメントを得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const std::vector<std::string>& comment_vec =
      pgn_ptr->game_vec()[*current_index_ptr]->comment_vec();
      int len = comment_vec.size();

      LPointerVec ret_vec(len);
      for (int i = 0; i < len; ++i) {
        ret_vec[i] = NewString(comment_vec[i]);
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@get-current-move-comments"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在の指し手のコメントを得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const std::vector<std::string>& comment_vec =
      pgn_ptr->game_vec()[*current_index_ptr]->current_node_ptr()->
      comment_vec_;

      int len = comment_vec.size();

      LPointerVec ret_vec(len);
      for (int i = 0; i < len; ++i) {
        ret_vec[i] = NewString(comment_vec[i]);
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@length"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      return NewNumber(pgn_ptr->game_vec().size());
    };

    message_func_map["@set-current-game"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

      // インデックス番号を得る。
      LPointer index_ptr = caller->Evaluate(*(args_ptr->car()));
      CheckType(*index_ptr, LType::NUMBER);
      int len = pgn_ptr->game_vec().size();
      int index = index_ptr->number();
      index = index < 0 ? len + index : index;
      if ((index < 0) || (index >= len)) {
        throw GenError("@function-error", "Index '" + index_ptr->ToString()
        + "'is out of range.");
      }

      int old_index = *current_index_ptr;
      *current_index_ptr = index;

      return NewNumber(old_index);
    };

    message_func_map["@get-current-game-headers"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在のゲームのヘッダを得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const PGNHeader& header =
      pgn_ptr->game_vec()[*current_index_ptr]->header();
      int len = header.size();

      LPointerVec ret_vec(len);
      LPointer temp;
      int i = 0;
      for (auto& pair : header) {
        ret_vec[i] = NewPair(NewString(pair.first),
        NewPair(NewString(pair.second), NewNil()));
        ++i;
      }

      return LPointerVecToList(ret_vec);
    };

    message_func_map["@current-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 現在の指し手を得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const MoveNode* node_ptr =
      pgn_ptr->game_vec()[*current_index_ptr]->current_node_ptr();

      return NewString(node_ptr->text_);
    };

    message_func_map["@next-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 次の手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Next()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@prev-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 前の手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Back()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@alt-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 代替手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Alt()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@orig-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // オリジナルへ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Orig()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    message_func_map["@rewind-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol, LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // オリジナルへ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Rewind()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewString("");
    };

    // PGNオブジェクトを作成。
    auto pgn_func =
    [message_func_map](LPointer self, LObject* caller, const LObject& args)
    -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // メッセージシンボルを得る。
      LPointer symbol_ptr = caller->Evaluate(*(args_ptr->car()));
      CheckType(*symbol_ptr, LType::SYMBOL);
      std::string symbol = symbol_ptr->symbol();

      // メッセージシンボル関数を呼び出す。
      if (message_func_map.find(symbol) != message_func_map.end()) {
        return message_func_map.at(symbol)(symbol, self, caller, args);
      }

      throw Lisp::GenError("@engine-error",
      "'" + symbol + "' is not message symbol.");
    };

    return NewN_Function(pgn_func, "Sayulisp:gen-pgn:"
    + std::to_string(reinterpret_cast<std::size_t>(pgn_ptr.get())),
    caller->scope_chain());
  }

  // %%% parse-fen/epd
  DEF_LC_FUNCTION(Sayulisp::ParseFENEPD) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 文字列を得る。
    LPointer fen_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*fen_ptr, LType::STRING);

    // パースする。
    std::map<std::string, std::string> fen_map =
    Util::ParseFEN(fen_ptr->string());

    // パース結果を格納していく。
    LPointerVec ret_vec(fen_map.size());
    LPointerVec::iterator ret_itr = ret_vec.begin();
    LPointer tuple;
    for (auto& pair : fen_map) {
      tuple = NewList(2);
      tuple->car(NewString(pair.first));

      // ポジションをパース。
      if (pair.first == "fen position") {
        LPointerVec position_vec(NUM_SQUARES);
        LPointerVec::iterator position_itr = position_vec.begin();
        for (auto c : pair.second) {
          switch (c) {
            case 'P':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("PAWN"), NewNil()));
              break;
            case 'N':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("KNIGHT"), NewNil()));
              break;
            case 'B':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("BISHOP"), NewNil()));
              break;
            case 'R':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("ROOK"), NewNil()));
              break;
            case 'Q':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("QUEEN"), NewNil()));
              break;
            case 'K':
              *position_itr = NewPair(NewSymbol("WHITE"),
              NewPair(NewSymbol("KING"), NewNil()));
              break;
            case 'p':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("PAWN"), NewNil()));
              break;
            case 'n':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("KNIGHT"), NewNil()));
              break;
            case 'b':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("BISHOP"), NewNil()));
              break;
            case 'r':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("ROOK"), NewNil()));
              break;
            case 'q':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("QUEEN"), NewNil()));
              break;
            case 'k':
              *position_itr = NewPair(NewSymbol("BLACK"),
              NewPair(NewSymbol("KING"), NewNil()));
              break;
            default:
              *position_itr = NewPair(NewSymbol("NO_SIDE"),
              NewPair(NewSymbol("EMPTY"), NewNil()));
              break;
          }
          ++position_itr;
        }

        tuple->cdr()->car(LPointerVecToList(position_vec));
        *ret_itr = tuple;

        ++ret_itr;
        continue;
      }

      if (pair.first == "fen to_move") {
        if (pair.second == "w") tuple->cdr()->car(NewSymbol("WHITE"));
        else tuple->cdr()->car(NewSymbol("BLACK"));

        *ret_itr = tuple;

        ++ret_itr;
        continue;
      }

      if (pair.first == "fen castling") {
        LPointerVec castling_vec;
        if (pair.second[0] != '-') {
          castling_vec.push_back(NewSymbol("WHITE_SHORT_CASTLING"));
        }
        if (pair.second[1] != '-') {
          castling_vec.push_back(NewSymbol("WHITE_LONG_CASTLING"));
        }
        if (pair.second[2] != '-') {
          castling_vec.push_back(NewSymbol("BLACK_SHORT_CASTLING"));
        }
        if (pair.second[3] != '-') {
          castling_vec.push_back(NewSymbol("BLACK_LONG_CASTLING"));
        }

        tuple->cdr()->car(LPointerVecToList(castling_vec));
        *ret_itr = tuple;

        ++ret_itr;
        continue;
      }

      if (pair.first == "fen en_passant") {
        if (pair.second != "-") {
          char temp[3] {
            static_cast<char>(pair.second[0] - 'a' + 'A'),
            pair.second[1],
            '\0'
          };
          tuple->cdr()->car(NewSymbol(temp));
        } else {
          tuple->cdr()->car(NewNil());
        }

        *ret_itr = tuple;
        ++ret_itr;
        continue;
      }

      if ((pair.first == "fen ply") || (pair.first == "fen clock")) {
        tuple->cdr()->car(NewNumber(std::stod(pair.second)));
        *ret_itr = tuple;
        ++ret_itr;
        continue;
      }

      // EPD拡張部分。
      tuple->cdr()->car(NewString(pair.second));
      *ret_itr = tuple;
      ++ret_itr;
    }

    return LPointerVecToList(ret_vec);
  }

  // to-fen-position
  DEF_LC_FUNCTION(Sayulisp::ToFENPosition) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 文字列を得る。
    LPointer position_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*position_ptr);

    // ビットボードを作る。
    Bitboard position[NUM_SIDES][NUM_PIECE_TYPES];
    INIT_ARRAY(position);

    // ビットボードにビットをセット。
    LObject* ptr = position_ptr.get();
    FOR_SQUARES(square) {
      // 要素をチェック。
      if (!ptr) {
        throw GenError("@sayulisp-error",
        "Not enough elements. Required 64 elemsents.");
      }
      CheckPiece(*(ptr->car()));

      // セット。
      Side side = ptr->car()->car()->number();
      PieceType piece_type = ptr->car()->cdr()->car()->number();
      position[side][piece_type] |= Util::SQUARE[square][R0];

      Next(&ptr);
    }

    return NewString(Util::ToFENPosition(position));
  }
//  // ヘルプを作成する。
//  void Sayulisp::SetHelp() {
//    std::string temp = "";
//
//    // --- 定数 --- //
//    temp =
//R"...(### A1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A1 square.
//* Value is '0'.)...";
//    AddHelpDict("A1", temp);
//
//    temp =
//R"...(### B1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B1 square.
//* Value is '1'.)...";
//    AddHelpDict("B1", temp);
//
//    temp =
//R"...(### C1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C1 square.
//* Value is '2'.)...";
//    AddHelpDict("C1", temp);
//
//    temp =
//R"...(### D1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D1 square.
//* Value is '3'.)...";
//    AddHelpDict("D1", temp);
//
//    temp =
//R"...(### E1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E1 square.
//* Value is '4'.)...";
//    AddHelpDict("E1", temp);
//
//    temp =
//R"...(### F1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F1 square.
//* Value is '5'.)...";
//    AddHelpDict("F1", temp);
//
//    temp =
//R"...(### G1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G1 square.
//* Value is '6'.)...";
//    AddHelpDict("G1", temp);
//
//    temp =
//R"...(### H1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H1 square.
//* Value is '7'.)...";
//    AddHelpDict("H1", temp);
//
//temp =
//R"...(### A2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A2 square.
//* Value is '8'.)...";
//    AddHelpDict("A2", temp);
//
//    temp =
//R"...(### B2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B2 square.
//* Value is '9'.)...";
//    AddHelpDict("B2", temp);
//
//    temp =
//R"...(### C2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C2 square.
//* Value is '10'.)...";
//    AddHelpDict("C2", temp);
//
//    temp =
//R"...(### D2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D2 square.
//* Value is '11'.)...";
//    AddHelpDict("D2", temp);
//
//    temp =
//R"...(### E2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E2 square.
//* Value is '12'.)...";
//    AddHelpDict("E2", temp);
//
//    temp =
//R"...(### F2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F2 square.
//* Value is '13'.)...";
//    AddHelpDict("F2", temp);
//
//    temp =
//R"...(### G2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G2 square.
//* Value is '14'.)...";
//    AddHelpDict("G2", temp);
//
//    temp =
//R"...(### H2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H2 square.
//* Value is '15'.)...";
//    AddHelpDict("H2", temp);
//
//    temp =
//R"...(### A3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A3 square.
//* Value is '16'.)...";
//    AddHelpDict("A3", temp);
//
//    temp =
//R"...(### B3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B3 square.
//* Value is '17'.)...";
//    AddHelpDict("B3", temp);
//
//    temp =
//R"...(### C3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C3 square.
//* Value is '18'.)...";
//    AddHelpDict("C3", temp);
//
//    temp =
//R"...(### D3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D3 square.
//* Value is '19'.)...";
//    AddHelpDict("D3", temp);
//
//    temp =
//R"...(### E3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E3 square.
//* Value is '20'.)...";
//    AddHelpDict("E3", temp);
//
//    temp =
//R"...(### F3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F3 square.
//* Value is '21'.)...";
//    AddHelpDict("F3", temp);
//
//    temp =
//R"...(### G3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G3 square.
//* Value is '22'.)...";
//    AddHelpDict("G3", temp);
//
//    temp =
//R"...(### H3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H3 square.
//* Value is '23'.)...";
//    AddHelpDict("H3", temp);
//
//    temp =
//R"...(### A4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A4 square.
//* Value is '24'.)...";
//    AddHelpDict("A4", temp);
//
//    temp =
//R"...(### B4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B4 square.
//* Value is '25'.)...";
//    AddHelpDict("B4", temp);
//
//    temp =
//R"...(### C4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C4 square.
//* Value is '26'.)...";
//    AddHelpDict("C4", temp);
//
//    temp =
//R"...(### D4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D4 square.
//* Value is '27'.)...";
//    AddHelpDict("D4", temp);
//
//    temp =
//R"...(### E4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E4 square.
//* Value is '28'.)...";
//    AddHelpDict("E4", temp);
//
//    temp =
//R"...(### F4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F4 square.
//* Value is '29'.)...";
//    AddHelpDict("F4", temp);
//
//    temp =
//R"...(### G4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G4 square.
//* Value is '30'.)...";
//    AddHelpDict("G4", temp);
//
//    temp =
//R"...(### H4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H4 square.
//* Value is '31'.)...";
//    AddHelpDict("H4", temp);
//
//    temp =
//R"...(### A5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A5 square.
//* Value is '32'.)...";
//    AddHelpDict("A5", temp);
//
//    temp =
//R"...(### B5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B5 square.
//* Value is '33'.)...";
//    AddHelpDict("B5", temp);
//
//    temp =
//R"...(### C5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C5 square.
//* Value is '34'.)...";
//    AddHelpDict("C5", temp);
//
//    temp =
//R"...(### D5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D5 square.
//* Value is '35'.)...";
//    AddHelpDict("D5", temp);
//
//    temp =
//R"...(### E5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E5 square.
//* Value is '36'.)...";
//    AddHelpDict("E5", temp);
//
//    temp =
//R"...(### F5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F5 square.
//* Value is '37'.)...";
//    AddHelpDict("F5", temp);
//
//    temp =
//R"...(### G5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G5 square.
//* Value is '38'.)...";
//    AddHelpDict("G5", temp);
//
//    temp =
//R"...(### H5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H5 square.
//* Value is '39'.)...";
//    AddHelpDict("H5", temp);
//
//    temp =
//R"...(### A6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A6 square.
//* Value is '40'.)...";
//    AddHelpDict("A6", temp);
//
//    temp =
//R"...(### B6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B6 square.
//* Value is '41'.)...";
//    AddHelpDict("B6", temp);
//
//    temp =
//R"...(### C6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C6 square.
//* Value is '42'.)...";
//    AddHelpDict("C6", temp);
//
//    temp =
//R"...(### D6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D6 square.
//* Value is '43'.)...";
//    AddHelpDict("D6", temp);
//
//    temp =
//R"...(### E6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E6 square.
//* Value is '44'.)...";
//    AddHelpDict("E6", temp);
//
//    temp =
//R"...(### F6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F6 square.
//* Value is '45'.)...";
//    AddHelpDict("F6", temp);
//
//    temp =
//R"...(### G6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G6 square.
//* Value is '46'.)...";
//    AddHelpDict("G6", temp);
//
//    temp =
//R"...(### H6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H6 square.
//* Value is '47'.)...";
//    AddHelpDict("H6", temp);
//
//    temp =
//R"...(### A7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A7 square.
//* Value is '48'.)...";
//    AddHelpDict("A7", temp);
//
//    temp =
//R"...(### B7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B7 square.
//* Value is '49'.)...";
//    AddHelpDict("B7", temp);
//
//    temp =
//R"...(### C7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C7 square.
//* Value is '50'.)...";
//    AddHelpDict("C7", temp);
//
//    temp =
//R"...(### D7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D7 square.
//* Value is '51'.)...";
//    AddHelpDict("D7", temp);
//
//    temp =
//R"...(### E7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E7 square.
//* Value is '52'.)...";
//    AddHelpDict("E7", temp);
//
//    temp =
//R"...(### F7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F7 square.
//* Value is '53'.)...";
//    AddHelpDict("F7", temp);
//
//    temp =
//R"...(### G7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G7 square.
//* Value is '54'.)...";
//    AddHelpDict("G7", temp);
//
//    temp =
//R"...(### H7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H7 square.
//* Value is '55'.)...";
//    AddHelpDict("H7", temp);
//
//    temp =
//R"...(### A8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A8 square.
//* Value is '56'.)...";
//    AddHelpDict("A8", temp);
//
//    temp =
//R"...(### B8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B8 square.
//* Value is '57'.)...";
//    AddHelpDict("B8", temp);
//
//    temp =
//R"...(### C8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C8 square.
//* Value is '58'.)...";
//    AddHelpDict("C8", temp);
//
//    temp =
//R"...(### D8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D8 square.
//* Value is '59'.)...";
//    AddHelpDict("D8", temp);
//
//    temp =
//R"...(### E8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E8 square.
//* Value is '60'.)...";
//    AddHelpDict("E8", temp);
//
//    temp =
//R"...(### F8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F8 square.
//* Value is '61'.)...";
//    AddHelpDict("F8", temp);
//
//    temp =
//R"...(### G8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G8 square.
//* Value is '62'.)...";
//    AddHelpDict("G8", temp);
//
//    temp =
//R"...(### H8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H8 square.
//* Value is '63'.)...";
//    AddHelpDict("H8", temp);
//
//    temp =
//R"...(### FYLE_A ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates A-fyle.
//* Value is '0'.)...";
//    AddHelpDict("FYLE_A", temp);
//
//    temp =
//R"...(### FYLE_B ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates B-fyle.
//* Value is '1'.)...";
//    AddHelpDict("FYLE_B", temp);
//
//    temp =
//R"...(### FYLE_C ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates C-fyle.
//* Value is '2'.)...";
//    AddHelpDict("FYLE_C", temp);
//
//    temp =
//R"...(### FYLE_D ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates D-fyle.
//* Value is '3'.)...";
//    AddHelpDict("FYLE_D", temp);
//
//    temp =
//R"...(### FYLE_E ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates E-fyle.
//* Value is '4'.)...";
//    AddHelpDict("FYLE_E", temp);
//
//    temp =
//R"...(### FYLE_F ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates F-fyle.
//* Value is '5'.)...";
//    AddHelpDict("FYLE_F", temp);
//
//    temp =
//R"...(### FYLE_G ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates G-fyle.
//* Value is '6'.)...";
//    AddHelpDict("FYLE_G", temp);
//
//    temp =
//R"...(### FYLE_H ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates H-fyle.
//* Value is '7'.)...";
//    AddHelpDict("FYLE_H", temp);
//
//    temp =
//R"...(### RANK_1 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 1st rank.
//* Value is '0'.)...";
//    AddHelpDict("RANK_1", temp);
//
//    temp =
//R"...(### RANK_2 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 2nd rank.
//* Value is '1'.)...";
//    AddHelpDict("RANK_2", temp);
//
//    temp =
//R"...(### RANK_3 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 3rd rank.
//* Value is '2'.)...";
//    AddHelpDict("RANK_3", temp);
//
//    temp =
//R"...(### RANK_4 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 4th rank.
//* Value is '3'.)...";
//    AddHelpDict("RANK_4", temp);
//
//    temp =
//R"...(### RANK_5 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 5th rank.
//* Value is '4'.)...";
//    AddHelpDict("RANK_5", temp);
//
//    temp =
//R"...(### RANK_6 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 6th rank.
//* Value is '5'.)...";
//    AddHelpDict("RANK_6", temp);
//
//    temp =
//R"...(### RANK_7 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 7th rank.
//* Value is '6'.)...";
//    AddHelpDict("RANK_7", temp);
//
//    temp =
//R"...(### RANK_8 ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates the 8th rank.
//* Value is '7'.)...";
//    AddHelpDict("RANK_8", temp);
//
//    temp =
//R"...(### NO_SIDE ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates neither of sides.
//* Value is '0'.)...";
//    AddHelpDict("NO_SIDE", temp);
//
//    temp =
//R"...(### WHITE ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates White.
//* Value is '1'.)...";
//    AddHelpDict("WHITE", temp);
//
//    temp =
//R"...(### BLACK ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Black.
//* Value is '2'.)...";
//    AddHelpDict("BLACK", temp);
//
//    temp =
//R"...(### EMPTY ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates no piece.
//* Value is '0'.)...";
//    AddHelpDict("EMPTY", temp);
//
//    temp =
//R"...(### PAWN ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Pawn.
//* Value is '1'.)...";
//    AddHelpDict("PAWN", temp);
//
//    temp =
//R"...(### KNIGHT ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Knight.
//* Value is '2'.)...";
//    AddHelpDict("KNIGHT", temp);
//
//    temp =
//R"...(### BISHOP ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Bishop.
//* Value is '3'.)...";
//    AddHelpDict("BISHOP", temp);
//
//    temp =
//R"...(### ROOK ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Rook.
//* Value is '4'.)...";
//    AddHelpDict("ROOK", temp);
//
//    temp =
//R"...(### QUEEN ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Queen.
//* Value is '5'.)...";
//    AddHelpDict("QUEEN", temp);
//
//    temp =
//R"...(### KING ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates King.
//* Value is '6'.)...";
//    AddHelpDict("KING", temp);
//
//    temp =
//R"...(### NO_CASTLING ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates no one to castle.
//* Value is '0'.)...";
//    AddHelpDict("NO_CASTLING", temp);
//
//    temp =
//R"...(### WHITE_SHORT_CASTLING ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates White's Short Castling.
//* Value is '1'.)...";
//    AddHelpDict("WHITE_SHORT_CASTLING", temp);
//
//    temp =
//R"...(### WHITE_LONG_CASTLING ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates White's Long Castling.
//* Value is '2'.)...";
//    AddHelpDict("WHITE_LONG_CASTLING", temp);
//
//    temp =
//R"...(### BLACK_SHORT_CASTLING ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Black's Short Castling.
//* Value is '3'.)...";
//    AddHelpDict("BLACK_SHORT_CASTLING", temp);
//
//    temp =
//R"...(### BLACK_LONG_CASTLING ###
//
//<h6> Description </h6>
//
//* Constant value of Number that indicates Black's Long Castling.
//* Value is '4'.)...";
//
//    AddHelpDict("BLACK_LONG_CASTLING", temp);
//    // %%% exit
//
//    temp =
//R"...(### Getting squares ###
//
//* `@get-white-pawn-position`
//    + Returns List of Symbols of squares where White Pawns are on.
//* `@get-white-knight-position`
//    + Returns List of Symbols of squares where White Knights are on.
//* `@get-white-bishop-position`
//    + Returns List of Symbols of squares where White Bishops are on.
//* `@get-white-rook-position`
//    + Returns List of Symbols of squares where White Rooks are on.
//* `@get-white-queen-position`
//    + Returns List of Symbols of squares where White Queens are on.
//* `@get-white-king-position`
//    + Returns List of Symbols of squares where White King is on.
//* `@get-black-pawn-position`
//    + Returns List of Symbols of squares where Black Pawns are on.
//* `@get-black-knight-position`
//    + Returns List of Symbols of squares where Black Knights are on.
//* `@get-black-bishop-position`
//    + Returns List of Symbols of squares where Black Bishops are on.
//* `@get-black-rook-position`
//    + Returns List of Symbols of squares where Black Rooks are on.
//* `@get-black-queen-position`
//    + Returns List of Symbols of squares where Black Queens are on.
//* `@get-black-king-position`
//    + Returns List of Symbols of squares where Black King is on.
//* `@get-empty-square-position`
//    + Returns List of Symbols of empty squares.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    (display (my-engine '@get-white-pawn-position))
//    
//    ;; Output
//    ;; > (A2 B2 C2 D2 E2 F2 G2 H2))...";
//    AddHelpDict("engine @get-white-pawn-position", temp);
//    AddHelpDict("engine @get-white-knight-position", temp);
//    AddHelpDict("engine @get-white-bishop-position", temp);
//    AddHelpDict("engine @get-white-rook-position", temp);
//    AddHelpDict("engine @get-white-queen-position", temp);
//    AddHelpDict("engine @get-white-king-position", temp);
//    AddHelpDict("engine @get-black-pawn-position", temp);
//    AddHelpDict("engine @get-black-knight-position", temp);
//    AddHelpDict("engine @get-black-bishop-position", temp);
//    AddHelpDict("engine @get-black-rook-position", temp);
//    AddHelpDict("engine @get-black-queen-position", temp);
//    AddHelpDict("engine @get-black-king-position", temp);
//
//    temp =
//R"...(### Getting pieces ###
//
//* `@get-piece <Square : Number or Symbol>`
//    + Returns a side and type of the piece on `<Square>` as List.
//
//* `@get-all-pieces`
//    + Returns  pieces of each square on the board as List.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    (display (my-engine '@get-piece D1))
//    
//    ;; Output
//    ;; > (WHITE QUEEN)
//    
//    (display (my-engine '@get-all-pieces))
//    
//    ;; Output
//    ;; > ((WHITE ROOK) (WHITE KNIGHT) (WHITE BISHOP) (WHITE QUEEN)
//    ;; > (WHITE KING) (WHITE BISHOP) (WHITE KNIGHT) (WHITE ROOK)
//    ;; > (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN)
//    ;; > (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN)
//    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN)
//    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK ROOK) (BLACK KNIGHT) (BLACK BISHOP)
//    ;; > (BLACK QUEEN) (BLACK KING) (BLACK BISHOP) (BLACK KNIGHT)
//    ;; > (BLACK ROOK)))...";
//    AddHelpDict("engine @get-piece", temp);
//    AddHelpDict("engine @get-all-pieces", temp);
//
//    temp =
//R"...(### Getting states of game ###
//
//* `@get-to-move`
//    + Returns turn to move as Symbol.
//* `@get-castling-rights`
//    + Returns castling rights as Symbol.
//* `@get-en-passant-square`
//    + Returns en passant square as Symbol if it exists now.
//* `@get-ply`
//    + Returns plies of moves from starting of the game.
//    + 1 move = 2 plies.
//* `@get-clock`
//    + Returns clock(plies for 50 Moves Rule).
//* `@get-white-has-castled`
//    + Returns Boolean whether White King has castled or not.
//* `@get-black-has-castled`
//    + Returns Boolean whether Black King has castled or not.
//* `@get-fen`
//    + Returns FEN of current position.
//* `@to-string`
//    + Returns visualized board.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    ;; Move pieces by UCI command.
//    ;; 1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 4.O-O d5
//    ;; +---------------+
//    ;; |r . b q k . n r|
//    ;; |p p p . . p p p|
//    ;; |. . n . . . . .|
//    ;; |. . b p p . . .|
//    ;; |. . B . P . . .|
//    ;; |. . . . . N . .|
//    ;; |P P P P . P P P|
//    ;; |R N B Q . R K .|
//    ;; +---------------+
//    (my-engine '@input-uci-command
//        "position startpos moves e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 e1g1 d7d5")
//    
//    (display (my-engine '@get-to-move))
//    ;; Output
//    ;; > Symbol: WHITE
//    
//    (display (my-engine '@get-castling-rights))
//    ;; Output
//    ;; > (BLACK_SHORT_CASTLING BLACK_LONG_CASTLING)
//    
//    (display (my-engine '@get-en-passant-square))
//    ;; Output
//    ;; > Symbol: D6
//    
//    (display (my-engine '@get-ply))
//    ;; Output
//    ;; > 9
//    
//    (display (my-engine '@get-clock))
//    ;; Output
//    ;; > 0
//    
//    (display (my-engine '@get-white-has-castled))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@get-black-has-castled))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@get-fen))
//    ;; Output
//    ;; > r1bqk1nr/ppp2ppp/2n5/2bpp3/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 w kq d6 0 5
//    
//    (display (my-engine '@to-string))
//    ;; Output
//    ;; >  +-----------------+
//    ;; > 8| r . b q k . n r |
//    ;; > 7| p p p . . p p p |
//    ;; > 6| . . n . . . . . |
//    ;; > 5| . . b p p . . . |
//    ;; > 4| . . B . P . . . |
//    ;; > 3| . . . . . N . . |
//    ;; > 2| P P P P . P P P |
//    ;; > 1| R N B Q . R K . |
//    ;; >  +-----------------+
//    ;; >    a b c d e f g h
//    ;; > To Move: w | Clock: 0 | Ply: 8
//    ;; > En Passant Square: d6
//    ;; > Castling Rights : kq)...";
//    AddHelpDict("engine @get-to-move", temp);
//    AddHelpDict("engine @get-castling-rights", temp);
//    AddHelpDict("engine @get-en-passant-square", temp);
//    AddHelpDict("engine @get-ply", temp);
//    AddHelpDict("engine @get-clock", temp);
//    AddHelpDict("engine @get-white-has-castled", temp);
//    AddHelpDict("engine @get-black-has-castled", temp);
//
//    temp =
//R"...(### Setting states of game ###
//
//* `@set-to-move <Side : Number or Symbol>`
//    + Sets turn to move.
//    + Returns previous setting.
//* `@set-castling_rights <Castling rights : List>`
//    + Sets castling rights.
//    + Returns previous setting.
//* `@set-en-passant-square <<Square : Number or Symbol> or <Nil>>`
//    + Sets en passant square.
//    + Returns previous setting.
//* `@set-ply <Ply : Number>`
//    + Sets plies(a half of one move).
//    + Returns previous setting.
//* `@set-clock <Ply : Number>`
//    + Sets clock(plies for 50 moves rule).
//    + Returns previous setting.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    (my-engine '@place-piece E4 PAWN WHITE)
//    
//    (display (my-engine '@set-to-move BLACK))
//    ;; Output
//    ;; > Symbol: WHITE
//    
//    (display (my-engine '@set-castling-rights
//        (list WHITE_LONG_CASTLING BLACK_LONG_CASTLING)))
//    ;; Output
//    ;; > (WHITE_SHORT_CASTLING WHITE_LONG_CASTLING
//    ;; > BLACK_SHORT_CASTLING BLACK_LONG_CASTLING)
//    
//    (display (my-engine '@set-en-passant-square E3))
//    ;; Output
//    ;; > ()
//    
//    (display (my-engine '@set-ply 111))
//    ;; Output
//    ;; > 1
//    
//    (display (my-engine '@set-clock 22))
//    ;; Output
//    ;; > 0
//    
//    ;; ---------------- ;;
//    ;; Current Settings ;;
//    ;; ---------------- ;;
//    
//    (display (my-engine '@get-to-move))
//    ;; Output
//    ;; > Symbol: BLACK
//    
//    (display (my-engine '@get-castling-rights))
//    ;; Output
//    ;; > (WHITE_LONG_CASTLING BLACK_LONG_CASTLING)
//    
//    (display (my-engine '@get-en-passant-square))
//    ;; Output
//    ;; > Symbol: E3
//    
//    (display (my-engine '@get-ply))
//    ;; Output
//    ;; > 111
//    
//    (display (my-engine '@get-clock))
//    ;; Output
//    ;; > 22)...";
//    AddHelpDict("engine @set-to-move", temp);
//    AddHelpDict("engine @set-castling-rights", temp);
//    AddHelpDict("engine @set-en-passant-square", temp);
//    AddHelpDict("engine @set-ply", temp);
//    AddHelpDict("engine @set-clock", temp);
//
//    temp =
//R"...(### Placing pieces ###
//
//* `@set-new-game`
//    + Sets starting position.
//    + Returns #t.
//* `@set-fen <FEN : String>`
//    + Sets position with FEN.
//    + Returns #t.
//* `@place-piece <Square : Number or Symbol> <Piece : List>`
//    + Sets a `<Piece>` on `<Square>`
//      and returns the previous piece placed on `<Square>`.
//    + `<Piece>` is `(<Side : Number or Symbol> <Type : Number or Symbol>).
//        - For example, White Pawn is `(list WHITE PAWN)`.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@set-new-game))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@set-fen
//        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1"))
//    ;; Output
//    ;; > #t
//    
//    ;; Sets Black Rook on D1 where White Queen is placed.
//    (display (my-engine '@place-piece D1 (list BLACK ROOK)))
//    ;; Output
//    ;; > (WHITE QUEEN))...";
//    AddHelpDict("engine @set-new-game", temp);
//    AddHelpDict("engine @set-fen", temp);
//    AddHelpDict("engine @place-piece", temp);
//
//    temp =
//R"...(### Getting candidate moves ###
//
//* `@get-candidate-moves`
//    + Generates and returns List of candidate moves.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@get-candidate-moves))
//    ;; Output
//    ;; >  ((H2 H4 EMPTY) (H2 H3 EMPTY) (G2 G4 EMPTY) (G2 G3 EMPTY)
//    ;; > (F2 F4 EMPTY) (F2 F3 EMPTY) (E2 E4 EMPTY) (E2 E3 EMPTY) (D2 D4 EMPTY)
//    ;; > (D2 D3 EMPTY) (C2 C4 EMPTY) (C2 C3 EMPTY) (B2 B4 EMPTY) (B2 B3 EMPTY)
//    ;; > (A2 A4 EMPTY) (A2 A3 EMPTY) (G1 H3 EMPTY) (G1 F3 EMPTY) (B1 C3 EMPTY)
//    ;; > (B1 A3 EMPTY)))...";
//    AddHelpDict("engine @get-candidate-moves", temp);
//
//    temp =
//R"...(### Predicate functions ###
//
//Judges each state of the current position.
//
//* `@correct-position?`
//    + If Pawn is on 1st or 8th rank, it returns #f.
//    + When turn to move is White, if Black King is checked,
//      then it returns #f.
//    + When turn to move is Black, if White King is checked,
//      then it returns #f.
//    + Otherwise, returns #t.
//* `@white-checked?`
//* `@black-checked?`
//* `@checkmated?`
//* `@stalemated?`
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    ;; Put Pawn on 1st rank.
//    (my-engine '@place-piece D1 PAWN WHITE)
//    
//    (display (my-engine '@correct-position?))
//    ;; Output
//    ;; > #f
//    
//    (define my-engine (gen-engine))
//    
//    ;; Move pieces by UCI command.
//    ;; 1.d4 e6 2.Nf3 Bb4+
//    ;; +---------------+
//    ;; |r n b q k . n r|
//    ;; |p p p p . p p p|
//    ;; |. . . . p . . .|
//    ;; |. . . . . . . .|
//    ;; |. b . P . . . .|
//    ;; |. . . . . N . .|
//    ;; |P P P . P P P P|
//    ;; |R N B Q K B . R|
//    ;; +---------------+
//    (my-engine '@input-uci-command
//        "position startpos moves d2d4 e7e6 g1f3 f8b4")
//    
//    (display (my-engine '@white-checked?))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@black-checked?))
//    ;; Output
//    ;; > #f
//    
//    (define my-engine (gen-engine))
//    
//    ;; Move pieces by UCI command.
//    ;; 1.f3 e5 2.g4 Qh4#
//    ;; +---------------+
//    ;; |r n b . k b n r|
//    ;; |p p p p . p p p|
//    ;; |. . . . . . . .|
//    ;; |. . . . p . . .|
//    ;; |. . . . . . P q|
//    ;; |. . . . . P . .|
//    ;; |P P P P P . . P|
//    ;; |R N B Q K B N R|
//    ;; +---------------+
//    (my-engine '@input-uci-command
//        "position startpos moves f2f3 e7e5 g2g4 d8h4")
//    
//    (display (my-engine '@checkmated?))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@stalemated?))
//    ;; Output
//    ;; > #f)...";
//    AddHelpDict("engine @correct-position?", temp);
//    AddHelpDict("engine @white-checked?", temp);
//    AddHelpDict("engine @black-checked?", temp);
//    AddHelpDict("engine @checkmated?", temp);
//    AddHelpDict("engine @stalemated?", temp);
//
//    temp =
//R"...(### Taking a move ###
//
//A move is represented by List.  The List is  
//`(<From : Number or Symbol>
//  <To : Number or Symbol>
//  <Promotion : Number or Symbol>)`.
//
//* `@play-move <Move : List>`
//    + Moves one piece legally.
//    + Returns #t if it has succeeded, otherwise returns #f.
//
//* `@undo-move`
//    + Undoes previous move.
//    + Returns previous move.
//
//* `@play-note <PGN move text : String>`
//    + Moves one piece legally with `<PGN move text>`.
//    + Returns #t if it has succeeded, otherwise returns #f.
//
//* `@move->note <Move : List>`
//    + Translates Move into PGN move text according to the current position.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@play-move (list E2 E4 EMPTY)))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@get-white-pawn-position))
//    ;; Output
//    ;; > (A2 B2 C2 D2 F2 G2 H2 E4)
//    
//    (display (my-engine '@undo-move))
//    ;; Output
//    ;; > (E2 E4 EMPTY)
//    
//    (display (my-engine '@get-white-pawn-position))
//    ;; Output
//    ;; > (A2 B2 C2 D2 E2 F2 G2 H2)
//    
//    (display (my-engine '@play-note "Nf3"))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@get-white-knight-position))
//    ;; Output
//    ;; > (B1 F3)
//    
//    (display (my-engine '@move->note (list B8 C6 EMPTY)))
//    ;; Output
//    ;; > Nc6)...";
//    AddHelpDict("engine @play-move", temp);
//    AddHelpDict("engine @undo-move", temp);
//    AddHelpDict("engine @play-note", temp);
//    AddHelpDict("engine @move->note", temp);
//
//    temp =
//R"...(### UCI Command ###
//
//* `@input-uci-command <UCI command : String>`
//    + Executes `<UCI command>`.
//    + If success, returns #t. Otherwise, returns #f.
//    + If you have input "go" command,
//      the engine starts thinking in background.
//      So control will come back soon.
//* `@add-uci-output-listener <Listener : Function>`
//    + Registers Function to receive UCI output from the engine.
//    + `<Listener>` is Function that has one argument(UCI output).
//* `@run`
//    + Runs as UCI Chess Engine until the engine gets "quit" command.
//    + The control doesn't come back while the engine is running.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    ;; Create a listener.
//    (define (listener message)
//        (display "I'm Listener : " message))
//    
//    ;; Register the listener.
//    (my-engine '@add-uci-output-listener listener)
//    
//    (display (my-engine '@input-uci-command "uci"))
//    ;; Output
//    ;; > I'm Listener : id name Sayuri 2015.03.27 devel
//    ;; > I'm Listener : id author Hironori Ishibashi
//    ;; > I'm Listener : option name Hash type spin default 32 min 8 max 8192
//    ;; > I'm Listener : option name Clear Hash type button
//    ;; > I'm Listener : option name Ponder type check default true
//    ;; > I'm Listener : option name Threads type spin default 1 min 1 max 64
//    ;; > I'm Listener : option name UCI_AnalyseMode type check default false
//    ;; > I'm Listener : uciok
//    ;; > #t)...";
//    AddHelpDict("engine @input-uci-command", temp);
//    AddHelpDict("engine @add-uci-output-listener", temp);
//    AddHelpDict("engine @run", temp);
//
//    temp =
//R"...(### Searching the best move ###
//
//Thinks and returns the best move in each condition.  
//Different from "go" command,
//the control won't come back until the engine have found the best move.
//
//* `@go-movetime <Milliseconds : Number> [<Candidate move list : List>]`
//    + Thinks for `<Milliseconds>`.
//* `@go-timelimit <Milliseconds : Number> [<Candidate move list : List>]`
//    + Thinks on the basis of `<Milliseconds>`.
//        - If `<Milliseconds>` is more than 600000,
//          the engine thinks for 60000 milliseconds.
//        - If `<Milliseconds>` is less than 600000,
//          the engine thinks for "`<Milliseconds>` / 10" milliseconds.
//* `@go-depth <Ply : Number> [<Candidate move list : List>]`
//    + Thinks until to reach `<Ply>`th depth.
//* `@go-nodes <Nodes : Number> [<Candidate move list : List>]`
//    + Thinks until to search `<Nodes>` nodes.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    ;; Register a listener.
//    (define (listener message) (display "Engine > " message))
//    (my-engine '@add-uci-output-listener listener)
//    
//    (display (my-engine '@go-movetime 10000))
//    ;; Output
//    ;; > Engine > info depth 1
//    ;; > Engine > info currmove h2h4 currmovenumber 1
//    ;; > Engine > info depth 1 seldepth 1 score cp 12 time 0 nodes 2 pv h2h4
//    ;; > Engine > info currmove h2h3 currmovenumber 2
//    ;; > Engine > info depth 1 seldepth 1 score cp 22 time 1 nodes 4 pv h2h3
//    ;; > Engine > info currmove g2g4 currmovenumber 3
//    ;; > Engine > info depth 1 seldepth 1 score cp 23 time 1 nodes 6 pv g2g4
//    ;; > Engine > info currmove g2g3 currmovenumber 4
//    ;; > Engine > info depth 1 seldepth 1 score cp 33 time 1 nodes 8 pv g2g3
//    ;; > Engine > info currmove f2f4 currmovenumber 5
//    ;; > Engine > info currmove f2f3 currmovenumber 6
//    ;; > Engine > info depth 1 seldepth 1 score cp 36 time 1 nodes 11 pv f2f3
//    ;; > Engine > info currmove e2e4 currmovenumber 7
//    ;; > Engine > info depth 1 seldepth 1 score cp 45 time 1 nodes 13 pv e2e4
//    ;; > Engine > info currmove e2e3 currmovenumber 8
//    ;; > Engine > info currmove d2d4 currmovenumber 9
//    ;; > Engine > info depth 1 seldepth 1 score cp 50 time 1 nodes 16 pv d2d4
//    ;; > Engine > info currmove d2d3 currmovenumber 10
//    ;; > Engine > info currmove c2c4 currmovenumber 11
//    ;; > Engine > info currmove c2c3 currmovenumber 12
//    ;; > Engine > info currmove b2b4 currmovenumber 13
//    ;; > Engine > info currmove b2b3 currmovenumber 14
//    ;; > Engine > info currmove a2a4 currmovenumber 15
//    ;; > Engine > info currmove a2a3 currmovenumber 16
//    ;; > Engine > info currmove g1h3 currmovenumber 17
//    ;; > Engine > info currmove g1f3 currmovenumber 18
//    ;; > Engine > info depth 1 seldepth 1 score cp 68 time 1 nodes 26 pv g1f3
//    ;; > Engine > info currmove b1c3 currmovenumber 19
//    ;; > Engine > info currmove b1a3 currmovenumber 20
//    ;;
//    ;; (Omitted)
//    ;;
//    ;; > Engine > info depth 11
//    ;; > Engine > info currmove e2e4 currmovenumber 1
//    ;; > Engine > info time 10000 nodes 5599214 hashfull 390 nps 559921
//    ;; > score cp 45 pv e2e4 b8c6 g1f3 g8f6 e4e5 f6g4 d2d4 e7e6 h2h3 f8b4
//    ;; > Engine > bestmove e2e4 ponder b8c6
//    ;; > (E2 E4 EMPTY))...";
//    AddHelpDict("engine @go-movetime", temp);
//    AddHelpDict("engine @go-timelimit", temp);
//    AddHelpDict("engine @go-depth", temp);
//    AddHelpDict("engine @go-nodes", temp);
//
//    temp =
//R"...(### Hash and threads ###
//
//* `@set-hash-size <Size : Number>`
//    + Sets size of Hash Table(Transposition Table)
//      and returns the previous size.
//    + The unit of size is "byte".
//* `@set-threads <Number of threads : Number>`
//    + Sets `<Number of threads>` and returns the previous number.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    ;; Set size of Hash Table to 128 MB.
//    (my-engine '@input-uci-command "setoption name hash value 128")
//    
//    (display (my-engine '@set-hash-size (* 256 1024 1024)))
//    ;; Set size of Hash Table to 256 MB and return 128 * 1024 * 1024 bytes.
//    ;; Output
//    ;; > 1.34218e+08
//    
//    ;; Set the number of threads to 3.
//    (my-engine '@input-uci-command "setoption name threads value 3")
//    
//    (display (my-engine '@set-threads 4))
//    ;; Set the number of threads to 4 and return 3.
//    ;; Output
//    ;; > 3)...";
//    AddHelpDict("engine @set-hash-size", temp);
//    AddHelpDict("engine @set-threads", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Material ###
//
//* `@material [<New materal : List>]`
//    + Returns List of material.
//        - 1st : Empty (It is always 0)
//        - 2nd : Pawn
//        - 3rd : Knight
//        - 4th : Bishop
//        - 5th : Rook
//        - 6th : Queen
//        - 7th : King
//    + If you specify `<New materal>`, the material is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@material (list 111 222 333 444 555 666 777)))
//    ;; Output
//    ;; > (0 100 400 400 600 1200 1e+06)
//    
//    (display (my-engine '@material))
//    ;; Output
//    ;; > (0 222 333 444 555 666 777))...";
//    AddHelpDict("engine @material", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Quiescence Search ###
//
//* `@enable-quiesce-search [<New setting : Boolean>]`
//    + Returns whether Quiescence Search is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Quiescence Search is set to be enabled.
//      Otherwise, it is set to be disabled.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-quiesce-search #f))
//    ;; Output
//    :: > #t
//    
//    (display (my-engine '@enable-quiesce-search))
//    ;; Output
//    :: > #f)...";
//    AddHelpDict("engine @enable-quiesce-search", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Repetition Check ###
//
//* `@enable-repetition-check [<New setting : Boolean>]`
//    + Returns whether Repetition Check is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Repetition Check is set to be enabled.
//      Otherwise, it is set to be disabled.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-repetition-check #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-repetition-check))
//    ;; Output
//    ;; > #f)...";
//    AddHelpDict("engine @enable-repetition-check", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Check Extension ###
//
//* `@enable-check-extension [<New setting : Boolean>]`
//    + Returns whether Check Extension is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Check Extension is set to be enabled.
//      Otherwise, it is set to be disabled.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-check-extension #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-check-extension))
//    ;; Output
//    ;; > #f)...";
//    AddHelpDict("engine @enable-check-extension", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - YBWC ###
//
//* `@ybwc-limit-depth [<New depth : Number>]`
//    + If remaining depth is less than this parameter, YBWC is invalidated.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//
//* `@ybwc-invalid-moves [<New number of moves : Number>]`
//    + YBWC searches with one thread during this parameter of candidate moves.
//    + Return this parameter.
//    + If you specify `<New number of moves>`, this parameter is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@ybwc-limit-depth 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@ybwc-limit-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@ybwc-invalid-moves 10))
//    ;; Output
//    ;; > 3
//    
//    (display (my-engine '@ybwc-invalid-moves))
//    ;; Output
//    ;; > 10)...";
//    AddHelpDict("engine @ybwc-limit-depth", temp);
//    AddHelpDict("engine @ybwc-invalid-moves", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Aspiration Windows ###
//
//* `@enable-aspiration-windows [<New setting : Boolean>]`
//    + Returns whether Aspiration Windows is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Aspiration Windows is set to be enabled.
//      Otherwise, it is set to be disabled.
//* `@aspiration-windows-limit-depth [<New depth : Number>]`
//    + If remaining depth is less than this parameter at the root node,
//      Aspiration Windows is invalidated.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//* `@aspiration-windows-delta [<New delta : Number>]`
//    + Return Delta.
//    + If you specify `<New delta>`, Delta is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-aspiration-windows #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-aspiration-windows))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@aspiration-windows-limit-depth 10))
//    ;; Output
//    ;; > 5
//    
//    (display (my-engine '@aspiration-windows-limit-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@aspiration-windows-delta 20))
//    ;; Output
//    ;; > 15
//    
//    (display (my-engine '@aspiration-windows-delta))
//    ;; Output
//    ;; > 20)...";
//    AddHelpDict("engine @enable-aspiration-windows", temp);
//    AddHelpDict("engine @aspiration-windows-limit-depth", temp);
//    AddHelpDict("engine @aspiration-windows-delta", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Aspiration Windows ###
//
//* `@enable-see [<New setting : Boolean>]`
//    + Returns whether SEE is enabled or not.
//    + If you specify #t to `<New setting>`,
//      SEE is set to be enabled. Otherwise, it is set to be disabled.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-see #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-see))
//    ;; Output
//    ;; > #f)...";
//    AddHelpDict("engine @enable-see", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - History Heuristics ###
//
//* `@enable-history [<New setting : Boolean>]`
//    + Returns whether History Heuristics is enabled or not.
//    + If you specify #t to `<New setting>`,
//      History Heuristics is set to be enabled.
//      Otherwise, it is set to be disabled.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-history #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-history))
//    ;; Output
//    ;; > #f)...";
//    AddHelpDict("engine @enable-history", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Killer Move Heuristics ###
//
//* `@enable-killer [<New setting : Boolean>]`
//    + Returns whether Killer Move Heuristics is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Killer Move Hiuristics is set to be enabled.
//      Otherwise, it is set to be disabled.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-killer #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-killer))
//    ;; Output
//    ;; > #f)...";
//    AddHelpDict("engine @enable-killer", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Hash Table ###
//
//* `@enable-hash-table [<New setting : Boolean>]`
//    + Returns whether Transposition Table is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Transposition Table is set to be enabled.
//      Otherwise, it is set to be disabled.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-hash-table #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-hash-table))
//    ;; Output
//    ;; > #t)...";
//    AddHelpDict("engine @enable-hash-table", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Internal Iterative Deepening ###
//
//* `@enable-iid [<New setting : Boolean>]`
//    + Returns whether Internal Iterative Deepening is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Internal Iterative Deepening is set to be enabled.
//      Otherwise, it is set to be disabled.
//* `@iid-limit-depth [<New depth : Number>]`
//    + If remaining depth is less than this parameter,
//      Internal Iterative Deepening is invalidated.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//* `@iid-search-depth [<New depth : Number>]`
//    + Internal Iterative Deepening searches until depth of this parameter.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-iid #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-iid))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@iid-limit-depth 10))
//    ;; Output
//    ;; > 5
//    
//    (display (my-engine '@iid-limit-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@iid-search-depth 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@iid-search-depth))
//    ;; Output
//    ;; > 10)...";
//    AddHelpDict("engine @enable-iid", temp);
//    AddHelpDict("engine @iid-limit-depth", temp);
//    AddHelpDict("engine @iid-search-depth", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Null Move Reduction ###
//
//* `@enable-nmr [<New setting : Boolean>]`
//    + Returns whether Null Move Reduction is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Null Move Reduction is set to be enabled.
//      Otherwise, it is set to be disabled.
//* `@nmr-limit-depth [<New depth : Number>]`
//    + If remaining depth is less than this parameter,
//      Null Move Reduction is invalidated.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//* `@nmr-search-reduction [<New reduction : Number>]`
//    + When searching shallowly, the depth is the actual depth
//      minus this parameter.
//    + Return this parameter.
//    + If you specify `<New reduction>`, this parameter is updated.
//* `@nmr-reduction [<New reduction : Number>]`
//    + If the score is greater than or equals to Beta,
//      the remaining depth is reduced by this parameter.
//    + Return this parameter.
//    + If you specify `<New reduction>`, this parameter is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-nmr #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-nmr))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@nmr-limit-depth 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@nmr-limit-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@nmr-search-reduction 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@nmr-search-reduction))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@nmr-reduction 10))
//    ;; Output
//    ;; > 3
//    
//    (display (my-engine '@nmr-reduction))
//    ;; Output
//    ;; > 10)...";
//    AddHelpDict("engine @enable-nmr", temp);
//    AddHelpDict("engine @nmr-limit-depth", temp);
//    AddHelpDict("engine @nmr-search-reduction", temp);
//    AddHelpDict("engine @nmr-reduction", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - ProbCut ###
//
//* `@enable-probcut [<New setting : Boolean>]`
//    + Returns whether ProbCut is enabled or not.
//    + If you specify #t to `<New setting>`, ProbCut is set to be enabled.
//      Otherwise, it is set to be disabled.
//* `@probcut-limit-depth [<New depth : Number>]`
//    + If remaining depth is less than this parameter, ProbCut is invalidated.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//* `@probcut-margin [<New margin : Number>]`
//    + When Zero Window Search,
//      ProbCut uses the current Beta plus this parameter as temporary Beta.
//* `@probcut-search-reduction [<New reduction : Number>]`
//    + When Zero Window Search, the depth is the actual depth
//      minus this parameter.
//    + Return this parameter.
//    + If you specify `<New reduction>`, this parameter is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-probcut #t))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@enable-probcut))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@probcut-limit-depth 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@probcut-limit-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@probcut-margin 1200))
//    ;; Output
//    ;; > 400
//    
//    (display (my-engine '@probcut-margin))
//    ;; Output
//    ;; > 1200
//    
//    (display (my-engine '@probcut-search-reduction 10))
//    ;; Output
//    ;; > 3
//    
//    (display (my-engine '@probcut-search-reduction))
//    ;; Output
//    ;; > 10)...";
//    AddHelpDict("engine @enable-probcut", temp);
//    AddHelpDict("engine @probcut-limit-depth", temp);
//    AddHelpDict("engine @probcut-margin", temp);
//    AddHelpDict("engine @probcut-search-reduction", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - History Pruning ###
//
//* `@enable-history-pruning [<New setting : Boolean>]`
//    + Returns whether History Pruning is enabled or not.
//    + If you specify #t to `<New setting>`,
//      History Pruning is set to be enabled.
//      Otherwise, it is set to be disabled.
//* `@history-pruning-limit-depth [<New depth : Number>]`
//    + If remaining depth is less than this parameter,
//      History Pruning is invalidated.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//* `@history-pruning-move-threshold [<New threshold : Number>]`
//    + If the number of the candidate move is less
//      than the number of all moves times this parameter,
//      History Pruning is invalidated.
//    + This parameter is between 0.0 and 1.0.
//    + Return this parameter.
//    + If you specify `<New threshold>`, this parameter is updated.
//* `@history-pruning-invalid-moves [<New number of moves : Number>]`
//    + If the number of the candidate moves is less than this parameter,
//      History Pruning is invalidated.
//    + This parameter is given priority to `@history-pruning-move-threshold`.
//    + Return this parameter.
//    + If you specify `<New number of moves>`, this parameter is updated.
//* `@history-pruning-threshold [<New threshold : Number>]`
//    + If the history value of the current candidate move is lower
//      than the max history value times this parameter,
//      History Pruning temporarily reduces the remaining depth.
//    + Return this parameter.
//    + If you specify `<New threshold>`, this parameter is updated.
//* `@history-pruning-reduction [<New reduction : Number>]`
//    + When History Pruning reduces the remaining depth,
//      a new depth is the current depth minus this parameter.
//    + Return this parameter.
//    + If you specify `<New reduction>`, this parameter is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-history-pruning #t))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@enable-history-pruning))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@history-pruning-limit-depth 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@history-pruning-limit-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@history-pruning-move-threshold 0.8))
//    ;; Output
//    ;; > 0.6
//    
//    (display (my-engine '@history-pruning-move-threshold))
//    ;; Output
//    ;; > 0.8
//    
//    (display (my-engine '@history-pruning-invalid-moves 20))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@history-pruning-invalid-moves))
//    ;; Output
//    ;; > 20
//    
//    (display (my-engine '@history-pruning-threshold 0.8))
//    ;; Output
//    ;; > 0.5
//    
//    (display (my-engine '@history-pruning-threshold))
//    ;; Output
//    ;; > 0.8
//    
//    (display (my-engine '@history-pruning-reduction 10))
//    ;; Output
//    ;; > 1
//    
//    (display (my-engine '@history-pruning-reduction))
//    ;; Output
//    ;; > 10)...";
//    AddHelpDict("engine @enable-history-pruning", temp);
//    AddHelpDict("engine @history-pruning-limit-depth", temp);
//    AddHelpDict("engine @history-pruning-move-threshold", temp);
//    AddHelpDict("engine @history-pruning-invalid-moves", temp);
//    AddHelpDict("engine @history-pruning-threshold", temp);
//    AddHelpDict("engine @history-pruning-reduction", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Late Move Reduction ###
//
//* `@enable-lmr [<New setting : Boolean>]`
//    + Returns whether Late Move Reduction is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Late Move Reduction is set to be enabled.
//      Otherwise, it is set to be disabled.
//* `@lmr-limit-depth [<New depth : Number>]`
//    + If remaining depth is less than this parameter,
//      Late Move Reduction is invalidated.
//    + Return this parameter.
//    + If you specify `<New depth>`, this parameter is updated.
//* `@lmr-move-threshold [<New threshold : Number>]`
//    + If the number of the candidate move is less
//      than the number of all moves times this parameter,
//      Late Move Reduction is invalidated.
//    + This parameter is between 0.0 and 1.0.
//    + Return this parameter.
//    + If you specify `<New threshold>`, this parameter is updated.
//* `@lmr-invalid-moves [<New number of moves : Number>]`
//    + If the number of the candidate moves is less than this parameter,
//      Late Move Reduction is invalidated.
//    + This parameter is given priority to `@lmr-move-threshold`.
//    + Return this parameter.
//    + If you specify `<New number of moves>`, this parameter is updated.
//* `@lmr-search-reduction [<New reduction : Number>]`
//    + When searching shallowly, the depth is the actual depth
//      minus this parameter.
//    + Return this parameter.
//    + If you specify `<New reduction>`, this parameter is updated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-lmr #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-lmr))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@lmr-limit-depth 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@lmr-limit-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@lmr-move-threshold 0.8))
//    ;; Output
//    ;; > 0.3
//    
//    (display (my-engine '@lmr-move-threshold))
//    ;; Output
//    ;; > 0.8
//    
//    (display (my-engine '@lmr-invalid-moves 10))
//    ;; Output
//    ;; > 4
//    
//    (display (my-engine '@lmr-invalid-moves))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@lmr-search-reduction 5))
//    ;; Output
//    ;; > 1
//    
//    (display (my-engine '@lmr-search-reduction))
//    ;; Output
//    ;; > 5)...";
//    AddHelpDict("engine @enable-lmr", temp);
//    AddHelpDict("engine @lmr-move-threshold", temp);
//    AddHelpDict("engine @lmr-invalid-moves", temp);
//    AddHelpDict("engine @lmr-search-reduction", temp);
//
//    temp =
//R"...(### Customizing Search Algorithm - Futility Pruning ###
//
//* `@enable-futility-pruning [<New setting : Boolean>]`
//    + Returns whether Futility Pruning is enabled or not.
//    + If you specify #t to `<New setting>`,
//      Futility Pruning is set to be enabled.
//      Otherwise, it is set to be disabled.
//* `@futility-pruning-depth [<New depth : Number>]`
//    + If the remaining depth is less than or equals to this parameter,
//      Futility Pruning is executed.
//    + Return this parameter.
//    + If you specify `<New reduction>`, this parameter is updated.
//* `@futility-pruning-margin [<New margin : Number>]`
//    + If the material after the move is lower than Alpha minus this parameter,
//      the move is not evaluated.
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@enable-futility-pruning #f))
//    ;; Output
//    ;; > #t
//    
//    (display (my-engine '@enable-futility-pruning))
//    ;; Output
//    ;; > #f
//    
//    (display (my-engine '@futility-pruning-depth 10))
//    ;; Output
//    ;; > 3
//    
//    (display (my-engine '@futility-pruning-depth))
//    ;; Output
//    ;; > 10
//    
//    (display (my-engine '@futility-pruning-margin 1200))
//    ;; Output
//    ;; > 400
//    
//    (display (my-engine '@futility-pruning-margin))
//    ;; Output
//    ;; > 1200)...";
//    AddHelpDict("engine @enable-futility-pruning", temp);
//    AddHelpDict("engine @futility-pruning-depth", temp);
//    AddHelpDict("engine @futility-pruning-margin", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Piece Square Table ###
//
//Returns Piece Square Table for each piece type.  
//If you specify `<New table>`, this parameter is updated.
//
//`score = weight * value_table[square]`
//
//* `@pawn-square-table-opening [<New table : List>]`
//* `@knight-square-table-opening [<New table : List>]`
//* `@bishop-square-table-opening [<New table : List>]`
//* `@rook-square-table-opening [<New table : List>]`
//* `@queen-square-table-opening [<New table : List>]`
//* `@king-square-table-opening [<New table : List>]`
//* `@pawn-square-table-ending [<New table : List>]`
//* `@knight-square-table-ending [<New table : List>]`
//* `@bishop-square-table-ending [<New table : List>]`
//* `@rook-square-table-ending [<New table : List>]`
//* `@queen-square-table-ending [<New table : List>]`
//* `@king-square-table-ending [<New table : List>]`
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@pawn-square-table-opening
//    (list
//      1 2 3 4 5 6 7 8
//      11 22 33 44 55 66 77 88
//      111 222 333 444 555 666 777 888
//      1111 2222 3333 4444 5555 6666 7777 8888
//      11111 22222 33333 44444 55555 66666 77777 88888
//      111111 222222 333333 444444 555555 666666 777777 888888
//      1111111 2222222 3333333 4444444 5555555 6666666 7777777 8888888
//      11111111 22222222 33333333 44444444 55555555 66666666 77777777 88888888
//    )))
//    ;; Output
//    ;; > (0 0 0 0 0 0 0 0
//    ;; > 0 0 0 0 0 0 0 0
//    ;; > 5 10 15 20 20 15 10 5
//    ;; > 10 20 30 40 40 30 20 10
//    ;; > 15 30 45 60 60 45 30 15
//    ;; > 20 40 60 80 80 60 40 20
//    ;; > 25 50 75 100 100 75 50 25
//    ;; > 30 60 90 120 120 90 60 30)
//    
//    (display (my-engine '@pawn-square-table-opening))
//    ;; Output
//    ;; > (1 2 3 4 5 6 7 8
//    ;; > 11 22 33 44 55 66 77 88
//    ;; > 111 222 333 444 555 666 777 888
//    ;; > 1111 2222 3333 4444 5555 6666 7777 8888
//    ;; > 11111 22222 33333 44444 55555 66666 77777 88888
//    ;; > 111111 222222 333333 444444 555555 666666 777777 888888
//    ;; > 1111111 2222222 3333333 4444444 5555555 6666666 7777777 8888888
//    ;; > 11111111 22222222 33333333 44444444 55555555 66666666 77777777
//    ;; > 88888888))...";
//    AddHelpDict("engine @pawn-square-table-opening", temp);
//    AddHelpDict("engine @knight-square-table-opening", temp);
//    AddHelpDict("engine @bishop-square-table-opening", temp);
//    AddHelpDict("engine @rook-square-table-opening", temp);
//    AddHelpDict("engine @queen-square-table-opening", temp);
//    AddHelpDict("engine @king-square-table-opening", temp);
//    AddHelpDict("engine @pawn-square-table-ending", temp);
//    AddHelpDict("engine @knight-square-table-ending", temp);
//    AddHelpDict("engine @bishop-square-table-ending", temp);
//    AddHelpDict("engine @rook-square-table-ending", temp);
//    AddHelpDict("engine @queen-square-table-ending", temp);
//    AddHelpDict("engine @king-square-table-ending", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Attack ###
//
//Returns List composed of 7 values of attacking score.  
//1st: Not used. This is always 0. (for EMPTY)  
//2nd: Attacking Pawn.  
//3rd: Attacking Knight.  
//4th: Attacking Bishop.  
//5th: Attacking Rook.  
//6th: Attacking Queen.  
//7th: Attacking King.
//
//If you specify `<New table>`, this parameter is updated.
//
//`score = weight * value_table[attacking_piece][attacked_piece]`
//
//* `@pawn-attack-table [<New table : List>]`
//* `@knight-attack-table [<New table : List>]`
//* `@bishop-attack-table [<New table : List>]`
//* `@rook-attack-table [<New table : List>]`
//* `@queen-attack-table [<New table : List>]`
//* `@king-attack-table [<New table : List>]`
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@pawn-attack-table (list 1 2 3 4 5 6 7)))
//    ;; Output
//    ;; > (0 10 12 14 16 18 20)
//    
//    (display (my-engine '@pawn-attack-table))
//    ;; Output
//    ;; > (0 2 3 4 5 6 7)
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-attack [<New weight : List>]`
//* `@weight-knight-attack [<New weight : List>]`
//* `@weight-bishop-attack [<New weight : List>]`
//* `@weight-rook-attack [<New weight : List>]`
//* `@weight-queen-attack [<New weight : List>]`
//* `@weight-king-attack [<New weight : List>]`)...";
//    AddHelpDict("engine @pawn-attack-table", temp);
//    AddHelpDict("engine @knight-attack-table", temp);
//    AddHelpDict("engine @bishop-attack-table", temp);
//    AddHelpDict("engine @rook-attack-table", temp);
//    AddHelpDict("engine @queen-attack-table", temp);
//    AddHelpDict("engine @king-attack-table", temp);
//    AddHelpDict("engine @weight-pawn-attack", temp);
//    AddHelpDict("engine @weight-knight-attack", temp);
//    AddHelpDict("engine @weight-bishop-attack", temp);
//    AddHelpDict("engine @weight-rook-attack", temp);
//    AddHelpDict("engine @weight-queen-attack", temp);
//    AddHelpDict("engine @weight-king-attack", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Defense ###
//
//Returns List composed of 7 values of defense score.  
//1st: Not used. This is always 0. (for EMPTY)  
//2nd: Protecting Pawn.  
//3rd: Protecting Knight.  
//4th: Protecting Bishop.  
//5th: Protecting Rook.  
//6th: Protecting Queen.  
//7th: Protecting King.
//
//If you specify `<New table>`, this parameter is updated.
//
//`score = weight * value_table[defensing_piece][defensed_piece]`
//
//* `@pawn-defense-table [<New table : List>]`
//* `@knight-defense-table [<New table : List>]`
//* `@bishop-defense-table [<New table : List>]`
//* `@rook-defense-table [<New table : List>]`
//* `@queen-defense-table [<New table : List>]`
//* `@king-defense-table [<New table : List>]`
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@pawn-defense-table (list 1 2 3 4 5 6 7)))
//    ;; Output
//    ;; > (0 10 0 0 0 0 0)
//    
//    (display (my-engine '@pawn-defense-table))
//    ;; Output
//    ;; > (0 2 3 4 5 6 7)
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-defense [<New weight : List>]`
//* `@weight-knight-defense [<New weight : List>]`
//* `@weight-bishop-defense [<New weight : List>]`
//* `@weight-rook-defense [<New weight : List>]`
//* `@weight-queen-defense [<New weight : List>]`
//* `@weight-king-defense [<New weight : List>]`)...";
//    AddHelpDict("engine @pawn-defense-table", temp);
//    AddHelpDict("engine @knight-defense-table", temp);
//    AddHelpDict("engine @bishop-defense-table", temp);
//    AddHelpDict("engine @rook-defense-table", temp);
//    AddHelpDict("engine @queen-defense-table", temp);
//    AddHelpDict("engine @king-defense-table", temp);
//    AddHelpDict("engine @weight-pawn-defense", temp);
//    AddHelpDict("engine @weight-knight-defense", temp);
//    AddHelpDict("engine @weight-bishop-defense", temp);
//    AddHelpDict("engine @weight-rook-defense", temp);
//    AddHelpDict("engine @weight-queen-defense", temp);
//    AddHelpDict("engine @weight-king-defense", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Pin ###
//
//Returns List of 7 Lists of 7 values.
//
//1st list : Futile list. (For EMPTY)  
//2nd list : A target piece is Pawn.  
//3rd list : A target piece is Knight.  
//4th list : A target piece is Bishop.  
//5th list : A target piece is Rook.  
//6th list : A target piece is Queen.  
//7th list : A target piece is King.
//
//1st value of each list : Futile value.
//2nd value of each list : A piece over the target is Pawn.
//3rd value of each list : A piece over the target is Knight.
//4th value of each list : A piece over the target is Bishop.
//5th value of each list : A piece over the target is Rook.
//6th value of each list : A piece over the target is Queen.
//7th value of each list : A piece over the target is King.
//
//`score = weight * value_table[pinning_piece][target][over_the_target]`
//
//* `@bishop-pin-table [<New value table : List>]`
//* `@rook-pin-table [<New value table : List>]`
//* `@queen-pin-table [<New value table : List>]`
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (display (my-engine '@bishop-pin-table
//      (list (list 1 2 3 4 5 6 7)
//            (list 8 9 10 11 12 13 14)
//            (list 15 16 17 18 19 20 21)
//            (list 22 23 24 25 26 27 28)
//            (list 29 30 31 32 33 34 35)
//            (list 36 37 38 39 40 41 42)
//            (list 43 44 45 46 47 48 49))))
//    ;; Output
//    ;; > ((0 0 0 0 0 0 0)
//    ;; > (0 0 0 0 5 5 5)
//    ;; > (0 0 0 0 10 10 10)
//    ;; > (0 0 0 0 0 0 0)
//    ;; > (0 0 0 0 20 30 40)
//    ;; > (0 0 0 0 30 40 50)
//    ;; > (0 0 0 0 40 50 0))
//    
//    (display (my-engine '@bishop-pin-table))
//    ;; Output
//    ;; >  ((0 0 0 0 0 0 0)
//    ;; > (0 9 10 11 12 13 14)
//    ;; > (0 16 17 18 19 20 21)
//    ;; > (0 23 24 25 26 27 28)
//    ;; > (0 30 31 32 33 34 35)
//    ;; > (0 37 38 39 40 41 42)
//    ;; > (0 44 45 46 47 48 49))
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-bishop-pin [<New weight : List>]`
//* `@weight-rook-pin [<New weight : List>]`
//* `@weight-queen-pin [<New weight : List>]`)...";
//    AddHelpDict("engine @bishop-pin-table", temp);
//    AddHelpDict("engine @rook-pin-table", temp);
//    AddHelpDict("engine @queen-pin-table", temp);
//    AddHelpDict("engine @weight-bishop-pin", temp);
//    AddHelpDict("engine @weight-rook-pin", temp);
//    AddHelpDict("engine @weight-queen-pin", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Pawn Shield ###
//
//If King on f1(f8) f2(f7) g1(g8) g2(g7) h1(h8) h2(h7),
//then Pawn Shield is pawns on f g h files.  
//If King on a1(a8) a2(a7) b1(b8) b2(b7) c1(c8) c2(c7),
//then Pawn Shield is pawns on a b c files.
//
//`score = weight * value_table[square]`
//
//* `@pawn-shield-table [<New table : List>]`
//    + Returns Piece Square Table for Pawn Shield
//      as List composed of 64 numbers.
//    + If you specify `<New table>`, this parameter is updated.
//
//
//<h6> Example </h6>
//
//    (define my-engine (gen-engine))
//    
//    (define table
//      (list 1 2 3 4 5 6 7 8
//            9 10 11 12 13 14 15 16
//            17 18 19 20 21 22 23 24
//            25 26 27 28 29 30 31 32
//            33 34 35 36 37 38 39 40
//            41 42 43 44 45 46 47 48
//            49 50 51 52 53 54 55 56
//            57 58 59 60 61 62 63 64))
//    
//    (display (my-engine '@pawn-shield-table table))
//    ;; Output
//    ;; > (0 0 0 0 0 0 0 0
//    ;; > 30 30 30 30 30 30 30 30
//    ;; > 0 0 0 0 0 0 0 0
//    ;; > -30 -30 -30 -30 -30 -30 -30 -30
//    ;; > -60 -60 -60 -60 -60 -60 -60 -60
//    ;; > -90 -90 -90 -90 -90 -90 -90 -90
//    ;; > -60 -60 -60 -60 -60 -60 -60 -60
//    ;; > -30 -30 -30 -30 -30 -30 -30 -30)
//    
//    (display (my-engine '@pawn-shield-table))
//    ;; Output
//    ;; > (1 2 3 4 5 6 7 8
//    ;; > 9 10 11 12 13 14 15 16
//    ;; > 17 18 19 20 21 22 23 24
//    ;; > 25 26 27 28 29 30 31 32
//    ;; > 33 34 35 36 37 38 39 40
//    ;; > 41 42 43 44 45 46 47 48
//    ;; > 49 50 51 52 53 54 55 56
//    ;; > 57 58 59 60 61 62 63 64)
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-shield [<New weight : List>]`)...";
//    AddHelpDict("engine @pawn-shield-table", temp);
//    AddHelpDict("engine @weight-pawn-shield", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Mobility ###
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`score = weight * num_of_squares_that_piece_can_move_to`
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-mobility [<New weight : List>]`
//* `@weight-knight-mobility [<New weight : List>]`
//* `@weight-bishop-mobility [<New weight : List>]`
//* `@weight-rook-mobility [<New weight : List>]`
//* `@weight-queen-mobility [<New weight : List>]`
//* `@weight-king-mobility [<New weight : List>]`)...";
//    AddHelpDict("engine @weight-pawn-mobility", temp);
//    AddHelpDict("engine @weight-knight-mobility", temp);
//    AddHelpDict("engine @weight-bishop-mobility", temp);
//    AddHelpDict("engine @weight-rook-mobility", temp);
//    AddHelpDict("engine @weight-queen-mobility", temp);
//    AddHelpDict("engine @weight-king-mobility", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Controlling Center ###
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//Center square are c3 c4 c5 c6 d3 d4 d5 d6 e3 e4 e5 e6 f3 f4 f5 f6.
//
//`score = weight * num_of_center_square_that_piece_attacks`
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-center-control [<New weight : List>]`
//* `@weight-knight-center-control [<New weight : List>]`
//* `@weight-bishop-center-control [<New weight : List>]`
//* `@weight-rook-center-control [<New weight : List>]`
//* `@weight-queen-center-control [<New weight : List>]`
//* `@weight-king-center-control [<New weight : List>]`)...";
//    AddHelpDict("engine @weight-pawn-center-control", temp);
//    AddHelpDict("engine @weight-knight-center-control", temp);
//    AddHelpDict("engine @weight-bishop-center-control", temp);
//    AddHelpDict("engine @weight-rook-center-control", temp);
//    AddHelpDict("engine @weight-queen-center-control", temp);
//    AddHelpDict("engine @weight-king-center-control", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Controlling Sweet Center ###
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//Sweet Center square are d3 d4 d5 d6 e3 e4 e5 e6.
//
//`score = weight * num_of_sweet_center_square_that_piece_attacks`
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-sweet-center-control [<New weight : List>]`
//* `@weight-knight-sweet-center-control [<New weight : List>]`
//* `@weight-bishop-sweet-center-control [<New weight : List>]`
//* `@weight-rook-sweet-center-control [<New weight : List>]`
//* `@weight-queen-sweet-center-control [<New weight : List>]`
//* `@weight-king-sweet-center-control [<New weight : List>]`)...";
//    AddHelpDict("engine @weight-pawn-sweet-center-control", temp);
//    AddHelpDict("engine @weight-knight-sweet-center-control", temp);
//    AddHelpDict("engine @weight-bishop-sweet-center-control", temp);
//    AddHelpDict("engine @weight-rook-sweet-center-control", temp);
//    AddHelpDict("engine @weight-queen-sweet-center-control", temp);
//    AddHelpDict("engine @weight-king-sweet-center-control", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Development ###
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`score = weight * num_of_minor_pieces_on_starting_position`
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-development [<New weight : List>]`
//* `@weight-knight-development [<New weight : List>]`
//* `@weight-bishop-development [<New weight : List>]`
//* `@weight-rook-development [<New weight : List>]`
//* `@weight-queen-development [<New weight : List>]`
//* `@weight-king-development [<New weight : List>]`)...";
//    AddHelpDict("engine @weight-pawn-development", temp);
//    AddHelpDict("engine @weight-knight-development", temp);
//    AddHelpDict("engine @weight-bishop-development", temp);
//    AddHelpDict("engine @weight-rook-development", temp);
//    AddHelpDict("engine @weight-queen-development", temp);
//    AddHelpDict("engine @weight-king-development", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Attack around Enemy King ###
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`score = weight * num_of_squares_around_enemy_king_attacked_by_pieces.`
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pawn-attack-around-king [<New weight : List>]`
//* `@weight-knight-attack-around-king [<New weight : List>]`
//* `@weight-bishop-attack-around-king [<New weight : List>]`
//* `@weight-rook-attack-around-king [<New weight : List>]`
//* `@weight-queen-attack-around-king [<New weight : List>]`
//* `@weight-king-attack-around-king [<New weight : List>]`)...";
//    AddHelpDict("engine @weight-pawn-attack-around-king", temp);
//    AddHelpDict("engine @weight-knight-attack-around-king", temp);
//    AddHelpDict("engine @weight-bishop-attack-around-king", temp);
//    AddHelpDict("engine @weight-rook-attack-around-king", temp);
//    AddHelpDict("engine @weight-queen-attack-around-king", temp);
//    AddHelpDict("engine @weight-king-attack-around-king", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Pawn Structure ###
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-pass-pawn [<New weight : List>]`
//    + `score = weight * num_of_pass_pawns`
//* `@weight-protected-pass-pawn [<New weight : List>]`
//    + `score = weight * num_of_protected_pass_pawns`
//* `@weight-double-pawn [<New weight : List>]`
//    + `score = weight * num_of_doubled_pawns`
//* `@weight-iso-pawn [<New weight : List>]`
//    + `score = weight * num_of_isorated_pawns`)...";
//    AddHelpDict("engine @weight-pass-pawn", temp);
//    AddHelpDict("engine @weight-protected-pass-pawn", temp);
//    AddHelpDict("engine @weight-double-pawn", temp);
//    AddHelpDict("engine @weight-iso-pawn", temp);
//
//    temp =
//R"...(### Customizing Evaluation Function - Piece ###
//
//Returns opening weight and ending weight.  
//If you specify `<New weight>`, this parameter is updated.
//
//`num_of_pieces := pieces on the current board except Kings`  
//`weight =
//(((opening_weight - ending_weight) / 30) * num_of_pieces) + ending_weight`
//
//* `@weight-bishop-pair [<New weight : List>]`
//    + `score = weight * if_bishop_pair_exists_then_1_else_0`
//* `@weight-bad-bishop [<New weight : List>]`
//    + `score = weight * num_of_pawns_on_same_colored_square_as_bishop_on`
//
//* `@weight-rook-pair [<New weight : List>]`
//* `@weight-rook-semiopen-fyle [<New weight : List>]`
//* `@weight-rook-open-fyle [<New weight : List>]`
//* `@weight-early-queen-starting [<New weight : List>]`
//* `@weight-weak-square [<New weight : List>]`
//* `@weight-castling [<New weight : List>]`
//* `@weight-abandoned-castling [<New weight : List>]`)...";
//    AddHelpDict("engine @weight-bishop-pair", temp);
//    AddHelpDict("engine @weight-bad-bishop", temp);
//    AddHelpDict("engine @weight-rook-pair", temp);
//    AddHelpDict("engine @weight-rook-semiopen-fyle", temp);
//    AddHelpDict("engine @weight-rook-open-fyle", temp);
//    AddHelpDict("engine @weight-early-queen-starting", temp);
//    AddHelpDict("engine @weight-weak-square", temp);
//    AddHelpDict("engine @weight-castling", temp);
//    AddHelpDict("engine @weight-abandoned-castling", temp);
//
//    temp =
//R"...(### gen-pgn ###
//
//<h6> Usage </h6>
//
//* `(gen-pgn <PGN string : String>)`
//
//<h6> Description </h6>
//
//* Generates and returns PGN object from `<PGN string>`.
//* PGN object is operated by Message Symbol.
//* PGN object has 2 states.
//    + Current game.
//        - This can be changed by `@set-current-game`.
//    + Current move.
//        - This can be changed by `@next-move`, `@prev-move`, `@alt-move`,
//          `@orig-move`, `@rewind-move`.
//
//<h6> Description of Message Symbols </h6>
//
//* `@get-pgn-comments`
//    + Returns Lists of comments about PGN.
//
//* `@get-current-game-comments.`
//    + Returns List of comments about the current game.
//
//* `@get-current-move-comments`
//    + Returns List of comments about the current move.
//
//* `@length`
//    + Returns the number of games that PGN has.
//
//* `@set-current-game <Index : Number>`
//    + Sets a current game into the `<Index>`th game.
//
//* `@get-current-game-headers`
//    + Returns List of Lists composed with headers of the current game.
//        - The format is "`((<Name 1> <value 1>) (<Name 2> <Value 2>)...)`".
//
//* `@current-move`
//    + Returns the current move text.
//
//* `@next-move`
//    + Change the current move into the next move
//      and returns the move text.
//
//* `@prev-move`
//    + Change the current move into the previous move
//      and returns the move text.
//
//* `@alt-move`
//    + Change the current move into the alternative move
//      and returns the move text.
//
//* `@orig-move`
//    + If the current move is an alternative move,
//      then change a current move into the original move
//      and returns the move text.
//
//* `@rewind-move`
//    + Change a current move into the first move
//      and returns the move text.
//
//<h6> Example </h6>
//
//    ;; Open PGN File.
//    (define pgn-file (input-stream "/path/to/pgnfile.pgn"))
//    
//    ;; Reads the file and generates PGN object.
//    (define my-pgn (gen-pgn (pgn-file '@read)))
//    
//    ;; Displays the current game headers.
//    (display (my-pgn '@get-current-game-headers))
//    
//    ;; Output
//    ;; > (("Black" "Hanako Yamada") ("Site" "Japan")
//    ;; > ("White" "Hironori Ishibashi")))...";
//    AddHelpDict("gen-pgn", temp);
//
//    temp =
//R"...(### parse-fen/epd ###
//
//<h6> Usage </h6>
//
//* `(parse-fen/epd <FEN or EPD : String>)`
//
//<h6> Description </h6>
//
//* Parses `<FEN or EPD>` and returns result value.
//    +  A result value is `((<Tag 1 : String> <Object 1>)...)`.
//
//<h6> Example </h6>
//
//    (display (parse-fen/epd
//        "rnbqkbnr/pp2pppp/3p4/2p5/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3"))
//    ;; Output
//    ;; > (("fen castling" (WHITE_SHORT_CASTLING
//    ;; > WHITE_LONG_CASTLING BLACK_SHORT_CASTLING BLACK_LONG_CASTLING))
//    ;; > ("fen clock" 0)
//    ;; > ("fen en_passant" D3)
//    ;; > ("fen ply" 5)
//    ;; > ("fen position" ((WHITE ROOK) (WHITE KNIGHT) (WHITE BISHOP)
//    ;; > (WHITE QUEEN) (WHITE KING) (WHITE BISHOP) (NO_SIDE EMPTY)
//    ;; > (WHITE ROOK) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (WHITE PAWN) (WHITE PAWN) (WHITE PAWN)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (WHITE KNIGHT) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (WHITE PAWN)
//    ;; > (WHITE PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (BLACK PAWN)
//    ;; > (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (BLACK PAWN) (BLACK PAWN) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//    ;; > (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK PAWN) (BLACK ROOK)
//    ;; > (BLACK KNIGHT) (BLACK BISHOP) (BLACK QUEEN) (BLACK KING)
//    ;; > (BLACK BISHOP) (BLACK KNIGHT) (BLACK ROOK)))
//    ;; > ("fen to_move" BLACK)))...";
//    AddHelpDict("parse-fen/epd", temp);
//
//    temp =
//R"...(### to-fen-position ###
//
//<h6> Usage </h6>
//
//* `(to-fen-position <Pieces list : List>)`
//
//<h6> Description </h6>
//
//* Analyses `<Pieces list>` and returns FEN position.
//
//<h6> Example </h6>
//
//    (display (to-fen-position
//        '((WHITE KING) (WHITE KING)(WHITE KING) (WHITE KING)
//        (WHITE QUEEN) (WHITE QUEEN)(WHITE QUEEN) (WHITE QUEEN)
//        (WHITE ROOK) (WHITE ROOK)(WHITE ROOK) (WHITE ROOK)
//        (WHITE BISHOP) (WHITE BISHOP)(WHITE BISHOP) (WHITE BISHOP)
//        (WHITE KNIGHT) (WHITE KNIGHT)(WHITE KNIGHT) (WHITE KNIGHT)
//        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//        (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY) (NO_SIDE EMPTY)
//        (BLACK KNIGHT) (BLACK KNIGHT)(BLACK KNIGHT) (BLACK KNIGHT)
//        (BLACK BISHOP) (BLACK BISHOP)(BLACK BISHOP) (BLACK BISHOP)
//        (BLACK ROOK) (BLACK ROOK)(BLACK ROOK) (BLACK ROOK)
//        (BLACK QUEEN) (BLACK QUEEN)(BLACK QUEEN) (BLACK QUEEN)
//        (BLACK KING) (BLACK KING)(BLACK KING) (BLACK KING))))
//    ;; Output
//    ;; > qqqqkkkk/bbbbrrrr/4nnnn/8/8/NNNN4/RRRRBBBB/KKKKQQQQ)...";
//    AddHelpDict("to-fen-position", temp);
//  }

  // =========== //
  // EngineSuite //
  // =========== //
  // コンストラクタ。
  EngineSuite::EngineSuite() :
  search_params_ptr_(new SearchParams()),
  eval_params_ptr_(new EvalParams()),
  table_ptr_(new TranspositionTable(UCI_DEFAULT_TABLE_SIZE)),
  engine_ptr_(new ChessEngine(*search_params_ptr_, *eval_params_ptr_,
  *table_ptr_)),
  board_ptr_(&(engine_ptr_->board())),
  shell_ptr_(new UCIShell(*engine_ptr_)) {
    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});

    // メッセージシンボル関数の登録。
    SetMessageFunctions();
  }

  // コピーコンストラクタ。
  EngineSuite::EngineSuite(const EngineSuite& suite) :
  search_params_ptr_(new SearchParams(*(suite.search_params_ptr_))),
  eval_params_ptr_(new EvalParams(*(suite.eval_params_ptr_))),
  table_ptr_(new TranspositionTable(suite.table_ptr_->GetSizeBytes())),
  engine_ptr_(new ChessEngine(*search_params_ptr_, *eval_params_ptr_,
  *table_ptr_)),
  board_ptr_(&(engine_ptr_->board())),
  shell_ptr_(new UCIShell(*engine_ptr_)) {
    PositionRecord record(*(suite.engine_ptr_));
    engine_ptr_->LoadRecord(record);

    // 出力リスナー。
    shell_ptr_->AddOutputListener
    ([this](const std::string& message) {this->ListenUCIOutput(message);});

    // メッセージシンボル関数の登録。
    SetMessageFunctions();
  }

  // ムーブコンストラクタ。
  EngineSuite::EngineSuite(EngineSuite&& suite) :
  search_params_ptr_(std::move(suite.search_params_ptr_)),
  eval_params_ptr_(std::move(suite.eval_params_ptr_)),
  table_ptr_(std::move(suite.table_ptr_)),
  engine_ptr_(std::move(suite.engine_ptr_)),
  board_ptr_(&(engine_ptr_->board())),
  shell_ptr_(std::move(suite.shell_ptr_)),
  message_func_map_(std::move(suite.message_func_map_)) {
  }

  // コピー代入演算子。
  EngineSuite& EngineSuite::operator=(const EngineSuite& suite) {
    search_params_ptr_.reset(new SearchParams(*(suite.search_params_ptr_)));
    eval_params_ptr_.reset(new EvalParams(*(suite.eval_params_ptr_)));
    table_ptr_.reset(new TranspositionTable(suite.table_ptr_->GetSizeBytes()));
    engine_ptr_.reset(new ChessEngine(*search_params_ptr_, *eval_params_ptr_,
    *table_ptr_));
    board_ptr_ = &(engine_ptr_->board());
    shell_ptr_.reset(new UCIShell(*engine_ptr_));

    PositionRecord record(*(suite.engine_ptr_));
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
    table_ptr_ = std::move(suite.table_ptr_);
    engine_ptr_ = std::move(suite.engine_ptr_);
    board_ptr_ = &(engine_ptr_->board());
    shell_ptr_ = std::move(suite.shell_ptr_);

    return *this;
  }

  // メッセージシンボル関数を登録する。
  void EngineSuite::SetMessageFunctions() {
    message_func_map_["@get-white-pawn-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, PAWN>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-knight-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, KNIGHT>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-bishop-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, BISHOP>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-rook-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, ROOK>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-queen-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, QUEEN>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-king-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, KING>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-pawn-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, PAWN>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-knight-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, KNIGHT>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-bishop-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, BISHOP>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-rook-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, ROOK>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-queen-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, QUEEN>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-king-position"] =
    [this](const std::string& symbol, LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, KING>(symbol, self, caller, args);
    };

    message_func_map_["@get-piece"] =
    INSERT_MESSAGE_FUNCTION(GetPiece);

    message_func_map_["@get-all-pieces"] =
    INSERT_MESSAGE_FUNCTION(GetAllPieces);

    message_func_map_["@get-to-move"] =
    INSERT_MESSAGE_FUNCTION(GetToMove);

    message_func_map_["@get-castling-rights"] =
    INSERT_MESSAGE_FUNCTION(GetCastlingRights);

    message_func_map_["@get-en-passant-square"] =
    INSERT_MESSAGE_FUNCTION(GetEnPassantSquare);

    message_func_map_["@get-ply"] =
    INSERT_MESSAGE_FUNCTION(GetPly);

    message_func_map_["@get-clock"] =
    INSERT_MESSAGE_FUNCTION(GetClock);

    message_func_map_["@get-white-has-castled"] =
    INSERT_MESSAGE_FUNCTION(GetHasCastled<WHITE>);

    message_func_map_["@get-black-has-castled"] =
    INSERT_MESSAGE_FUNCTION(GetHasCastled<BLACK>);

    message_func_map_["@get-fen"] =
    INSERT_MESSAGE_FUNCTION(GetFEN);

    message_func_map_["@to-string"] =
    INSERT_MESSAGE_FUNCTION(BoardToString);

    message_func_map_["@set-new-game"] =
    INSERT_MESSAGE_FUNCTION(SetNewGame);

    message_func_map_["@set-fen"] =
    INSERT_MESSAGE_FUNCTION(SetFEN);

    message_func_map_["@place-piece"] =
    INSERT_MESSAGE_FUNCTION(PlacePiece);

    message_func_map_["@get-candidate-moves"] =
    INSERT_MESSAGE_FUNCTION(GetCandidateMoves);

    message_func_map_["@set-to-move"] =
    INSERT_MESSAGE_FUNCTION(SetToMove);

    message_func_map_["@set-castling-rights"] =
    INSERT_MESSAGE_FUNCTION(SetCastlingRights);

    message_func_map_["@set-en-passant-square"] =
    INSERT_MESSAGE_FUNCTION(SetEnPassantSquare);

    message_func_map_["@set-ply"] =
    INSERT_MESSAGE_FUNCTION(SetPly);

    message_func_map_["@set-clock"] =
    INSERT_MESSAGE_FUNCTION(SetClock);

    message_func_map_["@correct-position?"] =
    INSERT_MESSAGE_FUNCTION(IsCorrectPosition);

    message_func_map_["@white-checked?"] =
    INSERT_MESSAGE_FUNCTION(IsChecked<WHITE>);

    message_func_map_["@black-checked?"] =
    INSERT_MESSAGE_FUNCTION(IsChecked<BLACK>);

    message_func_map_["@checkmated?"] =
    INSERT_MESSAGE_FUNCTION(IsCheckmated);

    message_func_map_["@stalemated?"] =
    INSERT_MESSAGE_FUNCTION(IsStalemated);

    message_func_map_["@play-move"] =
    INSERT_MESSAGE_FUNCTION(PlayMoveOrNote);

    message_func_map_["@play-note"] =
    INSERT_MESSAGE_FUNCTION(PlayMoveOrNote);

    message_func_map_["@undo-move"] =
    INSERT_MESSAGE_FUNCTION(UndoMove);

    message_func_map_["@move->note"] =
    INSERT_MESSAGE_FUNCTION(MoveToNote);

    message_func_map_["@input-uci-command"] =
    INSERT_MESSAGE_FUNCTION(InputUCICommand);

    message_func_map_["@add-uci-output-listener"] =
    INSERT_MESSAGE_FUNCTION(AddUCIOutputListener);

    message_func_map_["@run"] =
    INSERT_MESSAGE_FUNCTION(RunEngine);

    message_func_map_["@go-movetime"] =
    INSERT_MESSAGE_FUNCTION(GoMoveTime);

    message_func_map_["@go-timelimit"] =
    INSERT_MESSAGE_FUNCTION(GoTimeLimit);

    message_func_map_["@go-depth"] =
    INSERT_MESSAGE_FUNCTION(GoDepth);

    message_func_map_["@go-nodes"] =
    INSERT_MESSAGE_FUNCTION(GoNodes);

    message_func_map_["@set-hash-size"] =
    INSERT_MESSAGE_FUNCTION(SetHashSize);

    message_func_map_["@set-threads"] =
    INSERT_MESSAGE_FUNCTION(SetThreads);

    message_func_map_["@material"] =
    INSERT_MESSAGE_FUNCTION(SetMaterial);

    message_func_map_["@enable-quiesce-search"] =
    INSERT_MESSAGE_FUNCTION(SetEnabelQuiesceSearch);

    message_func_map_["@enable-repetition-check"] =
    INSERT_MESSAGE_FUNCTION(SetEnabelRepetitionCheck);

    message_func_map_["@enable-check-extension"] =
    INSERT_MESSAGE_FUNCTION(SetEnableCheckExtension);

    message_func_map_["@ybwc-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetYBWCLimitDepth);

    message_func_map_["@ybwc-invalid-moves"] =
    INSERT_MESSAGE_FUNCTION(SetYBWCInvalidMoves);

    message_func_map_["@enable-aspiration-windows"] =
    INSERT_MESSAGE_FUNCTION(SetEnableAspirationWindows);

    message_func_map_["@aspiration-windows-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetAspirationWindowsLimitDepth);

    message_func_map_["@aspiration-windows-delta"] =
    INSERT_MESSAGE_FUNCTION(SetAspirationWindowsDelta);

    message_func_map_["@enable-see"] =
    INSERT_MESSAGE_FUNCTION(SetEnableSEE);

    message_func_map_["@enable-history"] =
    INSERT_MESSAGE_FUNCTION(SetEnableHistory);

    message_func_map_["@enable-killer"] =
    INSERT_MESSAGE_FUNCTION(SetEnableKiller);

    message_func_map_["@enable-hash-table"] =
    INSERT_MESSAGE_FUNCTION(SetEnableHashTable);

    message_func_map_["@enable-iid"] =
    INSERT_MESSAGE_FUNCTION(SetEnableIID);

    message_func_map_["@iid-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetIIDLimitDepth);

    message_func_map_["@iid-search-depth"] =
    INSERT_MESSAGE_FUNCTION(SetIIDSearchDepth);

    message_func_map_["@enable-nmr"] =
    INSERT_MESSAGE_FUNCTION(SetEnableNMR);

    message_func_map_["@nmr-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetNMRLimitDepth);

    message_func_map_["@nmr-search-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetNMRSearchReduction);

    message_func_map_["@nmr-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetNMRReduction);

    message_func_map_["@enable-probcut"] =
    INSERT_MESSAGE_FUNCTION(SetEnableProbCut);

    message_func_map_["@probcut-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetProbCutLimitDepth);

    message_func_map_["@probcut-margin"] =
    INSERT_MESSAGE_FUNCTION(SetProbCutMargin);

    message_func_map_["@probcut-search-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetProbCutSearchReduction);

    message_func_map_["@enable-history-pruning"] =
    INSERT_MESSAGE_FUNCTION(SetEnableHistoryPruning);

    message_func_map_["@history-pruning-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningLimitDepth);

    message_func_map_["@history-pruning-move-threshold"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningMoveThreshold);

    message_func_map_["@history-pruning-threshold"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningThreshold);

    message_func_map_["@history-pruning-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningReduction);

    message_func_map_["@enable-lmr"] =
    INSERT_MESSAGE_FUNCTION(SetEnableLMR);

    message_func_map_["@lmr-limit-depth"] =
    INSERT_MESSAGE_FUNCTION(SetLMRLimitDepth);

    message_func_map_["@lmr-move-threshold"] =
    INSERT_MESSAGE_FUNCTION(SetLMRMoveThreshold);

    message_func_map_["@lmr-invalid-moves"] =
    INSERT_MESSAGE_FUNCTION(SetLMRInvalidMoves);

    message_func_map_["@lmr-search-reduction"] =
    INSERT_MESSAGE_FUNCTION(SetLMRSearchReduction);

    message_func_map_["@enable-futility-pruning"] =
    INSERT_MESSAGE_FUNCTION(SetEnableFutilityPruning);

    message_func_map_["@futility-pruning-depth"] =
    INSERT_MESSAGE_FUNCTION(SetFutilityPruningDepth);

    message_func_map_["@futility-pruning-margin"] =
    INSERT_MESSAGE_FUNCTION(SetFutilityPruningMargin);

    message_func_map_["@pawn-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<PAWN>);

    message_func_map_["@knight-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<KNIGHT>);

    message_func_map_["@bishop-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<BISHOP>);

    message_func_map_["@rook-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<ROOK>);

    message_func_map_["@queen-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<QUEEN>);

    message_func_map_["@king-square-table-opening"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableOpening<KING>);

    message_func_map_["@pawn-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<PAWN>);

    message_func_map_["@knight-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<KNIGHT>);

    message_func_map_["@bishop-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<BISHOP>);

    message_func_map_["@rook-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<ROOK>);

    message_func_map_["@queen-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<QUEEN>);

    message_func_map_["@king-square-table-ending"] =
    INSERT_MESSAGE_FUNCTION(SetPieceSquareTableEnding<KING>);

    message_func_map_["@pawn-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<PAWN>);

    message_func_map_["@knight-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<KNIGHT>);

    message_func_map_["@bishop-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<BISHOP>);

    message_func_map_["@rook-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<ROOK>);

    message_func_map_["@queen-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<QUEEN>);

    message_func_map_["@king-attack-table"] =
    INSERT_MESSAGE_FUNCTION(SetAttackTable<KING>);

    message_func_map_["@pawn-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<PAWN>);

    message_func_map_["@knight-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<KNIGHT>);

    message_func_map_["@bishop-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<BISHOP>);

    message_func_map_["@rook-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<ROOK>);

    message_func_map_["@queen-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<QUEEN>);

    message_func_map_["@king-defense-table"] =
    INSERT_MESSAGE_FUNCTION(SetDefenseTable<KING>);

    message_func_map_["@bishop-pin-table"] =
    INSERT_MESSAGE_FUNCTION(SetPinTable<BISHOP>);

    message_func_map_["@rook-pin-table"] =
    INSERT_MESSAGE_FUNCTION(SetPinTable<ROOK>);

    message_func_map_["@queen-pin-table"] =
    INSERT_MESSAGE_FUNCTION(SetPinTable<QUEEN>);

    message_func_map_["@pawn-shield-table"] =
    INSERT_MESSAGE_FUNCTION(SetPawnShieldTable);
  }

  // 関数オブジェクト。
  DEF_LC_FUNCTION(EngineSuite::operator()) {
    // 準備。
    LObject* args_ptr = nullptr;
    Lisp::GetReadyForFunction(args, 1, &args_ptr);

    // メッセージシンボルを抽出。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*result, LType::SYMBOL);
    const std::string& symbol = result->symbol();

    if (message_func_map_.find(symbol) != message_func_map_.end()) {
      return message_func_map_.at(symbol)(symbol, self, caller, args);
    }

    throw Lisp::GenError("@engine-error",
    "'" + symbol + "' is not message symbol.");
  }

  // ====================== //
  // メッセージシンボル関数 //
  // ====================== //
  // %%% @get-white-pawn-position
  // %%% @get-white-knight-position
  // %%% @get-white-bishop-position
  // %%% @get-white-rook-position
  // %%% @get-white-queen-position
  // %%% @get-white-king-position
  // %%% @get-black-pawn-position
  // %%% @get-black-knight-position
  // %%% @get-black-bishop-position
  // %%% @get-black-rook-position
  // %%% @get-black-queen-position
  // %%% @get-black-king-position
  template<Side SIDE, PieceType PIECE_TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::GetPosition) {
    LPointerVec ret_vec;
    for (Bitboard bb = board_ptr_->position_[SIDE][PIECE_TYPE]; bb;
    NEXT_BITBOARD(bb)) {
      ret_vec.push_back(Lisp::NewSymbol
      (Sayulisp::SQUARE_MAP_INV[Util::GetSquare(bb)]));
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template LPointer EngineSuite::GetPosition<WHITE, PAWN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, KNIGHT>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, BISHOP>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, ROOK>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, QUEEN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, KING>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, PAWN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, KNIGHT>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, BISHOP>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, ROOK>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, QUEEN>
  (const std::string&, LPointer, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, KING>
  (const std::string&, LPointer, LObject*, const LObject&);

  // %%% @get-piece
  DEF_MESSAGE_FUNCTION(EngineSuite::GetPiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 引数のチェック。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSquare(*result);
    Square square = result->number();

    // サイドと駒の種類を得る。
    LPointer ret_ptr = Lisp::NewList(2);
    ret_ptr->car(Lisp::NewSymbol
    (Sayulisp::SIDE_MAP_INV[board_ptr_->side_board_[square]]));
    ret_ptr->cdr()->car(Lisp::NewSymbol
    (Sayulisp::PIECE_MAP_INV[board_ptr_->piece_board_[square]]));

    return ret_ptr;
  }

  // %%% @get-all-pieces
  DEF_MESSAGE_FUNCTION(EngineSuite::GetAllPieces) {
    LPointerVec ret_vec(NUM_SQUARES);
    LPointerVec::iterator itr = ret_vec.begin();

    // 各マスの駒のリストを作る。
    LPointer elm_ptr;
    FOR_SQUARES(square) {
      elm_ptr = Lisp::NewList(2);
      elm_ptr->car(Lisp::NewSymbol
      (Sayulisp::SIDE_MAP_INV[board_ptr_->side_board_[square]]));
      elm_ptr->cdr()->car(Lisp::NewSymbol
      (Sayulisp::PIECE_MAP_INV[board_ptr_->piece_board_[square]]));

      *itr = elm_ptr;
      ++itr;
    }

    return Lisp::LPointerVecToList(ret_vec);
  }

  // %%% @set-fen
  DEF_MESSAGE_FUNCTION(EngineSuite::SetFEN) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // FENを得る。
    LPointer fen_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*fen_ptr, LType::STRING);

    // パースする。
    FEN fen;
    try {
      fen = FEN(fen_ptr->string());
    } catch (...) {
      throw Lisp::GenError("@engine-error", "Couldn't parse FEN.");
    }

    // キングの数をチェック。
    if ((Util::CountBits(fen.position()[WHITE][KING]) != 1)
    || (Util::CountBits(fen.position()[BLACK][KING]) != 1)) {
      throw Lisp::GenError("@engine-error", "This FEN is invalid position.");
    }

    engine_ptr_->LoadFEN(fen);
    return Lisp::NewBoolean(true);
  }

  // %%% @place-piece
  DEF_MESSAGE_FUNCTION(EngineSuite::PlacePiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 2, &args_ptr);

    // マスを得る。
    LPointer square_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSquare(*square_ptr);
    Lisp::Next(&args_ptr);
    Square square = square_ptr->number();

    // 駒を得る。
    LPointer piece_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckPiece(*piece_ptr);
    Side side = piece_ptr->car()->number();
    PieceType piece_type = piece_ptr->cdr()->car()->number();

    // キングを置き換えることはできない。
    if (board_ptr_->piece_board_[square] == KING) {
      throw Lisp::GenError("@engine-error", "Couldn't overwrite King.");
    }

    // 前の駒のリストを作る。
    LPointer ret_ptr = Lisp::NewList(2);
    ret_ptr->car(Lisp::NewSymbol
    (Sayulisp::SIDE_MAP_INV[board_ptr_->side_board_[square]]));
    ret_ptr->cdr()->car(Lisp::NewSymbol
    (Sayulisp::PIECE_MAP_INV[board_ptr_->piece_board_[square]]));

    // 置き換える。
    engine_ptr_->PlacePiece(square, piece_type, side);

    return ret_ptr;
  }

  // %%% @get-candidate-moves
  DEF_MESSAGE_FUNCTION(EngineSuite::GetCandidateMoves) {
    // 指し手の生成。
    std::vector<Move> move_vec = engine_ptr_->GetLegalMoves();
    LPointer ret_ptr = Lisp::NewList(move_vec.size());

    // リストに代入していく。
    LObject* ptr = ret_ptr.get();
    for (auto move : move_vec) {
      ptr->car(Sayulisp::MoveToList(move));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @set-to-move
  DEF_MESSAGE_FUNCTION(EngineSuite::SetToMove) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 指し手を得る。
    LPointer to_move_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSide(*to_move_ptr);
    Side to_move = to_move_ptr->number();

    // NO_SIDEはダメ。
    if (to_move == NO_SIDE) {
      throw Lisp::GenError("@engine-error", "NO_SIDE is not allowed.");
    }

    // 前の状態。
    LPointer ret_ptr =
    Lisp::NewSymbol(Sayulisp::SIDE_MAP_INV[board_ptr_->to_move_]);

    engine_ptr_->to_move(to_move);
    return ret_ptr;
  }

  // %%% @set-castling-rights
  DEF_MESSAGE_FUNCTION(EngineSuite::SetCastlingRights) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // キャスリングの権利を得る。
    LPointer castling_list_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckList(*castling_list_ptr);

    // キャスリングの権利フラグを作成。
    Castling rights = 0;
    LPointer result;
    int rights_number = 0;
    for (LObject* ptr = castling_list_ptr.get(); ptr->IsPair();
    Lisp::Next(&ptr)) {
      result = caller->Evaluate(*(ptr->car()));
      Sayulisp::CheckCastling(*result);

      rights_number = result->number();
      switch (rights_number) {
        case 1:
          rights |= WHITE_SHORT_CASTLING;
          break;
        case 2:
          rights |= WHITE_LONG_CASTLING;
          break;
        case 3:
          rights |= BLACK_SHORT_CASTLING;
          break;
        case 4:
          rights |= BLACK_LONG_CASTLING;
          break;
      }
    }

    // 前のキャスリングの権利を得る。
    Castling origin_rights = board_ptr_->castling_rights_;
    LPointerVec ret_vec;
    if ((origin_rights & WHITE_SHORT_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("WHITE_SHORT_CASTLING"));
    }
    if ((origin_rights & WHITE_LONG_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("WHITE_LONG_CASTLING"));
    }
    if ((origin_rights & BLACK_SHORT_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("BLACK_SHORT_CASTLING"));
    }
    if ((origin_rights & BLACK_LONG_CASTLING)) {
      ret_vec.push_back(Lisp::NewSymbol("BLACK_LONG_CASTLING"));
    }

    // セットして返す。
    engine_ptr_->castling_rights(rights);
    return Lisp::LPointerVecToList(ret_vec);
  }

  // %%% set-en-passant-square
  DEF_MESSAGE_FUNCTION(EngineSuite::SetEnPassantSquare) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // マスを得る。
    LPointer square_ptr = caller->Evaluate(*(args_ptr->car()));
    Sayulisp::CheckSquare(*square_ptr);
    Square square = square_ptr->number();

    // 前のアンパッサンのマスを作る。
    LPointer ret_ptr = Lisp::NewNil();
    if (board_ptr_->en_passant_square_) {
      ret_ptr = Lisp::NewNumber(board_ptr_->en_passant_square_);
    }

    // マスがアンパッサンのマスのチェック。
    if (board_ptr_->to_move_ == WHITE) {
      // 白番の時は黒側のアンパッサン。
      if (Util::SquareToRank(square) == RANK_6) {
        // 一つ上にポーンがいて、アンパッサンのマスが空の時にセットできる。
        if ((board_ptr_->piece_board_[square] == EMPTY)
        && (board_ptr_->side_board_[square - 8] == BLACK)
        && (board_ptr_->piece_board_[square - 8] == PAWN)) {
          engine_ptr_->en_passant_square(square);
          return ret_ptr;
        }
      }
    } else {
      // 黒番の時は白側のアンパッサン。
      if (Util::SquareToRank(square) == RANK_3) {
        // 一つ上にポーンがいて、アンパッサンのマスが空の時にセットできる。
        if ((board_ptr_->piece_board_[square] == EMPTY)
        && (board_ptr_->side_board_[square + 8] == WHITE)
        && (board_ptr_->piece_board_[square + 8] == PAWN)) {
          engine_ptr_->en_passant_square(square);
          return ret_ptr;
        }
      }
    }

    throw Lisp::GenError("@engine-error", "'" + square_ptr->ToString()
    + "' couldn't be en passant square.");
  }

  // %%% @set-play
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPly) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 手数を得る。
    LPointer ply_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*ply_ptr, LType::NUMBER);
    int ply = ply_ptr->number();

    // 手数はプラスでないとダメ。
    if (ply < 0) {
      throw Lisp::GenError("@engine-error", "Ply must be positive number.");
    }

    LPointer ret_ptr = Lisp::NewNumber(board_ptr_->ply_);
    engine_ptr_->ply(ply);
    return ret_ptr;
  }

  // %%% @set-clock
  DEF_MESSAGE_FUNCTION(EngineSuite::SetClock) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 手数を得る。
    LPointer clock_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*clock_ptr, LType::NUMBER);
    int clock = clock_ptr->number();

    // 手数はプラスでないとダメ。
    if (clock < 0) {
      throw Lisp::GenError("@engine-error", "Clock must be positive number.");
    }

    LPointer ret_ptr = Lisp::NewNumber(board_ptr_->clock_);
    engine_ptr_->clock(clock);
    return ret_ptr;
  }

  // %%% @play-move
  // %%% @play-note
  /** 手を指す。 */
  DEF_MESSAGE_FUNCTION(EngineSuite::PlayMoveOrNote) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 指し手を得る。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Move move = 0;
    if (symbol == "@play-move") {  // @play-move
      move = Sayulisp::ListToMove(*result);
    } else {  // @play-note
      Lisp::CheckType(*result, LType::STRING);
      std::vector<Move> move_vec = engine_ptr_->GuessNote(result->string());
      if (move_vec.size() == 0) return Lisp::NewBoolean(false);
      move = move_vec[0];
    }

    return Lisp::NewBoolean(engine_ptr_->PlayMove(move));
  }

  // %%% @undo-move
  DEF_MESSAGE_FUNCTION(EngineSuite::UndoMove) {
    Move move = engine_ptr_->UndoMove();
    if (!move) return Lisp::NewNil();

    return Sayulisp::MoveToList(move);
  }

  // %%% @move->note
  DEF_MESSAGE_FUNCTION(EngineSuite::MoveToNote) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 指し手を得る。
    LPointer move_ptr = caller->Evaluate(*(args_ptr->car()));

    return Lisp::NewString(engine_ptr_->MoveToNote
    (Sayulisp::ListToMove(*move_ptr)));
  }

  // %%% @input-uci-command
  DEF_MESSAGE_FUNCTION(EngineSuite::InputUCICommand) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    LPointer command_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*command_ptr, LType::STRING);

    return Lisp::NewBoolean(shell_ptr_->InputCommand(command_ptr->string()));
  }

  // %%% @add-uci-output-listener
  DEF_MESSAGE_FUNCTION(EngineSuite::AddUCIOutputListener) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    LPointer result = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*result, LType::FUNCTION);

    // リストを作る。
    LPointer listener_ptr =
    Lisp::NewPair(result, Lisp::NewPair(Lisp::NewString(""), Lisp::NewNil()));

    // callerと同じスコープの関数オブジェクトを作る。
    LPointer caller_scope =
    Lisp::NewN_Function(LC_Function(), "", caller->scope_chain());

    // コールバック関数を作成。
    auto callback =
    [caller_scope, listener_ptr](const std::string& message) {
      listener_ptr->cdr()->car()->string(message);
      caller_scope->Evaluate(*listener_ptr);
    };

    // コールバック関数を登録。
    callback_vec_.push_back(callback);

    return Lisp::NewBoolean(true);
  }

  // %%% @run
  DEF_MESSAGE_FUNCTION(EngineSuite::RunEngine) {
    // 出力リスナー。
    auto callback = [](const std::string& message) {
      std::cout << message << std::endl;
    };
    callback_vec_.push_back(callback);

    // quitが来るまでループ。
    std::string input;
    while (true) {
      std::getline(std::cin, input);
      if (input == "quit") break;
      shell_ptr_->InputCommand(input);
    }

    return Lisp::NewBoolean(true);
  }

  // Go...()で使う関数。
  LPointer EngineSuite::GoFunc(std::uint32_t depth, std::uint64_t nodes,
  int thinking_time, const LObject& candidate_list) {
    // 候補手のリストを作成。
    std::vector<Move> candidate_vec(Lisp::CountList(candidate_list));
    std::vector<Move>::iterator candidate_itr = candidate_vec.begin();
    LPointer car;
    for (const LObject* ptr = &candidate_list; ptr->IsPair();
    ptr = ptr->cdr().get(), ++candidate_itr) {
      car = ptr->car();
      Sayulisp::CheckMove(*car);
      *candidate_itr = Sayulisp::ListToMove(*car);
    }

    // ストッパーを登録。
    engine_ptr_->SetStopper(Util::GetMin(depth, MAX_PLYS),
    Util::GetMin(nodes, MAX_NODES),
    Chrono::milliseconds(thinking_time), false);

    // テーブルの年齢を上げる。
    table_ptr_->GrowOld();

    // 思考開始。
    PVLine pv_line = engine_ptr_->Calculate(shell_ptr_->num_threads(),
    candidate_vec, *shell_ptr_);

    // 最善手、Ponderをアウトプットリスナーに送る。
    std::ostringstream oss;
    int len = pv_line.length();
    if (len) {
      oss << "bestmove " << Util::MoveToString(pv_line[0]);
      if (len >= 2) {
        oss << " ponder " << Util::MoveToString(pv_line[1]);
      }
      for (auto& callback : callback_vec_) callback(oss.str());
    }

    // PVラインのリストを作る。
    LPointer ret_ptr = Lisp::NewList(len + 2);
    LObject* ptr = ret_ptr.get();
    ptr->car(Lisp::NewNumber(pv_line.score()));
    Lisp::Next(&ptr);
    ptr->car(Lisp::NewNumber(pv_line.mate_in()));
    Lisp::Next(&ptr);
    for (int i = 0; i < len; ++i, Lisp::Next(&ptr)) {
      ptr->car(Sayulisp::MoveToList(pv_line[i]));
    }

    return ret_ptr;
  }

  // %%% @go-movetime
  DEF_MESSAGE_FUNCTION(EngineSuite::GoMoveTime) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 思考時間を得る。
    LPointer time_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*time_ptr, LType::NUMBER);
    int time = time_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(MAX_PLYS, MAX_NODES, time, *candidate_list_ptr);
  }

  // %%% @go-timelimit
  DEF_MESSAGE_FUNCTION(EngineSuite::GoTimeLimit) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 持ち時間を得る。
    LPointer time_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*time_ptr, LType::NUMBER);
    int time = TimeLimitToMoveTime(time_ptr->number());
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(MAX_PLYS, MAX_NODES, time, *candidate_list_ptr);
  }

  // %%% @go-depth
  DEF_MESSAGE_FUNCTION(EngineSuite::GoDepth) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 深さを得る。
    LPointer depth_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*depth_ptr, LType::NUMBER);
    std::uint32_t depth = depth_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(depth, MAX_NODES, INT_MAX, *candidate_list_ptr);
  }

  // %%% @go-nodes
  DEF_MESSAGE_FUNCTION(EngineSuite::GoNodes) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // ノード数を得る。
    LPointer node_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*node_ptr, LType::NUMBER);
    std::uint64_t node = node_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);
      candidate_list_ptr = result;
    }

    // GoFuncに渡して終わる。
    return GoFunc(MAX_PLYS, node, INT_MAX, *candidate_list_ptr);
  }

  // %%% @set-hash-size
  DEF_MESSAGE_FUNCTION(EngineSuite::SetHashSize) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // サイズを得る。
    LPointer size_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*size_ptr, LType::NUMBER);
    std::size_t size = Util::GetMax(size_ptr->number(),
    TTEntry::TTENTRY_HARD_CODED_SIZE);

    // 古いサイズ。
    LPointer ret_ptr = Lisp::NewNumber(table_ptr_->GetSizeBytes());

    // サイズを更新。
    table_ptr_->SetSize(size);

    return ret_ptr;
  }

  // %%% @set-threads
  DEF_MESSAGE_FUNCTION(EngineSuite::SetThreads) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // スレッド数を得る。
    LPointer threads_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*threads_ptr, LType::NUMBER);
    int threads = Util::GetMax(threads_ptr->number(), 1);

    // 古いスレッド数。
    LPointer ret_ptr = Lisp::NewNumber(shell_ptr_->num_threads());

    // スレッド数を更新。
    shell_ptr_->num_threads(threads);

    return ret_ptr;
  }

  // %%% @material
  DEF_MESSAGE_FUNCTION(EngineSuite::SetMaterial) {
    // 古い設定を得る。
    const int (& material)[NUM_PIECE_TYPES] = search_params_ptr_->material();
    LPointerVec ret_vec(7);
    FOR_PIECE_TYPES(piece_type) {
      ret_vec[piece_type] = Lisp::NewNumber(material[piece_type]);
    }

    // もし引数があるなら設定。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      int len = Lisp::CountList(*result);
      if (len < 7) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "'"" requires List of 7 elements.");
      }

      int new_material[NUM_PIECE_TYPES];
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type) {
        if (piece_type == EMPTY) {
          new_material[piece_type] = 0;
        } else {
          new_material[piece_type] = ptr->car()->number();
        }
        Lisp::Next(&ptr);
      }

      search_params_ptr_->material(new_material);
    }

    return Lisp::LPointerVecToList(ret_vec);
  }

// @pawn-square-table-opening
// @knight-square-table-opening
// @bishop-square-table-opening
// @rook-square-table-opening
// @queen-square-table-opening
// @king-square-table-opening
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_SQUARES);
    const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES] =
    eval_params_ptr_->opening_position_value_table();
    FOR_SQUARES(square) {
      ret_vec[square] = Lisp::NewNumber(table[TYPE][square]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_SQUARES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_SQUARES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_SQUARES(square) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->opening_position_value_table
        (TYPE, square, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  // インスタンス化。
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableOpening<KING>);

// @pawn-square-table-ending
// @knight-square-table-ending
// @bishop-square-table-ending
// @rook-square-table-ending
// @queen-square-table-ending
// @king-square-table-ending
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_SQUARES);
    const double (& table)[NUM_PIECE_TYPES][NUM_SQUARES] =
    eval_params_ptr_->ending_position_value_table();
    FOR_SQUARES(square) {
      ret_vec[square] = Lisp::NewNumber(table[TYPE][square]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_SQUARES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_SQUARES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_SQUARES(square) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->ending_position_value_table
        (TYPE, square, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  // インスタンス化。
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPieceSquareTableEnding<KING>);

  // %%% @pawn-attack-table
  // %%% @knight-attack-table
  // %%% @bishop-attack-table
  // %%% @rook-attack-table
  // %%% @queen-attack-table
  // %%% @king-attack-table
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_PIECE_TYPES);
    const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    eval_params_ptr_->attack_value_table();
    FOR_PIECE_TYPES(piece_type) {
      ret_vec[piece_type] = Lisp::NewNumber(table[TYPE][piece_type]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_PIECE_TYPES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_PIECE_TYPES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->attack_value_table
        (TYPE, piece_type, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetAttackTable<KING>);

  // %%% @pawn-defense-table
  // %%% @knight-defense-table
  // %%% @bishop-defense-table
  // %%% @rook-defense-table
  // %%% @queen-defense-table
  // %%% @king-defense-table
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_PIECE_TYPES);
    const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    eval_params_ptr_->defense_value_table();
    FOR_PIECE_TYPES(piece_type) {
      ret_vec[piece_type] = Lisp::NewNumber(table[TYPE][piece_type]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_PIECE_TYPES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_PIECE_TYPES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->defense_value_table
        (TYPE, piece_type, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetDefenseTable<KING>);

  // @bishop-pin-tab
  // @rook-pin-table
  // @queen-pin-table
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_PIECE_TYPES);
    const double (& table)
    [NUM_PIECE_TYPES][NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    eval_params_ptr_->pin_value_table();
    FOR_PIECE_TYPES(piece_type_1) {
      LPointerVec temp_vec(NUM_PIECE_TYPES);

      FOR_PIECE_TYPES(piece_type_2) {
        temp_vec[piece_type_2] =
        Lisp::NewNumber(table[TYPE][piece_type_1][piece_type_2]);
      }

      ret_vec[piece_type_1] = Lisp::LPointerVecToList(temp_vec);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_PIECE_TYPES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_PIECE_TYPES) + " x "
        + std::to_string(NUM_PIECE_TYPES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_PIECE_TYPES(piece_type_1) {
        const LPointer& car = ptr->car();

        // チェックする。
        Lisp::CheckList(*car);
        if (Lisp::CountList(*car) < static_cast<int>(NUM_PIECE_TYPES)) {
          throw Lisp::GenError("@engine-error",
          "'" + symbol + "' requires List of "
          + std::to_string(NUM_PIECE_TYPES) + "x"
          + std::to_string(NUM_PIECE_TYPES) + " elements.");
        }

        // 内側のループ。
        LObject* ptr_2 = car.get();
        FOR_PIECE_TYPES(piece_type_2) {
          Lisp::CheckType(*(ptr_2->car()), LType::NUMBER);

          eval_params_ptr_->pin_value_table
          (TYPE, piece_type_1, piece_type_2, ptr_2->car()->number());

          Lisp::Next(&ptr_2);
        }

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetPinTable<QUEEN>);

  DEF_MESSAGE_FUNCTION(EngineSuite::SetPawnShieldTable) {
    // 古い設定を得る。
    LPointerVec ret_vec(NUM_SQUARES);
    const double (& table)[NUM_SQUARES] =
    eval_params_ptr_->pawn_shield_value_table();
    FOR_SQUARES(square) {
      ret_vec[square] = Lisp::NewNumber(table[square]);
    }

    // 引数があれば設定する。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(*(args_ptr->car()));
      Lisp::CheckList(*result);

      // マスの数がちゃんとあるかどうか。
      if (Lisp::CountList(*result) < static_cast<int>(NUM_SQUARES)) {
        throw Lisp::GenError("@engine-error",
        "'" + symbol + "' requires List of "
        + std::to_string(NUM_SQUARES) + " elements.");
      }

      // セットする。
      LObject* ptr = result.get();
      FOR_SQUARES(square) {
        Lisp::CheckType(*(ptr->car()), LType::NUMBER);

        eval_params_ptr_->pawn_shield_value_table
        (square, ptr->car()->number());

        Lisp::Next(&ptr);
      }
    }

    return Lisp::LPointerVecToList(ret_vec);
  }
}  // namespace Sayuri
