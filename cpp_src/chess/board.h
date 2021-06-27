#ifndef WRB_BITBOARD
#define WRB_BITBOARD

#include <cstdint>

namespace WRB_Chess
{
	enum Color
	{
		White = 0,
		Black = 1
	};

	enum Piece
	{
		Pawn = 0,
		Bishop = 1,
		Rook = 2,
		Knight = 3,
		Queen = 4,
		King = 5
	};

	class Bitboard
	{
		private:
			uint64_t color_masks[2];
			uint64_t piece_masks[6];
			bool queensideCastle[2];
			bool kingsideCastle[2];
		public:
			Bitboard();
			inline uint64_t Pieces(Color c) { return color_masks[c]; };
			inline uint64_t Pieces(Color c, Piece p) { return color_masks[c] & piece_masks[p];};
			inline uint64_t Pieces() { return color_masks[Color::White] | color_masks[Color::Black]; };
			inline uint64_t Pawns() { return piece_masks[Piece::Pawn]; };
			inline uint64_t Bishops() { return piece_masks[Piece::Bishop]; };
			inline uint64_t Rooks() { return piece_masks[Piece::Rook]; };
			inline uint64_t Knights() { return piece_masks[Piece::Knight]; };
			inline uint64_t Queens() { return piece_masks[Piece::Queen]; };
			inline uint64_t Kings() { return piece_masks[Piece::King]; };
	};
}

#endif
