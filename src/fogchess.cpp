
#include <iostream>
#include <string>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.hpp"
#include "serializer.hpp"
#include "utils.hpp"

using namespace fogchess;

int main() {
  // --- Socket setup ---
  int server_fd1, server_fd2, client_fd1, client_fd2;
  struct sockaddr_in address1, address2;
  int opt = 1;
  socklen_t addrlen = sizeof(sockaddr_in);

  server_fd1 = socket(AF_INET, SOCK_STREAM, 0);
  server_fd2 = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd1 < 0 || server_fd2 < 0) { perror("socket"); exit(EXIT_FAILURE); }
  setsockopt(server_fd1, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  setsockopt(server_fd2, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  address1.sin_family = AF_INET;
  address1.sin_addr.s_addr = INADDR_ANY;
  address1.sin_port = htons(8001);
  address2.sin_family = AF_INET;
  address2.sin_addr.s_addr = INADDR_ANY;
  address2.sin_port = htons(8002);

  if (bind(server_fd1, (struct sockaddr *)&address1, sizeof(address1)) < 0) { perror("bind 8001"); exit(EXIT_FAILURE); }
  if (bind(server_fd2, (struct sockaddr *)&address2, sizeof(address2)) < 0) { perror("bind 8002"); exit(EXIT_FAILURE); }

  if (listen(server_fd1, 1) < 0) { perror("listen 8001"); exit(EXIT_FAILURE); }
  if (listen(server_fd2, 1) < 0) { perror("listen 8002"); exit(EXIT_FAILURE); }

  std::cout << "Waiting for White (port 8001) and Black (port 8002) to connect...\n";
  client_fd1 = accept(server_fd1, (struct sockaddr *)&address1, &addrlen);
  if (client_fd1 < 0) { perror("accept 8001"); exit(EXIT_FAILURE); }
  std::cout << "White connected!\n";
  client_fd2 = accept(server_fd2, (struct sockaddr *)&address2, &addrlen);
  if (client_fd2 < 0) { perror("accept 8002"); exit(EXIT_FAILURE); }
  std::cout << "Black connected!\n";

  // Initialize board
  std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  real_board_t board = board_from_fen(start_fen);

  bool is_white_turn = true;

  std::cout << "Fog of War Chess (C++ prototype)\n";
  std::cout << "Rules: No check; capture the king to win. Promotions auto-queen. Castling/en passant TODO.\n";

  while (true) {
    print_real_board(board, std::cout);

    bool is_player_white = is_white_turn;
    int client_fd = is_player_white ? client_fd1 : client_fd2;

    // Get player's board view
    player_board_t player_board = board_for_player(board, is_player_white);
    std::string serialized = serialize_board(player_board);

    std::string prompt = (is_player_white ? "White" : "Black") + std::string(" to move\n") + serialized + "\nEnter move (e.g., e2e4) or 'q': ";
    send(client_fd, prompt.c_str(), prompt.size(), 0);

    // --- Socket input ---
    char buf[16] = {0};
    ssize_t n = recv(client_fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) {
      std::cout << "Connection closed or error.\n";
      break;
    }
    std::string s(buf);
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    if (s == "q" || s == "quit")
      break;
    if (s.size() != 4) {
      std::string msg = "Format: e2e4\n";
      std::cout << msg;
      send(client_fd, msg.c_str(), msg.size(), 0);
      continue;
    }

    // Convert input to move_t
    std::string from_str = s.substr(0, 2);
    std::string to_str = s.substr(2, 2);
    std::pair<uint8_t, uint8_t> from_rf = {from_str[1] - '1', from_str[0] - 'a'};
    std::pair<uint8_t, uint8_t> to_rf = {to_str[1] - '1', to_str[0] - 'a'};
    if (!is_valid(from_rf) || !is_valid(to_rf)) {
      std::string msg = "Bad squares\n";
      std::cout << msg;
      send(client_fd, msg.c_str(), msg.size(), 0);
      continue;
    }
    cell_t from_cell{static_cast<int>(from_rf.first * 8 + from_rf.second)};
    cell_t to_cell{static_cast<int>(to_rf.first * 8 + to_rf.second)};
    move_t move{from_cell, to_cell};

    // TODO: Validate move legality using get_move and apply move
    // For now, just apply the move
    board.last_move = move;
    board.board[to_cell.cell_id] = board.board[from_cell.cell_id];
    board.board[from_cell.cell_id] = EMPTY;

    // After move, send updated board to both players
    for (int i = 0; i < 2; ++i) {
      bool is_player_white = (i == 0);
      int client_fd = is_player_white ? client_fd1 : client_fd2;
      player_board_t player_board = board_for_player(board, is_player_white);
      std::string serialized = serialize_board(player_board);
      std::string prompt = (is_player_white ? "White" : "Black") + std::string(" to move\n") + serialized + "\nEnter move (e.g., e2e4) or 'q': ";
      send(client_fd, prompt.c_str(), prompt.size(), 0);
    }
    is_white_turn = !is_white_turn;
  }
  return 0;
}
