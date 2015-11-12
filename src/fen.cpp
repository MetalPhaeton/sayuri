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
 * @file fen.cpp
 * @author Hironori Ishibashi
 * @brief FEN文字列のパーサの実装。
 */

#include "fen.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <set>
#include <cstdlib>
#include <sstream>
#include <map>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  FEN::FEN(const std::string fen_str) :
  to_move_(WHITE),
  castling_rights_(ALL_CASTLING),
  en_passant_square_(0),
  clock_(0),
  ply_(0) {
    // 駒の配置を初期化。
    INIT_ARRAY(position_);

    try {
      // 構文木にパース。
      std::map<std::string, std::string> tree = ParseFEN(fen_str);

      // 配置を解析。
      std::string::iterator itr = tree["position"].begin();
      FOR_SQUARES(square) {
        Bitboard bb = Util::SQUARE[square][R0];

        switch (*itr) {
          case 'P':
            position_[WHITE][PAWN] |= bb;
            break;
          case 'N':
            position_[WHITE][KNIGHT] |= bb;
            break;
          case 'B':
            position_[WHITE][BISHOP] |= bb;
            break;
          case 'R':
            position_[WHITE][ROOK] |= bb;
            break;
          case 'Q':
            position_[WHITE][QUEEN] |= bb;
            break;
          case 'K':
            position_[WHITE][KING] |= bb;
            break;
          case 'p':
            position_[BLACK][PAWN] |= bb;
            break;
          case 'n':
            position_[BLACK][KNIGHT] |= bb;
            break;
          case 'b':
            position_[BLACK][BISHOP] |= bb;
            break;
          case 'r':
            position_[BLACK][ROOK] |= bb;
            break;
          case 'q':
            position_[BLACK][QUEEN] |= bb;
            break;
          case 'k':
            position_[BLACK][KING] |= bb;
            break;
        }

        ++itr;
      }

      // 手番を解析。
      if (tree["to_move"] == "w") {
        to_move_ = WHITE;
      } else {
        to_move_ = BLACK;
      }

      // キャスリングの権利を解析。
      castling_rights_ = 0;
      if (tree["castling_rights"][0] != '-') {
        castling_rights_ |= WHITE_SHORT_CASTLING;
      }
      if (tree["castling_rights"][1] != '-') {
        castling_rights_ |= WHITE_LONG_CASTLING;
      }
      if (tree["castling_rights"][2] != '-') {
        castling_rights_ |= BLACK_SHORT_CASTLING;
      }
      if (tree["castling_rights"][3] != '-') {
        castling_rights_ |= BLACK_LONG_CASTLING;
      }

      // アンパッサンのマスを解析。
      std::string temp = tree["en_passant_square"];
      if (temp != "-") {
        en_passant_square_ =
        Util::CoordToSquare(temp[0] - 'a', temp[1] - '1');
      }

      // 50手ルールの手数を解析。 あれば。
      if (tree.find("clock") != tree.end()) {
        clock_ = std::stoul(tree["clock"]);
      }

      // 手数を解析。 あれば。
      if (tree.find("ply") != tree.end()) {
        ply_ = std::stoul(tree["ply"]);
      }
    } catch (...) {
      SetStartPosition();
    }
  }

  // デフォルトコンストラクタ。
  FEN::FEN() {
    SetStartPosition();
  }

  // コピーコンストラクタ。
  FEN::FEN(const FEN& fen) :
  to_move_(fen.to_move_),
  castling_rights_(fen.castling_rights_),
  en_passant_square_(fen.en_passant_square_),
  clock_(fen.clock_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    COPY_ARRAY(position_, fen.position_);
  }

  // ムーブコンストラクタ。
  FEN::FEN(FEN&& fen) :
  to_move_(fen.to_move_),
  castling_rights_(fen.castling_rights_),
  en_passant_square_(fen.en_passant_square_),
  clock_(fen.clock_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    COPY_ARRAY(position_, fen.position_);
  }

  // コピー代入演算子。
  FEN& FEN::operator=(const FEN& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    clock_ = fen.clock_;
    ply_ = fen.ply_;
    COPY_ARRAY(position_, fen.position_);

    return *this;
  }

  // ムーブ代入演算子。
  FEN& FEN::operator=(FEN&& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    clock_ = fen.clock_;
    ply_ = fen.ply_;
    COPY_ARRAY(position_, fen.position_);

    return *this;
  }

  // ================ //
  // プライベート関数 //
  // ================ //
  // スタートポジションにセット。
  void FEN::SetStartPosition() {
    to_move_ = WHITE;
    castling_rights_ = ALL_CASTLING;
    en_passant_square_ = 0;
    clock_ = 0;
    ply_ = 0;

    // 駒を初期配置にする。
    // ポーン。
    position_[WHITE][PAWN] = Util::RANK[RANK_2];
    position_[BLACK][PAWN] = Util::RANK[RANK_7];
    // ナイト。
    position_[WHITE][KNIGHT] = Util::SQUARE[B1][R0] | Util::SQUARE[G1][R0];
    position_[BLACK][KNIGHT] = Util::SQUARE[B8][R0] | Util::SQUARE[G8][R0];
    // ビショップ。
    position_[WHITE][BISHOP] = Util::SQUARE[C1][R0] | Util::SQUARE[F1][R0];
    position_[BLACK][BISHOP] = Util::SQUARE[C8][R0] | Util::SQUARE[F8][R0];
    // ルーク。
    position_[WHITE][ROOK] = Util::SQUARE[A1][R0] | Util::SQUARE[H1][R0];
    position_[BLACK][ROOK] = Util::SQUARE[A8][R0] | Util::SQUARE[H8][R0];
    // クイーン。
    position_[WHITE][QUEEN] = Util::SQUARE[D1][R0];
    position_[BLACK][QUEEN] = Util::SQUARE[D8][R0];
    // キング。
    position_[WHITE][KING] = Util::SQUARE[E1][R0];
    position_[BLACK][KING] = Util::SQUARE[E8][R0];
  }

  // 構文木を作る。
  std::map<std::string, std::string>
  FEN::ParseFEN(const std::string& fen_str) {
    std::map<std::string, std::string> ret;

    // トークンに分ける。
    std::vector<std::string> token_vec;
    std::ostringstream stream;
    for (auto c : fen_str) {
      if ((c == ' ') || (c == '/')) {
        if (stream.str().size() > 0) {
          token_vec.push_back(stream.str());
          stream.str("");
        }
      }else {
        stream << c;
      }
    }

    if (stream.str().size() > 0) {
      token_vec.push_back(stream.str());
    }
    stream.str("");
    std::vector<std::string>::iterator fen_itr = token_vec.begin();

    // 個数チェック。
    if (fen_str.size() < 11) throw false;

    // 駒の配置を読みやすいように展開する。
    std::vector<std::string>::iterator temp_itr = token_vec.begin();
    temp_itr += 7;
    for (int i = 0; i < 8; ++i, ++fen_itr, --temp_itr) {
      for (auto c : *temp_itr) {
        switch (c) {
          case 'P':
          case 'N':
          case 'B':
          case 'R':
          case 'Q':
          case 'K':
          case 'p':
          case 'n':
          case 'b':
          case 'r':
          case 'q':
          case 'k':
            stream << c;
            break;
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
            {
              int num = c - '0';
              for (int j = 0; j < num; ++j) {
                stream << '-';
              }
            }
            break;
          default:
            throw false;
        }
      }
    }
    if (stream.str().size() != 64) throw false;
    ret["position"] = stream.str();
    stream.str("");

    // 手番をパース。
    if ((*fen_itr == "w") || (*fen_itr == "b")) {
      ret["to_move"] = *(fen_itr++);
    } else {
      throw false;
    }

    // キャスリングの権利をパース。
    std::string temp_str = "----";
    for (auto c : *fen_itr) {
      switch (c) {
        case 'K':
          temp_str[0] = 'K';
          break;
        case 'Q':
          temp_str[1] = 'Q';
          break;
        case 'k':
          temp_str[2] = 'k';
          break;
        case 'q':
          temp_str[3] = 'q';
          break;
        case '-':
          break;
        default:
          throw false;
      }
    }
    ++fen_itr;
    ret["castling_rights"] = temp_str;

    // アンパッサンのマスをパース。
    if (*fen_itr == "-") {
      ret["en_passant_square"] = *(fen_itr++);
    } else if (fen_itr->size() == 2) {
      char fyle_c = (*fen_itr)[0];
      char rank_c = (*fen_itr)[1];
      if (((fyle_c >= 'a') && (fyle_c <= 'h'))
      && ((rank_c == '3') || (rank_c == '6'))) {
        ret["en_passant_square"] = *(fen_itr++);
      } else {
        throw false;
      }
    } else {
      throw false;
    }

    // 50手ルールの手数をパース。 オプショナル。
    if (fen_itr != token_vec.end()) {
      try {
        unsigned long clock = std::stoul(*(fen_itr++));
        ret["clock"] = std::to_string(clock);
      } catch (...) {
        throw false;
      }
    }

    // 手数をパース。 オプショナル。
    if (fen_itr != token_vec.end()) {
      try {
        unsigned long ply = std::stoul(*(fen_itr++)) * 2;

        if (ply < 1) throw false;

        if (ret["to_move"] == "w") {
          ply -= 1;
        }

        ret["ply"] = std::to_string(ply);
      } catch (...) {
        throw false;
      }
    }

    return ret;
  }
}  // namespace Sayuri
