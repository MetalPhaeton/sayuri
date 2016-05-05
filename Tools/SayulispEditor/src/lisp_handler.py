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

"""Lisp code handler."""

import sys,os

# Tokenize Lisp code simply.
def TokenizeSimply(code):
    # Split function.
    def MySplit(code, letter):
        tokens = code.split(letter)
        if len(tokens) <= 0: return tokens

        ret = [tokens[0]]
        for i in range(1, len(tokens)):
            ret += [letter]
            ret += [tokens[i]]
        return ret

    # Tokenize.
    step_1 = MySplit(code, "\n")
    step_2 = []
    for token in step_1: step_2 += MySplit(token, "(")
    step_3 = []
    for token in step_2: step_3 += MySplit(token, ")")
    step_4 = []
    for token in step_3: step_4 += MySplit(token, " ")

    # Clean.
    step_5 = []
    for token in step_4:
        if token not in {"", " "}: step_5 += [token]

    # Replace Number or Boolean.
    step_6 = []
    for token in step_5:
        if token == "#t": step_6 += [True]
        elif token == "#f": step_6 += [False]
        else:
            try: step_6 += [float(token)]
            except: step_6 += [token]

    # Replace () to List.
    step_7 = []
    i = 0
    size = len(step_6)
    def Core():
        nonlocal i
        if step_6[i] == "(":
            i += 1
            ret = []
            while step_6[i] != ")":
                if step_6[i] == "(":
                    ret += [Core()]
                else:
                    ret += [step_6[i]]
                    i += 1
            i += 1
            return ret
        return list()
    step_7 = Core()

    return step_7

# List to S-Expression.
def ToSExprSimply(li):
    # When li is not list.
    if type(li) != list:
        if type(li) == str: return li
        elif type(li) == bool:
            if li: return "#t"
            else: return "#f"
        else: return str(li)

    ret = ""
    temp = []

    # To string.
    for elm in li:
        if type(elm) != str:
            if type(elm) == list:
                temp += [ToSExprSimply(elm)]
            elif type(elm) == bool:
                if elm: temp += ["#t"]
                else: temp += ["#f"]
            else: temp += [str(elm)]
        else: temp += [elm]

    # Generate S-Expr.
    for i in range(len(temp)):
        if i  == 0: ret += temp[i]
        else: ret += " " + temp[i]

    return "(" + ret + ")"

# Quote S-Expression.
def Quote(s): return "(quote " + ToSExprSimply(s) + ")"

# Engine S-Expression.
# Format : (<engine_name> (quote <symbol>) (quote <param>))
def EngineSExpr(engine_name, symbol, param):
    return "(" + engine_name + " " + Quote(symbol) + " " + Quote(param) + ")"

# S-Expression to List of engine settings.
def ToEngineList(code):
    tokens = TokenizeSimply(code)
    if len(tokens) >= 3:
        if (len(tokens[1]) >= 2) and (len(tokens[2]) >= 2):
            if (tokens[1][0] == "quote") and (tokens[2][0] == "quote"):
                return (tokens[0], tokens[1][1], tokens[2][1])
    return (None, None, None)

if __name__ == "__main__":
    pass
