CC = g++
CFLAGS  = -o3 --std=c++11

INCLUDES = -I. -Ichess -IreconUtils -Ibots -Iutilities

all: BuildPath BoardTest BotBattle

BuildPath : 
	mkdir -p build
	mkdir -p build/bots
	mkdir -p build/utilities
	mkdir -p build/recon
	mkdir -p build/chess

BoardTest : boardTest.o ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest build/boardTest.o build/chess/*.o

BotBattle : botBattle.cpp ChessUtilities ReconUtilities Bots Utilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/botBattle  botBattle.cpp build/chess/*.o build/recon/*.o build/bots/*.o build/utilities/*.o

Bots : fullRandom.o inference.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/Bots.o -c build/bots/*.o

fullRandom.o : bots/fullRandom.cpp bots/fullRandom.h
	$(CC) $(CFLAGS) $(INCLUDES) -o build/bots/fullRandom.o -c bots/fullRandom.cpp

inference.o : bots/inference.cpp bots/inference.h
	$(CC) $(CFLAGS) $(INCLUDES) -o build/bots/inference.o -c bots/inference.cpp

ChessUtilities : BuildPath build/chess/board.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/ChessUtilities.o -c chess/*.cpp

build/chess/%.o : chess/%.cpp BuildPath
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

ReconUtilities : BuildPath build/recon/LocalGame.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/ReconUtilities.o -c reconUtils/*.cpp

build/recon/%.o : reconUtils/%.cpp BuildPath
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

Utilities : BuildPath build/utilities/BoardManager.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/Utilities.o -c utilities/*.cpp

build/utilities/%.o : utilities/%.cpp BuildPath
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

boardTest.o: boardTest.cpp ChessUtilities
	$(CC) $(CFLAGS) $(INCLUDES) -o build/boardTest.o -c boardTest.cpp



clean:
	rm -r -f build/*

