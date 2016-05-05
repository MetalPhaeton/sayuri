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

"""Main Frame"""

import sys, os
import tkinter as tk
import tkinter.filedialog as tkfd

from src.common import *
from src.model import *
from src.base_window import *
from src.mywidget import *
from src.linegraph_frame import *
from src.searchparams_windows import *
from src.evalparams_windows import *

class ScrollingFrame(tk.LabelFrame):
    def __init__(self, master, text):
        tk.LabelFrame.__init__(self, master, text=text)

        self.canvas = tk.Canvas(self, bg="#ffffff", borderwidth=0, \
        highlightthickness=0)

        self.BindMouseWheel(self.canvas)

        scroll = tk.Scrollbar(self, orient=tk.VERTICAL, \
        command=self.canvas.yview)
        self.canvas["yscrollcommand"] = scroll.set
        scroll.pack(side=tk.RIGHT, fill=tk.Y, padx=0)

        self.canvas.pack()

    def SetFrame(self, frame, width, height):
        self.canvas.create_window(0, 0, window=frame, anchor=tk.NW)

        def OnConfigure(event):
            self.canvas["scrollregion"] = self.canvas.bbox(tk.ALL)
            self.canvas.configure(width=width, height=height)
        frame.bind("<Configure>", OnConfigure)

    def BindMouseWheel(self, widget):
        widget.bind("<MouseWheel>", \
        lambda e: self.canvas.yview_scroll(1, tk.UNITS))
        widget.bind("<Button-4>", \
        lambda e: self.canvas.yview_scroll(-1, tk.UNITS))
        widget.bind("<Button-5>", \
        lambda e: self.canvas.yview_scroll(1, tk.UNITS))

