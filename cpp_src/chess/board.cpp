#include "board.h"
#include <cctype>
#include <stdlib.h>

const char* WRB_Chess::SquareNames[] = {"A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
										"A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
										"A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
										"A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
										"A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
										"A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
										"A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
										"A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"};

const char* WRB_Chess::PieceNames[] = {"Pawn", "Bishop", "Rook", "Knight", "Queen", "King"};
const char* WRB_Chess::ColorNames[] = {"White", "Black"};



short WRB_Chess::RankAndFileToSquare(short rank, short file)
{
	return 8*(rank - 1) + file;
}

short WRB_Chess::RankAndFileToSquare(short rank, WRB_Chess::File file)
{
	return WRB_Chess::RankAndFileToSquare(rank, (short) file);
}

short WRB_Chess::ManhattanDistance(short sq1, short sq2)
{
	return abs((sq1/8) - (sq2/8)) + abs((sq1%8) - (sq2%8));
}

std::bitset<64> WRB_Chess::SquareToMask(short x)
{
	std::bitset<64> r;
	r[x] = true;
	return r;
}

std::bitset<64> WRB_Chess::SquareToMask(std::vector<short> x)
{
	std::bitset<64> r;
	for (int i = 0; i < x.size(); i++)
	{
		if (x[i] >= 0 && x[i] < 64)
		{
			r[x[i]] = true;
		}
	}
	return r;
}

std::vector<short> WRB_Chess::MaskToSquares(std::bitset<64> mask)
{
	std::vector<short> out;
	for (int i = 0; i < 64; i++)
	{
		if (mask[i])
		{
			out.push_back(i);
		}
	}

	return out;
}


WRB_Chess::Bitboard::Bitboard()
{
	this->color_masks[WRB_Chess::Color::White] = 0b0000000000000000000000000000000000000000000000001111111111111111;
	this->color_masks[WRB_Chess::Color::Black] = 0b1111111111111111000000000000000000000000000000000000000000000000;
	this->piece_masks[WRB_Chess::Piece::Pawn] = 0b0000000011111111000000000000000000000000000000001111111100000000;
	this->piece_masks[WRB_Chess::Piece::Bishop] = 0b0010010000000000000000000000000000000000000000000000000000100100;
	this->piece_masks[WRB_Chess::Piece::Rook] = 0b1000000100000000000000000000000000000000000000000000000010000001;
	this->piece_masks[WRB_Chess::Piece::Knight] = 0b0100001000000000000000000000000000000000000000000000000001000010;
	this->piece_masks[WRB_Chess::Piece::Queen] = 0b0000100000000000000000000000000000000000000000000000000000001000;	
	this->piece_masks[WRB_Chess::Piece::King] = 0b0001000000000000000000000000000000000000000000000000000000010000;
	this->queensideCastle[WRB_Chess::Color::White] = true;
	this->queensideCastle[WRB_Chess::Color::Black] = true;
	this->kingsideCastle[WRB_Chess::Color::White] = true;
	this->kingsideCastle[WRB_Chess::Color::Black] = true;
	this->epSquare = -1;
}

WRB_Chess::ColorPiece WRB_Chess::Bitboard::PieceAt(short square)
{
	ColorPiece ret;
	ret.color = WRB_Chess::Color::NoColor;
	ret.piece = WRB_Chess::Piece::NoPiece;

	if ((square < 0) || (square > 63))
		return ret;

	for (int pt = 0; pt < 6; pt++)
	{
		if (this->piece_masks[pt][square])
		{
			ret.piece = (WRB_Chess::Piece)pt;
			break;
		}
	}

	for (int pt = 0; pt < 2; pt++)
	{
		if (this->color_masks[pt][square])
		{
			ret.color = (WRB_Chess::Color)pt;
			break;
		}
	}

	return ret;
}

