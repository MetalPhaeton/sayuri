/*
   chess_engine.cpp: チェスボードの基本的な実装。

   The MIT License (MIT)

   Copyright (c) 2013 Ishibashi Hironori

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
  ChessEngine::ChessEngine() {
    SetNewGame();
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

  /********************
   * パブリック関数。 *
   ********************/
  void ChessEngine::LoadFen(const Fen& fen) {
    // キングの数がおかしいならやめる。
    int num_white_king = Util::CountBits(fen.position()[WHITE][KING]);
    int num_black_king = Util::CountBits(fen.position()[BLACK][KING]);
    if ((num_white_king != 1) || (num_black_king != 1)) return;

    // まず駒の配置を空にする。
    for (Bitboard bb = blocker_0_; bb; bb &= bb - 1) {
      Square square = Util::GetSquare(bb);
      PutPiece(square, EMPTY, NO_SIDE);
    }
          

    // 配置をする。
    for (Side side = WHITE; side < NUM_SIDES; side++) {
      for (Piece piece_type = PAWN; piece_type < NUM_PIECE_TYPES;
      piece_type++) {
        // 駒をセット。
        Bitboard bb = fen.position()[side][piece_type];
        for (; bb; bb &= bb - 1) {
          Square square = Util::GetSquare(bb);
          PutPiece(square, piece_type, side);
        }
      }
    }

    // 残りを設定。
    to_move_ = fen.to_move();
    castling_rights_ = fen.castling_rights();
    en_passant_square_ = fen.en_passant_square();
    can_en_passant_ = fen.can_en_passant();
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
    // まず駒の配置を空にする。
    for (Bitboard bb = blocker_0_; bb; bb &= bb - 1) {
      Square square = Util::GetSquare(bb);
      PutPiece(square, EMPTY, NO_SIDE);
    }
          

    // 配置をする。ついでにhas_castled_もセット。
    for (Side side = WHITE; side < NUM_SIDES; side++) {
      has_castled_[side] = record.has_castled()[side];
      for (Piece piece_type = PAWN; piece_type < NUM_PIECE_TYPES;
      piece_type++) {
        // 駒をセット。
        Bitboard bb = record.position()[side][piece_type];
        for (; bb; bb &= bb - 1) {
          Square square = Util::GetSquare(bb);
          PutPiece(square, piece_type, side);
        }
      }
    }

    // 残りを設定。
    to_move_ = record.to_move();
    castling_rights_ = record.castling_rights();
    en_passant_square_ = record.en_passant_square();
    can_en_passant_ = record.can_en_passant();
    ply_100_ = record.ply_100();
    ply_ = record.ply();
  }

  // 新しいゲームの準備をする。
  void ChessEngine::SetNewGame() {
    // 駒の配置を作る。
    // どちらでもない。
    for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      position_[NO_SIDE][piece_type] = 0;
    }
    // 白の駒。
    position_[WHITE][EMPTY] = 0ULL;
    position_[WHITE][PAWN] = Util::RANK[RANK_2];
    position_[WHITE][KNIGHT] = Util::BIT[B1] | Util::BIT[G1];
    position_[WHITE][BISHOP] = Util::BIT[C1] | Util::BIT[F1];
    position_[WHITE][ROOK] = Util::BIT[A1] | Util::BIT[H1];
    position_[WHITE][QUEEN] = Util::BIT[D1];
    position_[WHITE][KING] = Util::BIT[E1];
    // 黒の駒。
    position_[BLACK][EMPTY] = 0ULL;
    position_[BLACK][PAWN] = Util::RANK[RANK_7];
    position_[BLACK][KNIGHT] = Util::BIT[B8] | Util::BIT[G8];
    position_[BLACK][BISHOP] = Util::BIT[C8] | Util::BIT[F8];
    position_[BLACK][ROOK] = Util::BIT[A8] | Util::BIT[H8];
    position_[BLACK][QUEEN] = Util::BIT[D8];
    position_[BLACK][KING] = Util::BIT[E8];

    // 各サイドの駒の配置を作る。
    side_pieces_[NO_SIDE] = 0ULL;
    side_pieces_[WHITE] = 0ULL;
    side_pieces_[BLACK] = 0ULL;
    for (int i = PAWN; i < NUM_PIECE_TYPES; i++) {
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

      blocker_45_ |= Util::BIT[Util::ROT45[square]];
      blocker_90_ |= Util::BIT[Util::ROT90[square]];
      blocker_135_ |= Util::BIT[Util::ROT135[square]];
    }

    // 駒の種類とサイドの配置を作る。
    Bitboard point = 1ULL;
    for (int i = 0; i < NUM_SQUARES; i++) {
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
    can_en_passant_ = false;

    // 50手ルールを初期化。
    ply_100_ = 0;

    // 手数を初期化。
    ply_ = 1;

    // キャスリングしたかどうかの初期化。
    for (int i = 0; i < NUM_SIDES; i++) {
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
    // 駒の配置をコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = engine.position_[i][j];
      }
    }

    // 駒の種類の配置のコピー。
    for (int i = 0; i < NUM_SQUARES; i++) {
      piece_board_[i] = engine.piece_board_[i];
    }

    // サイドの配置のコピー。
    for (int i = 0; i < NUM_SQUARES; i++) {
      side_board_[i] = engine.side_board_[i];
    }

    // 各サイドの駒の配置のコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      side_pieces_[i] = engine.side_pieces_[i];
    }

    // ブロッカーのコピー。
    blocker_0_ = engine.blocker_0_;
    blocker_45_ = engine.blocker_45_;
    blocker_90_ = engine.blocker_90_;
    blocker_135_ = engine.blocker_135_;

    // キングの位置のコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      king_[i] = engine.king_[i];
    }

    // 手番のコピー。
    to_move_ = engine.to_move_;

    // キャスリングの権利のコピー。
    castling_rights_ = engine.castling_rights_;

    // アンパッサンのコピー。
    en_passant_square_ = engine.en_passant_square_;
    can_en_passant_ = engine.can_en_passant_;

    // 50手ルールの手数のコピー。
    ply_100_ = engine.ply_100_;

    // 手数のコピー。
    ply_ = engine.ply_;

    // キャスリングしたかどうかのコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      has_castled_[i] = engine.has_castled_[i];
    }
  }

  // 思考を始める。
  PVLine ChessEngine::Calculate(int num_threads, TranspositionTable& table,
  std::vector<Move>* moves_to_search_ptr) {
    num_threads = num_threads >= 1 ? num_threads : 1;
    thread_vec_.resize(num_threads);
    return std::move(SearchRoot(table, moves_to_search_ptr));
  }

  // 思考を停止する。
  void ChessEngine::StopCalculation() {
    shared_st_ptr_->stop_now_ = true;
  }

  // 手を指す。
  void ChessEngine::PlayMove(Move move) {
    // 合法手かどうか調べる。
    // 手を展開する。
    shared_st_ptr_->history_max_ = 1ULL;  // makerが0の乗算をしないように。
    MoveMaker maker(*this);
    maker.GenMoves<GenMoveType::ALL>(Move(), Move(), Move());
    // 合法手かどうか調べる。
    bool is_legal = false;
    Side side = to_move_;
    Side enemy_side = side ^ 0x3;
    for (Move temp_move = maker.PickMove(); temp_move.all_;
    temp_move = maker.PickMove()) {
      MakeMove(temp_move);
      // temp_moveが合法手かどうか調べる。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(temp_move);
        continue;
      }
      // temp_moveと同じ手かどうか調べる。
      if (move == temp_move) {
        UnmakeMove(temp_move);
        move = temp_move;
        is_legal = true;
        break;
      }
      UnmakeMove(temp_move);
    }

    if (is_legal) {
      ply_++;
      if ((piece_board_[move.from_] == PAWN)
      || (piece_board_[move.to_] != EMPTY)) {
        ply_100_ = 0;
      } else {
        ply_100_++;
      }
      shared_st_ptr_->move_history_.push_back(move);
      shared_st_ptr_->ply_100_history_.push_back(ply_100_);
      MakeMove(move);
      shared_st_ptr_->position_history_.push_back(PositionRecord(*this));
    } else {
      throw SayuriError("合法手ではありません。");
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
      if (!(position_[WHITE][ROOK] & Util::BIT[H1])) return false;
      if (IsAttacked(E1, BLACK)) return false;
      if (IsAttacked(F1, BLACK)) return false;
      if (IsAttacked(G1, BLACK)) return false;
      if (piece_board_[F1]) return false;
      if (piece_board_[G1]) return false;
    } else if (Flag == WHITE_LONG_CASTLING) {
      if (king_[WHITE] != E1) return false;
      if (!(position_[WHITE][ROOK] & Util::BIT[A1])) return false;
      if (IsAttacked(E1, BLACK)) return false;
      if (IsAttacked(D1, BLACK)) return false;
      if (IsAttacked(C1, BLACK)) return false;
      if (piece_board_[D1]) return false;
      if (piece_board_[C1]) return false;
      if (piece_board_[B1]) return false;
    } else if (Flag == BLACK_SHORT_CASTLING) {
      if (king_[BLACK] != E8) return false;
      if (!(position_[BLACK][ROOK] & Util::BIT[H8])) return false;
      if (IsAttacked(E8, WHITE)) return false;
      if (IsAttacked(F8, WHITE)) return false;
      if (IsAttacked(G8, WHITE)) return false;
      if (piece_board_[F8]) return false;
      if (piece_board_[G8]) return false;
    } else if (Flag == BLACK_LONG_CASTLING){
      if (king_[BLACK] != E8) return false;
      if (!(position_[BLACK][ROOK] & Util::BIT[A8])) return false;
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
    move.last_castling_rights_ = castling_rights_;
    move.last_can_en_passant_ = can_en_passant_;
    move.last_en_passant_square_ = en_passant_square_;

    // NULL_MOVEならNull moveする。
    if (move.move_type_ == NULL_MOVE) {
      can_en_passant_ = false;
      return;
    }

    // 移動前と移動後の位置を得る。
    // 移動前と移動後が同じならNull Move。
    if (move.from_ == move.to_) {
      move.move_type_ = NULL_MOVE;
      can_en_passant_ = false;
      return;
    }

    // 手の種類によって分岐する。
    if (move.move_type_ == CASTLING) {  // キャスリングの場合。
      // キングを動かす。
      ReplacePiece(move.from_, move.to_);
      // ルークを動かす。
      if (move.to_ == G1) {
        ReplacePiece(H1, F1);
      } else if (move.to_ == C1) {
        ReplacePiece(A1, D1);
      } else if (move.to_ == G8) {
        ReplacePiece(H8, F8);
      } else if (move.to_ == C8) {
        ReplacePiece(A8, D8);
      }
      has_castled_[side] = true;
      can_en_passant_ = false;
    } else if (move.move_type_ == EN_PASSANT) {  // アンパッサンの場合。
      // 取った駒をボーンにする。
      move.captured_piece_ = PAWN;
      // 動かす。
      ReplacePiece(move.from_, move.to_);
      // アンパッサンのターゲットを消す。
      Square en_passant_target =
      side == WHITE ? en_passant_square_ - 8 : en_passant_square_ + 8;
      PutPiece(en_passant_target, EMPTY);

      can_en_passant_ = false;
    } else {  // それ以外の場合。
      // 取る駒を登録する。
      move.captured_piece_ = piece_board_[move.to_];
      // 駒を動かす。
      ReplacePiece(move.from_, move.to_);
      // 駒を昇格させるなら、駒を昇格させる。
      if (move.promotion_) {
        PutPiece(move.to_, move.promotion_, side);
      }
      // ポーンの2歩の動きの場合はアンパッサンできるようにする。
      if (piece_board_[move.to_] == PAWN) {
        if (((side == WHITE) && ((move.from_ + 16) == move.to_))
        || ((side == BLACK) && ((move.from_ - 16) == move.to_))) {
          can_en_passant_ = true;
          en_passant_square_ = side == WHITE ? move.to_ - 8 : move.to_ + 8;
        } else {
          can_en_passant_ = false;
        }
      } else {
        can_en_passant_ = false;
      }
    }

    UpdateCastlingRights();
  }

  // 手を元に戻す。
  void ChessEngine::UnmakeMove(Move move) {
    // 相手のサイドを得る。
    Side enemy_side = to_move_;

    // 手番を反転させる。
    to_move_ ^=  0x3;

    // 動かす前のキャスリングの権利とアンパッサンを復元する。
    castling_rights_ = move.last_castling_rights_;
    can_en_passant_ = move.last_can_en_passant_;
    en_passant_square_ = move.last_en_passant_square_;

    // moveがNULL_MOVEなら返る。
    if (move.move_type_ == NULL_MOVE) {
      return;
    }

    // 駒の位置を戻す。
    ReplacePiece(move.to_, move.from_);

    // 手の種類で分岐する。
    if (move.move_type_ == CASTLING) {  // キャスリングの場合。
      // ルークを戻す。
      if (move.to_ == G1) {
        ReplacePiece(F1, H1);
      } else if (move.to_ == C1) {
        ReplacePiece(D1, A1);
      } else if (move.to_ == G8) {
        ReplacePiece(F8, H8);
      } else if (move.to_ == C8) {
        ReplacePiece(D8, A8);
      }
      has_castled_[to_move_] = false;
    } else if (move.move_type_ == EN_PASSANT) {  // アンパッサンの場合。
      // アンパッサンのターゲットを戻す。
      Square en_passant_target =
      to_move_ == WHITE ? en_passant_square_ - 8 : en_passant_square_ + 8;
      PutPiece(en_passant_target, move.captured_piece_, enemy_side);
    } else {  // それ以外の場合。
      // 取った駒を戻す。
      if (move.captured_piece_) {
        PutPiece(move.to_, move.captured_piece_, enemy_side);
      }
      // 昇格ならポーンに戻す。
      if (move.promotion_) {
        PutPiece(move.from_, PAWN, to_move_);
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

  // 現在の局面のハッシュを計算する。
  Hash ChessEngine::GetCurrentHash() const {
    Hash hash = 0ULL;

    // 駒の情報からハッシュを得る。
    for (int square = 0; square < NUM_SQUARES; square++) {
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
    if (can_en_passant_) {
      hash ^= en_passant_hash_table_[en_passant_square_];
    }

    return hash;
  }

  // 次の局面のハッシュを得る。
  Hash ChessEngine::GetNextHash(Hash current_hash, Move move) const {
    // 駒の位置の種類とサイドを得る。
    Piece piece_type = piece_board_[move.from_];
    Side piece_side = side_board_[move.from_];

    // 取る駒の種類とサイドを得る。
    Piece target_type = piece_board_[move.to_];
    Side target_side = side_board_[move.to_];
    Square target_square = move.to_;
    if ((piece_type == PAWN) && can_en_passant_
    && (move.to_ == en_passant_square_)) {
      // アンパッサンの時。
      if (piece_side == WHITE) {
        target_square = move.to_ - 8;
      } else {
        target_square = move.to_ + 8;
      }
      target_type = piece_board_[target_square];
    }

    // 移動する駒のハッシュを得る。
    Hash piece_hash =
    piece_hash_table_[piece_side][piece_type][move.from_];

    // 取る駒のハッシュを得る。
    Hash target_hash =
    piece_hash_table_[target_side][target_type][target_square];

    // 移動する駒の移動先のハッシュを得る。
    Hash move_hash;
    if (move.promotion_) {
      move_hash =
      piece_hash_table_[piece_side][move.promotion_][move.to_];
    } else {
      move_hash =
      piece_hash_table_[piece_side][piece_type][move.to_];
    }

    // 移動する駒のハッシュを削除する。
    current_hash ^= piece_hash;

    // 取る駒のハッシュを削除する。
    current_hash ^= target_hash;

    // 移動する駒の移動先のハッシュを追加する。
    current_hash ^= move_hash;

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
        if (move.from_ == H1) {
          loss_rights |= WHITE_SHORT_CASTLING;
        } else if (move.from_ == A1) {
          loss_rights |= WHITE_LONG_CASTLING;
        }
      }
    } else {
      if (piece_type == KING) {
        loss_rights |= BLACK_CASTLING;
      } else if (piece_type == ROOK) {
        if (move.from_ == H8) {
          loss_rights |= BLACK_SHORT_CASTLING;
        } else if (move.from_ == A8) {
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

    // アンパッサンのマスからハッシュを得る。
    if (can_en_passant_) {
      // とりあえずアンパッサンのハッシュを削除。
      current_hash ^= en_passant_hash_table_[en_passant_square_];
    }
    // ポーンの2歩の動きの場合はアンパッサンハッシュを追加。
    if (piece_type == PAWN) {
      int move_diff = move.to_ - move.from_;
      if (move_diff > 8) {
        current_hash ^= en_passant_hash_table_[move.to_ - 8];
      } else if (move_diff < -8) {
        current_hash ^= en_passant_hash_table_[move.to_ + 8];
      }
    }

    // 次の局面のハッシュを返す。
    return current_hash;
  }

  /******************************
   * その他のプライベート関数。 *
   ******************************/
  // 駒を置く。（駒の種類piece_typeにEMPTYをおけば、駒を削除できる。）
  void ChessEngine::PutPiece(Square square, Piece piece_type, Side side) {
    // 置く位置の現在の駒の種類を入手する。
    Piece placed_piece = piece_board_[square];

    // 置く位置の現在の駒のサイドを得る。
    Side placed_side = side_board_[square];

    // 置く位置のメンバを消す。
    if (placed_piece) {
      position_[placed_side][placed_piece] &= ~Util::BIT[square];
      side_pieces_[placed_side] &= ~Util::BIT[square];
    }

    // 置く駒がEMPTYか置くサイドがNO_SIDEなら
    // その位置のメンバを消して返る。
    if ((!piece_type) || (!side)) {
      piece_board_[square] = EMPTY;
      side_board_[square] = NO_SIDE;
      if (placed_piece) {
        blocker_0_ &= ~Util::BIT[square];
        blocker_45_ &= ~Util::BIT[Util::ROT45[square]];
        blocker_90_ &= ~Util::BIT[Util::ROT90[square]];
        blocker_135_ &= ~Util::BIT[Util::ROT135[square]];
      }
      return;
    }

    // 置く位置の駒の種類を書き変える。
    piece_board_[square] = piece_type;
    // 置く位置のサイドを書き変える。
    side_board_[square] = side;

    // 置く位置のビットボードをセットする。
    position_[side][piece_type] |= Util::BIT[square];
    side_pieces_[side] |= Util::BIT[square];
    blocker_0_ |= Util::BIT[square];
    blocker_45_ |= Util::BIT[Util::ROT45[square]];
    blocker_90_ |= Util::BIT[Util::ROT90[square]];
    blocker_135_ |= Util::BIT[Util::ROT135[square]];

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

  // キャスリングの権利を更新する。
  void ChessEngine::UpdateCastlingRights() {
    // 白キングがe1にいなければ白のキャスリングの権利を放棄。
    if (king_[WHITE] != E1)
      castling_rights_ &= ~WHITE_CASTLING;

    // 黒キングがe8にいなければ黒のキャスリングの権利を放棄。
    if (king_[BLACK] != E8) castling_rights_ &= ~BLACK_CASTLING;

    // 白のルークがh1にいなければ白のショートキャスリングの権利を放棄。
    if (!(position_[WHITE][ROOK] & Util::BIT[H1]))
      castling_rights_ &= ~WHITE_SHORT_CASTLING;
    // 白のルークがa1にいなければ白のロングキャスリングの権利を放棄。
    if (!(position_[WHITE][ROOK] & Util::BIT[A1]))
      castling_rights_ &= ~WHITE_LONG_CASTLING;

    // 黒のルークがh8にいなければ黒のショートキャスリングの権利を放棄。
    if (!(position_[BLACK][ROOK] & Util::BIT[H8]))
      castling_rights_ &= ~BLACK_SHORT_CASTLING;
    // 黒のルークがa8にいなければ黒のロングキャスリングの権利を放棄。
    if (!(position_[BLACK][ROOK] & Util::BIT[A8]))
      castling_rights_ &= ~BLACK_LONG_CASTLING;
  }
  /******************/
  /* ハッシュ関連。 */
  /******************/
  // ハッシュの配列を初期化する。
  void ChessEngine::InitHashTable() {
    // メルセンヌツイスターの準備。
    std::mt19937 engine(SysClock::to_time_t (SysClock::now()));
    std::uniform_int_distribution<Hash> dist(0ULL, 0xffffffffffffffffULL);

    // 駒の情報の配列を初期化。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        for (int square = 0; square < NUM_SQUARES; square++) {
          if ((side == NO_SIDE) || (piece_type == EMPTY)) {
            piece_hash_table_[side][piece_type][square] = 0ULL;
          } else {
            piece_hash_table_[side][piece_type][square] = dist(engine);
          }
        }
      }
    }

    // 手番の配列を初期化。
    to_move_hash_table_[NO_SIDE] = 0ULL;
    to_move_hash_table_[WHITE] = 0ULL;
    to_move_hash_table_[BLACK] = dist(engine);

    // キャスリングの配列を初期化。
    // 0: 白のショートキャスリング。
    // 1: 白のロングキャスリング。
    // 2: 黒のショートキャスリング。
    // 3: 黒のロングキャスリング。
    for (int i = 0; i < 4; i++) {
      castling_hash_table_[i] = dist(engine);
    }

    // アンパッサンの配列を初期化。
    for (int square = 0; square < NUM_SQUARES; square++) {
      en_passant_hash_table_[square] = dist(engine);
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
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = 0;
        }
      }
    }
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = Move();
      killer_stack_[i] = Move();
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
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = shared_st.history_[i][j][k];
        }
      }
    }
    history_max_ = shared_st.history_max_;
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = shared_st.iid_stack_[i];
      killer_stack_[i] = shared_st.killer_stack_[i];
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
