#pragma once

#include <cstdint>
#include <map>
#include <array>

namespace fogchess {

    const uint8_t PIECE_COLOR_MASK = 0b01100000;

    enum piece_t : uint8_t  {
        EMPTY       = 0,
        PAWN        = 1 << 0,
        KING        = 1 << 1,
        KNIGHT      = 1 << 2,
        BISHOP      = 1 << 3,
        ROOK        = 1 << 4,
        QUEEN       = BISHOP | ROOK,
        BLACK       = 1 << 5,
        WHITE       = 1 << 6,
        UNKNOWN     = 1 << 7,
    };

    struct castling_info_t {
        uint8_t white_king_moved;
        uint8_t white_kingside_rook_moved;
        uint8_t white_queenside_rook_moved;
        uint8_t black_king_moved;
        uint8_t black_kingside_rook_moved;
        uint8_t black_queenside_rook_moved;
    };

    struct cell_t { int cell_id; };

    struct move_t {
        cell_t start_cell;
        cell_t end_cell;
    };

    struct real_board_t {
        std::array<piece_t, 64> board;
        move_t last_move;
        castling_info_t info;
    };

    struct player_board_t { std::array<piece_t, 64> board; };
}