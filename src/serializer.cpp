#include "serializer.hpp"

#include <sstream>

namespace fogchess
{
    const piece_t char_to_piece(char c)
    {
        switch (c) {
        case 'P': return static_cast<piece_t>(PAWN | WHITE);
        case 'p': return static_cast<piece_t>(PAWN | BLACK);
        case 'R': return static_cast<piece_t>(ROOK | WHITE);
        case 'r': return static_cast<piece_t>(ROOK | BLACK);
        case 'N': return static_cast<piece_t>(KNIGHT | WHITE);
        case 'n': return static_cast<piece_t>(KNIGHT | BLACK);
        case 'B': return static_cast<piece_t>(BISHOP | WHITE);
        case 'b': return static_cast<piece_t>(BISHOP | BLACK);
        case 'Q': return static_cast<piece_t>(QUEEN | WHITE);
        case 'q': return static_cast<piece_t>(QUEEN | BLACK);
        case 'K': return static_cast<piece_t>(KING | WHITE);
        case 'k': return static_cast<piece_t>(KING | BLACK);
        case '.': return static_cast<piece_t>(EMPTY);
        default:  return static_cast<piece_t>(UNKNOWN);
        }
    }

    const char* piece_to_char(piece_t piece)
    {
        switch (piece & 0x9f) {
        case PAWN:   return (piece & WHITE) ? "P" : "p";
        case ROOK:   return (piece & WHITE) ? "R" : "r";
        case KNIGHT: return (piece & WHITE) ? "N" : "n";
        case BISHOP: return (piece & WHITE) ? "B" : "b";
        case QUEEN:  return (piece & WHITE) ? "Q" : "q";
        case KING:   return (piece & WHITE) ? "K" : "k";
        case EMPTY:  return ".";
        default:     return "#";
        }
    }

    std::string serialize_board(const player_board_t& board)
    {
        std::ostringstream oss;
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 0; file < 8; ++file) {
                int cell_id = rank * 8 + file;
                oss << piece_to_char(board.board[cell_id]);
            }
            if (rank > 0) {
                oss << '\n';
            }
        }
        return oss.str();
    }

    player_board_t deserialize_board(const std::string& board_str)
    {
        player_board_t board;
        board.board.fill(EMPTY);

        std::istringstream iss(board_str);
        std::string token;
        int index = 0;

        while (std::getline(iss, token, '\n') && index < 64) {
            for (char c : token) {
                board.board[index] = char_to_piece(c);
                index++;
            }
        }

        return board;
    }

}