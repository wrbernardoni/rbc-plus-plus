#include "board.h"
#include <cctype>
#include <stdlib.h>
#include <string>

const char* WRB_Chess::SquareNames[] = {"A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
										"A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
										"A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
										"A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
										"A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
										"A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
										"A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
										"A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8"};

const char* WRB_Chess::squarenames[] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
										"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
										"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
										"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
										"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
										"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
										"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
										"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

const char* WRB_Chess::PieceNames[] = {"Pawn", "Bishop", "Rook", "Knight", "Queen", "King"};
const char* WRB_Chess::ColorNames[] = {"White", "Black"};

std::size_t WRB_Chess::BoardHash::hashKey[64][12];

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

std::string WRB_Chess::move_to_UCI(WRB_Chess::Move mv)
{
	std::string s = "";
	if ((mv.toSquare == -1) || (mv.fromSquare == -1))
		return "0000";
	s += WRB_Chess::squarenames[mv.fromSquare];
	s += WRB_Chess::squarenames[mv.toSquare];

	switch (mv.promotion)
	{
		case WRB_Chess::Piece::Knight:
			s += "n";
		break;

		case WRB_Chess::Piece::Rook:
			s += "r";
		break;

		case WRB_Chess::Piece::Bishop:
			s += "b";
		break;

		case WRB_Chess::Piece::Queen:
			s += "q";
		break;
	}

	return s;
}

WRB_Chess::Move WRB_Chess::UCI_to_move(std::string s)
{
	if (s == "0000")
	{
		return WRB_Chess::Move();
	}
	else
	{
		int srank = s[1] - '0';
		int sfile = 0;

		switch(s[0])
		{
			case 'a':
				sfile = 0;
			break;

			case 'b':
				sfile = 1;
			break;

			case 'c':
				sfile = 2;
			break;

			case 'd':
				sfile = 3;
			break;

			case 'e':
				sfile = 4;
			break;

			case 'f':
				sfile = 5;
			break;

			case 'g':
				sfile = 6;
			break;

			case 'h':
				sfile = 7;
			break;
		};

		int trank = s[3] - '0';
		int tfile = 0;
		switch(s[2])
		{
			case 'a':
				tfile = 0;
			break;

			case 'b':
				tfile = 1;
			break;

			case 'c':
				tfile = 2;
			break;

			case 'd':
				tfile = 3;
			break;

			case 'e':
				tfile = 4;
			break;

			case 'f':
				tfile = 5;
			break;

			case 'g':
				tfile = 6;
			break;

			case 'h':
				tfile = 7;
			break;
		};

		WRB_Chess::Piece promo;
		if (s.size() > 4)
		{
			switch(s[4])
			{
				case 'b':
					promo = WRB_Chess::Piece::Bishop;
				break;

				case 'r':
					promo = WRB_Chess::Piece::Rook;
				break;

				case 'n':
					promo = WRB_Chess::Piece::Knight;
				break;

				case 'q':
					promo = WRB_Chess::Piece::Queen;
				break;
			};
		}
		else
		{
			promo = WRB_Chess::Piece::Queen;
		}

		WRB_Chess::Move  mv;
		mv.fromSquare = WRB_Chess::RankAndFileToSquare(srank, sfile);
		mv.toSquare = WRB_Chess::RankAndFileToSquare(trank, tfile);
		mv.promotion = promo;

		return mv;
	}
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
	this->epDefender = -1;
}

void WRB_Chess::Bitboard::clear()
{
	this->color_masks[WRB_Chess::Color::White] = 0;
	this->color_masks[WRB_Chess::Color::Black] = 0;
	this->piece_masks[WRB_Chess::Piece::Pawn] = 0;
	this->piece_masks[WRB_Chess::Piece::Bishop] = 0;
	this->piece_masks[WRB_Chess::Piece::Rook] = 0;
	this->piece_masks[WRB_Chess::Piece::Knight] = 0;
	this->piece_masks[WRB_Chess::Piece::Queen] = 0;	
	this->piece_masks[WRB_Chess::Piece::King] = 0;
	this->queensideCastle[WRB_Chess::Color::White] = false;
	this->queensideCastle[WRB_Chess::Color::Black] = false;
	this->kingsideCastle[WRB_Chess::Color::White] = false;
	this->kingsideCastle[WRB_Chess::Color::Black] = false;
	this->epSquare = -1;
	this->epDefender = -1;
}

