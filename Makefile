.PHONY: all
all: test misaki

# オブジェクトファイル。
objs = misaki_debug.o \
       chess_def.o \
       chess_util.o \
       chess_board.o \
       chess_board_gen_move.o \
       move.o\
       game_record.o\
       transposition_table.o \
       chess_board_analyze.o \
       chess_board_eval.o \
       chess_board_search.o \
       chess_board_pondering.o \
       chess_board_eval_weights.o \
       opening_book.o \
       pgn_parser.o

# 使用するライブラリ。
libs = -lboost_thread

# コンパイルオプション。
opts = -O3

# テスト用アプリの作成。
test: test_main.o ${objs}
	g++ ${opts} -o $@ test_main.o ${objs} ${libs}
test_main.o: test_main.cpp *.h
	g++ ${opts} -c test_main.cpp

# ライブラリを作成。
direxist=`ls | grep "misaki"`
misaki: libmisaki.a
	${if ${direxist}, , mkdir misaki}
	mv libmisaki.a misaki/
	cp *.h misaki/
libmisaki.a: ${objs}
	ar rcs libmisaki.a ${objs}

# オブジェクトファイルの作成。
misaki_debug.o: misaki_debug.cpp *.h
	g++ ${opts} -c misaki_debug.cpp

chess_def.o: chess_def.cpp *.h
	g++ ${opts} -c chess_def.cpp

chess_util.o: chess_util.cpp *.h
	g++ ${opts} -c chess_util.cpp

chess_board.o: chess_board.cpp *.h
	g++ ${opts} -c chess_board.cpp

chess_board_gen_move.o: chess_board_gen_move.cpp *.h
	g++ ${opts} -c chess_board_gen_move.cpp

move.o: move.cpp *.h
	g++ ${opts} -c move.cpp

game_record.o: game_record.cpp *.h
	g++ ${opts} -c game_record.cpp

transposition_table.o: transposition_table.cpp *.h
	g++ ${opts} -c transposition_table.cpp

chess_board_analyze.o: chess_board_analyze.cpp *.h
	g++ ${opts} -c chess_board_analyze.cpp

chess_board_eval.o: chess_board_eval.cpp *.h
	g++ ${opts} -c chess_board_eval.cpp

chess_board_search.o: chess_board_search.cpp *.h
	g++ ${opts} -c chess_board_search.cpp

chess_board_pondering.o: chess_board_pondering.cpp *.h
	g++ ${opts} -c chess_board_pondering.cpp

chess_board_eval_weights.o: chess_board_eval_weights.cpp *.h
	g++ ${opts} -c chess_board_eval_weights.cpp

opening_book.o: opening_book.cpp *.h
	g++ ${opts} -c opening_book.cpp

pgn_parser.o: pgn_parser.cpp *.h
	g++ ${opts} -c pgn_parser.cpp

# クリーン。
.PHONY: clean
clean:
	git rm *.o test
	git rm -r misaki
