OBJS=libevent_sudoku.o Sudoku.o wrap.o pub.o
TARGET=libevent_sudoku

$(TARGET):$(OBJS)
	g++ -w -std=c++11 $(OBJS) -o $(TARGET) -levent

wrap.o:wrap.c
	g++ -w -c $< -o $@

pub.o:pub.c
	g++ -w -c $< -o $@

Sudoku.o:Sudoku.cpp
	g++ -w -std=c++11 -c $< -o $@

libevent_sudoku.o:libevent_sudoku.cpp
	g++ -w -std=c++11 -c $< -o $@ -levent
