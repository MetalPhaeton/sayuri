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
 * @file chess_engine.cpp
 * @author Hironori Ishibashi
 * @brief チェスエンジンの本体の実装。
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
#include "common.h"
#include "transposition_table.h"
#include "fen.h"
#include "move_maker.h"
#include "pv_line.h"
#include "uci_shell.h"
#include "position_record.h"
#include "job.h"
#include "helper_queue.h"
#include "params.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  ChessEngine::ChessEngine(const SearchParams& search_params,
  const EvalParams& eval_params) :
  is_null_searching_(false),
  my_job_ptr_(nullptr),
  is_job_ended_(false),
  evaluator_(*this) {
    SetNewGame();

    // 探索関数用パラメータ。
    shared_st_ptr_->search_params_ptr_ = &search_params;
    // 評価関数用パラメータ。
    shared_st_ptr_->eval_params_ptr_ = &eval_params;

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      job_table_[i].client_ptr_ = this;
    }

    // PositionRecord。
    record_table_.reset(new PositionRecord[MAX_PLYS + 1]);
  }

  // プライベートコンストラクタ。
  ChessEngine::ChessEngine() : 
  is_null_searching_(false),
  my_job_ptr_(nullptr),
  is_job_ended_(false),
  evaluator_(*this) {
    SetNewGame();

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      job_table_[i].client_ptr_ = this;
    }

    // PositionRecord。
    record_table_.reset(new PositionRecord[MAX_PLYS + 1]);
  }

  // コピーコンストラクタ。
  ChessEngine::ChessEngine(const ChessEngine& engine) :
  is_null_searching_(false),
  my_job_ptr_(nullptr),
  is_job_ended_(false),
  evaluator_(*this) {
    // 基本メンバをコピー。
    ScanBasicMember(engine);

    // 共有メンバのコピー。
    shared_st_ptr_.reset(new SharedStruct(*(engine.shared_st_ptr_)));

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      job_table_[i].client_ptr_ = this;
    }

    // PositionRecord。
    record_table_.reset(new PositionRecord[MAX_PLYS + 1]);
  }

  // ムーブコンストラクタ。
  ChessEngine::ChessEngine(ChessEngine&& engine) :
  is_null_searching_(false),
  my_job_ptr_(nullptr),
  is_job_ended_(false),
  evaluator_(*this) {
    // 基本メンバをコピー。
    ScanBasicMember(engine);

    // 共有メンバのムーブ。
    shared_st_ptr_ = std::move(shared_st_ptr_);

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      job_table_[i].client_ptr_ = this;
    }

    // PositionRecord。
    record_table_.reset(new PositionRecord[MAX_PLYS + 1]);
  }

  // コピー代入演算子。
  ChessEngine& ChessEngine::operator=(const ChessEngine& engine) {
    // 基本メンバをコピー。
    ScanBasicMember(engine);

    // 共有メンバをコピー。
    shared_st_ptr_.reset(new SharedStruct(*(engine.shared_st_ptr_)));

    return *this;
  }

  // ムーブ代入演算子。
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

  // ========================= //
  // ChessEngineクラスの初期化 //
  // ========================= //
  // static変数の初期化。
  void ChessEngine::InitChessEngine() {
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // FEN文字列を読み込む。
  void ChessEngine::LoadFEN(const FEN& fen) {
    // キングの数がおかしいならやめる。
    int num_white_king = Util::CountBits(fen.position()[WHITE][KING]);
    int num_black_king = Util::CountBits(fen.position()[BLACK][KING]);
    if ((num_white_king != 1) || (num_black_king != 1)) return;

    // 空にする。
    for (Square square = 0; square < NUM_SQUARES; square++) {
      piece_board_[square] = EMPTY;
      side_board_[square] = NO_SIDE;
    }
    blocker_0_ = 0;
    blocker_45_ = 0;
    blocker_90_ = 0;
    blocker_135_ = 0;

    // 駒を配置する。
    for (Side side = WHITE; side <= BLACK; side++) {
      side_pieces_[side] = 0;
      for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
        position_[side][piece_type] = fen.position()[side][piece_type];
        for (Bitboard bb = position_[side][piece_type]; bb; NEXT(bb)) {
          Square square = Util::GetSquare(bb);
          side_board_[square] = side;
          piece_board_[square] = piece_type;
          side_pieces_[side] |= Util::SQUARE[square];
          blocker_0_ |= Util::SQUARE[square];
          blocker_45_ |= Util::SQUARE[Util::ROT45[square]];
          blocker_90_ |= Util::SQUARE[Util::ROT90[square]];
          blocker_135_ |= Util::SQUARE[Util::ROT135[square]];
          if (piece_type == KING) {
            king_[side] = square;
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
    shared_st_ptr_->position_history_.push_back
    (PositionRecord(*this, GetCurrentHash()));
  }

  // PositionRecordから局面読み込む。
  void ChessEngine::LoadRecord(const PositionRecord& record) {
    // サイド毎のコピー。
    for (Side side = WHITE; side <= BLACK; side++) {
      for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
        position_[side][piece_type] = record.position()[side][piece_type];
      }
      side_pieces_[side] = record.side_pieces()[side];
      king_[side] = record.king()[side];
      has_castled_[side] = record.has_castled()[side];
    }

    // マス毎のコピー。
    for (Square square = A1; square <= H8; square++) {
      piece_board_[square] = record.piece_board()[square];
      side_board_[square] = record.side_board()[square];
    }

    // 全駒のコピー。
    blocker_0_ = record.blocker_0();
    blocker_45_ = record.blocker_45();
    blocker_90_ = record.blocker_90();
    blocker_135_ = record.blocker_135();

    // その他のコピー。
    to_move_ = record.to_move();
    castling_rights_ = record.castling_rights();
    en_passant_square_ = record.en_passant_square();
    ply_100_ = record.ply_100();
    ply_ = record.ply();
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      position_memo_[i] = record.position_memo()[i];
    }
  }

  // 駒を初期配置にセットする。
  void ChessEngine::SetStartPosition() {
    // 駒の配置を作る。
    // どちらでもない。
    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      position_[NO_SIDE][piece_type] = 0;
    }
    // 白の駒。
    position_[WHITE][EMPTY] = 0;
    position_[WHITE][PAWN] = Util::RANK[RANK_2];
    position_[WHITE][KNIGHT] = Util::SQUARE[B1] | Util::SQUARE[G1];
    position_[WHITE][BISHOP] = Util::SQUARE[C1] | Util::SQUARE[F1];
    position_[WHITE][ROOK] = Util::SQUARE[A1] | Util::SQUARE[H1];
    position_[WHITE][QUEEN] = Util::SQUARE[D1];
    position_[WHITE][KING] = Util::SQUARE[E1];
    // 黒の駒。
    position_[BLACK][EMPTY] = 0;
    position_[BLACK][PAWN] = Util::RANK[RANK_7];
    position_[BLACK][KNIGHT] = Util::SQUARE[B8] | Util::SQUARE[G8];
    position_[BLACK][BISHOP] = Util::SQUARE[C8] | Util::SQUARE[F8];
    position_[BLACK][ROOK] = Util::SQUARE[A8] | Util::SQUARE[H8];
    position_[BLACK][QUEEN] = Util::SQUARE[D8];
    position_[BLACK][KING] = Util::SQUARE[E8];

    // 各サイドの駒の配置を作る。
    side_pieces_[NO_SIDE] = 0;
    side_pieces_[WHITE] = 0;
    side_pieces_[BLACK] = 0;
    for (Piece piece_type = PAWN; piece_type < NUM_PIECE_TYPES; piece_type++) {
      side_pieces_[WHITE] |= position_[WHITE][piece_type];
      side_pieces_[BLACK] |= position_[BLACK][piece_type];
    }

    // ブロッカーのビットボードを作る。
    blocker_0_ = side_pieces_[WHITE] | side_pieces_[BLACK];
    blocker_45_ = 0;
    blocker_90_ = 0;
    blocker_135_ = 0;
    for (Bitboard copy = blocker_0_; copy; NEXT(copy)) {
      Square square = Util::GetSquare(copy);

      blocker_45_ |= Util::SQUARE[Util::ROT45[square]];
      blocker_90_ |= Util::SQUARE[Util::ROT90[square]];
      blocker_135_ |= Util::SQUARE[Util::ROT135[square]];
    }

    // 駒の種類とサイドの配置を作る。
    Bitboard point = 1;
    for (Square square = 0; square < NUM_SQUARES; square++) {
      // サイドの配置。
      if (side_pieces_[WHITE] & point) side_board_[square] = WHITE;
      else if (side_pieces_[BLACK] & point) side_board_[square] = BLACK;
      else side_board_[square] = NO_SIDE;

      // 駒の種類の配置。
      if ((point & position_[WHITE][PAWN])
      || (point & position_[BLACK][PAWN])) {
        piece_board_[square] = PAWN;
      } else if ((point & position_[WHITE][KNIGHT])
      || (point & position_[BLACK][KNIGHT])) {
        piece_board_[square] = KNIGHT;
      } else if ((point & position_[WHITE][BISHOP])
      || (point & position_[BLACK][BISHOP])) {
        piece_board_[square] = BISHOP;
      } else if ((point & position_[WHITE][ROOK])
      || (point & position_[BLACK][ROOK])) {
        piece_board_[square] = ROOK;
      } else if ((point & position_[WHITE][QUEEN])
      || (point & position_[BLACK][QUEEN])) {
        piece_board_[square] = QUEEN;
      } else if ((point & position_[WHITE][KING])
      || (point & position_[BLACK][KING])) {
        piece_board_[square] = KING;
      } else {
        piece_board_[square] = EMPTY;
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
    for (Side side = 0; side < NUM_SIDES; side++) {
      has_castled_[side] = false;
    }

    // 駒の配置のメモを初期化。
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      position_memo_[i] = 0;
    }

    if (shared_st_ptr_) {
      // 50手ルールの履歴を初期化。
      shared_st_ptr_->ply_100_history_.clear();
      shared_st_ptr_->ply_100_history_.push_back(0);

      // 駒の配置の履歴を初期化。
      shared_st_ptr_->position_history_.clear();
      shared_st_ptr_->position_history_.push_back
      (PositionRecord(*this, GetCurrentHash()));
    }
  }

  // 新しいゲームの準備をする。
  void ChessEngine::SetNewGame() {
    // 駒の配置を初期化。
    SetStartPosition();

    // 共有メンバ構造体を初期化。
    const SearchParams* temp_sp_ptr = nullptr;
    const EvalParams* temp_ep_ptr = nullptr;
    if (shared_st_ptr_) {
      temp_sp_ptr = shared_st_ptr_->search_params_ptr_;  // 一時待避。
      temp_ep_ptr = shared_st_ptr_->eval_params_ptr_;  // 一時待避。
    }
    shared_st_ptr_.reset(new SharedStruct());
    shared_st_ptr_->search_params_ptr_ = temp_sp_ptr;  // 復帰。
    shared_st_ptr_->eval_params_ptr_ = temp_ep_ptr;  // 復帰。

    // 50手ルールの履歴を初期化。
    shared_st_ptr_->ply_100_history_.push_back(0);

    // 駒の配置の履歴を初期化。
    shared_st_ptr_->position_history_.push_back
    (PositionRecord(*this, GetCurrentHash()));
  }

  // 他のエンジンの基本メンバをコピーする。
  void ChessEngine::ScanBasicMember(const ChessEngine& engine) {
    // サイド毎のコピー。
    for (Side side = 0; side < NUM_SIDES; side++) {
      // 駒の配置をコピー。
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = engine.position_[side][piece_type];
      }

      // 各サイドの駒の配置のコピー。
      side_pieces_[side] = engine.side_pieces_[side];

      // キングの位置のコピー。
      king_[side] = engine.king_[side];

      // キャスリングしたかどうかのコピー。
      has_castled_[side] = engine.has_castled_[side];
    }

    // マス毎のコピー。
    for (Square square = 0; square < NUM_SQUARES; square++) {
      // 駒の種類の配置のコピー。
      piece_board_[square] = engine.piece_board_[square];

      // サイドの配置のコピー。
      side_board_[square] = engine.side_board_[square];
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

    // 駒の配置のメモをコピー。
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      position_memo_[i] = engine.position_memo_[i];
    }
  }

  // 探索を開始する。
  PVLine ChessEngine::Calculate(int num_threads, TranspositionTable& table,
  const std::vector<Move>& moves_to_search, UCIShell& shell) {
    num_threads = MAX(num_threads, 1);
    num_threads = MIN(num_threads, UCI_MAX_THREADS);
    thread_vec_.resize(num_threads);
    return std::move(SearchRoot(table, moves_to_search, shell));
  }

  // 探索を終了させる。
  void ChessEngine::StopCalculation() {
    shared_st_ptr_->stop_now_ = true;
  }

  // 手を指す。
  void ChessEngine::PlayMove(Move move) {
    // 合法手かどうか調べる。
    // 手を展開する。
    shared_st_ptr_->history_max_ = 1;  // makerが0の除算をしないように。
    MoveMaker maker(*this);
    maker.GenMoves<GenMoveType::ALL>(0, 0, 0, 0);
    // 合法手かどうか調べる。
    bool is_legal = false;
    Side side = to_move_;
    Side enemy_side = OPPOSITE_SIDE(side);
    for (Move temp_move = maker.PickMove(); temp_move;
    temp_move = maker.PickMove()) {
      MakeMove(temp_move);
      // temp_moveが合法手かどうか調べる。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(temp_move);
        continue;
      }
      // temp_moveと同じ手かどうか調べる。
      if (EQUAL_MOVE(move, temp_move)) {
        UnmakeMove(temp_move);
        move = temp_move;
        is_legal = true;
        break;
      }
      UnmakeMove(temp_move);
    }

    if (is_legal) {
      ply_++;
      if ((piece_board_[GET_FROM(move)] == PAWN)
      || (piece_board_[GET_TO(move)] != EMPTY)) {
        ply_100_ = 0;
      } else {
        ply_100_++;
      }
      shared_st_ptr_->move_history_.push_back(move);
      shared_st_ptr_->ply_100_history_.push_back(ply_100_);
      MakeMove(move);
      shared_st_ptr_->position_history_.push_back
      (PositionRecord(*this, GetCurrentHash()));
    }
  }

  // 1手戻す。
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
      throw SayuriError("手を戻すことができません。 ChessEngine::UndoMove()");
    }
  }

  // キャスリング出来るかどうかを判定する。
  template<Castling Flag>
  bool ChessEngine::CanCastling() const {
    if (!(castling_rights_ & Flag)) return false;

    if (Flag == WHITE_SHORT_CASTLING) {
      if (IsAttacked(E1, BLACK)) return false;
      if (IsAttacked(F1, BLACK)) return false;
      if (IsAttacked(G1, BLACK)) return false;
      if (piece_board_[F1]) return false;
      if (piece_board_[G1]) return false;
    } else if (Flag == WHITE_LONG_CASTLING) {
      if (IsAttacked(E1, BLACK)) return false;
      if (IsAttacked(D1, BLACK)) return false;
      if (IsAttacked(C1, BLACK)) return false;
      if (piece_board_[D1]) return false;
      if (piece_board_[C1]) return false;
      if (piece_board_[B1]) return false;
    } else if (Flag == BLACK_SHORT_CASTLING) {
      if (IsAttacked(E8, WHITE)) return false;
      if (IsAttacked(F8, WHITE)) return false;
      if (IsAttacked(G8, WHITE)) return false;
      if (piece_board_[F8]) return false;
      if (piece_board_[G8]) return false;
    } else if (Flag == BLACK_LONG_CASTLING){
      if (IsAttacked(E8, WHITE)) return false;
      if (IsAttacked(D8, WHITE)) return false;
      if (IsAttacked(C8, WHITE)) return false;
      if (piece_board_[D8]) return false;
      if (piece_board_[C8]) return false;
      if (piece_board_[B8]) return false;
    }

    return true;
  }
  // 実体化。
  template bool ChessEngine::CanCastling<WHITE_SHORT_CASTLING>() const;
  template bool ChessEngine::CanCastling<WHITE_LONG_CASTLING>() const;
  template bool ChessEngine::CanCastling<BLACK_SHORT_CASTLING>() const;
  template bool ChessEngine::CanCastling<BLACK_LONG_CASTLING>() const;

  // 次の手を指す。
  void ChessEngine::MakeMove(Move& move) {
    // 動かす側のサイドを得る。
    Side side = to_move_;

    // 手番を反転させる。
    to_move_ = OPPOSITE_SIDE(to_move_);

    // 動かす前のキャスリングの権利とアンパッサンを記録する。
    SET_CASTLING_RIGHTS(move, castling_rights_);
    SET_EN_PASSANT_SQUARE(move, en_passant_square_);

    // アンパッサンを解除。
    en_passant_square_ = 0;

    // 手の要素を得る。
    Square from = GET_FROM(move);
    Square to = GET_TO(move);
    Piece promotion = GET_PROMOTION(move);
    MoveType move_type = GET_MOVE_TYPE(move);

    // キャスリングの権利を更新。
    Piece piece = piece_board_[from];
    if ((castling_rights_ & WHITE_CASTLING) && (side == WHITE)) {
      if (piece == KING) {
        castling_rights_ &= ~WHITE_CASTLING;
      } else if (piece == ROOK) {
        if ((castling_rights_ & WHITE_SHORT_CASTLING) && (from == H1)) {
          castling_rights_ &= ~WHITE_SHORT_CASTLING;
        } else if ((castling_rights_ & WHITE_LONG_CASTLING) && (from == A1)) {
          castling_rights_ &= ~WHITE_LONG_CASTLING;
        }
      }
    } else if ((castling_rights_ & BLACK_CASTLING) && (side == BLACK)) {
      if (piece == KING) {
        castling_rights_ &= ~BLACK_CASTLING;
      } else if (piece == ROOK) {
        if ((castling_rights_ & BLACK_SHORT_CASTLING) && (from == H8)) {
          castling_rights_ &= ~BLACK_SHORT_CASTLING;
        } else if ((castling_rights_ & BLACK_LONG_CASTLING) && (from == A8)) {
          castling_rights_ &= ~BLACK_LONG_CASTLING;
        }
      }
    }

    // 手の種類によって分岐する。
    if (move_type == NORMAL) {
      // 取る駒を登録する。
      SET_CAPTURED_PIECE(move, piece_board_[to]);
      // 駒を動かす。
      ReplacePiece(from, to);
      // 駒を昇格させるなら、駒を昇格させる。
      if (promotion) {
        PutPiece(to, promotion, side);
      }
      // ポーンの2歩の動きの場合はアンパッサンできるようにする。
      // temp[][]は計算時間の省略のための、2歩の動きの判定用テーブル。
      static const Square temp[NUM_SIDES][NUM_SQUARES] {
        {
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U
        },
        {
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          A4, B4, C4, D4, E4, F4, G4, H4,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U
        },
        {
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
          A5, B5, C5, D5, E5, F5, G5, H5,
          -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U
        }
      };
      if (piece_board_[to] == PAWN) {
        if (temp[side][from] == to) {
          en_passant_square_ = Util::TO_EN_PASSANT_SQUARE[side][to];
        }
      }
    } else if (move_type == EN_PASSANT) {  // アンパッサンの場合。
      // 取った駒をボーンにする。
      SET_CAPTURED_PIECE(move, PAWN);
      // 動かす。
      ReplacePiece(from, to);
      // アンパッサンのターゲットを消す。
      PutPiece(Util::TO_EN_PASSANT_TARGET[side][to], EMPTY);
    } else if (move_type == CASTLE_WS) {  // 白のショートキャスリング。
      // キングを動かす。
      ReplacePiece(from, to);

      // ルークを動かす。
      ReplacePiece(H1, F1);

      has_castled_[WHITE] = true;
    } else if (move_type == CASTLE_WL) {  // 白のロングキャスリング。
      // キングを動かす。
      ReplacePiece(from, to);

      // ルークを動かす。
      ReplacePiece(A1, D1);

      has_castled_[WHITE] = true;
    } else if (move_type == CASTLE_BS) {  // 黒のショートキャスリング。
      // キングを動かす。
      ReplacePiece(from, to);

      // ルークを動かす。
      ReplacePiece(H8, F8);

      has_castled_[BLACK] = true;
    } else if (move_type == CASTLE_BL) {  // 黒のロングキャスリング。
      // キングを動かす。
      ReplacePiece(from, to);

      // ルークを動かす。
      ReplacePiece(A8, D8);

      has_castled_[BLACK] = true;
    } else {  // それ以外の場合。 (Null Moveなど。)
      return;
    }
  }

  // MakeMove()で動かした手を元に戻す。
  void ChessEngine::UnmakeMove(Move move) {
    // 相手のサイドを得る。
    Side enemy_side = to_move_;

    // 手番を反転させる。
    to_move_ ^=  0x3;

    // 動かす前のキャスリングの権利とアンパッサンを復元する。
    castling_rights_ = GET_CASTLING_RIGHTS(move);
    en_passant_square_ = GET_EN_PASSANT_SQUARE(move);

    // 手の情報を得る。
    Square from = GET_FROM(move);
    Square to = GET_TO(move);
    Piece promotion = GET_PROMOTION(move);
    MoveType move_type = GET_MOVE_TYPE(move);

    // 駒の位置を戻す。
    ReplacePiece(to, from);

    // 手の種類で分岐する。
    if (move_type == NORMAL) {  // 普通の手の場合。
      // 取った駒を戻す。
      PutPiece(to, GET_CAPTURED_PIECE(move), enemy_side);

      // 昇格ならポーンに戻す。
      if (promotion) {
        PutPiece(from, PAWN, to_move_);
      }
    } else if (move_type == CASTLE_WS) {  // 白のショートキャスリング。
      // ルークを戻す。
      ReplacePiece(F1, H1);

      has_castled_[WHITE] = false;
    } else if (move_type == CASTLE_WL) {  // 白のロングキャスリング。
      // ルークを戻す。
      ReplacePiece(D1, A1);

      has_castled_[WHITE] = false;
    } else if (move_type == CASTLE_BS) {  // 黒のショートキャスリング。
      // ルークを戻す。
      ReplacePiece(F8, H8);

      has_castled_[BLACK] = false;
    } else if (move_type == CASTLE_BL) {  // 黒のロングキャスリング。
      // ルークを戻す。
      ReplacePiece(D8, A8);

      has_castled_[BLACK] = false;
    } else if (move_type == EN_PASSANT) {  // アンパッサンの場合。
      // アンパッサンのターゲットを戻す。
      PutPiece(Util::TO_EN_PASSANT_TARGET[to_move_][en_passant_square_],
      PAWN, enemy_side);
    } else {  // それ以外の場合。 (Null Moveなど。)
      return;
    }
  }

  // その位置が他の位置の駒に攻撃されているかどうかチェックする。
  bool ChessEngine::IsAttacked(Square square, Side side) const {
    // ポーンに攻撃されているかどうか調べる。
    Bitboard attack = Util::GetPawnAttack(square, OPPOSITE_SIDE(side));
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

  // 現在のマテリアルを得る。
  int ChessEngine::GetMaterial(Side side) const {
    // 相手のサイド。
    Side enemy_side = OPPOSITE_SIDE(side);

    int material = 0;
    for (Piece piece_type = PAWN; piece_type <= QUEEN; piece_type++) {
      material += shared_st_ptr_->search_params_ptr_->material()[piece_type]
      * (Util::CountBits(position_[side][piece_type])
      - Util::CountBits(position_[enemy_side][piece_type]));
    }

    return material;
  }

  // 次の局面の「自分」のマテリアルを得る。
  int ChessEngine::GetNextMyMaterial(int current_material, Move move) const {
    if (GET_MOVE_TYPE(move) == EN_PASSANT) {
      // アンパッサン。
      return current_material
      + shared_st_ptr_->search_params_ptr_->material()[PAWN];
    } else if (Piece promotion = GET_PROMOTION(move)) {
      // プロモーション。
      return current_material + shared_st_ptr_->search_params_ptr_->
      material()[piece_board_[GET_TO(move)]]
      + shared_st_ptr_->search_params_ptr_->material()[promotion]
      - shared_st_ptr_->search_params_ptr_->material()[PAWN];
    } else {
      // その他の手。
      return current_material + shared_st_ptr_->search_params_ptr_->
      material()[piece_board_[GET_TO(move)]];
    }
  }

  // 現在の局面のハッシュを計算する。
  Hash ChessEngine::GetCurrentHash() const {
    Hash hash = 0;

    // 駒の情報からハッシュを得る。
    for (Square square = 0; square < NUM_SQUARES; square++) {
      hash ^= shared_st_ptr_->piece_hash_value_table_
      [side_board_[square]][piece_board_[square]][square];
    }

    // 手番からハッシュを得る。
    hash ^= shared_st_ptr_->to_move_hash_value_table_[to_move_];

    // キャスリングの権利からハッシュを得る。
    Castling bit = 1;
    for (int i = 0; i < 4; i++) {
      if (castling_rights_ & bit) {
        hash ^= shared_st_ptr_->castling_hash_value_table_[i];
      }
      bit <<= 1;
    }

    // アンパッサンからハッシュを得る。
    hash ^= shared_st_ptr_->en_passant_hash_value_table_[en_passant_square_];

    return hash;
  }

  // 次の局面のハッシュを得る。
  Hash ChessEngine::GetNextHash(Hash current_hash, Move move) const {
    // 駒の情報を得る。
    Square from = GET_FROM(move);
    Square to = GET_TO(move);
    Piece promotion = GET_PROMOTION(move);

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
      target_type = piece_board_[Util::TO_EN_PASSANT_TARGET[piece_side][to]];
    }

    // 移動する駒のハッシュを削除する。
    current_hash ^=
    shared_st_ptr_->piece_hash_value_table_[piece_side][piece_type][from];

    // 取る駒のハッシュを削除する。
    current_hash ^= shared_st_ptr_->piece_hash_value_table_
    [target_side][target_type][target_square];

    // 移動する駒の移動先のハッシュを追加する。
    if (promotion) {
      current_hash ^=
      shared_st_ptr_->piece_hash_value_table_[piece_side][promotion][to];
    } else {
      current_hash ^=
      shared_st_ptr_->piece_hash_value_table_[piece_side][piece_type][to];
    }

    // 現在の手番のハッシュを削除。
    current_hash ^= shared_st_ptr_->to_move_hash_value_table_[to_move_];

    // 次の手番のハッシュを追加。
    current_hash ^=
    shared_st_ptr_->to_move_hash_value_table_[OPPOSITE_SIDE(to_move_)];

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
        current_hash ^= shared_st_ptr_->castling_hash_value_table_[i];
      }
      bit <<= 1;
    }

    // とりあえずアンパッサンのハッシュを削除。
    current_hash ^=
    shared_st_ptr_->en_passant_hash_value_table_[en_passant_square_];

    // ポーンの2歩の動きの場合はアンパッサンハッシュを追加。
    // temp[][]は計算時間の省略のための、2歩の動きの判定用テーブル。
    static const Square temp[NUM_SIDES][NUM_SQUARES] {
      {
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U
      },
      {
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        A4, B4, C4, D4, E4, F4, G4, H4,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U
      },
      {
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U,
        A5, B5, C5, D5, E5, F5, G5, H5,
        -1U, -1U, -1U, -1U, -1U, -1U, -1U, -1U
      }
    };
    if (piece_type == PAWN) {
      if (temp[piece_side][from] == to) {
        current_hash ^= shared_st_ptr_->en_passant_hash_value_table_
        [Util::TO_EN_PASSANT_SQUARE[piece_side][to]];
      }
    }

    // 次の局面のハッシュを返す。
    return current_hash;
  }

  // ======================== //
  // その他のプライベート関数 //
  // ======================== //
  // 駒を置く。
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

  // 駒の位置を変える。
  void ChessEngine::ReplacePiece(Square from, Square to) {
    // 移動する位置と移動先の位置が同じなら何もしない。
    if (from == to) return;

    // 移動。
    PutPiece(to, piece_board_[from], side_board_[from]);
    PutPiece(from, EMPTY, NO_SIDE);
  }

  // ================ //
  // 共有メンバ構造体 //
  // ================ //
  // コンストラクタ。
  ChessEngine::SharedStruct::SharedStruct() :
  history_max_(1),
  i_depth_(1),
  searched_nodes_(0),
  searched_level_(0),
  stop_now_(false),
  max_nodes_(-1ULL),
  max_depth_(MAX_PLYS),
  thinking_time_(-1U >> 1),
  infinite_thinking_(false),
  move_history_(0),
  ply_100_history_(0),
  position_history_(0),
  eval_params_ptr_(nullptr) {
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square from = 0; from < NUM_SQUARES; from++) {
        for (Square to = 0; to < NUM_SQUARES; to++) {
          history_[side][from][to] = 0;
        }
      }
    }
    for (unsigned int i = 0; i < (MAX_PLYS + 1); i++) {
      iid_stack_[i] = 0;
      killer_stack_[i][0] = 0;
      killer_stack_[i][1] = 0;
      killer_stack_[i + 2][0] = 0;
      killer_stack_[i + 2][1] = 0;
    }
    helper_queue_ptr_.reset(new HelperQueue());
    InitHashValueTable();
  }

  // コピーコンストラクタ。
  ChessEngine::SharedStruct::SharedStruct(const SharedStruct& shared_st) {
    ScanMember(shared_st);
  }

  // ムーブコンストラクタ。
  ChessEngine::SharedStruct::SharedStruct(SharedStruct&& shared_st) {
    ScanMember(shared_st);
  }

  // コピー代入演算子。
  ChessEngine::SharedStruct& ChessEngine::SharedStruct::operator=
  (const SharedStruct& shared_st) {
    ScanMember(shared_st);
    return *this;
  }

  // ムーブ代入演算子。
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
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      iid_stack_[i] = shared_st.iid_stack_[i];
      killer_stack_[i][0] = shared_st.killer_stack_[i][0];
      killer_stack_[i][0] = shared_st.killer_stack_[i][0];
      killer_stack_[i + 2][1] = shared_st.killer_stack_[i + 2][1];
      killer_stack_[i + 2][1] = shared_st.killer_stack_[i + 2][1];
    }
    i_depth_ = shared_st.i_depth_;
    searched_nodes_ = shared_st.searched_nodes_;
    searched_level_ = shared_st.searched_level_;
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
    search_params_ptr_ = shared_st.search_params_ptr_;
    eval_params_ptr_ = shared_st.eval_params_ptr_;

    // ハッシュ関連。
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        for (Square square = 0; square < NUM_SQUARES; square++) {
          piece_hash_value_table_[side][piece_type][square] =
          shared_st.piece_hash_value_table_[side][piece_type][square];
        }
      }
    }
    for (Side side = 0; side < NUM_SIDES; side++) {
      to_move_hash_value_table_[side] =
      shared_st.to_move_hash_value_table_[side];
    }
    for (int i = 0; i < 4; i++) {
      castling_hash_value_table_[i] = shared_st.castling_hash_value_table_[i];
    }
    for (Square square = 0; square < NUM_SQUARES; square++) {
      en_passant_hash_value_table_[square] =
      shared_st.en_passant_hash_value_table_[square];
    }
  }

  // ハッシュ値のテーブルを初期化する。
  void ChessEngine::SharedStruct::InitHashValueTable() {
    // ダブリのないハッシュを生成。
    constexpr int LENGTH =
    (NUM_SIDES * NUM_PIECE_TYPES * NUM_SQUARES) + 1 + 4 + NUM_SQUARES;
    Hash temp_table[LENGTH];
    int temp_count = 0;
    for (int i = 0; i < LENGTH; i++) {
      // ダブリを調べる。
      bool loop = true;
      Hash hash = 0;
      while (loop) {
        hash = Util::GetRandomHash();
        loop = false;
        if (hash == 0) {
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
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES;
      piece_type++) {
        for (Square square = 0; square < NUM_SQUARES; square++) {
          if ((side == NO_SIDE) || (piece_type == EMPTY)) {
            piece_hash_value_table_[side][piece_type][square] = 0;
          } else {
            piece_hash_value_table_[side][piece_type][square] =
            temp_table[temp_count];
            temp_count++;
          }
        }
      }
    }

    // 手番の配列を初期化。
    to_move_hash_value_table_[NO_SIDE] = 0;
    to_move_hash_value_table_[WHITE] = 0;
    to_move_hash_value_table_[BLACK] = temp_table[temp_count];
    temp_count++;

    // キャスリングの配列を初期化。
    // 0: 白のショートキャスリング。
    // 1: 白のロングキャスリング。
    // 2: 黒のショートキャスリング。
    // 3: 黒のロングキャスリング。
    for (int i = 0; i < 4; i++) {
      castling_hash_value_table_[i] = temp_table[temp_count];
      temp_count++;
    }

    // アンパッサンの配列を初期化。
    en_passant_hash_value_table_[0] = 0;
    for (Square square = 1; square < NUM_SQUARES; square++) {
      en_passant_hash_value_table_[square] = temp_table[temp_count];
      temp_count++;
    }
  }
}  // namespace Sayuri
