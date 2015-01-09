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
#include "common.h"
#include "params.h"
#include "chess_engine.h"
#include "transposition_table.h"
#include "uci_shell.h"
#include "lisp_core.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ========== //
  // static定数 //
  // ========== //
  const std::string EngineSuite::square_symbol_table_[NUM_SQUARES] {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"
  };
  const std::string EngineSuite::fyle_symbol_table_[NUM_FYLES] {
    "FYLE_A", "FYLE_B", "FYLE_C", "FYLE_D",
    "FYLE_E", "FYLE_F", "FYLE_G", "FYLE_H"
  };
  const std::string EngineSuite::rank_symbol_table_[NUM_RANKS] {
    "RANK_1", "RANK_2", "RANK_3", "RANK_4",
    "RANK_5", "RANK_6", "RANK_7", "RANK_8"
  };
  const std::string EngineSuite::side_symbol_table_[NUM_SIDES] {
    "NO_SIDE", "WHITE", "BLACK"
  };
  const std::string EngineSuite::piece_symbol_table_[NUM_PIECE_TYPES] {
    "EMPTY", "PAWN", "KNIGHT", "BISHOP", "ROOK", "QUEEN", "KING"
  };
  const std::string EngineSuite::castling_symbol_table_[5] {
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
  shell_ptr_(new UCIShell(*engine_ptr_, *table_ptr_)) {}

  // コピーコンストラクタ。
  EngineSuite::EngineSuite(const EngineSuite& suite) :
  search_params_ptr_(new SearchParams(*(suite.search_params_ptr_))),
  eval_params_ptr_(new EvalParams(*(suite.eval_params_ptr_))),
  engine_ptr_(new ChessEngine(*(suite.engine_ptr_))),
  table_ptr_(new TranspositionTable(*(suite.table_ptr_))),
  shell_ptr_(new UCIShell(*(suite.shell_ptr_))) {}

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
    engine_ptr_.reset(new ChessEngine(*(suite.engine_ptr_)));
    table_ptr_.reset(new TranspositionTable(*(suite.table_ptr_)));
    shell_ptr_.reset(new UCIShell(*(suite.shell_ptr_)));
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

  // ================================== //
  // Lispオブジェクトとしてのメンバ関数 //
  // ================================== //
  // 関数オブジェクト。
  LispObjectPtr EngineSuite::operator()
  (LispObjectPtr self, const LispObject& caller, const LispObject& list) {
    // 準備。
    return LispObject::NewNil();
  }

  // 白ポーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhitePawn() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][PAWN];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白ナイトの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteKnight() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][KNIGHT];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白ビショップの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteBishop() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][BISHOP];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白ルークの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteRook() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][ROOK];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白クイーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteQueen() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][QUEEN];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 白キングの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhiteKing() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][KING];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ポーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackPawn() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][PAWN];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ナイトの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackKnight() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][KNIGHT];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ビショップの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackBishop() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][BISHOP];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒ルークの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackRook() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][ROOK];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒クイーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackQueen() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][QUEEN];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 黒キングの配置にアクセス。
  LispObjectPtr EngineSuite::GetBlackKing() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[BLACK][KING];
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }
  // 空のマスの配置にアクセス。
  LispObjectPtr EngineSuite::GetEmptySquare() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = ~(engine_ptr_->blocker_0());
    bb; Util::SetNext(bb)) {
      LispObjectPtr temp = LispObject::NewPair
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
      LispObject::NewNil());

      ret_ptr->Append(temp);
    }

    return ret_ptr;
  }

  // 駒を得る。
  LispObjectPtr EngineSuite::GetPiece(const std::string& func_name,
  const LispObject& square) const {
    LispObjectPtr ret_ptr = LispObject::NewList(2);
    int square_2 = square.number_value();
    if ((square_2 < 0) || (square_2 >= static_cast<int>(NUM_SQUARES))) {
      throw GenWrongSquareError(func_name, square_2);
    }

    (*ret_ptr)[0] = *(LispObject::NewSymbol
    (side_symbol_table_[engine_ptr_->side_board()[square_2]]));

    (*ret_ptr)[1] = *(LispObject::NewSymbol
    (side_symbol_table_[engine_ptr_->piece_board()[square_2]]));

    return ret_ptr;
  }

  // 手番にアクセス。
  LispObjectPtr EngineSuite::GetToMove() const {
    return LispObject::NewSymbol(side_symbol_table_[engine_ptr_->to_move()]);
  }

  // キャスリングの権利にアクセス。
  LispObjectPtr EngineSuite::GetCastlingRights() const {
    Castling rights = engine_ptr_->castling_rights();

    LispObjectPtr ret_ptr = LispObject::NewNil();
    if ((rights & WHITE_SHORT_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(castling_symbol_table_[1]),
      LispObject::NewNil()));
    }
    if ((rights & WHITE_LONG_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(castling_symbol_table_[2]),
      LispObject::NewNil()));
    }
    if ((rights & BLACK_SHORT_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(castling_symbol_table_[3]),
      LispObject::NewNil()));
    }
    if ((rights & BLACK_LONG_CASTLING)) {
      ret_ptr->Append(LispObject::NewPair
      (LispObject::NewSymbol(castling_symbol_table_[4]),
      LispObject::NewNil()));
    }

    return ret_ptr;
  }

  // アンパッサンのマスにアクセス。
  LispObjectPtr EngineSuite::GetEnPassantSquare() const {
    Square en_passant_square = engine_ptr_->en_passant_square();

    if (en_passant_square) {
      return LispObject::NewSymbol(square_symbol_table_[en_passant_square]);
    }

    return LispObject::NewNil();
  }

  // 手数にアクセス。
  LispObjectPtr EngineSuite::GetPly() const {
    return LispObject::NewNumber(engine_ptr_->ply());
  }

  // 50手ルールの手数にアクセス。
  LispObjectPtr EngineSuite::GetPly100() const {
    return LispObject::NewNumber(engine_ptr_->ply_100());
  }

  // 白がキャスリングしたかどうかのフラグにアクセス。
  LispObjectPtr EngineSuite::GetWhiteHasCastled() const {
    return LispObject::NewBoolean(engine_ptr_->has_castled()[WHITE]);
  }
  // 黒がキャスリングしたかどうかのフラグにアクセス。
  LispObjectPtr EngineSuite::GetBlackHasCastled() const {
    return LispObject::NewBoolean(engine_ptr_->has_castled()[BLACK]);
  }
}  // namespace Sayuri