WRB_Chess::Bitboard::Bitboard(const Bitboard &bb)
{
	this->color_masks[WRB_Chess::Color::White] = bb.color_masks[WRB_Chess::Color::White];
	this->color_masks[WRB_Chess::Color::Black] = bb.color_masks[WRB_Chess::Color::Black];
	this->piece_masks[WRB_Chess::Piece::Pawn] = bb.piece_masks[WRB_Chess::Piece::Pawn];
	this->piece_masks[WRB_Chess::Piece::Bishop] = bb.piece_masks[WRB_Chess::Piece::Bishop];
	this->piece_masks[WRB_Chess::Piece::Rook] = bb.piece_masks[WRB_Chess::Piece::Rook];
	this->piece_masks[WRB_Chess::Piece::Knight] = bb.piece_masks[WRB_Chess::Piece::Knight];
	this->piece_masks[WRB_Chess::Piece::Queen] = bb.piece_masks[WRB_Chess::Piece::Queen];
	this->piece_masks[WRB_Chess::Piece::King] = bb.piece_masks[WRB_Chess::Piece::King];
	this->queensideCastle[WRB_Chess::Color::White] = bb.queensideCastle[WRB_Chess::Color::White];
	this->queensideCastle[WRB_Chess::Color::Black] = bb.queensideCastle[WRB_Chess::Color::Black];
	this->kingsideCastle[WRB_Chess::Color::White] = bb.kingsideCastle[WRB_Chess::Color::White];
	this->kingsideCastle[WRB_Chess::Color::Black] = bb.kingsideCastle[WRB_Chess::Color::Black];
	this->epSquare = bb.epSquare;
	this->epDefender = bb.epDefender;
}

WRB_Chess::Bitboard::Bitboard(std::vector<std::pair<short, WRB_Chess::ColorPiece>> mask)
{
	this->color_masks[WRB_Chess::Color::White] = 0;
	this->color_masks[WRB_Chess::Color::Black] = 0;
	this->piece_masks[WRB_Chess::Piece::Pawn] = 0;
	this->piece_masks[WRB_Chess::Piece::Bishop] = 0;
	this->piece_masks[WRB_Chess::Piece::Rook] = 0;
	this->piece_masks[WRB_Chess::Piece::Knight] = 0;
	this->piece_masks[WRB_Chess::Piece::Queen] = 0;	
	this->piece_masks[WRB_Chess::Piece::King] = 0;
	this->queensideCastle[WRB_Chess::Color::White] = false;
	this->queensideCastle[WRB_Chess::Color::Black] = false;
	this->kingsideCastle[WRB_Chess::Color::White] = false;
	this->kingsideCastle[WRB_Chess::Color::Black] = false;
	this->epSquare = -1;
	this->epDefender = -1;

	for (int i = 0; i < mask.size(); i++)
	{
		if ((mask[i].second.color != WRB_Chess::Color::NoColor) && (mask[i].second.piece != WRB_Chess::Piece::NoPiece))
		{
			this->color_masks[mask[i].second.color][mask[i].first] = true;
			this->piece_masks[mask[i].second.piece][mask[i].first] = true;
		}
	}
}

WRB_Chess::Bitboard& WRB_Chess::Bitboard::operator=(const Bitboard& bb)
{
	this->color_masks[WRB_Chess::Color::White] = bb.color_masks[WRB_Chess::Color::White];
	this->color_masks[WRB_Chess::Color::Black] = bb.color_masks[WRB_Chess::Color::Black];
	this->piece_masks[WRB_Chess::Piece::Pawn] = bb.piece_masks[WRB_Chess::Piece::Pawn];
	this->piece_masks[WRB_Chess::Piece::Bishop] = bb.piece_masks[WRB_Chess::Piece::Bishop];
	this->piece_masks[WRB_Chess::Piece::Rook] = bb.piece_masks[WRB_Chess::Piece::Rook];
	this->piece_masks[WRB_Chess::Piece::Knight] = bb.piece_masks[WRB_Chess::Piece::Knight];
	this->piece_masks[WRB_Chess::Piece::Queen] = bb.piece_masks[WRB_Chess::Piece::Queen];
	this->piece_masks[WRB_Chess::Piece::King] = bb.piece_masks[WRB_Chess::Piece::King];
	this->queensideCastle[WRB_Chess::Color::White] = bb.queensideCastle[WRB_Chess::Color::White];
	this->queensideCastle[WRB_Chess::Color::Black] = bb.queensideCastle[WRB_Chess::Color::Black];
	this->kingsideCastle[WRB_Chess::Color::White] = bb.kingsideCastle[WRB_Chess::Color::White];
	this->kingsideCastle[WRB_Chess::Color::Black] = bb.kingsideCastle[WRB_Chess::Color::Black];
	this->epSquare = bb.epSquare;
	this->epDefender = bb.epDefender;

	return *this;
}


