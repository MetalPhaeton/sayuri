/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
#include <cstring>
#include <climits>
#include "common.h"
#include "board.h"
#include "transposition_table.h"
#include "fen.h"
#include "move_maker.h"
#include "pv_line.h"
#include "uci_shell.h"
#include "position_record.h"
#include "job.h"
#include "helper_queue.h"
#include "params.h"
#include "cache.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  ChessEngine::ChessEngine(const SearchParams& search_params,
  const EvalParams& eval_params, TranspositionTable& table) :
  is_null_searching_(false),
  table_ptr_(nullptr),
  evaluator_(*this),
  notice_cut_level_(MAX_PLYS + 1) {
    SetNewGame();

    // 探索関数用パラメータ。
    shared_st_ptr_->search_params_ptr_ = &search_params;
    // 評価関数用パラメータ。
    shared_st_ptr_->eval_params_ptr_ = &eval_params;
    // トランスポジションテーブル。
    shared_st_ptr_->table_ptr_ = &table;

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      job_table_[i].client_ptr_ = this;
      job_table_[i].level_ = i;
    }
  }

  // プライベートコンストラクタ。
  ChessEngine::ChessEngine() : 
  is_null_searching_(false),
  evaluator_(*this) {
    SetNewGame();

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      job_table_[i].client_ptr_ = this;
      job_table_[i].level_ = i;
    }
  }

  // コピーコンストラクタ。
  ChessEngine::ChessEngine(const ChessEngine& engine) :
  is_null_searching_(false),
  evaluator_(*this),
  notice_cut_level_(MAX_PLYS + 1) {
    // 基本メンバをコピー。
    basic_st_ = engine.basic_st_;

    // 共有メンバのコピー。
    shared_st_ptr_.reset(new SharedStruct(*(engine.shared_st_ptr_)));

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      job_table_[i].client_ptr_ = this;
      job_table_[i].level_ = i;
    }
  }

  // ムーブコンストラクタ。
  ChessEngine::ChessEngine(ChessEngine&& engine) :
  is_null_searching_(false),
  evaluator_(*this),
  notice_cut_level_(MAX_PLYS + 1)  {
    // 基本メンバをコピー。
    basic_st_ = engine.basic_st_;

    // 共有メンバのムーブ。
    shared_st_ptr_ = std::move(engine.shared_st_ptr_);

    // ムーブメーカー。
    maker_table_.reset(new MoveMaker[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      maker_table_[i] = MoveMaker(*this);
    }

    // PVLine。
    pv_line_table_.reset(new PVLine[MAX_PLYS + 1]);

    // Job。
    job_table_.reset(new Job[MAX_PLYS + 1]);
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); ++i) {
      job_table_[i].client_ptr_ = this;
      job_table_[i].level_ = i;
    }
  }

  // コピー代入演算子。
  ChessEngine& ChessEngine::operator=(const ChessEngine& engine) {
    // 基本メンバをコピー。
    basic_st_ = engine.basic_st_;

    // 共有メンバをコピー。
    shared_st_ptr_.reset(new SharedStruct(*(engine.shared_st_ptr_)));

    return *this;
  }

  // ムーブ代入演算子。
  ChessEngine& ChessEngine::operator=(ChessEngine&& engine) {
    // 基本メンバをコピー。
    basic_st_ = engine.basic_st_;


    // 共有メンバをムーブ。
    shared_st_ptr_ = std::move(engine.shared_st_ptr_);

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
    FOR_SQUARES(square) {
      basic_st_.piece_board_[square] = EMPTY;
      basic_st_.side_board_[square] = NO_SIDE;
    }
    INIT_ARRAY(basic_st_.blocker_);

    // 駒を配置する。
    for (Side side = WHITE; side <= BLACK; ++side) {
      basic_st_.side_pieces_[side] = 0;
      for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
        basic_st_.position_[side][piece_type] =
        fen.position()[side][piece_type];
        for (Bitboard bb = basic_st_.position_[side][piece_type]; bb;
        NEXT_BITBOARD(bb)) {
          Square square = Util::GetSquare(bb);
          basic_st_.side_board_[square] = side;
          basic_st_.piece_board_[square] = piece_type;
          basic_st_.side_pieces_[side] |= Util::SQUARE[square][R0];
          basic_st_.blocker_[R0] |= Util::SQUARE[square][R0];
          basic_st_.blocker_[R45] |= Util::SQUARE[square][R45];
          basic_st_.blocker_[R90] |= Util::SQUARE[square][R90];
          basic_st_.blocker_[R135] |= Util::SQUARE[square][R135];
          if (piece_type == KING) {
            basic_st_.king_[side] = square;
          }
        }
      }
    }

    // 残りを設定。
    basic_st_.to_move_ = fen.to_move();
    basic_st_.castling_rights_ = fen.castling_rights();
    basic_st_.en_passant_square_ = fen.en_passant_square();
    basic_st_.clock_ = fen.clock();
    basic_st_.ply_ = fen.ply();

    // 履歴を設定。
    shared_st_ptr_->clock_history_.clear();
    shared_st_ptr_->clock_history_.push_back(basic_st_.clock_);
    shared_st_ptr_->position_history_.clear();
    shared_st_ptr_->position_history_.push_back(PositionRecord(*this));

    // キャスリングの権利を更新。
    UpdateCastlingRights();
  }

  // PositionRecordから局面読み込む。
  void ChessEngine::LoadRecord(const PositionRecord& record) {
    Board& this_base = basic_st_;
    this_base = record;
    COPY_ARRAY(basic_st_.position_memo_, record.position_memo_);
  }

  // 駒を初期配置にセットする。
  void ChessEngine::SetStartPosition() {
    constexpr static const Bitboard
    STARTING_POSITION[NUM_SIDES][NUM_PIECE_TYPES] {
      {0, 0, 0, 0, 0, 0, 0},
      {
        0,
        Util::RANK[RANK_2],
        Util::SQUARE[B1][R0] | Util::SQUARE[G1][R0],
        Util::SQUARE[C1][R0] | Util::SQUARE[F1][R0],
        Util::SQUARE[A1][R0] | Util::SQUARE[H1][R0],
        Util::SQUARE[D1][R0],
        Util::SQUARE[E1][R0]
      },
      {
        0,
        Util::RANK[RANK_7],
        Util::SQUARE[B8][R0] | Util::SQUARE[G8][R0],
        Util::SQUARE[C8][R0] | Util::SQUARE[F8][R0],
        Util::SQUARE[A8][R0] | Util::SQUARE[H8][R0],
        Util::SQUARE[D8][R0],
        Util::SQUARE[E8][R0]
      }
    };
    COPY_ARRAY(basic_st_.position_, STARTING_POSITION);

    // 各サイドの駒の配置を作る。
    basic_st_.side_pieces_[NO_SIDE] = 0;
    basic_st_.side_pieces_[WHITE] = Util::RANK[RANK_1] | Util::RANK[RANK_2];
    basic_st_.side_pieces_[BLACK] = Util::RANK[RANK_8] | Util::RANK[RANK_7];

    // ブロッカーのビットボードを作る。
    basic_st_.blocker_[R0] = Util::RANK[RANK_1] | Util::RANK[RANK_2]
    | Util::RANK[RANK_8] | Util::RANK[RANK_7];
    basic_st_.blocker_[R45] = 0;
    basic_st_.blocker_[R90] = 0;
    basic_st_.blocker_[R135] = 0;
    for (Bitboard bb = basic_st_.blocker_[R0]; bb; NEXT_BITBOARD(bb)) {
      Square square = Util::GetSquare(bb);

      basic_st_.blocker_[R45] |= Util::SQUARE[square][R45];
      basic_st_.blocker_[R90] |= Util::SQUARE[square][R90];
      basic_st_.blocker_[R135] |= Util::SQUARE[square][R135];
    }

    // 駒の種類とサイドの配置を作る。
    constexpr static const PieceType STARTING_PIECE_BOARD[NUM_SQUARES] {
      ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
      PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
      EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
      PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
      ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK
    };
    constexpr static const Side STARTING_SIDE_BOARD[NUM_SQUARES] {
      WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
      WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
      NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE,
      NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE,
      NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE,
      NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE, NO_SIDE,
      BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
      BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK
    };
    COPY_ARRAY(basic_st_.piece_board_, STARTING_PIECE_BOARD);
    COPY_ARRAY(basic_st_.side_board_, STARTING_SIDE_BOARD);

    // キングを入れる。
    basic_st_.king_[NO_SIDE] = 0;
    basic_st_.king_[WHITE] = E1;
    basic_st_.king_[BLACK] = E8;

    // 手番を初期化。
    basic_st_.to_move_ = WHITE;

    // キャスリングの権利を初期化。
    basic_st_.castling_rights_ = ALL_CASTLING;

    // アンパッサンを初期化。
    basic_st_.en_passant_square_ = 0;

    // 50手ルールを初期化。
    basic_st_.clock_ = 0;

    // 手数を初期化。
    basic_st_.ply_ = 0;

    // キャスリングしたかどうかの初期化。
    FOR_SIDES(side) {
      basic_st_.has_castled_[side] = false;
    }

    // 駒の配置のメモを初期化。
    INIT_ARRAY(basic_st_.position_memo_);

    if (shared_st_ptr_) {
      // 50手ルールの履歴を初期化。
      shared_st_ptr_->clock_history_.clear();
      shared_st_ptr_->clock_history_.push_back(0);

      // 駒の配置の履歴を初期化。
      shared_st_ptr_->position_history_.clear();
      shared_st_ptr_->position_history_.push_back(PositionRecord(*this));
    }
  }

  // 新しいゲームの準備をする。
  void ChessEngine::SetNewGame() {
    // 駒の配置を初期化。
    SetStartPosition();

    // 共有メンバ構造体を初期化。
    const SearchParams* temp_sp_ptr = nullptr;
    const EvalParams* temp_ep_ptr = nullptr;
    TranspositionTable* temp_table_ptr = nullptr;
    if (shared_st_ptr_) {
      temp_sp_ptr = shared_st_ptr_->search_params_ptr_;  // 一時待避。
      temp_ep_ptr = shared_st_ptr_->eval_params_ptr_;  // 一時待避。
      temp_table_ptr = shared_st_ptr_->table_ptr_;  // 一時待避。
    }
    shared_st_ptr_.reset(new SharedStruct());
    shared_st_ptr_->search_params_ptr_ = temp_sp_ptr;  // 復帰。
    shared_st_ptr_->eval_params_ptr_ = temp_ep_ptr;  // 復帰。
    shared_st_ptr_->table_ptr_ = temp_table_ptr; // 復帰。

    // 50手ルールの履歴を初期化。
    shared_st_ptr_->clock_history_.push_back(0);

    // 駒の配置の履歴を初期化。
    shared_st_ptr_->position_history_.push_back(PositionRecord(*this));
  }

  // 探索を開始する。
  PVLine ChessEngine::Calculate(int num_threads,
  const std::vector<Move>& moves_to_search, UCIShell& shell) {
    Util::UpdateMax(num_threads, 1);
    Util::UpdateMin(num_threads, UCI_MAX_THREADS);
    thread_vec_.resize(num_threads);
    return SearchRoot(moves_to_search, shell);
  }

  // 探索を終了させる。
  void ChessEngine::StopCalculation() {
    shared_st_ptr_->stop_now_ = true;
  }

  // 合法手かどうか判定。
  bool ChessEngine::IsLegalMove(Move& move) const {
    // 合法手かどうか調べる。
    // 手を展開する。
    shared_st_ptr_->history_max_ = 1;  // makerが0の除算をしないように。
    MoveMaker maker(*this);
    maker.GenMoves<GenMoveType::ALL>(0, 0, 0, 0);

    // 合法手かどうか調べる。
    ChessEngine* self = const_cast<ChessEngine*>(this);
    bool is_legal = false;
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);
    for (Move temp_move = maker.PickMove(); temp_move;
    temp_move = maker.PickMove()) {
      self->MakeMove(temp_move);
      // temp_moveが合法手かどうか調べる。
      if (IsAttacked(basic_st_.king_[side], enemy_side)) {
        self->UnmakeMove(temp_move);
        continue;
      }

      // temp_moveと同じ手かどうか調べる。
      if (EqualMove(move, temp_move)) {
        self->UnmakeMove(temp_move);
        move = temp_move;
        is_legal = true;
        break;
      }
      self->UnmakeMove(temp_move);
    }

    return is_legal;
  }

  // 合法手のベクトルを得る。
  std::vector<Move> ChessEngine::GetLegalMoves() const {
    // 手を展開する。
    shared_st_ptr_->history_max_ = 1;  // makerが0の除算をしないように。
    MoveMaker maker(*this);
    maker.GenMoves<GenMoveType::ALL>(0, 0, 0, 0);

    // 合法手かどうか調べながらベクトルに格納していく。
    ChessEngine* self = const_cast<ChessEngine*>(this);
    std::vector<Move> ret_vec;
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);
    for (Move move = maker.PickMove(); move; move = maker.PickMove()) {
      self->MakeMove(move);

      // 動かした結果、チェックされているかどうか。
      if (IsAttacked(basic_st_.king_[side], enemy_side)) {
        // チェックされていれば違法。
        self->UnmakeMove(move);
        continue;
      }

      // 合法手。
      ret_vec.push_back(move);
      self->UnmakeMove(move);
    }

    return ret_vec;
  }

  // Algebraic Notetionの指し手を予測する。
  std::vector<Move> ChessEngine::GuessNote(const std::string& note) const {
    // パース。
    std::string note_2 = Util::ParseAlgebraicNotation(note);

    // まず、候補手を得る。
    std::vector<Move> legal_move_vec = GetLegalMoves();

    // キャスリング判定。
    if (note_2 == "O-O") {
      for (auto move : legal_move_vec) {
        MoveType move_type = Get<MOVE_TYPE>(move);
        if ((move_type == CASTLE_WS) || (move_type == CASTLE_BS)) {
          return std::vector<Move> {move};
        }
      }
      return std::vector<Move>();
    } else if (note_2 == "O-O-O") {
      for (auto move : legal_move_vec) {
        MoveType move_type = Get<MOVE_TYPE>(move);
        if ((move_type == CASTLE_WL) || (move_type == CASTLE_BL)) {
          return std::vector<Move> {move};
        }
      }
      return std::vector<Move>();
    }

    // 駒の種類で弾く。
    if (note_2[0] != '-') {
      std::vector<Move>::iterator itr = legal_move_vec.begin();
      PieceType piece_type = EMPTY;
      switch (note_2[0]) {
        case 'P': piece_type = PAWN; break;
        case 'N': piece_type = KNIGHT; break;
        case 'B': piece_type = BISHOP; break;
        case 'R': piece_type = ROOK; break;
        case 'Q': piece_type = QUEEN; break;
        case 'K': piece_type = KING; break;
      }
      while (itr != legal_move_vec.end()) {
        if (basic_st_.piece_board_[Get<FROM>(*itr)] != piece_type) {
          legal_move_vec.erase(itr);
        } else {
          ++itr;
        }
      }
    } else {
      // 駒の種類は必須。
      return std::vector<Move>();
    }

    // 基点ファイルで弾く。
    if (note_2[1] != '-') {
      std::vector<Move>::iterator itr = legal_move_vec.begin();
      Fyle fyle = note_2[1] - 'a';
      while (itr != legal_move_vec.end()) {
        if (Util::SquareToFyle(Get<FROM>(*itr)) != fyle) {
          legal_move_vec.erase(itr);
        } else {
          ++itr;
        }
      }
    }

    // 基点ランクで弾く。
    if (note_2[2] != '-') {
      std::vector<Move>::iterator itr = legal_move_vec.begin();
      Rank rank = note_2[2] - '1';
      while (itr != legal_move_vec.end()) {
        if (Util::SquareToRank(Get<FROM>(*itr)) != rank) {
          legal_move_vec.erase(itr);
        } else {
          ++itr;
        }
      }
    }

    // 終点ファイルで弾く。
    if (note_2[3] != '-') {
      std::vector<Move>::iterator itr = legal_move_vec.begin();
      Fyle fyle = note_2[3] - 'a';
      while (itr != legal_move_vec.end()) {
        if (Util::SquareToFyle(Get<TO>(*itr)) != fyle) {
          legal_move_vec.erase(itr);
        } else {
          ++itr;
        }
      }
    } else {
      // 終点ファイルは必須。
      return std::vector<Move>();
    }


    // 終点ランクで弾く。
    if (note_2[4] != '-') {
      std::vector<Move>::iterator itr = legal_move_vec.begin();
      Rank rank = note_2[4] - '1';
      while (itr != legal_move_vec.end()) {
        if (Util::SquareToRank(Get<TO>(*itr)) != rank) {
          legal_move_vec.erase(itr);
        } else {
          ++itr;
        }
      }
    } else {
      // 終点ランクは必須。
      return std::vector<Move>();
    }

    // 昇格で弾く。
    if (note_2[5] != '-') {
      std::vector<Move>::iterator itr = legal_move_vec.begin();
      PieceType piece_type = EMPTY;
      switch (note_2[5]) {
        case 'P': piece_type = PAWN; break;
        case 'N': piece_type = KNIGHT; break;
        case 'B': piece_type = BISHOP; break;
        case 'R': piece_type = ROOK; break;
        case 'Q': piece_type = QUEEN; break;
        case 'K': piece_type = KING; break;
      }
      while (itr != legal_move_vec.end()) {
        if (Get<PROMOTION>(*itr) != piece_type) {
          legal_move_vec.erase(itr);
        } else {
          ++itr;
        }
      }
    }

    return legal_move_vec;
  }

  // Algebraic Notationを作成する。
  std::string ChessEngine::MoveToNote(Move move) const {
    // 候補手を得る。
    std::vector<Move> legal_move_vec = GetLegalMoves();

    // 先ず、moveを検査。 OKなら合法手からコピー。
    bool is_ok = false;
    for (auto move_2 : legal_move_vec) {
      if (EqualMove(move, move_2)) {
        move = move_2;
        is_ok = true;
        break;
      }
    }
    if (!is_ok) return std::string();

    // 情報収集。
    Square from = Get<FROM>(move);
    Square to = Get<TO>(move);
    PieceType promotion = Get<PROMOTION>(move);
    PieceType piece_type = basic_st_.piece_board_[from];
    MoveType move_type = Get<MOVE_TYPE>(move);

    std::ostringstream oss;

    // チェック・チェックメイトを調べる。
    bool is_check = false;
    bool is_checkmate = false;
    Side side = basic_st_.to_move_;
    Side enemy_side = Util::GetOppositeSide(side);
    ChessEngine* self = const_cast<ChessEngine*>(this);
    self->MakeMove(move);
    if (self->IsAttacked(basic_st_.king_[enemy_side], side)) {
      is_check = true;
      std::vector<Move> temp = self->GetLegalMoves();
      if (temp.size() <= 0) {
        is_checkmate = true;
      }
    }
    self->UnmakeMove(move);


    // キャスリング判定。
    if ((move_type == CASTLE_WS) || (move_type == CASTLE_BS)) {
      // ショートキャスリング。
      oss << "O-O";
      if (is_checkmate) oss << "#";
      else if (is_check) oss << "+";
      return oss.str();
    } else if ((move_type == CASTLE_WL) || (move_type == CASTLE_BL)) {
      // ロングキャスリング。
      oss << "O-O-O";
      if (is_checkmate) oss << "#";
      else if (is_check) oss << "+";
      return oss.str();
    }

    // 駒の種類を書き込む。
    switch (piece_type) {
      case KNIGHT: oss << "N"; break;
      case BISHOP: oss << "B"; break;
      case ROOK: oss << "R"; break;
      case QUEEN: oss << "Q"; break;
      case KING: oss << "K"; break;
    }

    // 同じ駒の同じ目的地のものを得る。 自分自身は除く。
    std::vector<Move> same_to;
    for (auto move_2 : legal_move_vec) {
      if (EqualMove(move, move_2)) break;

      if (Get<TO>(move_2) == to) {
        if (basic_st_.piece_board_[Get<FROM>(move_2)] == piece_type) {
          same_to.push_back(move_2);
        }
      }
    }

    // 基点を書き込む。
    if (same_to.size() >= 1) {
      // 同じファイルかランクかを判定。
      bool same_fyle = false;
      bool same_rank = false;
      for (auto move_2 : same_to) {
        if (Util::SquareToFyle(Get<FROM>(move_2))
        == Util::SquareToFyle(from)) {
          same_fyle = true;
        }
        if (Util::SquareToRank(Get<FROM>(move_2))
        == Util::SquareToRank(from)) {
          same_rank = true;
        }
      }

      // 書き込む。 同じファイルがあるときはランクを書く。
      if (same_fyle) {
        if (same_rank) {
          // 同じランクがあるので、ファイルを書く。
          oss << static_cast<char>('a' + Util::SquareToFyle(from));
        }
        oss << static_cast<char>('1' + Util::SquareToRank(from));
      } else {
        oss << static_cast<char>('a' + Util::SquareToFyle(from));
      }
    }

    // 駒を取る手の印を書き込む。 ポーンの場合は基点も書く。
    if (Get<CAPTURED_PIECE>(move)) {
      if (piece_type == PAWN) {
        oss << static_cast<char>('a' + Util::SquareToFyle(from));
      }
      oss << "x";
    }

    // 終点を書き込む。
    oss << static_cast<char>('a' + Util::SquareToFyle(to));
    oss << static_cast<char>('1' + Util::SquareToRank(to));

    // 昇格を書き込む。
    switch (promotion) {
      case KNIGHT: oss << "=N"; break;
      case BISHOP: oss << "=B"; break;
      case ROOK: oss << "=R"; break;
      case QUEEN: oss << "=Q"; break;
    }

    // チェック・チェックメイトを書き込む。
    if (is_checkmate) oss << "#";
    else if (is_check) oss << "+";

    return oss.str();
  }

  // 駒を配置する。
  void ChessEngine::PlacePiece(Square square, PieceType piece_type,
  Side piece_side) {
    // 各引数をチェック。
    if (square >= NUM_SQUARES) return;
    if (piece_type >= NUM_PIECE_TYPES) return;
    if (piece_side >= NUM_SIDES) return;
    if (piece_type && !piece_side) return;
    if (!piece_type && piece_side) return;

    // 元の位置にあったもの。
    PieceType origin_type = basic_st_.piece_board_[square];

    // 元の位置にあるものがキングだった時は実行しない。
    if (origin_type == KING) return;

    // 置きたい駒がキングだった時は、先ず元のキングを削除する。
    if (piece_type == KING) {
      PutPiece(basic_st_.king_[piece_side], EMPTY);
    }

    // 駒を置く。
    PutPiece(square, piece_type, piece_side);

    // キャスリングの権利を更新。
    UpdateCastlingRights();
  }

  // 手を指す。
  bool ChessEngine::PlayMove(Move move) {
    if (IsLegalMove(move)) {
      ++(basic_st_.ply_);
      if ((basic_st_.piece_board_[Get<FROM>(move)] == PAWN)
      || (basic_st_.piece_board_[Get<TO>(move)] != EMPTY)) {
        basic_st_.clock_ = 0;
      } else {
        ++(basic_st_.clock_);
      }
      shared_st_ptr_->move_history_.push_back(move);
      shared_st_ptr_->clock_history_.push_back(basic_st_.clock_);
      MakeMove(move);
      shared_st_ptr_->position_history_.push_back(PositionRecord(*this));

      return true;
    }
    return false;
  }

  // 1手戻す。
  Move ChessEngine::UndoMove() {
    // 戻せない。
    if (shared_st_ptr_->move_history_.size() <= 0) return 0;

    --(basic_st_.ply_);
    Move move = shared_st_ptr_->move_history_.back();
    basic_st_.clock_ = shared_st_ptr_->clock_history_.back();
    shared_st_ptr_->move_history_.pop_back();
    shared_st_ptr_->clock_history_.pop_back();
    shared_st_ptr_->position_history_.pop_back();
    UnmakeMove(move);

    return move;
  }

  // FEN文字列を返す。
  std::string ChessEngine::GetFENString() const {
    std::ostringstream oss;

    // 駒の配置。
    oss << Util::ToFENPosition(basic_st_.position_) << " ";

    // 手番。
    oss << (basic_st_.to_move_ == WHITE ? "w " : "b ");

    // キャスリングの権利。
    if (basic_st_.castling_rights_) {
      if ((basic_st_.castling_rights_ & WHITE_SHORT_CASTLING)) {
        oss << "K";
      }
      if ((basic_st_.castling_rights_ & WHITE_LONG_CASTLING)) {
        oss << "Q";
      }
      if ((basic_st_.castling_rights_ & BLACK_SHORT_CASTLING)) {
        oss << "k";
      }
      if ((basic_st_.castling_rights_ & BLACK_LONG_CASTLING)) {
        oss << "q";
      }
      oss << " ";
    } else {
      oss << "- ";
    }

    // アンパッサン。
    if (basic_st_.en_passant_square_) {
      oss << static_cast<char>('a'
      + Util::SquareToFyle(basic_st_.en_passant_square_))
      << static_cast<char>('1'
      + Util::SquareToRank(basic_st_.en_passant_square_)) << " ";
    } else {
      oss << "- ";
    }

    // クロック。
    oss << std::to_string(basic_st_.clock_) << " ";

    // 手数。
    oss << std::to_string((basic_st_.ply_ / 2) + 1);

    return oss.str();
  }

  // 駒を置く。
  void ChessEngine::PutPiece(Square square, PieceType piece_type, Side side) {
    // 置く位置の現在の駒の種類を入手する。
    PieceType placed_piece = basic_st_.piece_board_[square];

    // 置く位置の現在の駒のサイドを得る。
    Side placed_side = basic_st_.side_board_[square];

    // 置く位置のメンバを消す。
    if (placed_piece) {
      basic_st_.position_[placed_side][placed_piece] &=
      ~Util::SQUARE[square][R0];
      basic_st_.side_pieces_[placed_side] &= ~Util::SQUARE[square][R0];
    }

    // 置く駒がEMPTYか置くサイドがNO_SIDEなら
    // その位置のメンバを消して返る。
    if (!piece_type) {
      basic_st_.piece_board_[square] = EMPTY;
      basic_st_.side_board_[square] = NO_SIDE;
      if (placed_piece) {
        basic_st_.blocker_[R0] &= ~Util::SQUARE[square][R0];
        basic_st_.blocker_[R45] &= ~Util::SQUARE[square][R45];
        basic_st_.blocker_[R90] &= ~Util::SQUARE[square][R90];
        basic_st_.blocker_[R135] &= ~Util::SQUARE[square][R135];
      }
      return;
    }

    // 置く位置の駒の種類を書き変える。
    basic_st_.piece_board_[square] = piece_type;
    // 置く位置のサイドを書き変える。
    basic_st_.side_board_[square] = side;

    // 置く位置のビットボードをセットする。
    basic_st_.position_[side][piece_type] |= Util::SQUARE[square][R0];
    basic_st_.side_pieces_[side] |= Util::SQUARE[square][R0];
    basic_st_.blocker_[R0] |= Util::SQUARE[square][R0];
    basic_st_.blocker_[R45] |= Util::SQUARE[square][R45];
    basic_st_.blocker_[R90] |= Util::SQUARE[square][R90];
    basic_st_.blocker_[R135] |= Util::SQUARE[square][R135];

    // キングの位置を更新する。
    if (piece_type == KING) {
      basic_st_.king_[side] = square;
    }
  }

  // 次の手を指す。
  void ChessEngine::MakeMove(Move& move) {
    // 動かす側のサイドを得る。
    Side side = basic_st_.to_move_;

    // 手番を反転させる。
    basic_st_.to_move_ = Util::GetOppositeSide(basic_st_.to_move_);

    // 動かす前のキャスリングの権利とアンパッサンを記録する。
    Set<CASTLING_RIGHTS>(move, basic_st_.castling_rights_);
    Set<EN_PASSANT_SQUARE>(move, basic_st_.en_passant_square_);

    // アンパッサンを解除。
    basic_st_.en_passant_square_ = 0;

    // 手の要素を得る。
    Square from = Get<FROM>(move);
    Square to = Get<TO>(move);

    // キャスリングの権利を更新。
    if (basic_st_.castling_rights_) {
      switch (basic_st_.piece_board_[from]) {
        case KING:
          basic_st_.castling_rights_ &=
          ~(side == WHITE ? WHITE_CASTLING : BLACK_CASTLING);
          break;
        case ROOK:
          if (side == WHITE) {
            if (from == H1) {
              basic_st_.castling_rights_ &= ~(WHITE_SHORT_CASTLING);
            } else if (from == A1) {
              basic_st_.castling_rights_ &= ~(WHITE_LONG_CASTLING);
            }
          } else {
            if (from == H8) {
              basic_st_.castling_rights_ &= ~(BLACK_SHORT_CASTLING);
            } else if (from == A8) {
              basic_st_.castling_rights_ &= ~(BLACK_LONG_CASTLING);
            }
          }
          break;
      }
    }

    // 手の種類によって分岐する。
    switch (Get<MOVE_TYPE>(move)) {
      case NORMAL:  // 通常の手。
        // 取る駒を登録する。
        Set<CAPTURED_PIECE>(move, basic_st_.piece_board_[to]);
        // 駒を動かす。
        ReplacePiece(from, to);
        // 駒を昇格させるなら、駒を昇格させる。
        if (move & MASK[PROMOTION]) {
          PutPiece(to, Get<PROMOTION>(move), side);
        }

        // ポーンの2歩の動きの場合はアンパッサンできるようにする。
        if ((basic_st_.piece_board_[to] == PAWN)
        && (Util::Is2StepMove(from, to))) {
          basic_st_.en_passant_square_ = Util::EN_PASSANT_TRANS_TABLE[to];
        }
        break;
      case EN_PASSANT:  // アンパッサン。
        // 取った駒をボーンにする。
        Set<CAPTURED_PIECE>(move, PAWN);
        // 動かす。
        ReplacePiece(from, to);
        // アンパッサンのターゲットを消す。
        PutPiece(Util::EN_PASSANT_TRANS_TABLE[to], EMPTY);
        break;
      case CASTLE_WS:  // 白ショートキャスリング。
        // キングを動かす。
        ReplacePiece(from, to);
        // ルークを動かす。
        ReplacePiece(H1, F1);
        basic_st_.has_castled_[WHITE] = true;
        break;
      case CASTLE_WL:  // 白ロングキャスリング。
        // キングを動かす。
        ReplacePiece(from, to);
        // ルークを動かす。
        ReplacePiece(A1, D1);
        basic_st_.has_castled_[WHITE] = true;
        break;
      case CASTLE_BS:  // 黒ショートキャスリング。
        // キングを動かす。
        ReplacePiece(from, to);
        // ルークを動かす。
        ReplacePiece(H8, F8);
        basic_st_.has_castled_[BLACK] = true;
        break;
      case CASTLE_BL:  // 黒ロングキャスリング。
        // キングを動かす。
        ReplacePiece(from, to);
        // ルークを動かす。
        ReplacePiece(A8, D8);
        basic_st_.has_castled_[BLACK] = true;
        break;
    }
  }

  // MakeMove()で動かした手を元に戻す。
  void ChessEngine::UnmakeMove(Move move) {
    // 相手のサイドを得る。
    Side enemy_side = basic_st_.to_move_;

    // 手番を反転させる。
    basic_st_.to_move_ = Util::GetOppositeSide(basic_st_.to_move_);

    // 動かす前のキャスリングの権利とアンパッサンを復元する。
    basic_st_.castling_rights_ = Get<CASTLING_RIGHTS>(move);
    basic_st_.en_passant_square_ = Get<EN_PASSANT_SQUARE>(move);

    // 手の情報を得る。
    Square from = Get<FROM>(move);
    Square to = Get<TO>(move);

    // 駒の位置を戻す。
    ReplacePiece(to, from);

    // 手の種類で分岐する。
    switch (Get<MOVE_TYPE>(move)) {
      case NORMAL:
        // 取った駒を戻す。
        PutPiece(to, Get<CAPTURED_PIECE>(move), enemy_side);
        // 昇格ならポーンに戻す。
        if (Get<PROMOTION>(move)) {
          PutPiece(from, PAWN, basic_st_.to_move_);
        }
        break;
      case EN_PASSANT:
        // アンパッサンのターゲットを戻す。
        PutPiece(Util::EN_PASSANT_TRANS_TABLE[basic_st_.en_passant_square_],
        PAWN, enemy_side);
        break;
      case CASTLE_WS:
        // ルークを戻す。
        ReplacePiece(F1, H1);
        basic_st_.has_castled_[WHITE] = false;
        break;
      case CASTLE_WL:
        // ルークを戻す。
        ReplacePiece(D1, A1);
        basic_st_.has_castled_[WHITE] = false;
        break;
      case CASTLE_BS:
        // ルークを戻す。
        ReplacePiece(F8, H8);
        basic_st_.has_castled_[BLACK] = false;
        break;
      case CASTLE_BL:
        // ルークを戻す。
        ReplacePiece(D8, A8);
        basic_st_.has_castled_[BLACK] = false;
        break;
    }
  }

  // その位置が他の位置の駒に攻撃されているかどうかチェックする。
  bool ChessEngine::IsAttacked(Square square, Side side) const {
    // ポーンに攻撃されているかどうか調べる。
    Bitboard attack = Util::PAWN_ATTACK[Util::GetOppositeSide(side)][square];
    if (attack & basic_st_.position_[side][PAWN]) return true;

    // ナイトに攻撃されているかどうか調べる。
    attack = Util::KNIGHT_MOVE[square];
    if (attack & basic_st_.position_[side][KNIGHT]) return true;

    // ビショップとクイーンの斜めに攻撃されているかどうか調べる。
    attack = GetBishopAttack(square);
    if (attack & (basic_st_.position_[side][BISHOP]
    | basic_st_.position_[side][QUEEN])) return true;

    // ルークとクイーンの縦横に攻撃されているかどうか調べる。
    attack = GetRookAttack(square);
    if (attack & (basic_st_.position_[side][ROOK]
    | basic_st_.position_[side][QUEEN])) return true;

    // キングに攻撃されているかどうか調べる。
    attack = Util::KING_MOVE[square];
    if (attack & basic_st_.position_[side][KING]) return true;

    // 何にも攻撃されていない。
    return false;
  }

  // 現在のマテリアルを得る。
  int ChessEngine::GetMaterial(Side side) const {
    // 相手のサイド。
    Side enemy_side = Util::GetOppositeSide(side);

    int material = 0;
    for (PieceType piece_type = PAWN; piece_type <= QUEEN; ++piece_type) {
      material += shared_st_ptr_->search_params_ptr_->material_[piece_type]
      * (Util::CountBits(basic_st_.position_[side][piece_type])
      - Util::CountBits(basic_st_.position_[enemy_side][piece_type]));
    }

    return material;
  }

  // 現在の局面のハッシュを計算する。
  Hash ChessEngine::GetCurrentHash() const {
    Hash hash = 0;

    // 駒の情報からハッシュを得る。
    FOR_SQUARES(square) {
      hash ^= shared_st_ptr_->piece_hash_value_table_
      [basic_st_.side_board_[square]][basic_st_.piece_board_[square]][square];
    }

    // 手番からハッシュを得る。
    hash ^= shared_st_ptr_->to_move_hash_value_table_[basic_st_.to_move_];

    // キャスリングの権利からハッシュを得る。
    hash ^= shared_st_ptr_->castling_hash_value_table_
    [basic_st_.castling_rights_];

    // アンパッサンからハッシュを得る。
    hash ^= shared_st_ptr_->en_passant_hash_value_table_
    [basic_st_.en_passant_square_];

    return hash;
  }

  // 次の局面のハッシュを得る。
  Hash ChessEngine::GetNextHash(Hash current_hash, Move move) const {
    // キャッシュ。
    Cache& cache = shared_st_ptr_->cache_;

    // 駒の情報を得る。
    Square from = Get<FROM>(move);
    Square to = Get<TO>(move);

    // 駒の位置の種類とサイドを得る。
    PieceType piece_type = basic_st_.piece_board_[from];
    Side piece_side = basic_st_.side_board_[from];

    // 取る駒の種類とサイドを得る。
    PieceType target_type = basic_st_.piece_board_[to];
    Side target_side = basic_st_.side_board_[to];
    Square target_square = to;
    switch (Get<MOVE_TYPE>(move)) {
      case EN_PASSANT:  // アンパッサン。
        {
          target_type = PAWN;
          target_square = Util::EN_PASSANT_TRANS_TABLE[to];
        }
        break;
      case CASTLE_WS:  // 白ショートキャスリング。
        {
          current_hash ^= cache.piece_hash_value_table_[WHITE][ROOK][H1];
          current_hash ^= cache.piece_hash_value_table_[WHITE][ROOK][F1];
        }
        break;
      case CASTLE_WL:  // 白ロングキャスリング。
        {
          current_hash ^= cache.piece_hash_value_table_[WHITE][ROOK][A1];
          current_hash ^= cache.piece_hash_value_table_[WHITE][ROOK][D1];
        }
        break;
      case CASTLE_BS:  // 黒ショートキャスリング。
        {
          current_hash ^= cache.piece_hash_value_table_[BLACK][ROOK][H8];
          current_hash ^= cache.piece_hash_value_table_[BLACK][ROOK][F8];
        }
        break;
      case CASTLE_BL:  // 黒ロングキャスリング。
        {
          current_hash ^= cache.piece_hash_value_table_[BLACK][ROOK][A8];
          current_hash ^= cache.piece_hash_value_table_[BLACK][ROOK][D8];
        }
        break;
    }

    // 移動する駒のハッシュを削除する。
    current_hash ^=
    cache.piece_hash_value_table_[piece_side][piece_type][from];

    // 取る駒のハッシュを削除する。
    current_hash ^=
    cache.piece_hash_value_table_[target_side][target_type][target_square];

    // 移動する駒の移動先のハッシュを追加する。
    if (move & MASK[PROMOTION]) {
      current_hash ^=
      cache.piece_hash_value_table_[piece_side][Get<PROMOTION>(move)][to];
    } else {
      current_hash ^=
      cache.piece_hash_value_table_[piece_side][piece_type][to];
    }

    // 現在の手番のハッシュを削除。
    current_hash ^= cache.to_move_hash_value_table_[basic_st_.to_move_];

    // 次の手番のハッシュを追加。
    current_hash ^=
    cache.to_move_hash_value_table_[Util::GetOppositeSide(basic_st_.to_move_)];

    // キャスリングの権利のハッシュをセット。
    if (basic_st_.castling_rights_) {
      Hash next_rights = basic_st_.castling_rights_;
      switch (piece_type) {
        case KING:
          next_rights &=
          ~(piece_side == WHITE ? WHITE_CASTLING : BLACK_CASTLING);
        case ROOK:
          if (piece_side == WHITE) {
            if (from == H1) next_rights &= ~WHITE_SHORT_CASTLING;
            else if (from == A1) next_rights &= ~WHITE_LONG_CASTLING;
          } else {
            if (from == H8) next_rights &= ~BLACK_SHORT_CASTLING;
            else if (from == A8) next_rights &= ~BLACK_LONG_CASTLING;
          }
      }

      // 現在のキャスリングのハッシュを消す。
      current_hash ^= cache.castling_hash_value_table_
      [basic_st_.castling_rights_];

      // 次のキャスリングのハッシュをセット。
      current_hash ^= cache.castling_hash_value_table_[next_rights];
    }

    // とりあえずアンパッサンのハッシュを削除。
    current_hash ^= cache.en_passant_hash_value_table_
    [basic_st_.en_passant_square_];

    // ポーンの2歩の動きの場合はアンパッサンハッシュを追加。
    if ((piece_type == PAWN) && (Util::Is2StepMove(from, to))) {
      current_hash ^= cache.en_passant_hash_value_table_
      [Util::EN_PASSANT_TRANS_TABLE[to]];
    }

    // 次の局面のハッシュを返す。
    return current_hash;
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
  max_nodes_(ULLONG_MAX),
  max_depth_(MAX_PLYS),
  end_time_(Chrono::milliseconds(INT_MAX)),
  is_time_over_(false),
  infinite_thinking_(false),
  move_history_(0),
  clock_history_(0),
  position_history_(0),
  search_params_ptr_(nullptr),
  eval_params_ptr_(nullptr),
  table_ptr_(nullptr) {
    // volatileは 'void*' にできない。
    FOR_SIDES(side) {
      FOR_SQUARES(from) {
        FOR_SQUARES(to) {
          history_[side][from][to] = 0;
        }
      }
    }
    for (unsigned int i = 0; i < (MAX_PLYS + 1); ++i) {
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
    // volatileは 'void*' にできない。
    FOR_SIDES(side) {
      FOR_SQUARES(from) {
        FOR_SQUARES(to) {
          history_[side][from][to] = shared_st.history_[side][from][to];
        }
      }
    }
    for (unsigned int i = 0; i < (MAX_PLYS + 1); ++i) {
      iid_stack_[i] = shared_st.iid_stack_[i];
      killer_stack_[i][0] = shared_st.killer_stack_[i][0];
      killer_stack_[i][1] = shared_st.killer_stack_[i][1];
      killer_stack_[i + 2][0] = shared_st.killer_stack_[i + 2][0];
      killer_stack_[i + 2][1] = shared_st.killer_stack_[i + 2][1];
    }

    i_depth_ = shared_st.i_depth_;
    searched_nodes_ = shared_st.searched_nodes_;
    searched_level_ = shared_st.searched_level_;
    start_time_ = shared_st.start_time_;
    stop_now_ = shared_st.stop_now_;
    max_nodes_ = shared_st.max_nodes_;
    max_depth_ = shared_st.max_depth_;
    end_time_ = shared_st.end_time_;
    is_time_over_ = shared_st.is_time_over_;
    infinite_thinking_ = shared_st.infinite_thinking_;
    move_history_ = shared_st.move_history_;
    clock_history_ = shared_st.clock_history_;
    position_history_ = shared_st.position_history_;
    helper_queue_ptr_.reset(new HelperQueue(*(shared_st.helper_queue_ptr_)));
    search_params_ptr_ = shared_st.search_params_ptr_;
    eval_params_ptr_ = shared_st.eval_params_ptr_;
    table_ptr_ = shared_st.table_ptr_;

    // ハッシュ関連。
    COPY_ARRAY(piece_hash_value_table_, shared_st.piece_hash_value_table_);

    COPY_ARRAY(to_move_hash_value_table_, shared_st.to_move_hash_value_table_);

    COPY_ARRAY(castling_hash_value_table_,
    shared_st.castling_hash_value_table_);

    COPY_ARRAY(en_passant_hash_value_table_,
    shared_st.en_passant_hash_value_table_);

    // キャッシュ。
    cache_ = shared_st.cache_;
  }

  // ハッシュ値のテーブルを初期化する。
  void ChessEngine::SharedStruct::InitHashValueTable() {
    // ダブリのないハッシュを生成。
    constexpr int LENGTH =
    (NUM_SIDES * NUM_PIECE_TYPES * NUM_SQUARES) + 1 + 16 + NUM_SQUARES;
    Hash temp_table[LENGTH];
    int temp_count = 0;
    for (int i = 0; i < LENGTH; ++i) {
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
        for (int j = 0; j < i; ++j) {
          if (hash == temp_table[j]) {
            loop = true;
            break;
          }
        }
      }

      temp_table[i] = hash;
    }

    // 駒の情報の配列を初期化。
    FOR_SIDES(side) {
      FOR_PIECE_TYPES(piece_type) {
        FOR_SQUARES(square) {
          if ((side == NO_SIDE) || (piece_type == EMPTY)) {
            piece_hash_value_table_[side][piece_type][square] = 0;
          } else {
            piece_hash_value_table_[side][piece_type][square] =
            temp_table[temp_count++];
          }
        }
      }
    }

    // 手番の配列を初期化。
    to_move_hash_value_table_[NO_SIDE] = 0;
    to_move_hash_value_table_[WHITE] = 0;
    to_move_hash_value_table_[BLACK] = temp_table[temp_count++];

    // キャスリングの配列を初期化。
    for (int i = 0; i < 16; ++i) {
      castling_hash_value_table_[i] = temp_table[temp_count++];
    }

    // アンパッサンの配列を初期化。
    en_passant_hash_value_table_[0] = 0;
    for (Square square = 1; square < NUM_SQUARES; ++square) {
      en_passant_hash_value_table_[square] = temp_table[temp_count++];
    }
  }

  // 定期処理する。
  void ChessEngine::SharedStruct::ThreadPeriodicProcess(UCIShell& shell) {
    static const Chrono::milliseconds SLEEP_TIME(3);
    static const Chrono::seconds INTERVAL(1);
    TimePoint next_point = SysClock::now() + INTERVAL;

    while (!stop_now_) {
      std::this_thread::sleep_for(SLEEP_TIME);
      if (stop_now_) break;

      TimePoint now = SysClock::now();

      // 思考終了判定。
      if (!is_time_over_ && (now >= end_time_)) {
        is_time_over_ = true;
      }

      // 定期出力。
      if (now >= next_point) {
        shell.PrintOtherInfo
        (Chrono::duration_cast<Chrono::milliseconds>(now - start_time_),
        searched_nodes_, table_ptr_->GetUsedPermill());
        next_point = now + INTERVAL;
      }
      if (stop_now_) break;
    }
  }
}  // namespace Sayuri
