#ifndef WRB_BITBOARD
#define WRB_BITBOARD

#include <cstdint>
#include <vector>
#include <bitset>
#include <string>

#include <iostream>

namespace WRB_Chess
{
	enum File
	{
		A = 0,
		B = 1,
		C = 2,
		D = 3,
		E = 4,
		F = 5,
		G = 6,
		H = 7
	};

	extern const char* SquareNames[];
	extern const char* PieceNames[];
	extern const char* ColorNames[];

	inline const char* GetPieceName(short n) { return (n < 0 || n > 5)? "None" : PieceNames[n];};
	inline const char* GetColorName(short n) { return (n < 0 || n > 1)? "None" : ColorNames[n];};

	short RankAndFileToSquare(short rank, short file); // Keep in mind that rank is 1 indexed and file is 0 indexed
	short RankAndFileToSquare(short rank, File file);

	short ManhattanDistance(short sq1, short sq2);

	std::bitset<64> SquareToMask(short);
	std::bitset<64> SquareToMask(std::vector<short>);
	std::vector<short> MaskToSquares(std::bitset<64>);

	enum Color
	{
		NoColor = -1,
		White = 0,
		Black = 1
	};

	enum Piece
	{
		NoPiece = -1,
		Pawn = 0,
		Bishop = 1,
		Rook = 2,
		Knight = 3,
		Queen = 4,
		King = 5
	};

	struct Move
	{
		short fromSquare = -1;
		short toSquare = -1;
		Piece promotion = WRB_Chess::Piece::Queen;
	};

	struct ColorPiece
	{
		Piece piece = WRB_Chess::Piece::NoPiece;
		Color color = WRB_Chess::Color::NoColor;
	};

	class Bitboard
	{
		protected:
			std::bitset<64> color_masks[2];
			std::bitset<64> piece_masks[6];
			short epSquare;
			short epDefender; // Square that the pawn vulnerable to en-passant is on
			bool queensideCastle[2];
			bool kingsideCastle[2];

			Move RectifySlide(Move m, bool canCapture);
		public:
			Bitboard();
			Bitboard(const Bitboard &bb); // Copy constructor
			inline std::bitset<64> Pieces(Color c) { return color_masks[c]; };
			inline std::bitset<64> Pieces(Color c, Piece p) { return color_masks[c] & piece_masks[p];};
			inline std::bitset<64> Pieces(ColorPiece p) { return color_masks[p.color] & piece_masks[p.piece];};
			inline std::bitset<64> Pieces() { return color_masks[Color::White] | color_masks[Color::Black]; };
			inline std::bitset<64> Pawns() { return piece_masks[Piece::Pawn]; };
			inline std::bitset<64> Bishops() { return piece_masks[Piece::Bishop]; };
			inline std::bitset<64> Rooks() { return piece_masks[Piece::Rook]; };
			inline std::bitset<64> Knights() { return piece_masks[Piece::Knight]; };
			inline std::bitset<64> Queens() { return piece_masks[Piece::Queen]; };
			inline std::bitset<64> Kings() { return piece_masks[Piece::King]; };

			ColorPiece PieceAt(short square);

			std::vector<Move> AvailableMoves(Color c);
			Move RectifyMove(Move m);
			Move ApplyMove(Move m, bool& capture, short& captureSquare);
	};

	std::string GetPrintable(Bitboard);
	std::string GetPrintable(Move);
}

#endif
