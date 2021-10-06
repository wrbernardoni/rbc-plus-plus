CC = g++
CFLAGS  = -o3 --std=c++11

LDFLAGS=-lssl
LDFLAGS+=-lcrypto


ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
    LDFLAGS+=-lws2_32
	LDFLAGS+=-lgdi32
	LDFLAGS+=-ladvapi32
	LDFLAGS+=-lcrypt32
    LDFLAGS+=-luser32
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
    CFLAGS += -pthread
endif

INCLUDES = -I. -Ichess -IreconUtils -Ibots -Iutilities


all: BuildPath BoardTest BotBattle HTTPTest Connect LocalTournament EndGames Replay

BuildPath : 
	mkdir -p build
	mkdir -p build/bots
	mkdir -p build/utilities
	mkdir -p build/recon
	mkdir -p build/chess

HTTPTest : httpTest.cpp
	$(CC) $(CFLAGS) -o build/httpTest httpTest.cpp $(INCLUDES) $(LDFLAGS) 

LocalTournament : localTournament.cpp ChessUtilities ReconUtilities Bots Utilities
	$(CC) $(CFLAGS) -o build/localTournament localTournament.cpp build/chess/*.o build/recon/*.o build/bots/*.o build/utilities/*.o $(INCLUDES) $(LDFLAGS) 

Connect : connectToServer.cpp ChessUtilities ReconUtilities Bots Utilities
	$(CC) $(CFLAGS) -o build/connect connectToServer.cpp build/chess/*.o build/recon/*.o build/bots/*.o build/utilities/*.o $(INCLUDES) $(LDFLAGS) 

EndGames : noMoreGames.cpp
	$(CC) $(CFLAGS) -o build/endGames noMoreGames.cpp $(INCLUDES) $(LDFLAGS) 


BoardTest : boardTest.o ChessUtilities
	$(CC) $(CFLAGS) -o build/boardTest build/boardTest.o build/chess/*.o $(INCLUDES) $(LDFLAGS)

Replay : replayGame.o ChessUtilities
	$(CC) $(CFLAGS) -o build/replay build/replayGame.o build/chess/*.o $(INCLUDES) $(LDFLAGS)

BotBattle : botBattle.cpp ChessUtilities ReconUtilities Bots Utilities
	$(CC) $(CFLAGS) -o build/botBattle  botBattle.cpp build/chess/*.o build/recon/*.o build/bots/*.o build/utilities/*.o $(INCLUDES) $(LDFLAGS)

Bots : fullRandom.o inference.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/Bots.o -c build/bots/*.o

fullRandom.o : bots/fullRandom.cpp bots/fullRandom.h
	$(CC) $(CFLAGS) -o build/bots/fullRandom.o -c bots/fullRandom.cpp $(INCLUDES) $(LDFLAGS)

inference.o : bots/inference.cpp bots/inference.h
	$(CC) $(CFLAGS) -o build/bots/inference.o -c bots/inference.cpp

ChessUtilities : BuildPath build/chess/board.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/ChessUtilities.o -c chess/*.cpp

build/chess/%.o : chess/%.cpp BuildPath
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES) $(LDFLAGS)

ReconUtilities : BuildPath build/recon/LocalGame.o build/recon/RemoteGame.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/ReconUtilities.o -c reconUtils/*.cpp

build/recon/%.o : reconUtils/%.cpp BuildPath
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES) $(LDFLAGS)

Utilities : BuildPath build/utilities/BoardManager.o build/utilities/EngineBase.o build/utilities/RandomEngine.o build/utilities/MinBoardsScanEngine.o build/utilities/PureMonteEvaluator.o build/utilities/UniformExpectimaxEngine.o build/utilities/ExpectimaxEngine.o build/utilities/ExpectimaxEngineMT.o
#	$(CC) $(CFLAGS) $(INCLUDES) -o build/Utilities.o -c utilities/*.cpp

build/utilities/%.o : utilities/%.cpp BuildPath
	$(CC) $(CFLAGS) -o $@ -c $< $(INCLUDES) $(LDFLAGS)

boardTest.o: boardTest.cpp ChessUtilities
	$(CC) $(CFLAGS) -o build/boardTest.o -c boardTest.cpp $(INCLUDES) $(LDFLAGS)

replayGame.o: replayGame.cpp ChessUtilities
	$(CC) $(CFLAGS) -o build/replayGame.o -c replayGame.cpp $(INCLUDES) $(LDFLAGS)


clean:
	rm -r -f build/*