// TODO make fen conversion
WRB_Chess::Bitboard::Bitboard(std::string fen)
{
	this->color_masks[WRB_Chess::Color::White] = 0b0;
	this->color_masks[WRB_Chess::Color::Black] = 0b0;
	this->piece_masks[WRB_Chess::Piece::Pawn] = 0b0;
	this->piece_masks[WRB_Chess::Piece::Bishop] = 0b0;
	this->piece_masks[WRB_Chess::Piece::Rook] = 0b0;
	this->piece_masks[WRB_Chess::Piece::Knight] = 0b0;
	this->piece_masks[WRB_Chess::Piece::Queen] = 0b0;	
	this->piece_masks[WRB_Chess::Piece::King] = 0b0;
	this->queensideCastle[WRB_Chess::Color::White] = false;
	this->queensideCastle[WRB_Chess::Color::Black] = false;
	this->kingsideCastle[WRB_Chess::Color::White] = false;
	this->kingsideCastle[WRB_Chess::Color::Black] = false;
	this->epSquare = -1;
	this->epDefender = -1;

	int component = 0;
	int rank = 7;
	int file = 0;

	int tempInt;
	for (int i = 0; i < fen.size(); i++)
	{
		switch(component)
		{
		case 0: // Getting piece info
			switch (fen[i])
			{
				case ' ':
				component++;
				break;

				case '/':
				rank -= 1;
				file = 0;
				break;

				case 'r':
				case 'R':
					if (fen[i] == 'R')
					{
						this->color_masks[WRB_Chess::Color::White][8*rank + file] = true;
					}
					else
					{
						this->color_masks[WRB_Chess::Color::Black][8*rank + file] = true;
					}

					this->piece_masks[WRB_Chess::Piece::Rook][8*rank + file] = true;

					file++;
				break;

				case 'p':
				case 'P':
					if (fen[i] == 'P')
					{
						this->color_masks[WRB_Chess::Color::White][8*rank + file] = true;
					}
					else
					{
						this->color_masks[WRB_Chess::Color::Black][8*rank + file] = true;
					}

					this->piece_masks[WRB_Chess::Piece::Pawn][8*rank + file] = true;

					file++;
				break;

				case 'n':
				case 'N':
					if (fen[i] == 'N')
					{
						this->color_masks[WRB_Chess::Color::White][8*rank + file] = true;
					}
					else
					{
						this->color_masks[WRB_Chess::Color::Black][8*rank + file] = true;
					}

					this->piece_masks[WRB_Chess::Piece::Knight][8*rank + file] = true;

					file++;
				break;

				case 'b':
				case 'B':
					if (fen[i] == 'B')
					{
						this->color_masks[WRB_Chess::Color::White][8*rank + file] = true;
					}
					else
					{
						this->color_masks[WRB_Chess::Color::Black][8*rank + file] = true;
					}

					this->piece_masks[WRB_Chess::Piece::Bishop][8*rank + file] = true;

					file++;
				break;

				case 'q':
				case 'Q':
					if (fen[i] == 'Q')
					{
						this->color_masks[WRB_Chess::Color::White][8*rank + file] = true;
					}
					else
					{
						this->color_masks[WRB_Chess::Color::Black][8*rank + file] = true;
					}

					this->piece_masks[WRB_Chess::Piece::Queen][8*rank + file] = true;

					file++;
				break;

				case 'k':
				case 'K':
					if (fen[i] == 'K')
					{
						this->color_masks[WRB_Chess::Color::White][8*rank + file] = true;
					}
					else
					{
						this->color_masks[WRB_Chess::Color::Black][8*rank + file] = true;
					}

					this->piece_masks[WRB_Chess::Piece::King][8*rank + file] = true;

					file++;
				break;

				case '1':
					file += 1;
				break;

				case '2':
					file += 2;
				break;

				case '3':
					file += 3;
				break;

				case '4':
					file += 4;
				break;

				case '5':
					file += 5;
				break;

				case '6':
					file += 6;
				break;

				case '7':
					file += 7;
				break;

				case '8':
					file += 8;
				break;

			};
		break;

		case 1: // To move, ignore
			i++;
			component++;
		break;

		case 3: // Castling info
			switch(fen[i])
			{
				case 'K':
					this->kingsideCastle[WRB_Chess::Color::White] = true;
				break;

				case 'k':
					this->kingsideCastle[WRB_Chess::Color::Black] = true;
				break;

				case 'Q':
					this->queensideCastle[WRB_Chess::Color::White] = true;
				break;

				case 'q':
					this->queensideCastle[WRB_Chess::Color::Black] = true;
				break;

				case ' ':
					component++;
				break;
			}
		break;

		case 4: // En passant info
			if (fen[i] != '-')
			{
				tempInt = 0;

				switch(fen[i])
				{
					case 'a':
						tempInt = 0;
					break;

					case 'b':
						tempInt = 1;
					break;

					case 'c':
						tempInt = 2;
					break;

					case 'd':
						tempInt = 3;
					break;

					case 'e':
						tempInt = 4;
					break;

					case 'f':
						tempInt = 5;
					break;

					case 'g':
						tempInt = 6;
					break;

					case 'h':
						tempInt = 7;
					break;
				}

				tempInt += (int)(fen[i+1] - '0') * 8;

				this->epSquare = tempInt;

				if (tempInt > 4 * 8)
					this->epDefender = tempInt + 8;
				else
					this->epDefender = tempInt - 8;
			}
		break;

		default: // End
			return;

		}
		
	}
}

