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

"""My Widget."""

import sys,os
import tkinter as tk
import tkinter.messagebox as tkm
from src.common import *

class MyStringVar(tk.StringVar):
    def __init__(self):
        tk.StringVar.__init__(self)

    def GetInt(self):
        try:
            return int(self.get())
        except:
            tkm.showerror("Error", "'" + self.get()
            + "'couldn't convert to integer.")
            return None

    def GetFloat(self):
        try:
            return float(self.get())
        except:
            tkm.showerror("Error", "'" + self.get()
            + "'couldn't convert to float.")
            return None

    def Set(self, value):
        self.set(str(value))

class MyEntry(tk.Frame):
    def __init__(self, master, text):
        tk.Frame.__init__(self, master)

        # Label.
        tk.Label(self, text=text).pack(side=tk.LEFT, **PACK_PARAMS)

        # Entry.
        self.variable = MyStringVar()
        tk.Entry(self, textvariable=self.variable, width=10).\
        pack(side=tk.LEFT, **PACK_PARAMS)

    def Get(self): return self.variable.get()
    def GetInt(self): return self.variable.GetInt()
    def GetFloat(self): return self.variable.GetFloat()
    def Set(self, value): return self.variable.Set(value)

class MyCheckbutton(tk.Frame):
    def __init__(self, master, text):
        tk.Frame.__init__(self, master)

        # Check Button.
        self.variable = tk.IntVar()
        tk.Checkbutton(self, text=text, variable=self.variable).\
        pack(**PACK_PARAMS)

    def Get(self): return True if self.variable.get() else False
    def Set(self, value):
        if value: self.variable.set(1)
        else: self.variable.set(0)

if __name__ == "__main__":
    pass
