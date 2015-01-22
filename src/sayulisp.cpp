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
  const std::string EngineSuite::PIECE_SYMBOL[NUM_PIECE_TYPES] {
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

    } else if (message_symbol == "@get-ply-100") {
      return GetPly100();

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
    (PIECE_SYMBOL[engine_ptr_->piece_board()[square]]));

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
    for (Square square = 0; square < NUM_SQUARES; ++square) {
      global_ptr_->BindSymbol(EngineSuite::SQUARE_SYMBOL[square],
      LispObject::NewNumber(square));
    }

    // ファイルの定数をバインド。
    for (Fyle fyle = 0; fyle < NUM_FYLES; ++fyle) {
      global_ptr_->BindSymbol(EngineSuite::FYLE_SYMBOL[fyle],
      LispObject::NewNumber(fyle));
    }

    // ランクの定数をバインド。
    for (Rank rank = 0; rank < NUM_RANKS; ++rank) {
      global_ptr_->BindSymbol(EngineSuite::RANK_SYMBOL[rank],
      LispObject::NewNumber(rank));
    }

    // サイドの定数をバインド。
    for (Side side = 0; side < NUM_SIDES; ++side) {
      global_ptr_->BindSymbol(EngineSuite::SIDE_SYMBOL[side],
      LispObject::NewNumber(side));
    }

    // 駒の定数をバインド。
    for (PieceType piece_type = 0; piece_type < NUM_PIECE_TYPES; ++piece_type) {
      global_ptr_->BindSymbol(EngineSuite::PIECE_SYMBOL[piece_type],
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
  void Sayulisp::Run(std::istream* stream_ptr) {
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
          }

          tokenizer.Reset();
        } else if (parentheses < 0) {
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

  // ヘルプを作成する。
  void Sayulisp::SetHelp() {
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

    + `@get-ply-100`
        - Return moves for 50 Moves Rule as plies. (1 move = 2 plies)

    + `@get-white-has-castled`
        - Return #t if White has castled. If not, it returns #f.

    + `@get-black-has-castled`
        - Return #t if Black has castled. If not, it returns #f.

    + `@set-new-game`
        - Set starting position and state.
        - Return #t.

    + `@set-fen`
        - Set position and state into indicated position by FEN.
        - Return #t.

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
