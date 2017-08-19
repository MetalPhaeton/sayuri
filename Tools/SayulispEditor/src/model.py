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

"""Model of SayulispModel."""

import sys,os
import copy
import tkinter.messagebox as tkm
import src.lisp_handler as lh
from src.common import *

def GetFullFileName(filename):
    return os.path.realpath(os.path.abspath\
    (os.path.expandvars(os.path.expanduser(filename))))

class Model:
    DEFAULT_ENGINE_NAME = "my-engine"

    MATERIAL_SYMBOL = "@material"
    ENABLE_QUIESCE_SEARCH_SYMBOL = "@enable-quiesce-search"
    ENABLE_REPETITION_CHECK_SYMBOL = "@enable-repetition-check"
    ENABLE_CHECK_EXTENSION_SYMBOL = "@enable-check-extension"
    YBWC_LIMIT_DEPTH_SYMBOL = "@ybwc-limit-depth"
    YBWC_INVALID_MOVES_SYMBOL = "@ybwc-invalid-moves"
    ENABLE_ASPIRATION_WINDOWS_SYMBOL = "@enable-aspiration-windows"
    ASPIRATION_WINDOWS_LIMIT_DEPTH_SYMBOL = "@aspiration-windows-limit-depth"
    ASPIRATION_WINDOWS_DELTA_SYMBOL = "@aspiration-windows-delta"
    ENABLE_SEE_SYMBOL = "@enable-see"
    ENABLE_HISTORY_SYMBOL = "@enable-history"
    ENABLE_KILLER_SYMBOL = "@enable-killer"
    ENABLE_HASH_TABLE_SYMBOL = "@enable-hash-table"
    ENABLE_IID_SYMBOL = "@enable-iid"
    IID_LIMIT_DEPTH_SYMBOL = "@iid-limit-depth"
    IID_SEARCH_DEPTH_SYMBOL = "@iid-search-depth"
    ENABLE_NMR_SYMBOL = "@enable-nmr"
    NMR_LIMIT_DEPTH_SYMBOL = "@nmr-limit-depth"
    NMR_SEARCH_REDUCTION_SYMBOL = "@nmr-search-reduction"
    NMR_REDUCTION_SYMBOL = "@nmr-reduction"
    ENABLE_PROBCUT_SYMBOL = "@enable-probcut"
    PROBCUT_LIMIT_DEPTH_SYMBOL = "@probcut-limit-depth"
    PROBCUT_MARGIN_SYMBOL = "@probcut-margin"
    PROBCUT_SEARCH_REDUCTION_SYMBOL = "@probcut-search-reduction"
    ENABLE_HISTORY_PRUNING_SYMBOL = "@enable-history-pruning"
    HISTORY_PRUNING_LIMIT_DEPTH_SYMBOL = "@history-pruning-limit-depth"
    HISTORY_PRUNING_MOVE_THRESHOLD_SYMBOL = "@history-pruning-move-threshold"
    HISTORY_PRUNING_INVALID_MOVES_SYMBOL = "@history-pruning-invalid-moves"
    HISTORY_PRUNING_THRESHOLD_SYMBOL = "@history-pruning-threshold"
    HISTORY_PRUNING_REDUCTION_SYMBOL = "@history-pruning-reduction"
    ENABLE_LMR_SYMBOL = "@enable-lmr"
    LMR_LIMIT_DEPTH_SYMBOL = "@lmr-limit-depth"
    LMR_MOVE_THRESHOLD_SYMBOL = "@lmr-move-threshold"
    LMR_INVALID_MOVES_SYMBOL = "@lmr-invalid-moves"
    LMR_SEARCH_REDUCTION_SYMBOL = "@lmr-search-reduction"
    ENABLE_FUTILITY_PRUNING_SYMBOL = "@enable-futility-pruning"
    FUTILITY_PRUNING_DEPTH_SYMBOL = "@futility-pruning-depth"
    FUTILITY_PRUNING_MARGIN_SYMBOL = "@futility-pruning-margin"
    PAWN_SQUARE_TABLE_OPENING_SYMBOL = "@pawn-square-table-opening"
    KNIGHT_SQUARE_TABLE_OPENING_SYMBOL = "@knight-square-table-opening"
    BISHOP_SQUARE_TABLE_OPENING_SYMBOL = "@bishop-square-table-opening"
    ROOK_SQUARE_TABLE_OPENING_SYMBOL = "@rook-square-table-opening"
    QUEEN_SQUARE_TABLE_OPENING_SYMBOL = "@queen-square-table-opening"
    KING_SQUARE_TABLE_OPENING_SYMBOL = "@king-square-table-opening"
    PAWN_SQUARE_TABLE_ENDING_SYMBOL = "@pawn-square-table-ending"
    KNIGHT_SQUARE_TABLE_ENDING_SYMBOL = "@knight-square-table-ending"
    BISHOP_SQUARE_TABLE_ENDING_SYMBOL = "@bishop-square-table-ending"
    ROOK_SQUARE_TABLE_ENDING_SYMBOL = "@rook-square-table-ending"
    QUEEN_SQUARE_TABLE_ENDING_SYMBOL = "@queen-square-table-ending"
    KING_SQUARE_TABLE_ENDING_SYMBOL = "@king-square-table-ending"
    PAWN_ATTACK_TABLE_SYMBOL = "@pawn-attack-table"
    KNIGHT_ATTACK_TABLE_SYMBOL = "@knight-attack-table"
    BISHOP_ATTACK_TABLE_SYMBOL = "@bishop-attack-table"
    ROOK_ATTACK_TABLE_SYMBOL = "@rook-attack-table"
    QUEEN_ATTACK_TABLE_SYMBOL = "@queen-attack-table"
    KING_ATTACK_TABLE_SYMBOL = "@king-attack-table"
    PAWN_DEFENSE_TABLE_SYMBOL = "@pawn-defense-table"
    KNIGHT_DEFENSE_TABLE_SYMBOL_SYMBOL = "@knight-defense-table"
    BISHOP_DEFENSE_TABLE_SYMBOL = "@bishop-defense-table"
    ROOK_DEFENSE_TABLE_SYMBOL = "@rook-defense-table"
    QUEEN_DEFENSE_TABLE_SYMBOL = "@queen-defense-table"
    KING_DEFENSE_TABLE_SYMBOL = "@king-defense-table"
    PAWN_SHIELD_TABLE_SYMBOL = "@pawn-shield-table"
    WEIGHT_PAWN_OPENING_POSITION_SYMBOL = "@weight-pawn-opening-position"
    WEIGHT_KNIGHT_OPENING_POSITION_SYMBOL = "@weight-knight-opening-position"
    WEIGHT_BISHOP_OPENING_POSITION_SYMBOL = "@weight-bishop-opening-position"
    WEIGHT_ROOK_OPENING_POSITION_SYMBOL = "@weight-rook-opening-position"
    WEIGHT_QUEEN_OPENING_POSITION_SYMBOL = "@weight-queen-opening-position"
    WEIGHT_KING_OPENING_POSITION_SYMBOL = "@weight-king-opening-position"
    WEIGHT_PAWN_ENDING_POSITION_SYMBOL = "@weight-pawn-ending-position"
    WEIGHT_KNIGHT_ENDING_POSITION_SYMBOL = "@weight-knight-ending-position"
    WEIGHT_BISHOP_ENDING_POSITION_SYMBOL = "@weight-bishop-ending-position"
    WEIGHT_ROOK_ENDING_POSITION_SYMBOL = "@weight-rook-ending-position"
    WEIGHT_QUEEN_ENDING_POSITION_SYMBOL = "@weight-queen-ending-position"
    WEIGHT_KING_ENDING_POSITION_SYMBOL = "@weight-king-ending-position"
    WEIGHT_PAWN_MOBILITY_SYMBOL = "@weight-pawn-mobility"
    WEIGHT_KNIGHT_MOBILITY_SYMBOL = "@weight-knight-mobility"
    WEIGHT_BISHOP_MOBILITY_SYMBOL = "@weight-bishop-mobility"
    WEIGHT_ROOK_MOBILITY_SYMBOL = "@weight-rook-mobility"
    WEIGHT_QUEEN_MOBILITY_SYMBOL = "@weight-queen-mobility"
    WEIGHT_KING_MOBILITY_SYMBOL = "@weight-king-mobility"
    WEIGHT_PAWN_CENTER_CONTROL_SYMBOL = "@weight-pawn-center-control"
    WEIGHT_KNIGHT_CENTER_CONTROL_SYMBOL = "@weight-knight-center-control"
    WEIGHT_BISHOP_CENTER_CONTROL_SYMBOL = "@weight-bishop-center-control"
    WEIGHT_ROOK_CENTER_CONTROL_SYMBOL = "@weight-rook-center-control"
    WEIGHT_QUEEN_CENTER_CONTROL_SYMBOL = "@weight-queen-center-control"
    WEIGHT_KING_CENTER_CONTROL_SYMBOL = "@weight-king-center-control"
    WEIGHT_PAWN_SWEET_CENTER_CONTROL_SYMBOL = \
    "@weight-pawn-sweet-center-control"
    WEIGHT_KNIGHT_SWEET_CENTER_CONTROL_SYMBOL = \
    "@weight-knight-sweet-center-control"
    WEIGHT_BISHOP_SWEET_CENTER_CONTROL_SYMBOL = \
    "@weight-bishop-sweet-center-control"
    WEIGHT_ROOK_SWEET_CENTER_CONTROL_SYMBOL = \
    "@weight-rook-sweet-center-control"
    WEIGHT_QUEEN_SWEET_CENTER_CONTROL_SYMBOL = \
    "@weight-queen-sweet-center-control"
    WEIGHT_KING_SWEET_CENTER_CONTROL_SYMBOL = \
    "@weight-king-sweet-center-control"
    WEIGHT_PAWN_DEVELOPMENT_SYMBOL = "@weight-pawn-development"
    WEIGHT_KNIGHT_DEVELOPMENT_SYMBOL = "@weight-knight-development"
    WEIGHT_BISHOP_DEVELOPMENT_SYMBOL = "@weight-bishop-development"
    WEIGHT_ROOK_DEVELOPMENT_SYMBOL = "@weight-rook-development"
    WEIGHT_QUEEN_DEVELOPMENT_SYMBOL = "@weight-queen-development"
    WEIGHT_KING_DEVELOPMENT_SYMBOL = "@weight-king-development"
    WEIGHT_PAWN_ATTACK_SYMBOL = "@weight-pawn-attack"
    WEIGHT_KNIGHT_ATTACK_SYMBOL = "@weight-knight-attack"
    WEIGHT_BISHOP_ATTACK_SYMBOL = "@weight-bishop-attack"
    WEIGHT_ROOK_ATTACK_SYMBOL = "@weight-rook-attack"
    WEIGHT_QUEEN_ATTACK_SYMBOL = "@weight-queen-attack"
    WEIGHT_KING_ATTACK_SYMBOL = "@weight-king-attack"
    WEIGHT_PAWN_DEFENSE_SYMBOL = "@weight-pawn-defense"
    WEIGHT_KNIGHT_DEFENSE_SYMBOL = "@weight-knight-defense"
    WEIGHT_BISHOP_DEFENSE_SYMBOL = "@weight-bishop-defense"
    WEIGHT_ROOK_DEFENSE_SYMBOL = "@weight-rook-defense"
    WEIGHT_QUEEN_DEFENSE_SYMBOL = "@weight-queen-defense"
    WEIGHT_KING_DEFENSE_SYMBOL = "@weight-king-defense"
    WEIGHT_PAWN_ATTACK_AROUND_KING_SYMBOL = "@weight-pawn-attack-around-king"
    WEIGHT_KNIGHT_ATTACK_AROUND_KING_SYMBOL = \
    "@weight-knight-attack-around-king"
    WEIGHT_BISHOP_ATTACK_AROUND_KING_SYMBOL = \
    "@weight-bishop-attack-around-king"
    WEIGHT_ROOK_ATTACK_AROUND_KING_SYMBOL = "@weight-rook-attack-around-king"
    WEIGHT_QUEEN_ATTACK_AROUND_KING_SYMBOL = "@weight-queen-attack-around-king"
    WEIGHT_KING_ATTACK_AROUND_KING_SYMBOL = "@weight-king-attack-around-king"
    WEIGHT_PASS_PAWN_SYMBOL = "@weight-pass-pawn"
    WEIGHT_PROTECTED_PASS_PAWN_SYMBOL = "@weight-protected-pass-pawn"
    WEIGHT_DOUBLE_PAWN_SYMBOL = "@weight-double-pawn"
    WEIGHT_ISO_PAWN_SYMBOL = "@weight-iso-pawn"
    WEIGHT_PAWN_SHIELD_SYMBOL = "@weight-pawn-shield"
    WEIGHT_BISHOP_PAIR_SYMBOL = "@weight-bishop-pair"
    WEIGHT_BAD_BISHOP_SYMBOL = "@weight-bad-bishop"
    WEIGHT_ROOK_PAIR_SYMBOL = "@weight-rook-pair"
    WEIGHT_ROOK_SEMIOPEN_FYLE_SYMBOL = "@weight-rook-semiopen-fyle"
    WEIGHT_ROOK_OPEN_FYLE_SYMBOL = "@weight-rook-open-fyle"
    WEIGHT_EARLY_QUEEN_STARTING_SYMBOL = "@weight-early-queen-starting"
    WEIGHT_WEAK_SQUARE_SYMBOL = "@weight-weak-square"
    WEIGHT_CASTLING_SYMBOL = "@weight-castling"
    WEIGHT_ABANDONED_CASTLING_SYMBOL = "@weight-abandoned-castling"

    def GenSayulispCode(self):
        ret = ""
        if self.is_generating:
            ret += ";; Generates Engine.\n"
            ret += "(define " + self.engine_name + " (gen-engine))\n\n"

        to_sexpr = lambda symbol, param: \
        lh.EngineSExpr(self.engine_name, symbol, param) + "\n"

        ret += ";; Configures Engine.\n"

        ret += to_sexpr(Model.MATERIAL_SYMBOL, self.material)
        ret += to_sexpr(Model.ENABLE_QUIESCE_SEARCH_SYMBOL, \
        self.enable_quiesce_search)
        ret += to_sexpr(Model.ENABLE_REPETITION_CHECK_SYMBOL, \
        self.enable_repetition_check)
        ret += to_sexpr(Model.ENABLE_CHECK_EXTENSION_SYMBOL, \
        self.enable_check_extension)
        ret += to_sexpr(Model.YBWC_LIMIT_DEPTH_SYMBOL, \
        self.ybwc_limit_depth)
        ret += to_sexpr(Model.YBWC_INVALID_MOVES_SYMBOL, \
        self.ybwc_invalid_moves)
        ret += to_sexpr(Model.ENABLE_ASPIRATION_WINDOWS_SYMBOL, \
        self.enable_aspiration_windows)
        ret += to_sexpr(Model.ASPIRATION_WINDOWS_LIMIT_DEPTH_SYMBOL, \
        self.aspiration_windows_limit_depth)
        ret += to_sexpr(Model.ASPIRATION_WINDOWS_DELTA_SYMBOL, \
        self.aspiration_windows_delta)
        ret += to_sexpr(Model.ENABLE_SEE_SYMBOL, \
        self.enable_see)
        ret += to_sexpr(Model.ENABLE_HISTORY_SYMBOL, \
        self.enable_history)
        ret += to_sexpr(Model.ENABLE_KILLER_SYMBOL, \
        self.enable_killer)
        ret += to_sexpr(Model.ENABLE_HASH_TABLE_SYMBOL, \
        self.enable_hash_table)
        ret += to_sexpr(Model.ENABLE_IID_SYMBOL, \
        self.enable_iid)
        ret += to_sexpr(Model.IID_LIMIT_DEPTH_SYMBOL, \
        self.iid_limit_depth)
        ret += to_sexpr(Model.IID_SEARCH_DEPTH_SYMBOL, \
        self.iid_search_depth)
        ret += to_sexpr(Model.ENABLE_NMR_SYMBOL, \
        self.enable_nmr)
        ret += to_sexpr(Model.NMR_LIMIT_DEPTH_SYMBOL, \
        self.nmr_limit_depth)
        ret += to_sexpr(Model.NMR_SEARCH_REDUCTION_SYMBOL, \
        self.nmr_search_reduction)
        ret += to_sexpr(Model.NMR_REDUCTION_SYMBOL, \
        self.nmr_reduction)
        ret += to_sexpr(Model.ENABLE_PROBCUT_SYMBOL, \
        self.enable_probcut)
        ret += to_sexpr(Model.PROBCUT_LIMIT_DEPTH_SYMBOL, \
        self.probcut_limit_depth)
        ret += to_sexpr(Model.PROBCUT_MARGIN_SYMBOL, \
        self.probcut_margin)
        ret += to_sexpr(Model.PROBCUT_SEARCH_REDUCTION_SYMBOL, \
        self.probcut_search_reduction)
        ret += to_sexpr(Model.ENABLE_HISTORY_PRUNING_SYMBOL, \
        self.enable_history_pruning)
        ret += to_sexpr(Model.HISTORY_PRUNING_LIMIT_DEPTH_SYMBOL, \
        self.history_pruning_limit_depth)
        ret += to_sexpr(Model.HISTORY_PRUNING_MOVE_THRESHOLD_SYMBOL, \
        self.history_pruning_move_threshold)
        ret += to_sexpr(Model.HISTORY_PRUNING_INVALID_MOVES_SYMBOL, \
        self.history_pruning_invalid_moves)
        ret += to_sexpr(Model.HISTORY_PRUNING_THRESHOLD_SYMBOL, \
        self.history_pruning_threshold)
        ret += to_sexpr(Model.HISTORY_PRUNING_REDUCTION_SYMBOL, \
        self.history_pruning_reduction)
        ret += to_sexpr(Model.ENABLE_LMR_SYMBOL, \
        self.enable_lmr)
        ret += to_sexpr(Model.LMR_LIMIT_DEPTH_SYMBOL, \
        self.lmr_limit_depth)
        ret += to_sexpr(Model.LMR_MOVE_THRESHOLD_SYMBOL, \
        self.lmr_move_threshold)
        ret += to_sexpr(Model.LMR_INVALID_MOVES_SYMBOL, \
        self.lmr_invalid_moves)
        ret += to_sexpr(Model.LMR_SEARCH_REDUCTION_SYMBOL, \
        self.lmr_search_reduction)
        ret += to_sexpr(Model.ENABLE_FUTILITY_PRUNING_SYMBOL, \
        self.enable_futility_pruning)
        ret += to_sexpr(Model.FUTILITY_PRUNING_DEPTH_SYMBOL, \
        self.futility_pruning_depth)
        ret += to_sexpr(Model.FUTILITY_PRUNING_MARGIN_SYMBOL, \
        self.futility_pruning_margin)
        ret += to_sexpr(Model.PAWN_SQUARE_TABLE_OPENING_SYMBOL, \
        self.pawn_square_table_opening)
        ret += to_sexpr(Model.KNIGHT_SQUARE_TABLE_OPENING_SYMBOL, \
        self.knight_square_table_opening)
        ret += to_sexpr(Model.BISHOP_SQUARE_TABLE_OPENING_SYMBOL, \
        self.bishop_square_table_opening)
        ret += to_sexpr(Model.ROOK_SQUARE_TABLE_OPENING_SYMBOL, \
        self.rook_square_table_opening)
        ret += to_sexpr(Model.QUEEN_SQUARE_TABLE_OPENING_SYMBOL, \
        self.queen_square_table_opening)
        ret += to_sexpr(Model.KING_SQUARE_TABLE_OPENING_SYMBOL, \
        self.king_square_table_opening)
        ret += to_sexpr(Model.PAWN_SQUARE_TABLE_ENDING_SYMBOL, \
        self.pawn_square_table_ending)
        ret += to_sexpr(Model.KNIGHT_SQUARE_TABLE_ENDING_SYMBOL, \
        self.knight_square_table_ending)
        ret += to_sexpr(Model.BISHOP_SQUARE_TABLE_ENDING_SYMBOL, \
        self.bishop_square_table_ending)
        ret += to_sexpr(Model.ROOK_SQUARE_TABLE_ENDING_SYMBOL, \
        self.rook_square_table_ending)
        ret += to_sexpr(Model.QUEEN_SQUARE_TABLE_ENDING_SYMBOL, \
        self.queen_square_table_ending)
        ret += to_sexpr(Model.KING_SQUARE_TABLE_ENDING_SYMBOL, \
        self.king_square_table_ending)
        ret += to_sexpr(Model.PAWN_ATTACK_TABLE_SYMBOL, \
        self.pawn_attack_table)
        ret += to_sexpr(Model.KNIGHT_ATTACK_TABLE_SYMBOL, \
        self.knight_attack_table)
        ret += to_sexpr(Model.BISHOP_ATTACK_TABLE_SYMBOL, \
        self.bishop_attack_table)
        ret += to_sexpr(Model.ROOK_ATTACK_TABLE_SYMBOL, \
        self.rook_attack_table)
        ret += to_sexpr(Model.QUEEN_ATTACK_TABLE_SYMBOL, \
        self.queen_attack_table)
        ret += to_sexpr(Model.KING_ATTACK_TABLE_SYMBOL, \
        self.king_attack_table)
        ret += to_sexpr(Model.PAWN_DEFENSE_TABLE_SYMBOL, \
        self.pawn_defense_table)
        ret += to_sexpr(Model.KNIGHT_DEFENSE_TABLE_SYMBOL_SYMBOL, \
        self.knight_defense_table)
        ret += to_sexpr(Model.BISHOP_DEFENSE_TABLE_SYMBOL, \
        self.bishop_defense_table)
        ret += to_sexpr(Model.ROOK_DEFENSE_TABLE_SYMBOL, \
        self.rook_defense_table)
        ret += to_sexpr(Model.QUEEN_DEFENSE_TABLE_SYMBOL, \
        self.queen_defense_table)
        ret += to_sexpr(Model.KING_DEFENSE_TABLE_SYMBOL, \
        self.king_defense_table)
        ret += to_sexpr(Model.PAWN_SHIELD_TABLE_SYMBOL, \
        self.pawn_shield_table)
        ret += to_sexpr(Model.WEIGHT_PAWN_OPENING_POSITION_SYMBOL, \
        self.weight_pawn_opening_position)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_OPENING_POSITION_SYMBOL, \
        self.weight_knight_opening_position)
        ret += to_sexpr(Model.WEIGHT_BISHOP_OPENING_POSITION_SYMBOL, \
        self.weight_bishop_opening_position)
        ret += to_sexpr(Model.WEIGHT_ROOK_OPENING_POSITION_SYMBOL, \
        self.weight_rook_opening_position)
        ret += to_sexpr(Model.WEIGHT_QUEEN_OPENING_POSITION_SYMBOL, \
        self.weight_queen_opening_position)
        ret += to_sexpr(Model.WEIGHT_KING_OPENING_POSITION_SYMBOL, \
        self.weight_king_opening_position)
        ret += to_sexpr(Model.WEIGHT_PAWN_ENDING_POSITION_SYMBOL, \
        self.weight_pawn_ending_position)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_ENDING_POSITION_SYMBOL, \
        self.weight_knight_ending_position)
        ret += to_sexpr(Model.WEIGHT_BISHOP_ENDING_POSITION_SYMBOL, \
        self.weight_bishop_ending_position)
        ret += to_sexpr(Model.WEIGHT_ROOK_ENDING_POSITION_SYMBOL, \
        self.weight_rook_ending_position)
        ret += to_sexpr(Model.WEIGHT_QUEEN_ENDING_POSITION_SYMBOL, \
        self.weight_queen_ending_position)
        ret += to_sexpr(Model.WEIGHT_KING_ENDING_POSITION_SYMBOL, \
        self.weight_king_ending_position)
        ret += to_sexpr(Model.WEIGHT_PAWN_MOBILITY_SYMBOL, \
        self.weight_pawn_mobility)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_MOBILITY_SYMBOL, \
        self.weight_knight_mobility)
        ret += to_sexpr(Model.WEIGHT_BISHOP_MOBILITY_SYMBOL, \
        self.weight_bishop_mobility)
        ret += to_sexpr(Model.WEIGHT_ROOK_MOBILITY_SYMBOL, \
        self.weight_rook_mobility)
        ret += to_sexpr(Model.WEIGHT_QUEEN_MOBILITY_SYMBOL, \
        self.weight_queen_mobility)
        ret += to_sexpr(Model.WEIGHT_KING_MOBILITY_SYMBOL, \
        self.weight_king_mobility)
        ret += to_sexpr(Model.WEIGHT_PAWN_CENTER_CONTROL_SYMBOL, \
        self.weight_pawn_center_control)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_CENTER_CONTROL_SYMBOL, \
        self.weight_knight_center_control)
        ret += to_sexpr(Model.WEIGHT_BISHOP_CENTER_CONTROL_SYMBOL, \
        self.weight_bishop_center_control)
        ret += to_sexpr(Model.WEIGHT_ROOK_CENTER_CONTROL_SYMBOL, \
        self.weight_rook_center_control)
        ret += to_sexpr(Model.WEIGHT_QUEEN_CENTER_CONTROL_SYMBOL, \
        self.weight_queen_center_control)
        ret += to_sexpr(Model.WEIGHT_KING_CENTER_CONTROL_SYMBOL, \
        self.weight_king_center_control)
        ret += to_sexpr(Model.WEIGHT_PAWN_SWEET_CENTER_CONTROL_SYMBOL, \
        self.weight_pawn_sweet_center_control)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_SWEET_CENTER_CONTROL_SYMBOL, \
        self.weight_knight_sweet_center_control)
        ret += to_sexpr(Model.WEIGHT_BISHOP_SWEET_CENTER_CONTROL_SYMBOL, \
        self.weight_bishop_sweet_center_control)
        ret += to_sexpr(Model.WEIGHT_ROOK_SWEET_CENTER_CONTROL_SYMBOL, \
        self.weight_rook_sweet_center_control)
        ret += to_sexpr(Model.WEIGHT_QUEEN_SWEET_CENTER_CONTROL_SYMBOL, \
        self.weight_queen_sweet_center_control)
        ret += to_sexpr(Model.WEIGHT_KING_SWEET_CENTER_CONTROL_SYMBOL, \
        self.weight_king_sweet_center_control)
        ret += to_sexpr(Model.WEIGHT_PAWN_DEVELOPMENT_SYMBOL, \
        self.weight_pawn_development)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_DEVELOPMENT_SYMBOL, \
        self.weight_knight_development)
        ret += to_sexpr(Model.WEIGHT_BISHOP_DEVELOPMENT_SYMBOL, \
        self.weight_bishop_development)
        ret += to_sexpr(Model.WEIGHT_ROOK_DEVELOPMENT_SYMBOL, \
        self.weight_rook_development)
        ret += to_sexpr(Model.WEIGHT_QUEEN_DEVELOPMENT_SYMBOL, \
        self.weight_queen_development)
        ret += to_sexpr(Model.WEIGHT_KING_DEVELOPMENT_SYMBOL, \
        self.weight_king_development)
        ret += to_sexpr(Model.WEIGHT_PAWN_ATTACK_SYMBOL, \
        self.weight_pawn_attack)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_ATTACK_SYMBOL, \
        self.weight_knight_attack)
        ret += to_sexpr(Model.WEIGHT_BISHOP_ATTACK_SYMBOL, \
        self.weight_bishop_attack)
        ret += to_sexpr(Model.WEIGHT_ROOK_ATTACK_SYMBOL, \
        self.weight_rook_attack)
        ret += to_sexpr(Model.WEIGHT_QUEEN_ATTACK_SYMBOL, \
        self.weight_queen_attack)
        ret += to_sexpr(Model.WEIGHT_KING_ATTACK_SYMBOL, \
        self.weight_king_attack)
        ret += to_sexpr(Model.WEIGHT_PAWN_DEFENSE_SYMBOL, \
        self.weight_pawn_defense)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_DEFENSE_SYMBOL, \
        self.weight_knight_defense)
        ret += to_sexpr(Model.WEIGHT_BISHOP_DEFENSE_SYMBOL, \
        self.weight_bishop_defense)
        ret += to_sexpr(Model.WEIGHT_ROOK_DEFENSE_SYMBOL, \
        self.weight_rook_defense)
        ret += to_sexpr(Model.WEIGHT_QUEEN_DEFENSE_SYMBOL, \
        self.weight_queen_defense)
        ret += to_sexpr(Model.WEIGHT_KING_DEFENSE_SYMBOL, \
        self.weight_king_defense)
        ret += to_sexpr(Model.WEIGHT_PAWN_ATTACK_AROUND_KING_SYMBOL, \
        self.weight_pawn_attack_around_king)
        ret += to_sexpr(Model.WEIGHT_KNIGHT_ATTACK_AROUND_KING_SYMBOL, \
        self.weight_knight_attack_around_king)
        ret += to_sexpr(Model.WEIGHT_BISHOP_ATTACK_AROUND_KING_SYMBOL, \
        self.weight_bishop_attack_around_king)
        ret += to_sexpr(Model.WEIGHT_ROOK_ATTACK_AROUND_KING_SYMBOL, \
        self.weight_rook_attack_around_king)
        ret += to_sexpr(Model.WEIGHT_QUEEN_ATTACK_AROUND_KING_SYMBOL, \
        self.weight_queen_attack_around_king)
        ret += to_sexpr(Model.WEIGHT_KING_ATTACK_AROUND_KING_SYMBOL, \
        self.weight_king_attack_around_king)
        ret += to_sexpr(Model.WEIGHT_PASS_PAWN_SYMBOL, \
        self.weight_pass_pawn)
        ret += to_sexpr(Model.WEIGHT_PROTECTED_PASS_PAWN_SYMBOL, \
        self.weight_protected_pass_pawn)
        ret += to_sexpr(Model.WEIGHT_DOUBLE_PAWN_SYMBOL, \
        self.weight_double_pawn)
        ret += to_sexpr(Model.WEIGHT_ISO_PAWN_SYMBOL, \
        self.weight_iso_pawn)
        ret += to_sexpr(Model.WEIGHT_PAWN_SHIELD_SYMBOL, \
        self.weight_pawn_shield)
        ret += to_sexpr(Model.WEIGHT_BISHOP_PAIR_SYMBOL, \
        self.weight_bishop_pair)
        ret += to_sexpr(Model.WEIGHT_BAD_BISHOP_SYMBOL, \
        self.weight_bad_bishop)
        ret += to_sexpr(Model.WEIGHT_ROOK_PAIR_SYMBOL, \
        self.weight_rook_pair)
        ret += to_sexpr(Model.WEIGHT_ROOK_SEMIOPEN_FYLE_SYMBOL, \
        self.weight_rook_semiopen_fyle)
        ret += to_sexpr(Model.WEIGHT_ROOK_OPEN_FYLE_SYMBOL, \
        self.weight_rook_open_fyle)
        ret += to_sexpr(Model.WEIGHT_EARLY_QUEEN_STARTING_SYMBOL, \
        self.weight_early_queen_starting)
        ret += to_sexpr(Model.WEIGHT_WEAK_SQUARE_SYMBOL, \
        self.weight_weak_square)
        ret += to_sexpr(Model.WEIGHT_CASTLING_SYMBOL, \
        self.weight_castling)
        ret += to_sexpr(Model.WEIGHT_ABANDONED_CASTLING_SYMBOL, \
        self.weight_abandoned_castling)

        if self.is_runnable:
            ret += "\n;; Runs Engine.\n"
            ret += "(" + self.engine_name + " (quote @run))"

        return ret

    def SetFromCode(self, code_lines):
        engine_name = ""
        for line in code_lines:
            tup = lh.ToEngineList(line)
            if tup[0]:
                if not engine_name: tup[0]

                if tup[1] == Model.MATERIAL_SYMBOL:
                    self.material = tup[2]
                elif tup[1] == Model.ENABLE_QUIESCE_SEARCH_SYMBOL:
                    self.enable_quiesce_search = tup[2]
                elif tup[1] == Model.ENABLE_REPETITION_CHECK_SYMBOL:
                    self.enable_repetition_check = tup[2]
                elif tup[1] == Model.ENABLE_CHECK_EXTENSION_SYMBOL:
                    self.enable_check_extension = tup[2]
                elif tup[1] == Model.YBWC_LIMIT_DEPTH_SYMBOL:
                    self.ybwc_limit_depth = tup[2]
                elif tup[1] == Model.YBWC_INVALID_MOVES_SYMBOL:
                    self.ybwc_invalid_moves = tup[2]
                elif tup[1] == Model.ENABLE_ASPIRATION_WINDOWS_SYMBOL:
                    self.enable_aspiration_windows = tup[2]
                elif tup[1] == Model.ASPIRATION_WINDOWS_LIMIT_DEPTH_SYMBOL:
                    self.aspiration_windows_limit_depth = tup[2]
                elif tup[1] == Model.ASPIRATION_WINDOWS_DELTA_SYMBOL:
                    self.aspiration_windows_delta = tup[2]
                elif tup[1] == Model.ENABLE_SEE_SYMBOL:
                    self.enable_see = tup[2]
                elif tup[1] == Model.ENABLE_HISTORY_SYMBOL:
                    self.enable_history = tup[2]
                elif tup[1] == Model.ENABLE_KILLER_SYMBOL:
                    self.enable_killer = tup[2]
                elif tup[1] == Model.ENABLE_HASH_TABLE_SYMBOL:
                    self.enable_hash_table = tup[2]
                elif tup[1] == Model.ENABLE_IID_SYMBOL:
                    self.enable_iid = tup[2]
                elif tup[1] == Model.IID_LIMIT_DEPTH_SYMBOL:
                    self.iid_limit_depth = tup[2]
                elif tup[1] == Model.IID_SEARCH_DEPTH_SYMBOL:
                    self.iid_search_depth = tup[2]
                elif tup[1] == Model.ENABLE_NMR_SYMBOL:
                    self.enable_nmr = tup[2]
                elif tup[1] == Model.NMR_LIMIT_DEPTH_SYMBOL:
                    self.nmr_limit_depth = tup[2]
                elif tup[1] == Model.NMR_SEARCH_REDUCTION_SYMBOL:
                    self.nmr_search_reduction = tup[2]
                elif tup[1] == Model.NMR_REDUCTION_SYMBOL:
                    self.nmr_reduction = tup[2]
                elif tup[1] == Model.ENABLE_PROBCUT_SYMBOL:
                    self.enable_probcut = tup[2]
                elif tup[1] == Model.PROBCUT_LIMIT_DEPTH_SYMBOL:
                    self.probcut_limit_depth = tup[2]
                elif tup[1] == Model.PROBCUT_MARGIN_SYMBOL:
                    self.probcut_margin = tup[2]
                elif tup[1] == Model.PROBCUT_SEARCH_REDUCTION_SYMBOL:
                    self.probcut_search_reduction = tup[2]
                elif tup[1] == Model.ENABLE_HISTORY_PRUNING_SYMBOL:
                    self.enable_history_pruning = tup[2]
                elif tup[1] == Model.HISTORY_PRUNING_LIMIT_DEPTH_SYMBOL:
                    self.history_pruning_limit_depth = tup[2]
                elif tup[1] == Model.HISTORY_PRUNING_MOVE_THRESHOLD_SYMBOL:
                    self.history_pruning_move_threshold = tup[2]
                elif tup[1] == Model.HISTORY_PRUNING_INVALID_MOVES_SYMBOL:
                    self.history_pruning_invalid_moves = tup[2]
                elif tup[1] == Model.HISTORY_PRUNING_THRESHOLD_SYMBOL:
                    self.history_pruning_threshold = tup[2]
                elif tup[1] == Model.HISTORY_PRUNING_REDUCTION_SYMBOL:
                    self.history_pruning_reduction = tup[2]
                elif tup[1] == Model.ENABLE_LMR_SYMBOL:
                    self.enable_lmr = tup[2]
                elif tup[1] == Model.LMR_LIMIT_DEPTH_SYMBOL:
                    self.lmr_limit_depth = tup[2]
                elif tup[1] == Model.LMR_MOVE_THRESHOLD_SYMBOL:
                    self.lmr_move_threshold = tup[2]
                elif tup[1] == Model.LMR_INVALID_MOVES_SYMBOL:
                    self.lmr_invalid_moves = tup[2]
                elif tup[1] == Model.LMR_SEARCH_REDUCTION_SYMBOL:
                    self.lmr_search_reduction = tup[2]
                elif tup[1] == Model.ENABLE_FUTILITY_PRUNING_SYMBOL:
                    self.enable_futility_pruning = tup[2]
                elif tup[1] == Model.FUTILITY_PRUNING_DEPTH_SYMBOL:
                    self.futility_pruning_depth = tup[2]
                elif tup[1] == Model.FUTILITY_PRUNING_MARGIN_SYMBOL:
                    self.futility_pruning_margin = tup[2]
                elif tup[1] == Model.PAWN_SQUARE_TABLE_OPENING_SYMBOL:
                    self.pawn_square_table_opening = tup[2]
                elif tup[1] == Model.KNIGHT_SQUARE_TABLE_OPENING_SYMBOL:
                    self.knight_square_table_opening = tup[2]
                elif tup[1] == Model.BISHOP_SQUARE_TABLE_OPENING_SYMBOL:
                    self.bishop_square_table_opening = tup[2]
                elif tup[1] == Model.ROOK_SQUARE_TABLE_OPENING_SYMBOL:
                    self.rook_square_table_opening = tup[2]
                elif tup[1] == Model.QUEEN_SQUARE_TABLE_OPENING_SYMBOL:
                    self.queen_square_table_opening = tup[2]
                elif tup[1] == Model.KING_SQUARE_TABLE_OPENING_SYMBOL:
                    self.king_square_table_opening = tup[2]
                elif tup[1] == Model.PAWN_SQUARE_TABLE_ENDING_SYMBOL:
                    self.pawn_square_table_ending = tup[2]
                elif tup[1] == Model.KNIGHT_SQUARE_TABLE_ENDING_SYMBOL:
                    self.knight_square_table_ending = tup[2]
                elif tup[1] == Model.BISHOP_SQUARE_TABLE_ENDING_SYMBOL:
                    self.bishop_square_table_ending = tup[2]
                elif tup[1] == Model.ROOK_SQUARE_TABLE_ENDING_SYMBOL:
                    self.rook_square_table_ending = tup[2]
                elif tup[1] == Model.QUEEN_SQUARE_TABLE_ENDING_SYMBOL:
                    self.queen_square_table_ending = tup[2]
                elif tup[1] == Model.KING_SQUARE_TABLE_ENDING_SYMBOL:
                    self.king_square_table_ending = tup[2]
                elif tup[1] == Model.PAWN_ATTACK_TABLE_SYMBOL:
                    self.pawn_attack_table = tup[2]
                elif tup[1] == Model.KNIGHT_ATTACK_TABLE_SYMBOL:
                    self.knight_attack_table = tup[2]
                elif tup[1] == Model.BISHOP_ATTACK_TABLE_SYMBOL:
                    self.bishop_attack_table = tup[2]
                elif tup[1] == Model.ROOK_ATTACK_TABLE_SYMBOL:
                    self.rook_attack_table = tup[2]
                elif tup[1] == Model.QUEEN_ATTACK_TABLE_SYMBOL:
                    self.queen_attack_table = tup[2]
                elif tup[1] == Model.KING_ATTACK_TABLE_SYMBOL:
                    self.king_attack_table = tup[2]
                elif tup[1] == Model.PAWN_DEFENSE_TABLE_SYMBOL:
                    self.pawn_defense_table = tup[2]
                elif tup[1] == Model.KNIGHT_DEFENSE_TABLE_SYMBOL_SYMBOL:
                    self.knight_defense_table = tup[2]
                elif tup[1] == Model.BISHOP_DEFENSE_TABLE_SYMBOL:
                    self.bishop_defense_table = tup[2]
                elif tup[1] == Model.ROOK_DEFENSE_TABLE_SYMBOL:
                    self.rook_defense_table = tup[2]
                elif tup[1] == Model.QUEEN_DEFENSE_TABLE_SYMBOL:
                    self.queen_defense_table = tup[2]
                elif tup[1] == Model.KING_DEFENSE_TABLE_SYMBOL:
                    self.king_defense_table = tup[2]
                elif tup[1] == Model.PAWN_SHIELD_TABLE_SYMBOL:
                    self.pawn_shield_table = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_OPENING_POSITION_SYMBOL:
                    self.weight_pawn_opening_position = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_OPENING_POSITION_SYMBOL:
                    self.weight_knight_opening_position = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_OPENING_POSITION_SYMBOL:
                    self.weight_bishop_opening_position = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_OPENING_POSITION_SYMBOL:
                    self.weight_rook_opening_position = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_OPENING_POSITION_SYMBOL:
                    self.weight_queen_opening_position = tup[2]
                elif tup[1] == Model.WEIGHT_KING_OPENING_POSITION_SYMBOL:
                    self.weight_king_opening_position = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_ENDING_POSITION_SYMBOL:
                    self.weight_pawn_ending_position = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_ENDING_POSITION_SYMBOL:
                    self.weight_knight_ending_position = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_ENDING_POSITION_SYMBOL:
                    self.weight_bishop_ending_position = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_ENDING_POSITION_SYMBOL:
                    self.weight_rook_ending_position = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_ENDING_POSITION_SYMBOL:
                    self.weight_queen_ending_position = tup[2]
                elif tup[1] == Model.WEIGHT_KING_ENDING_POSITION_SYMBOL:
                    self.weight_king_ending_position = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_MOBILITY_SYMBOL:
                    self.weight_pawn_mobility = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_MOBILITY_SYMBOL:
                    self.weight_knight_mobility = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_MOBILITY_SYMBOL:
                    self.weight_bishop_mobility = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_MOBILITY_SYMBOL:
                    self.weight_rook_mobility = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_MOBILITY_SYMBOL:
                    self.weight_queen_mobility = tup[2]
                elif tup[1] == Model.WEIGHT_KING_MOBILITY_SYMBOL:
                    self.weight_king_mobility = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_CENTER_CONTROL_SYMBOL:
                    self.weight_pawn_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_CENTER_CONTROL_SYMBOL:
                    self.weight_knight_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_CENTER_CONTROL_SYMBOL:
                    self.weight_bishop_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_CENTER_CONTROL_SYMBOL:
                    self.weight_rook_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_CENTER_CONTROL_SYMBOL:
                    self.weight_queen_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_KING_CENTER_CONTROL_SYMBOL:
                    self.weight_king_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_SWEET_CENTER_CONTROL_SYMBOL:
                    self.weight_pawn_sweet_center_control = tup[2]
                elif tup[1] == \
                Model.WEIGHT_KNIGHT_SWEET_CENTER_CONTROL_SYMBOL:
                    self.weight_knight_sweet_center_control = tup[2]
                elif tup[1] == \
                Model.WEIGHT_BISHOP_SWEET_CENTER_CONTROL_SYMBOL:
                    self.weight_bishop_sweet_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_SWEET_CENTER_CONTROL_SYMBOL:
                    self.weight_rook_sweet_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_SWEET_CENTER_CONTROL_SYMBOL:
                    self.weight_queen_sweet_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_KING_SWEET_CENTER_CONTROL_SYMBOL:
                    self.weight_king_sweet_center_control = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_DEVELOPMENT_SYMBOL:
                    self.weight_pawn_development = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_DEVELOPMENT_SYMBOL:
                    self.weight_knight_development = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_DEVELOPMENT_SYMBOL:
                    self.weight_bishop_development = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_DEVELOPMENT_SYMBOL:
                    self.weight_rook_development = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_DEVELOPMENT_SYMBOL:
                    self.weight_queen_development = tup[2]
                elif tup[1] == Model.WEIGHT_KING_DEVELOPMENT_SYMBOL:
                    self.weight_king_development = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_ATTACK_SYMBOL:
                    self.weight_pawn_attack = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_ATTACK_SYMBOL:
                    self.weight_knight_attack = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_ATTACK_SYMBOL:
                    self.weight_bishop_attack = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_ATTACK_SYMBOL:
                    self.weight_rook_attack = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_ATTACK_SYMBOL:
                    self.weight_queen_attack = tup[2]
                elif tup[1] == Model.WEIGHT_KING_ATTACK_SYMBOL:
                    self.weight_king_attack = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_DEFENSE_SYMBOL:
                    self.weight_pawn_defense = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_DEFENSE_SYMBOL:
                    self.weight_knight_defense = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_DEFENSE_SYMBOL:
                    self.weight_bishop_defense = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_DEFENSE_SYMBOL:
                    self.weight_rook_defense = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_DEFENSE_SYMBOL:
                    self.weight_queen_defense = tup[2]
                elif tup[1] == Model.WEIGHT_KING_DEFENSE_SYMBOL:
                    self.weight_king_defense = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_ATTACK_AROUND_KING_SYMBOL:
                    self.weight_pawn_attack_around_king = tup[2]
                elif tup[1] == Model.WEIGHT_KNIGHT_ATTACK_AROUND_KING_SYMBOL:
                    self.weight_knight_attack_around_king = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_ATTACK_AROUND_KING_SYMBOL:
                    self.weight_bishop_attack_around_king = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_ATTACK_AROUND_KING_SYMBOL:
                    self.weight_rook_attack_around_king = tup[2]
                elif tup[1] == Model.WEIGHT_QUEEN_ATTACK_AROUND_KING_SYMBOL:
                    self.weight_queen_attack_around_king = tup[2]
                elif tup[1] == Model.WEIGHT_KING_ATTACK_AROUND_KING_SYMBOL:
                    self.weight_king_attack_around_king = tup[2]
                elif tup[1] == Model.WEIGHT_PASS_PAWN_SYMBOL:
                    self.weight_pass_pawn = tup[2]
                elif tup[1] == Model.WEIGHT_PROTECTED_PASS_PAWN_SYMBOL:
                    self.weight_protected_pass_pawn = tup[2]
                elif tup[1] == Model.WEIGHT_DOUBLE_PAWN_SYMBOL:
                    self.weight_double_pawn = tup[2]
                elif tup[1] == Model.WEIGHT_ISO_PAWN_SYMBOL:
                    self.weight_iso_pawn = tup[2]
                elif tup[1] == Model.WEIGHT_PAWN_SHIELD_SYMBOL:
                    self.weight_pawn_shield = tup[2]
                elif tup[1] == Model.WEIGHT_BISHOP_PAIR_SYMBOL:
                    self.weight_bishop_pair = tup[2]
                elif tup[1] == Model.WEIGHT_BAD_BISHOP_SYMBOL:
                    self.weight_bad_bishop = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_PAIR_SYMBOL:
                    self.weight_rook_pair = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_SEMIOPEN_FYLE_SYMBOL:
                    self.weight_rook_semiopen_fyle = tup[2]
                elif tup[1] == Model.WEIGHT_ROOK_OPEN_FYLE_SYMBOL:
                    self.weight_rook_open_fyle = tup[2]
                elif tup[1] == Model.WEIGHT_EARLY_QUEEN_STARTING_SYMBOL:
                    self.weight_early_queen_starting = tup[2]
                elif tup[1] == Model.WEIGHT_WEAK_SQUARE_SYMBOL:
                    self.weight_weak_square = tup[2]
                elif tup[1] == Model.WEIGHT_CASTLING_SYMBOL:
                    self.weight_castling = tup[2]
                elif tup[1] == Model.WEIGHT_ABANDONED_CASTLING_SYMBOL:
                    self.weight_abandoned_castling = tup[2]

        if engine_name: self.engine_name = engine_name

    def SaveSayulisp(self, filename):
        try:
            f = open(filename, "w")
            f.write(self.GenSayulispCode())
            f.close()
        except:
            tkm.showerror("Error", "Couldn't Open '" + filename + "'")

    def LoadSayulisp(self, filename):
        try:
            f = open(filename, "r")
            self.SetFromCode(f.readlines())
            f.close
        except:
            tkm.showerror("Error", "Couldn't Open '" + filename + "'")

    def __init__(self):
        self.is_generating = True
        self.engine_name = "my-engine"
        self.is_runnable = True

        # Load default_params.scm.
        default_filename = os.path.dirname(GetFullFileName(__file__)) \
        + "/default_params.scm"

        self.LoadSayulisp(default_filename)

if __name__ == "__main__":
    pass
