OBJS=epoll_web_sudoku.o Sudoku.o wrap.o pub.o
TARGET=epoll_web_sudoku

$(TARGET):$(OBJS)
	g++ -w -std=c++11 $(OBJS) -o $(TARGET)

wrap.o:wrap.c
	g++ -w -c $< -o $@

pub.o:pub.c
	g++ -w -c $< -o $@

Sudoku.o:Sudoku.cpp
	g++ -w -std=c++11 -c $< -o $@

epoll_web_sudoku.o:epoll_web_sudoku.cpp
	g++ -w -std=c++11 -c $< -o $@

