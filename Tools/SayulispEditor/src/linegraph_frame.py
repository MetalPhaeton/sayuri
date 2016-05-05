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

"""Tkinter Frame of line graph."""

import sys,os
import tkinter as tk
from src.common import *
from src.mywidget import *

class LineGraphFrame(tk.LabelFrame):
    GRAPH_WIDTH = 300
    GRAPH_HEIGHT = 200
    REDUCTION_WIDTH =25 
    REDUCTION_HEIGHT =25 
    CANVAS_COLOR = "#ffffff"
    LINE_COLOR = "#000000"
    DOT_COLOR = "#ff0000"
    DOT_RADIUS = 4

    def DataVariablesToFloatList(self):
        ret = []
        for elm in self.data_variables:
            ret += [[elm[0].GetInt(), elm[1].GetFloat()]]
            if (ret[-1][0] == None) or (ret[-1][1] == None): return None
        return ret

    def SortDataVariables(self):
        list_data = self.DataVariablesToFloatList()
        if not list_data: return

        list_data.sort(key=lambda x: x[0])

        for i in range(len(self.data_variables)):
            self.data_variables[i][0].Set(list_data[i][0])
            self.data_variables[i][1].Set(list_data[i][1])

    # line_data -> self.data_variables
    # [[float, float], ...] -> [[MyStringVar, MyStringVar], ...]
    def InitDataVariables(self, line_data):
        self.data_variables = []

        if not line_data: return
        for data in line_data:
            if len(data) >= 2:
                if type(data[0]) == float: data[0] = int(data[0])
                if type(data[1]) == int: data[1] = float(data[1])

                if (type(data[0]) == int) and (type(data[1]) == float):
                    temp = [MyStringVar(), MyStringVar()]
                    temp[0].Set(data[0])
                    temp[1].Set(data[1])
                    self.data_variables += [temp]

    # var : [MyStringVar, MyStringVar]
    def GenListItem(self, master, var):
        ret = tk.Frame(master)

        num_pieces_entry = tk.Entry(ret, width=10, textvariable=var[0])
        weight_entry = tk.Entry(ret, width=10, textvariable=var[1])

        num_pieces_entry.pack(side=tk.LEFT)
        weight_entry.pack(side=tk.LEFT)

        # Delete Button
        def command():
            nonlocal self
            nonlocal ret
            nonlocal var

            # Delete from self.data_variables.
            self.data_variables.remove(var)

            # Update.
            self.update_btn.invoke()

            # Delete from frame.
            ret.destroy()
        delete_btn = tk.Button(ret, text="Delete", command=command)
        delete_btn.pack(side=tk.LEFT)

        return ret

    def GenGraph(self, master):
        ret = tk.Frame(master)

        canvas = tk.Canvas(ret, bg=LineGraphFrame.CANVAS_COLOR, \
        width=LineGraphFrame.GRAPH_WIDTH, \
        height=LineGraphFrame.GRAPH_HEIGHT, \
        borderwidth=1, relief="solid", \
        highlightthickness=0)

        line_data = self.DataVariablesToFloatList()
        if not line_data:
            canvas.pack(**PACK_PARAMS)
            return ret
        if len(line_data) <= 0:
            canvas.pack(**PACK_PARAMS)
            return ret

        x = LineGraphFrame.REDUCTION_WIDTH
        y = LineGraphFrame.REDUCTION_HEIGHT
        width = LineGraphFrame.GRAPH_WIDTH - (2 * x)
        height = LineGraphFrame.GRAPH_HEIGHT - (2 * y)

        # Look up max and min.
        max_num_pieces = line_data[0][0]
        min_num_pieces = line_data[0][0]
        max_weight = line_data[0][1]
        min_weight = line_data[0][1]
        for elm in line_data:
            if elm[0] > max_num_pieces: max_num_pieces = elm[0]
            if elm[0] < min_num_pieces: min_num_pieces = elm[0]
            if elm[1] > max_weight: max_weight = elm[1]
            if elm[1] < min_weight: min_weight = elm[1]

        # Calculate scales.
        scale_num_pieces = width
        scale_weight = height
        if len(line_data) >= 2:
            delta_num_pieces = max_num_pieces - min_num_pieces
            if delta_num_pieces <= 0: delta_num_pieces = 1
            scale_num_pieces = width / delta_num_pieces

            delta_weight = max_weight - min_weight
            if delta_weight <= 0: delta_weight = 1
            scale_weight = height / delta_weight


        # Calculate coordination.
        for elm in line_data:
            elm[0] = int((scale_num_pieces * (elm[0] - min_num_pieces)) + x)
            elm[1] = int((scale_weight * (elm[1] - min_weight)) + y)
            elm[1] = (-1 * elm[1]) + LineGraphFrame.GRAPH_HEIGHT

        # Draw lines on canvas.
        for i in range(1, len(line_data)):
            canvas.create_line(line_data[i - 1][0], line_data[i - 1][1], \
            line_data[i][0], line_data[i][1], \
            fill=LineGraphFrame.LINE_COLOR, width=2.0)

        # Draw oval on canvas.
        dot_col = LineGraphFrame.DOT_COLOR
        dot_radius = LineGraphFrame.DOT_RADIUS
        for elm in line_data:
            canvas.create_oval(elm[0] - dot_radius, elm[1] - \
            dot_radius,elm[0] + dot_radius, elm[1] + dot_radius, \
            fill=dot_col, width=0)

        canvas.pack(**PACK_PARAMS)

        # Display left and right.
        lr_frame = tk.Frame(ret)

        tk.Label(lr_frame, text="|<- Ending").\
        pack(side=tk.LEFT, anchor=tk.W, **PACK_PARAMS)

        tk.Label(lr_frame, text="Opening ->|").\
        pack(side=tk.RIGHT, anchor=tk.E, **PACK_PARAMS)

        lr_frame.pack(fill=tk.X, **PACK_PARAMS)

        # Display Max and Min.
        display_frame = tk.Frame(ret)

        tk.Label(display_frame, text="Num of Pieces :").\
        grid(row=0, column=0, sticky=tk.E, **PACK_PARAMS)

        tk.Label(display_frame, text="Weight Min / Max :").\
        grid(row=1, column=0, sticky=tk.E, **PACK_PARAMS)

        tk.Label(display_frame, text="from {0} to {1}".format(min_num_pieces, \
        max_num_pieces)).grid(row=0, column=1, sticky=tk.E, **PACK_PARAMS)

        tk.Label(display_frame, text="{0} / {1}".format(min_weight, \
        max_weight)).grid(row=1, column=1, sticky=tk.E, **PACK_PARAMS)

        display_frame.pack(**PACK_PARAMS)

        return ret

    def GetData(self):
        li = self.DataVariablesToFloatList()
        ret = []
        for elm in li:
            ret += [[float(elm[0]), elm[1]]]
        return ret

    def __init__(self, master, line_data):
        tk.LabelFrame.__init__(self, master, text="Weight")

        self.InitDataVariables(line_data)

        # Generate widget list.
        self.list_frame = tk.Frame(self)
        # Buttons
        temp_frame = tk.Frame(self.list_frame)
        def sort_command():
            nonlocal self
            self.SortDataVariables()
            self.update_btn.invoke()
        tk.Button(temp_frame, text="Sort", command=sort_command).\
        pack(side=tk.LEFT)
        def add_command():
            nonlocal self
            index = len(self.data_variables)
            self.data_variables += [[MyStringVar(), MyStringVar()]]
            self.data_variables[index][0].Set(0)
            self.data_variables[index][1].Set(0.0)

            self.GenListItem(self.list_frame, \
            self.data_variables[index]).pack(anchor=tk.W)
        tk.Button(temp_frame, text="Add", command=add_command).\
        pack(side=tk.LEFT)
        temp_frame.pack(anchor=tk.W)
        # Title.
        temp_frame = tk.Frame(self.list_frame)
        tk.Label(temp_frame, text="Num Pieces", width=10).pack(side=tk.LEFT)
        tk.Label(temp_frame, text="Weight", width=10).pack(side=tk.LEFT)
        temp_frame.pack(anchor=tk.W)
        # Lists.
        for i in range(len(self.data_variables)):
            self.GenListItem(self.list_frame, self.data_variables[i])\
            .pack(anchor=tk.W)
        self.list_frame.pack(side=tk.LEFT, anchor=tk.NW)

        # DrawGraph.
        self.graph_frame = tk.Frame(self)
        self.graph = self.GenGraph(self.graph_frame)
        def update_command():
            nonlocal self
            self.graph.destroy()
            self.update_btn.pack_forget()
            self.graph = self.GenGraph(self.graph_frame)
            self.graph.pack()
            self.update_btn.pack()
        self.update_btn = tk.Button(self.graph_frame, text="Update Graph", \
        command=update_command)
        self.graph.pack()
        self.update_btn.pack()
        self.graph_frame.pack(side=tk.LEFT)

if __name__ == "__main__":
    pass
