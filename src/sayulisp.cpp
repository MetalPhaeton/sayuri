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

    } else if (message_symbol == "@get-ply-100") {
      return GetPly100();

    } else if (message_symbol == "@get-white-has-castled") {
      return GetWhiteHasCastled();

    } else if (message_symbol == "@get-black-has-castled") {
      return GetBlackHasCastled();

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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
      (LispObject::NewSymbol(square_symbol_table_[Util::GetSquare(bb)]),
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
    (side_symbol_table_[engine_ptr_->side_board()[square]]));

    ret_ptr->cdr()->car(LispObject::NewSymbol
    (piece_symbol_table_[engine_ptr_->piece_board()[square]]));

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
    LispObject::SetBasicFunctions(global_ptr_, dict_ptr_);

    LispObjectPtr func_ptr = LispObject::NewNativeFunction();
    func_ptr->scope_chain(global_ptr_->scope_chain());
    func_ptr->native_function([this]
    (LispObjectPtr self, const LispObject& caller, const LispObject& list)
    -> LispObjectPtr {
      return this->GenEngine();
    });

    global_ptr_->BindSymbol("gen-engine", func_ptr);
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
  void Sayulisp::Run(std::istream* stream_ptr) {
    SyntaxChecker checker;

    try {
      std::string input;
      while (std::getline(*(stream_ptr), input)) {
        input += "\n";
        int count = checker.Input(input);
        if (count == 0) {
          LispObjectPtr lisp_obj_ptr =
          LispObject::Parse(checker.no_comment_str());

          global_ptr_->Evaluate(*lisp_obj_ptr);

          checker.Reset();
        } else if (count < 0) {
          throw LispObject::GenError("@parse-error", "Too many parentheses.");
        }
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
}  // namespace Sayuri
