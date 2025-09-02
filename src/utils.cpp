#include "utils.hpp"

#include <cctype>
#include <iostream>
#include <string>
#include <set>

namespace fogchess
{
    bool is_last_move_was_double_pawn_push(const real_board_t& board)
    {
        move_t last_move = board.last_move;

        if (last_move.start_cell.cell_id == -1 || last_move.end_cell.cell_id == -1) {
            return false;
        }

        int start_rank = last_move.start_cell.cell_id / 8;
        int end_rank = last_move.end_cell.cell_id / 8;
        bool pawn_move = get_piece_at_cell(board, last_move.start_cell) & PAWN;

        return pawn_move && ((start_rank == 1 && end_rank == 3) || (start_rank == 6 && end_rank == 4));
    }

    bool is_valid(std::pair<int, int> rank_and_file)
    {
        int rank = rank_and_file.first;
        int file = rank_and_file.second;
        return (0 <= rank && rank < 8 && 0 <= file && file < 8);
    }

    piece_t get_piece_at_cell(const real_board_t& board, const cell_t& cell)
    {
        return board.board[cell.cell_id];
    }

    std::pair<int, int> get_rank_and_file_from_cell(const cell_t& cell)
    {
        int rank = cell.cell_id / 8;
        int file = cell.cell_id % 8;
        return {rank, file};
    }

    real_board_t board_from_fen(const std::string& fen_notation)
    {
        real_board_t board;
        board.board.fill(EMPTY);
        board.last_move = {{-1}, {-1}};

        const std::map<char, uint8_t> fen_to_piece = {
            {'p', PAWN | BLACK}, {'P', PAWN | WHITE},
            {'r', ROOK | BLACK}, {'R', ROOK | WHITE},
            {'n', KNIGHT | BLACK}, {'N', KNIGHT | WHITE},
            {'b', BISHOP | BLACK}, {'B', BISHOP | WHITE},
            {'q', QUEEN | BLACK}, {'Q', QUEEN | WHITE},
            {'k', KING | BLACK}, {'K', KING | WHITE}
        };

        int rank = 7;
        int file = 0;

        for (const char c : fen_notation) {
            if (c == ' ') {
                break; // End of board part
            } else if (c == '/') {
                rank--;
                file = 0;
            } else if (std::isdigit(c)) {
                file += c - '0'; // Empty squares
            } else if (fen_to_piece.find(c) != fen_to_piece.end()) {
                if (file < 8 && rank >= 0) {
                    cell_t cell{ rank * 8 + file };
                    piece_t piece = static_cast<piece_t>(fen_to_piece.at(c));
                    board.board[cell.cell_id] = piece;
                    file++;
                }
            }
        }

        return std::move(board);
    }

