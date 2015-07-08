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
  // 繰り返しテスト。
  void DoRepeatTest(UCIShell& shell) {
    volatile bool loop = true;
    volatile int count = 0;

    shell.AddOutputListener
    ([&loop, &count](const std::string& message) {
      std::vector<std::string> words =
      Util::Split<char>(message, {' '}, std::set<char> {});

      if (words[0] == "bestmove") {
        std::cout << "----- " << count++ << " -----" << std::endl;
        std::cout << message << std::endl;
        loop = false;
      }
    });

    shell.InputCommand("setoption name threads value 8");
    shell.InputCommand("setoption name hash value 256");
    for (int i = 0; i < 200; ++i) {
      loop = true;
      shell.InputCommand("ucinewgame");
      shell.InputCommand("position startpos");
      shell.InputCommand("go movetime 15000");
      while (loop) {
        std::this_thread::sleep_for(Chrono::milliseconds(500));
      }
    }
  }

  int DebugMain(int argc, char* argv[]) {
    // プログラムの起動。
    // 初期化。
    Init();
    // エンジン準備。
    std::unique_ptr<Sayuri::SearchParams>
    search_params_ptr(new Sayuri::SearchParams());

    std::unique_ptr<Sayuri::EvalParams>
    eval_params_ptr(new Sayuri::EvalParams());

    std::unique_ptr<Sayuri::ChessEngine>
    engine_ptr(new Sayuri::ChessEngine(*search_params_ptr, *eval_params_ptr));

    std::unique_ptr<Sayuri::TranspositionTable>
    table_ptr(new Sayuri::TranspositionTable(Sayuri::UCI_DEFAULT_TABLE_SIZE));

    std::unique_ptr<Sayuri::UCIShell>
    shell_ptr(new Sayuri::UCIShell(*engine_ptr, *table_ptr));

    // ========================================================================

    DoRepeatTest(*shell_ptr);

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
    FOR_RANKS(rank) {
      osstream << c << "| ";
      FOR_FYLES(fyle) {
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
    fyle = Util::SquareToFyle(GetFrom(move));
    rank = Util::SquareToRank(GetFrom(move));
    std::cout << fyle_table[fyle] << rank_table[rank] << std::endl;

    // 移動先の位置を出力する。
    std::cout << "To: ";
    fyle = Util::SquareToFyle(GetTo(move));
    rank = Util::SquareToRank(GetTo(move));
    std::cout << fyle_table[fyle] << rank_table[rank] << std::endl;

    // 取った駒の種類を出力する。
    std::cout << "Captured PieceType: ";
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
        throw SayuriError("debug.cpp::PrintMove()_1");
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
        throw SayuriError("debug.cpp::PrintMove()_2");
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
      fyle = Util::SquareToFyle(en_passant_square);
      rank = Util::SquareToRank(en_passant_square);
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
        throw SayuriError("debug.cpp::PrintMove()_3");
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
    FOR_RANKS(rank) {
      osstream << c << "| ";
      FOR_FYLES(fyle) {
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
      std::cout << fyle_str[Util::SquareToFyle(en_passant)];
      std::cout << rank_str[Util::SquareToRank(en_passant)] << std::endl;
    } else {
      std::cout << "No Square" << std::endl;
    }
    std::cout << "Ply 100 Rule: " << record.clock() << std::endl;
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

  // Evaluatorのvalue_table_の中身を表示する。
  /*
  void PrintValueTable(const Evaluator& evaluator) {
    const int (& table)[Evaluator::TABLE_SIZE][NUM_PIECE_TYPES] =
    evaluator.value_table_;

    std::function<void(unsigned int)> print = [&table](unsigned int index) {
      std::cout << "    EMPTY: " << table[index][EMPTY] << std::endl;
      std::cout << "    PAWN: " << table[index][PAWN] << std::endl;
      std::cout << "    KNIGHT: " << table[index][KNIGHT] << std::endl;
      std::cout << "    BISHOP: " << table[index][BISHOP] << std::endl;
      std::cout << "    ROOK: " << table[index][ROOK] << std::endl;
      std::cout << "    QUEEN: " << table[index][QUEEN] << std::endl;
      std::cout << "    KING: " << table[index][KING] << std::endl;
    };

    // オープニングの配置。
    std::cout << "OPENING_POSITION:" << std::endl;
    print(Evaluator::OPENING_POSITION);

    // エンディングの配置。
    std::cout << "ENDING_POSITION:" << std::endl;
    print(Evaluator::ENDING_POSITION);

    // 機動力。
    std::cout << "MOBILITY:" << std::endl;
    print(Evaluator::MOBILITY);

    // センターコントロール。
    std::cout << "CENTER_CONTROL:" << std::endl;
    print(Evaluator::CENTER_CONTROL);

    // スウィートセンターのコントロール。
    std::cout << "SWEET_CENTER_CONTROL:" << std::endl;
    print(Evaluator::SWEET_CENTER_CONTROL);

    // ピースの展開。
    std::cout << "DEVELOPMENT:" << std::endl;
    print(Evaluator::DEVELOPMENT);

    // 攻撃。
    std::cout << "ATTACK:" << std::endl;
    print(Evaluator::ATTACK);

    // 防御。
    std::cout << "DEFENSE:" << std::endl;
    print(Evaluator::DEFENSE);

    // ピン。
    std::cout << "PIN:" << std::endl;
    print(Evaluator::PIN);

    // キング周辺への攻撃。
    std::cout << "ATTACK_AROUND_KING:" << std::endl;
    print(Evaluator::ATTACK_AROUND_KING);

    // パスポーン。
    std::cout << "PASS_PAWN:" << std::endl;
    print(Evaluator::PASS_PAWN);

    // 守られたパスポーン。
    std::cout << "PROTECTED_PASS_PAWN:" << std::endl;
    print(Evaluator::PROTECTED_PASS_PAWN);

    // ダブルポーン。
    std::cout << "DOUBLE_PAWN:" << std::endl;
    print(Evaluator::DOUBLE_PAWN);

    // 孤立ポーン。
    std::cout << "ISO_PAWN:" << std::endl;
    print(Evaluator::ISO_PAWN);

    // ポーンの盾。
    std::cout << "PAWN_SHIELD:" << std::endl;
    print(Evaluator::PAWN_SHIELD);

    // ビショップペア。
    std::cout << "BISHOP_PAIR:" << std::endl;
    print(Evaluator::BISHOP_PAIR);

    // バッドビショップ。
    std::cout << "BAD_BISHOP:" << std::endl;
    print(Evaluator::BAD_BISHOP);

    // ルークペア。
    std::cout << "ROOK_PAIR:" << std::endl;
    print(Evaluator::ROOK_PAIR);

    // セミオープンファイルのルーク。
    std::cout << "ROOK_SEMIOPEN_FYLE:" << std::endl;
    print(Evaluator::ROOK_SEMIOPEN_FYLE);

    // オープンファイルのルーク。
    std::cout << "ROOK_OPEN_FYLE:" << std::endl;
    print(Evaluator::ROOK_OPEN_FYLE);

    // 早すぎるクイーンの始動。
    std::cout << "EARLY_QUEEN_LAUNCHED:" << std::endl;
    print(Evaluator::EARLY_QUEEN_LAUNCHED);

    // キング周りの弱いマス。
    std::cout << "WEAK_SQUARE:" << std::endl;
    print(Evaluator::WEAK_SQUARE);

    // キャスリング。
    std::cout << "CASTLING:" << std::endl;
    print(Evaluator::CASTLING);

    // キャスリングの放棄。
    std::cout << "ABANDONED_CASTLING:" << std::endl;
    print(Evaluator::ABANDONED_CASTLING);
  }
  */

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
