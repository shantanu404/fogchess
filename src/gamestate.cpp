#include "gamestate.hpp"

#include "utils.hpp"

namespace fogchess
{
    bool GameState::is_valid_move(const move_t& move) const
    {
        auto from = move.start_cell;
        auto to = move.end_cell;

        auto [rank0, file0] = get_rank_and_file_from_cell(from);
        auto [rank1, file1] = get_rank_and_file_from_cell(to);

        if (!is_valid({rank0, file0}))
            return false;

        if (!is_valid({rank1, file1}))
            return false;
        
        auto piece = is_player_white_turn
                        ? white_player.board[from.cell_id]
                        : black_player.board[from.cell_id];

        auto color = is_player_white_turn ? WHITE : BLACK;

        if (piece == EMPTY || piece == UNKNOWN)
            return false;

        if ((piece & PIECE_COLOR_MASK) != color)
            return false;

        auto moves = get_move(board, from);
        for (auto move : moves) {
            if (move.end_cell.cell_id == to.cell_id)
                return true;
        }

        return false;
    }

    GameState::GameState(const std::string& fen)
    {
        board = board_from_fen(fen);
        white_player = board_for_player(board, true);
        black_player = board_for_player(board, false);
        winner = 0;
        is_player_white_turn = true;
    }

    bool GameState::make_move(const move_t& move)
    {
        if (!is_valid_move(move))
            return false;

        auto piece_moved = board.board[move.start_cell.cell_id];

        if (piece_moved & KING) {
            if (piece_moved & WHITE) {
                board.info.white_king_moved = 1;
            } else {
                board.info.black_king_moved = 1;
            }

            auto [rank0, file0] = get_rank_and_file_from_cell(move.start_cell);
            auto [rank1, file1] = get_rank_and_file_from_cell(move.end_cell);

            if (rank0 == 0 && rank1 == 0 && std::abs(file1 - file0) == 2) {
                // Castling move
                if (file1 == 6) {
                    // Kingside castling
                    board.board[5] = board.board[7];
                    board.board[7] = EMPTY;
                    board.info.white_kingside_rook_moved = 1;
                } else if (file1 == 2) {
                    // Queenside castling
                    board.board[3] = board.board[0];
                    board.board[0] = EMPTY;
                    board.info.white_queenside_rook_moved = 1;
                }
            }

            if (rank0 == 7 && rank1 == 7 && std::abs(file1 - file0) == 2) {
                // Castling move
                if (file1 == 6) {
                    // Kingside castling
                    board.board[61] = board.board[63];
                    board.board[63] = EMPTY;
                    board.info.black_kingside_rook_moved = 1;
                } else if (file1 == 2) {
                    // Queenside castling
                    board.board[59] = board.board[56];
                    board.board[56] = EMPTY;
                    board.info.black_queenside_rook_moved = 1;
                }
            }
        }


        if (piece_moved & ROOK) {
            if (move.start_cell.cell_id == 63)
                board.info.white_kingside_rook_moved = 1;
            else if (move.start_cell.cell_id == 56)
                board.info.white_queenside_rook_moved = 1;
            else if (move.start_cell.cell_id == 7)
                board.info.white_kingside_rook_moved = 1;
            else if (move.start_cell.cell_id == 0)
                board.info.white_queenside_rook_moved = 1;
        }

        auto captured_piece = board.board[move.end_cell.cell_id];

        board.board[move.end_cell.cell_id] = board.board[move.start_cell.cell_id];
        board.board[move.start_cell.cell_id] = EMPTY;

        white_player = board_for_player(board, true);
        black_player = board_for_player(board, false);

        is_player_white_turn = !is_player_white_turn;

        if (captured_piece & KING) {
            if (captured_piece & WHITE) {
                winner = -1;
            } else {
                winner = 1;
            }
        }

        return true;
    }

    bool GameState::has_winner() const
    {
        return (winner != 0);
    }
}
