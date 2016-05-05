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

"""My Board Frame."""

import sys, os
import tkinter as tk
from src.common import *
from src.mywidget import *

class BoardFrame(tk.LabelFrame):
    LIGHT_COLOR = "#bb7777"
    DARK_COLOR = "#770000"
    def Get(self, index):
        if 0 <= index <= 63:
            return self.variables[index].GetFloat()
        return None

    def GetData(self):
        ret = []
        for elm in self.variables: ret += [elm.GetFloat()]
        return ret

    def __init__(self, master, data):
        tk.LabelFrame.__init__(self, master, text="Square Value")

        tk.Label(self, text="Southeast corner is A1(A8 for Black).\n" \
        + " Northwest corner is H8(H1 for Black).").pack()

        frame = tk.Frame(self)
        self.variables = []
        for i in range(64):
            col = i % 8
            row = 7 - (i // 8)
            color = ""
            if (row % 2) == 1:
                if (col % 2) == 0: color = BoardFrame.DARK_COLOR
                else: color = BoardFrame.LIGHT_COLOR
            else:
                if (col % 2) == 0: color = BoardFrame.LIGHT_COLOR
                else: color = BoardFrame.DARK_COLOR

            self.variables += [MyStringVar()]
            self.variables[-1].Set(data[i])

            canvas = tk.Canvas(frame, bg=color, width=60, height=60, \
            borderwidth=0, highlightthickness=0)

            entry = tk.Entry(canvas, textvariable=self.variables[-1], width=4)

            canvas.create_window(7, 20, window=entry, anchor=tk.NW, \
            width=46, height=20)

            canvas.grid(row=row, column=col, padx=0, pady=0, \
            ipadx=0, ipady=0)
        frame.pack(**PACK_PARAMS)

if __name__ == "__main__":
    pass
