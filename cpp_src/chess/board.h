#ifndef WRB_BITBOARD
#define WRB_BITBOARD

#include <cstdint>
#include <vector>
#include <bitset>
#include <string>
#include <ctime>
#include <string>

#include <iostream>

#define OPPOSITE_COLOR(c) (c == WRB_Chess::Color::White ? WRB_Chess::Color::Black : (c != WRB_Chess::Color::NoColor ? WRB_Chess::Color::White : WRB_Chess::Color::NoColor))

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
	extern const char* squarenames[];
	extern const char* PieceNames[];
	extern const char* ColorNames[];

	inline const char* GetPieceName(short n) { return (n < 0 || n > 5)? "None" : PieceNames[n];};
	inline const char* GetColorName(short n) { return (n < 0 || n > 1)? "None" : ColorNames[n];};

	extern short RankAndFileToSquare(short rank, short file); // Keep in mind that rank is 1 indexed and file is 0 indexed
	extern short RankAndFileToSquare(short rank, File file);

	extern short ManhattanDistance(short sq1, short sq2);

	extern std::bitset<64> SquareToMask(short);
	extern std::bitset<64> SquareToMask(std::vector<short>);
	extern std::vector<short> MaskToSquares(std::bitset<64>);

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
		Piece promotion = WRB_Chess::Piece::NoPiece;
		bool operator==(const Move& rhs) const
		{
			return (this->fromSquare == rhs.fromSquare) && (this->toSquare == rhs.toSquare) && (this->promotion == rhs.promotion);
		}
		inline bool operator!=(const Move& rhs) const { return !(*this == rhs); }
	};

	struct ColorPiece
	{
		Piece piece = WRB_Chess::Piece::NoPiece;
		Color color = WRB_Chess::Color::NoColor;
		bool operator==(const ColorPiece& rhs) const
		{
			return (this->piece == rhs.piece) && (this->color == rhs.color);
		}
		inline bool operator!=(const ColorPiece& rhs) const { return !(*this == rhs); }
	};

	extern Move UCI_to_move(std::string);
	extern std::string move_to_UCI(Move);

	class Bitboard
	{
		protected:
			std::bitset<64> color_masks[2];
			std::bitset<64> piece_masks[6];
			short epSquare;
			short epDefender; // Square that the pawn vulnerable to en-passant is on
			bool queensideCastle[2];
			bool kingsideCastle[2];

			Move RectifySlide(Move m, bool canCapture) const;
		public:
			Bitboard();
			Bitboard(const Bitboard &bb); // Copy constructor
			Bitboard(std::vector<std::pair<short, WRB_Chess::ColorPiece>>);
			Bitboard(std::string); // Get bitboard from FEN

			std::string fen();

			Bitboard& operator=(const Bitboard& other);
			bool operator==(const Bitboard& rhs) const;
			inline bool operator!=(const Bitboard& rhs) const { return !(*this == rhs); }
			void clear();

			inline short getEP() const { return epSquare; };
			inline bool queenCastle(bool b) const { return queensideCastle[b];};
			inline bool kingCastle(bool b) const { return kingsideCastle[b];};
			inline std::bitset<64> Pieces(Color c) const { return color_masks[c]; };
			inline std::bitset<64> Pieces(Color c, Piece p) const { return color_masks[c] & piece_masks[p];};
			inline std::bitset<64> Pieces(ColorPiece p) const { return color_masks[p.color] & piece_masks[p.piece];};
			inline std::bitset<64> Pieces() const { return color_masks[Color::White] | color_masks[Color::Black]; };
			inline std::bitset<64> Pawns() const { return piece_masks[Piece::Pawn]; };
			inline std::bitset<64> Bishops() const { return piece_masks[Piece::Bishop]; };
			inline std::bitset<64> Rooks() const { return piece_masks[Piece::Rook]; };
			inline std::bitset<64> Knights() const { return piece_masks[Piece::Knight]; };
			inline std::bitset<64> Queens() const { return piece_masks[Piece::Queen]; };
			inline std::bitset<64> Kings() const { return piece_masks[Piece::King]; };

			inline bool KingsAlive() const { return ((piece_masks[Piece::King] & color_masks[Color::White]) != 0) && ((piece_masks[Piece::King] & color_masks[Color::Black]) != 0); };

			ColorPiece PieceAt(short square) const;
			std::vector<std::pair<short, WRB_Chess::ColorPiece>> sense(short square) const;
			Bitboard senseMask(short square) const;

			std::vector<Move> AvailableMoves(Color c) const;
			std::vector<Move> Attacks(Color c, short t);
			Move RectifyMove(Move m) const;
			Move ApplyMove(Move m, bool& capture, short& captureSquare);
			Move ApplyMove(Move m)
			{
				bool cap;
				short cS;
				return ApplyMove(m, cap, cS);
			};

			friend class BoardHash;
	};


	class BoardHash
	{
	public:
		static std::size_t hashKey[64][12];

		static void Init()
		{
			srand(time(0));

			for (int i = 0; i < 64; i++)
			{
				for (int j = 0; j < 12; j++)
				{
					BoardHash::hashKey[i][j] = rand();
				}
			}
		}

		std::size_t operator()(const Bitboard& brd) const
		{
			std::size_t hash = 0;

			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 6; j++)
				{
					for (int k = 0; k < 64; k++)
					{
						if ((brd.color_masks[i] & brd.piece_masks[j])[k])
							hash = hash ^ BoardHash::hashKey[k][j + 6 * i];
					}
				}
			}

			return hash;
		}
	};

	class MoveHash
	{
	public:
		
		long operator()(const Move& mv) const
		{
			long hash = ((mv.toSquare << 8) | mv.fromSquare) | (((short) mv.promotion) << 16);

			return hash;
		}
	};

	extern std::string GetPrintable(Bitboard);
	extern std::string GetPrintable(Move);
}

#endif
