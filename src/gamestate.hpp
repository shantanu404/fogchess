#pragma once

#include <string>

#include "common.hpp"

namespace fogchess
{
    class GameState
    {
    private:
        real_board_t board;
        player_board_t white_player;
        player_board_t black_player;

        uint8_t winner;

        bool is_player_white_turn;

    public:
        GameState(const std::string& fen);

        bool make_move(const move_t& move);
        bool has_winner() const;
        int get_winner() const;

        // Getters for private members
        const real_board_t& get_board() const { return board; }
        const player_board_t& get_white_player() const { return white_player; }
        const player_board_t& get_black_player() const { return black_player; }
        bool is_white_turn() const { return is_player_white_turn; }
        uint8_t get_winner_raw() const { return winner; }

        // Make move validation public
        bool is_valid_move(const move_t& move) const;
    };
}