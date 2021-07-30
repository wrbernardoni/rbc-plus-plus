CC = g++
CFLAGS  = -o3 --std=c++11

INCLUDES = -I. -Ichess -IreconUtils -Ibots

all: BuildPath BoardTest BotBattle

BuildPath : 
	mkdir -p build

BoardTest : boardTest.o ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest build/boardTest.o build/ChessUtilities.o

BotBattle : botBattle.cpp ChessUtilities ReconUtilities Bots
	$(CC) $(CFLAGS) $(INCLUDES) -o build/botBattle  botBattle.cpp build/ChessUtilities.o build/ReconUtilities.o build/Bots.o

Bots : bots/* 
	$(CC) $(CFLAGS) $(INCLUDES) -o build/Bots.o -c bots/*.cpp

ChessUtilities : BuildPath chess/*
	$(CC) $(CFLAGS) $(INCLUDES) -o build/ChessUtilities.o -c chess/*.cpp

ReconUtilities : BuildPath reconUtils/*
	$(CC) $(CFLAGS) $(INCLUDES) -o build/ReconUtilities.o -c reconUtils/*.cpp

boardTest.o: boardTest.cpp ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest.o -c boardTest.cpp



clean:
	rm -r -f build/*

