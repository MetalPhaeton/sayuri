# The MIT License (MIT)
# 
# Copyright (c) 2016 Hironori Ishibashi
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Eval Params Windows."""

import sys, os
import tkinter as tk
from src.common import *
from src.model import *
from src.base_window import *
from src.mywidget import *
from src.linegraph_frame import *
from src.board_frame import *

class PawnOpeningPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Opening Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_opening_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.pawn_square_table_opening)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_opening_position = self.weight_graph.GetData()
        self.model.pawn_square_table_opening = self.board.GetData()

        self.OnCancel()

class KnightOpeningPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Opening Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_opening_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.knight_square_table_opening)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_opening_position = self.weight_graph.GetData()
        self.model.knight_square_table_opening = self.board.GetData()

        self.OnCancel()

class BishopOpeningPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Opening Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_opening_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.bishop_square_table_opening)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_opening_position = self.weight_graph.GetData()
        self.model.bishop_square_table_opening = self.board.GetData()

        self.OnCancel()

class RookOpeningPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Opening Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_opening_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.rook_square_table_opening)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_opening_position = self.weight_graph.GetData()
        self.model.rook_square_table_opening = self.board.GetData()

        self.OnCancel()

class QueenOpeningPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Opening Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_opening_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.queen_square_table_opening)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_opening_position = self.weight_graph.GetData()
        self.model.queen_square_table_opening = self.board.GetData()

        self.OnCancel()

class KingOpeningPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Opening Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_opening_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.king_square_table_opening)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_opening_position = self.weight_graph.GetData()
        self.model.king_square_table_opening = self.board.GetData()

        self.OnCancel()

class PawnEndingPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Ending Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_ending_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.pawn_square_table_ending)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_ending_position = self.weight_graph.GetData()
        self.model.pawn_square_table_ending = self.board.GetData()

        self.OnCancel()

class KnightEndingPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Ending Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_ending_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.knight_square_table_ending)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_ending_position = self.weight_graph.GetData()
        self.model.knight_square_table_ending = self.board.GetData()

        self.OnCancel()

class BishopEndingPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Ending Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_ending_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.bishop_square_table_ending)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_ending_position = self.weight_graph.GetData()
        self.model.bishop_square_table_ending = self.board.GetData()

        self.OnCancel()

class RookEndingPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Ending Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_ending_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.rook_square_table_ending)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_ending_position = self.weight_graph.GetData()
        self.model.rook_square_table_ending = self.board.GetData()

        self.OnCancel()

class QueenEndingPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Ending Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_ending_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.queen_square_table_ending)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_ending_position = self.weight_graph.GetData()
        self.model.queen_square_table_ending = self.board.GetData()

        self.OnCancel()

class KingEndingPositionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Ending Position")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_ending_position)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.king_square_table_ending)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_ending_position = self.weight_graph.GetData()
        self.model.king_square_table_ending = self.board.GetData()

        self.OnCancel()

class PawnMobilityConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Mobility")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_mobility)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_mobility = self.weight_graph.GetData()

        self.OnCancel()

class KnightMobilityConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Mobility")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_mobility)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_mobility = self.weight_graph.GetData()

        self.OnCancel()

class BishopMobilityConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Mobility")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_mobility)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_mobility = self.weight_graph.GetData()

        self.OnCancel()

class RookMobilityConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Mobility")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_mobility)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_mobility = self.weight_graph.GetData()

        self.OnCancel()

class QueenMobilityConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Mobility")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_mobility)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_mobility = self.weight_graph.GetData()

        self.OnCancel()

class KingMobilityConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Mobility")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_mobility)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_mobility = self.weight_graph.GetData()

        self.OnCancel()

class PawnCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_center_control = self.weight_graph.GetData()

        self.OnCancel()

class KnightCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_center_control = self.weight_graph.GetData()

        self.OnCancel()

class BishopCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_center_control = self.weight_graph.GetData()

        self.OnCancel()

class RookCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_center_control = self.weight_graph.GetData()

        self.OnCancel()

class QueenCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_center_control = self.weight_graph.GetData()

        self.OnCancel()

class KingCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_center_control = self.weight_graph.GetData()

        self.OnCancel()

class PawnSweetCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Sweet Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_sweet_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_sweet_center_control = \
        self.weight_graph.GetData()

        self.OnCancel()

class KnightSweetCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Sweet Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_sweet_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_sweet_center_control = \
        self.weight_graph.GetData()

        self.OnCancel()

class BishopSweetCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Sweet Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_sweet_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_sweet_center_control = \
        self.weight_graph.GetData()

        self.OnCancel()

class RookSweetCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Sweet Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_sweet_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_sweet_center_control = \
        self.weight_graph.GetData()

        self.OnCancel()

class QueenSweetCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Sweet Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_sweet_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_sweet_center_control = \
        self.weight_graph.GetData()

        self.OnCancel()

class KingSweetCenterControlConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Sweet Center Control")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_sweet_center_control)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_sweet_center_control = \
        self.weight_graph.GetData()

        self.OnCancel()

class PawnDevelopmentConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Development")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_development)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_development = self.weight_graph.GetData()

        self.OnCancel()

class KnightDevelopmentConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Development")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_development)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_development = self.weight_graph.GetData()

        self.OnCancel()

class BishopDevelopmentConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Development")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_development)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_development = self.weight_graph.GetData()

        self.OnCancel()

class RookDevelopmentConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Development")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_development)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_development = self.weight_graph.GetData()

        self.OnCancel()

class QueenDevelopmentConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Development")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_development)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_development = self.weight_graph.GetData()

        self.OnCancel()

class KingDevelopmentConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Development")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_development)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_development = self.weight_graph.GetData()

        self.OnCancel()

class PieceDataFrame(tk.LabelFrame):
    def __init__(self, master, data):
        tk.LabelFrame.__init__(self, master, text="Piece Value")

        self.entry_list = []
        self.entry_list += [MyEntry(self, "Value for Pawn")]
        self.entry_list += [MyEntry(self, "Value for Knight")]
        self.entry_list += [MyEntry(self, "Value for Bishop")]
        self.entry_list += [MyEntry(self, "Value for Rook")]
        self.entry_list += [MyEntry(self, "Value for Queen")]
        self.entry_list += [MyEntry(self, "Value for King")]

        for i in range(1, len(data)):
            self.entry_list[i - 1].Set(data[i])
            self.entry_list[i - 1].pack(**PACK_PARAMS)

    def GetData(self):
        ret = [0.0]
        for elm in self.entry_list: ret += [elm.GetFloat()]
        return ret

class PawnAttackConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Attack")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_attack)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.pawn_attack_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_attack = self.weight_graph.GetData()
        self.model.pawn_attack_table = self.piece_data_frame.GetData()

        self.OnCancel()

class KnightAttackConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Attack")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_attack)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.knight_attack_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_attack = self.weight_graph.GetData()
        self.model.knight_attack_table = self.piece_data_frame.GetData()

        self.OnCancel()

class BishopAttackConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Attack")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_attack)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.bishop_attack_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_attack = self.weight_graph.GetData()
        self.model.bishop_attack_table = self.piece_data_frame.GetData()

        self.OnCancel()

class RookAttackConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Attack")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_attack)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.rook_attack_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_attack = self.weight_graph.GetData()
        self.model.rook_attack_table = self.piece_data_frame.GetData()

        self.OnCancel()

class QueenAttackConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Attack")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_attack)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.queen_attack_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_attack = self.weight_graph.GetData()
        self.model.queen_attack_table = self.piece_data_frame.GetData()

        self.OnCancel()

class KingAttackConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Attack")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_attack)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.king_attack_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_attack = self.weight_graph.GetData()
        self.model.king_attack_table = self.piece_data_frame.GetData()

        self.OnCancel()

class PawnDefenseConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Defense")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_defense)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.pawn_defense_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_defense = self.weight_graph.GetData()
        self.model.pawn_defense_table = self.piece_data_frame.GetData()

        self.OnCancel()

class KnightDefenseConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Defense")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_defense)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.knight_defense_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_defense = self.weight_graph.GetData()
        self.model.knight_defense_table = self.piece_data_frame.GetData()

        self.OnCancel()

class BishopDefenseConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Defense")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_defense)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.bishop_defense_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_defense = self.weight_graph.GetData()
        self.model.bishop_defense_table = self.piece_data_frame.GetData()

        self.OnCancel()

class RookDefenseConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Defense")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_defense)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.rook_defense_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_defense = self.weight_graph.GetData()
        self.model.rook_defense_table = self.piece_data_frame.GetData()

        self.OnCancel()

class QueenDefenseConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Defense")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_defense)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.queen_defense_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_defense = self.weight_graph.GetData()
        self.model.queen_defense_table = self.piece_data_frame.GetData()

        self.OnCancel()

class KingDefenseConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Defense")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_defense)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        self.piece_data_frame = \
        PieceDataFrame(self.content_frame, model.king_defense_table)
        self.piece_data_frame.pack(side=tk.LEFT, anchor=tk.NW, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_defense = self.weight_graph.GetData()
        self.model.king_defense_table = self.piece_data_frame.GetData()

        self.OnCancel()

class PawnAttackAroundKingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Attack around Enemy King")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_attack_around_king)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_attack_around_king = \
        self.weight_graph.GetData()

        self.OnCancel()

class KnightAttackAroundKingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Knight Attack around Enemy King")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_knight_attack_around_king)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_knight_attack_around_king = \
        self.weight_graph.GetData()

        self.OnCancel()

class BishopAttackAroundKingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Attack around Enemy King")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_attack_around_king)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_attack_around_king = \
        self.weight_graph.GetData()

        self.OnCancel()

class RookAttackAroundKingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Attack around Enemy King")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_attack_around_king)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_attack_around_king = \
        self.weight_graph.GetData()

        self.OnCancel()

class QueenAttackAroundKingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Attack around Enemy King")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_queen_attack_around_king)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_queen_attack_around_king = \
        self.weight_graph.GetData()

        self.OnCancel()

class KingAttackAroundKingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "King Attack around Enemy King")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_king_attack_around_king)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_king_attack_around_king = \
        self.weight_graph.GetData()

        self.OnCancel()

class PassPawnConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pass Pawn")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pass_pawn)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pass_pawn = \
        self.weight_graph.GetData()

        self.OnCancel()

class ProtectedPassPawnConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Protected Pass Pawn")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_protected_pass_pawn)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_protected_pass_pawn = \
        self.weight_graph.GetData()

        self.OnCancel()

class DoublePawnConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Double Pawn")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_double_pawn)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_double_pawn = \
        self.weight_graph.GetData()

        self.OnCancel()

class IsoPawnConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Iso Pawn")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_iso_pawn)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_iso_pawn = \
        self.weight_graph.GetData()

        self.OnCancel()

class PawnShieldConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Pawn Shield")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_pawn_shield)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

        # Board.
        self.board = BoardFrame(self.content_frame, \
        self.model.pawn_shield_table)
        self.board.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_pawn_shield = self.weight_graph.GetData()
        self.model.pawn_shield_table = self.board.GetData()

        self.OnCancel()

class BishopPairConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bishop Pair")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bishop_pair)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bishop_pair = self.weight_graph.GetData()

        self.OnCancel()

class BadBishopConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Bad Bishop")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_bad_bishop)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_bad_bishop = self.weight_graph.GetData()

        self.OnCancel()

class RookPairConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook Pari")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_pair)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_pair = self.weight_graph.GetData()

        self.OnCancel()

class RookSemiOpenFyleConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook on Semi-Open File")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_semiopen_fyle)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_semiopen_fyle = self.weight_graph.GetData()

        self.OnCancel()

class RookOpenFyleConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Rook on Open File")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_rook_open_fyle)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_rook_open_fyle = self.weight_graph.GetData()

        self.OnCancel()

class EarlyQueenStartingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Queen Starting Too Early")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_early_queen_starting)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_early_queen_starting = self.weight_graph.GetData()

        self.OnCancel()

class WeakSquareConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Weak Square around Castled King")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_weak_square)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_weak_square = self.weight_graph.GetData()

        self.OnCancel()

class CastlingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Castling")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_castling)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_castling = self.weight_graph.GetData()

        self.OnCancel()

class AbandonedCastlingConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Abandoned Castling Rights")
        self.model = model

        # Weight.
        self.weight_graph = LineGraphFrame(self.content_frame, \
        self.model.weight_abandoned_castling)
        self.weight_graph.pack(side=tk.LEFT, **PACK_PARAMS)

    def OnOk(self):
        self.model.weight_abandoned_castling = self.weight_graph.GetData()

        self.OnCancel()

if __name__ == "__main__":
    pass
