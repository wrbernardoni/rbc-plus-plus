#include "MinBoardsScanEngine.h"
#include <unordered_map>


short WRB_Chess::MinBoardsScanEngine::RecommendScan(const WRB_Chess::InformationSet& brds, WRB_Chess::Color c, double& d)
{
	d = 0;
	short minSense = -1;
	int maxSize = -1;
	for (int j = 1; j < 7; j++)
	{
		for (int i = 1; i < 7; i++)
		{
			std::unordered_map<WRB_Chess::Bitboard, std::unordered_set<WRB_Chess::Bitboard, WRB_Chess::BoardHash>, WRB_Chess::BoardHash> partition;
			for (auto it = brds.boards.begin(); it != brds.boards.end(); it++)
			{
				WRB_Chess::Bitboard masked = (*it).senseMask(i + 8*j);
				partition[masked].emplace((*it));
			}
	
			int localMax = -1;
			
			for (auto it = partition.begin(); it != partition.end(); it++)
			{
				int pSize = (*it).second.size();
				if (pSize > localMax)
				{
					localMax = pSize;
				}
			}
	
			if ((maxSize == -1) || (localMax < maxSize))
			{
				maxSize = localMax;
				minSense = i + 8 * j;
			}
		}
	}
	
	std::cout << "Max number of boards: " << maxSize << std::endl;
	

	return minSense;
}