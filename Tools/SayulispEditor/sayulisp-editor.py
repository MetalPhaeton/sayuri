#! /usr/bin/env python3
# coding: utf-8

# The MIT License (MIT)
#
# Copyright (c) 2015 Hironori Ishibashi
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import sys,os,re
import copy
import tkinter as tk
import tkinter.messagebox as tkm
import tkinter.filedialog as tkf

def Tokenize(s):
    ret = []
    blank_c = re.compile(R"\s")
    temp = ""
    in_string = False
    in_comment = False
    in_escape = False
    for c in s:
        if in_comment:
            if c == "\n": in_comment = False
        elif in_string:
            if blank_c.match(c): temp += " "
            elif in_escape:
                in_escape = False
                if c == "n": temp += "\n"
                elif c == "r": temp += "\r"
                elif c == "t": temp += "\t"
                elif c == "b": temp += "\b"
                elif c == "a": temp += "\a"
                elif c == "f": temp += "\f"
                elif c == "0": temp += "\0"
                else: temp += c
            else:
                if c == '"':
                    ret += [temp + c]
                    temp = ""
                    in_string = False
                elif c == "\\": in_escape = True
                else: temp += c
        else:
            if c == ";": in_comment = True
            elif c == '"':
                temp += c
                in_string = True
            elif (c == "'") or (c == "(") or (c == "{") or (c == "[")\
            or (c == ")") or (c == "}") or (c == "]"):
                if temp: ret += [temp]; temp = ""
                ret += [c]
            elif blank_c.match(c):
                if temp: ret += [temp]; temp = ""
            else: temp += c
    if temp: ret += [temp]
    return ret

def SExprToList(s_expr):
    token_list = Tokenize("(" + s_expr + ")")

    def Core(token_list_2):
        if not token_list_2: return None

        front = token_list_2.pop(0)
        if front == "(":
            ret = []
            while True:
                if not token_list_2: return ret
                if token_list_2[0] == ")": token_list_2.pop(0); return ret
                ret += [Core(token_list_2)]
        else:
            if front == "'": return ["quote", Core(token_list_2)]
            elif (front == "#t") or (front == "#T"): return True
            elif (front == "#f") or (front == "#F"): return False
            else:
                try: return float(front)
                except: return front
    return Core(token_list)
                    

def ListToSExpr(li):
    if isinstance(li, list):
        ret = "("
        for i in li: ret += ListToSExpr(i) + " "
        if ret[-1] == " ": ret = ret[:-1]
        return ret + ")"
    elif isinstance(li, bool):
        if li: return "#t"
        else: return "#f"
    elif isinstance(li, float) or isinstance(li, int): return str(li)
    elif isinstance(li, str):
        ret = ""
        for c in li:
            if c == "\n": ret += "\\n"
            elif c == "\r": ret += "\\r"
            elif c == "\t": ret += "\\t"
            elif c == "\a": ret += "\\a"
            elif c == "\b": ret += "\\b"
            elif c == "\f": ret += "\\f"
            elif c == "\\": ret += "\\\\"
            else: ret += c
        return ret
    else: return None