std::vector<WRB_Chess::Move> WRB_Chess::Bitboard::AvailableMoves(WRB_Chess::Color c)
{
	//TODO
	std::vector<WRB_Chess::Move> mvs;
	std::bitset<64> myPieces = this->color_masks[c];

	// Null move is always acceptible
	mvs.push_back(WRB_Chess::Move());

	//Generate Pawn moves
	std::bitset<64> correctPawns = this->piece_masks[WRB_Chess::Piece::Pawn] & this->color_masks[c];
	for (int i = 0; i < 64; i++)
	{
		if (correctPawns[i])
		{
			if (c == WRB_Chess::Color::White)
			{
				// Move one
				if ((i < 56) && (!myPieces[i+8]))
				{
					if (i >= 51)
					{
						// Promotion matters
						for (int p = 1; p <= 4; p++)
						{
							WRB_Chess::Move oneUp;
							oneUp.fromSquare = i;
							oneUp.toSquare = i+8;
							oneUp.promotion = (WRB_Chess::Piece)p;
							mvs.push_back(oneUp);
						}
					}
					else
					{
						// Promotion does not matter
						WRB_Chess::Move oneUp;
						oneUp.fromSquare = i;
						oneUp.toSquare = i+8;
						mvs.push_back(oneUp);
					}
					
					// Move two if you can move one
					if ((i >= 8) && (i < 16) && (!myPieces[i+16]))
					{
						WRB_Chess::Move oneUp;
						oneUp.fromSquare = i;
						oneUp.toSquare = i+16;
						mvs.push_back(oneUp);
					}
				}
				

				// Attacks
				if (i < 56)
				{
					if (((i%8) != 0) && (!myPieces[i + 7]))
					{
						if (i >= 52)
						{
							for (int p = 1; p <= 4; p++)
							{
								WRB_Chess::Move oneUp;
								oneUp.fromSquare = i;
								oneUp.toSquare = i+7;
								oneUp.promotion = (WRB_Chess::Piece)p;
								mvs.push_back(oneUp);
							}
						}
						else
						{
							WRB_Chess::Move oneUp;
							oneUp.fromSquare = i;
							oneUp.toSquare = i+7;
							mvs.push_back(oneUp);
						}
					}

					if (((i%8) != 7) && (!myPieces[i + 9]))
					{
						if (i >= 52)
						{
							for (int p = 1; p <= 4; p++)
							{
								WRB_Chess::Move oneUp;
								oneUp.fromSquare = i;
								oneUp.toSquare = i+9;
								oneUp.promotion = (WRB_Chess::Piece)p;
								mvs.push_back(oneUp);
							}
						}
						else
						{
							WRB_Chess::Move oneUp;
							oneUp.fromSquare = i;
							oneUp.toSquare = i+9;
							mvs.push_back(oneUp);
						}
					}
				}
			}
			else // Color is black
			{
				// Move one
				if ((i > 7) && (!myPieces[i-8]))
				{
					if (i <= 15)
					{
						// Promotion matters
						for (int p = 1; p <= 4; p++)
						{
							WRB_Chess::Move oneUp;
							oneUp.fromSquare = i;
							oneUp.toSquare = i-8;
							oneUp.promotion = (WRB_Chess::Piece)p;
							mvs.push_back(oneUp);
						}
					}
					else
					{
						// Promotion does not matter
						WRB_Chess::Move oneUp;
						oneUp.fromSquare = i;
						oneUp.toSquare = i-8;
						mvs.push_back(oneUp);
					}
					
					// Move two if you can move one
					if ((i < 56) && (i >= 48) && (!myPieces[i-16]))
					{
						WRB_Chess::Move oneUp;
						oneUp.fromSquare = i;
						oneUp.toSquare = i-16;
						mvs.push_back(oneUp);
					}
				}
				

				// Attacks
				if (i > 7)
				{
					if (((i%8) != 0) && (!myPieces[i - 9]))
					{
						if (i <= 15)
						{
							for (int p = 1; p <= 4; p++)
							{
								WRB_Chess::Move oneUp;
								oneUp.fromSquare = i;
								oneUp.toSquare = i-9;
								oneUp.promotion = (WRB_Chess::Piece)p;
								mvs.push_back(oneUp);
							}
						}
						else
						{
							WRB_Chess::Move oneUp;
							oneUp.fromSquare = i;
							oneUp.toSquare = i-9;
							mvs.push_back(oneUp);
						}
					}

					if (((i%8) != 7) && (!myPieces[i - 7]))
					{
						if (i <= 15)
						{
							for (int p = 1; p <= 4; p++)
							{
								WRB_Chess::Move oneUp;
								oneUp.fromSquare = i;
								oneUp.toSquare = i-7;
								oneUp.promotion = (WRB_Chess::Piece)p;
								mvs.push_back(oneUp);
							}
						}
						else
						{
							WRB_Chess::Move oneUp;
							oneUp.fromSquare = i;
							oneUp.toSquare = i-7;
							mvs.push_back(oneUp);
						}
					}
				}
			}
		}
	}

	//Generate Bishop moves

	//Generate Rook moves

	//Generate Knight moves

	//Generate Queen moves

	//Generate King moves

	return mvs;
}