std::string WRB_Chess::Bitboard::fen()
{
	std::string fen = "";
	for (int i = 7; i >= 0; i--) // Rank
	{
		if (i != 7)
			fen += "/";

		int nullCounter = 0;
		for (int j = 0; j < 8; j++) // File
		{
			WRB_Chess::ColorPiece pA = PieceAt(i*8 + j);

			if (pA.piece == WRB_Chess::Piece::NoPiece)
			{
				nullCounter++;
			}
			else
			{
				if (nullCounter != 0)
				{
					fen += std::to_string(nullCounter);
					nullCounter = 0;
				}

				char c = ' ';
				switch (pA.piece)
				{
					case WRB_Chess::NoPiece:
						c = '-';
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

				if (pA.color == WRB_Chess::Color::White)
				{
					c = toupper(c);
				}

				fen += c;
			} 
		}

		if (nullCounter != 0)
		{
			fen += std::to_string(nullCounter);
			nullCounter = 0;
		}
	}

	fen += " - ";

	if (kingsideCastle[0])
	{
		fen += "K";
	}

	if (queensideCastle[0])
	{
		fen += "Q";
	}

	if (kingsideCastle[1])
	{
		fen += "k";
	}

	if (queensideCastle[1])
	{
		fen += "q";
	}

	if (!queensideCastle[0] && !queensideCastle[1] && !kingsideCastle[0] && !kingsideCastle[1])
	{
		fen += "-";
	}

	fen += " ";

	if (epSquare != -1)
	{
		fen += WRB_Chess::squarenames[epSquare];
	}
	else
	{
		fen += "-";
	}

	fen += " 0 0";

	return fen;
}

bool WRB_Chess::Bitboard::operator==(const Bitboard& rhs) const
{
	return (this->color_masks[WRB_Chess::Color::White] == rhs.color_masks[WRB_Chess::Color::White])
		&& (this->color_masks[WRB_Chess::Color::Black] == rhs.color_masks[WRB_Chess::Color::Black])
		&& (this->piece_masks[WRB_Chess::Piece::Pawn] == rhs.piece_masks[WRB_Chess::Piece::Pawn])
		&& (this->piece_masks[WRB_Chess::Piece::Bishop] == rhs.piece_masks[WRB_Chess::Piece::Bishop])
		&& (this->piece_masks[WRB_Chess::Piece::Rook] == rhs.piece_masks[WRB_Chess::Piece::Rook])
		&& (this->piece_masks[WRB_Chess::Piece::Knight] == rhs.piece_masks[WRB_Chess::Piece::Knight])
		&& (this->piece_masks[WRB_Chess::Piece::Queen] == rhs.piece_masks[WRB_Chess::Piece::Queen])
		&& (this->piece_masks[WRB_Chess::Piece::King] == rhs.piece_masks[WRB_Chess::Piece::King])
		&& (this->queensideCastle[WRB_Chess::Color::White] == rhs.queensideCastle[WRB_Chess::Color::White])
		&& (this->queensideCastle[WRB_Chess::Color::Black] == rhs.queensideCastle[WRB_Chess::Color::Black])
		&& (this->kingsideCastle[WRB_Chess::Color::White] == rhs.kingsideCastle[WRB_Chess::Color::White])
		&& (this->kingsideCastle[WRB_Chess::Color::Black] == rhs.kingsideCastle[WRB_Chess::Color::Black])
		&& (this->epSquare == rhs.epSquare)
		&& (this->epDefender == rhs.epDefender);
}

WRB_Chess::ColorPiece WRB_Chess::Bitboard::PieceAt(short square) const
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

std::vector<std::pair<short, WRB_Chess::ColorPiece>> WRB_Chess::Bitboard::sense(short square) const
{
	std::vector<std::pair<short, WRB_Chess::ColorPiece>> vision;

	for (short i = -1; i <= 1; i++)
	{
		for (short j = -1; j <= 1; j++)
		{
			short convSq = square + i + 8*j;
			if ((convSq >= 0) && (convSq < 64))
			{
				vision.push_back(std::pair<short,  WRB_Chess::ColorPiece>(convSq, this->PieceAt(convSq)));
			}
		}
	}

	return vision;
}

WRB_Chess::Bitboard WRB_Chess::Bitboard::senseMask(short square) const
{
	WRB_Chess::Bitboard masked;
	masked.clear();

	std::bitset<64> mask;
	for (short i = -1; i <= 1; i++)
	{
		for (short j = -1; j <= 1; j++)
		{
			short convSq = square + i + 8*j;
			if ((convSq >= 0) && (convSq < 64))
			{
				mask[convSq] = 1;
			}
		}
	}

	masked.color_masks[0] = this->color_masks[0] & mask;
	masked.color_masks[1] = this->color_masks[1] & mask;

	masked.piece_masks[0] = this->piece_masks[0] & mask;
	masked.piece_masks[1] = this->piece_masks[1] & mask;
	masked.piece_masks[2] = this->piece_masks[2] & mask;
	masked.piece_masks[3] = this->piece_masks[3] & mask;
	masked.piece_masks[4] = this->piece_masks[4] & mask;
	masked.piece_masks[5] = this->piece_masks[5] & mask;

	return masked;
}

std::vector<WRB_Chess::Move> WRB_Chess::Bitboard::Attacks(Color c, short t)
{
	std::vector<WRB_Chess::Move> mvs;

	std::bitset<64> allPieces = this->color_masks[0] | this->color_masks[1];

	// Find potential pawn attacks
	if (c == WRB_Chess::Color::White)
	{
		if ((t - 7) >= 0)
		{
			if (this->piece_masks[WRB_Chess::Piece::Pawn][t - 7] && this->color_masks[c][t - 7])
			{
				WRB_Chess::Move pawnAttack;
				pawnAttack.fromSquare = t - 7;
				pawnAttack.toSquare = t;
				mvs.push_back(pawnAttack);
			}
		}

		if ((t - 9) >= 0)
		{
			if (this->piece_masks[WRB_Chess::Piece::Pawn][t - 9] && this->color_masks[c][t - 9])
			{
				WRB_Chess::Move pawnAttack;
				pawnAttack.fromSquare = t - 9;
				pawnAttack.toSquare = t;
				mvs.push_back(pawnAttack);
			}
		}
	}
	else if (c == WRB_Chess::Color::Black)
	{
		if ((t + 7) < 64)
		{
			if (this->piece_masks[WRB_Chess::Piece::Pawn][t + 7] && this->color_masks[c][t + 7])
			{
				WRB_Chess::Move pawnAttack;
				pawnAttack.fromSquare = t + 7;
				pawnAttack.toSquare = t;
				mvs.push_back(pawnAttack);
			}
		}

		if ((t + 9) < 64)
		{
			if (this->piece_masks[WRB_Chess::Piece::Pawn][t + 9] && this->color_masks[c][t + 9])
			{
				WRB_Chess::Move pawnAttack;
				pawnAttack.fromSquare = t + 9;
				pawnAttack.toSquare = t;
				mvs.push_back(pawnAttack);
			}
		}
	}

	// Find potential knight attacks
	short potAtt [8] = { t + 6, t - 10, t + 15, t - 17, t + 17, t - 15, t + 10, t - 6 };
	for (int i = 0; i < 8; i++)
	{
		if ((potAtt[i] >= 0) && (potAtt[i] < 64))
		{
			if (this->piece_masks[WRB_Chess::Piece::Knight][potAtt[i]] && this->color_masks[c][potAtt[i]])
			{
				WRB_Chess::Move knightAttack;
				knightAttack.fromSquare = potAtt[i];
				knightAttack.toSquare = t;
				mvs.push_back(knightAttack);
			}
		}
	}

	// Find potential king attacks
	short pKA [8] = {t - 1, t +7, t - 9, t + 8, t - 8, t +9, t -7, t + 1};
	for (int i = 0; i < 8; i++)
	{
		if ((pKA[i] >= 0) && (pKA[i] < 64))
		{
			if (this->piece_masks[WRB_Chess::Piece::King][pKA[i]] && this->color_masks[c][pKA[i]])
			{
				WRB_Chess::Move kingAttack;
				kingAttack.fromSquare = pKA[i];
				kingAttack.toSquare = t;
				mvs.push_back(kingAttack);
			}
		}
	}

	// Find potential rook/queen attacks
	short stepLeft = 1;
	short stepUp = 0;
	do
	{
		short scan = t;

		do
		{
			if ((((scan % 8) + stepLeft) < 0) || (((scan % 8) + stepLeft) > 7))
			{
				break;
			}
			else if ((((scan / 8) + stepUp) < 0) || (((scan / 8) + stepUp) > 7))
			{
				break;
			}
			scan = scan + stepLeft + 8 * stepUp;
			if (this->color_masks[c][scan] && (this->piece_masks[WRB_Chess::Piece::Queen] | this->piece_masks[WRB_Chess::Piece::Rook])[scan])
			{
				WRB_Chess::Move slideAttack;
				slideAttack.fromSquare = scan;
				slideAttack.toSquare = t;
				mvs.push_back(slideAttack);
			}

		} while (!allPieces[scan]);

		short tmp = stepUp;
		stepUp = -stepLeft;
		stepLeft = tmp;
	} while ((stepLeft != 1) && (stepUp != 0));

	// Find potential bishop/queen attacks
	stepLeft = 1;
	stepUp = 1;
	do
	{
		short scan = t;

		do
		{
			if ((((scan % 8) + stepLeft) < 0) || (((scan % 8) + stepLeft) > 7))
			{
				break;
			}
			else if ((((scan / 8) + stepUp) < 0) || (((scan / 8) + stepUp) > 7))
			{
				break;
			}
			scan = scan + stepLeft + 8 * stepUp;
			if (this->color_masks[c][scan] && (this->piece_masks[WRB_Chess::Piece::Queen] | this->piece_masks[WRB_Chess::Piece::Bishop])[scan])
			{
				WRB_Chess::Move slideAttack;
				slideAttack.fromSquare = scan;
				slideAttack.toSquare = t;
				mvs.push_back(slideAttack);
			}

		} while (!allPieces[scan]);

		short tmp = stepUp;
		stepUp = -stepLeft;
		stepLeft = tmp;
	} while ((stepLeft != 1) && (stepUp != 1));

	return mvs;
}

std::vector<WRB_Chess::Move> WRB_Chess::Bitboard::AvailableMoves(WRB_Chess::Color c) const
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

		if (((this->piece_masks[WRB_Chess::Piece::Bishop] | this->piece_masks[WRB_Chess::Piece::Queen]) & this->color_masks[c])[i])
		{
			// Generate Bishop moves
			short stepLeft = 1;
			short stepUp = 1;

			do
			{
				// Generate all moves along that direction
				short pos = i;
				do
				{
					if ((((pos % 8) + stepLeft) < 0) || (((pos % 8) + stepLeft) > 7))
					{
						break;
					}
					else if ((((pos / 8) + stepUp) < 0) || (((pos / 8) + stepUp) > 7))
					{
						break;
					}
					pos = pos + stepLeft + 8 * stepUp;
					if (this->color_masks[c][pos])
						break;

					WRB_Chess::Move slide;
					slide.fromSquare = i;
					slide.toSquare = pos;
					mvs.push_back(slide);
				}while (!this->color_masks[c][pos]);


				short t = stepLeft;
				stepLeft = stepUp;
				stepUp = -t;
			} while ((stepLeft != 1) || (stepUp != 1));
		}
		
		if (((this->piece_masks[WRB_Chess::Piece::Rook] | this->piece_masks[WRB_Chess::Piece::Queen]) & this->color_masks[c])[i])
		{
			// Generate Rook moves
			short stepLeft = 0;
			short stepUp = 1;

			do
			{
				// Generate all moves along that direction
				short pos = i;
				do
				{
					if ((((pos % 8) + stepLeft) < 0) || (((pos % 8) + stepLeft) > 7))
					{
						break;
					}
					else if ((((pos / 8) + stepUp) < 0) || (((pos / 8) + stepUp) > 7))
					{
						break;
					}
					pos = pos + stepLeft + 8 * stepUp;
					if (this->color_masks[c][pos])
						break;

					WRB_Chess::Move slide;
					slide.fromSquare = i;
					slide.toSquare = pos;
					mvs.push_back(slide);
				}while (!this->color_masks[c][pos]);


				short t = stepLeft;
				stepLeft = stepUp;
				stepUp = -t;
			} while ((stepLeft != 0) || (stepUp != 1));
		}

		if ((this->piece_masks[WRB_Chess::Piece::Knight] & this->color_masks[c])[i])
		{
			short potAtt [8] = { i + 6, i - 10, i + 15, i - 17, i + 17, i - 15, i + 10, i - 6 };
			for (int k = 0; k < 8; k++)
			{
				if ((potAtt[k] < 0) || (potAtt[k] >= 64) || this->color_masks[c][potAtt[k]])
					continue;

				if ((abs(potAtt[k]/8 - i/8) != 1) && (abs(potAtt[k]/8 - i/8) != 2))
					continue;

				if ((abs(potAtt[k]%8 - i%8) != 1) && ((abs(potAtt[k]%8 - i%8) != 2)))
					continue;

				WRB_Chess::Move knightMove;
				knightMove.fromSquare = i;
				knightMove.toSquare = potAtt[k];
				mvs.push_back(knightMove);
			}
		}

		if ((this->piece_masks[WRB_Chess::Piece::King] & this->color_masks[c])[i])
		{
			// First generate normal moves
			for (int l = -1; l <= 1; l++)
			{
				for (int u = -1; u <= 1; u++)
				{
					if ((l == 0) && (u == 0))
						continue;

					if ((((i % 8) + l) < 0) || (((i % 8) + l) > 7))
					{
						continue;
					}
					else if ((((i / 8) + u) < 0) || (((i / 8) + u) > 7))
					{
						continue;
					}
					else if (this->color_masks[c][i + l + 8 * u])
					{
						continue;
					}

					WRB_Chess::Move slide;
					slide.fromSquare = i;
					slide.toSquare = i + l + 8 * u;
					mvs.push_back(slide);
				}
			}
			// Then generate castles

			if ((this->kingsideCastle[c]) && ((i%8) == 4))
			{
				if (!(this->color_masks[c][i + 1] || this->color_masks[c][i+2]) && (this->color_masks[c] & this->piece_masks[WRB_Chess::Piece::Rook])[i+3])
				{
					WRB_Chess::Move slide;
					slide.fromSquare = i;
					slide.toSquare = i + 2;
					mvs.push_back(slide);
				}
			}

			if ((this->queensideCastle[c]) && ((i%8) == 4))
			{
				if (!(this->color_masks[c][i - 1] || this->color_masks[c][i-2] || this->color_masks[c][i-3]) && (this->color_masks[c] & this->piece_masks[WRB_Chess::Piece::Rook])[i-4])
				{
					WRB_Chess::Move slide;
					slide.fromSquare = i;
					slide.toSquare = i - 2;
					mvs.push_back(slide);
				}
			}
		}

	}

	return mvs;
}