class MainFrame(tk.Frame):
    def GenSearchParamsFrame(self, master):
        ret = ScrollingFrame(master, "Search Algorithm")
        master.search_parent = tk.Frame(ret, bg="#ffffff")
        ret.BindMouseWheel(master.search_parent)
        ret.SetFrame(master.search_parent, 300, 300)

        # Function to Add item
        def AddWindow(text, constructor):
            item_frame = tk.Frame(master.search_parent, bg="#ffffff")
            ret.BindMouseWheel(item_frame)

            label = tk.Label(item_frame, text=text, bg="#ffffff")
            ret.BindMouseWheel(label)
            label.pack(side=tk.LEFT, **PACK_PARAMS)

            btn = tk.Button(item_frame, text="Configure", \
            command=lambda: constructor(master, self.model))
            ret.BindMouseWheel(btn)
            btn.pack(side=tk.LEFT, **PACK_PARAMS)

            item_frame.pack(anchor=tk.E, **PACK_PARAMS)

        # Add items.
        AddWindow("Material", MaterialConfig)
        AddWindow("Quiescence Search", QuiesceSearchConfig)
        AddWindow("Repetition Check", RepetitionCheckConfig)
        AddWindow("Check Extension", CheckExtensionConfig)
        AddWindow("YBWC", YBWCConfig)
        AddWindow("Aspiration Windows", AspirationWindowsConfig)
        AddWindow("SEE", SEEConfig)
        AddWindow("History Heuristics", HistoryConfig)
        AddWindow("Killer Move Heuristics", KillerConfig)
        AddWindow("Transposition Table", TranspositionTableConfig)
        AddWindow("Internal Iterative Deepening", IIDConfig)
        AddWindow("Null Move Reduction", NMRConfig)
        AddWindow("ProbCut", ProbCutConfig)
        AddWindow("History Pruning", HistoryPruningConfig)
        AddWindow("Late Move Reduction", LMRConfig)
        AddWindow("Futility Pruning", FutilityPruningConfig)

        return ret

    def GenEvalParamsFrame(self, master):
        ret = ScrollingFrame(master, "Evaluation Function")
        master.eval_parent = tk.Frame(ret, bg="#ffffff")
        ret.BindMouseWheel(master.eval_parent)
        ret.SetFrame(master.eval_parent, 300, 300)

        def AddWindow(text, constructor):
            item_frame = tk.Frame(master.eval_parent, bg="#ffffff")
            ret.BindMouseWheel(item_frame)

            label = tk.Label(item_frame, text=text, bg="#ffffff")
            ret.BindMouseWheel(label)
            label.pack(side=tk.LEFT, **PACK_PARAMS)

            btn = tk.Button(item_frame, text="Configure", \
            command=lambda: constructor(master, self.model))
            ret.BindMouseWheel(btn)
            btn.pack(side=tk.LEFT, **PACK_PARAMS)

            item_frame.pack(anchor=tk.E, **PACK_PARAMS)

        AddWindow("Pawn Opening Position", PawnOpeningPositionConfig)
        AddWindow("Knight Opening Position", KnightOpeningPositionConfig)
        AddWindow("Bishop Opening Position", BishopOpeningPositionConfig)
        AddWindow("Rook Opening Position", RookOpeningPositionConfig)
        AddWindow("Queen Opening Position", QueenOpeningPositionConfig)
        AddWindow("King Opening Position", KingOpeningPositionConfig)

        AddWindow("Pawn Ending Position", PawnEndingPositionConfig)
        AddWindow("Knight Ending Position", KnightEndingPositionConfig)
        AddWindow("Bishop Ending Position", BishopEndingPositionConfig)
        AddWindow("Rook Ending Position", RookEndingPositionConfig)
        AddWindow("Queen Ending Position", QueenEndingPositionConfig)
        AddWindow("King Ending Position", KingEndingPositionConfig)

        AddWindow("Pawn Mobility", PawnMobilityConfig)
        AddWindow("Knight Mobility", KnightMobilityConfig)
        AddWindow("Bishop Mobility", BishopMobilityConfig)
        AddWindow("Rook Mobility", RookMobilityConfig)
        AddWindow("Queen Mobility", QueenMobilityConfig)
        AddWindow("King Mobility", KingMobilityConfig)

        AddWindow("Pawn Center Control", PawnCenterControlConfig)
        AddWindow("Knight Center Control", KnightCenterControlConfig)
        AddWindow("Bishop Center Control", BishopCenterControlConfig)
        AddWindow("Rook Center Control", RookCenterControlConfig)
        AddWindow("Queen Center Control", QueenCenterControlConfig)
        AddWindow("King Center Control", KingCenterControlConfig)

        AddWindow("Pawn Sweet Center Control", PawnSweetCenterControlConfig)
        AddWindow("Knight Sweet Center Control", KnightSweetCenterControlConfig)
        AddWindow("Bishop Sweet Center Control", BishopSweetCenterControlConfig)
        AddWindow("Rook Sweet Center Control", RookSweetCenterControlConfig)
        AddWindow("Queen Sweet Center Control", QueenSweetCenterControlConfig)
        AddWindow("King Sweet Center Control", KingSweetCenterControlConfig)

        AddWindow("Pawn Development", PawnDevelopmentConfig)
        AddWindow("Knight Development", KnightDevelopmentConfig)
        AddWindow("Bishop Development", BishopDevelopmentConfig)
        AddWindow("Rook Development", RookDevelopmentConfig)
        AddWindow("Queen Development", QueenDevelopmentConfig)
        AddWindow("King Development", KingDevelopmentConfig)

        AddWindow("Pawn Attack", PawnAttackConfig)
        AddWindow("Knight Attack", KnightAttackConfig)
        AddWindow("Bishop Attack", BishopAttackConfig)
        AddWindow("Rook Attack", RookAttackConfig)
        AddWindow("Queen Attack", QueenAttackConfig)
        AddWindow("King Attack", KingAttackConfig)

        AddWindow("Pawn Defense", PawnDefenseConfig)
        AddWindow("Knight Defense", KnightDefenseConfig)
        AddWindow("Bishop Defense", BishopDefenseConfig)
        AddWindow("Rook Defense", RookDefenseConfig)
        AddWindow("Queen Defense", QueenDefenseConfig)
        AddWindow("King Defense", KingDefenseConfig)

        AddWindow("Bishop Pin", BishopPinConfig)
        AddWindow("Rook Pin", RookPinConfig)
        AddWindow("Queen Pin", QueenPinConfig)

        AddWindow("Pawn Attack around King", PawnAttackAroundKingConfig)
        AddWindow("Knight Attack around King", KnightAttackAroundKingConfig)
        AddWindow("Bishop Attack around King", BishopAttackAroundKingConfig)
        AddWindow("Rook Attack around King", RookAttackAroundKingConfig)
        AddWindow("Queen Attack around King", QueenAttackAroundKingConfig)
        AddWindow("King Attack around King", KingAttackAroundKingConfig)

        AddWindow("Pass Pawn", PassPawnConfig)
        AddWindow("Protected Pass Pawn", ProtectedPassPawnConfig)
        AddWindow("Double Pawn", DoublePawnConfig)
        AddWindow("Isolated Pawn", IsoPawnConfig)
        AddWindow("Pawn Shield", PawnShieldConfig)

        AddWindow("Bishop Pair", BishopPairConfig)
        AddWindow("Bad Bishop", BadBishopConfig)

        AddWindow("Rook Pair", RookPairConfig)
        AddWindow("Rook Semi-Open File", RookSemiOpenFyleConfig)
        AddWindow("Rook Open File", RookOpenFyleConfig)

        AddWindow("Queen Starting Too Early", EarlyQueenStartingConfig)
        AddWindow("Weak Square around King", WeakSquareConfig)
        AddWindow("Castling", CastlingConfig)
        AddWindow("Abandoned Castling Rights", AbandonedCastlingConfig)

        return ret

    def Import(self):
        filename = tkfd.askopenfilename(parent=self, title="Import Sayulisp")
        if filename and (type(filename) == str):
            self.model.LoadSayulisp(filename)
            self.engine_name_entry.Set(self.model.engine_name)
            self.is_generating_btn.Set(self.model.is_generating)
            self.is_runnable_btn.Set(self.model.is_runnable)

    def Export(self):
        filename = tkfd.asksaveasfilename(parent=self, title="Export Sayulisp")
        if filename and (type(filename) == str):
            self.model.engine_name = self.engine_name_entry.Get()
            self.model.is_generating = self.is_generating_btn.Get()
            self.model.is_runnable = self.is_runnable_btn.Get()
            self.model.SaveSayulisp(filename)

    def GenTopFrame(self, master):
        ret = tk.Frame(master)

        self.GenSearchParamsFrame(ret).pack(side=tk.LEFT, **PACK_PARAMS)
        self.GenEvalParamsFrame(ret).pack(side=tk.LEFT, **PACK_PARAMS)

        return ret
    def GenMiddleFrame(self, master):
        ret = tk.Frame(master)

        # Engine name.
        self.engine_name_entry = MyEntry(ret, "Engine Name")
        self.engine_name_entry.Set(self.model.engine_name)
        self.engine_name_entry.pack(anchor=tk.W, **PACK_PARAMS)

        # Generate code: (define <Engine name> (gen-engine))
        self.is_generating_btn = MyCheckbutton(ret, \
        "Generate code '(define <Engine name> (gen-engine))' on top.")
        self.is_generating_btn.Set(self.model.is_generating)
        self.is_generating_btn.pack(anchor=tk.W, **PACK_PARAMS)

        # Runnable.
        self.is_runnable_btn = MyCheckbutton(ret, "Runnable")
        self.is_runnable_btn.Set(self.model.is_runnable)
        self.is_runnable_btn.pack(anchor=tk.W, **PACK_PARAMS)

        return ret

    def GenBottomFrame(self, master):
        ret = tk.Frame(master)

        # Import Button.
        tk.Button(ret, text="Import", command=self.Import).\
        pack(side=tk.LEFT, **PACK_PARAMS)

        # Export Button.
        tk.Button(ret, text="Export", command=self.Export).\
        pack(side=tk.LEFT, **PACK_PARAMS)

        return ret
    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.model = Model()

        self.GenTopFrame(self).pack(**PACK_PARAMS)
        self.GenMiddleFrame(self).pack(**PACK_PARAMS)
        self.GenBottomFrame(self).pack(**PACK_PARAMS)

if __name__ == "__main__":
    pass