#define SCAN_SLIDE_STEP(a) ((a) > 0 ? 1 : (a) < 0 ? -1 : 0)
#include <iostream>

WRB_Chess::Move WRB_Chess::Bitboard::RectifySlide(WRB_Chess::Move m, bool canCapture)
{
	if ((m.fromSquare == -1) || (m.toSquare == -1))
	{
		return m;
	}

	WRB_Chess::ColorPiece pMove = this->PieceAt(m.fromSquare);

	short scan = m.fromSquare;
	while (scan != m.toSquare)
	{
		short pScan = scan;
		scan += 8 * SCAN_SLIDE_STEP((m.toSquare/8) - (m.fromSquare/8)) + SCAN_SLIDE_STEP((m.toSquare%8) - (m.fromSquare%8));
		WRB_Chess::ColorPiece pAt = this->PieceAt(scan);
		if (pAt.piece != WRB_Chess::Piece::NoPiece)
		{
			if ((!canCapture) || (pAt.color == pMove.color))
			{
				Move newM;
				newM.fromSquare = m.fromSquare;
				newM.toSquare = pScan;
				newM.promotion = m.promotion;
			}
			else if (pAt.color != pMove.color)
			{
				Move newM;
				newM.fromSquare = m.fromSquare;
				newM.toSquare = scan;
				newM.promotion = m.promotion;
			}
		}
	}

	return m;
}

WRB_Chess::Move WRB_Chess::Bitboard::RectifyMove(WRB_Chess::Move m)
{
	//TODO
	WRB_Chess::ColorPiece pMove = this->PieceAt(m.fromSquare);
	if ((pMove.piece == WRB_Chess::Piece::NoPiece) || (pMove.color == WRB_Chess::Color::NoColor))
	{
		return m;
	}

	if (pMove.piece == WRB_Chess::Piece::Pawn)
	{
		short mDist = ManhattanDistance(m.fromSquare, m.toSquare);
		if (mDist == 1)
		{
			WRB_Chess::ColorPiece pAt = this->PieceAt(m.toSquare);
			if (this->PieceAt(m.toSquare).color != WRB_Chess::Color::NoColor)
				return Move();
		}
		else if (mDist == 2)
		{
			if ((m.toSquare % 8) != (m.fromSquare % 8))
			{
				short epCheck = m.fromSquare + SCAN_SLIDE_STEP((m.toSquare%8) - (m.fromSquare%8));
				// Attack
				WRB_Chess::ColorPiece pAt = this->PieceAt(m.toSquare);
				if ((pAt.color == pMove.color) || ((pAt.color == WRB_Chess::Color::NoColor) && epCheck != this->epSquare))
				{
					return Move();
				}
			}
			else
			{
				// Slide
				return this->RectifySlide(m, false);
			}
		}
		else
		{
			return Move();
		}
	}

	return m;
}

std::string WRB_Chess::GetPrintable(WRB_Chess::Bitboard brd)
{
	std::string b;
	b.resize(9*8);

	for (int j = 0; j < 8; j++)
	{
		for (int i = 0; i < 8; i++)
		{
			WRB_Chess::ColorPiece p = brd.PieceAt(WRB_Chess::RankAndFileToSquare((8 - j), i));
			char c = ' ';
			switch (p.piece)
			{
				case WRB_Chess::NoPiece:
					c = ' ';
				break;

				case WRB_Chess::Pawn:
					c = 'p';
				break;

				case WRB_Chess::Bishop:
					c = 'b';
				break;

				case WRB_Chess::Rook:
					c = 'r';
				break;

				case WRB_Chess::Knight:
					c = 'n';
				break;

				case WRB_Chess::Queen:
					c = 'q';
				break;

				case WRB_Chess::King:
					c = 'k';
			}

			if (p.color == WRB_Chess::Color::White)
			{
				c = toupper(c);
			}

			b[(j) * 9 + i] = c;
		}
		b[(j) * 9 + 8] = '\n';
	}
	
	return b;
}

std::string WRB_Chess::GetPrintable(WRB_Chess::Move mv)
{
	std::string ret = "";
	if ((mv.fromSquare < 0) ||(mv.toSquare < 0))
	{
		ret = "Nil";
	}
	else
	{
		ret = ret + WRB_Chess::SquareNames[mv.fromSquare] + "-" + WRB_Chess::SquareNames[mv.toSquare];
	}

	return ret;
}

