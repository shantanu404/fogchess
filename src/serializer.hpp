#pragma once

#include <string>

#include "common.hpp"

namespace fogchess
{
    const piece_t char_to_piece(char c);
    const char* piece_to_char(piece_t piece);

    std::string serialize_board(const player_board_t& board);
    player_board_t deserialize_board(const std::string& board_str);
}