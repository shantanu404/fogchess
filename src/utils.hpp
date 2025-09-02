#pragma once

#include <memory>
#include <vector>

#include "common.hpp"

namespace fogchess {
    bool is_last_move_was_double_pawn_push(const real_board_t& board);
    bool is_valid(std::pair<int, int> rank_and_file);

    piece_t get_piece_at_cell(const real_board_t& board, const cell_t& cell);

    std::pair<int, int> get_rank_and_file_from_cell(const cell_t& cell);

    real_board_t board_from_fen(const std::string& fen_notation);
    void print_real_board(const real_board_t& board, std::ostream& os);

    player_board_t board_for_player(const real_board_t& board, bool is_player_white);

    std::vector<move_t> get_move(const real_board_t& board, cell_t location);

    std::vector<move_t> melee_move(const real_board_t& board, cell_t location, const std::vector<std::pair<int, int>>& directions);
    std::vector<move_t> sliding_move(const real_board_t& board, cell_t location, const std::vector<std::pair<int, int>>& directions);

    std::vector<move_t> pawn_move(const real_board_t& board, cell_t location);
    std::vector<move_t> king_move(const real_board_t& board, cell_t location);
    std::vector<move_t> knight_move(const real_board_t& board, cell_t location);

    std::vector<move_t> bishop_move(const real_board_t& board, cell_t location);
    std::vector<move_t> rook_move(const real_board_t& board, cell_t location);

    std::vector<move_t> queen_move(const real_board_t& board, cell_t location);
}
