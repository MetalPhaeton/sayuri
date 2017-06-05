/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
#include <tuple>
#include <cstdlib>
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
#include "analyse.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
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
  const std::map<std::string, std::uint32_t> Sayulisp::CASTLING_MAP {
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

    func = LC_FUNCTION_OBJ(SayuriLicense);
    INSERT_LC_FUNCTION(func, "sayuri-license", "Sayulisp:sayuri-license");

    func = LC_FUNCTION_OBJ(SquareToNumber);
    INSERT_LC_FUNCTION(func, "square->number", "Sayulisp:square->number");

    func = LC_FUNCTION_OBJ(FyleToNumber);
    INSERT_LC_FUNCTION(func, "fyle->number", "Sayulisp:fyle->number");

    func = LC_FUNCTION_OBJ(RankToNumber);
    INSERT_LC_FUNCTION(func, "rank->number", "Sayulisp:rank->number");

    func = LC_FUNCTION_OBJ(SideToNumber);
    INSERT_LC_FUNCTION(func, "side->number", "Sayulisp:side->number");

    func = LC_FUNCTION_OBJ(PieceToNumber);
    INSERT_LC_FUNCTION(func, "piece->number", "Sayulisp:piece->number");

    func = LC_FUNCTION_OBJ(CastlingToNumber);
    INSERT_LC_FUNCTION(func, "castling->number", "Sayulisp:castling->number");

    func = LC_FUNCTION_OBJ(ChessToNumber);
    INSERT_LC_FUNCTION(func, "chess->number", "Sayulisp:chess->number");

    func = LC_FUNCTION_OBJ(NumberToSquare);
    INSERT_LC_FUNCTION(func, "number->square", "Sayulisp:number->square");

    func = LC_FUNCTION_OBJ(NumberToFyle);
    INSERT_LC_FUNCTION(func, "number->fyle", "Sayulisp:number->fyle");

    func = LC_FUNCTION_OBJ(NumberToRank);
    INSERT_LC_FUNCTION(func, "number->rank", "Sayulisp:number->rank");

    func = LC_FUNCTION_OBJ(NumberToSide);
    INSERT_LC_FUNCTION(func, "number->side", "Sayulisp:number->side");

    func = LC_FUNCTION_OBJ(NumberToPiece);
    INSERT_LC_FUNCTION(func, "number->piece", "Sayulisp:number->piece");

    func = LC_FUNCTION_OBJ(NumberToCastling);
    INSERT_LC_FUNCTION(func, "number->castling", "Sayulisp:number->castling");

    func = LC_FUNCTION_OBJ(GenEngine);
    INSERT_LC_FUNCTION(func, "gen-engine", "Sayulisp:gen-engine");

    func = LC_FUNCTION_OBJ(GenPGN);
    INSERT_LC_FUNCTION(func, "gen-pgn", "Sayulisp:gen-pgn");

    func = LC_FUNCTION_OBJ(ParseFENEPD);
    INSERT_LC_FUNCTION(func, "parse-fen/epd", "Sayulisp:parse-fen/epd");

    func = LC_FUNCTION_OBJ(ToFENPosition);
    INSERT_LC_FUNCTION(func, "to-fen-position", "Sayulisp:to-fen-position");
  }

  // Sayulispを開始する。
  int Sayulisp::Run(std::istream* stream_ptr) {
    // 終了ステータス。
    int status = 0;

    // (exit)関数を作成。
    bool loop = true;
    auto func = [&status, &loop](const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = args.cdr().get();

      // ループをセット。
      loop = false;

      // 引数があった場合は終了ステータスあり。
      if (args_ptr->IsPair()) {
        LPointer result = caller->Evaluate(args_ptr->car());
        status = result->number();
      }

      std::exit(status);
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
          Evaluate(s);
        }

        if (!loop) break;
      }
    } catch (LPointer error) {
      PrintError(error);
    }

    return status;
  }
  // Sayulispを開始する。
  int Sayulisp::Run(const std::string& code) {
    // 終了ステータス。
    int status = 0;

    // (exit)関数を作成。
    auto func = [&status](const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = args.cdr().get();

      // 引数があった場合は終了ステータスあり。
      if (args_ptr->IsPair()) {
        LPointer result = caller->Evaluate(args_ptr->car());
        status = result->number();
      }

      std::exit(status);
      return Lisp::NewNumber(status);
    };
    scope_chain_.InsertSymbol("exit",
    NewN_Function(func, "Sayulisp:exit", scope_chain_));

    try {
      Tokenize(code);

      LPointerVec s_tree = Parse();
      for (auto& s : s_tree) Evaluate(s);
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

  // Walk用関数オブジェクトを作成する。
  LFuncForWalk Sayulisp::GenFuncToNumber
  (const std::map<std::string, std::uint32_t>& map) {
    return [&map](LObject& pair, const std::string& path) {
      // Car。
      LObject* car = pair.car().get();
      if (car->IsSymbol()) {
        // 探す。
        std::map<std::string, std::uint32_t>::const_iterator itr =
        map.find(car->symbol());

        // 見つかった。
        if (itr != map.end()) {
          pair.car(Lisp::NewNumber(itr->second));
        }
      }

      // Cdr。
      LObject* cdr = pair.cdr().get();
      if (cdr->IsSymbol()) {
        // 探す。
        std::map<std::string, std::uint32_t>::const_iterator itr =
        map.find(cdr->symbol());

        // 見つかった。
        if (itr != map.end()) {
          pair.cdr(Lisp::NewNumber(itr->second));
        }
      }
    };
  }

  // チェスのシンボルを数字に変えるWalk用関数。
  void Sayulisp::ChessToNumberCore(LObject& pair, const std::string& path) {
    // Car。
    LObject* car = pair.car().get();
    if (car->IsSymbol()) {
      do {
        // マス。
        std::map<std::string, std::uint32_t>::const_iterator itr =
        SQUARE_MAP.find(car->symbol());
        if (itr != SQUARE_MAP.end()) {
          pair.car(Lisp::NewNumber(itr->second));
          break;
        }

        // サイド。
        itr = SIDE_MAP.find(car->symbol());
        if (itr != SIDE_MAP.end()) {
          pair.car(Lisp::NewNumber(itr->second));
          break;
        }

        // 駒の種類。
        itr = PIECE_MAP.find(car->symbol());
        if (itr != PIECE_MAP.end()) {
          pair.car(Lisp::NewNumber(itr->second));
          break;
        }

        // キャスリング。
        itr = CASTLING_MAP.find(car->symbol());
        if (itr != CASTLING_MAP.end()) {
          pair.car(Lisp::NewNumber(itr->second));
          break;
        }

        // ファイル。
        itr = FYLE_MAP.find(car->symbol());
        if (itr != FYLE_MAP.end()) {
          pair.car(Lisp::NewNumber(itr->second));
          break;
        }

        // ランク。
        itr = RANK_MAP.find(car->symbol());
        if (itr != RANK_MAP.end()) {
          pair.car(Lisp::NewNumber(itr->second));
          break;
        }
      } while (false);
    }

    // Cdr。
    LObject* cdr = pair.cdr().get();
    if (cdr->IsSymbol()) {
      do {
        // マス。
        std::map<std::string, std::uint32_t>::const_iterator itr =
        SQUARE_MAP.find(cdr->symbol());
        if (itr != SQUARE_MAP.end()) {
          pair.cdr(Lisp::NewNumber(itr->second));
          break;
        }

        // サイド。
        itr = SIDE_MAP.find(cdr->symbol());
        if (itr != SIDE_MAP.end()) {
          pair.cdr(Lisp::NewNumber(itr->second));
          break;
        }

        // 駒の種類。
        itr = PIECE_MAP.find(cdr->symbol());
        if (itr != PIECE_MAP.end()) {
          pair.cdr(Lisp::NewNumber(itr->second));
          break;
        }

        // キャスリング。
        itr = CASTLING_MAP.find(cdr->symbol());
        if (itr != CASTLING_MAP.end()) {
          pair.cdr(Lisp::NewNumber(itr->second));
          break;
        }

        // ファイル。
        itr = FYLE_MAP.find(cdr->symbol());
        if (itr != FYLE_MAP.end()) {
          pair.cdr(Lisp::NewNumber(itr->second));
          break;
        }

        // ランク。
        itr = RANK_MAP.find(cdr->symbol());
        if (itr != RANK_MAP.end()) {
          pair.cdr(Lisp::NewNumber(itr->second));
          break;
        }
      } while (false);
    }
  }

  // エンジンを生成する。
  DEF_LC_FUNCTION(Sayulisp::GenEngine) {
    // スイートを作成。
    std::shared_ptr<EngineSuite> suite_ptr(new EngineSuite());

    // ネイティブ関数オブジェクトを作成。
    auto func = [suite_ptr](const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return (*suite_ptr)(self, caller, args);
    };

    return NewN_Function(func, "Sayulisp:gen-engine:"
    + std::to_string(reinterpret_cast<std::size_t>(suite_ptr.get())),
    caller->scope_chain());
  }

#define TO_NUMBER_DEFINITION(map_name) \
    LObject* args_ptr = nullptr;\
    GetReadyForFunction(args, 1, &args_ptr);\
\
    LPointer list_ptr = caller->Evaluate(args_ptr->car())->Clone();\
\
    if (list_ptr->IsSymbol()) {\
      std::map<std::string, std::uint32_t>::const_iterator itr =\
      map_name.find(list_ptr->symbol());\
      if (itr != map_name.end()) {\
        return NewNumber(itr->second);\
      }\
    } else if (list_ptr->IsPair()) {\
      Walk(*list_ptr, GenFuncToNumber(map_name));\
    }\
\
    return list_ptr

  // マスのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::SquareToNumber) {
    TO_NUMBER_DEFINITION(SQUARE_MAP);
  }

  // ファイルのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::FyleToNumber) {
    TO_NUMBER_DEFINITION(FYLE_MAP);
  }

  // ランクのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::RankToNumber) {
    TO_NUMBER_DEFINITION(RANK_MAP);
  }

  // サイドのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::SideToNumber) {
    TO_NUMBER_DEFINITION(SIDE_MAP);
  }

  // 駒の種類のシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::PieceToNumber) {
    TO_NUMBER_DEFINITION(PIECE_MAP);
  }

  // キャスリングのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::CastlingToNumber) {
    TO_NUMBER_DEFINITION(CASTLING_MAP);
  }

  // チェスのシンボルを数値に変換する。
  DEF_LC_FUNCTION(Sayulisp::ChessToNumber) {
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    LPointer list_ptr = caller->Evaluate(args_ptr->car())->Clone();

    if (list_ptr->IsSymbol()) {
      // マス。
      std::map<std::string, std::uint32_t>::const_iterator itr =
      SQUARE_MAP.find(list_ptr->symbol());
      if (itr != SQUARE_MAP.end()) return NewNumber(itr->second);

      // サイド。
      itr = SIDE_MAP.find(list_ptr->symbol());
      if (itr != SIDE_MAP.end()) return NewNumber(itr->second);

      // 駒の種類。
      itr = PIECE_MAP.find(list_ptr->symbol());
      if (itr != PIECE_MAP.end()) return NewNumber(itr->second);

      // キャスリング。
      itr = CASTLING_MAP.find(list_ptr->symbol());
      if (itr != CASTLING_MAP.end()) return NewNumber(itr->second);

      // ファイル。
      itr = FYLE_MAP.find(list_ptr->symbol());
      if (itr != FYLE_MAP.end()) return NewNumber(itr->second);

      // ランク。
      itr = RANK_MAP.find(list_ptr->symbol());
      if (itr != RANK_MAP.end()) return NewNumber(itr->second);
    } else if (list_ptr->IsPair()) {
      Walk(*list_ptr, ChessToNumberCore);
    }

    return list_ptr;
  }

