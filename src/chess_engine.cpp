/*
   chess_engine.cpp: チェスボードの基本的な実装。

   The MIT License (MIT)

   Copyright (c) 2014 Hironori Ishibashi

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#include "chess_engine.h"

#include <iostream>
#include <sstream>
#include <random>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <memory>
#include <vector>
#include <array>
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"
#include "fen.h"
#include "move_maker.h"
#include "error.h"
#include "uci_shell.h"
#include "position_record.h"
#include "job.h"
#include "helper_queue.h"
#include "params.h"

namespace Sayuri {
  /****************/
  /* static変数。 */
  /****************/
  // 駒の情報からハッシュを得るための配列。
  // piece_hash_table_[サイド][駒の種類][駒の位置]
  Hash ChessEngine::piece_hash_table_
  [NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
  // 手番からハッシュを得るための配列。
  Hash ChessEngine::to_move_hash_table_[NUM_SIDES];
  // キャスリングの権利からハッシュを得るための配列。
  Hash ChessEngine::castling_hash_table_[4];
  // アンパッサンの位置からハッシュを得るための配列。
  Hash ChessEngine::en_passant_hash_table_[NUM_SQUARES];

  /**************************/
  /* コンストラクタと代入。 */
  /***************************/
  // コンストラクタ。
  ChessEngine::ChessEngine(const EvalParams& eval_params) {
    SetNewGame();

    // 評価関数用パラメータのコピー。
    shared_st_ptr_->eval_params_ = eval_params;
  }

  // コピーコンストラクタ。
  ChessEngine::ChessEngine(const ChessEngine& engine) {
    // 基本メンバをコピー。
    ScanBasicMember(engine);

    // 共有メンバのコピー。
    shared_st_ptr_.reset(new SharedStruct(*(engine.shared_st_ptr_)));
  }

  // ムーブコンストラクタ。
  ChessEngine::ChessEngine(ChessEngine&& engine) {
    // 基本メンバをコピー。
    ScanBasicMember(engine);

    // 共有メンバのムーブ。
    shared_st_ptr_ = std::move(shared_st_ptr_);
  }

  // コピー代入。
  ChessEngine& ChessEngine::operator=(const ChessEngine& engine) {
    // 基本メンバをコピー。
    ScanBasicMember(engine);

    // 共有メンバをコピー。
    shared_st_ptr_.reset(new SharedStruct(*(engine.shared_st_ptr_)));

    return *this;
  }

  // ムーブ代入。
  ChessEngine& ChessEngine::operator=(ChessEngine&& engine) {
    // 基本メンバをコピー。
    ScanBasicMember(engine);

    // 共有メンバをムーブ。
    shared_st_ptr_ = std::move(shared_st_ptr_);

    return *this;
  }
  // デストラクタ。
  ChessEngine::~ChessEngine() {
  }

  /*******************************/
  /* ChessEngineクラスの初期化。 */
  /*******************************/
  void ChessEngine::InitChessEngine() {
    // ハッシュの配列を初期化する。
    InitHashTable();
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  void ChessEngine::LoadFen(const Fen& fen) {
    // キングの数がおかしいならやめる。
    int num_white_king = Util::CountBits(fen.position()[WHITE][KING]);
    int num_black_king = Util::CountBits(fen.position()[BLACK][KING]);
    if ((num_white_king != 1) || (num_black_king != 1)) return;

    // 空にする。
    for (Square i = 0; i < NUM_SQUARES; i++) {
      piece_board_[i] = EMPTY;
      side_board_[i] = NO_SIDE;
    }
    blocker_0_ = 0ULL;
    blocker_45_ = 0ULL;
    blocker_90_ = 0ULL;
    blocker_135_ = 0ULL;

    // 駒を配置する。
    for (Side i = WHITE; i <= BLACK; i++) {
      side_pieces_[i] = 0ULL;
      for (Piece j = PAWN; j <= KING; j++) {
        position_[i][j] = fen.position()[i][j];
        for (Bitboard bb = position_[i][j]; bb; bb &= bb - 1) {
          Square square = Util::GetSquare(bb);
          side_board_[square] = i;
          piece_board_[square] = j;
          side_pieces_[i] |= Util::SQUARE[square];
          blocker_0_ |= Util::SQUARE[square];
          blocker_45_ |= Util::SQUARE[Util::ROT45[square]];
          blocker_90_ |= Util::SQUARE[Util::ROT90[square]];
          blocker_135_ |= Util::SQUARE[Util::ROT135[square]];
          if (j == KING) {
            king_[i] = square;
          }
        }
      }
    }

    // 残りを設定。
    to_move_ = fen.to_move();
    castling_rights_ = fen.castling_rights();
    en_passant_square_ = fen.en_passant_square();
    ply_100_ = fen.ply_100();
    ply_ = fen.ply();

    // 履歴を設定。
    shared_st_ptr_->ply_100_history_.clear();
    shared_st_ptr_->ply_100_history_.push_back(ply_100_);
    shared_st_ptr_->position_history_.clear();
    shared_st_ptr_->position_history_.push_back(PositionRecord(*this));
  }

  // PositionRecordから読み込む。
  void ChessEngine::LoadRecord(const PositionRecord& record) {
    // 空にする。
    for (Square i = 0; i < NUM_SQUARES; i++) {
      piece_board_[i] = EMPTY;
      side_board_[i] = NO_SIDE;
    }
    blocker_0_ = 0ULL;
    blocker_45_ = 0ULL;
    blocker_90_ = 0ULL;
    blocker_135_ = 0ULL;

    // 駒を配置する。
    for (Side i = WHITE; i <= BLACK; i++) {
      has_castled_[i] = record.has_castled()[i];
      side_pieces_[i] = 0ULL;
      for (Piece j = PAWN; j <= KING; j++) {
        position_[i][j] = record.position()[i][j];
        for (Bitboard bb = position_[i][j]; bb; bb &= bb - 1) {
          Square square = Util::GetSquare(bb);
          side_board_[square] = i;
          piece_board_[square] = j;
          side_pieces_[i] |= Util::SQUARE[square];
          blocker_0_ |= Util::SQUARE[square];
          blocker_45_ |= Util::SQUARE[Util::ROT45[square]];
          blocker_90_ |= Util::SQUARE[Util::ROT90[square]];
          blocker_135_ |= Util::SQUARE[Util::ROT135[square]];
          if (j == KING) {
            king_[i] = square;
          }
        }
      }
    }

    // 残りを設定。
    to_move_ = record.to_move();
    castling_rights_ = record.castling_rights();
    en_passant_square_ = record.en_passant_square();
    ply_100_ = record.ply_100();
    ply_ = record.ply();
  }

  // 新しいゲームの準備をする。
  void ChessEngine::SetNewGame() {
    // 駒の配置を作る。
    // どちらでもない。
    for (Piece piece_type = 0U; piece_type < NUM_PIECE_TYPES; piece_type++) {
      position_[NO_SIDE][piece_type] = 0ULL;
    }
    // 白の駒。
    position_[WHITE][EMPTY] = 0ULL;
    position_[WHITE][PAWN] = Util::RANK[RANK_2];
    position_[WHITE][KNIGHT] = Util::SQUARE[B1] | Util::SQUARE[G1];
    position_[WHITE][BISHOP] = Util::SQUARE[C1] | Util::SQUARE[F1];
    position_[WHITE][ROOK] = Util::SQUARE[A1] | Util::SQUARE[H1];
    position_[WHITE][QUEEN] = Util::SQUARE[D1];
    position_[WHITE][KING] = Util::SQUARE[E1];
    // 黒の駒。
    position_[BLACK][EMPTY] = 0ULL;
    position_[BLACK][PAWN] = Util::RANK[RANK_7];
    position_[BLACK][KNIGHT] = Util::SQUARE[B8] | Util::SQUARE[G8];
    position_[BLACK][BISHOP] = Util::SQUARE[C8] | Util::SQUARE[F8];
    position_[BLACK][ROOK] = Util::SQUARE[A8] | Util::SQUARE[H8];
    position_[BLACK][QUEEN] = Util::SQUARE[D8];
    position_[BLACK][KING] = Util::SQUARE[E8];

    // 各サイドの駒の配置を作る。
    side_pieces_[NO_SIDE] = 0ULL;
    side_pieces_[WHITE] = 0ULL;
    side_pieces_[BLACK] = 0ULL;
    for (Piece i = PAWN; i < NUM_PIECE_TYPES; i++) {
      side_pieces_[WHITE] |= position_[WHITE][i];
      side_pieces_[BLACK] |= position_[BLACK][i];
    }

    // ブロッカーのビットボードを作る。
    blocker_0_ = side_pieces_[WHITE] | side_pieces_[BLACK];
    blocker_45_ = 0ULL;
    blocker_90_ = 0ULL;
    blocker_135_ = 0ULL;
    for (Bitboard copy = blocker_0_; copy; copy &= copy - 1) {
      Square square = Util::GetSquare(copy);

      blocker_45_ |= Util::SQUARE[Util::ROT45[square]];
      blocker_90_ |= Util::SQUARE[Util::ROT90[square]];
      blocker_135_ |= Util::SQUARE[Util::ROT135[square]];
    }

    // 駒の種類とサイドの配置を作る。
    Bitboard point = 1ULL;
    for (Square i = 0U; i < NUM_SQUARES; i++) {
      // サイドの配置。
      if (side_pieces_[WHITE] & point) side_board_[i] = WHITE;
      else if (side_pieces_[BLACK] & point) side_board_[i] = BLACK;
      else side_board_[i] = NO_SIDE;

      // 駒の種類の配置。
      if ((point & position_[WHITE][PAWN])
      || (point & position_[BLACK][PAWN])) {
        piece_board_[i] = PAWN;
      } else if ((point & position_[WHITE][KNIGHT])
      || (point & position_[BLACK][KNIGHT])) {
        piece_board_[i] = KNIGHT;
      } else if ((point & position_[WHITE][BISHOP])
      || (point & position_[BLACK][BISHOP])) {
        piece_board_[i] = BISHOP;
      } else if ((point & position_[WHITE][ROOK])
      || (point & position_[BLACK][ROOK])) {
        piece_board_[i] = ROOK;
      } else if ((point & position_[WHITE][QUEEN])
      || (point & position_[BLACK][QUEEN])) {
        piece_board_[i] = QUEEN;
      } else if ((point & position_[WHITE][KING])
      || (point & position_[BLACK][KING])) {
        piece_board_[i] = KING;
      } else {
        piece_board_[i] = EMPTY;
      }
      point <<= 1;
    }

    // キングを入れる。
    king_[NO_SIDE] = A1;  // これは使わない。
    king_[WHITE] = E1;
    king_[BLACK] = E8;

    // 手番を初期化。
    to_move_ = WHITE;

    // キャスリングの権利を初期化。
    castling_rights_ = ALL_CASTLING;

    // アンパッサンを初期化。
    en_passant_square_ = 0;

    // 50手ルールを初期化。
    ply_100_ = 0;

    // 手数を初期化。
    ply_ = 1;

    // キャスリングしたかどうかの初期化。
    for (Side i = 0U; i < NUM_SIDES; i++) {
      has_castled_[i] = false;
    }

    // 共有メンバ構造体を初期化。
    shared_st_ptr_.reset(new SharedStruct());

    // 50手ルールの履歴を初期化。
    shared_st_ptr_->ply_100_history_.push_back(0);

    // 駒の配置を初期化。
    shared_st_ptr_->position_history_.push_back(PositionRecord(*this));
  }

  // 他のエンジンの基本メンバをコピーする。
  void ChessEngine::ScanBasicMember(const ChessEngine& engine) {
    // サイド毎のコピー。
    for (Side i = 0; i < NUM_SIDES; i++) {
      // 駒の配置をコピー。
      for (Piece j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = engine.position_[i][j];
      }

      // 各サイドの駒の配置のコピー。
      side_pieces_[i] = engine.side_pieces_[i];

      // キングの位置のコピー。
      king_[i] = engine.king_[i];

      // キャスリングしたかどうかのコピー。
      has_castled_[i] = engine.has_castled_[i];
    }

    // マス毎のコピー。
    for (Square i = 0; i < NUM_SQUARES; i++) {
      // 駒の種類の配置のコピー。
      piece_board_[i] = engine.piece_board_[i];

      // サイドの配置のコピー。
      side_board_[i] = engine.side_board_[i];
    }

    // ブロッカーのコピー。
    blocker_0_ = engine.blocker_0_;
    blocker_45_ = engine.blocker_45_;
    blocker_90_ = engine.blocker_90_;
    blocker_135_ = engine.blocker_135_;

    // 手番のコピー。
    to_move_ = engine.to_move_;

    // キャスリングの権利のコピー。
    castling_rights_ = engine.castling_rights_;

    // アンパッサンのコピー。
    en_passant_square_ = engine.en_passant_square_;

    // 50手ルールの手数のコピー。
    ply_100_ = engine.ply_100_;

    // 手数のコピー。
    ply_ = engine.ply_;
  }

  // 思考を始める。
  PVLine ChessEngine::Calculate(int num_threads, TranspositionTable& table,
  std::vector<Move>* moves_to_search_ptr, UCIShell& shell) {
    num_threads = num_threads >= 1 ? num_threads : 1;
    thread_vec_.resize(num_threads);
    return std::move(SearchRoot(table, moves_to_search_ptr, shell));
  }

  // 思考を停止する。
  void ChessEngine::StopCalculation() {
    shared_st_ptr_->stop_now_ = true;
  }

  // 手を指す。
  void ChessEngine::PlayMove(Move move) {
    // 合法手かどうか調べる。
    // 手を展開する。
    shared_st_ptr_->history_max_ = 1ULL;  // makerが0の除算をしないように。
    MoveMaker maker(*this);
    maker.GenMoves<GenMoveType::ALL>(0U, 0U, 0U, 0U);
    // 合法手かどうか調べる。
    bool is_legal = false;
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    for (Move temp_move = maker.PickMove(); temp_move;
    temp_move = maker.PickMove()) {
      MakeMove(temp_move);
      // temp_moveが合法手かどうか調べる。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(temp_move);
        continue;
      }
      // temp_moveと同じ手かどうか調べる。
      if (EqualMove(move, temp_move)) {
        UnmakeMove(temp_move);
        move = temp_move;
        is_legal = true;
        break;
      }
      UnmakeMove(temp_move);
    }

    if (is_legal) {
      ply_++;
      if ((piece_board_[move_from(move)] == PAWN)
      || (piece_board_[move_to(move)] != EMPTY)) {
        ply_100_ = 0;
      } else {
        ply_100_++;
      }
      shared_st_ptr_->move_history_.push_back(move);
      shared_st_ptr_->ply_100_history_.push_back(ply_100_);
      MakeMove(move);
      shared_st_ptr_->position_history_.push_back(PositionRecord(*this));
    }
  }

  // 手を戻す。
  void ChessEngine::UndoMove() {
    if (shared_st_ptr_->move_history_.size() >= 1) {
      ply_--;
      Move move = shared_st_ptr_->move_history_.back();
      ply_100_ = shared_st_ptr_->ply_100_history_.back();
      shared_st_ptr_->move_history_.pop_back();
      shared_st_ptr_->ply_100_history_.pop_back();
      shared_st_ptr_->position_history_.pop_back();
      UnmakeMove(move);
    } else {
      throw SayuriError("手を戻すことができません。");
    }
  }

  // キャスリングできるかどうか。
  template<Castling Flag>
  bool ChessEngine::CanCastling() const {
    if (!(castling_rights_ & Flag)) return false;

    if (Flag == WHITE_SHORT_CASTLING) {
      if (king_[WHITE] != E1) return false;
      if (!(position_[WHITE][ROOK] & Util::SQUARE[H1])) return false;
      if (IsAttacked(E1, BLACK)) return false;
      if (IsAttacked(F1, BLACK)) return false;
      if (IsAttacked(G1, BLACK)) return false;
      if (piece_board_[F1]) return false;
      if (piece_board_[G1]) return false;
    } else if (Flag == WHITE_LONG_CASTLING) {
      if (king_[WHITE] != E1) return false;
      if (!(position_[WHITE][ROOK] & Util::SQUARE[A1])) return false;
      if (IsAttacked(E1, BLACK)) return false;
      if (IsAttacked(D1, BLACK)) return false;
      if (IsAttacked(C1, BLACK)) return false;
      if (piece_board_[D1]) return false;
      if (piece_board_[C1]) return false;
      if (piece_board_[B1]) return false;
    } else if (Flag == BLACK_SHORT_CASTLING) {
      if (king_[BLACK] != E8) return false;
      if (!(position_[BLACK][ROOK] & Util::SQUARE[H8])) return false;
      if (IsAttacked(E8, WHITE)) return false;
      if (IsAttacked(F8, WHITE)) return false;
      if (IsAttacked(G8, WHITE)) return false;
      if (piece_board_[F8]) return false;
      if (piece_board_[G8]) return false;
    } else if (Flag == BLACK_LONG_CASTLING){
      if (king_[BLACK] != E8) return false;
      if (!(position_[BLACK][ROOK] & Util::SQUARE[A8])) return false;
      if (IsAttacked(E8, WHITE)) return false;
      if (IsAttacked(D8, WHITE)) return false;
      if (IsAttacked(C8, WHITE)) return false;
      if (piece_board_[D8]) return false;
      if (piece_board_[C8]) return false;
      if (piece_board_[B8]) return false;
    } else {
      throw SayuriError("キャスリングのフラグが不正です。");
    }

    return true;
  }
  // 実体化。
  template bool ChessEngine::CanCastling<WHITE_SHORT_CASTLING>() const;
  template bool ChessEngine::CanCastling<WHITE_LONG_CASTLING>() const;
  template bool ChessEngine::CanCastling<BLACK_SHORT_CASTLING>() const;
  template bool ChessEngine::CanCastling<BLACK_LONG_CASTLING>() const;

  // 駒を動かす。
  void ChessEngine::MakeMove(Move& move) {
    // 動かす側のサイドを得る。
    Side side = to_move_;

    // 手番を反転させる。
    to_move_ = to_move_ ^ 0x3;

    // 動かす前のキャスリングの権利とアンパッサンを記録する。
    move_castling_rights(move, castling_rights_);
    move_en_passant_square(move, en_passant_square_);

    // アンパッサンを解除。
    en_passant_square_ = 0;

    // 手の要素を得る。
    Square from = move_from(move);
    Square to = move_to(move);
    Piece promotion = move_promotion(move);
    MoveType move_type = move_move_type(move);

    // NULL_MOVEならNull moveする。
    if (move_type == NULL_MOVE) {
      return;
    }

    // キャスリングの権利を更新。
    Piece piece = piece_board_[from];
    if (side == WHITE) {
      if (piece == KING) {
        castling_rights_ &= ~WHITE_CASTLING;
      } else if (piece == ROOK) {
        if (from == H1) {
          castling_rights_ &= ~WHITE_SHORT_CASTLING;
        } else if (from == A1) {
          castling_rights_ &= ~WHITE_LONG_CASTLING;
        }
      }
    } else {
      if (piece == KING) {
        castling_rights_ &= ~BLACK_CASTLING;
      } else if (piece == ROOK) {
        if (from == H8) {
          castling_rights_ &= ~BLACK_SHORT_CASTLING;
        } else if (from == A8) {
          castling_rights_ &= ~BLACK_LONG_CASTLING;
        }
      }
    }

    // 手の種類によって分岐する。
    if (move_type == CASTLING) {  // キャスリングの場合。
      // キングを動かす。
      ReplacePiece(from, to);
      // ルークを動かす。
      if (to == G1) {
        ReplacePiece(H1, F1);
      } else if (to == C1) {
        ReplacePiece(A1, D1);
      } else if (to == G8) {
        ReplacePiece(H8, F8);
      } else if (to == C8) {
        ReplacePiece(A8, D8);
      }
      has_castled_[side] = true;
    } else if (move_type == EN_PASSANT) {  // アンパッサンの場合。
      // 取った駒をボーンにする。
      move_captured_piece(move, PAWN);
      // 動かす。
      ReplacePiece(from, to);
      // アンパッサンのターゲットを消す。
      Square en_passant_target =
      side == WHITE ? to - 8 : to + 8;
      PutPiece(en_passant_target, EMPTY);
    } else {  // それ以外の場合。
      // 取る駒を登録する。
      move_captured_piece(move, piece_board_[to]);
      // 駒を動かす。
      ReplacePiece(from, to);
      // 駒を昇格させるなら、駒を昇格させる。
      if (promotion) {
        PutPiece(to, promotion, side);
      }
      // ポーンの2歩の動きの場合はアンパッサンできるようにする。
      if (piece_board_[to] == PAWN) {
        if (((side == WHITE) && ((from + 16) == to))
        || ((side == BLACK) && ((from - 16) == to))) {
          en_passant_square_ = side == WHITE ? to - 8 : to + 8;
        }
      }
    }
  }

  // 手を元に戻す。
  void ChessEngine::UnmakeMove(Move move) {
    // 相手のサイドを得る。
    Side enemy_side = to_move_;

    // 手番を反転させる。
    to_move_ ^=  0x3;

    // 動かす前のキャスリングの権利とアンパッサンを復元する。
    castling_rights_ = move_castling_rights(move);
    en_passant_square_ = move_en_passant_square(move);

    // 手の情報を得る。
    Square from = move_from(move);
    Square to = move_to(move);
    Piece promotion = move_promotion(move);
    MoveType move_type = move_move_type(move);

    // moveがNULL_MOVEなら返る。
    if (move_type == NULL_MOVE) {
      return;
    }

    // 駒の位置を戻す。
    ReplacePiece(to, from);

    // 手の種類で分岐する。
    if (move_type == CASTLING) {  // キャスリングの場合。
      // ルークを戻す。
      if (to == G1) {
        ReplacePiece(F1, H1);
      } else if (to == C1) {
        ReplacePiece(D1, A1);
      } else if (to == G8) {
        ReplacePiece(F8, H8);
      } else if (to == C8) {
        ReplacePiece(D8, A8);
      }
      has_castled_[to_move_] = false;
    } else if (move_type == EN_PASSANT) {  // アンパッサンの場合。
      // アンパッサンのターゲットを戻す。
      Square en_passant_target =
      to_move_ == WHITE ? en_passant_square_ - 8 : en_passant_square_ + 8;
      PutPiece(en_passant_target, PAWN, enemy_side);
    } else {  // それ以外の場合。
      // 取った駒を戻す。
      PutPiece(to, move_captured_piece(move), enemy_side);

      // 昇格ならポーンに戻す。
      if (promotion) {
        PutPiece(from, PAWN, to_move_);
      }
    }
  }

  // 攻撃されているかどうか調べる。
  bool ChessEngine::IsAttacked(Square square, Side side) const {
    // ポーンに攻撃されているかどうか調べる。
    Bitboard attack = Util::GetPawnAttack(square, side ^ 0x3);
    if (attack & position_[side][PAWN]) return true;

    // ナイトに攻撃されているかどうか調べる。
    attack = Util::GetKnightMove(square);
    if (attack & position_[side][KNIGHT]) return true;

    // ビショップとクイーンの斜めに攻撃されているかどうか調べる。
    attack = GetBishopAttack(square);
    if (attack & (position_[side][BISHOP] | position_[side][QUEEN]))
      return true;

    // ルークとクイーンの縦横に攻撃されているかどうか調べる。
    attack = GetRookAttack(square);
    if (attack & (position_[side][ROOK] | position_[side][QUEEN]))
      return true;

    // キングに攻撃されているかどうか調べる。
    attack = Util::GetKingMove(square);
    if (attack & position_[side][KING]) return true;

    // 何にも攻撃されていない。
    return false;
  }

  // マテリアルを得る。
  int ChessEngine::GetMaterial(Side side) const {
    // 相手のサイド。
    Side enemy_side = side ^ 0x3;

    int material = 0;
    for (Piece piece_type = PAWN; piece_type <= QUEEN; piece_type++) {
      material += MATERIAL[piece_type]
      * (Util::CountBits(position_[side][piece_type])
      - Util::CountBits(position_[enemy_side][piece_type]));
    }

    return material;
  }

  // 次の局面の自分のマテリアルを得る。
  int ChessEngine::GetNextMyMaterial(int current_material, Move move) const {
    if (move_move_type(move) == EN_PASSANT) {
      // アンパッサン。
      return current_material + MATERIAL[PAWN];
    } else if (Piece promotion = move_promotion(move)) {
      // プロモーション。
      return current_material + MATERIAL[piece_board_[move_to(move)]]
      + MATERIAL[promotion] - MATERIAL[PAWN];
    } else {
      // その他の手。
      return current_material + MATERIAL[piece_board_[move_to(move)]];
    }
  }

  // 現在の局面のハッシュを計算する。
  Hash ChessEngine::GetCurrentHash() const {
    Hash hash = 0ULL;

    // 駒の情報からハッシュを得る。
    for (Square square = 0U; square < NUM_SQUARES; square++) {
      hash ^= piece_hash_table_
      [side_board_[square]][piece_board_[square]][square];
    }

    // 手番からハッシュを得る。
    hash ^= to_move_hash_table_[to_move_];

    // キャスリングの権利からハッシュを得る。
    Castling bit = 1;
    for (int i = 0; i < 4; i++) {
      if (castling_rights_ & bit) {
        hash ^= castling_hash_table_[i];
      }
      bit <<= 1;
    }

    // アンパッサンからハッシュを得る。
    hash ^= en_passant_hash_table_[en_passant_square_];

    return hash;
  }

  // 次の局面のハッシュを得る。
  Hash ChessEngine::GetNextHash(Hash current_hash, Move move) const {
    // 駒の情報を得る。
    Square from = move_from(move);
    Square to = move_to(move);
    Piece promotion = move_promotion(move);

    // 駒の位置の種類とサイドを得る。
    Piece piece_type = piece_board_[from];
    Side piece_side = side_board_[from];

    // 取る駒の種類とサイドを得る。
    Piece target_type = piece_board_[to];
    Side target_side = side_board_[to];
    Square target_square = to;
    if ((piece_type == PAWN) && en_passant_square_
    && (to == en_passant_square_)) {
      // アンパッサンの時。
      if (piece_side == WHITE) {
        target_square = to - 8;
      } else {
        target_square = to + 8;
      }
      target_type = piece_board_[target_square];
    }

    // 移動する駒のハッシュを削除する。
    current_hash ^=
    piece_hash_table_[piece_side][piece_type][from];

    // 取る駒のハッシュを削除する。
    current_hash ^=
    piece_hash_table_[target_side][target_type][target_square];

    // 移動する駒の移動先のハッシュを追加する。
    if (promotion) {
      current_hash ^=
      piece_hash_table_[piece_side][promotion][to];
    } else {
      current_hash ^=
      piece_hash_table_[piece_side][piece_type][to];
    }

    // 現在の手番のハッシュを削除。
    current_hash ^= to_move_hash_table_[to_move_];

    // 次の手番のハッシュを追加。
    current_hash ^= to_move_hash_table_[to_move_ ^ 0x3];

    // キャスリングのハッシュをセット。
    Castling loss_rights = 0;
    if (piece_side == WHITE) {
      if (piece_type == KING) {
        loss_rights |= WHITE_CASTLING;
      } else if (piece_type == ROOK) {
        if (from == H1) {
          loss_rights |= WHITE_SHORT_CASTLING;
        } else if (from == A1) {
          loss_rights |= WHITE_LONG_CASTLING;
        }
      }
    } else {
      if (piece_type == KING) {
        loss_rights |= BLACK_CASTLING;
      } else if (piece_type == ROOK) {
        if (from == H8) {
          loss_rights |= BLACK_SHORT_CASTLING;
        } else if (from == A8) {
          loss_rights |= BLACK_LONG_CASTLING;
        }
      }
    }
    Castling castling_diff =
    (castling_rights_ & ~(loss_rights)) ^ castling_rights_;
    Castling bit = 1;
    for (int i = 0; i < 4; i++) {
      if (castling_diff & bit) {
        current_hash ^= castling_hash_table_[i];
      }
      bit <<= 1;
    }

    // とりあえずアンパッサンのハッシュを削除。
    current_hash ^= en_passant_hash_table_[en_passant_square_];

    // ポーンの2歩の動きの場合はアンパッサンハッシュを追加。
    if (piece_type == PAWN) {
      int move_diff = to - from;
      if (move_diff == 16) {
        current_hash ^= en_passant_hash_table_[to - 8];
      } else if (move_diff == -16) {
        current_hash ^= en_passant_hash_table_[to + 8];
      }
    }

    // 次の局面のハッシュを返す。
    return current_hash;
  }

  /******************************/
  /* その他のプライベート関数。 */
  /******************************/
  // 駒を置く。（駒の種類piece_typeにEMPTYをおけば、駒を削除できる。）
  void ChessEngine::PutPiece(Square square, Piece piece_type, Side side) {
    // 置く位置の現在の駒の種類を入手する。
    Piece placed_piece = piece_board_[square];

    // 置く位置の現在の駒のサイドを得る。
    Side placed_side = side_board_[square];

    // 置く位置のメンバを消す。
    if (placed_piece) {
      position_[placed_side][placed_piece] &= ~Util::SQUARE[square];
      side_pieces_[placed_side] &= ~Util::SQUARE[square];
    }

    // 置く駒がEMPTYか置くサイドがNO_SIDEなら
    // その位置のメンバを消して返る。
    if ((!piece_type) || (!side)) {
      piece_board_[square] = EMPTY;
      side_board_[square] = NO_SIDE;
      if (placed_piece) {
        blocker_0_ &= ~Util::SQUARE[square];
        blocker_45_ &= ~Util::SQUARE[Util::ROT45[square]];
        blocker_90_ &= ~Util::SQUARE[Util::ROT90[square]];
        blocker_135_ &= ~Util::SQUARE[Util::ROT135[square]];
      }
      return;
    }

    // 置く位置の駒の種類を書き変える。
    piece_board_[square] = piece_type;
    // 置く位置のサイドを書き変える。
    side_board_[square] = side;

    // 置く位置のビットボードをセットする。
    position_[side][piece_type] |= Util::SQUARE[square];
    side_pieces_[side] |= Util::SQUARE[square];
    blocker_0_ |= Util::SQUARE[square];
    blocker_45_ |= Util::SQUARE[Util::ROT45[square]];
    blocker_90_ |= Util::SQUARE[Util::ROT90[square]];
    blocker_135_ |= Util::SQUARE[Util::ROT135[square]];

    // キングの位置を更新する。
    if (piece_type == KING) {
      king_[side] = square;
    }
  }

  // 駒の位置を入れ替える。
  void ChessEngine::ReplacePiece(Square from, Square to) {
    // 移動する位置と移動先の位置が同じなら何もしない。
    if (from == to) return;

    // 移動。
    PutPiece(to, piece_board_[from], side_board_[from]);
    PutPiece(from, EMPTY, NO_SIDE);
  }

  /******************/
  /* ハッシュ関連。 */
  /******************/
  // ハッシュの配列を初期化する。
  void ChessEngine::InitHashTable() {
    // メルセンヌツイスターの準備。
    std::mt19937 engine(SysClock::to_time_t (SysClock::now()));
    std::uniform_int_distribution<Hash> dist(0ULL, -1ULL);

    // ダブリのないハッシュを生成。
    constexpr int LENGTH =
    NUM_SIDES + NUM_PIECE_TYPES + NUM_SQUARES + 1 + 4 + NUM_SQUARES;
    Hash temp_table[LENGTH];
    int temp_count = 0;
    for (int i = 0; i < LENGTH; i++) {
      // ダブリを調べる。
      bool loop = true;
      Hash hash = 0ULL;
      while (loop) {
        hash = dist(engine);
        loop = false;
        if (hash == 0ULL) {
          loop = true;
          continue;
        }
        for (int j = 0; j < i; j++) {
          if (hash == temp_table[j]) {
            loop = true;
            break;
          }
        }
      }

      temp_table[i] = hash;
    }

    // 駒の情報の配列を初期化。
    for (Side side = 0U; side < NUM_SIDES; side++) {
      for (Piece piece_type = 0U; piece_type < NUM_PIECE_TYPES;
      piece_type++) {
        for (Square square = 0U; square < NUM_SQUARES; square++) {
          if ((side == NO_SIDE) || (piece_type == EMPTY)) {
            piece_hash_table_[side][piece_type][square] = 0ULL;
          } else {
            piece_hash_table_[side][piece_type][square] =
            temp_table[temp_count];
            temp_count++;
          }
        }
      }
    }

    // 手番の配列を初期化。
    to_move_hash_table_[NO_SIDE] = 0ULL;
    to_move_hash_table_[WHITE] = 0ULL;
    to_move_hash_table_[BLACK] = temp_table[temp_count];
    temp_count++;

    // キャスリングの配列を初期化。
    // 0: 白のショートキャスリング。
    // 1: 白のロングキャスリング。
    // 2: 黒のショートキャスリング。
    // 3: 黒のロングキャスリング。
    for (int i = 0; i < 4; i++) {
      castling_hash_table_[i] = temp_table[temp_count];
      temp_count++;
    }

    // アンパッサンの配列を初期化。
    en_passant_hash_table_[0] = 0ULL;
    for (Square square = 1; square < NUM_SQUARES; square++) {
      en_passant_hash_table_[square] = temp_table[temp_count];
      temp_count++;
    }
  }

  /**********************/
  /* 共有メンバ構造体。 */
  /**********************/
  // コンストラクタ。
  ChessEngine::SharedStruct::SharedStruct() :
  history_max_(1ULL),
  i_depth_(1),
  num_searched_nodes_(0),
  stop_now_(false),
  max_nodes_(-1ULL),
  max_depth_(MAX_PLYS),
  thinking_time_(-1U >> 1),
  infinite_thinking_(false),
  move_history_(0),
  ply_100_history_(0),
  position_history_(0) {
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square from = 0; from < NUM_SQUARES; from++) {
        for (Square to = 0; to < NUM_SQUARES; to++) {
          history_[side][from][to] = 0;
        }
      }
    }
    for (unsigned int i = 0; i < (MAX_PLYS + 1); i++) {
      iid_stack_[i] = 0U;
      killer_stack_[i][0] = 0U;
      killer_stack_[i][1] = 0U;
      killer_stack_[i + 2][0] = 0U;
      killer_stack_[i + 2][1] = 0U;
    }
    helper_queue_ptr_.reset(new HelperQueue());
  }

  // コピーコンストラクタ。
  ChessEngine::SharedStruct::SharedStruct(const SharedStruct& shared_st) {
    ScanMember(shared_st);
  }

  // ムーブコンストラクタ。
  ChessEngine::SharedStruct::SharedStruct(SharedStruct&& shared_st) {
    ScanMember(shared_st);
  }

  // コピー代入。
  ChessEngine::SharedStruct& ChessEngine::SharedStruct::operator=
  (const SharedStruct& shared_st) {
    ScanMember(shared_st);
    return *this;
  }

  // ムーブ代入。
  ChessEngine::SharedStruct& ChessEngine::SharedStruct::operator=
  (SharedStruct&& shared_st) {
    ScanMember(shared_st);
    return *this;
  }

  // メンバをコピーする。
  void ChessEngine::SharedStruct::ScanMember(const SharedStruct& shared_st) {
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square from = 0; from < NUM_SQUARES; from++) {
        for (Square to = 0; to < NUM_SQUARES; to++) {
          history_[side][from][to] = shared_st.history_[side][from][to];
        }
      }
    }
    history_max_ = shared_st.history_max_;
    for (std::uint32_t i = 0U; i < (MAX_PLYS + 1); i++) {
      iid_stack_[i] = shared_st.iid_stack_[i];
      killer_stack_[i][0] = shared_st.killer_stack_[i][0];
      killer_stack_[i][0] = shared_st.killer_stack_[i][0];
      killer_stack_[i + 2][1] = shared_st.killer_stack_[i + 2][1];
      killer_stack_[i + 2][1] = shared_st.killer_stack_[i + 2][1];
    }
    i_depth_ = shared_st.i_depth_;
    num_searched_nodes_ = shared_st.num_searched_nodes_;
    start_time_ = shared_st.start_time_;
    stop_now_ = shared_st.stop_now_;
    max_nodes_ = shared_st.max_nodes_;
    max_depth_ = shared_st.max_depth_;
    thinking_time_ = shared_st.thinking_time_;
    infinite_thinking_ = shared_st.infinite_thinking_;
    move_history_ = shared_st.move_history_;
    ply_100_history_ = shared_st.ply_100_history_;
    position_history_ = shared_st.position_history_;
    helper_queue_ptr_.reset(new HelperQueue(*(shared_st.helper_queue_ptr_)));
  }
}  // namespace Sayuri