#define SCAN_SLIDE_STEP(a) ((a) > 0 ? 1 : (a) < 0 ? -1 : 0)
#include <iostream>

WRB_Chess::Move WRB_Chess::Bitboard::RectifySlide(WRB_Chess::Move m, bool canCapture) const
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
				if (pScan == m.fromSquare)
					return WRB_Chess::Move();

				WRB_Chess::Move newM;
				newM.fromSquare = m.fromSquare;
				newM.toSquare = pScan;
				newM.promotion = m.promotion;
				return newM;
			}
			else if (pAt.color != pMove.color)
			{
				if (scan == m.fromSquare)
					return WRB_Chess::Move();

				WRB_Chess::Move newM;
				newM.fromSquare = m.fromSquare;
				newM.toSquare = scan;
				newM.promotion = m.promotion;
				return newM;
			}
		}
	}

	return m;
}

WRB_Chess::Move WRB_Chess::Bitboard::RectifyMove(WRB_Chess::Move m) const
{
	WRB_Chess::ColorPiece pMove = this->PieceAt(m.fromSquare);
	if ((pMove.piece == WRB_Chess::Piece::NoPiece) || (pMove.color == WRB_Chess::Color::NoColor))
	{
		return WRB_Chess::Move();
	}

	if (pMove.piece == WRB_Chess::Piece::Pawn)
	{
		short mDist = ManhattanDistance(m.fromSquare, m.toSquare);
		if (mDist == 1)
		{
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
	else if (pMove.piece == WRB_Chess::Piece::King)
	{
		short mDist = ManhattanDistance(m.fromSquare, m.toSquare);
		if (mDist > 2)
		{
			return Move();
		}
		if ((mDist == 1) || (((m.toSquare / 8) != (m.fromSquare / 8)) && ((m.toSquare % 8) != (m.fromSquare % 8))))
		{
			// Regular move
			WRB_Chess::ColorPiece pAt = this->PieceAt(m.toSquare);
			if ((pAt.color == pMove.color))
			{
				return Move();
			}
		}
		else
		{
			// Castle
			std::bitset<64> allPieces = this->color_masks[0]|this->color_masks[1];
			int i = m.fromSquare;
			if ((this->kingsideCastle[pMove.color]) && ((i%8) == 4) && (m.toSquare == (i + 2)))
			{
				if (!(allPieces[i + 1] || allPieces[i+2]) && (this->color_masks[pMove.color] & this->piece_masks[WRB_Chess::Piece::Rook])[i+3])
				{
				}
				else
				{
					return Move();
				}
			}
			else if ((this->queensideCastle[pMove.color]) && ((i%8) == 4) && (m.toSquare == (i - 2)))
			{
				if (!(allPieces[i - 1] || allPieces[i-2] || allPieces[i-3]) && (this->color_masks[pMove.color] & this->piece_masks[WRB_Chess::Piece::Rook])[i-4])
				{
				}
				else
				{
					return Move();
				}
			}
			else
			{
				return Move();
			}
		}

	}
	else if (pMove.piece != WRB_Chess::Piece::Knight)
	{
		return this->RectifySlide(m, true);
	}

	return m;
}

WRB_Chess::Move WRB_Chess::Bitboard::ApplyMove(WRB_Chess::Move m, bool& capture, short& captureSquare)
{
	WRB_Chess::Move taken = this->RectifyMove(m);
	capture = false;
	captureSquare = -1;

	if (taken.fromSquare == -1 || taken.toSquare == -1 || taken.fromSquare == taken.toSquare)
	{	
		// Null move
		return WRB_Chess::Move();
	}

	std::bitset<64> mvMask;
	mvMask[taken.fromSquare] = true;
	mvMask[taken.toSquare] = true;

	bool resetEP = true;
	bool promo = false;

	for (int i = 0; i < 6; i++)
	{
		if (this->piece_masks[i][taken.toSquare])
		{
			this->piece_masks[i][taken.toSquare] = false;
		}
	}

	for (int i = 0; i < 6; i++)
	{
		if (this->piece_masks[i][taken.fromSquare])
		{
			// If the king or rook moves we lose castle
			if (i == WRB_Chess::Piece::King)
			{
				//Check if we are castling first
				if ((ManhattanDistance(taken.fromSquare, taken.toSquare) == 2) && ((taken.fromSquare/8) == (taken.toSquare/8)))
				{
					// We are castling
					if ((taken.toSquare - taken.fromSquare) == 2)
					{
						// Kingside Castle
						short rookSquare = (taken.fromSquare / 8) * 8 + 7;
						if (this->color_masks[0][taken.fromSquare])
						{
							this->color_masks[0][rookSquare] = false;
							this->color_masks[0][rookSquare - 2] = true;
						}
						else if (this->color_masks[1][taken.fromSquare])
						{
							this->color_masks[1][rookSquare] = false;
							this->color_masks[1][rookSquare - 2] = true;
						}

						this->piece_masks[WRB_Chess::Piece::Rook][rookSquare] = false;
						this->piece_masks[WRB_Chess::Piece::Rook][rookSquare - 2] = true;
					}
					else
					{
						// Queenside Castle
						short rookSquare = (taken.fromSquare / 8) * 8;
						if (this->color_masks[0][taken.fromSquare])
						{
							this->color_masks[0][rookSquare] = false;
							this->color_masks[0][rookSquare + 3] = true;
						}
						else if (this->color_masks[1][taken.fromSquare])
						{
							this->color_masks[1][rookSquare] = false;
							this->color_masks[1][rookSquare + 3] = true;
						}

						this->piece_masks[WRB_Chess::Piece::Rook][rookSquare] = false;
						this->piece_masks[WRB_Chess::Piece::Rook][rookSquare + 3] = true;
					}
				}
				
				if (this->color_masks[0][taken.fromSquare])
				{
					this->queensideCastle[0] = false;
					this->kingsideCastle[0] = false;
				}
				else if (this->color_masks[1][taken.fromSquare])
				{
					this->queensideCastle[1] = false;
					this->kingsideCastle[1] = false;
				}
			}
			else if (i == WRB_Chess::Piece::Rook)
			{
				if (taken.fromSquare == 0)
				{
					this->queensideCastle[0] = false;
				}
				else if (taken.fromSquare == 7)
				{
					this->kingsideCastle[0] = false;
				}
				else if (taken.fromSquare == (7 * 8))
				{
					this->queensideCastle[1] = false;
				}
				else if (taken.fromSquare == (7 * 8 + 7))
				{
					this->kingsideCastle[1] = false;
				}
			}
			else if (i == WRB_Chess::Piece::Pawn)
			{
				if (taken.toSquare == this->epSquare)
				{	// This is an en-passant
					if (this->color_masks[0][taken.fromSquare])
					{
						this->color_masks[1][this->epDefender] = false;
					}
					else if (this->color_masks[1][taken.fromSquare])
					{
						this->color_masks[0][this->epDefender] = false;
					}

					capture = true;
					captureSquare = this->epDefender;


					this->piece_masks[WRB_Chess::Piece::Pawn][this->epDefender] = false;
				}

				if (((taken.fromSquare % 8) == (taken.toSquare % 8)) && (ManhattanDistance(taken.fromSquare, taken.toSquare) == 2))
				{
					// Move two, so open to EP
					resetEP = false;
					this->epDefender = taken.toSquare;
					this->epSquare = (taken.fromSquare + taken.toSquare)/2;
				}

				if (((taken.toSquare >= 56) && this->color_masks[0][taken.fromSquare]) || ((taken.toSquare <= 7) && this->color_masks[1][taken.fromSquare]))
				{
					if ((taken.promotion <= 1) || (taken.promotion > 4))
						taken.promotion = WRB_Chess::Piece::Queen;

					this->piece_masks[taken.promotion][taken.toSquare] = true;
					this->piece_masks[i][taken.fromSquare] = false;
					promo = true;
				}
			}

			if (!promo)
			{
				this->piece_masks[i][taken.toSquare] = true;
				this->piece_masks[i][taken.fromSquare] = false;
			}

			break;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (this->color_masks[i][taken.toSquare])
		{
			this->color_masks[i][taken.toSquare] = false;
			capture = true;
			captureSquare = taken.toSquare;
			break;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (this->color_masks[i][taken.fromSquare])
		{
			this->color_masks[i][taken.toSquare] = true;
			this->color_masks[i][taken.fromSquare] = false;
			break;
		}
	}

	if (resetEP)
	{
		this->epSquare = -1;
		this->epDefender = -1;
	}

	return taken;
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
					c = '-';
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