#define TO_SYMBOL_DEFINITION(array_name, limit) \
    LObject* args_ptr = nullptr;\
    GetReadyForFunction(args, 1, &args_ptr);\
\
    LPointer list_ptr = caller->Evaluate(args_ptr->car())->Clone();\
    if (list_ptr->IsNumber()) {\
      std::uint32_t index = list_ptr->number();\
      if (index < limit) {\
        return NewSymbol(array_name[index]);\
      }\
    } else if (list_ptr->IsList()) {\
      Walk(*list_ptr, GenFuncToSymbol<limit>(array_name));\
    }\
    return list_ptr

  // 数値をマスのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToSquare) {
    TO_SYMBOL_DEFINITION(SQUARE_MAP_INV, NUM_SQUARES);
  }

  // 数値をファイルのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToFyle) {
    TO_SYMBOL_DEFINITION(FYLE_MAP_INV, NUM_FYLES);
  }

  // 数値をランクのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToRank) {
    TO_SYMBOL_DEFINITION(RANK_MAP_INV, NUM_RANKS);
  }

  // 数値をサイドのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToSide) {
    TO_SYMBOL_DEFINITION(SIDE_MAP_INV, NUM_SIDES);
  }

  // 数値を駒の種類のシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToPiece) {
    TO_SYMBOL_DEFINITION(PIECE_MAP_INV, NUM_PIECE_TYPES);
  }

  // 数値をキャスリングのシンボルに変換する。
  DEF_LC_FUNCTION(Sayulisp::NumberToCastling) {
    TO_SYMBOL_DEFINITION(CASTLING_MAP_INV, 5);
  }


  DEF_LC_FUNCTION(Sayulisp::GenPGN) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // PGN文字列を得る。
    LPointer pgn_str_ptr = caller->Evaluate(args_ptr->car());
    CheckType(*pgn_str_ptr, LType::STRING);
    std::shared_ptr<PGN> pgn_ptr(new PGN());
    pgn_ptr->Parse(pgn_str_ptr->string());

    // 現在のゲームのインデックス。
    std::shared_ptr<int> current_index_ptr(new int(0));

    // メッセージシンボル用オブジェクトを作る。
    std::map<std::string, MessageFunction> message_func_map;

    message_func_map["@get-pgn-comments"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
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
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
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
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
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
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      return NewNumber(pgn_ptr->game_vec().size());
    };

    message_func_map["@set-current-game"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

      // インデックス番号を得る。
      LPointer index_ptr = caller->Evaluate(args_ptr->car());
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
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
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

    message_func_map["@get-current-game-result"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      // 現在のゲームの結果を得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();

      return NewString(pgn_ptr->game_vec()[*current_index_ptr]->result());
    };

    message_func_map["@current-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      // 現在の指し手を得る。
      if (pgn_ptr->game_vec().empty()) return NewNil();
      const MoveNode* node_ptr =
      pgn_ptr->game_vec()[*current_index_ptr]->current_node_ptr();

      if (node_ptr) {
        return NewString(node_ptr->text_);
      }

      return NewNil();
    };

    message_func_map["@next-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 次の手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Next()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewNil();
    };

    message_func_map["@prev-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 前の手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Back()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewNil();
    };

    message_func_map["@alt-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // 代替手へ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Alt()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewNil();
    };

    message_func_map["@orig-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // オリジナルへ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Orig()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewNil();
    };

    message_func_map["@rewind-move"] =
    [pgn_ptr, current_index_ptr](const std::string& symbol,
    const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      if (pgn_ptr->game_vec().empty()) return NewNil();

      // オリジナルへ。
      PGNGamePtr game_ptr = pgn_ptr->game_vec()[*current_index_ptr];
      if (game_ptr->Rewind()) {
        const MoveNode* node_ptr = game_ptr->current_node_ptr();
        if (node_ptr) {
          return NewString(node_ptr->text_);
        }
      }

      return NewNil();
    };

    // PGNオブジェクトを作成。
    auto pgn_func = [message_func_map]
    (const LObject& self, LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // メッセージシンボルを得る。
      LPointer symbol_ptr = caller->Evaluate(args_ptr->car());
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
    LPointer fen_ptr = caller->Evaluate(args_ptr->car());
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
    LPointer position_ptr = caller->Evaluate(args_ptr->car());
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
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, PAWN>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-knight-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, KNIGHT>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-bishop-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, BISHOP>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-rook-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, ROOK>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-queen-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, QUEEN>(symbol, self, caller, args);
    };

    message_func_map_["@get-white-king-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<WHITE, KING>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-pawn-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, PAWN>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-knight-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, KNIGHT>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-bishop-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, BISHOP>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-rook-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, ROOK>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-queen-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
    const LObject& args) -> LPointer {
      return this->GetPosition<BLACK, QUEEN>(symbol, self, caller, args);
    };

    message_func_map_["@get-black-king-position"] =
    [this](const std::string& symbol, const LObject& self, LObject* caller,
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

    message_func_map_["@set-white-has-castled"] =
    INSERT_MESSAGE_FUNCTION(SetHasCastled<WHITE>);

    message_func_map_["@set-black-has-castled"] =
    INSERT_MESSAGE_FUNCTION(SetHasCastled<BLACK>);

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

    message_func_map_["@note->move"] =
    INSERT_MESSAGE_FUNCTION(NoteToMove);

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

    message_func_map_["@clear-hash"] =
    INSERT_MESSAGE_FUNCTION(ClearHash);

    message_func_map_["@set-threads"] =
    INSERT_MESSAGE_FUNCTION(SetThreads);

    message_func_map_["@analyse-diff"] =
    INSERT_MESSAGE_FUNCTION(AnalyseDiff);

    message_func_map_["@analyse-mobility"] =
    INSERT_MESSAGE_FUNCTION(AnalyseMobility);

    message_func_map_["@analyse-attackers"] =
    INSERT_MESSAGE_FUNCTION(AnalyseAttackers);

    message_func_map_["@analyse-attacking"] =
    INSERT_MESSAGE_FUNCTION(AnalyseAttacking);

    message_func_map_["@analyse-attacked"] =
    INSERT_MESSAGE_FUNCTION(AnalyseAttacked);

    message_func_map_["@analyse-defensing"] =
    INSERT_MESSAGE_FUNCTION(AnalyseDefensing);

    message_func_map_["@analyse-defensed"] =
    INSERT_MESSAGE_FUNCTION(AnalyseDefensed);

    message_func_map_["@analyse-center-control"] =
    INSERT_MESSAGE_FUNCTION(AnalyseCenterControl);

    message_func_map_["@analyse-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(AnalyseSweetCenterControl);

    message_func_map_["@analyse-open-fyle"] =
    INSERT_MESSAGE_FUNCTION(AnalyseOpenFyle);

    message_func_map_["@analyse-development"] =
    INSERT_MESSAGE_FUNCTION(AnalyseDevelopment);

    message_func_map_["@analyse-double-pawn"] =
    INSERT_MESSAGE_FUNCTION(AnalyseDoublePawn);

    message_func_map_["@analyse-iso-pawn"] =
    INSERT_MESSAGE_FUNCTION(AnalyseIsoPawn);

    message_func_map_["@analyse-pass-pawn"] =
    INSERT_MESSAGE_FUNCTION(AnalysePassPawn);

    message_func_map_["@analyse-pin/skewer"] =
    INSERT_MESSAGE_FUNCTION(AnalysePinSkewer);

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

    message_func_map_["@history-pruning-invalid-moves"] =
    INSERT_MESSAGE_FUNCTION(SetHistoryPruningInvalidMoves);

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

    message_func_map_["@weight-pawn-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<PAWN>);
    message_func_map_["@weight-knight-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<KNIGHT>);
    message_func_map_["@weight-bishop-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<BISHOP>);
    message_func_map_["@weight-rook-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<ROOK>);
    message_func_map_["@weight-queen-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<QUEEN>);
    message_func_map_["@weight-king-opening-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightOpeningPosition<KING>);

    message_func_map_["@weight-pawn-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<PAWN>);
    message_func_map_["@weight-knight-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<KNIGHT>);
    message_func_map_["@weight-bishop-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<BISHOP>);
    message_func_map_["@weight-rook-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<ROOK>);
    message_func_map_["@weight-queen-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<QUEEN>);
    message_func_map_["@weight-king-ending-position"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEndingPosition<KING>);

    message_func_map_["@weight-pawn-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<PAWN>);
    message_func_map_["@weight-knight-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<KNIGHT>);
    message_func_map_["@weight-bishop-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<BISHOP>);
    message_func_map_["@weight-rook-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<ROOK>);
    message_func_map_["@weight-queen-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<QUEEN>);
    message_func_map_["@weight-king-mobility"] =
    INSERT_MESSAGE_FUNCTION(SetWeightMobility<KING>);

    message_func_map_["@weight-pawn-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<PAWN>);
    message_func_map_["@weight-knight-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<KNIGHT>);
    message_func_map_["@weight-bishop-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<BISHOP>);
    message_func_map_["@weight-rook-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<ROOK>);
    message_func_map_["@weight-queen-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<QUEEN>);
    message_func_map_["@weight-king-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCenterControll<KING>);

    message_func_map_["@weight-pawn-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<PAWN>);
    message_func_map_["@weight-knight-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<KNIGHT>);
    message_func_map_["@weight-bishop-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<BISHOP>);
    message_func_map_["@weight-rook-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<ROOK>);
    message_func_map_["@weight-queen-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<QUEEN>);
    message_func_map_["@weight-king-sweet-center-control"] =
    INSERT_MESSAGE_FUNCTION(SetWeightSweetCenterControll<KING>);

    message_func_map_["@weight-pawn-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<PAWN>);
    message_func_map_["@weight-knight-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<KNIGHT>);
    message_func_map_["@weight-bishop-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<BISHOP>);
    message_func_map_["@weight-rook-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<ROOK>);
    message_func_map_["@weight-queen-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<QUEEN>);
    message_func_map_["@weight-king-development"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDevelopment<KING>);

    message_func_map_["@weight-pawn-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<PAWN>);
    message_func_map_["@weight-knight-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<KNIGHT>);
    message_func_map_["@weight-bishop-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<BISHOP>);
    message_func_map_["@weight-rook-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<ROOK>);
    message_func_map_["@weight-queen-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<QUEEN>);
    message_func_map_["@weight-king-attack"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttack<KING>);

    message_func_map_["@weight-pawn-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<PAWN>);
    message_func_map_["@weight-knight-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<KNIGHT>);
    message_func_map_["@weight-bishop-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<BISHOP>);
    message_func_map_["@weight-rook-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<ROOK>);
    message_func_map_["@weight-queen-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<QUEEN>);
    message_func_map_["@weight-king-defense"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDefense<KING>);

    message_func_map_["@weight-bishop-pin"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPin<BISHOP>);
    message_func_map_["@weight-rook-pin"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPin<ROOK>);
    message_func_map_["@weight-queen-pin"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPin<QUEEN>);

    message_func_map_["@weight-pawn-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<PAWN>);
    message_func_map_["@weight-knight-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<KNIGHT>);
    message_func_map_["@weight-bishop-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<BISHOP>);
    message_func_map_["@weight-rook-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<ROOK>);
    message_func_map_["@weight-queen-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<QUEEN>);
    message_func_map_["@weight-king-attack-around-king"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAttackAroundKing<KING>);

    message_func_map_["@weight-pass-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPassPawn);

    message_func_map_["@weight-protected-pass-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightProtectedPassPawn);

    message_func_map_["@weight-double-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightDoublePawn);

    message_func_map_["@weight-iso-pawn"] =
    INSERT_MESSAGE_FUNCTION(SetWeightIsoPawn);

    message_func_map_["@weight-pawn-shield"] =
    INSERT_MESSAGE_FUNCTION(SetWeightPawnShield);

    message_func_map_["@weight-bishop-pair"] =
    INSERT_MESSAGE_FUNCTION(SetWeightBishopPair);

    message_func_map_["@weight-bad-bishop"] =
    INSERT_MESSAGE_FUNCTION(SetWeightBadBishop);

    message_func_map_["@weight-rook-pair"] =
    INSERT_MESSAGE_FUNCTION(SetWeightRookPair);

    message_func_map_["@weight-rook-semiopen-fyle"] =
    INSERT_MESSAGE_FUNCTION(SetWeightRookSemiopenFyle);

    message_func_map_["@weight-rook-open-fyle"] =
    INSERT_MESSAGE_FUNCTION(SetWeightRookOpenFyle);

    message_func_map_["@weight-early-queen-starting"] =
    INSERT_MESSAGE_FUNCTION(SetWeightEarlyQueenStarting);

    message_func_map_["@weight-weak-square"] =
    INSERT_MESSAGE_FUNCTION(SetWeightWeakSquare);

    message_func_map_["@weight-castling"] =
    INSERT_MESSAGE_FUNCTION(SetWeightCastling);

    message_func_map_["@weight-abandoned-castling"] =
    INSERT_MESSAGE_FUNCTION(SetWeightAbandonedCastling);

    message_func_map_["@get-feature-vector"] =
    INSERT_MESSAGE_FUNCTION(GetFeatureVector);
  }

  // 関数オブジェクト。
  DEF_LC_FUNCTION(EngineSuite::operator()) {
    // 準備。
    LObject* args_ptr = nullptr;
    Lisp::GetReadyForFunction(args, 1, &args_ptr);

    // メッセージシンボルを抽出。
    LPointer result = caller->Evaluate(args_ptr->car());
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
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, KNIGHT>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, BISHOP>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, ROOK>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, QUEEN>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<WHITE, KING>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, PAWN>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, KNIGHT>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, BISHOP>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, ROOK>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, QUEEN>
  (const std::string&, const LObject&, LObject*, const LObject&);
  template LPointer EngineSuite::GetPosition<BLACK, KING>
  (const std::string&, const LObject&, LObject*, const LObject&);

  // %%% @get-piece
  DEF_MESSAGE_FUNCTION(EngineSuite::GetPiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 引数のチェック。
    LPointer result = caller->Evaluate(args_ptr->car());
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
    LPointer fen_ptr = caller->Evaluate(args_ptr->car());
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
    table_ptr_->Clear();
    return Lisp::NewBoolean(true);
  }

  // %%% @place-piece
  DEF_MESSAGE_FUNCTION(EngineSuite::PlacePiece) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 2, &args_ptr);

    // 駒を得る。
    LPointer piece_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckPiece(*piece_ptr);
    Side side = piece_ptr->car()->number();
    PieceType piece_type = piece_ptr->cdr()->car()->number();

    Lisp::Next(&args_ptr);

    // マスを得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);
    Square square = square_ptr->number();

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
    LPointer to_move_ptr = caller->Evaluate(args_ptr->car());
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
    LPointer castling_list_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckList(*castling_list_ptr);

    // キャスリングの権利フラグを作成。
    Castling rights = 0;
    LPointer result;
    int rights_number = 0;
    for (LObject* ptr = castling_list_ptr.get(); ptr->IsPair();
    Lisp::Next(&ptr)) {
      result = caller->Evaluate(ptr->car());
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
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
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
    LPointer ply_ptr = caller->Evaluate(args_ptr->car());
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
    LPointer clock_ptr = caller->Evaluate(args_ptr->car());
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

  // %%% @set-white-has-castled
  // %%% @set-black-has-castled
  template<Side SIDE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetHasCastled) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 真偽値を得る。
    LPointer has_castled_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*has_castled_ptr, LType::BOOLEAN);
    bool has_castled = has_castled_ptr->boolean();

    LPointer ret_ptr = Lisp::NewBoolean(board_ptr_->has_castled_[SIDE]);
    engine_ptr_->has_castled(SIDE, has_castled);
    return ret_ptr;
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetHasCastled<WHITE>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetHasCastled<BLACK>);

  // %%% @play-move
  // %%% @play-note
  /** 手を指す。 */
  DEF_MESSAGE_FUNCTION(EngineSuite::PlayMoveOrNote) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 指し手を得る。
    LPointer result = caller->Evaluate(args_ptr->car());
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
    LPointer move_ptr = caller->Evaluate(args_ptr->car());

    return Lisp::NewString(engine_ptr_->MoveToNote
    (Sayulisp::ListToMove(*move_ptr)));
  }

  // %%% @note->move
  DEF_MESSAGE_FUNCTION(EngineSuite::NoteToMove) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // PGNの指し手を得る。
    LPointer note_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*note_ptr, LType::STRING);

    std::vector<Move> move_vec = engine_ptr_->GuessNote(note_ptr->string());
    if (move_vec.size() > 0) {
      return Sayulisp::MoveToList(move_vec[0]);
    }
    return Lisp::NewNil();
  }

  // %%% @input-uci-command
  DEF_MESSAGE_FUNCTION(EngineSuite::InputUCICommand) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    LPointer command_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*command_ptr, LType::STRING);

    return Lisp::NewBoolean(shell_ptr_->InputCommand(command_ptr->string()));
  }

  // %%% @add-uci-output-listener
  DEF_MESSAGE_FUNCTION(EngineSuite::AddUCIOutputListener) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    LPointer result = caller->Evaluate(args_ptr->car())->Clone();
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
      caller_scope->Evaluate(listener_ptr);
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
    // スコア。
    ptr->car(Lisp::NewNumber(pv_line.score()));
    Lisp::Next(&ptr);

    // メイトイン。
    int mate_in = pv_line.mate_in();
    if (mate_in >= 0) {
      if ((mate_in % 2) == 1) {
        mate_in = (mate_in / 2) + 1;
      } else {
        mate_in = -1 * (mate_in / 2);
      }
      ptr->car(Lisp::NewNumber(mate_in));
    } else {
      ptr->car(Lisp::NewNil());
    }
    Lisp::Next(&ptr);

    // PVライン。
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
    LPointer time_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*time_ptr, LType::NUMBER);
    int time = time_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(args_ptr->car());
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
    LPointer time_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*time_ptr, LType::NUMBER);
    int time = TimeLimitToMoveTime(time_ptr->number());
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(args_ptr->car());
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
    LPointer depth_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*depth_ptr, LType::NUMBER);
    std::uint32_t depth = depth_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(args_ptr->car());
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
    LPointer node_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*node_ptr, LType::NUMBER);
    std::uint64_t node = node_ptr->number();
    Lisp::Next(&args_ptr);

    // もしあるなら、候補手のリストを得る。
    LPointer candidate_list_ptr = Lisp::NewNil();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(args_ptr->car());
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
    LPointer size_ptr = caller->Evaluate(args_ptr->car());
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
    LPointer threads_ptr = caller->Evaluate(args_ptr->car());
    Lisp::CheckType(*threads_ptr, LType::NUMBER);
    int threads = Util::GetMax(threads_ptr->number(), 1);

    // 古いスレッド数。
    LPointer ret_ptr = Lisp::NewNumber(shell_ptr_->num_threads());

    // スレッド数を更新。
    shell_ptr_->num_threads(threads);

    return ret_ptr;
  }

  // %%% @analyse-diff
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseDiff) {
    LPointer ret_ptr = Lisp::NewList(NUM_PIECE_TYPES);
    LObject* ptr = ret_ptr.get();

    // EMPTY。
    ptr->car(Lisp::NewNumber(0));
    Lisp::Next(&ptr);
    for (PieceType piece_type = PAWN; ptr->IsPair();
    Lisp::Next(&ptr), ++piece_type) {
      ptr->car(Lisp::NewNumber(Sayuri::AnalyseDiff(*board_ptr_, piece_type)));
    }

    return ret_ptr;
  }

  // %%% @analyse-mobility
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseMobility) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒の位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseMobility(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-attackers
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseAttackers) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseAttackers(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-attacking
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseAttacking) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒の位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseAttacking(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-attacked
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseAttacked) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒の位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseAttacked(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-defensing
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseDefensing) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒の位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseDefensing(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-defensed
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseDefensed) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒の位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseDefensed(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-center-control
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseCenterControl) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒の位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseCenterControl(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-sweet-center-control
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseSweetCenterControl) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒の位置を得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 分析結果を得る。
    ResultSquares result =
    Sayuri::AnalyseSweetCenterControl(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-open-fyle
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseOpenFyle) {
    ResultFyles result = Sayuri::AnalyseOpenFyle(*board_ptr_);

    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();

    for (auto fyle : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::FYLE_MAP_INV[fyle]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-development
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseDevelopment) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // 駒を得る。
    LPointer piece_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckPiece(*piece_ptr);

    // 結果を得る。
    ResultSquares result =
    Sayuri::AnalyseDevelopment(*board_ptr_, piece_ptr->car()->number(),
    piece_ptr->cdr()->car()->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-double-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseDoublePawn) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // サイドを得る。
    LPointer side_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSide(*side_ptr);

    // 結果を得る。
    ResultSquares result =
    Sayuri::AnalyseDoublePawn(*board_ptr_, side_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-iso-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalyseIsoPawn) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // サイドを得る。
    LPointer side_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSide(*side_ptr);

    // 結果を得る。
    ResultSquares result =
    Sayuri::AnalyseIsoPawn(*board_ptr_, side_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-pass-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalysePassPawn) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // サイドを得る。
    LPointer side_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSide(*side_ptr);

    // 結果を得る。
    ResultSquares result =
    Sayuri::AnalysePassPawn(*board_ptr_, side_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto square : result) {
      ptr->car(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[square]));
      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @analyse-pin/skewer
  DEF_MESSAGE_FUNCTION(EngineSuite::AnalysePinSkewer) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // マスを得る。
    LPointer square_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSquare(*square_ptr);

    // 結果を得る。
    ResultPinSkewer result =
    Sayuri::AnalysePinSkewer(*board_ptr_, square_ptr->number());

    // 分析結果をリストにする。
    LPointer ret_ptr = Lisp::NewList(result.size());
    LObject* ptr = ret_ptr.get();
    for (auto array : result) {
      ptr->car(Lisp::NewPair(
      Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[array[0]]),
      Lisp::NewPair(Lisp::NewSymbol(Sayulisp::SQUARE_MAP_INV[array[1]]),
      Lisp::NewNil())));

      Lisp::Next(&ptr);
    }

    return ret_ptr;
  }

  // %%% @material
  DEF_MESSAGE_FUNCTION(EngineSuite::SetMaterial) {
    // 古い設定を得る。
    const int (& material)[NUM_PIECE_TYPES] = search_params_ptr_->material();
    LPointerVec ret_vec(NUM_PIECE_TYPES);
    FOR_PIECE_TYPES(piece_type) {
      ret_vec[piece_type] = Lisp::NewNumber(material[piece_type]);
    }

    // もし引数があるなら設定。
    LObject* args_ptr = args.cdr()->cdr().get();
    if (args_ptr->IsPair()) {
      LPointer result = caller->Evaluate(args_ptr->car());
      Lisp::CheckList(*result);

      int len = Lisp::CountList(*result);
      if (len < static_cast<int>(NUM_PIECE_TYPES)) {
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
      LPointer result = caller->Evaluate(args_ptr->car());
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
      LPointer result = caller->Evaluate(args_ptr->car());
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
      LPointer result = caller->Evaluate(args_ptr->car());
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
      LPointer result = caller->Evaluate(args_ptr->car());
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
      LPointer result = caller->Evaluate(args_ptr->car());
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
      LPointer result = caller->Evaluate(args_ptr->car());
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

#define SET_PIECE_WEIGHT(accessor) \
    const Weight& old = eval_params_ptr_->accessor()[TYPE];\
    std::size_t len = old.Size();\
    LPointerVec ret_vec(len);\
    for (std::size_t i = 0; i < len; ++i) {\
      ret_vec[i] = Lisp::NewPair(Lisp::NewNumber(std::get<0>(old.At(i))),\
      Lisp::NewPair(Lisp::NewNumber(std::get<1>(old.At(i))), Lisp::NewNil()));\
    }\
    \
    LObject* args_ptr = args.cdr()->cdr().get();\
    if (args_ptr->IsPair()) {\
      LPointer car = caller->Evaluate(args_ptr->car());\
      \
      if (car->IsNumber()) {\
        const LPointer& cdr = args_ptr->cdr();\
        if (cdr->IsPair()) {\
          LPointer cdar = caller->Evaluate(cdr->car());\
          Lisp::CheckType(*cdar, LType::NUMBER);\
          \
          eval_params_ptr_->accessor\
          (TYPE, Weight::CreateWeight(car->number(), cdar->number()));\
          \
          return Lisp::LPointerVecToList(ret_vec);\
        }\
      } else if (car->IsList()) {\
        Weight weight;\
        for (LObject* ptr = car.get(); ptr->IsPair(); Lisp::Next(&ptr)) {\
          const LPointer& temp = ptr->car();\
          \
          Lisp::CheckList(*temp);\
          if (Lisp::CountList(*temp) < 2) {\
            throw Lisp::GenError("@engine-error", "If you want to set weight, "\
            "the arguments are 2 numbers or List of Lists composed with 2 "\
            "numbers.");\
          }\
          \
          const LPointer& temp_car = temp->car();\
          Lisp::CheckType(*temp_car, LType::NUMBER);\
          const LPointer& temp_cdar = temp->cdr()->car();\
          Lisp::CheckType(*temp_cdar, LType::NUMBER);\
          \
          weight.Add(temp_car->number(), temp_cdar->number());\
        }\
        \
        eval_params_ptr_->accessor(TYPE, weight);\
        \
        return Lisp::LPointerVecToList(ret_vec);\
      }\
      \
      throw Lisp::GenError("@engine-error", "If you want to set weight, "\
      "the arguments are 2 numbers or List of Lists composed with 2 numbers.");\
    }\
    \
    return Lisp::LPointerVecToList(ret_vec)

  // %%% @weight-pawn-opening-position
  // %%% @weight-knight-opening-position
  // %%% @weight-bishop-opening-position
  // %%% @weight-rook-opening-position
  // %%% @weight-queen-opening-position
  // %%% @weight-king-opening-position
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition) {
    SET_PIECE_WEIGHT(weight_opening_position);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightOpeningPosition<KING>);

  // %%% @weight-pawn-ending-position
  // %%% @weight-knight-ending-position
  // %%% @weight-bishop-ending-position
  // %%% @weight-rook-ending-position
  // %%% @weight-queen-ending-position
  // %%% @weight-king-ending-position
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition) {
    SET_PIECE_WEIGHT(weight_ending_position);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEndingPosition<KING>);

  // %%% @weight-pawn-mobility
  // %%% @weight-knight-mobility
  // %%% @weight-bishop-mobility
  // %%% @weight-rook-mobility
  // %%% @weight-queen-mobility
  // %%% @weight-king-mobility
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility) {
    SET_PIECE_WEIGHT(weight_mobility);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightMobility<KING>);


  // %%% @weight-pawn-center-control
  // %%% @weight-knight-center-control
  // %%% @weight-bishop-center-control
  // %%% @weight-rook-center-control
  // %%% @weight-queen-center-control
  // %%% @weight-king-center-control
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll) {
    SET_PIECE_WEIGHT(weight_center_control);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCenterControll<KING>);

  // %%% @weight-pawn-sweet-center-control
  // %%% @weight-knight-sweet-center-control
  // %%% @weight-bishop-sweet-center-control
  // %%% @weight-rook-sweet-center-control
  // %%% @weight-queen-sweet-center-control
  // %%% @weight-king-sweet-center-control
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll) {
    SET_PIECE_WEIGHT(weight_sweet_center_control);
  }
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightSweetCenterControll<KING>);

  // %%% @weight-pawn-development
  // %%% @weight-knight-development
  // %%% @weight-bishop-development
  // %%% @weight-rook-development
  // %%% @weight-queen-development
  // %%% @weight-king-development
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment) {
    SET_PIECE_WEIGHT(weight_development);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDevelopment<KING>);

  // %%% @weight-pawn-attack
  // %%% @weight-knight-attack
  // %%% @weight-bishop-attack
  // %%% @weight-rook-attack
  // %%% @weight-queen-attack
  // %%% @weight-king-attack
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack) {
    SET_PIECE_WEIGHT(weight_attack);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttack<KING>);

  // %%% @weight-pawn-defense
  // %%% @weight-knight-defense
  // %%% @weight-bishop-defense
  // %%% @weight-rook-defense
  // %%% @weight-queen-defense
  // %%% @weight-king-defense
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense) {
    SET_PIECE_WEIGHT(weight_defense);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<PAWN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<KNIGHT>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<QUEEN>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDefense<KING>);

  // %%% @weight-bishop-pin
  // %%% @weight-rook-pin
  // %%% @weight-queen-pin
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin) {
    SET_PIECE_WEIGHT(weight_pin);
  }
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin<BISHOP>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin<ROOK>);
  template DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPin<QUEEN>);

  // %%% @weight-pawn-attack-around-king
  // %%% @weight-knight-attack-around-king
  // %%% @weight-bishop-attack-around-king
  // %%% @weight-rook-attack-around-king
  // %%% @weight-queen-attack-around-king
  // %%% @weight-king-attack-around-king
  template<PieceType TYPE>
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing) {
    SET_PIECE_WEIGHT(weight_attack_around_king);
  }
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<PAWN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<KNIGHT>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<BISHOP>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<ROOK>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<QUEEN>);
  template
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAttackAroundKing<KING>);

#define SET_WEIGHT(accessor) \
    const Weight& old = eval_params_ptr_->accessor();\
    std::size_t len = old.Size();\
    LPointerVec ret_vec(len);\
    for (std::size_t i = 0; i < len; ++i) {\
      ret_vec[i] = Lisp::NewPair(Lisp::NewNumber(std::get<0>(old.At(i))),\
      Lisp::NewPair(Lisp::NewNumber(std::get<1>(old.At(i))), Lisp::NewNil()));\
    }\
    \
    LObject* args_ptr = args.cdr()->cdr().get();\
    if (args_ptr->IsPair()) {\
      LPointer car = caller->Evaluate(args_ptr->car());\
      \
      if (car->IsNumber()) {\
        const LPointer& cdr = args_ptr->cdr();\
        if (cdr->IsPair()) {\
          LPointer cdar = caller->Evaluate(cdr->car());\
          Lisp::CheckType(*cdar, LType::NUMBER);\
          \
          eval_params_ptr_->accessor\
          (Weight::CreateWeight(car->number(), cdar->number()));\
          \
          return Lisp::LPointerVecToList(ret_vec);\
        }\
      } else if (car->IsList()) {\
        Weight weight;\
        for (LObject* ptr = car.get(); ptr->IsPair(); Lisp::Next(&ptr)) {\
          const LPointer& temp = ptr->car();\
          \
          Lisp::CheckList(*temp);\
          if (Lisp::CountList(*temp) < 2) {\
            throw Lisp::GenError("@engine-error", "If you want to set weight, "\
            "the arguments are 2 numbers or List of Lists composed with 2 "\
            "numbers.");\
          }\
          \
          const LPointer& temp_car = temp->car();\
          Lisp::CheckType(*temp_car, LType::NUMBER);\
          const LPointer& temp_cdar = temp->cdr()->car();\
          Lisp::CheckType(*temp_cdar, LType::NUMBER);\
          \
          weight.Add(temp_car->number(), temp_cdar->number());\
        }\
        \
        eval_params_ptr_->accessor(weight);\
        \
        return Lisp::LPointerVecToList(ret_vec);\
      }\
      \
      throw Lisp::GenError("@engine-error", "If you want to set weight, "\
      "the arguments are 2 numbers or List of Lists composed with 2 numbers.");\
    }\
    \
    return Lisp::LPointerVecToList(ret_vec)

  // %%% @weight-pass-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPassPawn) {
    SET_WEIGHT(weight_pass_pawn);
  }

  // %%% @weight-protected-pass-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightProtectedPassPawn) {
    SET_WEIGHT(weight_protected_pass_pawn);
  }

  // %%% @weight-double-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightDoublePawn) {
    SET_WEIGHT(weight_double_pawn);
  }

  // %%% @weight-iso-pawn
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightIsoPawn) {
    SET_WEIGHT(weight_iso_pawn);
  }

  // %%% @weight-pawn-shield
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightPawnShield) {
    SET_WEIGHT(weight_pawn_shield);
  }

  // %%% @weight-bishop-pair
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightBishopPair) {
    SET_WEIGHT(weight_bishop_pair);
  }

  // %%% @weight-bad-bishop
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightBadBishop) {
    SET_WEIGHT(weight_bad_bishop);
  }

  // %%% @weight-rook-pair
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightRookPair) {
    SET_WEIGHT(weight_rook_pair);
  }

  // %%% @weight-rook-semiopen-fyle
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightRookSemiopenFyle) {
    SET_WEIGHT(weight_rook_semiopen_fyle);
  }

  // %%% @weight-rook-open-fyle
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightRookOpenFyle) {
    SET_WEIGHT(weight_rook_open_fyle);
  }

  // %%% @weight-early-queen-starting
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightEarlyQueenStarting) {
    SET_WEIGHT(weight_early_queen_starting);
  }

  // %%% @weight-weak-square
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightWeakSquare) {
    SET_WEIGHT(weight_weak_square);
  }

  // %%% @weight-castling
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightCastling) {
    SET_WEIGHT(weight_castling);
  }

  // %%% @weight-abandoned-castling
  DEF_MESSAGE_FUNCTION(EngineSuite::SetWeightAbandonedCastling) {
    SET_WEIGHT(weight_abandoned_castling);
  }

  // %%% @get-feature-vector
  DEF_MESSAGE_FUNCTION(EngineSuite::GetFeatureVector) {
    // 準備。
    LObject* args_ptr = nullptr;
    Sayulisp::GetReadyForMessageFunction(symbol, args, 1, &args_ptr);

    // サイドを得る。
    LPointer side_ptr = caller->Evaluate(args_ptr->car());
    Sayulisp::CheckSide(*side_ptr);

    // 特徴ベクトルを得る。
    std::vector<std::pair<std::string, std::vector<double>>>
    feature_vec = GenFeatureVec(side_ptr->number());

    // 特徴ベクトルのリストを作る。
    std::size_t len = feature_vec.size();
    LPointerVec ret_vec(len);
    for (std::size_t i = 0; i < len; ++i) {
      // 特徴名を作る。
      LPointer name = Lisp::NewString(feature_vec[i].first);

      // ベクトルを作る。
      std::size_t len_2 = feature_vec[i].second.size();
      LPointerVec vec(len_2);
      for (std::size_t j = 0; j < len_2; ++j) {
        vec[j] = Lisp::NewNumber(feature_vec[i].second[j]);
      }
      ret_vec[i] = Lisp::NewPair(name, Lisp::NewPair
      (Lisp::LPointerVecToList(vec), Lisp::NewNil()));
    }

    return Lisp::LPointerVecToList(ret_vec);
  }

  // ========================== //
  // 特徴・ウェイトベクトル関連 //
  // ========================== //
  // 現在の特徴ベクトルを作る。
  std::vector<std::pair<std::string, std::vector<double>>>
  EngineSuite::GenFeatureVec(Side side) const {
    std::vector<std::pair<std::string, std::vector<double>>> ret;

    // ボード。
    const Board& board = *board_ptr_;

    // 駒の名前配列。
    static const std::array<std::string, NUM_PIECE_TYPES> piece_str {{
      "", "pawn", "knight", "bishop", "rook", "queen", "king"
    }};

    // 駒の配置の配置の特徴ベクトル。
    {
      std::vector<double> temp(NUM_SQUARES);
      FOR_PIECE_TYPES(piece_type) {
        if (piece_type) {
          FOR_SQUARES(square) {
            double value = 0.0;
            // 先ずは白。
            if ((board.piece_board_[square] == piece_type)
            && (board.side_board_[square] == WHITE)) {
              value += 1.0;
            }
            // 次に黒。
            if ((board.piece_board_[Util::FLIP[square]] == piece_type)
            && (board.side_board_[Util::FLIP[square]] == BLACK)) {
              value -= 1.0;
            }
            temp[square] = value;
          }
          ret.push_back(std::make_pair(piece_str[piece_type] + "_position",
          temp));
        }
      }
    }

    // 攻撃の特徴ベクトル。
    FOR_PIECE_TYPES(piece_type) {
      if (piece_type) {
        std::vector<double> temp_2(6, 0.0);

        // まずは白から。
        Bitboard white_bb = board.position_[WHITE][piece_type];
        for (; white_bb; NEXT_BITBOARD(white_bb)) {
          Square square = Util::GetSquare(white_bb);

          // 攻撃しているマスを得る。
          ResultSquares result = Sayuri::AnalyseAttacking(board, square);

          // 攻撃しているマスにいる駒を調べてバリューをプラス。
          for (auto attacked : result) {
            temp_2[board.piece_board_[attacked] - 1] += 1.0;
          }
        }

        // 次に黒から。
        Bitboard black_bb = board.position_[BLACK][piece_type];
        for (; black_bb; NEXT_BITBOARD(black_bb)) {
          Square square = Util::GetSquare(black_bb);

          // 攻撃しているマスを得る。
          ResultSquares result = Sayuri::AnalyseAttacking(board, square);

          // 攻撃しているマスにいる駒を調べてバリューをマイナス。
          for (auto attacked : result) {
            temp_2[board.piece_board_[attacked] - 1] -= 1.0;
          }
        }

        // プッシュ。
        ret.push_back(std::make_pair(piece_str[piece_type] + "_attack",
        temp_2));
      }
    }

    // 防御の特徴ベクトル。
    FOR_PIECE_TYPES(piece_type) {
      if (piece_type) {
        std::vector<double> temp_2(6, 0.0);

        // まずは白から。
        Bitboard white_bb = board.position_[WHITE][piece_type];
        for (; white_bb; NEXT_BITBOARD(white_bb)) {
          Square square = Util::GetSquare(white_bb);

          // 攻撃しているマスを得る。
          ResultSquares result = Sayuri::AnalyseDefensing(board, square);

          // 攻撃しているマスにいる駒を調べてバリューをプラス。
          for (auto attacked : result) {
            temp_2[board.piece_board_[attacked] - 1] += 1.0;
          }
        }

        // 次に黒。
        Bitboard black_bb = board.position_[BLACK][piece_type];
        for (; black_bb; NEXT_BITBOARD(black_bb)) {
          Square square = Util::GetSquare(black_bb);

          // 攻撃しているマスを得る。
          ResultSquares result = Sayuri::AnalyseDefensing(board, square);

          // 攻撃しているマスにいる駒を調べてバリューをマイナス。
          for (auto attacked : result) {
            temp_2[board.piece_board_[attacked] - 1] -= 1.0;
          }
        }

        // プッシュ。
        ret.push_back(std::make_pair(piece_str[piece_type] + "_defense",
        temp_2));
      }
    }

    // ピンの特徴ベクトルを作る。
    {
      static const int piece_index[3] {BISHOP, ROOK, QUEEN};
      for (int i = 0; i < 3; ++i) {
        PieceType piece_type = piece_index[i];
        std::vector<double> temp(36, 0.0);

        // 先ずは白から。
        Bitboard white_bb = board.position_[WHITE][piece_type];
        for (; white_bb; NEXT_BITBOARD(white_bb)) {
          Square square = Util::GetSquare(white_bb);

          // ピンを分析。
          ResultPinSkewer result = Sayuri::AnalysePinSkewer(board, square);
          for (auto& ary : result) {
            PieceType pinned = board.piece_board_[ary[0]];
            PieceType pinback = board.piece_board_[ary[1]];
            temp[((pinned - 1) * 6) + (pinback - 1)] += 1.0;
          }
        }

        // 次に黒。
        Bitboard black_bb = board.position_[BLACK][piece_type];
        for (; black_bb; NEXT_BITBOARD(black_bb)) {
          Square square = Util::GetSquare(black_bb);

          // ピンを分析。
          ResultPinSkewer result = Sayuri::AnalysePinSkewer(board, square);
          for (auto& ary : result) {
            PieceType pinned = board.piece_board_[ary[0]];
            PieceType pinback = board.piece_board_[ary[1]];
            temp[((pinned - 1) * 6) + (pinback - 1)] -= 1.0;
          }
        }

        ret.push_back(std::make_pair(piece_str[piece_type] + "_pin", temp));
      }
    }

    // ポーンシールドの特徴ベクトルを作る。
    {
      // 各キングのファイルとランクを得る。
      Fyle white_king_fyle = Util::SquareToFyle(board.king_[WHITE]);
      Rank white_king_rank = Util::SquareToRank(board.king_[WHITE]);
      Fyle black_king_fyle = Util::SquareToFyle(board.king_[BLACK]);
      Rank black_king_rank = Util::SquareToRank(board.king_[BLACK]);
      // ビットボードのマスク。
      static const Bitboard KING_SIDE_BB =
      Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H];
      static const Bitboard QUEEN_SIDE_BB =
      Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C];
      // ベクトルを作る。
      std::vector<double> temp(NUM_SQUARES, 0.0);
      // 先ず白から。
      if (white_king_rank <= RANK_2) {
        Bitboard pawns = 0;
        if (white_king_fyle >= FYLE_F) {  // キングサイドにいる場合。
          // キングサイドにいるポーンのみマスク。
          pawns = board.position_[WHITE][PAWN] & KING_SIDE_BB;
        } else if (white_king_fyle <= FYLE_C) {  // クイーンサイドにいる場合。
          pawns = board.position_[WHITE][PAWN] & QUEEN_SIDE_BB;
        }

        // ベクトルに記録していく。
        for (; pawns; NEXT_BITBOARD(pawns)) {
          Square square = Util::GetSquare(pawns);
          temp[square] += 1.0;
        }
      }
      // 次に黒。
      if (black_king_rank >= RANK_7) {
        Bitboard pawns = 0;
        if (black_king_fyle >= FYLE_F) {  // キングサイドにいる場合。
          // キングサイドにいるポーンのみマスク。
          pawns = board.position_[BLACK][PAWN] & KING_SIDE_BB;
        } else if (black_king_fyle <= FYLE_C) {  // クイーンサイドにいる場合。
          pawns = board.position_[BLACK][PAWN] & QUEEN_SIDE_BB;
        }

        // ベクトルに記録していく。
        for (; pawns; NEXT_BITBOARD(pawns)) {
          Square square = Util::FLIP[Util::GetSquare(pawns)];
          temp[square] -= 1.0;
        }
      }
      ret.push_back(std::make_pair("pawn_shield", temp));
    }

    // モビリティ、センターコントロール、スイートセンターコントロール
    // の特徴ベクトルを作る。
    {
      std::vector<double> mobility_temp(6, 0.0);
      std::vector<double> center_temp(6, 0.0);
      std::vector<double> sweet_center_temp(6, 0.0);
      auto is_center = [](Square square) -> bool {
        Fyle fyle = Util::SquareToFyle(square);
        Rank rank = Util::SquareToRank(square);
        if ((fyle >= FYLE_C) && (fyle <= FYLE_F)
        && (rank >= RANK_3) && (rank <= RANK_6)) {
          return true;
        }
        return false;
      };
      auto is_sweet_center = [](Square square) -> bool {
        if ((square == D4) || (square == D5)
        || (square == E4) || (square == E5)) {
          return true;
        }
        return false;
      };
      FOR_SQUARES(square) {
        // サイドと駒。
        Side side = board.side_board_[square];
        PieceType piece_type = board.piece_board_[square];
        if (piece_type) {
          ResultSquares result = Sayuri::AnalyseMobility(board, square);
          if (side == WHITE) {
            // モビリティ。
            mobility_temp[piece_type - 1] += result.size();

            for (auto target : result) {
              // センターコントロール。
              if (is_center(target)) {
                center_temp[piece_type - 1] += 1.0;
              }
              // スウィートセンターコントロール。
              if (is_sweet_center(target)) {
                sweet_center_temp[piece_type - 1] += 1.0;
              }
            }
          } else if (side == BLACK) {
            // モビリティ。
            mobility_temp[piece_type - 1] -= result.size();

            for (auto target : result) {
              // センターコントロール。
              if (is_center(target)) {
                center_temp[piece_type - 1] -= 1.0;
              }
              // スウィートセンターコントロール。
              if (is_sweet_center(target)) {
                sweet_center_temp[piece_type - 1] -= 1.0;
              }
            }
          }
        }
      }
      ret.push_back(std::make_pair("mobility", mobility_temp));
      ret.push_back(std::make_pair("center_control", center_temp));
      ret.push_back(std::make_pair("sweet_center_control", sweet_center_temp));
    }

    // コマの展開の特徴ベクトルを作る。
    {
      std::vector<double> temp(6, 0.0);
      FOR_PIECE_TYPES(piece_type) {
        double value = 0.0;
        if (piece_type) {
          // 先ずは白。
          value += Sayuri::AnalyseDevelopment(board, WHITE, piece_type).size();

          // 次に黒。
          value -= Sayuri::AnalyseDevelopment(board, BLACK, piece_type).size();

          // 登録。
          temp[piece_type - 1] = value;
        }
      }
      ret.push_back(std::make_pair("development", temp));
    }

    // 敵キング周辺への攻撃の特徴ベクトルを作る。
    {
      std::vector<double> temp(6, 0.0);
      // 先ずは白。
      Bitboard around_black_king = Util::KING_MOVE[board.king_[BLACK]];
      for (; around_black_king; NEXT_BITBOARD(around_black_king)) {
        Square target = Util::GetSquare(around_black_king);
        ResultSquares result = Sayuri::AnalyseAttackers(board, target);

        for (auto& attacker : result) {
          if (board.side_board_[attacker] == WHITE) {
            temp[board.piece_board_[attacker] - 1] += 1.0;
          }
        }
      }

      // 次に黒。
      Bitboard around_white_king = Util::KING_MOVE[board.king_[WHITE]];
      for (; around_white_king; NEXT_BITBOARD(around_white_king)) {
        Square target = Util::GetSquare(around_white_king);
        ResultSquares result = Sayuri::AnalyseAttackers(board, target);

        for (auto& attacker : result) {
          if (board.side_board_[attacker] == BLACK) {
            temp[board.piece_board_[attacker] - 1] -= 1.0;
          }
        }
      }

      ret.push_back(std::make_pair("attack_around_king", temp));
    }

    // パスポーンと守られたパスポーンの特徴ベクトルを作る。
    {
      double pass_value = 0.0;
      double protected_pass_value = 0.0;

      // 先ずは白。
      ResultSquares result = Sayuri::AnalysePassPawn(board, WHITE);
      pass_value += result.size();
      for (auto pawn : result) {
        // ポーンに守られているかどうかを調べる。
        ResultSquares defenders = Sayuri::AnalyseDefensed(board, pawn);

        // ディフェンダーがポーンなら守られたパスポーン。
        for (auto defender : defenders) {
          if (board.piece_board_[defender] == PAWN) {
            protected_pass_value += 1.0;
            break;
          }
        }
      }

      // 次に黒。
      result = Sayuri::AnalysePassPawn(board, BLACK);
      pass_value -= result.size();
      for (auto pawn : result) {
        // ポーンに守られているかどうかを調べる。
        ResultSquares defenders = Sayuri::AnalyseDefensed(board, pawn);

        // ディフェンダーがポーンなら守られたパスポーン。
        for (auto defender : defenders) {
          if (board.piece_board_[defender] == PAWN) {
            protected_pass_value -= 1.0;
            break;
          }
        }
      }

      // プッシュ。
      ret.push_back(std::make_pair("pass_pawn",
      std::vector<double> {pass_value}));
      ret.push_back(std::make_pair("protected_pass_pawn",
      std::vector<double> {protected_pass_value}));
    }

    // ダブルポーンの特徴ベクトルを作る。
    {
      double value = Sayuri::AnalyseDoublePawn(board, WHITE).size();
      value -= Sayuri::AnalyseDoublePawn(board, BLACK).size();
      ret.push_back(std::make_pair("double_pawn",
      std::vector<double> {value}));
    }

    // 孤立ポーンの特徴ベクトルを作る。
    {
      double value = Sayuri::AnalyseIsoPawn(board, WHITE).size();
      value -= Sayuri::AnalyseIsoPawn(board, BLACK).size();
      ret.push_back(std::make_pair("iso_pawn",
      std::vector<double> {value}));
    }

    // ビショップペアの特徴ベクトルを作る。
    {
      double value = 0.0;
      if ((board.position_[WHITE][BISHOP] & Util::SQCOLOR[WHITE])
      && (board.position_[WHITE][BISHOP] & Util::SQCOLOR[BLACK])) {
        value += 1.0;
      }
      if ((board.position_[BLACK][BISHOP] & Util::SQCOLOR[WHITE])
      && (board.position_[BLACK][BISHOP] & Util::SQCOLOR[BLACK])) {
        value -= 1.0;
      }
      ret.push_back(std::make_pair("bishop_pair",
      std::vector<double> {value}));
    }

    // バッドビショップの特徴ベクトルを作る。
    {
      double value = 0.0;
      if ((board.position_[WHITE][BISHOP] & Util::SQCOLOR[WHITE])) {
        value +=
        Util::CountBits(board.position_[WHITE][PAWN] & Util::SQCOLOR[WHITE]);
      }
      if ((board.position_[WHITE][BISHOP] & Util::SQCOLOR[BLACK])) {
        value +=
        Util::CountBits(board.position_[WHITE][PAWN] & Util::SQCOLOR[BLACK]);
      }
      if ((board.position_[BLACK][BISHOP] & Util::SQCOLOR[WHITE])) {
        value -=
        Util::CountBits(board.position_[BLACK][PAWN] & Util::SQCOLOR[WHITE]);
      }
      if ((board.position_[BLACK][BISHOP] & Util::SQCOLOR[BLACK])) {
        value -=
        Util::CountBits(board.position_[BLACK][PAWN] & Util::SQCOLOR[BLACK]);
      }

      ret.push_back(std::make_pair("bad_bishop", std::vector<double> {value}));
    }

    // ルークペアの特徴ベクトルを作る。
    {
      double value = 0.0;
      if (Util::CountBits(board.position_[WHITE][ROOK]) >= 2) value += 1.0;
      if (Util::CountBits(board.position_[BLACK][ROOK]) >= 2) value -= 1.0;
      ret.push_back(std::make_pair("rook_pair", std::vector<double> {value}));
    }

    // ルークのセミオープンファイルオープンファイルの特徴ベクトルを作る。
    {
      double semiopen_value = 0.0;
      double open_value = 0.0;

      // 先ずは白。
      for (Bitboard bb = board.position_[WHITE][ROOK]; bb; NEXT_BITBOARD(bb)) {
        Fyle rook_fyle = Util::SquareToFyle(Util::GetSquare(bb));
        if (!(board.position_[WHITE][PAWN] & Util::FYLE[rook_fyle])) {
          semiopen_value += 1.0;
          if (!(board.position_[BLACK][PAWN] & Util::FYLE[rook_fyle])) {
            open_value += 1.0;
          }
        }
      }

      // 次に黒。
      for (Bitboard bb = board.position_[BLACK][ROOK]; bb; NEXT_BITBOARD(bb)) {
        Fyle rook_fyle = Util::SquareToFyle(Util::GetSquare(bb));
        if (!(board.position_[BLACK][PAWN] & Util::FYLE[rook_fyle])) {
          semiopen_value -= 1.0;
          if (!(board.position_[WHITE][PAWN] & Util::FYLE[rook_fyle])) {
            open_value -= 1.0;
          }
        }
      }

      ret.push_back(std::make_pair("rook_semiopen_fyle",
      std::vector<double> {semiopen_value}));
      ret.push_back(std::make_pair("rook_open_fyle",
      std::vector<double> {open_value}));
    }

    // 早すぎるクイーンの出動の特徴ベクトルを作る。
    {
      double value = 0.0;
      // 先ずは白。
      if (!(board.position_[WHITE][QUEEN] & Util::SQUARE[D1][R0])) {
        value += Util::CountBits(board.position_[WHITE][KNIGHT]
        & (Util::SQUARE[B1][R0] | Util::SQUARE[G1][R0]))
        + Util::CountBits(board.position_[WHITE][BISHOP]
        & (Util::SQUARE[C1][R0] | Util::SQUARE[F1][R0]));
      }

      // 次に黒。
      if (!(board.position_[BLACK][QUEEN] & Util::SQUARE[D8][R0])) {
        value -= Util::CountBits(board.position_[BLACK][KNIGHT]
        & (Util::SQUARE[B8][R0] | Util::SQUARE[G8][R0]))
        + Util::CountBits(board.position_[BLACK][BISHOP]
        & (Util::SQUARE[C8][R0] | Util::SQUARE[F8][R0]));
      }

      ret.push_back(std::make_pair("early_queen_starting",
      std::vector<double> {value}));
    }

    // 弱いマスの特徴ベクトルを作る。
    {
      double value = 0.0;

      // 黒の白マスビショップがいる時。
      if ((board.position_[BLACK][BISHOP] & Util::SQCOLOR[WHITE])) {
        value += Util::CountBits(~(board.position_[WHITE][PAWN])
        & Util::SQCOLOR[WHITE]);
      }

      // 黒の黒マスビショップがいる時。
      if ((board.position_[BLACK][BISHOP] & Util::SQCOLOR[BLACK])) {
        value += Util::CountBits(~(board.position_[WHITE][PAWN])
        & Util::SQCOLOR[BLACK]);
      }

      // 白の白マスビショップがいる時。
      if ((board.position_[WHITE][BISHOP] & Util::SQCOLOR[WHITE])) {
        value -= Util::CountBits(~(board.position_[BLACK][PAWN])
        & Util::SQCOLOR[WHITE]);
      }

      // 白の黒マスビショップがいる時。
      if ((board.position_[WHITE][BISHOP] & Util::SQCOLOR[BLACK])) {
        value -= Util::CountBits(~(board.position_[BLACK][PAWN])
        & Util::SQCOLOR[BLACK]);
      }

      ret.push_back(std::make_pair("weak_square",
      std::vector<double> {value}));
    }

    // キャスリングとキャスリングの放棄の特徴ベクトルを作る。
    {
      double castling_value = 0.0;
      double abandoned_value = 0.0;

      // 先ずは白。
      if (board.has_castled_[WHITE]) {
        castling_value += 1.0;
      } else if (!(board.castling_rights_ & WHITE_CASTLING)) {
        abandoned_value += 1.0;
      }

      // 次に黒。
      if (board.has_castled_[BLACK]) {
        castling_value -= 1.0;
      } else if (!(board.castling_rights_ & BLACK_CASTLING)) {
        abandoned_value -= 1.0;
      }

      ret.push_back(std::make_pair("castling",
      std::vector<double> {castling_value}));
      ret.push_back(std::make_pair("abandoned_castling",
      std::vector<double> {abandoned_value}));
    }

    if (side == BLACK) {
      for (auto& pair : ret) {
        for (auto& feature : pair.second) {
          feature *= -1.0;
        }
      }
    }
    return ret;
  }
}  // namespace Sayuri
