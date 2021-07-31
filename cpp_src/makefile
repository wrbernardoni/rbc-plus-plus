CC = g++
CFLAGS  = -o3 --std=c++11

INCLUDES = -I. -Ichess -IreconUtils -Ibots -Iutilities

all: BuildPath BoardTest BotBattle

BuildPath : 
	mkdir -p build
	mkdir -p build/bots

BoardTest : boardTest.o ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest build/boardTest.o build/ChessUtilities.o

BotBattle : botBattle.cpp ChessUtilities ReconUtilities Bots Utilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/botBattle  botBattle.cpp build/ChessUtilities.o build/ReconUtilities.o build/bots/*.o build/Utilities.o

Bots : fullRandom.o inference.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/Bots.o -c build/bots/*.o

fullRandom.o : bots/fullRandom.cpp bots/fullRandom.h
	$(CC) $(CFLAGS) $(INCLUDES) -o build/bots/fullRandom.o -c bots/fullRandom.cpp

inference.o : bots/inference.cpp bots/inference.h
	$(CC) $(CFLAGS) $(INCLUDES) -o build/bots/inference.o -c bots/inference.cpp

ChessUtilities : BuildPath chess/*
	$(CC) $(CFLAGS) $(INCLUDES) -o build/ChessUtilities.o -c chess/*.cpp

ReconUtilities : BuildPath reconUtils/*
	$(CC) $(CFLAGS) $(INCLUDES) -o build/ReconUtilities.o -c reconUtils/*.cpp

Utilities : BuildPath utilities/*
	$(CC) $(CFLAGS) $(INCLUDES) -o build/Utilities.o -c utilities/*.cpp

boardTest.o: boardTest.cpp ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest.o -c boardTest.cpp



clean:
	rm -r -f build/*

