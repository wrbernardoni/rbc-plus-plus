CC = g++
CFLAGS  = -o3 --std=c++11

INCLUDES = -I. -Ichess -IreconUtils -Ibots

all: BoardTest Bots

BoardTest : boardTest.o ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest build/boardTest.o build/ChessUtilities.o

Bots : bots/* ChessUtilities ReconUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/bots.o -c bots/*.cpp build/ChessUtilities.o

ChessUtilities : chess/board.h board.o

ReconUtilities : reconUtils/BotBase.h reconUtils/GameHistory.h

boardTest.o: boardTest.cpp ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest.o -c boardTest.cpp


board.o : chess/board.h
	$(CC) $(CFLAGS) $(INCLUDES) -o build/ChessUtilities.o -c ./chess/board.cpp

clean:
	rm -f build/*

