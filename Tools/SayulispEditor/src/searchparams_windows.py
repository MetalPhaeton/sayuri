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

"""Search Params Windows."""

import sys, os
import tkinter as tk
from src.common import *
from src.model import *
from src.base_window import *
from src.mywidget import *
from src.board_frame import *

class MaterialConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Material")
        self.model = model

        data = self.model.material

        # Set widget.
        self.entries = []

        entry = MyEntry(self.content_frame, "Pawn")
        entry.Set(data[1])
        entry.pack(anchor=tk.E)
        self.entries += [entry]

        entry = MyEntry(self.content_frame, "Knight")
        entry.Set(data[2])
        entry.pack(anchor=tk.E)
        self.entries += [entry]

        entry = MyEntry(self.content_frame, "Bishop")
        entry.Set(data[3])
        entry.pack(anchor=tk.E)
        self.entries += [entry]

        entry = MyEntry(self.content_frame, "Rook")
        entry.Set(data[4])
        entry.pack(anchor=tk.E)
        self.entries += [entry]

        entry = MyEntry(self.content_frame, "Queen")
        entry.Set(data[5])
        entry.pack(anchor=tk.E)
        self.entries += [entry]

        entry = MyEntry(self.content_frame, "King")
        entry.Set(data[6])
        entry.pack(anchor=tk.E)
        self.entries += [entry]

    def OnOk(self):
        data = [0.0]
        for entry in self.entries:
            data += [entry.GetFloat()]
        self.model.material = data

        self.OnCancel()

class QuiesceSearchConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Quiescence Search")
        self.model = model

        self.btn = \
        MyCheckbutton(self.content_frame, "Enable Quiescence Search")
        self.btn.Set(self.model.enable_quiesce_search)
        self.btn.pack()

    def OnOk(self):
        self.model.enable_quiesce_search = self.btn.Get()
        self.OnCancel()

class RepetitionCheckConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Repetition Check")
        self.model = model

        self.btn = \
        MyCheckbutton(self.content_frame, "Enable Repetition Check")
        self.btn.Set(self.model.enable_repetition_check)
        self.btn.pack()

    def OnOk(self):
        self.model.enable_repetition_check = self.btn.Get()
        self.OnCancel()

class CheckExtensionConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Check Extension")
        self.model = model

        self.btn = \
        MyCheckbutton(self.content_frame, "Enable Check Extension")
        self.btn.Set(self.model.enable_check_extension)
        self.btn.pack()

    def OnOk(self):
        self.model.enable_check_extension = self.btn.Get()
        self.OnCancel()

class YBWCConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Check Extension")
        self.model = model

        entry = MyEntry(self.content_frame, "Depth of limit")
        entry.Set(int(self.model.ybwc_limit_depth))
        entry.pack(anchor=tk.E)
        self.limit_depth_entry = entry

        entry = MyEntry(self.content_frame, "Number of moves that void YBWC")
        entry.Set(int(self.model.ybwc_invalid_moves))
        entry.pack(anchor=tk.E)
        self.invalid_moves_entry = entry

    def OnOk(self):
        self.model.ybwc_limit_depth = float(self.limit_depth_entry.GetInt())

        self.model.ybwc_invalid_moves = \
        float(self.invalid_moves_entry.GetInt())

        self.OnCancel()

class AspirationWindowsConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Aspiration Windows")
        self.model = model

        self.enable_btn = \
        MyCheckbutton(self.content_frame, "Enable Aspiration Windows")
        self.enable_btn.Set(model.enable_aspiration_windows)
        self.enable_btn.pack()

        entry = MyEntry(self.content_frame, "Depth of limit")
        entry.Set(int(self.model.aspiration_windows_limit_depth))
        entry.pack(anchor=tk.E)
        self.limit_depth_entry = entry

        entry = MyEntry(self.content_frame, "Delta of increase")
        entry.Set(int(self.model.aspiration_windows_delta))
        entry.pack(anchor=tk.E)
        self.delta_entry = entry

    def OnOk(self):
        self.model.enable_aspiration_windows = self.enable_btn.Get()

        self.model.aspiration_windows_limit_depth = \
        float(self.limit_depth_entry.GetInt())

        self.model.aspiration_windows_delta = float(self.delta_entry.GetInt())

        self.OnCancel()

class SEEConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "SEE")
        self.model = model

        self.btn = \
        MyCheckbutton(self.content_frame, "Enable SEE")
        self.btn.Set(self.model.enable_see)
        self.btn.pack()

    def OnOk(self):
        self.model.enable_see = self.btn.Get()
        self.OnCancel()

class HistoryConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "History Heuristics")
        self.model = model

        self.btn = \
        MyCheckbutton(self.content_frame, "Enable History Heuristics")
        self.btn.Set(self.model.enable_history)
        self.btn.pack()

    def OnOk(self):
        self.model.enable_history = self.btn.Get()
        self.OnCancel()

class KillerConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Killer Move Heuristics")
        self.model = model

        self.btn = \
        MyCheckbutton(self.content_frame, "Enable Killer Move Heuristics")
        self.btn.Set(self.model.enable_killer)
        self.btn.pack()

    def OnOk(self):
        self.model.enable_killer = self.btn.Get()
        self.OnCancel()

class TranspositionTableConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Transposition Table")
        self.model = model

        self.btn = \
        MyCheckbutton(self.content_frame, "Enable Transposition Table")
        self.btn.Set(self.model.enable_hash_table)
        self.btn.pack()

    def OnOk(self):
        self.model.enable_hash_table = self.btn.Get()
        self.OnCancel()

class IIDConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Internal Iterative Deepening")
        self.model = model

        self.enable_btn = MyCheckbutton(self.content_frame, \
        "Enable Internal Iterative Deepening")
        self.enable_btn.Set(self.model.enable_iid)
        self.enable_btn.pack()

        entry = MyEntry(self.content_frame, "Depth of limit")
        entry.Set(int(self.model.iid_limit_depth))
        entry.pack(anchor=tk.E)
        self.limit_depth_entry = entry

        entry = MyEntry(self.content_frame, "Search depth")
        entry.Set(int(self.model.iid_search_depth))
        entry.pack(anchor=tk.E)
        self.search_depth_entry = entry

    def OnOk(self):
        self.model.enable_iid = self.enable_btn.Get()

        self.model.iid_limit_depth = float(self.limit_depth_entry.GetInt())

        self.model.iid_search_depth = float(self.search_depth_entry.GetInt())

        self.OnCancel()

class NMRConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Null Move Reduction")
        self.model = model

        self.enable_btn = MyCheckbutton(self.content_frame, \
        "Enable Null Move Reduction")
        self.enable_btn.Set(self.model.enable_nmr)
        self.enable_btn.pack()

        entry = MyEntry(self.content_frame, "Depth of limit")
        entry.Set(int(self.model.nmr_limit_depth))
        entry.pack(anchor=tk.E)
        self.limit_depth_entry = entry

        entry = MyEntry(self.content_frame, "Reduction For Null Move Search")
        entry.Set(int(self.model.nmr_search_reduction))
        entry.pack(anchor=tk.E)
        self.search_reduction_entry = entry

        entry = MyEntry(self.content_frame, "Reduction")
        entry.Set(int(self.model.nmr_reduction))
        entry.pack(anchor=tk.E)
        self.reduction_entry = entry

    def OnOk(self):
        self.model.enable_nmr = self.enable_btn.Get()

        self.model.nmr_limit_depth = float(self.limit_depth_entry.GetInt())

        self.model.nmr_search_reduction = \
        float(self.search_reduction_entry.GetInt())

        self.model.nmr_reduction = \
        float(self.reduction_entry.GetInt())

        self.OnCancel()

class ProbCutConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "ProbCut")
        self.model = model

        self.enable_btn = MyCheckbutton(self.content_frame, \
        "Enable ProbCut")
        self.enable_btn.Set(self.model.enable_probcut)
        self.enable_btn.pack()

        entry = MyEntry(self.content_frame, "Depth of limit")
        entry.Set(int(self.model.probcut_limit_depth))
        entry.pack(anchor=tk.E)
        self.limit_depth_entry = entry

        entry = MyEntry(self.content_frame, "Margin of Beta")
        entry.Set(int(self.model.probcut_margin))
        entry.pack(anchor=tk.E)
        self.margin_entry = entry

        entry = MyEntry(self.content_frame, "Reduction for search")
        entry.Set(int(self.model.probcut_search_reduction))
        entry.pack(anchor=tk.E)
        self.search_reduction_entry = entry

    def OnOk(self):
        self.model.enable_probcut = self.enable_btn.Get()

        self.model.probcut_limit_depth = float(self.limit_depth_entry.GetInt())

        self.model.probcut_margin = float(self.margin_entry.GetInt())

        self.model.probcut_search_reduction = \
        float(self.search_reduction_entry.GetInt())

        self.OnCancel()

class HistoryPruningConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "History Pruning")
        self.model = model

        self.enable_btn = MyCheckbutton(self.content_frame, \
        "Enable History Pruning")
        self.enable_btn.Set(self.model.enable_history_pruning)
        self.enable_btn.pack()

        entry = MyEntry(self.content_frame, "Depth of limit")
        entry.Set(int(self.model.history_pruning_limit_depth))
        entry.pack(anchor=tk.E)
        self.limit_depth_entry = entry

        entry = MyEntry(self.content_frame, \
        "Probability of moves that void History Pruning (0.0 ... 1.0)")
        entry.Set(self.model.history_pruning_move_threshold)
        entry.pack(anchor=tk.E)
        self.move_threshold_entry = entry

        entry = MyEntry(self.content_frame, \
        "Number of moves that void History Pruning")
        entry.Set(int(self.model.history_pruning_invalid_moves))
        entry.pack(anchor=tk.E)
        self.invalid_moves_entry = entry

        entry = MyEntry(self.content_frame, \
        "Probability of history value that void History Pruning (0.0 ... 1.0)")
        entry.Set(self.model.history_pruning_threshold)
        entry.pack(anchor=tk.E)
        self.threshold_entry = entry

        entry = MyEntry(self.content_frame, "Reduction")
        entry.Set(int(self.model.history_pruning_reduction))
        entry.pack(anchor=tk.E)
        self.reduction_entry = entry

    def OnOk(self):
        self.model.enable_history_pruning = self.enable_btn.Get()

        self.model.history_pruning_limit_depth = \
        float(self.limit_depth_entry.GetInt())

        self.model.history_pruning_move_threshold = \
        self.move_threshold_entry.GetFloat()

        self.model.history_pruning_invalid_moves = \
        float(self.invalid_moves_entry.GetInt())

        self.model.history_pruning_threshold = \
        self.threshold_entry.GetFloat()

        self.model.history_pruning_reduction = \
        float(self.reduction_entry.GetInt())

        self.OnCancel()

class LMRConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Late Move Reduction")
        self.model = model

        self.enable_btn = MyCheckbutton(self.content_frame, \
        "Enable Late Move Reduction")
        self.enable_btn.Set(self.model.enable_lmr)
        self.enable_btn.pack()

        entry = MyEntry(self.content_frame, "Depth of limit")
        entry.Set(int(self.model.lmr_limit_depth))
        entry.pack(anchor=tk.E)
        self.limit_depth_entry = entry

        entry = MyEntry(self.content_frame, \
        "Probability of moves that void Late Move Reduction (0.0 ... 1.0)")
        entry.Set(self.model.lmr_move_threshold)
        entry.pack(anchor=tk.E)
        self.move_threshold_entry = entry

        entry = MyEntry(self.content_frame, \
        "Number of moves that void Late Move Redcution")
        entry.Set(int(self.model.lmr_invalid_moves))
        entry.pack(anchor=tk.E)
        self.invalid_moves_entry = entry

        entry = MyEntry(self.content_frame, "Reduction for search")
        entry.Set(int(self.model.lmr_search_reduction))
        entry.pack(anchor=tk.E)
        self.search_reduction_entry = entry

    def OnOk(self):
        self.model.enable_lmr = self.enable_btn.Get()

        self.model.lmr_limit_depth = \
        float(self.limit_depth_entry.GetInt())

        self.model.lmr_move_threshold = \
        self.move_threshold_entry.GetFloat()

        self.model.lmr_invalid_moves = \
        float(self.invalid_moves_entry.GetInt())

        self.model.lmr_search_reduction = \
        float(self.search_reduction_entry.GetInt())

        self.OnCancel()

class FutilityPruningConfig(BaseWindow):
    def __init__(self, master, model):
        BaseWindow.__init__(self, master, "Futility Pruning")
        self.model = model

        self.enable_btn = MyCheckbutton(self.content_frame, \
        "Enable Futility Pruning")
        self.enable_btn.Set(self.model.enable_futility_pruning)
        self.enable_btn.pack()

        entry = MyEntry(self.content_frame, "Depth")
        entry.Set(int(self.model.futility_pruning_depth))
        entry.pack(anchor=tk.E)
        self.depth_entry = entry

        entry = MyEntry(self.content_frame, "Margin of Alpha")
        entry.Set(int(self.model.futility_pruning_margin))
        entry.pack(anchor=tk.E)
        self.margin_entry = entry

    def OnOk(self):
        self.model.enable_futility_pruning = self.enable_btn.Get()

        self.model.futility_pruning_depth = \
        float(self.depth_entry.GetInt())

        self.model.futility_pruning_margin = \
        float(self.margin_entry.GetInt())

        self.OnCancel()

if __name__ == "__main__":
    pass