class Editor:
    DEFAULT_ENGINE_NAME = "my-engine"

    MATERIAL_SYMBOL = [["quote", "@material"]]
    ENABLE_QUIESCE_SEARCH_SYMBOL = [["quote", "@enable-quiesce-search"]]
    ENABLE_REPETITION_CHECK_SYMBOL = [["quote", "@enable-repetition-check"]]
    ENABLE_CHECK_EXTENSION_SYMBOL = [["quote", "@enable-check-extension"]]
    YBWC_LIMIT_DEPTH_SYMBOL = [["quote", "@ybwc-limit-depth"]]
    YBWC_INVALID_MOVES_SYMBOL = [["quote", "@ybwc-invalid-moves"]]
    ENABLE_ASPIRATION_WINDOWS_SYMBOL = \
    [["quote", "@enable-aspiration-windows"]]
    ASPIRATION_WINDOWS_LIMIT_DEPTH_SYMBOL = \
    [["quote", "@aspiration-windows-limit-depth"]]
    ASPIRATION_WINDOWS_DELTA_SYMBOL = [["quote", "@aspiration-windows-delta"]]
    ENABLE_SEE_SYMBOL = [["quote", "@enable-see"]]
    ENABLE_HISTORY_SYMBOL = [["quote", "@enable-history"]]
    ENABLE_KILLER_SYMBOL = [["quote", "@enable-killer"]]
    ENABLE_HASH_TABLE_SYMBOL = [["quote", "@enable-hash-table"]]
    ENABLE_IID_SYMBOL = [["quote", "@enable-iid"]]
    IID_LIMIT_DEPTH_SYMBOL = [["quote", "@iid-limit-depth"]]
    IID_SEARCH_DEPTH_SYMBOL = [["quote", "@iid-search-depth"]]
    ENABLE_NMR_SYMBOL = [["quote", "@enable-nmr"]]
    NMR_LIMIT_DEPTH_SYMBOL = [["quote", "@nmr-limit-depth"]]
    NMR_SEARCH_REDUCTION_SYMBOL = [["quote", "@nmr-search-reduction"]]
    NMR_REDUCTION_SYMBOL = [["quote", "@nmr-reduction"]]
    ENABLE_PROBCUT_SYMBOL = [["quote", "@enable-probcut"]]
    PROBCUT_LIMIT_DEPTH_SYMBOL = [["quote", "@probcut-limit-depth"]]
    PROBCUT_MARGIN_SYMBOL = [["quote", "@probcut-margin"]]
    PROBCUT_SEARCH_REDUCTION_SYMBOL = [["quote", "@probcut-search-reduction"]]
    ENABLE_HISTORY_PRUNING_SYMBOL = [["quote", "@enable-history-pruning"]]
    HISTORY_PRUNING_LIMIT_DEPTH_SYMBOL = \
    [["quote", "@history-pruning-limit-depth"]]
    HISTORY_PRUNING_MOVE_THRESHOLD_SYMBOL = \
    [["quote", "@history-pruning-move-threshold"]]
    HISTORY_PRUNING_INVALID_MOVES_SYMBOL = \
    [["quote", "@history-pruning-invalid-moves"]]
    HISTORY_PRUNING_THRESHOLD_SYMBOL = \
    [["quote", "@history-pruning-threshold"]]
    HISTORY_PRUNING_REDUCTION_SYMBOL = \
    [["quote", "@history-pruning-reduction"]]
    ENABLE_LMR_SYMBOL = [["quote", "@enable-lmr"]]
    LMR_LIMIT_DEPTH_SYMBOL = [["quote", "@lmr-limit-depth"]]
    LMR_MOVE_THRESHOLD_SYMBOL = [["quote", "@lmr-move-threshold"]]
    LMR_INVALID_MOVES_SYMBOL = [["quote", "@lmr-invalid-moves"]]
    LMR_SEARCH_REDUCTION_SYMBOL = [["quote", "@lmr-search-reduction"]]
    ENABLE_FUTILITY_PRUNING_SYMBOL = [["quote", "@enable-futility-pruning"]]
    FUTILITY_PRUNING_DEPTH_SYMBOL = [["quote", "@futility-pruning-depth"]]
    FUTILITY_PRUNING_MARGIN_SYMBOL = [["quote", "@futility-pruning-margin"]]
    PAWN_SQUARE_TABLE_OPENING_SYMBOL = \
    [["quote", "@pawn-square-table-opening"]]
    KNIGHT_SQUARE_TABLE_OPENING_SYMBOL = \
    [["quote", "@knight-square-table-opening"]]
    BISHOP_SQUARE_TABLE_OPENING_SYMBOL = \
    [["quote", "@bishop-square-table-opening"]]
    ROOK_SQUARE_TABLE_OPENING_SYMBOL = \
    [["quote", "@rook-square-table-opening"]]
    QUEEN_SQUARE_TABLE_OPENING_SYMBOL = \
    [["quote", "@queen-square-table-opening"]]
    KING_SQUARE_TABLE_OPENING_SYMBOL = \
    [["quote", "@king-square-table-opening"]]
    PAWN_SQUARE_TABLE_ENDING_SYMBOL = [["quote", "@pawn-square-table-ending"]]
    KNIGHT_SQUARE_TABLE_ENDING_SYMBOL = \
    [["quote", "@knight-square-table-ending"]]
    BISHOP_SQUARE_TABLE_ENDING_SYMBOL = \
    [["quote", "@bishop-square-table-ending"]]
    ROOK_SQUARE_TABLE_ENDING_SYMBOL = [["quote", "@rook-square-table-ending"]]
    QUEEN_SQUARE_TABLE_ENDING_SYMBOL = \
    [["quote", "@queen-square-table-ending"]]
    KING_SQUARE_TABLE_ENDING_SYMBOL = [["quote", "@king-square-table-ending"]]
    PAWN_ATTACK_TABLE_SYMBOL = [["quote", "@pawn-attack-table"]]
    KNIGHT_ATTACK_TABLE_SYMBOL = [["quote", "@knight-attack-table"]]
    BISHOP_ATTACK_TABLE_SYMBOL = [["quote", "@bishop-attack-table"]]
    ROOK_ATTACK_TABLE_SYMBOL = [["quote", "@rook-attack-table"]]
    QUEEN_ATTACK_TABLE_SYMBOL = [["quote", "@queen-attack-table"]]
    KING_ATTACK_TABLE_SYMBOL = [["quote", "@king-attack-table"]]
    PAWN_DEFENSE_TABLE_SYMBOL = [["quote", "@pawn-defense-table"]]
    KNIGHT_DEFENSE_TABLE_SYMBOL_SYMBOL = [["quote", "@knight-defense-table"]]
    BISHOP_DEFENSE_TABLE_SYMBOL = [["quote", "@bishop-defense-table"]]
    ROOK_DEFENSE_TABLE_SYMBOL = [["quote", "@rook-defense-table"]]
    QUEEN_DEFENSE_TABLE_SYMBOL = [["quote", "@queen-defense-table"]]
    KING_DEFENSE_TABLE_SYMBOL = [["quote", "@king-defense-table"]]
    BISHOP_PIN_TABLE_SYMBOL = [["quote", "@bishop-pin-table"]]
    ROOK_PIN_TABLE_SYMBOL = [["quote", "@rook-pin-table"]]
    QUEEN_PIN_TABLE_SYMBOL = [["quote", "@queen-pin-table"]]
    PAWN_SHIELD_TABLE_SYMBOL = [["quote", "@pawn-shield-table"]]
    WEIGHT_PAWN_MOBILITY_SYMBOL = [["quote", "@weight-pawn-mobility"]]
    WEIGHT_KNIGHT_MOBILITY_SYMBOL = [["quote", "@weight-knight-mobility"]]
    WEIGHT_BISHOP_MOBILITY_SYMBOL = [["quote", "@weight-bishop-mobility"]]
    WEIGHT_ROOK_MOBILITY_SYMBOL = [["quote", "@weight-rook-mobility"]]
    WEIGHT_QUEEN_MOBILITY_SYMBOL = [["quote", "@weight-queen-mobility"]]
    WEIGHT_KING_MOBILITY_SYMBOL = [["quote", "@weight-king-mobility"]]
    WEIGHT_PAWN_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-pawn-center-control"]]
    WEIGHT_KNIGHT_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-knight-center-control"]]
    WEIGHT_BISHOP_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-bishop-center-control"]]
    WEIGHT_ROOK_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-rook-center-control"]]
    WEIGHT_QUEEN_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-queen-center-control"]]
    WEIGHT_KING_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-king-center-control"]]
    WEIGHT_PAWN_SWEET_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-pawn-sweet-center-control"]]
    WEIGHT_KNIGHT_SWEET_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-knight-sweet-center-control"]]
    WEIGHT_BISHOP_SWEET_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-bishop-sweet-center-control"]]
    WEIGHT_ROOK_SWEET_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-rook-sweet-center-control"]]
    WEIGHT_QUEEN_SWEET_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-queen-sweet-center-control"]]
    WEIGHT_KING_SWEET_CENTER_CONTROL_SYMBOL = \
    [["quote", "@weight-king-sweet-center-control"]]
    WEIGHT_PAWN_DEVELOPMENT_SYMBOL = [["quote", "@weight-pawn-development"]]
    WEIGHT_KNIGHT_DEVELOPMENT_SYMBOL = \
    [["quote", "@weight-knight-development"]]
    WEIGHT_BISHOP_DEVELOPMENT_SYMBOL = \
    [["quote", "@weight-bishop-development"]]
    WEIGHT_ROOK_DEVELOPMENT_SYMBOL = [["quote", "@weight-rook-development"]]
    WEIGHT_QUEEN_DEVELOPMENT_SYMBOL = [["quote", "@weight-queen-development"]]
    WEIGHT_KING_DEVELOPMENT_SYMBOL = [["quote", "@weight-king-development"]]
    WEIGHT_PAWN_ATTACK_SYMBOL = [["quote", "@weight-pawn-attack"]]
    WEIGHT_KNIGHT_ATTACK_SYMBOL = [["quote", "@weight-knight-attack"]]
    WEIGHT_BISHOP_ATTACK_SYMBOL = [["quote", "@weight-bishop-attack"]]
    WEIGHT_ROOK_ATTACK_SYMBOL = [["quote", "@weight-rook-attack"]]
    WEIGHT_QUEEN_ATTACK_SYMBOL = [["quote", "@weight-queen-attack"]]
    WEIGHT_KING_ATTACK_SYMBOL = [["quote", "@weight-king-attack"]]
    WEIGHT_PAWN_DEFENSE_SYMBOL = [["quote", "@weight-pawn-defense"]]
    WEIGHT_KNIGHT_DEFENSE_SYMBOL = [["quote", "@weight-knight-defense"]]
    WEIGHT_BISHOP_DEFENSE_SYMBOL = [["quote", "@weight-bishop-defense"]]
    WEIGHT_ROOK_DEFENSE_SYMBOL = [["quote", "@weight-rook-defense"]]
    WEIGHT_QUEEN_DEFENSE_SYMBOL = [["quote", "@weight-queen-defense"]]
    WEIGHT_KING_DEFENSE_SYMBOL = [["quote", "@weight-king-defense"]]
    WEIGHT_BISHOP_PIN_SYMBOL = [["quote", "@weight-bishop-pin"]]
    WEIGHT_ROOK_PIN_SYMBOL = [["quote", "@weight-rook-pin"]]
    WEIGHT_QUEEN_PIN_SYMBOL = [["quote", "@weight-queen-pin"]]
    WEIGHT_PAWN_ATTACK_AROUND_KING_SYMBOL = \
    [["quote", "@weight-pawn-attack-around-king"]]
    WEIGHT_KNIGHT_ATTACK_AROUND_KING_SYMBOL = \
    [["quote", "@weight-knight-attack-around-king"]]
    WEIGHT_BISHOP_ATTACK_AROUND_KING_SYMBOL = \
    [["quote", "@weight-bishop-attack-around-king"]]
    WEIGHT_ROOK_ATTACK_AROUND_KING_SYMBOL = \
    [["quote", "@weight-rook-attack-around-king"]]
    WEIGHT_QUEEN_ATTACK_AROUND_KING_SYMBOL = \
    [["quote", "@weight-queen-attack-around-king"]]
    WEIGHT_KING_ATTACK_AROUND_KING_SYMBOL = \
    [["quote", "@weight-king-attack-around-king"]]
    WEIGHT_PASS_PAWN_SYMBOL = [["quote", "@weight-pass-pawn"]]
    WEIGHT_PROTECTED_PASS_PAWN_SYMBOL = \
    [["quote", "@weight-protected-pass-pawn"]]
    WEIGHT_DOUBLE_PAWN_SYMBOL = [["quote", "@weight-double-pawn"]]
    WEIGHT_ISO_PAWN_SYMBOL = [["quote", "@weight-iso-pawn"]]
    WEIGHT_PAWN_SHIELD_SYMBOL = [["quote", "@weight-pawn-shield"]]
    WEIGHT_BISHOP_PAIR_SYMBOL = [["quote", "@weight-bishop-pair"]]
    WEIGHT_BAD_BISHOP_SYMBOL = [["quote", "@weight-bad-bishop"]]
    WEIGHT_ROOK_PAIR_SYMBOL = [["quote", "@weight-rook-pair"]]
    WEIGHT_ROOK_SEMIOPEN_FYLE_SYMBOL = \
    [["quote", "@weight-rook-semiopen-fyle"]]
    WEIGHT_ROOK_OPEN_FYLE_SYMBOL = [["quote", "@weight-rook-open-fyle"]]
    WEIGHT_EARLY_QUEEN_STARTING_SYMBOL = \
    [["quote", "@weight-early-queen-starting"]]
    WEIGHT_WEAK_SQUARE_SYMBOL = [["quote", "@weight-weak-square"]]
    WEIGHT_CASTLING_SYMBOL = [["quote", "@weight-castling"]]
    WEIGHT_ABANDONED_CASTLING_SYMBOL = \
    [["quote", "@weight-abandoned-castling"]]

    DEFAULT_MATERIAL = [0.0, 100.0, 400.0, 400.0, 600.0, 1200.0, 1000000.0]
    DEFAULT_ENABLE_QUIESCE_SEARCH = True
    DEFAULT_ENABLE_REPETITION_CHECK = True
    DEFAULT_ENABLE_CHECK_EXTENSION = True
    DEFAULT_YBWC_LIMIT_DEPTH = 4.0
    DEFAULT_YBWC_INVALID_MOVES = 3.0
    DEFAULT_ENABLE_ASPIRATION_WINDOWS = True
    DEFAULT_ASPIRATION_WINDOWS_LIMIT_DEPTH = 5.0
    DEFAULT_ASPIRATION_WINDOWS_DELTA = 15.0
    DEFAULT_ENABLE_SEE = True
    DEFAULT_ENABLE_HISTORY = True
    DEFAULT_ENABLE_KILLER = True
    DEFAULT_ENABLE_HASH_TABLE = True
    DEFAULT_ENABLE_IID = True
    DEFAULT_IID_LIMIT_DEPTH = 5.0
    DEFAULT_IID_SEARCH_DEPTH = 4.0
    DEFAULT_ENABLE_NMR = True
    DEFAULT_NMR_LIMIT_DEPTH = 4.0
    DEFAULT_NMR_SEARCH_REDUCTION = 4.0
    DEFAULT_NMR_REDUCTION = 3.0
    DEFAULT_ENABLE_PROBCUT = False
    DEFAULT_PROBCUT_LIMIT_DEPTH = 4.0
    DEFAULT_PROBCUT_MARGIN = 400.0
    DEFAULT_PROBCUT_SEARCH_REDUCTION = 3.0
    DEFAULT_ENABLE_HISTORY_PRUNING = False
    DEFAULT_HISTORY_PRUNING_LIMIT_DEPTH = 4.0
    DEFAULT_HISTORY_PRUNING_MOVE_THRESHOLD = 0.6
    DEFAULT_HISTORY_PRUNING_INVALID_MOVES = 10.0
    DEFAULT_HISTORY_PRUNING_THRESHOLD = 0.5
    DEFAULT_HISTORY_PRUNING_REDUCTION = 1.0
    DEFAULT_ENABLE_LMR = True
    DEFAULT_LMR_LIMIT_DEPTH = 4.0
    DEFAULT_LMR_MOVE_THRESHOLD = 0.3
    DEFAULT_LMR_INVALID_MOVES = 4.0
    DEFAULT_LMR_SEARCH_REDUCTION = 1.0
    DEFAULT_ENABLE_FUTILITY_PRUNING = True
    DEFAULT_FUTILITY_PRUNING_DEPTH = 3.0
    DEFAULT_FUTILITY_PRUNING_MARGIN = 400.0
    DEFAULT_PAWN_SQUARE_TABLE_OPENING = [\
     0.0,  0.0,  0.0,   0.0,   0.0,  0.0,  0.0,  0.0,\
     0.0,  0.0,  0.0,   0.0,   0.0,  0.0,  0.0,  0.0,\
     5.0, 10.0, 15.0,  20.0,  20.0, 15.0, 10.0,  5.0,\
    10.0, 20.0, 30.0,  40.0,  40.0, 30.0, 20.0, 10.0,\
    15.0, 30.0, 45.0,  60.0,  60.0, 45.0, 30.0, 15.0,\
    20.0, 40.0, 60.0,  80.0,  80.0, 60.0, 40.0, 20.0,\
    25.0, 50.0, 75.0, 100.0, 100.0, 75.0, 50.0, 25.0,\
    30.0, 60.0, 90.0, 120.0, 120.0, 90.0, 60.0, 30.0]
    DEFAULT_KNIGHT_SQUARE_TABLE_OPENING = [\
    -30.0, -20.0, -10.0,   0.0,   0.0, -10.0, -20.0, -30.0,\
    -20.0, -10.0,   0.0,  10.0,  10.0,   0.0, -10.0, -20.0,\
    -10.0,   0.0,  10.0,  20.0,  20.0,  10.0,   0.0, -10.0,\
      0.0,  10.0,  20.0,  30.0,  30.0,  20.0,  10.0,   0.0,\
     10.0,  20.0,  30.0,  40.0,  40.0,  30.0,  20.0,  10.0,\
     20.0,  30.0,  40.0,  50.0,  50.0,  40.0,  30.0,  20.0,\
     10.0,  20.0,  30.0,  40.0,  40.0,  30.0,  20.0,  10.0,\
      0.0,  10.0,  20.0,  30.0,  30.0,  20.0,  10.0,   0.0]
    DEFAULT_BISHOP_SQUARE_TABLE_OPENING = [\
    15.0, 10.0,  5.0,  0.0,  0.0,  5.0, 10.0, 15.0,\
    10.0, 20.0, 15.0, 10.0, 10.0, 15.0, 20.0, 10.0,\
     5.0, 15.0, 20.0, 15.0, 15.0, 20.0, 15.0,  5.0,\
     5.0, 10.0, 15.0, 25.0, 25.0, 15.0, 10.0,  5.0,\
     0.0,  7.5, 12.5, 22.5, 22.5, 12.5,  7.5,  0.0,\
     2.5,  7.5, 17.5, 12.5, 12.5, 17.5,  7.5,  2.5,\
     2.5, 12.5,  7.5,  2.5,  2.5,  7.5, 12.5,  2.5,\
     7.5,  2.5,  2.5,  0.0,  0.0,  2.5,  2.5,  7.5]
    DEFAULT_ROOK_SQUARE_TABLE_OPENING = [\
     0.0,  5.0, 10.0, 15.0, 15.0, 10.0,  5.0,  0.0,\
     0.0,  5.0, 10.0, 15.0, 15.0, 10.0,  5.0,  0.0,\
     0.0,  5.0, 10.0, 15.0, 15.0, 10.0,  5.0,  0.0,\
     0.0,  5.0, 10.0, 15.0, 15.0, 10.0,  5.0,  0.0,\
     0.0,  5.0, 10.0, 15.0, 15.0, 10.0,  5.0,  0.0,\
     0.0,  5.0, 10.0, 15.0, 15.0, 10.0,  5.0,  0.0,\
    20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0,\
    20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0]
    DEFAULT_QUEEN_SQUARE_TABLE_OPENING = [\
    -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0,\
    -5.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -5.0,\
    -5.0,  0.0,  5.0,  5.0,  5.0,  5.0,  0.0, -5.0,\
    -5.0,  0.0,  5.0, 10.0, 10.0,  5.0,  0.0, -5.0,\
    -5.0,  0.0,  5.0, 10.0, 10.0,  5.0,  0.0, -5.0,\
    -5.0,  0.0,  5.0,  5.0,  5.0,  5.0,  0.0, -5.0,\
    -5.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -5.0,\
    -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0]
    DEFAULT_KING_SQUARE_TABLE_OPENING = [\
     30.0,  30.0,  30.0, -30.0, -30.0,  30.0,  30.0,  30.0,\
      0.0,   0.0,   0.0, -30.0, -30.0,   0.0,   0.0,   0.0,\
    -30.0, -30.0, -40.0, -40.0, -40.0, -40.0, -30.0, -30.0,\
    -30.0, -40.0, -40.0, -40.0, -40.0, -40.0, -40.0, -30.0,\
    -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0, -50.0,\
    -60.0, -60.0, -60.0, -60.0, -60.0, -60.0, -60.0, -60.0,\
    -70.0, -70.0, -70.0, -70.0, -70.0, -70.0, -70.0, -70.0,\
    -80.0, -80.0, -80.0, -80.0, -80.0, -80.0, -80.0, -80.0]
    DEFAULT_PAWN_SQUARE_TABLE_ENDING = [\
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,\
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,\
     20.0,  20.0,  20.0,  20.0,  20.0,  20.0,  20.0,  20.0,\
     40.0,  40.0,  40.0,  40.0,  40.0,  40.0,  40.0,  40.0,\
     60.0,  60.0,  60.0,  60.0,  60.0,  60.0,  60.0,  60.0,\
     80.0,  80.0,  80.0,  80.0,  80.0,  80.0,  80.0,  80.0,\
    100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0,\
    120.0, 120.0, 120.0, 120.0, 120.0, 120.0, 120.0, 120.0]
    DEFAULT_KNIGHT_SQUARE_TABLE_ENDING = [\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_BISHOP_SQUARE_TABLE_ENDING = [\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_ROOK_SQUARE_TABLE_ENDING = [\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_QUEEN_SQUARE_TABLE_ENDING = [\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,\
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_KING_SQUARE_TABLE_ENDING = [\
    -30.0, -15.0,  0.0, 15.0, 15.0,  0.0, -15.0, -30.0,\
    -15.0,   0.0, 15.0, 30.0, 30.0, 15.0,   0.0, -15.0,\
      0.0,  15.0, 30.0, 45.0, 45.0, 30.0,  15.0,   0.0,\
     15.0,  30.0, 45.0, 60.0, 60.0, 45.0,  30.0,  15.0,\
     15.0,  30.0, 45.0, 60.0, 60.0, 45.0,  30.0,  15.0,\
      0.0,  15.0, 30.0, 45.0, 45.0, 30.0,  15.0,   0.0,\
    -15.0,   0.0, 15.0, 30.0, 30.0, 15.0,   0.0, -15.0,\
    -30.0, -15.0,  0.0, 15.0, 15.0,  0.0, -15.0, -30.0]
    DEFAULT_PAWN_ATTACK_TABLE = [0.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0]
    DEFAULT_KNIGHT_ATTACK_TABLE = [0.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0]
    DEFAULT_BISHOP_ATTACK_TABLE = [0.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0]
    DEFAULT_ROOK_ATTACK_TABLE = [0.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0]
    DEFAULT_QUEEN_ATTACK_TABLE = [0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0]
    DEFAULT_KING_ATTACK_TABLE = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_PAWN_DEFENSE_TABLE = [0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_KNIGHT_DEFENSE_TABLE = [0.0, 7.5, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_BISHOP_DEFENSE_TABLE = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_ROOK_DEFENSE_TABLE = [0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_QUEEN_DEFENSE_TABLE = [0.0, 2.5, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_KING_DEFENSE_TABLE = [0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    DEFAULT_BISHOP_PIN_TABLE = [\
    [0.0, 0.0, 0.0, 0.0,   0.0,   0.0,   0.0],\
    [0.0, 0.0, 0.0, 0.0,   5.0,  10.0,  10.0],\
    [0.0, 0.0, 0.0, 0.0,  20.0,  30.0,  40.0],\
    [0.0, 0.0, 0.0, 0.0,   0.0,   0.0,   0.0],\
    [0.0, 0.0, 0.0, 0.0, 100.0, 100.0, 100.0],\
    [0.0, 0.0, 0.0, 0.0, 100.0,   0.0, 500.0],\
    [0.0, 0.0, 0.0, 0.0, 100.0, 500.0,   0.0]]
    DEFAULT_ROOK_PIN_TABLE = [\
    [0.0, 0.0, 0.0, 0.0, 0.0,   0.0,   0.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0,   5.0,  10.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0,  10.0,  20.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0,  10.0,  20.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0,   0.0,   0.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0,   0.0, 250.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0, 250.0,   0.0]]
    DEFAULT_QUEEN_PIN_TABLE = [\
    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 10.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0],\
    [0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0]]
    DEFAULT_PAWN_SHIELD_TABLE = [\
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,\
     30.0,  30.0,  30.0,  30.0,  30.0,  30.0,  30.0,  30.0,\
      0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,\
    -30.0, -30.0, -30.0, -30.0, -30.0, -30.0, -30.0, -30.0,\
    -60.0, -60.0, -60.0, -60.0, -60.0, -60.0, -60.0, -60.0,\
    -90.0, -90.0, -90.0, -90.0, -90.0, -90.0, -90.0, -90.0,\
    -60.0, -60.0, -60.0, -60.0, -60.0, -60.0, -60.0, -60.0,\
    -30.0, -30.0, -30.0, -30.0, -30.0, -30.0, -30.0, -30.0]
    DEFAULT_WEIGHT_PAWN_MOBILITY = [0.0, 0.0]
    DEFAULT_WEIGHT_KNIGHT_MOBILITY = [1.0, 2.0]
    DEFAULT_WEIGHT_BISHOP_MOBILITY = [1.5, 3.0]
    DEFAULT_WEIGHT_ROOK_MOBILITY = [1.0, 2.0]
    DEFAULT_WEIGHT_QUEEN_MOBILITY = [1.0, 2.0]
    DEFAULT_WEIGHT_KING_MOBILITY = [0.0, 0.0]
    DEFAULT_WEIGHT_PAWN_CENTER_CONTROL = [5.0, 0.0]
    DEFAULT_WEIGHT_KNIGHT_CENTER_CONTROL = [4.0, 0.0]
    DEFAULT_WEIGHT_BISHOP_CENTER_CONTROL = [2.5, 0.0]
    DEFAULT_WEIGHT_ROOK_CENTER_CONTROL = [2.5, 0.0]
    DEFAULT_WEIGHT_QUEEN_CENTER_CONTROL = [2.5, 0.0]
    DEFAULT_WEIGHT_KING_CENTER_CONTROL = [0.0, 0.0]
    DEFAULT_WEIGHT_PAWN_SWEET_CENTER_CONTROL = [5.0, 0.0]
    DEFAULT_WEIGHT_KNIGHT_SWEET_CENTER_CONTROL = [4.0, 0.0]
    DEFAULT_WEIGHT_BISHOP_SWEET_CENTER_CONTROL = [2.5, 0.0]
    DEFAULT_WEIGHT_ROOK_SWEET_CENTER_CONTROL = [2.5, 0.0]
    DEFAULT_WEIGHT_QUEEN_SWEET_CENTER_CONTROL = [2.5, 0.0]
    DEFAULT_WEIGHT_KING_SWEET_CENTER_CONTROL = [0.0, 0.0]
    DEFAULT_WEIGHT_PAWN_DEVELOPMENT = [0.0, 0.0]
    DEFAULT_WEIGHT_KNIGHT_DEVELOPMENT = [15.0, 0.0]
    DEFAULT_WEIGHT_BISHOP_DEVELOPMENT = [15.0, 0.0]
    DEFAULT_WEIGHT_ROOK_DEVELOPMENT = [0.0, 0.0]
    DEFAULT_WEIGHT_QUEEN_DEVELOPMENT = [0.0, 0.0]
    DEFAULT_WEIGHT_KING_DEVELOPMENT = [0.0, 0.0]
    DEFAULT_WEIGHT_PAWN_ATTACK = [1.0, 0.3]
    DEFAULT_WEIGHT_KNIGHT_ATTACK = [1.0, 0.3]
    DEFAULT_WEIGHT_BISHOP_ATTACK = [1.0, 0.3]
    DEFAULT_WEIGHT_ROOK_ATTACK = [1.0, 0.3]
    DEFAULT_WEIGHT_QUEEN_ATTACK = [1.0, 0.3]
    DEFAULT_WEIGHT_KING_ATTACK = [1.0, 0.0]
    DEFAULT_WEIGHT_PAWN_DEFENSE = [1.0, 0.5]
    DEFAULT_WEIGHT_KNIGHT_DEFENSE = [1.0, 0.5]
    DEFAULT_WEIGHT_BISHOP_DEFENSE = [1.0, 0.5]
    DEFAULT_WEIGHT_ROOK_DEFENSE = [1.0, 0.5]
    DEFAULT_WEIGHT_QUEEN_DEFENSE = [1.0, 0.5]
    DEFAULT_WEIGHT_KING_DEFENSE = [1.0, 3.0]
    DEFAULT_WEIGHT_BISHOP_PIN = [1.0, 1.0]
    DEFAULT_WEIGHT_ROOK_PIN = [1.0, 1.0]
    DEFAULT_WEIGHT_QUEEN_PIN = [1.0, 1.0]
    DEFAULT_WEIGHT_PAWN_ATTACK_AROUND_KING = [3.0, 1.5]
    DEFAULT_WEIGHT_KNIGHT_ATTACK_AROUND_KING = [3.0, 1.5]
    DEFAULT_WEIGHT_BISHOP_ATTACK_AROUND_KING = [3.0, 1.5]
    DEFAULT_WEIGHT_ROOK_ATTACK_AROUND_KING = [3.0, 1.5]
    DEFAULT_WEIGHT_QUEEN_ATTACK_AROUND_KING = [3.0, 1.5]
    DEFAULT_WEIGHT_KING_ATTACK_AROUND_KING = [0.0, 5.0]
    DEFAULT_WEIGHT_PASS_PAWN = [20.0, 30.0]
    DEFAULT_WEIGHT_PROTECTED_PASS_PAWN = [10.0, 10.0]
    DEFAULT_WEIGHT_DOUBLE_PAWN = [-10.0, -20.0]
    DEFAULT_WEIGHT_ISO_PAWN = [-10.0, -5.0]
    DEFAULT_WEIGHT_PAWN_SHIELD = [1.0, 0.0]
    DEFAULT_WEIGHT_BISHOP_PAIR = [60.0, 60.0]
    DEFAULT_WEIGHT_BAD_BISHOP = [-1.5, 0.0]
    DEFAULT_WEIGHT_ROOK_PAIR = [10.0, 20.0]
    DEFAULT_WEIGHT_ROOK_SEMIOPEN_FYLE = [7.5, 7.5]
    DEFAULT_WEIGHT_ROOK_OPEN_FYLE = [7.5, 7.5]
    DEFAULT_WEIGHT_EARLY_QUEEN_STARTING = [-20.0, 0.0]
    DEFAULT_WEIGHT_WEAK_SQUARE = [-5.0, 0.0]
    DEFAULT_WEIGHT_CASTLING = [20.0, 0.0]
    DEFAULT_WEIGHT_ABANDONED_CASTLING = [-110.0, 0.0]

    def SetDefault(self):
        self.engine_name = Editor.DEFAULT_ENGINE_NAME
        self.is_generating = True
        self.is_runnable = True
        self.material = copy.deepcopy(Editor.DEFAULT_MATERIAL)
        self.enable_quiesce_search = Editor.DEFAULT_ENABLE_QUIESCE_SEARCH
        self.enable_repetition_check = Editor.DEFAULT_ENABLE_REPETITION_CHECK
        self.enable_check_extension = Editor.DEFAULT_ENABLE_CHECK_EXTENSION
        self.ybwc_limit_depth = Editor.DEFAULT_YBWC_LIMIT_DEPTH
        self.ybwc_invalid_moves = Editor.DEFAULT_YBWC_INVALID_MOVES
        self.enable_aspiration_windows = \
        Editor.DEFAULT_ENABLE_ASPIRATION_WINDOWS
        self.aspiration_windows_limit_depth = \
        Editor.DEFAULT_ASPIRATION_WINDOWS_LIMIT_DEPTH
        self.aspiration_windows_delta = Editor.DEFAULT_ASPIRATION_WINDOWS_DELTA
        self.enable_see = Editor.DEFAULT_ENABLE_SEE
        self.enable_history = Editor.DEFAULT_ENABLE_HISTORY
        self.enable_killer = Editor.DEFAULT_ENABLE_KILLER
        self.enable_hash_table = Editor.DEFAULT_ENABLE_HASH_TABLE
        self.enable_iid = Editor.DEFAULT_ENABLE_IID
        self.iid_limit_depth = Editor.DEFAULT_IID_LIMIT_DEPTH
        self.iid_search_depth = Editor.DEFAULT_IID_SEARCH_DEPTH
        self.enable_nmr = Editor.DEFAULT_ENABLE_NMR
        self.nmr_limit_depth = Editor.DEFAULT_NMR_LIMIT_DEPTH
        self.nmr_search_reduction = Editor.DEFAULT_NMR_SEARCH_REDUCTION
        self.nmr_reduction = Editor.DEFAULT_NMR_REDUCTION
        self.enable_probcut = Editor.DEFAULT_ENABLE_PROBCUT
        self.probcut_limit_depth = Editor.DEFAULT_PROBCUT_LIMIT_DEPTH
        self.probcut_margin = Editor.DEFAULT_PROBCUT_MARGIN
        self.probcut_search_reduction = Editor.DEFAULT_PROBCUT_SEARCH_REDUCTION
        self.enable_history_pruning = Editor.DEFAULT_ENABLE_HISTORY_PRUNING
        self.history_pruning_limit_depth = \
        Editor.DEFAULT_HISTORY_PRUNING_LIMIT_DEPTH
        self.history_pruning_move_threshold = \
        Editor.DEFAULT_HISTORY_PRUNING_MOVE_THRESHOLD
        self.history_pruning_invalid_moves = \
        Editor.DEFAULT_HISTORY_PRUNING_INVALID_MOVES
        self.history_pruning_threshold = \
        Editor.DEFAULT_HISTORY_PRUNING_THRESHOLD
        self.history_pruning_reduction = \
        Editor.DEFAULT_HISTORY_PRUNING_REDUCTION
        self.enable_lmr = Editor.DEFAULT_ENABLE_LMR
        self.lmr_limit_depth = Editor.DEFAULT_LMR_LIMIT_DEPTH
        self.lmr_move_threshold = Editor.DEFAULT_LMR_MOVE_THRESHOLD
        self.lmr_invalid_moves = Editor.DEFAULT_LMR_INVALID_MOVES
        self.lmr_search_reduction = Editor.DEFAULT_LMR_SEARCH_REDUCTION
        self.enable_futility_pruning = Editor.DEFAULT_ENABLE_FUTILITY_PRUNING
        self.futility_pruning_depth = Editor.DEFAULT_FUTILITY_PRUNING_DEPTH
        self.futility_pruning_margin = Editor.DEFAULT_FUTILITY_PRUNING_MARGIN
        self.pawn_square_table_opening = \
        copy.deepcopy(Editor.DEFAULT_PAWN_SQUARE_TABLE_OPENING)
        self.knight_square_table_opening = \
        copy.deepcopy(Editor.DEFAULT_KNIGHT_SQUARE_TABLE_OPENING)
        self.bishop_square_table_opening = \
        copy.deepcopy(Editor.DEFAULT_BISHOP_SQUARE_TABLE_OPENING)
        self.rook_square_table_opening = \
        copy.deepcopy(Editor.DEFAULT_ROOK_SQUARE_TABLE_OPENING)
        self.queen_square_table_opening = \
        copy.deepcopy(Editor.DEFAULT_QUEEN_SQUARE_TABLE_OPENING)
        self.king_square_table_opening = \
        copy.deepcopy(Editor.DEFAULT_KING_SQUARE_TABLE_OPENING)
        self.pawn_square_table_ending = \
        copy.deepcopy(Editor.DEFAULT_PAWN_SQUARE_TABLE_ENDING)
        self.knight_square_table_ending = \
        copy.deepcopy(Editor.DEFAULT_KNIGHT_SQUARE_TABLE_ENDING)
        self.bishop_square_table_ending = \
        copy.deepcopy(Editor.DEFAULT_BISHOP_SQUARE_TABLE_ENDING)
        self.rook_square_table_ending = \
        copy.deepcopy(Editor.DEFAULT_ROOK_SQUARE_TABLE_ENDING)
        self.queen_square_table_ending = \
        copy.deepcopy(Editor.DEFAULT_QUEEN_SQUARE_TABLE_ENDING)
        self.king_square_table_ending = \
        copy.deepcopy(Editor.DEFAULT_KING_SQUARE_TABLE_ENDING)
        self.pawn_attack_table = \
        copy.deepcopy(Editor.DEFAULT_PAWN_ATTACK_TABLE)
        self.knight_attack_table = \
        copy.deepcopy(Editor.DEFAULT_KNIGHT_ATTACK_TABLE)
        self.bishop_attack_table = \
        copy.deepcopy(Editor.DEFAULT_BISHOP_ATTACK_TABLE)
        self.rook_attack_table = \
        copy.deepcopy(Editor.DEFAULT_ROOK_ATTACK_TABLE)
        self.queen_attack_table = \
        copy.deepcopy(Editor.DEFAULT_QUEEN_ATTACK_TABLE)
        self.king_attack_table = \
        copy.deepcopy(Editor.DEFAULT_KING_ATTACK_TABLE)
        self.pawn_defense_table = \
        copy.deepcopy(Editor.DEFAULT_PAWN_DEFENSE_TABLE)
        self.knight_defense_table = \
        copy.deepcopy(Editor.DEFAULT_KNIGHT_DEFENSE_TABLE)
        self.bishop_defense_table = \
        copy.deepcopy(Editor.DEFAULT_BISHOP_DEFENSE_TABLE)
        self.rook_defense_table = \
        copy.deepcopy(Editor.DEFAULT_ROOK_DEFENSE_TABLE)
        self.queen_defense_table = \
        copy.deepcopy(Editor.DEFAULT_QUEEN_DEFENSE_TABLE)
        self.king_defense_table = \
        copy.deepcopy(Editor.DEFAULT_KING_DEFENSE_TABLE)
        self.bishop_pin_table = copy.deepcopy(Editor.DEFAULT_BISHOP_PIN_TABLE)
        self.rook_pin_table = copy.deepcopy(Editor.DEFAULT_ROOK_PIN_TABLE)
        self.queen_pin_table = copy.deepcopy(Editor.DEFAULT_QUEEN_PIN_TABLE)
        self.pawn_shield_table = \
        copy.deepcopy(Editor.DEFAULT_PAWN_SHIELD_TABLE)
        self.weight_pawn_mobility = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_MOBILITY)
        self.weight_knight_mobility = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KNIGHT_MOBILITY)
        self.weight_bishop_mobility = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_MOBILITY)
        self.weight_rook_mobility = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_MOBILITY)
        self.weight_queen_mobility = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_MOBILITY)
        self.weight_king_mobility = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KING_MOBILITY)
        self.weight_pawn_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_CENTER_CONTROL)
        self.weight_knight_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KNIGHT_CENTER_CONTROL)
        self.weight_bishop_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_CENTER_CONTROL)
        self.weight_rook_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_CENTER_CONTROL)
        self.weight_queen_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_CENTER_CONTROL)
        self.weight_king_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KING_CENTER_CONTROL)
        self.weight_pawn_sweet_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_SWEET_CENTER_CONTROL)
        self.weight_knight_sweet_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KNIGHT_SWEET_CENTER_CONTROL)
        self.weight_bishop_sweet_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_SWEET_CENTER_CONTROL)
        self.weight_rook_sweet_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_SWEET_CENTER_CONTROL)
        self.weight_queen_sweet_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_SWEET_CENTER_CONTROL)
        self.weight_king_sweet_center_control = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KING_SWEET_CENTER_CONTROL)
        self.weight_pawn_development = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_DEVELOPMENT)
        self.weight_knight_development = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KNIGHT_DEVELOPMENT)
        self.weight_bishop_development = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_DEVELOPMENT)
        self.weight_rook_development = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_DEVELOPMENT)
        self.weight_queen_development = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_DEVELOPMENT)
        self.weight_king_development = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KING_DEVELOPMENT)
        self.weight_pawn_attack = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_ATTACK)
        self.weight_knight_attack = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KNIGHT_ATTACK)
        self.weight_bishop_attack = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_ATTACK)
        self.weight_rook_attack = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_ATTACK)
        self.weight_queen_attack = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_ATTACK)
        self.weight_king_attack = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KING_ATTACK)
        self.weight_pawn_defense = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_DEFENSE)
        self.weight_knight_defense = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KNIGHT_DEFENSE)
        self.weight_bishop_defense = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_DEFENSE)
        self.weight_rook_defense = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_DEFENSE)
        self.weight_queen_defense = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_DEFENSE)
        self.weight_king_defense = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KING_DEFENSE)
        self.weight_bishop_pin = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_PIN)
        self.weight_rook_pin = copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_PIN)
        self.weight_queen_pin = copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_PIN)
        self.weight_pawn_attack_around_king = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_ATTACK_AROUND_KING)
        self.weight_knight_attack_around_king = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KNIGHT_ATTACK_AROUND_KING)
        self.weight_bishop_attack_around_king = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_ATTACK_AROUND_KING)
        self.weight_rook_attack_around_king = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_ATTACK_AROUND_KING)
        self.weight_queen_attack_around_king = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_QUEEN_ATTACK_AROUND_KING)
        self.weight_king_attack_around_king = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_KING_ATTACK_AROUND_KING)
        self.weight_pass_pawn = copy.deepcopy(Editor.DEFAULT_WEIGHT_PASS_PAWN)
        self.weight_protected_pass_pawn = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PROTECTED_PASS_PAWN)
        self.weight_double_pawn = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_DOUBLE_PAWN)
        self.weight_iso_pawn = copy.deepcopy(Editor.DEFAULT_WEIGHT_ISO_PAWN)
        self.weight_pawn_shield = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_PAWN_SHIELD)
        self.weight_bishop_pair = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BISHOP_PAIR)
        self.weight_bad_bishop = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_BAD_BISHOP)
        self.weight_rook_pair = copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_PAIR)
        self.weight_rook_semiopen_fyle = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_SEMIOPEN_FYLE)
        self.weight_rook_open_fyle = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ROOK_OPEN_FYLE)
        self.weight_early_queen_starting = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_EARLY_QUEEN_STARTING)
        self.weight_weak_square = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_WEAK_SQUARE)
        self.weight_castling = copy.deepcopy(Editor.DEFAULT_WEIGHT_CASTLING)
        self.weight_abandoned_castling = \
        copy.deepcopy(Editor.DEFAULT_WEIGHT_ABANDONED_CASTLING)

    def GenSayulispCode(self):
        ret = ""
        if self.is_generating:
            ret += ";; Generates Engine.\n"
            ret += "(define " + self.engine_name + " (gen-engine))\n\n"

        header = [self.engine_name]
        ret += ";; Configures Engine.\n"
        ret += ListToSExpr(header + Editor.MATERIAL_SYMBOL \
        + [["quote"] + [self.material]]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_QUIESCE_SEARCH_SYMBOL \
        + [self.enable_quiesce_search]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_REPETITION_CHECK_SYMBOL \
        + [self.enable_repetition_check]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_CHECK_EXTENSION_SYMBOL \
        + [self.enable_check_extension]) + "\n"
        ret += ListToSExpr(header + Editor.YBWC_LIMIT_DEPTH_SYMBOL \
        + [self.ybwc_limit_depth]) + "\n"
        ret += ListToSExpr(header + Editor.YBWC_INVALID_MOVES_SYMBOL \
        + [self.ybwc_invalid_moves]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_ASPIRATION_WINDOWS_SYMBOL \
        + [self.enable_aspiration_windows]) + "\n"
        ret += ListToSExpr(header \
        + Editor.ASPIRATION_WINDOWS_LIMIT_DEPTH_SYMBOL \
        + [self.aspiration_windows_limit_depth]) + "\n"
        ret += ListToSExpr(header + Editor.ASPIRATION_WINDOWS_DELTA_SYMBOL \
        + [self.aspiration_windows_delta]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_SEE_SYMBOL \
        + [self.enable_see]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_HISTORY_SYMBOL \
        + [self.enable_history]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_KILLER_SYMBOL \
        + [self.enable_killer]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_HASH_TABLE_SYMBOL \
        + [self.enable_hash_table]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_IID_SYMBOL \
        + [self.enable_iid]) + "\n"
        ret += ListToSExpr(header + Editor.IID_LIMIT_DEPTH_SYMBOL \
        + [self.iid_limit_depth]) + "\n"
        ret += ListToSExpr(header + Editor.IID_SEARCH_DEPTH_SYMBOL \
        + [self.iid_search_depth]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_NMR_SYMBOL \
        + [self.enable_nmr]) + "\n"
        ret += ListToSExpr(header + Editor.NMR_LIMIT_DEPTH_SYMBOL \
        + [self.nmr_limit_depth]) + "\n"
        ret += ListToSExpr(header + Editor.NMR_SEARCH_REDUCTION_SYMBOL \
        + [self.nmr_search_reduction]) + "\n"
        ret += ListToSExpr(header + Editor.NMR_REDUCTION_SYMBOL \
        + [self.nmr_reduction]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_PROBCUT_SYMBOL \
        + [self.enable_probcut]) + "\n"
        ret += ListToSExpr(header + Editor.PROBCUT_LIMIT_DEPTH_SYMBOL \
        + [self.probcut_limit_depth]) + "\n"
        ret += ListToSExpr(header + Editor.PROBCUT_MARGIN_SYMBOL \
        + [self.probcut_margin]) + "\n"
        ret += ListToSExpr(header + Editor.PROBCUT_SEARCH_REDUCTION_SYMBOL \
        + [self.probcut_search_reduction]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_HISTORY_PRUNING_SYMBOL \
        + [self.enable_history_pruning]) + "\n"
        ret += ListToSExpr(header + Editor.HISTORY_PRUNING_LIMIT_DEPTH_SYMBOL \
        + [self.history_pruning_limit_depth]) + "\n"
        ret += ListToSExpr(header \
        + Editor.HISTORY_PRUNING_MOVE_THRESHOLD_SYMBOL \
        + [self.history_pruning_move_threshold]) + "\n"
        ret += ListToSExpr(header \
        + Editor.HISTORY_PRUNING_INVALID_MOVES_SYMBOL \
        + [self.history_pruning_invalid_moves]) + "\n"
        ret += ListToSExpr(header + Editor.HISTORY_PRUNING_THRESHOLD_SYMBOL \
        + [self.history_pruning_threshold]) + "\n"
        ret += ListToSExpr(header + Editor.HISTORY_PRUNING_REDUCTION_SYMBOL \
        + [self.history_pruning_reduction]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_LMR_SYMBOL \
        + [self.enable_lmr]) + "\n"
        ret += ListToSExpr(header + Editor.LMR_LIMIT_DEPTH_SYMBOL \
        + [self.lmr_limit_depth]) + "\n"
        ret += ListToSExpr(header + Editor.LMR_MOVE_THRESHOLD_SYMBOL \
        + [self.lmr_move_threshold]) + "\n"
        ret += ListToSExpr(header + Editor.LMR_INVALID_MOVES_SYMBOL \
        + [self.lmr_invalid_moves]) + "\n"
        ret += ListToSExpr(header + Editor.LMR_SEARCH_REDUCTION_SYMBOL \
        + [self.lmr_search_reduction]) + "\n"
        ret += ListToSExpr(header + Editor.ENABLE_FUTILITY_PRUNING_SYMBOL \
        + [self.enable_futility_pruning]) + "\n"
        ret += ListToSExpr(header + Editor.FUTILITY_PRUNING_DEPTH_SYMBOL \
        + [self.futility_pruning_depth]) + "\n"
        ret += ListToSExpr(header + Editor.FUTILITY_PRUNING_MARGIN_SYMBOL \
        + [self.futility_pruning_margin]) + "\n"
        ret += ListToSExpr(header + Editor.PAWN_SQUARE_TABLE_OPENING_SYMBOL \
        + [["quote", self.pawn_square_table_opening]]) + "\n"
        ret += ListToSExpr(header + Editor.KNIGHT_SQUARE_TABLE_OPENING_SYMBOL \
        + [["quote", self.knight_square_table_opening]]) + "\n"
        ret += ListToSExpr(header + Editor.BISHOP_SQUARE_TABLE_OPENING_SYMBOL \
        + [["quote", self.bishop_square_table_opening]]) + "\n"
        ret += ListToSExpr(header + Editor.ROOK_SQUARE_TABLE_OPENING_SYMBOL \
        + [["quote", self.rook_square_table_opening]]) + "\n"
        ret += ListToSExpr(header + Editor.QUEEN_SQUARE_TABLE_OPENING_SYMBOL \
        + [["quote", self.queen_square_table_opening]]) + "\n"
        ret += ListToSExpr(header + Editor.KING_SQUARE_TABLE_OPENING_SYMBOL \
        + [["quote", self.king_square_table_opening]]) + "\n"
        ret += ListToSExpr(header + Editor.PAWN_SQUARE_TABLE_ENDING_SYMBOL \
        + [["quote", self.pawn_square_table_ending]]) + "\n"
        ret += ListToSExpr(header + Editor.KNIGHT_SQUARE_TABLE_ENDING_SYMBOL \
        + [["quote", self.knight_square_table_ending]]) + "\n"
        ret += ListToSExpr(header + Editor.BISHOP_SQUARE_TABLE_ENDING_SYMBOL \
        + [["quote", self.bishop_square_table_ending]]) + "\n"
        ret += ListToSExpr(header + Editor.ROOK_SQUARE_TABLE_ENDING_SYMBOL \
        + [["quote", self.rook_square_table_ending]]) + "\n"
        ret += ListToSExpr(header + Editor.QUEEN_SQUARE_TABLE_ENDING_SYMBOL \
        + [["quote", self.queen_square_table_ending]]) + "\n"
        ret += ListToSExpr(header + Editor.KING_SQUARE_TABLE_ENDING_SYMBOL \
        + [["quote", self.king_square_table_ending]]) + "\n"
        ret += ListToSExpr(header + Editor.PAWN_ATTACK_TABLE_SYMBOL \
        + [["quote", self.pawn_attack_table]]) + "\n"
        ret += ListToSExpr(header + Editor.KNIGHT_ATTACK_TABLE_SYMBOL \
        + [["quote", self.knight_attack_table]]) + "\n"
        ret += ListToSExpr(header + Editor.BISHOP_ATTACK_TABLE_SYMBOL \
        + [["quote", self.bishop_attack_table]]) + "\n"
        ret += ListToSExpr(header + Editor.ROOK_ATTACK_TABLE_SYMBOL \
        + [["quote", self.rook_attack_table]]) + "\n"
        ret += ListToSExpr(header + Editor.QUEEN_ATTACK_TABLE_SYMBOL \
        + [["quote", self.queen_attack_table]]) + "\n"
        ret += ListToSExpr(header + Editor.KING_ATTACK_TABLE_SYMBOL \
        + [["quote", self.king_attack_table]]) + "\n"
        ret += ListToSExpr(header + Editor.PAWN_DEFENSE_TABLE_SYMBOL \
        + [["quote", self.pawn_defense_table]]) + "\n"
        ret += ListToSExpr(header + Editor.KNIGHT_DEFENSE_TABLE_SYMBOL_SYMBOL \
        + [["quote", self.knight_defense_table]]) + "\n"
        ret += ListToSExpr(header + Editor.BISHOP_DEFENSE_TABLE_SYMBOL \
        + [["quote", self.bishop_defense_table]]) + "\n"
        ret += ListToSExpr(header + Editor.ROOK_DEFENSE_TABLE_SYMBOL \
        + [["quote", self.rook_defense_table]]) + "\n"
        ret += ListToSExpr(header + Editor.QUEEN_DEFENSE_TABLE_SYMBOL \
        + [["quote", self.queen_defense_table]]) + "\n"
        ret += ListToSExpr(header + Editor.KING_DEFENSE_TABLE_SYMBOL \
        + [["quote", self.king_defense_table]]) + "\n"
        ret += ListToSExpr(header + Editor.BISHOP_PIN_TABLE_SYMBOL \
        + [["quote", self.bishop_pin_table]]) + "\n"
        ret += ListToSExpr(header + Editor.ROOK_PIN_TABLE_SYMBOL \
        + [["quote", self.rook_pin_table]]) + "\n"
        ret += ListToSExpr(header + Editor.QUEEN_PIN_TABLE_SYMBOL \
        + [["quote", self.queen_pin_table]]) + "\n"
        ret += ListToSExpr(header + Editor.PAWN_SHIELD_TABLE_SYMBOL \
        + [["quote", self.pawn_shield_table]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PAWN_MOBILITY_SYMBOL \
        + [["quote", self.weight_pawn_mobility]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KNIGHT_MOBILITY_SYMBOL \
        + [["quote", self.weight_knight_mobility]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_BISHOP_MOBILITY_SYMBOL \
        + [["quote", self.weight_bishop_mobility]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_MOBILITY_SYMBOL \
        + [["quote", self.weight_rook_mobility]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_QUEEN_MOBILITY_SYMBOL \
        + [["quote", self.weight_queen_mobility]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KING_MOBILITY_SYMBOL \
        + [["quote", self.weight_king_mobility]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PAWN_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_pawn_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_KNIGHT_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_knight_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_BISHOP_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_bishop_center_control]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_rook_center_control]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_QUEEN_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_queen_center_control]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KING_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_king_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_PAWN_SWEET_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_pawn_sweet_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_KNIGHT_SWEET_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_knight_sweet_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_BISHOP_SWEET_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_bishop_sweet_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_ROOK_SWEET_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_rook_sweet_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_QUEEN_SWEET_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_queen_sweet_center_control]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_KING_SWEET_CENTER_CONTROL_SYMBOL \
        + [["quote", self.weight_king_sweet_center_control]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PAWN_DEVELOPMENT_SYMBOL \
        + [["quote", self.weight_pawn_development]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KNIGHT_DEVELOPMENT_SYMBOL \
        + [["quote", self.weight_knight_development]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_BISHOP_DEVELOPMENT_SYMBOL \
        + [["quote", self.weight_bishop_development]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_DEVELOPMENT_SYMBOL \
        + [["quote", self.weight_rook_development]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_QUEEN_DEVELOPMENT_SYMBOL \
        + [["quote", self.weight_queen_development]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KING_DEVELOPMENT_SYMBOL \
        + [["quote", self.weight_king_development]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PAWN_ATTACK_SYMBOL \
        + [["quote", self.weight_pawn_attack]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KNIGHT_ATTACK_SYMBOL \
        + [["quote", self.weight_knight_attack]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_BISHOP_ATTACK_SYMBOL \
        + [["quote", self.weight_bishop_attack]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_ATTACK_SYMBOL \
        + [["quote", self.weight_rook_attack]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_QUEEN_ATTACK_SYMBOL \
        + [["quote", self.weight_queen_attack]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KING_ATTACK_SYMBOL \
        + [["quote", self.weight_king_attack]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PAWN_DEFENSE_SYMBOL \
        + [["quote", self.weight_pawn_defense]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KNIGHT_DEFENSE_SYMBOL \
        + [["quote", self.weight_knight_defense]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_BISHOP_DEFENSE_SYMBOL \
        + [["quote", self.weight_bishop_defense]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_DEFENSE_SYMBOL \
        + [["quote", self.weight_rook_defense]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_QUEEN_DEFENSE_SYMBOL \
        + [["quote", self.weight_queen_defense]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_KING_DEFENSE_SYMBOL \
        + [["quote", self.weight_king_defense]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_BISHOP_PIN_SYMBOL \
        + [["quote", self.weight_bishop_pin]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_PIN_SYMBOL \
        + [["quote", self.weight_rook_pin]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_QUEEN_PIN_SYMBOL \
        + [["quote", self.weight_queen_pin]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_PAWN_ATTACK_AROUND_KING_SYMBOL \
        + [["quote", self.weight_pawn_attack_around_king]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_KNIGHT_ATTACK_AROUND_KING_SYMBOL \
        + [["quote", self.weight_knight_attack_around_king]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_BISHOP_ATTACK_AROUND_KING_SYMBOL \
        + [["quote", self.weight_bishop_attack_around_king]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_ROOK_ATTACK_AROUND_KING_SYMBOL \
        + [["quote", self.weight_rook_attack_around_king]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_QUEEN_ATTACK_AROUND_KING_SYMBOL \
        + [["quote", self.weight_queen_attack_around_king]]) + "\n"
        ret += ListToSExpr(header \
        + Editor.WEIGHT_KING_ATTACK_AROUND_KING_SYMBOL \
        + [["quote", self.weight_king_attack_around_king]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PASS_PAWN_SYMBOL \
        + [["quote", self.weight_pass_pawn]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PROTECTED_PASS_PAWN_SYMBOL \
        + [["quote", self.weight_protected_pass_pawn]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_DOUBLE_PAWN_SYMBOL \
        + [["quote", self.weight_double_pawn]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ISO_PAWN_SYMBOL \
        + [["quote", self.weight_iso_pawn]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_PAWN_SHIELD_SYMBOL \
        + [["quote", self.weight_pawn_shield]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_BISHOP_PAIR_SYMBOL \
        + [["quote", self.weight_bishop_pair]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_BAD_BISHOP_SYMBOL \
        + [["quote", self.weight_bad_bishop]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_PAIR_SYMBOL \
        + [["quote", self.weight_rook_pair]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_SEMIOPEN_FYLE_SYMBOL \
        + [["quote", self.weight_rook_semiopen_fyle]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ROOK_OPEN_FYLE_SYMBOL \
        + [["quote", self.weight_rook_open_fyle]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_EARLY_QUEEN_STARTING_SYMBOL \
        + [["quote", self.weight_early_queen_starting]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_WEAK_SQUARE_SYMBOL \
        + [["quote", self.weight_weak_square]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_CASTLING_SYMBOL \
        + [["quote", self.weight_castling]]) + "\n"
        ret += ListToSExpr(header + Editor.WEIGHT_ABANDONED_CASTLING_SYMBOL \
        + [["quote", self.weight_abandoned_castling]]) + "\n"

        if self.is_runnable:
            ret += "\n;; Runs Engine.\n"
            ret += "(" + self.engine_name + " (quote @run))"

        return ret

    def SetFromCode(self, code):
        li = SExprToList(code)
        self.is_runnable = False
        self.is_generating =False
        for item in li:
            if item[0] == "define":
                if item[2][0] == "gen-engine": self.is_generating = True
            else: self.engine_name = item[0]
            if not isinstance(li, list): continue
            if len(item) < 3: continue
            if item[0] == self.engine_name:
                if item[1][1] == "@material":
                    self.material = item[2][1]
                elif item[1][1] == "@enable-quiesce-search":
                    self.enable_quiesce_search = item[2]
                elif item[1][1] == "@enable-repetition-check":
                    self.enable_repetition_check = item[2]
                elif item[1][1] == "@enable-check-extension":
                    self.enable_check_extension = item[2]
                elif item[1][1] == "@ybwc-limit-depth":
                    self.ybwc_limit_depth = item[2]
                elif item[1][1] == "@ybwc-invalid-moves":
                    self.ybwc_invalid_moves = item[2]
                elif item[1][1] == "@enable-aspiration-windows":
                    self.enable_aspiration_windows = item[2]
                elif item[1][1] == "@aspiration-windows-limit-depth":
                    self.aspiration_windows_limit_depth = item[2]
                elif item[1][1] == "@aspiration-windows-delta":
                    self.aspiration_windows_delta = item[2]
                elif item[1][1] == "@enable-see":
                    self.enable_see = item[2]
                elif item[1][1] == "@enable-history":
                    self.enable_history = item[2]
                elif item[1][1] == "@enable-killer":
                    self.enable_killer = item[2]
                elif item[1][1] == "@enable-hash-table":
                    self.enable_hash_table = item[2]
                elif item[1][1] == "@enable-iid":
                    self.enable_iid = item[2]
                elif item[1][1] == "@iid-limit-depth":
                    self.iid_limit_depth = item[2]
                elif item[1][1] == "@iid-search-depth":
                    self.iid_search_depth = item[2]
                elif item[1][1] == "@enable-nmr":
                    self.enable_nmr = item[2]
                elif item[1][1] == "@nmr-limit-depth":
                    self.nmr_limit_depth = item[2]
                elif item[1][1] == "@nmr-search-reduction":
                    self.nmr_search_reduction = item[2]
                elif item[1][1] == "@nmr-reduction":
                    self.nmr_reduction = item[2]
                elif item[1][1] == "@enable-probcut":
                    self.enable_probcut = item[2]
                elif item[1][1] == "@probcut-limit-depth":
                    self.probcut_limit_depth = item[2]
                elif item[1][1] == "@probcut-margin":
                    self.probcut_margin = item[2]
                elif item[1][1] == "@probcut-search-reduction":
                    self.probcut_search_reduction = item[2]
                elif item[1][1] == "@enable-history-pruning":
                    self.enable_history_pruning = item[2]
                elif item[1][1] == "@history-pruning-limit-depth":
                    self.history_pruning_limit_depth = item[2]
                elif item[1][1] == "@history-pruning-move-threshold":
                    self.history_pruning_move_threshold = item[2]
                elif item[1][1] == "@history-pruning-invalid-moves":
                    self.history_pruning_invalid_moves = item[2]
                elif item[1][1] == "@history-pruning-threshold":
                    self.history_pruning_threshold = item[2]
                elif item[1][1] == "@history-pruning-reduction":
                    self.history_pruning_reduction = item[2]
                elif item[1][1] == "@enable-lmr":
                    self.enable_lmr = item[2]
                elif item[1][1] == "@lmr-limit-depth":
                    self.lmr_limit_depth = item[2]
                elif item[1][1] == "@lmr-move-threshold":
                    self.lmr_move_threshold = item[2]
                elif item[1][1] == "@lmr-invalid-moves":
                    self.lmr_invalid_moves = item[2]
                elif item[1][1] == "@lmr-search-reduction":
                    self.lmr_search_reduction = item[2]
                elif item[1][1] == "@enable-futility-pruning":
                    self.enable_futility_pruning = item[2]
                elif item[1][1] == "@futility-pruning-depth":
                    self.futility_pruning_depth = item[2]
                elif item[1][1] == "@futility-pruning-margin":
                    self.futility_pruning_margin = item[2]
                elif item[1][1] == "@pawn-square-table-opening":
                    self.pawn_square_table_opening = item[2][1]
                elif item[1][1] == "@knight-square-table-opening":
                    self.knight_square_table_opening = item[2][1]
                elif item[1][1] == "@bishop-square-table-opening":
                    self.bishop_square_table_opening = item[2][1]
                elif item[1][1] == "@rook-square-table-opening":
                    self.rook_square_table_opening = item[2][1]
                elif item[1][1] == "@queen-square-table-opening":
                    self.queen_square_table_opening = item[2][1]
                elif item[1][1] == "@king-square-table-opening":
                    self.king_square_table_opening = item[2][1]
                elif item[1][1] == "@pawn-square-table-ending":
                    self.pawn_square_table_ending = item[2][1]
                elif item[1][1] == "@knight-square-table-ending":
                    self.knight_square_table_ending = item[2][1]
                elif item[1][1] == "@bishop-square-table-ending":
                    self.bishop_square_table_ending = item[2][1]
                elif item[1][1] == "@rook-square-table-ending":
                    self.rook_square_table_ending = item[2][1]
                elif item[1][1] == "@queen-square-table-ending":
                    self.queen_square_table_ending = item[2][1]
                elif item[1][1] == "@king-square-table-ending":
                    self.king_square_table_ending = item[2][1]
                elif item[1][1] == "@pawn-attack-table":
                    self.pawn_attack_table = item[2][1]
                elif item[1][1] == "@knight-attack-table":
                    self.knight_attack_table = item[2][1]
                elif item[1][1] == "@bishop-attack-table":
                    self.bishop_attack_table = item[2][1]
                elif item[1][1] == "@rook-attack-table":
                    self.rook_attack_table = item[2][1]
                elif item[1][1] == "@queen-attack-table":
                    self.queen_attack_table = item[2][1]
                elif item[1][1] == "@king-attack-table":
                    self.king_attack_table = item[2][1]
                elif item[1][1] == "@pawn-defense-table":
                    self.pawn_defense_table = item[2][1]
                elif item[1][1] == "@knight-defense-table":
                    self.knight_defense_table = item[2][1]
                elif item[1][1] == "@bishop-defense-table":
                    self.bishop_defense_table = item[2][1]
                elif item[1][1] == "@rook-defense-table":
                    self.rook_defense_table = item[2][1]
                elif item[1][1] == "@queen-defense-table":
                    self.queen_defense_table = item[2][1]
                elif item[1][1] == "@king-defense-table":
                    self.king_defense_table = item[2][1]
                elif item[1][1] == "@bishop-pin-table":
                    self.bishop_pin_table = item[2][1]
                elif item[1][1] == "@rook-pin-table":
                    self.rook_pin_table = item[2][1]
                elif item[1][1] == "@queen-pin-table":
                    self.queen_pin_table = item[2][1]
                elif item[1][1] == "@pawn-shield-table":
                    self.pawn_shield_table = item[2][1]
                elif item[1][1] == "@weight-pawn-mobility":
                    self.weight_pawn_mobility = item[2][1]
                elif item[1][1] == "@weight-knight-mobility":
                    self.weight_knight_mobility = item[2][1]
                elif item[1][1] == "@weight-bishop-mobility":
                    self.weight_bishop_mobility = item[2][1]
                elif item[1][1] == "@weight-rook-mobility":
                    self.weight_rook_mobility = item[2][1]
                elif item[1][1] == "@weight-queen-mobility":
                    self.weight_queen_mobility = item[2][1]
                elif item[1][1] == "@weight-king-mobility":
                    self.weight_king_mobility = item[2][1]
                elif item[1][1] == "@weight-pawn-center-control":
                    self.weight_pawn_center_control = item[2][1]
                elif item[1][1] == "@weight-knight-center-control":
                    self.weight_knight_center_control = item[2][1]
                elif item[1][1] == "@weight-bishop-center-control":
                    self.weight_bishop_center_control = item[2][1]
                elif item[1][1] == "@weight-rook-center-control":
                    self.weight_rook_center_control = item[2][1]
                elif item[1][1] == "@weight-queen-center-control":
                    self.weight_queen_center_control = item[2][1]
                elif item[1][1] == "@weight-king-center-control":
                    self.weight_king_center_control = item[2][1]
                elif item[1][1] == "@weight-pawn-sweet-center-control":
                    self.weight_pawn_sweet_center_control = item[2][1]
                elif item[1][1] == "@weight-knight-sweet-center-control":
                    self.weight_knight_sweet_center_control = item[2][1]
                elif item[1][1] == "@weight-bishop-sweet-center-control":
                    self.weight_bishop_sweet_center_control = item[2][1]
                elif item[1][1] == "@weight-rook-sweet-center-control":
                    self.weight_rook_sweet_center_control = item[2][1]
                elif item[1][1] == "@weight-queen-sweet-center-control":
                    self.weight_queen_sweet_center_control = item[2][1]
                elif item[1][1] == "@weight-king-sweet-center-control":
                    self.weight_king_sweet_center_control = item[2][1]
                elif item[1][1] == "@weight-pawn-development":
                    self.weight_pawn_development = item[2][1]
                elif item[1][1] == "@weight-knight-development":
                    self.weight_knight_development = item[2][1]
                elif item[1][1] == "@weight-bishop-development":
                    self.weight_bishop_development = item[2][1]
                elif item[1][1] == "@weight-rook-development":
                    self.weight_rook_development = item[2][1]
                elif item[1][1] == "@weight-queen-development":
                    self.weight_queen_development = item[2][1]
                elif item[1][1] == "@weight-king-development":
                    self.weight_king_development = item[2][1]
                elif item[1][1] == "@weight-pawn-attack":
                    self.weight_pawn_attack = item[2][1]
                elif item[1][1] == "@weight-knight-attack":
                    self.weight_knight_attack = item[2][1]
                elif item[1][1] == "@weight-bishop-attack":
                    self.weight_bishop_attack = item[2][1]
                elif item[1][1] == "@weight-rook-attack":
                    self.weight_rook_attack = item[2][1]
                elif item[1][1] == "@weight-queen-attack":
                    self.weight_queen_attack = item[2][1]
                elif item[1][1] == "@weight-king-attack":
                    self.weight_king_attack = item[2][1]
                elif item[1][1] == "@weight-pawn-defense":
                    self.weight_pawn_defense = item[2][1]
                elif item[1][1] == "@weight-knight-defense":
                    self.weight_knight_defense = item[2][1]
                elif item[1][1] == "@weight-bishop-defense":
                    self.weight_bishop_defense = item[2][1]
                elif item[1][1] == "@weight-rook-defense":
                    self.weight_rook_defense = item[2][1]
                elif item[1][1] == "@weight-queen-defense":
                    self.weight_queen_defense = item[2][1]
                elif item[1][1] == "@weight-king-defense":
                    self.weight_king_defense = item[2][1]
                elif item[1][1] == "@weight-bishop-pin":
                    self.weight_bishop_pin = item[2][1]
                elif item[1][1] == "@weight-rook-pin":
                    self.weight_rook_pin = item[2][1]
                elif item[1][1] == "@weight-queen-pin":
                    self.weight_queen_pin = item[2][1]
                elif item[1][1] == "@weight-pawn-attack-around-king":
                    self.weight_pawn_attack_around_king = item[2][1]
                elif item[1][1] == "@weight-knight-attack-around-king":
                    self.weight_knight_attack_around_king = item[2][1]
                elif item[1][1] == "@weight-bishop-attack-around-king":
                    self.weight_bishop_attack_around_king = item[2][1]
                elif item[1][1] == "@weight-rook-attack-around-king":
                    self.weight_rook_attack_around_king = item[2][1]
                elif item[1][1] == "@weight-queen-attack-around-king":
                    self.weight_queen_attack_around_king = item[2][1]
                elif item[1][1] == "@weight-king-attack-around-king":
                    self.weight_king_attack_around_king = item[2][1]
                elif item[1][1] == "@weight-pass-pawn":
                    self.weight_pass_pawn = item[2][1]
                elif item[1][1] == "@weight-protected-pass-pawn":
                    self.weight_protected_pass_pawn = item[2][1]
                elif item[1][1] == "@weight-double-pawn":
                    self.weight_double_pawn = item[2][1]
                elif item[1][1] == "@weight-iso-pawn":
                    self.weight_iso_pawn = item[2][1]
                elif item[1][1] == "@weight-pawn-shield":
                    self.weight_pawn_shield = item[2][1]
                elif item[1][1] == "@weight-bishop-pair":
                    self.weight_bishop_pair = item[2][1]
                elif item[1][1] == "@weight-bad-bishop":
                    self.weight_bad_bishop = item[2][1]
                elif item[1][1] == "@weight-rook-pair":
                    self.weight_rook_pair = item[2][1]
                elif item[1][1] == "@weight-rook-semiopen-fyle":
                    self.weight_rook_semiopen_fyle = item[2][1]
                elif item[1][1] == "@weight-rook-open-fyle":
                    self.weight_rook_open_fyle = item[2][1]
                elif item[1][1] == "@weight-early-queen-starting":
                    self.weight_early_queen_starting = item[2][1]
                elif item[1][1] == "@weight-weak-square":
                    self.weight_weak_square = item[2][1]
                elif item[1][1] == "@weight-castling":
                    self.weight_castling = item[2][1]
                elif item[1][1] == "@weight-abandoned-castling":
                    self.weight_abandoned_castling = item[2][1]
                elif item[1][1] == "@run":
                    self.is_runnable = True

    def SaveAs(self, filename):
        try:
            f = open(filename, "w")
            f.write(self.GenSayulispCode())
            f.close()
        except:
            tkm.showerror("Error", "Couldn't Open '" + filename + "'")

    def LoadSayulisp(self, filename):
        try:
            f = open(filename, "r")
            self.SetFromCode(f.read())
            f.close
        except:
            tkm.showerror("Error", "Couldn't Open '" + filename + "'")

    def __init__(self):
        self.SetDefault()

INT_REG = re.compile(R"^-?\d+$")
NUMBER_REG = re.compile(R"^-?\d+\.?\d*$")
def CheckInteger(s):
    if INT_REG.match(s): return True
    tkm.showerror("Error", "There are invalid value.")
    return False
def CheckNumber(s):
    if NUMBER_REG.match(s): return True
    tkm.showerror("Error", "There are invalid value.")
    return False

class GUI(tk.Frame):
    class ConfigureWindow(tk.Toplevel):
        def __init__(self, master, title="Blank"):
            tk.Toplevel.__init__(self, master)
            self.resizable(False, False)
            self.editor = master.editor
            self.title(title)
            self.content_frame = tk.Frame(self)
            self.content_frame.pack(padx=10, pady=5)

            frame = tk.Frame(self)
            tk.Button(frame, text="OK", command=self.OnOk)\
            .grid(row=0, column=0)
            tk.Button(frame, text="Cancel", command=self.OnCancel)\
            .grid(row=0, column=1)
            frame.pack(padx=10, pady=5)

        def OnOk(self):
            self.OnCancel()

        def OnCancel(self):
            self.destroy()

    class Entry64Widget(tk.Frame):
        def __init__(self, master, light_color, dark_color):
            tk.Frame.__init__(self, master)
            self["relief"] = "solid"
            self["borderwidth"] = 1

            tk.Label(self, text="Southeast corner is A1(A8 for Black).\n" \
            + " Northwest corner is H8(H1 for Black).").pack()

            frame_2 = tk.Frame(self)
            self.value_array = []
            for i in range(64):
                col = i % 8
                row = 7 - (i // 8)
                color = ""
                if (row % 2) == 1:
                    if (col % 2) == 0: color = dark_color
                    else: color = light_color
                else:
                    if (col % 2) == 0: color = light_color
                    else: color = dark_color
                self.value_array += [tk.StringVar()]
                self.value_array[-1].set(str(i))
                canvas = tk.Canvas(frame_2, bg=color, width=60, height=60, \
                borderwidth=0, highlightthickness=0)
                entry = tk.Entry(canvas, textvariable=self.value_array[-1], \
                font=("Arial", 14), width=4)
                canvas.create_window(7, 20, window=entry, anchor=tk.NW)
                canvas.grid(row=row, column=col, padx=0, pady=0, \
                ipadx=0, ipady=0)
            frame_2.pack()

    def GenButtonCommand(self, constructor):
        def Closure():
            win = constructor(self)
            win.update_idletasks()
            w = win.winfo_width()
            h = win.winfo_height()
            pw = win.master.winfo_screenwidth()
            ph = win.master.winfo_screenheight()
            x = (pw // 2) - (w // 2)
            y = (ph // 2) - (h // 2)
            xs = "+" + str(x) if x >= 0 else "-" + str(x)
            ys = "+" + str(y) if y >= 0 else "-" + str(y)
            win.geometry("{0}x{1}{2}{3}".format(w, h, xs, ys))
        return Closure


    def GenSearchParamsBox(self):
        self.search_params_box.canvas = \
        tk.Canvas(self.search_params_box, bg="#ffffff", borderwidth=0, \
        highlightthickness=0)
        def BindMouseWheel(wid):
            wid.bind("<MouseWheel>", \
            lambda e: self.search_params_box.canvas.yview_scroll(1, tk.UNITS))
            wid.bind("<Button-4>", \
            lambda e: self.search_params_box.canvas.yview_scroll(-1, tk.UNITS))
            wid.bind("<Button-5>", \
            lambda e: self.search_params_box.canvas.yview_scroll(1, tk.UNITS))
        BindMouseWheel(self.search_params_box.canvas)

        scroll = tk.Scrollbar(self.search_params_box, orient=tk.VERTICAL, \
        command=self.search_params_box.canvas.yview)
        self.search_params_box.canvas["yscrollcommand"] = scroll.set
        scroll.pack(side=tk.RIGHT, fill=tk.Y, padx=0)

        frame = tk.Frame(self.search_params_box.canvas, bg="#ffffff")
        self.search_params_box.canvas.create_window\
        (0, 0, window=frame, anchor=tk.NW)
        def OnConfigure(event):
            temp = {\
            "scrollregion": self.search_params_box.canvas.bbox(tk.ALL), \
            "width": 400, "height": 400}
            self.search_params_box.canvas.configure(**temp)
        frame.bind("<Configure>", OnConfigure)
        BindMouseWheel(frame)

        self.search_params_list = []

        # 項目のセット。
        def SetTempFrame(title, config_class=GUI.ConfigureWindow):
            nonlocal self
            temp_frame = tk.Frame(frame, bg="#ffffff")
            temp_frame.title_frame = tk.Frame(temp_frame, bg="#ffffff")
            temp_frame.title_frame.config_button = \
            tk.Button(temp_frame.title_frame, \
            command=self.GenButtonCommand(config_class), \
            text="Configure", justify=tk.LEFT)
            temp_frame.title_frame.title_label = \
            tk.Label(temp_frame.title_frame, bg="#ffffff", \
            font=("Arial", "16", "bold"), anchor=tk.W, justify=tk.LEFT, \
            text=title)
            self.search_params_list += [temp_frame] 

        class MaterialConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Material")
                self.pawn_value = tk.StringVar()
                self.pawn_value.set(str(int(self.editor.material[1])))
                self.knight_value = tk.StringVar()
                self.knight_value.set(str(int(self.editor.material[2])))
                self.bishop_value = tk.StringVar()
                self.bishop_value.set(str(int(self.editor.material[3])))
                self.rook_value = tk.StringVar()
                self.rook_value.set(str(int(self.editor.material[4])))
                self.queen_value = tk.StringVar()
                self.queen_value.set(str(int(self.editor.material[5])))
                self.king_value = tk.StringVar()
                self.king_value.set(str(int(self.editor.material[6])))
                tk.Label(self.content_frame, text="Pawn")\
                .grid(row=0, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Knight")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Bishop")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Rook")\
                .grid(row=3, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Queen")\
                .grid(row=4, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="King")\
                .grid(row=5, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, textvariable=self.pawn_value, \
                width=10).grid(row=0, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, textvariable=self.knight_value, \
                width=10).grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, textvariable=self.bishop_value, \
                width=10).grid(row=2, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, textvariable=self.rook_value, \
                width=10).grid(row=3, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, textvariable=self.queen_value, \
                width=10).grid(row=4, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, textvariable=self.king_value, \
                width=10).grid(row=5, column=1, sticky=tk.W)

            def OnOk(self):
                if CheckInteger(self.pawn_value.get()) \
                and CheckInteger(self.knight_value.get()) \
                and CheckInteger(self.bishop_value.get()) \
                and CheckInteger(self.rook_value.get()) \
                and CheckInteger(self.queen_value.get()) \
                and CheckInteger(self.king_value.get()):
                    self.editor.material[1] = \
                    float(int(self.pawn_value.get()))
                    self.editor.material[2] = \
                    float(int(self.knight_value.get()))
                    self.editor.material[3] = \
                    float(int(self.bishop_value.get()))
                    self.editor.material[4] = \
                    float(int(self.rook_value.get()))
                    self.editor.material[5] = \
                    float(int(self.queen_value.get()))
                    self.editor.material[6] = \
                    float(int(self.king_value.get()))
                self.OnCancel()
        SetTempFrame("Material", MaterialConfig)
        class QuiesceSearchConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Quiescence Search")
                self.enable_value = tk.IntVar()
                if self.editor.enable_quiesce_search: self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable Quiescence Search")\
                .pack()
            def OnOk(self):
                if self.enable_value.get() == 0:
                    self.editor.enable_quiesce_search = False
                else:
                    self.editor.enable_quiesce_search = True
                self.OnCancel()
        SetTempFrame("Quiescence Search", QuiesceSearchConfig)
        class RepetitionConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Checking Repetition")
                self.enable_value = tk.IntVar()
                if self.editor.enable_repetition_check:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable Checking Repetition")\
                .pack()
            def OnOk(self):
                if self.enable_value.get() == 0:
                    self.editor.enable_repetition_check = False
                else:
                    self.editor.enable_repetition_check = True
                self.OnCancel()
        SetTempFrame("Checking Repetition", RepetitionConfig)
        class ExtensionConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Check Extension")
                self.enable_value = tk.IntVar()
                if self.editor.enable_check_extension:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable Check Extension")\
                .pack()
            def OnOk(self):
                if self.enable_value.get() == 0:
                    self.editor.enable_check_extension = False
                else:
                    self.editor.enable_check_extension = True
                self.OnCancel()
        SetTempFrame("Check Extension", ExtensionConfig)
        class YBWCConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "YBWC")
                self.limit_depth_value = tk.StringVar()
                self.limit_depth_value.set\
                (str(int(self.editor.ybwc_limit_depth)))
                self.invalid_moves_value = tk.StringVar()
                self.invalid_moves_value.set\
                (str(int(self.editor.ybwc_invalid_moves)))
                tk.Label(self.content_frame, text="Depth limit")\
                .grid(row=0, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=\
                "Number of moves searched by single thread")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.limit_depth_value, width=10)\
                .grid(row=0, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.invalid_moves_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.limit_depth_value.get()) \
                and CheckInteger(self.invalid_moves_value.get()):
                    self.editor.ybwc_limit_depth = \
                    float(int(self.limit_depth_value.get()))
                    self.editor.ybwc_invalid_moves = \
                    float(int(self.invalid_moves_value.get()))
                self.OnCancel()
        SetTempFrame("YBWC", YBWCConfig)
        class AspirationConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__\
                (self, master, "Aspiration Windows")
                self.enable_value = tk.IntVar()
                if self.editor.enable_aspiration_windows:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable Aspiration Windows")\
                .grid(row=0, column=0, columnspan=2)
                self.limit_depth_value = tk.StringVar()
                self.limit_depth_value.set\
                (str(int(self.editor.aspiration_windows_limit_depth)))
                self.delta_value = tk.StringVar()
                self.delta_value.set\
                (str(int(self.editor.aspiration_windows_delta)))
                tk.Label(self.content_frame, text="Depth limit")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Delta score")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.limit_depth_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.delta_value, width=10)\
                .grid(row=2, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.limit_depth_value.get()) \
                and CheckInteger(self.delta_value.get()):
                    if self.enable_value.get() == 0:
                        self.editor.enable_aspiration_windows = False
                    else:
                        self.editor.enable_aspiration_windows = True
                    self.editor.aspiration_windows_limit_depth = \
                    float(int(self.limit_depth_value.get()))
                    self.editor.aspiration_windows_delta = \
                    float(int(self.delta_value.get()))
                self.OnCancel()
        SetTempFrame("Aspiration Windows", AspirationConfig)
        class SEEConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "SEE")
                self.enable_value = tk.IntVar()
                if self.editor.enable_see:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable SEE").pack()
            def OnOk(self):
                if self.enable_value.get() == 0:
                    self.editor.enable_see = False
                else:
                    self.editor.enable_see = True
                self.OnCancel()
        SetTempFrame("SEE", SEEConfig)
        class HistoryConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "History Heuristics")
                self.enable_value = tk.IntVar()
                if self.editor.enable_history:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable History Heuristics")\
                .pack()
            def OnOk(self):
                if self.enable_value.get() == 0:
                    self.editor.enable_history = False
                else:
                    self.editor.enable_history = True
                self.OnCancel()
        SetTempFrame("History Heuristics", HistoryConfig)
        class KillerConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Killer Move Heuristics")
                self.enable_value = tk.IntVar()
                if self.editor.enable_killer:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text=\
                "Enable Killer Moves Heuristics")\
                .pack()
            def OnOk(self):
                if self.enable_value.get() == 0:
                    self.editor.enable_killer = False
                else:
                    self.editor.enable_killer = True
                self.OnCancel()
        SetTempFrame("Killer Move Heuristics", KillerConfig)
        class HashConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Transposition Table")
                self.enable_value = tk.IntVar()
                if self.editor.enable_hash_table:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable Transposition Table")\
                .pack()
            def OnOk(self):
                if self.enable_value.get() == 0:
                    self.editor.enable_hash_table = False
                else:
                    self.editor.enable_hash_table = True
                self.OnCancel()
        SetTempFrame("Transposition Table\n(Hash Table)", HashConfig)
        class IIDConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Internal Iterative Deepening")
                self.enable_value = tk.IntVar()
                if self.editor.enable_iid:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text=\
                "Enable Internal Iterative Deepening")\
                .grid(row=0, column=0, columnspan=2)
                self.limit_depth_value = tk.StringVar()
                self.limit_depth_value.set\
                (str(int(self.editor.iid_limit_depth)))
                self.search_depth_value = tk.StringVar()
                self.search_depth_value.set\
                (str(int(self.editor.iid_search_depth)))
                tk.Label(self.content_frame, text="Depth limit")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Search depth")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.limit_depth_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.search_depth_value, width=10)\
                .grid(row=2, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.limit_depth_value.get()) \
                and CheckInteger(self.search_depth_value.get()):
                    if self.enable_value.get() == 0:
                        self.editor.enable_iid = False
                    else:
                        self.editor.enable_iid = True
                    self.editor.iid_limit_depth = \
                    float(int(self.limit_depth_value.get()))
                    self.editor.iid_search_depth = \
                    float(int(self.search_depth_value.get()))
                self.OnCancel()
        SetTempFrame("Internal Iterative Deepening", IIDConfig)
        class NMRConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Null Move Reduction")
                self.enable_value = tk.IntVar()
                if self.editor.enable_nmr:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text=\
                "Enable Null Move Reduction")\
                .grid(row=0, column=0, columnspan=2)
                self.limit_depth_value = tk.StringVar()
                self.limit_depth_value.set\
                (str(int(self.editor.nmr_limit_depth)))
                self.search_reduction_value = tk.StringVar()
                self.search_reduction_value.set\
                (str(int(self.editor.nmr_search_reduction)))
                self.reduction_value = tk.StringVar()
                self.reduction_value.set\
                (str(int(self.editor.nmr_reduction)))
                tk.Label(self.content_frame, text="Depth limit")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=\
                "Reduction of null move search")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Reduction")\
                .grid(row=3, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.limit_depth_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.search_reduction_value, width=10)\
                .grid(row=2, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.reduction_value, width=10)\
                .grid(row=3, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.limit_depth_value.get()) \
                and CheckInteger(self.search_reduction_value.get()) \
                and CheckInteger(self.reduction_value.get()):
                    if self.enable_value.get() == 0:
                        self.editor.enable_nmr = False
                    else:
                        self.editor.enable_nmr = True
                    self.editor.nmr_limit_depth = \
                    float(int(self.limit_depth_value.get()))
                    self.editor.nmr_search_reduction = \
                    float(int(self.search_reduction_value.get()))
                    self.editor.nmr_reduction = \
                    float(int(self.reduction_value.get()))
                self.OnCancel()
        SetTempFrame("Null Move Reduction", NMRConfig)
        class ProbCutConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "ProbCut")
                self.enable_value = tk.IntVar()
                if self.editor.enable_probcut:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable ProbCut")\
                .grid(row=0, column=0, columnspan=2)
                self.limit_depth_value = tk.StringVar()
                self.limit_depth_value.set\
                (str(int(self.editor.probcut_limit_depth)))
                self.margin_value = tk.StringVar()
                self.margin_value.set\
                (str(int(self.editor.probcut_margin)))
                self.search_reduction_value = tk.StringVar()
                self.search_reduction_value.set\
                (str(int(self.editor.probcut_search_reduction)))
                tk.Label(self.content_frame, text="Depth limit")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Margin of Beta")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Reduction of search")\
                .grid(row=3, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.limit_depth_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.margin_value, width=10)\
                .grid(row=2, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.search_reduction_value, width=10)\
                .grid(row=3, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.limit_depth_value.get()) \
                and CheckInteger(self.margin_value.get()) \
                and CheckInteger(self.search_reduction_value.get()):
                    if self.enable_value.get() == 0:
                        self.editor.enable_probcut = False
                    else:
                        self.editor.enable_probcut = True
                    self.editor.probcut_limit_depth = \
                    float(int(self.limit_depth_value.get()))
                    self.editor.probcut_margin = \
                    float(int(self.margin_value.get()))
                    self.editor.probcut_search_reduction = \
                    float(int(self.search_reduction_value.get()))
                self.OnCancel()
        SetTempFrame("ProbCut", ProbCutConfig)
        class HistoryPruningConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "History Pruning")
                self.enable_value = tk.IntVar()
                if self.editor.enable_history_pruning:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable History Pruning")\
                .grid(row=0, column=0, columnspan=2)
                self.limit_depth_value = tk.StringVar()
                self.limit_depth_value.set\
                (str(int(self.editor.history_pruning_limit_depth)))
                self.move_threshold_value = tk.StringVar()
                self.move_threshold_value.set\
                (str(self.editor.history_pruning_move_threshold))
                self.invalid_moves_value = tk.StringVar()
                self.invalid_moves_value.set\
                (str(int(self.editor.history_pruning_invalid_moves)))
                self.threshold_value = tk.StringVar()
                self.threshold_value.set\
                (str(self.editor.history_pruning_threshold))
                self.reduction_value = tk.StringVar()
                self.reduction_value.set\
                (str(int(self.editor.history_pruning_reduction)))
                tk.Label(self.content_frame, text="Depth limit")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=\
                "Proportion of invalid moves (0.0 .. 1.0)")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Invalid moves")\
                .grid(row=3, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=\
                "Threshold of history value (0.0 .. 1.0)")\
                .grid(row=4, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Reduction")\
                .grid(row=5, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.limit_depth_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.move_threshold_value, width=10)\
                .grid(row=2, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.invalid_moves_value, width=10)\
                .grid(row=3, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.threshold_value, width=10)\
                .grid(row=4, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.reduction_value, width=10)\
                .grid(row=5, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.limit_depth_value.get()) \
                and CheckNumber(self.move_threshold_value.get()) \
                and CheckInteger(self.invalid_moves_value.get()) \
                and CheckNumber(self.threshold_value.get()) \
                and CheckInteger(self.reduction_value.get()):
                    if self.enable_value.get() == 0:
                        self.editor.enable_history_pruning = False
                    else:
                        self.editor.enable_history_pruning = True
                    self.editor.history_pruning_limit_depth = \
                    float(int(self.limit_depth_value.get()))
                    temp = float(self.move_threshold_value.get())
                    if 0.0 <= temp <= 1.0:
                        self.editor.history_pruning_move_threshold = temp
                    else:
                        tkm.showerror("Error", "There are invalid value.")
                    self.editor.history_pruning_invalid_moves = \
                    float(int(self.invalid_moves_value.get()))
                    temp = float(self.threshold_value.get())
                    if 0.0 <= temp <= 1.0:
                        self.editor.history_pruning_threshold = temp
                    else:
                        tkm.showerror("Error", "There are invalid value.")
                    self.editor.history_pruning_reduction = \
                    float(int(self.reduction_value.get()))
                self.OnCancel()
        SetTempFrame("History Pruning", HistoryPruningConfig)
        class LMRConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Late Move Reduction")
                self.enable_value = tk.IntVar()
                if self.editor.enable_lmr:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable Late Move Reduction")\
                .grid(row=0, column=0, columnspan=2)
                self.limit_depth_value = tk.StringVar()
                self.limit_depth_value.set\
                (str(int(self.editor.lmr_limit_depth)))
                self.move_threshold_value = tk.StringVar()
                self.move_threshold_value.set\
                (str(self.editor.lmr_move_threshold))
                self.invalid_moves_value = tk.StringVar()
                self.invalid_moves_value.set\
                (str(int(self.editor.lmr_invalid_moves)))
                self.search_reduction_value = tk.StringVar()
                self.search_reduction_value.set\
                (str(int(self.editor.lmr_search_reduction)))
                tk.Label(self.content_frame, text="Depth limit")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=\
                "Proportion of invalid moves (0.0 .. 1.0)")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Invalid moves")\
                .grid(row=3, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Reduction of search")\
                .grid(row=4, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.limit_depth_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.move_threshold_value, width=10)\
                .grid(row=2, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.invalid_moves_value, width=10)\
                .grid(row=3, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.search_reduction_value, width=10)\
                .grid(row=4, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.limit_depth_value.get()) \
                and CheckNumber(self.move_threshold_value.get()) \
                and CheckInteger(self.invalid_moves_value.get()) \
                and CheckInteger(self.search_reduction_value.get()):
                    if self.enable_value.get() == 0:
                        self.editor.enable_lmr = False
                    self.editor.lmr_limit_depth = \
                    float(int(self.limit_depth_value.get()))
                    temp = float(self.move_threshold_value.get())
                    if 0.0 <= temp <= 1.0:
                        self.editor.lmr_move_threshold = temp
                    else:
                        tkm.showerror("Error", "There are invalid value.")
                    self.editor.lmr_invalid_moves = \
                    float(int(self.invalid_moves_value.get()))
                    self.editor.lmr_search_reduction = \
                    float(int(self.search_reduction_value.get()))
                self.OnCancel()
        SetTempFrame("Late Move Reduction", LMRConfig)
        class FutilityConfig(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Futility Pruning")
                self.enable_value = tk.IntVar()
                if self.editor.enable_futility_pruning:
                    self.enable_value.set(1)
                else: self.enable_value.set(0)
                tk.Checkbutton(self.content_frame, \
                variable=self.enable_value, text="Enable Futility Pruning")\
                .grid(row=0, column=0, columnspan=2)
                self.depth_value = tk.StringVar()
                self.depth_value.set\
                (str(int(self.editor.futility_pruning_depth)))
                self.margin_value = tk.StringVar()
                self.margin_value.set\
                (str(int(self.editor.futility_pruning_margin)))
                tk.Label(self.content_frame, text="Depth")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text="Margin")\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.depth_value, width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.margin_value, width=10)\
                .grid(row=2, column=1, sticky=tk.W)
            def OnOk(self):
                if CheckInteger(self.depth_value.get()) \
                and CheckInteger(self.margin_value.get()):
                    if self.enable_value.get() == 0:
                        self.editor.enable_futility_pruning = False
                    else:
                        self.editor.enable_futility_pruning = True
                    self.editor.futility_pruning_depth = \
                    float(int(self.depth_value.get()))
                    self.editor.futility_pruning_margin = \
                    float(int(self.margin_value.get()))
                self.OnCancel()
        SetTempFrame("Futility Pruning", FutilityConfig)

        for param in self.search_params_list:
            BindMouseWheel(param)
            BindMouseWheel(param.title_frame)
            BindMouseWheel(param.title_frame.config_button)
            BindMouseWheel(param.title_frame.title_label)
            param.title_frame.config_button.pack(side=tk.LEFT)
            param.title_frame.title_label.pack(side=tk.LEFT)
            param.title_frame.pack(anchor=tk.W)
            param.pack(anchor=tk.W, pady=16)

        self.search_params_box.canvas.pack()

    def GenEvalParamsBox(self):
        self.eval_params_box.canvas = \
        tk.Canvas(self.eval_params_box, bg="#ffffff", borderwidth=0, \
        highlightthickness=0)
        def BindMouseWheel(wid):
            wid.bind("<MouseWheel>", \
            lambda e: self.eval_params_box.canvas.yview_scroll(1, tk.UNITS))
            wid.bind("<Button-4>", \
            lambda e: self.eval_params_box.canvas.yview_scroll(-1, tk.UNITS))
            wid.bind("<Button-5>", \
            lambda e: self.eval_params_box.canvas.yview_scroll(1, tk.UNITS))
        BindMouseWheel(self.eval_params_box.canvas)

        scroll = tk.Scrollbar(self.eval_params_box, orient=tk.VERTICAL, \
        command=self.eval_params_box.canvas.yview)
        self.eval_params_box.canvas["yscrollcommand"] = scroll.set
        scroll.pack(side=tk.RIGHT, fill=tk.Y, padx=0)

        frame = tk.Frame(self.eval_params_box.canvas, bg="#ffffff")
        self.eval_params_box.canvas.create_window\
        (0, 0, window=frame, anchor=tk.NW)
        def OnConfigure(event):
            temp = {\
            "scrollregion": self.eval_params_box.canvas.bbox(tk.ALL), \
            "width": 400, "height": 400}
            self.eval_params_box.canvas.configure(**temp)
        frame.bind("<Configure>", OnConfigure)
        BindMouseWheel(frame)

        self.eval_params_list = []

        # 項目のセット。
        def SetTempFrame(title, config_class=GUI.ConfigureWindow):
            nonlocal self
            temp_frame = tk.Frame(frame, bg="#ffffff")
            temp_frame.title_frame = tk.Frame(temp_frame, bg="#ffffff")
            temp_frame.title_frame.config_button = \
            tk.Button(temp_frame.title_frame, \
            command=self.GenButtonCommand(config_class), \
            text="Configure", justify=tk.LEFT)
            temp_frame.title_frame.title_label = \
            tk.Label(temp_frame.title_frame, bg="#ffffff", \
            font=("Arial", "16", "bold"), anchor=tk.W, justify=tk.LEFT, \
            text=title)
            self.eval_params_list += [temp_frame] 

        class PieceSquareWindow(GUI.ConfigureWindow):
            def __init__(self, master, title, op_table, ed_table):
                GUI.ConfigureWindow.__init__(self, master, title)
                self.op_table = op_table
                self.ed_table = ed_table
                op_frame = \
                tk.LabelFrame(self.content_frame, text="Opening Table")
                self.op_board = \
                GUI.Entry64Widget(op_frame, "#bb7777", "#770000")
                self.op_board.pack()
                op_frame.grid(row=0, column=0, padx=10)
                ed_frame = \
                tk.LabelFrame(self.content_frame, text="Ending Table")
                self.ed_board = \
                GUI.Entry64Widget(ed_frame, "#7777bb", "#000077")
                self.ed_board.pack()
                ed_frame.grid(row=0, column=1, padx=10)
                for i in range(64):
                    self.op_board.value_array[i].set(str(self.op_table[i]))
                    self.ed_board.value_array[i].set(str(self.ed_table[i]))
            def OnOk(self):
                for i in range(64):
                    if (not CheckNumber(self.op_board.value_array[i].get())) \
                    or (not CheckNumber(self.ed_board.value_array[i].get())):
                        break
                else:
                    for i in range(64):
                        self.op_table[i] = \
                        float(self.op_board.value_array[i].get())
                        self.ed_table[i] = \
                        float(self.ed_board.value_array[i].get())
                self.OnCancel()

        class PawnSquareWindow(PieceSquareWindow):
            def __init__(self, master):
                PieceSquareWindow.__init__(self, master, \
                "Piece Square Table (Pawn)",
                master.editor.pawn_square_table_opening, \
                master.editor.pawn_square_table_ending)
        SetTempFrame("Piece Square Table\n(Pawn)", PawnSquareWindow)
        class KnightSquareWindow(PieceSquareWindow):
            def __init__(self, master):
                PieceSquareWindow.__init__(self, master, \
                "Piece Square Table (Knight)",
                master.editor.knight_square_table_opening, \
                master.editor.knight_square_table_ending)
        SetTempFrame("Piece Square Table\n(Knight)", KnightSquareWindow)
        class BishopSquareWindow(PieceSquareWindow):
            def __init__(self, master):
                PieceSquareWindow.__init__(self, master, \
                "Piece Square Table (Bishop)",
                master.editor.bishop_square_table_opening, \
                master.editor.bishop_square_table_ending)
        SetTempFrame("Piece Square Table\n(Bishop)", BishopSquareWindow)
        class RookSquareWindow(PieceSquareWindow):
            def __init__(self, master):
                PieceSquareWindow.__init__(self, master, \
                "Piece Square Table (Rook)",
                master.editor.rook_square_table_opening, \
                master.editor.rook_square_table_ending)
        SetTempFrame("Piece Square Table\n(Rook)", RookSquareWindow)
        class QueenSquareWindow(PieceSquareWindow):
            def __init__(self, master):
                PieceSquareWindow.__init__(self, master, \
                "Piece Square Table (Queen)",
                master.editor.queen_square_table_opening, \
                master.editor.queen_square_table_ending)
        SetTempFrame("Piece Square Table\n(Queen)", QueenSquareWindow)
        class KingSquareWindow(PieceSquareWindow):
            def __init__(self, master):
                PieceSquareWindow.__init__(self, master, \
                "Piece Square Table (King)",
                master.editor.king_square_table_opening, \
                master.editor.king_square_table_ending)
        SetTempFrame("Piece Square Table\n(King)", KingSquareWindow)

        class WeightFrame(tk.Frame):
            def __init__(self, master, weight):
                tk.Frame.__init__(self, master)
                self.weight = weight
                self.value_array = [tk.StringVar(), tk.StringVar()]
                self.value_array[0].set(str(self.weight[0]))
                self.value_array[1].set(str(self.weight[1]))
                tk.Label(self, text="Opening Weight")\
                .grid(row=0, column=0, sticky=tk.E)
                tk.Label(self, text="Ending Weight")\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Entry(self, textvariable=self.value_array[0], width=10)\
                .grid(row=0, column=1, sticky=tk.W)
                tk.Entry(self, textvariable=self.value_array[1], width=10)\
                .grid(row=1, column=1, sticky=tk.W)
            def CheckValue(self):
                if CheckNumber(self.value_array[0].get()) \
                and CheckNumber(self.value_array[1].get()):
                    return True
                return False
            def SetValue(self):
                if CheckNumber(self.value_array[0].get()) \
                and CheckNumber(self.value_array[1].get()):
                    self.weight[0] = float(self.value_array[0].get())
                    self.weight[1] = float(self.value_array[1].get())
                    return True
                return False

        class AttackDefenseWindow(GUI.ConfigureWindow):
            def __init__(self, master, title, tag_table, weight, table):
                GUI.ConfigureWindow.__init__(self, master, title)
                self.w_frame = WeightFrame(self.content_frame, weight)
                self.w_frame.grid(row=0, column=0, columnspan=2)
                self.table = table
                self.value_array = []
                for i in range(7):
                    self.value_array += [tk.StringVar()]
                    self.value_array[i].set(str(self.table[i]))
                tk.Label(self.content_frame, text=tag_table[0])\
                .grid(row=1, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=tag_table[1])\
                .grid(row=2, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=tag_table[2])\
                .grid(row=3, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=tag_table[3])\
                .grid(row=4, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=tag_table[4])\
                .grid(row=5, column=0, sticky=tk.E)
                tk.Label(self.content_frame, text=tag_table[5])\
                .grid(row=6, column=0, sticky=tk.E)
                tk.Entry(self.content_frame, \
                textvariable=self.value_array[1], width=10)\
                .grid(row=1, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.value_array[2], width=10)\
                .grid(row=2, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.value_array[3], width=10)\
                .grid(row=3, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.value_array[4], width=10)\
                .grid(row=4, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.value_array[5], width=10)\
                .grid(row=5, column=1, sticky=tk.W)
                tk.Entry(self.content_frame, \
                textvariable=self.value_array[6], width=10)\
                .grid(row=6, column=1, sticky=tk.W)
            def OnOk(self):
                if self.w_frame.SetValue():
                    for value in self.value_array:
                        if not CheckNumber(value.get()): break
                    else:
                        for i in range(7):
                            self.table[i] = float(self.value_array[i].get())
                self.OnCancel()

        tag_table_1 = [\
        "Attack Enemy Pawn", "Attack Enemy Knight", "Attack Enemy Bishop", \
        "Attack Enemy Rook", "Attack Enemy Queen", "Attack Enemy King",]
        class PawnAttackWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Attack (Pawn)", tag_table_1, \
                master.editor.weight_pawn_attack, \
                master.editor.pawn_attack_table)
        SetTempFrame("Attack (Pawn)", PawnAttackWindow)
        class KnightAttackWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Attack (Knight)", tag_table_1, \
                master.editor.weight_knight_attack, \
                master.editor.knight_attack_table)
        SetTempFrame("Attack (Knight)", KnightAttackWindow)
        class BishopAttackWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Attack (Bishop)", tag_table_1, \
                master.editor.weight_bishop_attack, \
                master.editor.bishop_attack_table)
        SetTempFrame("Attack (Bishop)", BishopAttackWindow)
        class RookAttackWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Attack (Rook)", tag_table_1, \
                master.editor.weight_rook_attack, \
                master.editor.rook_attack_table)
        SetTempFrame("Attack (Rook)", RookAttackWindow)
        class QueenAttackWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Attack (Queen)", tag_table_1, \
                master.editor.weight_queen_attack, \
                master.editor.queen_attack_table)
        SetTempFrame("Attack (Queen)", QueenAttackWindow)
        class KingAttackWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Attack (King)", tag_table_1, \
                master.editor.weight_king_attack, \
                master.editor.king_attack_table)
        SetTempFrame("Attack (King)", KingAttackWindow)
        tag_table_2 = [\
        "Defense Friend Pawn", "Defense Friend Knight", \
        "Defense Friend Bishop", "Defense Friend Rook", \
        "Defense Friend Queen", "Defense Friend King",]
        class PawnDefenseWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Defense (Pawn)", tag_table_2, \
                master.editor.weight_pawn_defense, \
                master.editor.pawn_defense_table)
        SetTempFrame("Defense (Pawn)", PawnDefenseWindow)
        class KnightDefenseWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Defense (Knight)", tag_table_2, \
                master.editor.weight_knight_defense, \
                master.editor.knight_defense_table)
        SetTempFrame("Defense (Knight)", KnightDefenseWindow)
        class BishopDefenseWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Defense (Bishop)", tag_table_2, \
                master.editor.weight_bishop_defense, \
                master.editor.bishop_defense_table)
        SetTempFrame("Defense (Bishop)", BishopDefenseWindow)
        class RookDefenseWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Defense (Rook)", tag_table_2, \
                master.editor.weight_rook_defense, \
                master.editor.rook_defense_table)
        SetTempFrame("Defense (Rook)", RookDefenseWindow)
        class QueenDefenseWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Defense (Queen)", tag_table_2, \
                master.editor.weight_queen_defense, \
                master.editor.queen_defense_table)
        SetTempFrame("Defense (Queen)", QueenDefenseWindow)
        class KingDefenseWindow(AttackDefenseWindow):
            def __init__(self, master):
                AttackDefenseWindow.__init__(self, master, \
                "Defense (King)", tag_table_2, \
                master.editor.weight_king_defense, \
                master.editor.king_defense_table)
        SetTempFrame("Defense (King)", KingDefenseWindow)

        class AllPiecesWeightFrame(tk.Frame):
            def __init__(self, master, weight_list):
                tk.Frame.__init__(self, master)
                self.w_frame = []
                temp = tk.LabelFrame(self, text="Pawn")
                self.w_frame += [WeightFrame(temp, weight_list[0])]
                self.w_frame[0].pack()
                temp.pack()
                temp = tk.LabelFrame(self, text="Knight")
                self.w_frame += [WeightFrame(temp, weight_list[1])]
                self.w_frame[1].pack()
                temp.pack()
                temp = tk.LabelFrame(self, text="Bishop")
                self.w_frame += [WeightFrame(temp, weight_list[2])]
                self.w_frame[2].pack()
                temp.pack()
                temp = tk.LabelFrame(self, text="Rook")
                self.w_frame += [WeightFrame(temp, weight_list[3])]
                self.w_frame[3].pack()
                temp.pack()
                temp = tk.LabelFrame(self, text="Queen")
                self.w_frame += [WeightFrame(temp, weight_list[4])]
                self.w_frame[4].pack()
                temp.pack()
                temp = tk.LabelFrame(self, text="King")
                self.w_frame += [WeightFrame(temp, weight_list[5])]
                self.w_frame[5].pack()
                temp.pack()
            def SetValue(self):
                for w_frame in self.w_frame:
                    if not w_frame.CheckValue(): break
                else:
                    for w_frame in self.w_frame:
                        if not w_frame.SetValue(): break

        class PinFrame(tk.Frame):
            def __init__(self, master, matrix):
                tk.Frame.__init__(self, master)
                self.matrix = matrix
                tk.Label(self).grid(row=0, column=0)
                tk.Label(self, text="Pawn (Pinned)").grid(row=1, column=0)
                tk.Label(self, text="Knight (Pinned)").grid(row=2, column=0)
                tk.Label(self, text="Bishop (Pinned)").grid(row=3, column=0)
                tk.Label(self, text="Rook (Pinned)").grid(row=4, column=0)
                tk.Label(self, text="Queen (Pinned)").grid(row=5, column=0)
                tk.Label(self, text="King (Pinned)").grid(row=6, column=0)
                tk.Label(self, text="Pawn (Behind)").grid(row=0, column=1)
                tk.Label(self, text="Knight (Behind)").grid(row=0, column=2)
                tk.Label(self, text="Bishop (Behind)").grid(row=0, column=3)
                tk.Label(self, text="Rook (Behind)").grid(row=0, column=4)
                tk.Label(self, text="Queen (Behind)").grid(row=0, column=5)
                tk.Label(self, text="King (Behind)").grid(row=0, column=6)
                self.value_matrix = []
                for i in range(7):
                    self.value_matrix += [[]]
                    for j in range(7):
                        self.value_matrix[i] += [tk.StringVar()]
                        self.value_matrix[i][j].set(str(self.matrix[i][j]))
                        if (i >= 1) and (j >= 1):
                            tk.Entry(self, \
                            textvariable=self.value_matrix[i][j], width=15)\
                            .grid(row=i, column=j)
            def SetValue(self):
                check = True
                for i in range(7):
                    for j in range(7):
                        if not CheckNumber(self.value_matrix[i][j].get()):
                            check = False
                            break
                    if not check: break
                else:
                    for i in range(7):
                        for j in range(7):
                            self.matrix[i][j] = \
                            float(self.value_matrix[i][j].get())

        class BishopPinWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Pin (Bishop)")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_bishop_pin)
                self.w_frame.pack()
                self.m_frame = \
                PinFrame(self.content_frame, master.editor.bishop_pin_table)
                self.m_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.m_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Pin (Bishop)", BishopPinWindow)
        class RookPinWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Pin (Rook)")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_rook_pin)
                self.w_frame.pack()
                self.m_frame = \
                PinFrame(self.content_frame, master.editor.rook_pin_table)
                self.m_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.m_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Pin (Rook)", RookPinWindow)
        class QueenPinWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Pin (Queen)")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_queen_pin)
                self.w_frame.pack()
                self.m_frame = \
                PinFrame(self.content_frame, master.editor.queen_pin_table)
                self.m_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.m_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Pin (Queen)", QueenPinWindow)
        class MobilityWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Mobility")
                temp_array = [\
                master.editor.weight_pawn_mobility, \
                master.editor.weight_knight_mobility, \
                master.editor.weight_bishop_mobility, \
                master.editor.weight_rook_mobility, \
                master.editor.weight_queen_mobility, \
                master.editor.weight_king_mobility]
                self.all_frame = \
                AllPiecesWeightFrame(self.content_frame, temp_array)
                self.all_frame.pack()
            def OnOk(self):
                self.all_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Mobility", MobilityWindow)
        class CenterControlWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "CenterControl")
                temp_array = [\
                master.editor.weight_pawn_center_control, \
                master.editor.weight_knight_center_control, \
                master.editor.weight_bishop_center_control, \
                master.editor.weight_rook_center_control, \
                master.editor.weight_queen_center_control, \
                master.editor.weight_king_center_control]
                self.all_frame = \
                AllPiecesWeightFrame(self.content_frame, temp_array)
                self.all_frame.pack()
            def OnOk(self):
                self.all_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Center Control", CenterControlWindow)
        class SweetCenterControlWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "SweetCenterControl")
                temp_array = [\
                master.editor.weight_pawn_sweet_center_control, \
                master.editor.weight_knight_sweet_center_control, \
                master.editor.weight_bishop_sweet_center_control, \
                master.editor.weight_rook_sweet_center_control, \
                master.editor.weight_queen_sweet_center_control, \
                master.editor.weight_king_sweet_center_control]
                self.all_frame = \
                AllPiecesWeightFrame(self.content_frame, temp_array)
                self.all_frame.pack()
            def OnOk(self):
                self.all_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Sweet Center Control", SweetCenterControlWindow)
        class DevelopmentWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Development")
                temp_array = [\
                master.editor.weight_pawn_development, \
                master.editor.weight_knight_development, \
                master.editor.weight_bishop_development, \
                master.editor.weight_rook_development, \
                master.editor.weight_queen_development, \
                master.editor.weight_king_development]
                self.all_frame = \
                AllPiecesWeightFrame(self.content_frame, temp_array)
                self.all_frame.pack()
            def OnOk(self):
                self.all_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Development", DevelopmentWindow)
        class AttackAroundKingWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Attack around Enemy King")
                temp_array = [\
                master.editor.weight_pawn_attack_around_king, \
                master.editor.weight_knight_attack_around_king, \
                master.editor.weight_bishop_attack_around_king, \
                master.editor.weight_rook_attack_around_king, \
                master.editor.weight_queen_attack_around_king, \
                master.editor.weight_king_attack_around_king]
                self.all_frame = \
                AllPiecesWeightFrame(self.content_frame, temp_array)
                self.all_frame.pack()
            def OnOk(self):
                self.all_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Attack around Enemy King", AttackAroundKingWindow)
        class PassPawnWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Pass Pawn")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_pass_pawn)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Pass Pawn", PassPawnWindow)
        class ProtectedPassPawnWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Protected Pass Pawn")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_protected_pass_pawn)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Protected Pass Pawn", ProtectedPassPawnWindow)
        class DoublePawnWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Double Pawn")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_double_pawn)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Double Pawn", DoublePawnWindow)
        class IsolatedPawnWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Isolated Pawn")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_iso_pawn)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Isolated Pawn", IsolatedPawnWindow)
        class PawnShieldWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Pawn Shield")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_pawn_shield)
                self.w_frame.pack()
                self.board_widget = GUI.Entry64Widget(self.content_frame, \
                "#bb7777", "#770000")
                for i in range(64):
                    self.board_widget.value_array[i]\
                    .set(str(master.editor.pawn_shield_table[i]))
                self.board_widget.pack()
            def OnOk(self):
                if self.w_frame.SetValue():
                    for value in self.board_widget.value_array:
                        if not CheckNumber(value.get()): break
                    else:
                        for i in range(64):
                            self.editor.pawn_shield_table[i] = \
                            float(self.board_widget.value_array[i].get())
                self.OnCancel()
        SetTempFrame("Pawn shield", PawnShieldWindow)
        class BishopPairWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Bishop Pair")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_bishop_pair)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Bishop Pair", BishopPairWindow)
        class BadBishopWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Bad Bishop")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_bad_bishop)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Bad Bishop", BadBishopWindow)
        class RookPairWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, "Rook Pair")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_rook_pair)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Rook Pair", RookPairWindow)
        class RookSemiOpenWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Rook on Semi-Open File")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_rook_semiopen_fyle)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Rook on Semi-Open File", RookSemiOpenWindow)
        class RookOpenWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Rook on Open File")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_rook_open_fyle)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Rook on Open File", RookOpenWindow)
        class QueenStartingWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Queen Starting Too Early")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_early_queen_starting)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Queen Starting Too Early", QueenStartingWindow)
        class WeakSquaregWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Weak Square around King")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_weak_square)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Weak Square around King", WeakSquaregWindow)
        class CastlingWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Castling Bonus")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_castling)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Castling Bonus", CastlingWindow)
        class AbandonedCastlingWindow(GUI.ConfigureWindow):
            def __init__(self, master):
                GUI.ConfigureWindow.__init__(self, master, \
                "Abandoned Castling Rights")
                self.w_frame = WeightFrame(self.content_frame, \
                master.editor.weight_abandoned_castling)
                self.w_frame.pack()
            def OnOk(self):
                self.w_frame.SetValue()
                self.OnCancel()
        SetTempFrame("Abandoned Castling Rights", AbandonedCastlingWindow)

        for param in self.eval_params_list:
            BindMouseWheel(param)
            BindMouseWheel(param.title_frame)
            BindMouseWheel(param.title_frame.config_button)
            BindMouseWheel(param.title_frame.title_label)
            param.title_frame.config_button.pack(side=tk.LEFT)
            param.title_frame.title_label.pack(side=tk.LEFT)
            param.title_frame.pack(anchor=tk.W)
            param.pack(anchor=tk.W, pady=16)

        self.eval_params_box.canvas.pack()

    def __init__(self, master=None):
        tk.Frame.__init__(self, master)

        self.editor = Editor()

        text = \
        "Push 'Configure' button of an item which you want to configure."
        temp = {"text": text, "font": ("Arial", 12)}
        tk.Label(self, **temp).pack(padx=10, pady=10)

        self.select_box = tk.Frame(self)

        temp = {"borderwidth": 1, "relief": "solid"}
        temp_2 = {"font": ("Arial", 24, "bold")}

        tk.Label(self.select_box, text="Search Algorithm", **temp_2).\
        grid(row=0, column=0, padx=10)
        self.search_params_box = tk.Frame(self.select_box, **temp)
        self.GenSearchParamsBox()
        self.search_params_box.grid(row=1, column=0, padx=10)

        tk.Label(self.select_box, text="Evaluation Function", **temp_2).\
        grid(row=0, column=1, padx=10)
        self.eval_params_box = tk.Frame(self.select_box, **temp)
        self.GenEvalParamsBox()
        self.eval_params_box.grid(row=1, column=1, padx=10)

        self.select_box.pack(pady=10)

        temp = ("Arial", 12)
        option_box = tk.Frame(self)
        temp_frame = tk.Frame(option_box)
        tk.Label(temp_frame, text="Engine Symbol", font=temp)\
        .pack(side=tk.LEFT, padx=5)
        self.engine_name = tk.StringVar()
        self.engine_name.set(self.editor.engine_name)
        tk.Entry(temp_frame, textvariable=self.engine_name, font=temp)\
        .pack(side=tk.LEFT, padx=5)
        temp_frame.pack(anchor=tk.W)
        self.is_generating = tk.IntVar()
        self.is_generating.set(1 if self.editor.is_generating else 0)
        tk.Checkbutton(option_box, text="Generating Engine", \
        variable=self.is_generating, font=temp).pack(anchor=tk.W)
        self.is_runnable = tk.IntVar()
        self.is_runnable.set(1 if self.editor.is_runnable else 0)
        tk.Checkbutton(option_box, text="Runnable", \
        variable=self.is_runnable, font=temp).pack(anchor=tk.W)
        option_box.pack()

        button_box = tk.Frame(self)

        file_types = [("All Types", ".*")]
        def OpenAndLoad():
            f_name = \
            tkf.askopenfilename(parent=self, title="Import Sayulisp", \
            filetypes=file_types)
            if f_name and isinstance(f_name, str):
                self.editor.LoadSayulisp(f_name)
                self.engine_name.set(self.editor.engine_name)
                self.is_generating.set(1 if self.editor.is_generating else 0)
                self.is_runnable.set(1 if self.editor.is_runnable else 0)
        font = ("Arial", 12)
        tk.Button(button_box, text="Import", \
        font=font, command=OpenAndLoad).grid(row=0, column=0, padx=50)

        def OpenAndSave():
            self.editor.engine_name = self.engine_name.get()
            self.editor.is_generating = \
            True if self.is_generating.get() else False
            self.editor.is_runnable = \
            True if self.is_runnable.get() else False
            f_name = \
            tkf.asksaveasfilename(parent=self, title="Export Sayulisp", \
            filetypes=file_types)
            if f_name and isinstance(f_name, str):
                self.editor.SaveAs(f_name)
        tk.Button(button_box, text="Export", \
        font=font, command=OpenAndSave).grid(row=0, column=1, padx=50)

        tk.Button(button_box, text="Quit", \
        font=font, command=master.destroy).grid(row=0, column=2, padx=50)

        button_box.pack(padx=10, pady=10)


if __name__ == "__main__":
    app = tk.Tk()
    gui = GUI(app)
    gui.pack()

    app.resizable(False, False)
    app.title("Sayulisp Editor")
    app.update_idletasks()
    width = app.winfo_width()
    height = app.winfo_height()
    x = (app.winfo_screenwidth() / 2) - (width / 2)
    y = (app.winfo_screenheight() / 2) - (height / 2)
    app.geometry("{0}x{1}+{2}+{3}".format(width, height, int(x), int(y)))
    app.option_add("*Font", "Arial")

    gui.mainloop()
