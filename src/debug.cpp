/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
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
 * @file debug.cpp
 * @author Hironori Ishibashi
 * @brief Sayuriのデバッグツールの実装。
 */

#include "debug.h"

#include <iostream>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <sstream>
#include <random>
#include <vector>
#include <thread>
#include <functional>

#include "sayuri.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // デバッグ用メイン関数 //
  // ==================== //
  int DebugMain(int argc, char* argv[]) {
    // プログラムの起動。
    // 初期化。
    Init();

    // エンジン準備。
    std::unique_ptr<SearchParams>
    search_params_ptr(new SearchParams());

    std::unique_ptr<EvalParams>
    eval_params_ptr(new EvalParams());

    std::unique_ptr<ChessEngine>
    engine_ptr(new ChessEngine(*search_params_ptr, *eval_params_ptr));

    std::unique_ptr<UCIShell>
    shell_ptr(new UCIShell(*engine_ptr));
    // ========================================================================

    return 0;
  }

  // ビットボードの状態を出力する。
  void PrintBitboard(Bitboard bitboard) {
    // 出力する文字列ストリーム。
    std::ostringstream osstream;

    // 上下のボーダー。
    std::string border(" +-----------------+");

    // 上のボーダーをストリームへ。
    osstream << border << std::endl;

    // ビットボードを出力。
    Bitboard bit = 0x1ULL << (8 * 7);  // 初期位置a8へシフト。
    char c = '8';  // ランクの文字。
    for (Rank rank = 0; rank < NUM_RANKS; ++rank) {
      osstream << c << "| ";
      for (Fyle fyle = 0; fyle < NUM_FYLES; ++fyle) {
        if (bitboard & bit) {
          osstream << "@ ";
        } else {
          osstream << ". ";
        }
        if (fyle < 7) bit <<= 1;
      }
      // 一つ下のランクへ。
      bit >>= (7 + 8);
      osstream << "|" << std::endl;
      --c;
    }

    // 下部分を書く。
    osstream << border << std::endl;
    osstream << "   a b c d e f g h" << std::endl;

    // 標準出力に出力。
    std::cout << osstream.str();
  }

  // Moveの状態を出力する。
  void PrintMove(Move move) {
    // ファイルとランクの文字の配列。
    constexpr char fyle_table[NUM_FYLES] = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };
    constexpr char rank_table[NUM_RANKS] = {
      '1', '2', '3', '4', '5', '6', '7', '8'
    };

    // ファイルとランク。
    Fyle fyle = 0;
    Rank rank = 0;

    // 動かす駒の位置を出力する。
    std::cout << "From: ";
    fyle = Util::SQUARE_TO_FYLE[GetFrom(move)];
    rank = Util::SQUARE_TO_RANK[GetFrom(move)];
    std::cout << fyle_table[fyle] << rank_table[rank] << std::endl;

    // 移動先の位置を出力する。
    std::cout << "To: ";
    fyle = Util::SQUARE_TO_FYLE[GetTo(move)];
    rank = Util::SQUARE_TO_RANK[GetTo(move)];
    std::cout << fyle_table[fyle] << rank_table[rank] << std::endl;

    // 取った駒の種類を出力する。
    std::cout << "Captured Piece: ";
    switch (GetCapturedPiece(move)) {
      case EMPTY:
        std::cout << "None";
        break;
      case PAWN:
        std::cout << "Pawn";
        break;
      case KNIGHT:
        std::cout << "Knight";
        break;
      case BISHOP:
        std::cout << "Bishop";
        break;
      case ROOK:
        std::cout << "Rook";
        break;
      case QUEEN:
        std::cout << "Queen";
        break;
      case KING:
        std::cout << "King";
        break;
      default:
        throw SayuriError("駒の種類が不正です。 in debug.cpp::PrintMove()");
        break;
    }
    std::cout << std::endl;

    // 昇格する駒の種類を出力する。
    std::cout << "Promotion: ";
    switch (GetPromotion(move)) {
      case EMPTY:
        std::cout << "None";
        break;
      case PAWN:
        std::cout << "Pawn";
        break;
      case KNIGHT:
        std::cout << "Knight";
        break;
      case BISHOP:
        std::cout << "Bishop";
        break;
      case ROOK:
        std::cout << "Rook";
        break;
      case QUEEN:
        std::cout << "Queen";
        break;
      case KING:
        std::cout << "King";
        break;
      default:
        throw SayuriError("駒の種類が不正です。 in debug.cpp::PrintMove()");
        break;
    }
    std::cout << std::endl;

    // キャスリングを出力する。
    Castling castling = GetCastlingRights(move);
    std::cout << "Castling Rights: ";
    if (castling & WHITE_SHORT_CASTLING)
      std::cout << "K";
    if (castling & WHITE_LONG_CASTLING)
      std::cout << "Q";
    if (castling & BLACK_SHORT_CASTLING)
      std::cout << "k";
    if (castling & BLACK_LONG_CASTLING)
      std::cout << "q";
    std::cout << std::endl;

    // アンパッサンのターゲットを出力する。
    if (Square en_passant_square = GetEnPassantSquare(move)) {
      fyle = Util::SQUARE_TO_FYLE[en_passant_square];
      rank = Util::SQUARE_TO_RANK[en_passant_square];
      std::cout << "En Passant Square: "
      << fyle_table[fyle] << rank_table[rank] << std::endl;
    } else {
      std::cout << "En Passant Square: Nothing" << std::endl;
    }

    // 手の種類を出力する。
    std::cout << "Move Type: ";
    switch (GetMoveType(move)) {
      case NORMAL:
        std::cout << "Normal";
        break;
      case CASTLE_WS:
        std::cout << "White Short Castling";
        break;
      case CASTLE_WL:
        std::cout << "White Long Castling";
        break;
      case CASTLE_BS:
        std::cout << "Black Short Castling";
        break;
      case CASTLE_BL:
        std::cout << "Black Long Castling";
        break;
      case EN_PASSANT:
        std::cout << "En Passant";
        break;
      case NULL_MOVE:
        std::cout << "Null Move";
        break;
      default:
        throw SayuriError("手の種類が不正です。 in debug.cpp::PrintMove()");
        break;
    }
    std::cout << std::endl;
  }

  // 駒の配置の状態を出力する。
  void PrintPosition(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]) {
    // 出力する文字列ストリーム。
    std::ostringstream osstream;

    // 上下のボーダー。
    std::string border(" +-----------------+");

    // 上のボーダーをストリームへ。
    osstream << border << std::endl;

    // 駒の配置を出力。
    Bitboard bit = 0x1ULL << (8 * 7);  // 初期位置a8へシフト。
    char c = '8';  // ランクの文字。
    for (Rank rank = 0; rank < NUM_RANKS; ++rank) {
      osstream << c << "| ";
      for (Fyle fyle = 0; fyle < NUM_FYLES; ++fyle) {
        if (position[WHITE][PAWN] & bit) {
          osstream << "P ";
        } else if (position[WHITE][KNIGHT] & bit) {
          osstream << "N ";
        } else if (position[WHITE][BISHOP] & bit) {
          osstream << "B ";
        } else if (position[WHITE][ROOK] & bit) {
          osstream << "R ";
        } else if (position[WHITE][QUEEN] & bit) {
          osstream << "Q ";
        } else if (position[WHITE][KING] & bit) {
          osstream << "K ";
        } else if (position[BLACK][PAWN] & bit) {
          osstream << "p ";
        } else if (position[BLACK][KNIGHT] & bit) {
          osstream << "n ";
        } else if (position[BLACK][BISHOP] & bit) {
          osstream << "b ";
        } else if (position[BLACK][ROOK] & bit) {
          osstream << "r ";
        } else if (position[BLACK][QUEEN] & bit) {
          osstream << "q ";
        } else if (position[BLACK][KING] & bit) {
          osstream << "k ";
        } else {
          osstream << ". ";
        }

        if (fyle < 7) bit <<= 1;
      }
      // 一つ下のランクへ。
      bit >>= (7 + 8);
      osstream << "|" << std::endl;
      --c;
    }

    // 下部分を書く。
    osstream << border << std::endl;
    osstream << "   a b c d e f g h" << std::endl;

    // 標準出力に出力。
    std::cout << osstream.str();
  }

  // PositionRecordの状態を出力。
  void PrintPositionRecord(const PositionRecord& record) {
    PrintPosition(record.position());
    std::cout << "To Move: ";
    switch (record.to_move()) {
      case NO_SIDE:
        std::cout << "No Side";
        break;
      case WHITE:
        std::cout << "White";
        break;
      case BLACK:
        std::cout << "Black";
        break;
      default:
        std::cout << "Error";
        break;
    }
    std::cout << std::endl;
    std::cout << "Castling Rights:" << std::endl;
    Castling rights = record.castling_rights();
    if (rights) {
      if ((rights & WHITE_SHORT_CASTLING)) {
        std::cout << "    White Short" << std::endl;
      }
      if ((rights & WHITE_LONG_CASTLING)) {
        std::cout << "    White Long" << std::endl;
      }
      if ((rights & BLACK_SHORT_CASTLING)) {
        std::cout << "    Black Short" << std::endl;
      }
      if ((rights & BLACK_LONG_CASTLING)) {
        std::cout << "    Black Long" << std::endl;
      }
    } else {
      std::cout << "    No One Has Rights" << std::endl;
    }
    std::cout << "En Passant Square: ";
    Square en_passant = record.en_passant_square();
    if (en_passant) {
      std::string fyle_str[NUM_FYLES] {
        "a", "b", "c", "d", "e", "f", "g", "h"
      };
      std::string rank_str[NUM_RANKS] {
        "1", "2", "3", "4", "5", "6", "7", "8"
      };
      std::cout << fyle_str[Util::SQUARE_TO_FYLE[en_passant]];
      std::cout << rank_str[Util::SQUARE_TO_RANK[en_passant]] << std::endl;
    } else {
      std::cout << "No Square" << std::endl;
    }
    std::cout << "Ply 100 Rule: " << record.ply_100() << std::endl;
    std::cout << "Ply: " << record.ply() << std::endl;
    std::cout << "Has Castled:" << std::endl;
    std::cout << "    White: ";
    if (record.has_castled()[WHITE]) std::cout << "Yes" << std::endl;
    else std::cout << "No" << std::endl;
    std::cout << "    Black: ";
    if (record.has_castled()[BLACK]) std::cout << "Yes" << std::endl;
    else std::cout << "No" << std::endl;
    std::cout << "Current Hash: " << record.pos_hash() << std::endl;
  }

  // EvalResultの状態を出力する。
  void PrintEvalResult(const EvalResult& result) {
    std::string piece_name[NUM_PIECE_TYPES] {
      "Empty", "Pawn", "Knight", "Bishop", "Rook","Queen", "King"
    };

    std::cout << "Total Score: " << result.score_ << std::endl;

    std::cout << "Material: " << result.material_ << std::endl;

    std::ostringstream os;
    double total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_opening_position_[type] << std::endl;
      total += result.score_opening_position_[type];
    }
    std::cout << "Opening Position: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_ending_position_[type] << std::endl;
      total += result.score_ending_position_[type];
    }
    std::cout << "Ending Position: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_mobility_[type] << std::endl;
      total += result.score_mobility_[type];
    }
    std::cout << "Mobility: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_center_control_[type] << std::endl;
      total += result.score_center_control_[type];
    }
    std::cout << "Center Control: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_sweet_center_control_[type] << std::endl;
      total += result.score_sweet_center_control_[type];
    }
    std::cout << "Sweet Center Control: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_development_[type] << std::endl;
      total += result.score_development_[type];
    }
    std::cout << "Development: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_attack_[type] << std::endl;
      total += result.score_attack_[type];
    }
    std::cout << "Attack: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_defense_[type] << std::endl;
      total += result.score_defense_[type];
    }
    std::cout << "Defense: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_pin_[type] << std::endl;
      total += result.score_pin_[type];
    }
    std::cout << "Pin: " << total << std::endl;
    std::cout << os.str();

    os.str("");
    total = 0.0;
    for (Piece type = 0; type < NUM_PIECE_TYPES; ++type) {
      os << "    " << piece_name[type] << ": "
      << result.score_attack_around_king_[type] << std::endl;
      total += result.score_attack_around_king_[type];
    }
    std::cout << "Attack Around King: " << total << std::endl;
    std::cout << os.str();

    std::cout << "Pass Pawn: " << result.score_pass_pawn_ << std::endl;

    std::cout << "Protected Pass Pawn: " << result.score_protected_pass_pawn_
    << std::endl;

    std::cout << "Double Pawn: " << result.score_double_pawn_ << std::endl;

    std::cout << "Iso Pawn: " << result.score_iso_pawn_ << std::endl;

    std::cout << "Pawn Shield: " << result.score_pawn_shield_ << std::endl;

    std::cout << "Bishop Pair: " << result.score_bishop_pair_ << std::endl;

    std::cout << "Bad Bishop: " << result.score_bad_bishop_ << std::endl;

    std::cout << "Rook Pair: " << result.score_rook_pair_ << std::endl;

    std::cout << "Rook Semiopen Fyle: " << result.score_rook_semiopen_fyle_
    << std::endl;

    std::cout << "Rook Open Fyle: " << result.score_rook_open_fyle_
    << std::endl;

    std::cout << "Early Queen Launched: "
    << result.score_early_queen_launched_ << std::endl;

    std::cout << "Weak Square: " << result.score_weak_square_ << std::endl;

    std::cout << "Castling: " << result.score_castling_ << std::endl;

    std::cout << "Abandoned Castling: " << result.score_abandoned_castling_
    << std::endl;
  }

  // ================ //
  // ストップウォッチ //
  // ================ //
  // コンストラクタ。
  StopWatch::StopWatch() :
  start_point_(SysClock::now()),
  stop_point_(SysClock::now()) {}

  // コピーコンストラクタ。
  StopWatch::StopWatch(const StopWatch& watch) :
  start_point_(watch.start_point_),
  stop_point_(watch.stop_point_) {}

  // ムーブコンストラクタ。
  StopWatch::StopWatch(StopWatch&& watch) :
  start_point_(std::move(watch.start_point_)),
  stop_point_(std::move(watch.stop_point_)) {}

  // コピー代入演算子。
  StopWatch& StopWatch::operator=(const StopWatch& watch) {
    start_point_ = watch.start_point_;
    stop_point_ = watch.stop_point_;
    return *this;
  }

  // ムーブ代入演算子。
  StopWatch& StopWatch::operator=(StopWatch&& watch) {
    start_point_ = std::move(watch.start_point_);
    stop_point_ = std::move(watch.stop_point_);
    return *this;
  }

  // ストップウォッチをスタート。
  void StopWatch::Start() {
    start_point_ = SysClock::now();
  }

  // ストップウォッチをストップ。
  void StopWatch::Stop() {
    stop_point_ = SysClock::now();
  }

  // 計測時間を得る。
  int StopWatch::GetTime() {
    return Chrono::duration_cast<Chrono::milliseconds>
    (stop_point_ - start_point_).count();
  }
}  // namespace Sayuri
