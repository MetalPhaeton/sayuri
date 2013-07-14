/* chess_engine.cpp: チェスボードの基本的な実装。
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
#include <ctime>
#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include "chess_def.h"
#include "chess_util.h"
#include "move.h"
#include "transposition_table.h"

#include "sayuri_debug.h"

namespace Sayuri {
  /******************
   * テスト用関数。 *
   ******************/
  void ChessEngine::Test() {
  }

  /********************************
   * チェスボード用の出力演算子。 *
   ********************************/
  std::ostream& operator<<(std::ostream& stream, const ChessEngine& board) {
    // 駒の配置の文字列の配列。
    // 白のポーンなら"-P-"、黒のポーンなら"<P>"、空白なら"   "。
    char piece_strings[NUM_SQUARES][4];

    // 駒の配置を配列に入れる。
    Bitboard point = 1ULL;
    for (int index = 0; index < NUM_SQUARES; index++) {
      // 終端文字を入れる。
      piece_strings[index][3] = '\0';
      // 白か黒の記号を入れる。
      if (board.side_board_[index] == WHITE) {
        piece_strings[index][0] = '-';
        piece_strings[index][2] = '-';
      } else if (board.side_board_[index] == BLACK) {
        piece_strings[index][0] = '<';
        piece_strings[index][2] = '>';
      } else {
        piece_strings[index][0] = ' ';
        piece_strings[index][2] = ' ';
      }
      // 駒の種類を入れる。
      switch (board.piece_board_[index]) {
        case PAWN:
          piece_strings[index][1] = 'P';
          break;
        case KNIGHT:
          piece_strings[index][1] = 'N';
          break;
        case BISHOP:
          piece_strings[index][1] = 'B';
          break;
        case ROOK:
          piece_strings[index][1] = 'R';
          break;
        case QUEEN:
          piece_strings[index][1] = 'Q';
          break;
        case KING:
          piece_strings[index][1] = 'K';
          break;
        default:
          piece_strings[index][1] = ' ';
          break;
      }
      point <<= 1;
    }

    // ファイルとランクの文字の配列。
    static const char fyle_array[NUM_FYLES] = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };
    static const char rank_array[NUM_RANKS] = {
      '1', '2', '3', '4', '5', '6', '7', '8'
    };

    // ファイルとランクを入れる変数。
    Fyle fyle;
    Rank rank;

    // 手番を文字列ストリームに入れる。
    std::ostringstream to_move_stream;
    to_move_stream << "To Move: ";
    if (board.to_move_ == WHITE) to_move_stream << "White";
    else if (board.to_move_ == BLACK) to_move_stream << "Black";

    // 白キングの位置を文字列ストリームに入れる。
    std::ostringstream white_king_stream;
    white_king_stream << "White King: ";
    fyle = Util::GetFyle(board.king_[WHITE]);
    rank = Util::GetRank(board.king_[WHITE]);
    white_king_stream << fyle_array[fyle] << rank_array[rank];

    // 黒キングの位置を文字列ストリームに入れる。
    std::ostringstream black_king_stream;
    black_king_stream << "Black King: ";
    fyle = Util::GetFyle(board.king_[BLACK]);
    rank = Util::GetRank(board.king_[BLACK]);
    black_king_stream << fyle_array[fyle] << rank_array[rank];

    // 白のキャスリングの権利を文字列ストリームに入れる。
    std::ostringstream white_castling_stream;
    white_castling_stream << "White Castling: ";
    if (board.castling_rights_ & WHITE_SHORT_CASTLING)
      white_castling_stream << "Short ";
    if (board.castling_rights_ & WHITE_LONG_CASTLING)
      white_castling_stream << "Long ";

    // 黒のキャスリングの権利を文字列ストリームに入れる。
    std::ostringstream black_castling_stream;
    black_castling_stream << "Black Castling: ";
    if (board.castling_rights_ & BLACK_SHORT_CASTLING)
      black_castling_stream << "Short ";
    if (board.castling_rights_ & BLACK_LONG_CASTLING)
      black_castling_stream << "Long";

    // アンパッサンのターゲットを文字列ストリームに入れる。
    std::ostringstream en_passant_stream;
    en_passant_stream << "En Passant Target: ";
    if (board.can_en_passant_) {
      fyle = Util::GetFyle(board.en_passant_target_);
      rank = Util::GetRank(board.en_passant_target_);
      en_passant_stream << fyle_array[fyle] << rank_array[rank];
    }

    // ボードの境界線。
    static const char* border = " +---+---+---+---+---+---+---+---+";
    // ボードのファイルの表記。
    static const char* fyle_row = "   a   b   c   d   e   f   g   h";

    // 1行目を出力。
    stream << border << "  " << to_move_stream.str() << std::endl;

    // 2行目を出力。
    stream << "8|";
    for (int index = 56; index <= 63; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << "  " << white_king_stream.str() << std::endl;

    // 3行目を出力。
    stream << border << "  " << black_king_stream.str() << std::endl;

    // 4行目を出力。
    stream << "7|";
    for (int index = 48; index <= 55; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << "  " << white_castling_stream.str() << std::endl;

    // 5行目を出力。
    stream << border << "  " << black_castling_stream.str() << std::endl;

    // 6行目を出力。
    stream << "6|";
    for (int index = 40; index <= 47; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << "  " << en_passant_stream.str() << std::endl;

    // 7行目を出力。
    stream << border << std::endl;

    // 8行目を出力。
    stream << "5|";
    for (int index = 32; index <= 39; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << std::endl;

    // 9行目を出力。
    stream << border << std::endl;

    // 10行目を出力。
    stream << "4|";
    for (int index = 24; index <= 31; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << std::endl;

    // 11行目を出力。
    stream << border << std::endl;

    // 12行目を出力。
    stream << "3|";
    for (int index = 16; index <= 23; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << std::endl;

    // 13行目を出力。
    stream << border << std::endl;

    // 14行目を出力。
    stream << "2|";
    for (int index = 8; index <= 15; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << std::endl;

    // 15行目を出力。
    stream << border << std::endl;

    // 16行目を出力。
    stream << "1|";
    for (int index = 0; index <= 7; index++) {
      stream << piece_strings[index] << "|";
    }
    stream << std::endl;

    // 17行目を出力。
    stream << border << std::endl;

    // 18行目を出力。
    stream << fyle_row << std::endl;

    return stream;
  }

  /**********************************
   * コンストラクタとデストラクタ。 *
   **********************************/
  // コンストラクタ。
  ChessEngine::ChessEngine() :
  to_move_(WHITE),
  castling_rights_(ALL_CASTLING),
  en_passant_target_(0),
  can_en_passant_(false),
  has_white_castled_(false),
  has_black_castled_(false),
  current_game_(0),
  pondering_thread_ptr_(NULL),
  stop_pondering_flag_(true) {
    // 駒の配置を作る。
    // どちらでもない。
    for (int Pieceype = 0; Pieceype < NUM_PIECE_TYPES; Pieceype++) {
      position_[NO_SIDE][Pieceype] = 0;
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
    side_pieces_[NO_SIDE] = 0;
    side_pieces_[WHITE] = 0;
    side_pieces_[BLACK] = 0;
    for (int index = PAWN; index < NUM_PIECE_TYPES; index++) {
      side_pieces_[WHITE] |= position_[WHITE][index];
      side_pieces_[BLACK] |= position_[BLACK][index];
    }

    // ブロッカーのビットボードを作る。
    blocker0_ = side_pieces_[WHITE] | side_pieces_[BLACK];
    blocker45_ = 0;
    blocker90_ = 0;
    blocker135_ = 0;
    Square square;
    for (Bitboard copy = blocker0_; copy; copy &= copy - 1) {
      square = Util::GetSquare(copy);

      blocker45_ |= Util::BIT[Util::ROT45[square]];
      blocker90_ |= Util::BIT[Util::ROT90[square]];
      blocker135_ |= Util::BIT[Util::ROT135[square]];
    }

    // 駒の種類とサイドの配置を作る。
    Bitboard point = 1ULL;
    for (int index = 0; index < NUM_SQUARES; index++) {
      // サイドの配置。
      if (side_pieces_[WHITE] & point) side_board_[index] = WHITE;
      else if (side_pieces_[BLACK] & point) side_board_[index] = BLACK;
      else side_board_[index] = NO_SIDE;

      // 駒の種類の配置。
      if ((point & position_[WHITE][PAWN])
      || (point & position_[BLACK][PAWN])) {
        piece_board_[index] = PAWN;
      } else if ((point & position_[WHITE][KNIGHT])
      || (point & position_[BLACK][KNIGHT])) {
        piece_board_[index] = KNIGHT;
      } else if ((point & position_[WHITE][BISHOP])
      || (point & position_[BLACK][BISHOP])) {
        piece_board_[index] = BISHOP;
      } else if ((point & position_[WHITE][ROOK])
      || (point & position_[BLACK][ROOK])) {
        piece_board_[index] = ROOK;
      } else if ((point & position_[WHITE][QUEEN])
      || (point & position_[BLACK][QUEEN])) {
        piece_board_[index] = QUEEN;
      } else if ((point & position_[WHITE][KING])
      || (point & position_[BLACK][KING])) {
        piece_board_[index] = KING;
      } else {
        piece_board_[index] = EMPTY;
      }
      point <<= 1;
    }

    // キングを入れる。
    king_[NO_SIDE] = A1;  // これは使わない。
    king_[WHITE] = E1;
    king_[BLACK] = E8;

    // ツリーのポインタを初期化する。
    tree_ptr_[0] = &(tree_[0]);
    stack_ptr_[0] = tree_ptr_[0];

    // 現在の局面のハッシュキーを作る。
    HashKey key = 0;
    for (int square = 0; square < NUM_SQUARES; square++) {
      key ^= key_array_[side_board_[square]][piece_board_[square]][square];
    }
  }
  // デストラクタ。
  ChessEngine::~ChessEngine() {
  }

  /********************
   * パブリック関数。 *
   ********************/

  /******************************
   * その他のプライベート関数。 *
   ******************************/
  // 駒を置く。（駒の種類PieceypeにEMPTYをおけば、駒を削除できる。）
  void ChessEngine::PutPiece(Square square, Piece Pieceype, Side side) {
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
    if ((!Pieceype) || (!side)) {
      piece_board_[square] = EMPTY;
      side_board_[square] = NO_SIDE;
      if (placed_piece) {
        blocker0_ &= ~Util::BIT[square];
        blocker45_ &= ~Util::BIT[Util::ROT45[square]];
        blocker90_ &= ~Util::BIT[Util::ROT90[square]];
        blocker135_ &= ~Util::BIT[Util::ROT135[square]];
      }
      return;
    }

    // 置く位置の駒の種類を書き変える。
    piece_board_[square] = Pieceype;
    // 置く位置のサイドを書き変える。
    side_board_[square] = side;

    // 置く位置のビットボードをセットする。
    position_[side][Pieceype] |= Util::BIT[square];
    side_pieces_[side] |= Util::BIT[square];
    blocker0_ |= Util::BIT[square];
    blocker45_ |= Util::BIT[Util::ROT45[square]];
    blocker90_ |= Util::BIT[Util::ROT90[square]];
    blocker135_ |= Util::BIT[Util::ROT135[square]];

    // キングの位置を更新する。
    if (Pieceype == KING) {
      king_[side] = square;
    }
  }
  // 駒の位置を変える。
  void ChessEngine::ReplacePiece(Square piece_square, Square goal_square) {
    // 移動する位置と移動先の位置が同じなら何もしない。
    if (piece_square == goal_square) return;

    // 移動する位置の駒の種類を得る。空なら何もしない。
    Piece Pieceype = piece_board_[piece_square];
    if (!Pieceype) return;
    // 移動する駒のサイドを得る。
    Side side = side_board_[piece_square];

    // 移動先の駒の種類を得る。
    Piece placed_Pieceype = piece_board_[goal_square];
    // 移動先の駒のサイドを得る。
    Side placed_side = side_board_[goal_square];

    // 移動する駒のメンバを消す。
    piece_board_[piece_square] = EMPTY;
    side_board_[piece_square] = NO_SIDE;
    position_[side][Pieceype] &= ~Util::BIT[piece_square];
    side_pieces_[side] &= ~Util::BIT[piece_square];
    blocker0_ &= ~Util::BIT[piece_square];
    blocker45_ &= ~Util::BIT[Util::ROT45[piece_square]];
    blocker90_ &= ~Util::BIT[Util::ROT90[piece_square]];
    blocker135_ &= ~Util::BIT[Util::ROT135[piece_square]];

    // 移動先の駒を消す。
    if (placed_Pieceype) {
      position_[placed_side][placed_Pieceype] &=
      ~Util::BIT[goal_square];
      side_pieces_[placed_side] &= ~Util::BIT[goal_square];
    }

    // 移動先の駒をセットする。
    piece_board_[goal_square] = Pieceype;
    side_board_[goal_square] = side;
    position_[side][Pieceype] |= Util::BIT[goal_square];
    side_pieces_[side] |= Util::BIT[goal_square];
    if (!placed_Pieceype) {
      blocker0_ |= Util::BIT[goal_square];
      blocker45_ |= Util::BIT[Util::ROT45[goal_square]];
      blocker90_ |= Util::BIT[Util::ROT90[goal_square]];
      blocker135_ |= Util::BIT[Util::ROT135[goal_square]];
    }

    // キングの位置を更新する。
    if (Pieceype == KING) {
      king_[side] = goal_square;
    }
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
    // サイドを確認する。
    if (side == NO_SIDE) return 0;

    // 白のマテリアル。
    int white_material = 0;
    white_material += SCORE_PAWN
    * Util::CountBits(position_[WHITE][PAWN]);
    white_material += SCORE_KNIGHT
    * Util::CountBits(position_[WHITE][KNIGHT]);
    white_material += SCORE_BISHOP
    * Util::CountBits(position_[WHITE][BISHOP]);
    white_material += SCORE_ROOK
    * Util::CountBits(position_[WHITE][ROOK]);
    white_material += SCORE_QUEEN
    * Util::CountBits(position_[WHITE][QUEEN]);
    white_material += SCORE_KING
    * Util::CountBits(position_[WHITE][KING]);

    // 黒のマテリアル。
    int black_material = 0;
    black_material += SCORE_PAWN
    * Util::CountBits(position_[BLACK][PAWN]);
    black_material += SCORE_KNIGHT
    * Util::CountBits(position_[BLACK][KNIGHT]);
    black_material += SCORE_BISHOP
    * Util::CountBits(position_[BLACK][BISHOP]);
    black_material += SCORE_ROOK
    * Util::CountBits(position_[BLACK][ROOK]);
    black_material += SCORE_QUEEN
    * Util::CountBits(position_[BLACK][QUEEN]);
    black_material += SCORE_KING
    * Util::CountBits(position_[BLACK][KING]);

    // マテリアルを計算して返す。
    int material = white_material - black_material;
    return side == WHITE ? material : -material;
  }
  // 合法手があるかどうかチェックする。
  bool ChessEngine::HasLegalMove(Side side) const {
    // サイドがどちらでもなければfalse。
    if (side == NO_SIDE) return false;

    // constを取る。
    ChessEngine* self = const_cast<ChessEngine*>(this);

    // スレッドをロック。
    boost::mutex::scoped_lock lock(self->sync_);

    // 敵のサイド。
    Side enemy_side = side ^ 0x3;

    // 自分の駒。
    Bitboard pieces = side_pieces_[side];

    // それぞれの駒を調べてみる。
    Move move;  // 手。
    Square piece_square;  // 駒の位置。
    Square goal_square;  // 移動先の位置。
    Piece Pieceype;  // 駒の種類。
    Bitboard move_bitboard;  // 動ける位置のビットボード。
    Side save_to_move;  // 手番を保存。
    for (; pieces; pieces &= pieces - 1) {
      piece_square = Util::GetSquare(pieces);
      Pieceype = piece_board_[piece_square];
      switch (Pieceype) {
        case PAWN:  // ポーンの場合。
          // 通常の動き。
          move_bitboard = Util::GetPawnMove(piece_square, side)
          & ~blocker0_;
          // 2歩の動き。
          if (move_bitboard) {
            move_bitboard |= Util::GetPawn2StepMove(piece_square, side)
            & ~blocker0_;
          }
          // 攻撃の動き。
          move_bitboard |= Util::GetPawnAttack(piece_square, side)
          & side_pieces_[enemy_side];
          // アンパッサンの動き。
          if (can_en_passant_) {
            if ((side == WHITE)
            && (side_board_[en_passant_target_] == BLACK)) {
              Rank target_rank = Util::GetRank(en_passant_target_);
              Rank attacker_rank = Util::GetRank(piece_square);
              if (target_rank == attacker_rank) {
                if ((piece_square == (en_passant_target_ - 1))
                || (piece_square == (en_passant_target_ + 1))) {
                  move_bitboard |= Util::BIT[en_passant_target_ + 8];
                }
              }
            } else if ((side == BLACK)
            && (side_board_[en_passant_target_] == WHITE)){
              Rank target_rank = Util::GetRank(en_passant_target_);
              Rank attacker_rank = Util::GetRank(piece_square);
              if (target_rank == attacker_rank) {
                if ((piece_square == (en_passant_target_ - 1))
                || (piece_square == (en_passant_target_ + 1))) {
                  move_bitboard |= Util::BIT[en_passant_target_ - 8];
                }
              }
            }
          }
          break;
        case KNIGHT:  // ナイトの場合。
          move_bitboard = Util::GetKnightMove(piece_square)
          & ~side_pieces_[side];
          break;
        case BISHOP:  // ビショップの場合。
          move_bitboard = GetBishopAttack(piece_square)
          & ~side_pieces_[side];
          break;
        case ROOK:  // ルークの場合。
          move_bitboard = GetRookAttack(piece_square)
          & ~side_pieces_[side];
          break;
        case QUEEN:  // クイーンの場合。
          move_bitboard = GetQueenAttack(piece_square)
          & ~side_pieces_[side];
          break;
        case KING:  // キングの場合。
          move_bitboard = Util::GetKingMove(piece_square)
          & ~side_pieces_[side];
          // キャスリングの動き。
          // 白のショートキャスリング。
          if ((side == WHITE) && (castling_rights_ & WHITE_SHORT_CASTLING)) {
            if (!IsAttacked(E1, enemy_side)
            && !IsAttacked(F1, enemy_side)
            && !IsAttacked(G1, enemy_side)) {
              if (!piece_board_[F1] && !piece_board_[G1]) {
                move_bitboard |= Util::BIT[G1];
              }
            }
          }
          // 白のロングキャスリング。
          if ((side == WHITE) && (castling_rights_ & WHITE_LONG_CASTLING)) {
            if (!IsAttacked(E1, enemy_side)
            && !IsAttacked(D1, enemy_side)
            && !IsAttacked(C1, enemy_side)) {
              if (!piece_board_[D1] && !piece_board_[C1]
              && !piece_board_[B1]) {
                move_bitboard |= Util::BIT[C1];
              }
            }
          }
          // 黒のショートキャスリング。
          if ((side == BLACK) && (castling_rights_ & BLACK_SHORT_CASTLING)) {
            if (!IsAttacked(E8, enemy_side)
            && !IsAttacked(F8, enemy_side)
            && !IsAttacked(G8, enemy_side)) {
              if (!piece_board_[F8] && !piece_board_[G8]) {
                move_bitboard |= Util::BIT[G8];
              }
            }
          } // 黒のロングキャスリング。
          if ((side == BLACK) && (castling_rights_ & BLACK_LONG_CASTLING)) {
            if (!IsAttacked(E8, enemy_side)
            && !IsAttacked(D8, enemy_side)
            && !IsAttacked(C8, enemy_side)) {
              if (!piece_board_[D8] && !piece_board_[C8]
              && !piece_board_[B8]) {
                move_bitboard |= Util::BIT[C8];
              }
            }
          }
          break;
      }

      // それぞれの動きを調べる。
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        goal_square = Util::GetSquare(move_bitboard);
        move.all_ = 0;

        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;

        // キングを取る手は無視する。
        if (goal_square == king_[enemy_side]) continue;

        // アンパッサンやキャスリングの手の種類を追加する。
        if ((can_en_passant_) && (Pieceype == PAWN)) {  // アンパッサン。
          if (((side == WHITE) && (goal_square == en_passant_target_ + 8))
          || ((side == BLACK) && (goal_square == en_passant_target_ - 8))) {
            move.move_type_ = EN_PASSANT;
          }
        } else if (Pieceype == KING) {  // キャスリング。
          if (side == WHITE) {  // 白のキャスリング。
            if (((piece_square == E1) && (goal_square == G1))
            || ((piece_square == E1) && (goal_square == C1))) {
              move.move_type_ = CASTLING;
            }
          } else {  // 黒のキャスリング。
            if (((piece_square == E8) && (goal_square == G8))
            || ((piece_square == E8) && (goal_square == C8))) {
              move.move_type_ = CASTLING;
            }
          }
        }

        // 動かしてみる。
        // 動けるならtrueを返す。
        save_to_move = self->to_move_;
        self->to_move_ = side;
        self->MakeMove(move);
        if (!IsAttacked(king_[side], enemy_side)) {
          self->UnmakeMove(move);
          self->to_move_ = save_to_move;
          return true;
        }
        self->UnmakeMove(move);
        self->to_move_ = save_to_move;
      }
    }

    // 動けなかったのでfalse。
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
      blocker = blocker0_ & ~(attackers | line_attackers);
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
      blocker = blocker0_ & ~(attackers | line_attackers);
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
    move.last_en_passant_target_ = en_passant_target_;

    // NULL_MOVEならNull moveする。
    if (move.move_type_ == NULL_MOVE) {
      can_en_passant_ = false;
      return;
    }

    // 移動前と移動後の位置を得る。
    Square piece_square = move.piece_square_;
    Square goal_square = move.goal_square_;
    // 移動前と移動後が同じならNull Move。
    if (piece_square == goal_square) {
      move.move_type_ = NULL_MOVE;
      can_en_passant_ = false;
      return;
    }

    // 手の種類によって分岐する。
    if (move.move_type_ == CASTLING) {  // キャスリングの場合。
      // キングを動かす。
      ReplacePiece(piece_square, goal_square);
      // ルークを動かす。
      if (goal_square == G1) {
        ReplacePiece(H1, F1);
      } else if (goal_square == C1) {
        ReplacePiece(A1, D1);
      } else if (goal_square == G8) {
        ReplacePiece(H8, F8);
      } else if (goal_square == C8) {
        ReplacePiece(A8, D8);
      }
      can_en_passant_ = false;
      // キャスリングをしたかどうかをセットする。
      if (side == WHITE) has_white_castled_ = true;
      else has_black_castled_ = true;
    } else if (move.move_type_ == EN_PASSANT) {  // アンパッサンの場合。
      // 取った駒をボーンにする。
      move.captured_piece_ = PAWN;
      // 動かす。
      ReplacePiece(piece_square, goal_square);
      // アンパッサンのターゲットを消す。
      PutPiece(en_passant_target_, EMPTY);

      can_en_passant_ = false;
    } else {  // それ以外の場合。
      // 取る駒を登録する。
      move.captured_piece_ = piece_board_[goal_square];
      // 駒を動かす。
      ReplacePiece(piece_square, goal_square);
      // 駒を昇格させるなら、駒を昇格させる。
      Piece promotion = move.promotion_;
      if (promotion) {
        PutPiece(goal_square, promotion, side);
      }
      // ポーンの2歩の動きの場合はアンパッサンできるようにする。
      if (piece_board_[goal_square] == PAWN) {
        if (((side == WHITE) && ((piece_square + 16) == goal_square))
        || ((side == BLACK) && ((piece_square - 16) == goal_square))) {
          can_en_passant_ = true;
          en_passant_target_ = goal_square;
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
    en_passant_target_ = move.last_en_passant_target_;

    // moveがNULL_MOVEなら返る。
    if (move.move_type_ == NULL_MOVE) {
      return;
    }

    // 移動前と移動後の位置を得る。
    Square piece_square = move.piece_square_;
    Square goal_square = move.goal_square_;

    // 駒の位置を戻す。
    ReplacePiece(goal_square, piece_square);

    // 手の種類で分岐する。
    if (move.move_type_ == CASTLING) {  // キャスリングの場合。
      // ルークを戻す。
      if (goal_square == G1) {
        ReplacePiece(F1, H1);
      } else if (goal_square == C1) {
        ReplacePiece(D1, A1);
      } else if (goal_square == G8) {
        ReplacePiece(F8, H8);
      } else if (goal_square == C8) {
        ReplacePiece(D8, A8);
      }
      // キャスリングしたかどうかをセットする。
      if (to_move_ == WHITE) has_white_castled_ = false;
      else has_black_castled_ = false;
    } else if (move.move_type_ == EN_PASSANT) {  // アンパッサンの場合。
      // アンパッサンのターゲットを戻す。
      PutPiece(en_passant_target_, move.captured_piece_, enemy_side);
    } else {  // それ以外の場合。
      // 取った駒を戻す。
      if (move.captured_piece_) {
        PutPiece(goal_square, move.captured_piece_, enemy_side);
      }
      // 昇格ならポーンに戻す。
      if (move.promotion_) {
        PutPiece(piece_square, PAWN, to_move_);
      }
    }
  }

  /**********************
   * ハッシュキー関連。 *
   **********************/
  // ハッシュキーの配列。
  HashKey ChessEngine::key_array_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
  // 乱数の種。
  uint64_t ChessEngine::seed_;
  // key_array_[][][]を初期化する。
  void ChessEngine::InitKeyArray()
  {
    // 乱数の種を初期化。
    seed_ = 1;

    // キーの配列を初期化。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int Pieceype = 0; Pieceype < NUM_PIECE_TYPES; Pieceype++) {
        for (int square = 0; square < NUM_SQUARES; square++) {
          if ((side == NO_SIDE) || (Pieceype == EMPTY)) {
            key_array_[side][Pieceype][square] = 0;
          } else {
            key_array_[side][Pieceype][square] = GetRand();
          }
        }
      }
    }
  }
  // 次の局面のハッシュキーを得る。
  HashKey ChessEngine::GetNextKey(HashKey current_key, Move move) const {
    // 駒の位置の種類とサイドを得る。
    Piece Pieceype = piece_board_[move.piece_square_];
    Side piece_side = side_board_[move.piece_square_];

    // 移動する位置の駒の種類とサイドを得る。
    Piece goal_type = piece_board_[move.goal_square_];
    Side goal_side = side_board_[move.goal_square_];

    // 移動する駒の移動元のハッシュキーを得る。
    HashKey piece_key =
    key_array_[piece_side][Pieceype][move.piece_square_];

    // 移動する位置のハッシュキーを得る。
    HashKey goal_key =
    key_array_[goal_side][goal_type][move.goal_square_];

    // 移動する駒の移動先のハッシュキーを得る。
    HashKey move_key;
    if (move.promotion_) {
      move_key =
      key_array_[piece_side][move.promotion_][move.goal_square_];
    } else {
      move_key =
      key_array_[piece_side][Pieceype][move.goal_square_];
    }

    // 移動する駒の移動元のハッシュキーを削除する。
    current_key ^= piece_key;

    // 移動する位置のハッシュキーを削除する。
    current_key ^= goal_key;

    // 移動する駒の移動先のハッシュキーを追加する。
    current_key ^= move_key;

    // 次の局面のハッシュキーを返す。
    return current_key;
  }
}  // namespace Sayuri
