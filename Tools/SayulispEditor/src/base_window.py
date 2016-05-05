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

"""Base Window."""

import sys, os
import tkinter as tk
from src.common import *

class BaseWindow(tk.Toplevel):
    def __init__(self, master, title):
        tk.Toplevel.__init__(self, master)
        self.resizable(False, False)
        self.title(title)
        self.content_frame = tk.Frame(self)
        self.content_frame.pack(**PACK_PARAMS)

        frame = tk.Frame(self)

        tk.Button(frame, text="OK", command=self.OnOk).pack(side=tk.LEFT)

        tk.Button(frame, text="Cancel", command=self.OnCancel)\
        .pack(side=tk.LEFT)

        frame.pack(**PACK_PARAMS)

    def OnOk(self):
        self.OnCancel()

    def OnCancel(self):
        self.destroy()

if __name__ == "__main__":
    pass
