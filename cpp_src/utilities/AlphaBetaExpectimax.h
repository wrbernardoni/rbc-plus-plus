#ifndef WRB_ENGINE_ALPHA_BETA_EXPECTI_H_
#define WRB_ENGINE_ALPHA_BETA_EXPECTI_H_

#include "ExpectimaxEngineMT.h"

#include <iostream>
#include <limits>
#include <algorithm>

namespace WRB_Chess
{
	class ABMoveComparator
	{
	private:
		WRB_std::RollingMap<WRB_Chess::Move, int, WRB_Chess::MoveHash>* killH;
	public:
		ABMoveComparator(WRB_std::RollingMap<WRB_Chess::Move, int, WRB_Chess::MoveHash>* m)
		{
			killH = m;
		};

		ABMoveComparator(const ABMoveComparator& o)
		{
			killH = o.killH;
		};

		ABMoveComparator& operator=(const ABMoveComparator& o)
		{
			killH = o.killH;

			return *this;
		};

		bool operator() (const WRB_Chess::Move& i, const WRB_Chess::Move& j)
		{
			int iV = 0;
			int jV = 0;
			if (killH->contains(i))
			{
				iV = killH->unmovingRead(i);
			}

			if (killH->contains(j))
			{
				jV = killH->unmovingRead(j);
			}

			return iV > jV;
		};
	};

	class AlphaBetaExpectimax : public ExpectimaxMT
	{
	private:
		//unsigned int playoutsPerEval;
		unsigned int depth;
		unsigned int saveDepth;
		int visitedBoards;
		int savedBoards;
		WRB_std::RollingMap<WRB_Chess::Move, int, WRB_Chess::MoveHash> killHeuristic;