    void print_real_board(const real_board_t& board, std::ostream& os)
    {
        const std::map<uint8_t, char> piece_to_char = {
            {PAWN | WHITE, 'P'}, {PAWN | BLACK, 'p'},
            {ROOK | WHITE, 'R'}, {ROOK | BLACK, 'r'},
            {KNIGHT | WHITE, 'N'}, {KNIGHT | BLACK, 'n'},
            {BISHOP | WHITE, 'B'}, {BISHOP | BLACK, 'b'},
            {QUEEN | WHITE, 'Q'}, {QUEEN | BLACK, 'q'},
            {KING | WHITE, 'K'}, {KING | BLACK, 'k'}
        };

        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 0; file < 8; ++file) {
                cell_t cell{ rank * 8 + file };
                piece_t piece = get_piece_at_cell(board, cell);
                os << (piece != EMPTY ? piece_to_char.at(piece) : '.');
            }
            os << '\n';
        }
    }

    player_board_t board_for_player(const real_board_t& board, bool is_player_white) {
        player_board_t player_board;
        player_board.board.fill(UNKNOWN);

        piece_t color = is_player_white ? WHITE : BLACK;
        std::set<cell_t> visible_cells;

        for (int cell_id = 0; cell_id < 64; ++cell_id) {
            piece_t piece = board.board[cell_id];
            if (piece & color) {
                player_board.board[cell_id] = piece;

                auto moves = get_move(board, {cell_id});
                for (auto move : moves) {
                    player_board.board[move.end_cell.cell_id] = get_piece_at_cell(board, move.end_cell);
                }
            }
        }

        return std::move(player_board);
    }

    std::vector<move_t> get_move(const real_board_t& board, cell_t location)
    {
        auto piece = get_piece_at_cell(board, location);

        if (piece & KING) {
            return king_move(board, location);
        }

        if (piece & KNIGHT) {
            return knight_move(board, location);
        }

        if ((piece & QUEEN) == QUEEN) {
            return queen_move(board, location);
        }

        if (piece & BISHOP) {
            return bishop_move(board, location);
        }

        if (piece & ROOK) {
            return rook_move(board, location);
        }

        return pawn_move(board, location);
    }

    std::vector<move_t> melee_move(const real_board_t& board, cell_t location, const std::vector<std::pair<int, int>>& directions)
    {
        std::vector<move_t> moves;

        cell_t start = location;
        piece_t piece = board.board[start.cell_id];

        for (const auto& [dx, dy] : directions) {
            auto [rank, file] = get_rank_and_file_from_cell(start);
            rank += dy;
            file += dx;
            cell_t end { rank * 8 + file };
            if (is_valid(get_rank_and_file_from_cell(end))) {
                auto piece_at_end = get_piece_at_cell(board, end);
                if (piece_at_end == EMPTY || (piece_at_end & PIECE_COLOR_MASK) != (piece & PIECE_COLOR_MASK)) {
                    moves.push_back({start, end});
                }
            }
        }

        return moves;
    }

    std::vector<move_t> sliding_move(const real_board_t& board, cell_t location, const std::vector<std::pair<int, int>>& directions)
    {
        std::vector<move_t> moves;

        cell_t start = location;
        piece_t piece = board.board[start.cell_id];


        for (const auto& [dx, dy] : directions) {
            auto [rank, file] = get_rank_and_file_from_cell(start);
            rank += dy;
            file += dx;
            cell_t end = { rank * 8 + file };

            while (is_valid({ rank, file })) {
                auto piece_at_end = get_piece_at_cell(board, end);

                if (piece_at_end == EMPTY) {
                    moves.push_back({start, end});
                } else {
                    if ((piece_at_end & PIECE_COLOR_MASK) != (piece & PIECE_COLOR_MASK)) {
                        moves.push_back({start, end});
                    }
                    break;
                }
                rank += dy;
                file += dx;
                end.cell_id = rank * 8 + file;
            }
        }

        return moves;
    }

    std::vector<move_t> pawn_move(const real_board_t& board, cell_t location)
    {
        std::vector<move_t> moves;
        auto pawn = get_piece_at_cell(board, location);
        int direction = (pawn & WHITE) ? 1 : -1;
        cell_t start = location;
        cell_t end;

        // Single step forward
        end.cell_id = start.cell_id + direction * 8;
        if (is_valid(get_rank_and_file_from_cell(end)) && get_piece_at_cell(board, end) == EMPTY) {
            moves.push_back({start, end});
            int start_rank = (pawn & WHITE) ? 1 : 6;
            if ((start.cell_id / 8) == start_rank) {
                end.cell_id += direction * 8;
                if (is_valid(get_rank_and_file_from_cell(end)) && get_piece_at_cell(board, end) == EMPTY) {
                    moves.push_back({start, end});
                }
            }
        }

        // Captures
        for (int df : {-1, 1}) {
            end.cell_id = start.cell_id + direction * 8 + df;
            if (is_valid(get_rank_and_file_from_cell(end)) && get_piece_at_cell(board, end) != EMPTY) {
                auto other_piece = get_piece_at_cell(board, end);

                if ((other_piece & PIECE_COLOR_MASK) != (pawn & PIECE_COLOR_MASK))
                    moves.push_back({start, end});
            }
        }

        // En Passant
        if (is_last_move_was_double_pawn_push(board)) {
            move_t last_move = board.last_move;
            int last_move_end_rank = last_move.end_cell.cell_id / 8;
            int last_move_end_file = last_move.end_cell.cell_id % 8;
            int pawn_rank = start.cell_id / 8;
            int pawn_file = start.cell_id % 8;

            if (pawn_rank == last_move_end_rank && std::abs(pawn_file - last_move_end_file) == 1) {
                end.cell_id = last_move.end_cell.cell_id + direction * 8;
                if (is_valid(get_rank_and_file_from_cell(end))) {
                    moves.push_back({start, end});
                }
            }
        }

        return moves;
    }

    std::vector<move_t> king_move(const real_board_t& board, cell_t location)
    {
        return melee_move(board, location, {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}});
    }

    std::vector<move_t> knight_move(const real_board_t& board, cell_t location)
    {
        return melee_move(board, location, {{2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}, {1, -2}, {2, -1}});
    }

    std::vector<move_t> bishop_move(const real_board_t& board, cell_t location)
    {
        return sliding_move(board, location, {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}});
    }

    std::vector<move_t> rook_move(const real_board_t& board, cell_t location)
    {
        return sliding_move(board, location, {{1, 0}, {0, 1}, {-1, 0}, {0, -1}});
    }

    std::vector<move_t> queen_move(const real_board_t& board, cell_t location)
    {
        return sliding_move(board, location, {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}});
    }
}
