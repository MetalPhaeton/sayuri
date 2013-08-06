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
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"
#include "fen.h"
#include "move_maker.h"
#include "error.h"
#include "uci_shell.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /***************************/
  // コンストラクタ。
  ChessEngine::ChessEngine():
  table_size_(TT_MIN_SIZE_BYTES) {
    SetNewGame();
  }

  // コピーコンストラクタ。
  ChessEngine::ChessEngine(const ChessEngine& engine) {
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

    // ヒストリーのコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = engine.history_[i][j][k];
        }
      }
    }

    // iid_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = engine.iid_stack_[i];
    }
    // killer_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      killer_stack_[i] = engine.killer_stack_[i];
    }

    // 指し手の履歴をコピー。
    move_history_ = engine.move_history_;

    // 50手ルールの履歴をコピー。
    ply_100_history_ = engine.ply_100_history_;

    // トランスポジションテーブルのサイズ。
    table_size_ = engine.table_size_;
  }

  // ムーブコンストラクタ。
  ChessEngine::ChessEngine(ChessEngine&& engine) {
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

    // ヒストリーのコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = engine.history_[i][j][k];
        }
      }
    }

    // iid_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = engine.iid_stack_[i];
    }
    // killer_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      killer_stack_[i] = engine.killer_stack_[i];
    }

    // 指し手の履歴をムーブ。
    move_history_ = std::move(engine.move_history_);

    // 50手ルールの履歴をコピー。
    ply_100_history_ = std::move(engine.ply_100_history_);

    // トランスポジションテーブルのサイズ。
    table_size_ = engine.table_size_;
  }

  // コピー代入。
  ChessEngine& ChessEngine::operator=(const ChessEngine& engine) {
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

    // ヒストリーのコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = engine.history_[i][j][k];
        }
      }
    }

    // iid_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = engine.iid_stack_[i];
    }
    // killer_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      killer_stack_[i] = engine.killer_stack_[i];
    }

    // 指し手の履歴をコピー。
    move_history_ = engine.move_history_;

    // 50手ルールの履歴をコピー。
    ply_100_history_ = engine.ply_100_history_;

    // トランスポジションテーブルのサイズ。
    table_size_ = engine.table_size_;

    return *this;
  }

  // ムーブ代入。
  ChessEngine& ChessEngine::operator=(ChessEngine&& engine) {
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

    // ヒストリーのコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = engine.history_[i][j][k];
        }
      }
    }

    // iid_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i] = engine.iid_stack_[i];
    }
    // killer_stack_のコピー。
    for (int i = 0; i < MAX_PLYS; i++) {
      killer_stack_[i] = engine.killer_stack_[i];
    }

    // 指し手の履歴をムーブ。
    move_history_ = std::move(engine.move_history_);

    // 50手ルールの履歴をコピー。
    ply_100_history_ = std::move(engine.ply_100_history_);

    // トランスポジションテーブルのサイズ。
    table_size_ = engine.table_size_;

    return *this;
  }
  // デストラクタ。
  ChessEngine::~ChessEngine() {
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
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        PutPiece(square, EMPTY, side);
      }
    }
          

    // 配置をする。
    Bitboard bb;
    Square square;
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        // 駒をセット。
        bb = fen.position()[side][piece_type];
        for (; bb; bb &= bb - 1) {
          square = Util::GetSquare(bb);
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
  }

  // 新しいゲームの準備をする。
  void ChessEngine::SetNewGame() {
    // 駒の配置を作る。
    // どちらでもない。
    for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      position_[NO_SIDE][piece_type] = 0;
    }
    // 白の駒。
    position_[WHITE][EMPTY] = 0;
    position_[WHITE][PAWN] = Util::RANK[RANK_2];
    position_[WHITE][KNIGHT] = Util::BIT[B1] | Util::BIT[G1];
    position_[WHITE][BISHOP] = Util::BIT[C1] | Util::BIT[F1];
    position_[WHITE][ROOK] = Util::BIT[A1] | Util::BIT[H1];
    position_[WHITE][QUEEN] = Util::BIT[D1];
    position_[WHITE][KING] = Util::BIT[E1];
    // 黒の駒。
    position_[BLACK][EMPTY] = 0;
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
    blocker_45_ = 0;
    blocker_90_ = 0;
    blocker_135_ = 0;
    Square square;
    for (Bitboard copy = blocker_0_; copy; copy &= copy - 1) {
      square = Util::GetSquare(copy);

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

    // ヒストリーの初期化。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_SQUARES; j++) {
        for (int k = 0; k < NUM_SQUARES; k++) {
          history_[i][j][k] = 0;
        }
      }
    }
    history_max_ = 1;

    // iid_stack_の初期化。
    for (int i = 0; i < MAX_PLYS; i++) {
      iid_stack_[i].all_ = 0;
    }
    // killer_stack_の初期化。
    for (int i = 0; i < MAX_PLYS; i++) {
      killer_stack_[i].all_ = 0;
    }

    // 手の履歴を削除。
    move_history_.clear();

    // 50手ルールの履歴を削除。
    ply_100_history_.clear();
  }

  // 思考を始める。
  void ChessEngine::Calculate(PVLine& pv_line,
  std::vector<Move>* moves_to_search_ptr) {
    SearchRoot(pv_line, moves_to_search_ptr);
  }

  // 思考を停止する。
  void ChessEngine::StopCalculation() {
    stopper_.stop_now_ = true;
  }

  // 手を指す。
  void ChessEngine::PlayMove(Move move) {
    // 合法手かどうか調べる。
    // 手を展開する。
    MoveMaker maker(this);
    HashKey temp_key = 0ULL;
    std::unique_ptr<TranspositionTable> temp_table
    (new TranspositionTable(TT_MIN_SIZE_BYTES));
    maker.GenMoves<GenMoveType::ALL>(temp_key, 0, 0, *(temp_table.get()));
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
      if ((move.to_ == temp_move.to_) && (move.from_ == temp_move.from_)
      && (move.promotion_ == temp_move.promotion_)) {
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
      move_history_.push_back(move);
      ply_100_history_.push_back(ply_100_);
      MakeMove(move);
    } else {
      throw SayuriError("合法手ではありません。");
    }
  }

  // 手を戻す。
  void ChessEngine::UndoMove() {
    if (move_history_.begin() < move_history_.end()) {
      ply_--;
      Move move = move_history_.back();
      ply_100_ = ply_100_history_.back();
      move_history_.pop_back();
      ply_100_history_.pop_back();
      UnmakeMove(move);
    } else {
      throw SayuriError("手を戻すことができません。");
    }
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

  // 攻撃されているかどうか調べる。
  bool ChessEngine::IsAttacked(Square square, Side side) const {
    // NO_SIDEならfalse。
    if (side == NO_SIDE) return false;

    // 攻撃。
    Bitboard attack;

    // ポーンに攻撃されているかどうか調べる。
    attack = Util::GetPawnAttack(square, side ^ 0x3);
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
    Assert((side == WHITE) || (side == BLACK));

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

  // 合法手があるかどうかチェックする。
  bool ChessEngine::HasLegalMove(Side side) {
    // 敵のサイド。
    Side enemy_side = side ^ 0x3;
    // 変数。
    Square from;  // 基点。
    Move move;  // 候補手。

    // ナイト、ビショップ、ルーク、クイーンの候補手を調べる。
    Bitboard pieces = 0ULL;
    Bitboard move_bitboard = 0ULL;
    for (int piece_type = KNIGHT; piece_type <= QUEEN; piece_type++) {
      pieces = position_[side][piece_type];

      for (; pieces; pieces &= pieces - 1) {
        from = Util::GetSquare(pieces);

        // 各ピースの動き。
        switch (piece_type) {
          case KNIGHT:
            move_bitboard = Util::GetKnightMove(from);
            break;
          case BISHOP:
            move_bitboard = GetBishopAttack(from);
            break;
          case ROOK:
            move_bitboard = GetRookAttack(from);
            break;
          case QUEEN:
            move_bitboard = GetQueenAttack(from);
            break;
          default:
            throw SayuriError("駒の種類が不正です。");
            break;
        }

        // 見方の駒は取れない。
        move_bitboard &= ~(side_pieces_[side]);

        for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
          move.all_ = 0;
          // 手を作る。
          move.from_ = from;
          move.to_ = Util::GetSquare(move_bitboard);
          move.move_type_ = NORMAL;

          // 動かしてみて、合法手かどうか調べる。
          MakeMove(move);
          if (!(IsAttacked(king_[side], enemy_side))) {
            UnmakeMove(move);
            return true;
          }
          UnmakeMove(move);
        }
      }
    }

    // ポーンの動きを作る。
    pieces = position_[side][PAWN];
    move_bitboard = 0ULL;
    for (; pieces; pieces &= pieces - 1) {
      from = Util::GetSquare(pieces);

      // ポーンの一歩の動き。
      move_bitboard = Util::GetPawnMove(from, side) & ~(blocker_0_);
      if (move_bitboard) {
        if (((side == WHITE) && (Util::GetRank(from) == RANK_2))
        || ((side == BLACK) && (Util::GetRank(from) == RANK_7))) {
          // ポーンの2歩の動き。
          move_bitboard |= Util::GetPawn2StepMove(from, side)
          & ~(blocker_0_);
        }
      }
      // 駒を取る動き。
      move_bitboard |= Util::GetPawnAttack(from, side)
      & side_pieces_[enemy_side];
      if (can_en_passant_) {
        move_bitboard |= Util::BIT[en_passant_square_]
        & Util::GetPawnAttack(from, side);
      }

      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        // 手を作る。
        move.from_ = from;
        move.to_ = Util::GetSquare(move_bitboard);
        if (can_en_passant_ && (move.to_ == en_passant_square_)) {
          move.move_type_ = EN_PASSANT;
        } else {
          move.move_type_ = NORMAL;
        }

        // 動かしてみて、合法手かどうか調べる。
        MakeMove(move);
        if (!(IsAttacked(king_[side], enemy_side))) {
          UnmakeMove(move);
          return true;
        }
        UnmakeMove(move);
      }
    }

    // キングの動きを作る。
    from = king_[side];
    move_bitboard = Util::GetKingMove(from) & ~(side_pieces_[side]);
    // キャスリングの動きを追加。
    if (side == WHITE) {
      if (CanCastling<WHITE_SHORT_CASTLING>()) {
        move_bitboard |= Util::BIT[G1];
      }
      if (CanCastling<WHITE_LONG_CASTLING>()) {
        move_bitboard |= Util::BIT[C1];
      }
    } else {
      if (CanCastling<BLACK_SHORT_CASTLING>()) {
        move_bitboard |= Util::BIT[G8];
      }
      if (CanCastling<BLACK_LONG_CASTLING>()) {
        move_bitboard |= Util::BIT[C8];
      }
    }
    for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
      move.all_ = 0;
      move.from_ = from;
      move.to_ = Util::GetSquare(move_bitboard);

      // キャスリングを設定。
      if (((side == WHITE) && (from == E1)
      && ((move.to_ == G1) || (move.to_ == C1)))
      || ((side == BLACK) && (from == E8)
      && ((move.to_ == G8) || (move.to_ == C8)))) {
        move.move_type_ = CASTLING;
      } else {
        move.move_type_ = NORMAL;
      }

      // 動かしてみて、合法手かどうか調べる。
      MakeMove(move);
      if (!(IsAttacked(king_[side], enemy_side))) {
        UnmakeMove(move);
        return true;
      }
      UnmakeMove(move);
    }

    // 合法手がない。
    return false;
  }
  // 攻撃している駒のビットボードを得る。
  Bitboard ChessEngine::GetAttackers(Square target_square, Side side)
  const {
    // サイドがなければ空を返す。
    if (side == NO_SIDE) return 0;

    // 攻撃している駒のビットボード。
    Bitboard attackers = 0;

    // ポーンを得る。
    attackers |= Util::GetPawnAttack(target_square, side ^ 0x3)
    & position_[side][PAWN];

    // ナイトを得る。
    attackers |= Util::GetKnightMove(target_square)
    & position_[side][KNIGHT];

    // キングを得る。
    attackers |= Util::GetKingMove(target_square)
    & position_[side][KING];

    // ブロッカー。
    Bitboard blocker;

    // ラインアタッカー。（ビショップ、ルーク、クイーン。）
    Bitboard line_attackers;

    // 攻撃駒の位置。
    Square attacker_square;

    // ライン。
    Bitboard line;

    // ビショップ、クイーンの斜めのラインから
    // 攻撃している駒を得る。（X-Rayを含む。）
    // ビショップ、クイーンを特定する。
    line_attackers = Util::GetBishopMove(target_square)
    & (position_[side][BISHOP] | position_[side][QUEEN]);
    if (line_attackers) {
      // そのラインのブロッカーを得る。
      blocker = blocker_0_ & ~(attackers | line_attackers);
      // ラインを調べる。
      for (; line_attackers; line_attackers &= line_attackers - 1) {
        attacker_square = Util::GetSquare(line_attackers);
        line = Util::GetLine(target_square, attacker_square)
        & ~(Util::BIT[target_square] | Util::BIT[attacker_square]);
        if (!(line & blocker)) attackers |= Util::BIT[attacker_square];
      }
    }

    // ルーク、クイーンの縦横のラインから
    // 攻撃している駒を得る。（X-Rayを含む。）
    // ルーク、クイーンを特定する。
    line_attackers = Util::GetRookMove(target_square)
    & (position_[side][ROOK] | position_[side][QUEEN]);
    if (line_attackers) {
      // そのラインのブロッカーを得る。
      blocker = blocker_0_ & ~(attackers | line_attackers);
      // ラインを調べる。
      for (; line_attackers; line_attackers &= line_attackers - 1) {
        attacker_square = Util::GetSquare(line_attackers);
        line = Util::GetLine(target_square, attacker_square)
        & ~(Util::BIT[target_square] | Util::BIT[attacker_square]);
        if (!(line & blocker)) attackers |= Util::BIT[attacker_square];
      }
    }

    return attackers;
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
  /****************
   * 駒を動かす。 *
   ****************/
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
    Square from = move.from_;
    Square to = move.to_;
    // 移動前と移動後が同じならNull Move。
    if (from == to) {
      move.move_type_ = NULL_MOVE;
      can_en_passant_ = false;
      return;
    }

    // 手の種類によって分岐する。
    if (move.move_type_ == CASTLING) {  // キャスリングの場合。
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
      can_en_passant_ = false;
    } else if (move.move_type_ == EN_PASSANT) {  // アンパッサンの場合。
      // 取った駒をボーンにする。
      move.captured_piece_ = PAWN;
      // 動かす。
      ReplacePiece(from, to);
      // アンパッサンのターゲットを消す。
      Square en_passant_target =
      side == WHITE ? en_passant_square_ - 8 : en_passant_square_ + 8;
      PutPiece(en_passant_target, EMPTY);

      can_en_passant_ = false;
    } else {  // それ以外の場合。
      // 取る駒を登録する。
      move.captured_piece_ = piece_board_[to];
      // 駒を動かす。
      ReplacePiece(from, to);
      // 駒を昇格させるなら、駒を昇格させる。
      Piece promotion = move.promotion_;
      if (promotion) {
        PutPiece(to, promotion, side);
      }
      // ポーンの2歩の動きの場合はアンパッサンできるようにする。
      if (piece_board_[to] == PAWN) {
        if (((side == WHITE) && ((from + 16) == to))
        || ((side == BLACK) && ((from - 16) == to))) {
          can_en_passant_ = true;
          en_passant_square_ = side == WHITE ? to - 8 : to + 8;
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

    // 移動前と移動後の位置を得る。
    Square from = move.from_;
    Square to = move.to_;

    // 駒の位置を戻す。
    ReplacePiece(to, from);

    // 手の種類で分岐する。
    if (move.move_type_ == CASTLING) {  // キャスリングの場合。
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
    } else if (move.move_type_ == EN_PASSANT) {  // アンパッサンの場合。
      // アンパッサンのターゲットを戻す。
      Square en_passant_target =
      to_move_ == WHITE ? en_passant_square_ - 8 : en_passant_square_ + 8;
      PutPiece(en_passant_target, move.captured_piece_, enemy_side);
    } else {  // それ以外の場合。
      // 取った駒を戻す。
      if (move.captured_piece_) {
        PutPiece(to, move.captured_piece_, enemy_side);
      }
      // 昇格ならポーンに戻す。
      if (move.promotion_) {
        PutPiece(from, PAWN, to_move_);
      }
    }
  }

  /**********************
   * ハッシュキー関連。 *
   **********************/
  // ハッシュキーの配列。
  HashKey ChessEngine::key_table_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
  // key_table_[][][]を初期化する。
  void ChessEngine::InitKeyTable() {
    // メルセンヌツイスターの準備。
    std::mt19937 engine(SysClock::to_time_t (SysClock::now()));
    std::uniform_int_distribution<HashKey> dist(0ULL, 0xffffffffffffffffULL);

    // キーの配列を初期化。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        for (int square = 0; square < NUM_SQUARES; square++) {
          if ((side == NO_SIDE) || (piece_type == EMPTY)) {
            key_table_[side][piece_type][square] = 0ULL;
          } else {
            key_table_[side][piece_type][square] = dist(engine);
          }
        }
      }
    }
  }
  // 現在の局面のハッシュキーを計算する。
  HashKey ChessEngine::GetCurrentKey() const {
    HashKey key = 0ULL;
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        for (Square square = 0; square < NUM_SQUARES; square++) {
          key ^= key_table_[side][piece_type][square];
        }
      }
    }
    return key;
  }
  // 次の局面のハッシュキーを得る。
  HashKey ChessEngine::GetNextKey(HashKey current_key, Move move) const {
    // 駒の位置の種類とサイドを得る。
    Piece piece_type = piece_board_[move.from_];
    Side piece_side = side_board_[move.from_];

    // 移動する位置の駒の種類とサイドを得る。
    Piece to_type = piece_board_[move.to_];
    Side to_side = side_board_[move.to_];

    // 移動する駒の移動元のハッシュキーを得る。
    HashKey from_key =
    key_table_[piece_side][piece_type][move.from_];

    // 移動する位置のハッシュキーを得る。
    HashKey to_key =
    key_table_[to_side][to_type][move.to_];

    // 移動する駒の移動先のハッシュキーを得る。
    HashKey move_key;
    if (move.promotion_) {
      move_key =
      key_table_[piece_side][move.promotion_][move.to_];
    } else {
      move_key =
      key_table_[piece_side][piece_type][move.to_];
    }

    // 移動する駒の移動元のハッシュキーを削除する。
    current_key ^= from_key;

    // 移動する位置のハッシュキーを削除する。
    current_key ^= to_key;

    // 移動する駒の移動先のハッシュキーを追加する。
    current_key ^= move_key;

    // 次の局面のハッシュキーを返す。
    return current_key;
  }

  // 情報を送る。
  void ChessEngine::SendOtherInfo(const TranspositionTable& table) const {
    // 時間。
    Chrono::milliseconds time = Chrono::duration_cast<Chrono::milliseconds>
    (SysClock::now() - start_time_);

    // トランスポジションテーブルの使用量。
    int hashfull = table.GetEntryPermill();

    // Node Per Seconds。
    int nps = searched_nodes_ / (time.count() / 1000);

    // 標準出力に送る。
    UCIShell::SendOtherInfo(time, hashfull, nps);
  }
}  // namespace Sayuri