		double Heuristic(const WRB_Chess::Bitboard& b, WRB_Chess::Color c)
		{
			double score = 0.0;
			score += 200.0 * ((int)b.Pieces(c, WRB_Chess::Piece::King).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King).count());
			score += 9.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Queen).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Queen).count());
			score += 5.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Rook).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Rook).count());
			score += 3.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Bishop).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Bishop).count());
			score += 3.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Knight).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Knight).count());
			score += 1.0 * ((int)b.Pieces(c, WRB_Chess::Piece::Pawn).count() - (int)b.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::Pawn).count());
			
			auto mob1 = b.AvailableMoves(c);
			int tM1 = 0;

			std::unordered_set<WRB_Chess::Move, WRB_Chess::MoveHash> uniqueMoves1;
			for (int i = 0; i < mob1.size(); i++)
			{
				WRB_Chess::Move taken = b.RectifyMove(mob1[i]);
				if (uniqueMoves1.count(taken) == 0)
				{
					uniqueMoves1.emplace(taken);
					WRB_Chess::Bitboard tB = b;
					tB.ApplyMove(taken);
					if (tB.Pieces(c, WRB_Chess::Piece::King) != 0)
					{
						bool good = true;
						if (!tB.Check(c))
						{
							tM1++;
						}
					}
				}
			}

			auto mob2 = b.AvailableMoves(OPPOSITE_COLOR(c));
			int tM2 = 0;

			std::unordered_set<WRB_Chess::Move, WRB_Chess::MoveHash> uniqueMoves2;
			for (int i = 0; i < mob2.size(); i++)
			{
				WRB_Chess::Move taken = b.RectifyMove(mob2[i]);
				if (uniqueMoves2.count(taken) == 0)
				{
					uniqueMoves2.emplace(taken);
					WRB_Chess::Bitboard tB = b;
					tB.ApplyMove(taken);
					if (tB.Pieces(OPPOSITE_COLOR(c), WRB_Chess::Piece::King) != 0)
					{
						if (!tB.Check(OPPOSITE_COLOR(c)))
						{
							tM2++;
						}
					}
				}
			}

			score += 0.1 * (tM1 - tM2);

			return score;
		}

		// Returns score from the perspective of white
		double AlphaBeta(const WRB_Chess::Bitboard& b, unsigned int d, double alpha, double beta, WRB_Chess::Color toMove, WRB_Chess::Color c)
		{
			if (scores.contains(b))
			{
				// savedBoards++;
				return scores[b];
			}
			else if (!b.KingsAlive())
			{
				// Terminal node
				if (b.Pieces(WRB_Chess::Color::White, WRB_Chess::Piece::King) == 0)
				{
					return -1000;
				}
				else
				{
					return 1000;
				}
			}
			else if (b.getHalfmoveClock() >= 100)
			{
				return 0;
			}
			else if (d <= 0)
			{
				// Heuristic
				if (toMove == WRB_Chess::Color::White)
					return Heuristic(b, toMove);
				else
					return -Heuristic(b, toMove);
			}

			// visitedBoards++;
			// std::cout << "vb " << visitedBoards << std::endl;
			// std::cout << "d:" << d << " a:" << alpha << " b: " << beta << std::endl;

			ABMoveComparator moveComp(&killHeuristic);

			if (toMove == WRB_Chess::Color::White)
			{
				// Maximizing
				double value = -std::numeric_limits<double>::infinity();

				std::vector<WRB_Chess::Move> mvs = b.AvailableMoves(toMove);

				killHeuristic.lock();
				std::sort(mvs.begin(), mvs.end(), moveComp);
				killHeuristic.unlock();

				bool good = true;
				for (int i = 0; i < mvs.size(); i++)
				{
					WRB_Chess::Bitboard tB = b;
					tB.ApplyMove(mvs[i]);
					double nV = AlphaBeta(tB, d - 1, alpha, beta, OPPOSITE_COLOR(toMove), c)/2;
					if (nV > value)
						value = nV;
					if (value >= beta)
					{
						if (killHeuristic.contains(mvs[i]))
						{
							killHeuristic[mvs[i]]++;
						}
						else
						{
							killHeuristic[mvs[i]] = 1;
						}

						good = false;
						break;
					}
					if (value > alpha)
						alpha = value;
				}

				if (good && (d >= saveDepth))
				{
					// std::cout << "\tsv" << std::endl;
					scores[b] = value;
				}

				// std::cout << "\tval:" << value << std::endl;
				return value;
			}
			else
			{
				// Minimizing
				double value = std::numeric_limits<double>::infinity();

				std::vector<WRB_Chess::Move> mvs = b.AvailableMoves(toMove);

				killHeuristic.lock();
				std::sort(mvs.begin(), mvs.end(), moveComp);
				killHeuristic.unlock();

				bool good = true;
				for (int i = 0; i < mvs.size(); i++)
				{
					WRB_Chess::Bitboard tB = b;
					tB.ApplyMove(mvs[i]);
					double nV = AlphaBeta(tB, d - 1, alpha, beta, OPPOSITE_COLOR(toMove), c)/2;
					if (nV < value)
						value = nV;
					if (value <= alpha)
					{
						if (killHeuristic.contains(mvs[i]))
						{
							killHeuristic[mvs[i]]++;
						}
						else
						{
							killHeuristic[mvs[i]] = 1;
						}

						good = false;
						break;
					}
					if (value < beta)
						beta = value;
				}

				if (good && (d >= saveDepth))
				{
					// std::cout << "\tsv" << std::endl;
					if (c == WRB_Chess::Color::Black)
						scores[b] = -value;
					else
						scores[b] = value;
				}

				// std::cout << "\tval:" << value << std::endl;
				return value;
			}
		}
	public:
		AlphaBetaExpectimax(unsigned int ppE, unsigned int d, unsigned int sD, size_t ns, int nThreads) : ExpectimaxMT(ppE, ns, nThreads), killHeuristic(10000)
		{
			depth = d;
			saveDepth = sD;
		};

		virtual double EvaluatePosition(const WRB_Chess::Bitboard& b, WRB_Chess::Color c)
		{
			if (scores.contains(b))
				return scores[b];

			// visitedBoards = 0;
			// savedBoards = 0;
			double sc = AlphaBeta(b, depth, -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), c, c);
			// std::cout << "vb " << visitedBoards << " | sb " << savedBoards << std::endl;
			if (c == WRB_Chess::Color::Black)
				sc = -sc;

			return sc;
		};
	};
};

#endif
