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
#include "fen.h"

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

    }

    throw LispObject::GenError("@engine-error", "(" + func_name
    + ") couldn't understand '" + message_symbol + "'.");
  }

  // 白ポーンの配置にアクセス。
  LispObjectPtr EngineSuite::GetWhitePawnPosition() const {
    LispObjectPtr ret_ptr = LispObject::NewNil();
    for (Bitboard bb = engine_ptr_->position()[WHITE][PAWN];
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
    bb; Util::SetNext(bb)) {
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
      "The square value '" + std::to_string(square_ptr->number_value())
      + "' doesn't indicate any square.");
    }
    if (piece_type >= NUM_PIECE_TYPES) {
      throw LispObject::GenError("@engine_error",
      "The piece type value '" + std::to_string(type_ptr->number_value())
      +  "' doesn't indicate any piece type.");
    }
    if (side >= NUM_SIDES) {
      throw LispObject::GenError("@engine_error",
      "The side value '" + std::to_string(side_ptr->number_value())
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
      "The side value '" + std::to_string(to_move_ptr->number_value())
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
      + std::to_string(en_passant_square_ptr->number_value())
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

  // 手数をセット。
  LispObjectPtr EngineSuite::SetPly(LispObjectPtr ply_ptr) {
    int ply = ply_ptr->number_value();
    if (ply < 1) {
      throw LispObject::GenError("@engine_error",
      "Minimum ply number is '1'. Given '"
      + std::to_string(ply_ptr->number_value()) + "'.");
    }

    int origin = engine_ptr_->ply();
    engine_ptr_->ply(ply);
    return LispObject::NewNumber(origin);
  }

  // 50手ルールの手数をセット。
  LispObjectPtr EngineSuite::SetClock(LispObjectPtr clock_ptr) {
    int clock = clock_ptr->number_value();
    if (clock < 0) {
      throw LispObject::GenError("@engine_error",
      "Minimum clock number is '0'. Given '"
      + std::to_string(clock_ptr->number_value()) + "'.");
    }

    int origin = engine_ptr_->clock();
    engine_ptr_->clock(clock);
    return LispObject::NewNumber(origin);
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
