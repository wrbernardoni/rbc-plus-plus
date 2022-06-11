#include "UCIInterface.h"

#include <iostream>
#include <vector>
#include <limits>

#define BUFSIZE 4096 
using namespace std;
#define MATEVALUE 1000000.0
#define INTERFACE_TIMEOUT 0.5
#define MAX_RECOVERIES 0

// Info on how to make processes and pipes are from https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
WRB_Chess::UCI_Interface::UCI_Interface(std::string enginePath)
{
	std::cout << "Engine Path:" << enginePath << std::endl;
	cmdline = enginePath;

	BootProcess();
}

WRB_Chess::UCI_Interface::~UCI_Interface()
{
	TerminateProcess(piProcInfo.hProcess, 1);
}

void WRB_Chess::UCI_Interface::BootProcess()
{
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL;

	CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0);
	SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
	CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0);
	SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);

	COMMTIMEOUTS timeouts = { 0, //interval timeout. 0 = not used
							  0, // read multiplier
							 500, // read constant (milliseconds)
							  0, // Write multiplier
							  0  // Write Constant
							};

	SetCommTimeouts(g_hChildStd_OUT_Rd, &timeouts);
	SetCommTimeouts(g_hChildStd_IN_Rd, &timeouts);

	TCHAR *szCmdline = new TCHAR[cmdline.size() + 1];
	szCmdline[cmdline.size()] = 0;
	std::copy(cmdline.begin(), cmdline.end(), szCmdline);
	 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE; 
 
// Set up members of the PROCESS_INFORMATION structure. 
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.
 
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
 
// Create the child process. 
	
	CreateProcess(NULL, 
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION

	WriteToPipe("uci");
	WriteToPipe("setoption name Threads value 2");
	WriteToPipe("setoption name UCI_AnalyseMode value true");
	// WriteToPipe("setoption name Use NNUE value false");
}

bool WRB_Chess::UCI_Interface::ProcessAlive()
{
	DWORD code;
	bool check = GetExitCodeProcess(piProcInfo.hProcess, &code);
	return (check != 0) && (code == STILL_ACTIVE);
}

void WRB_Chess::UCI_Interface::WriteToPipe(std::string input)
{
	if (!ProcessAlive())
		return;
	DWORD dwWritten;
	CHAR chBuf[BUFSIZE];

	input += "\n";
	WriteFile(g_hChildStd_IN_Wr, input.c_str(), strlen(input.c_str()), &dwWritten, NULL);
}

std::string WRB_Chess::UCI_Interface::ReadFromPipe()
{
	DWORD dwRead, dwWritten, dAvail;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	string output = "";
	bool flag = false;

	for (;;)
	{
		if (!ProcessAlive())
			return "";
		PeekNamedPipe(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, &dAvail, NULL);
		if (dAvail == 0)
		{
			return "";
		}
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		for (int i = 0; i < dwRead; i++)
		{
			if (chBuf[i] == '\n')
			{
				flag = true;
				break;
			}
			output += chBuf[i];
		}
		if (flag)
		{
			break;
		}
	}

	return output;
}

vector<string> splitString(string s)
{
	vector<string> tokens;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(" ")) != std::string::npos) {
		token = s.substr(0, pos);
		tokens.push_back(token);
		s.erase(0, pos + 1);
	}
	token = s.substr(0, pos);
	tokens.push_back(token);
	s.erase(0, pos + 1);
	return tokens;
}

#include <chrono>

double WRB_Chess::UCI_Interface::ScorePosition(WRB_Chess::Bitboard brd, int msec)
{
	if (!brd.KingsAlive())
	{
		if (brd.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::King) == 0)
			return -MATEVALUE;
		else if (brd.Pieces(WRB_Chess::Color::Black, WRB_Chess::Piece::King) == 0)
			return MATEVALUE;
	}

	string fen = brd.fen();
	double scoreMult = (brd.toMove() == WRB_Chess::Color::Black)? -1.0 :1.0;

	if (brd.Check(OPPOSITE_COLOR(brd.toMove())))
	{
		return MATEVALUE * scoreMult/2;
	}

	int recoveries = 0;
	do
	{
		if (!ProcessAlive())
		{
			BootProcess();
			if (recoveries >= MAX_RECOVERIES)
				break;
			recoveries++;
		}
		std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
		bool deadProcess = false;
		WriteToPipe("stop");
		WriteToPipe("ucinewgame");
		WriteToPipe("isready");
		while (true)
		{
			if (!ProcessAlive())
			{
				deadProcess = true;
				break;
			}
			string resp = ReadFromPipe();
			// if (resp != "")
				// cout << "RES: " << resp << endl;
			if (resp.find("readyok") != std::string::npos)
				break;
		}
		if (deadProcess)
		{
			continue;
		}

		string pos = "position fen " + fen;
		// cout << pos << endl;
		string mT = "go depth 1";//movetime " + to_string(msec);
		// cout << mT << endl;
		WriteToPipe(pos);
		WriteToPipe(mT);
		// cout << "Scoring" << endl;
		vector<string> pTok;
		while (true)
		{
			if (!ProcessAlive() || ((std::chrono::duration<double>)(std::chrono::steady_clock::now() - start)).count() > INTERFACE_TIMEOUT)
			{
				deadProcess = true;
				break;
			}
			string resp = ReadFromPipe();
			// if (resp != "")
			// 	cout << resp << endl;
			vector<string> res = splitString(resp);
			if (res[0] == "bestmove")
				break;
			if (res.size() != 0)
				pTok = res;
		}
		if (deadProcess)
		{
			continue;
		}
		

		if (pTok.size() > 0 && pTok[0] == "info")
		{
			int i = 0;
			bool mate = false;
			for (;i < pTok.size(); i++)
			{
				if (pTok[i] == "cp")
				{
					i++;
					break;
				}
				if (pTok[i] == "mate")
				{
					i++;
					mate = true;
					break;
				}
			}

			// cout << pTok[i] << endl;
			if (i == pTok.size())
			{
				return std::numeric_limits<double>::infinity();
			}
			else if (mate)
			{
				return scoreMult * MATEVALUE/((double)stoi(pTok[i]) + 1.0);
			}
			else
				return scoreMult * ((double)stoi(pTok[i])/100);
		}
	} while(true);
	return std::numeric_limits<double>::infinity();

}