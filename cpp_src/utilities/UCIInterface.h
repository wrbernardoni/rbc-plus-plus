#ifndef WRB_UCI_INTERFACE_H_
#define WRB_UCI_INTERFACE_H_

#include <string>
#include <windows.h> 
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>
#include "../chess/board.h"

namespace WRB_Chess
{

	class UCI_Interface
	{
	private:
		HANDLE g_hChildStd_IN_Rd = NULL;
		HANDLE g_hChildStd_IN_Wr = NULL;
		HANDLE g_hChildStd_OUT_Rd = NULL;
		HANDLE g_hChildStd_OUT_Wr = NULL;
		PROCESS_INFORMATION piProcInfo;
		std::string cmdline;

		void BootProcess();
		bool ProcessAlive();
		void WriteToPipe(std::string input);
		std::string ReadFromPipe();
	public:
		UCI_Interface(std::string enginePath);
		~UCI_Interface();

		double ScorePosition(WRB_Chess::Bitboard brd, int msec); 
		// Score is from white's perspective
	};
}

#endif
