#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;

enum Color { White = 0, Black = 1, NoColor = 2 };
enum PieceType {
  King = 0,
  Queen = 1,
  Rook = 2,
  Bishop = 3,
  Knight = 4,
  Pawn = 5,
  None = 6
};

struct Piece {
  PieceType type = None;
  Color color = NoColor;
  bool empty() const { return type == None; }
};

struct Move {
  int from, to;
  bool isCapture = false;
  bool isPromotion = false;
  PieceType promoTo = Queen;
};

static inline int file_of(int s) { return s % 8; }
static inline int rank_of(int s) { return s / 8; }
static inline bool on_board(int f, int r) {
  return (0 <= f && f < 8 && 0 <= r && r < 8);
}
static inline int sq(int f, int r) { return r * 8 + f; }

string sq_to_str(int s) {
  string out;
  out += char('a' + file_of(s));
  out += char('1' + rank_of(s));
  return out;
}
int str_to_sq(const string &s) {
  if (s.size() != 2)
    return -1;
  int f = s[0] - 'a', r = s[1] - '1';
  if (f < 0 || f > 7 || r < 0 || r > 7)
    return -1;
  return sq(f, r);
}

char piece_char(const Piece &p) {
  char c = '.';
  switch (p.type) {
  case King:
    c = 'K';
    break;
  case Queen:
    c = 'Q';
    break;
  case Rook:
    c = 'R';
    break;
  case Bishop:
    c = 'B';
    break;
  case Knight:
    c = 'N';
    break;
  case Pawn:
    c = 'P';
    break;
  default:
    c = '.';
  }
  if (p.color == Black)
    c = tolower(c);
  return c;
}

struct Board {
  array<Piece, 64> b{};
  Color stm = White; // side to move

  void clear() {
    for (auto &x : b) {
      x.type = None;
      x.color = NoColor;
    }
  }

  void set_startpos() {
    clear();
    auto place = [&](int f, int r, PieceType t, Color c) {
      b[sq(f, r)] = {t, c};
    };
    // White
    place(0, 0, Rook, White);
    place(1, 0, Knight, White);
    place(2, 0, Bishop, White);
    place(3, 0, Queen, White);
    place(4, 0, King, White);
    place(5, 0, Bishop, White);
    place(6, 0, Knight, White);
    place(7, 0, Rook, White);
    for (int f = 0; f < 8; f++)
      place(f, 1, Pawn, White);
    // Black
    place(0, 7, Rook, Black);
    place(1, 7, Knight, Black);
    place(2, 7, Bishop, Black);
    place(3, 7, Queen, Black);
    place(4, 7, King, Black);
    place(5, 7, Bishop, Black);
    place(6, 7, Knight, Black);
    place(7, 7, Rook, Black);
    for (int f = 0; f < 8; f++)
      place(f, 6, Pawn, Black);

    stm = White;
  }

  bool in_bounds(int s) const { return 0 <= s && s < 64; }

  // Helpers
  bool is_friend(int s, Color c) const {
    return in_bounds(s) && !b[s].empty() && b[s].color == c;
  }
  bool is_enemy(int s, Color c) const {
    return in_bounds(s) && !b[s].empty() && b[s].color != c;
  }
  bool is_empty(int s) const { return in_bounds(s) && b[s].empty(); }

  void add_slide_moves(vector<Move> &mv, int from, Color c,
                       const vector<pair<int, int>> &deltas,
                       bool capturesOnly = false) const {
    int f0 = file_of(from), r0 = rank_of(from);
    for (auto [df, dr] : deltas) {
      int f = f0 + df, r = r0 + dr;
      while (on_board(f, r)) {
        int to = sq(f, r);
        if (is_friend(to, c))
          break;
        if (is_enemy(to, c)) {
          mv.push_back({from, to, true, false, Queen});
          break;
        }
        if (!capturesOnly)
          mv.push_back({from, to, false, false, Queen});
        f += df;
        r += dr;
      }
    }
  }
  void add_step_moves(vector<Move> &mv, int from, Color c,
                      const vector<pair<int, int>> &steps,
                      bool capturesOnly = false) const {
    int f0 = file_of(from), r0 = rank_of(from);
    for (auto [df, dr] : steps) {
      int f = f0 + df, r = r0 + dr;
      if (!on_board(f, r))
        continue;
      int to = sq(f, r);
      if (is_friend(to, c))
        continue;
      if (is_enemy(to, c))
        mv.push_back({from, to, true, false, Queen});
      else if (!capturesOnly)
        mv.push_back({from, to, false, false, Queen});
    }
  }

  void gen_pawn_moves(vector<Move> &mv, int from, Color c) const {
    int f = file_of(from), r = rank_of(from);
    int dir = (c == White) ? +1 : -1;
    int startRank = (c == White) ? 1 : 6;
    int promoRank = (c == White) ? 7 : 0;

    // forward
    int r1 = r + dir;
    if (on_board(f, r1)) {
      int to = sq(f, r1);
      if (is_empty(to)) {
        bool promo = (r1 == promoRank);
        mv.push_back({from, to, false, promo, Queen});
        // double
        if (r == startRank) {
          int r2 = r + 2 * dir;
          int to2 = sq(f, r2);
          if (on_board(f, r2) && is_empty(to2))
            mv.push_back({from, to2, false, false, Queen});
        }
      }
    }
    // captures
    for (int df : {-1, +1}) {
      int f1 = f + df, r1 = r + dir;
      if (!on_board(f1, r1))
        continue;
      int to = sq(f1, r1);
      if (is_enemy(to, c)) {
        bool promo = (r1 == promoRank);
        mv.push_back({from, to, true, promo, Queen});
      }
    }
    // (en passant omitted for brevity)
  }

  vector<Move> pseudo_legal_moves(Color c) const {
    vector<Move> mv;
    for (int s = 0; s < 64; s++) {
      if (b[s].empty() || b[s].color != c)
        continue;
      switch (b[s].type) {
      case Pawn:
        gen_pawn_moves(mv, s, c);
        break;
      case Knight:
        add_step_moves(mv, s, c,
                       {{+1, +2},
                        {+2, +1},
                        {+2, -1},
                        {+1, -2},
                        {-1, -2},
                        {-2, -1},
                        {-2, +1},
                        {-1, +2}});
        break;
      case Bishop:
        add_slide_moves(mv, s, c, {{+1, +1}, {+1, -1}, {-1, +1}, {-1, -1}});
        break;
      case Rook:
        add_slide_moves(mv, s, c, {{+1, 0}, {-1, 0}, {0, +1}, {0, -1}});
        break;
      case Queen:
        add_slide_moves(mv, s, c,
                        {{+1, 0},
                         {-1, 0},
                         {0, +1},
                         {0, -1},
                         {+1, +1},
                         {+1, -1},
                         {-1, +1},
                         {-1, -1}});
        break;
      case King:
        add_step_moves(mv, s, c,
                       {{+1, 0},
                        {-1, 0},
                        {0, +1},
                        {0, -1},
                        {+1, +1},
                        {+1, -1},
                        {-1, +1},
                        {-1, -1}});
        break;
      default:
        break;
      }
    }
    return mv;
  }

  // apply move (assumes it's legal in this variant)
  // returns:  1 if side to move captured opponent king (game over win),
  //           0 otherwise.
  int apply_move(const Move &m) {
    Piece fromP = b[m.from];
    Piece toP = b[m.to];

    // capture?
    bool capture = !toP.empty();
    bool capturedKing = capture && toP.type == King;

    // move piece
    b[m.to] = fromP;
    b[m.from] = Piece{None, NoColor};

    // promotion (auto-queen)
    if (m.isPromotion && b[m.to].type == Pawn) {
      b[m.to].type = Queen;
    }

    stm = (stm == White ? Black : White);
    return capturedKing ? 1 : 0;
  }

  // -------- Fog of War --------
  enum FogRule { LichessLike, StrictLOS };

  // visibility mask for side c
  array<bool, 64> visibility(Color c, FogRule rule) const {
    array<bool, 64> vis{};
    vis.fill(false);

    // always see your own pieces
    for (int s = 0; s < 64; s++)
      if (is_friend(s, c))
        vis[s] = true;

    auto mark_slide = [&](int from, const vector<pair<int, int>> &deltas) {
      int f0 = file_of(from), r0 = rank_of(from);
      for (auto [df, dr] : deltas) {
        int f = f0 + df, r = r0 + dr;
        while (on_board(f, r)) {
          int to = sq(f, r);
          vis[to] = true;
          // blocking rule
          bool friendHere = is_friend(to, c);
          bool enemyHere = is_enemy(to, c);
          if (rule == StrictLOS) {
            if (friendHere || enemyHere)
              break; // any piece blocks
          } else {   // LichessLike
            if (friendHere)
              break; // only your own pieces block vision
            // enemies do *not* block vision (they're invisible to rays)
          }
          f += df;
          r += dr;
        }
      }
    };
    auto mark_steps = [&](int from, const vector<pair<int, int>> &steps) {
      int f0 = file_of(from), r0 = rank_of(from);
      for (auto [df, dr] : steps) {
        int f = f0 + df, r = r0 + dr;
        if (!on_board(f, r))
          continue;
        vis[sq(f, r)] = true;
      }
    };

    for (int s = 0; s < 64; s++) {
      if (b[s].empty() || b[s].color != c)
        continue;
      switch (b[s].type) {
      case Pawn: {
        int dir = (c == White) ? +1 : -1;
        // show forwards (for move planning) and diagonals (attacks)
        int f = file_of(s), r = rank_of(s);
        int fwdR = r + dir;
        if (on_board(f, fwdR))
          vis[sq(f, fwdR)] = true;
        if (r == ((c == White) ? 1 : 6)) { // double push visibility
          int fwd2R = r + 2 * dir;
          if (on_board(f, fwd2R))
            vis[sq(f, fwd2R)] = true;
        }
        // diagonals
        for (int df : {-1, 1})
          if (on_board(f + df, fwdR))
            vis[sq(f + df, fwdR)] = true;
      } break;
      case Knight:
        mark_steps(s, {{+1, +2},
                       {+2, +1},
                       {+2, -1},
                       {+1, -2},
                       {-1, -2},
                       {-2, -1},
                       {-2, +1},
                       {-1, +2}});
        break;
      case Bishop:
        mark_slide(s, {{+1, +1}, {+1, -1}, {-1, +1}, {-1, -1}});
        break;
      case Rook:
        mark_slide(s, {{+1, 0}, {-1, 0}, {0, +1}, {0, -1}});
        break;
      case Queen:
        mark_slide(s, {{+1, 0},
                       {-1, 0},
                       {0, +1},
                       {0, -1},
                       {+1, +1},
                       {+1, -1},
                       {-1, +1},
                       {-1, -1}});
        break;
      case King:
        mark_steps(s, {{+1, 0},
                       {-1, 0},
                       {0, +1},
                       {0, -1},
                       {+1, +1},
                       {+1, -1},
                       {-1, +1},
                       {-1, -1}});
        break;
      default:
        break;
      }
    }
    return vis;
  }

  void print(Color viewer, FogRule rule) const {
    if (viewer == NoColor) {
      cout << "  +-----------------+\n";
      for (int r = 7; r >= 0; --r) {
        cout << r + 1 << " | ";
        for (int f = 0; f < 8; ++f) {
          int s = sq(f, r);
          if (b[s].empty()) {
            cout << ". ";
          } else {
            cout << piece_char(b[s]) << ' ';
          }
        }
        cout << "|\n";
      }
      cout << "  +-----------------+\n    a b c d e f g h\n";
    } else {
      auto vis = visibility(viewer, rule);
      cout << "  +-----------------+\n";
      for (int r = 7; r >= 0; --r) {
        cout << r + 1 << " | ";
        for (int f = 0; f < 8; ++f) {
          int s = sq(f, r);
          if (!vis[s]) {
            cout << "? ";
          } else if (b[s].empty()) {
            cout << ". ";
          } else {
            cout << piece_char(b[s]) << ' ';
          }
        }
        cout << "|\n";
      }
      cout << "  +-----------------+\n    a b c d e f g h\n";
    }
  }

  bool has_king(Color c) const {
    for (int s = 0; s < 64; s++)
      if (!b[s].empty() && b[s].type == King && b[s].color == c)
        return true;
    return false;
  }

  // basic legality check for a user-entered move (pseudo-legal only)
  bool is_pseudolegal_move(const Move &m, Color who, Move &out) const {
    if (!in_bounds(m.from) || !in_bounds(m.to))
      return false;
    if (b[m.from].empty() || b[m.from].color != who)
      return false;
    if (is_friend(m.to, who))
      return false;

    vector<Move> gen = pseudo_legal_moves(who);
    for (const auto &g : gen) {
      if (g.from == m.from && g.to == m.to) {
        out = g;
        return true;
      }
    }
    return false;
  }
};

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

  cout << "Waiting for White (port 8001) and Black (port 8002) to connect...\n";
  client_fd1 = accept(server_fd1, (struct sockaddr *)&address1, &addrlen);
  if (client_fd1 < 0) { perror("accept 8001"); exit(EXIT_FAILURE); }
  cout << "White connected!\n";
  client_fd2 = accept(server_fd2, (struct sockaddr *)&address2, &addrlen);
  if (client_fd2 < 0) { perror("accept 8002"); exit(EXIT_FAILURE); }
  cout << "Black connected!\n";
  Board bd;
  bd.set_startpos();

  Board::FogRule fogRule =
      Board::LichessLike; // switch to StrictLOS to try the other style

  cout << "Fog of War Chess (C++ prototype)\n";
  cout << "Rules: No check; capture the king to win. Promotions auto-queen. "
          "Castling/en passant TODO.\n";
  cout << "Fog: "
       << (fogRule == Board::LichessLike
               ? "Lichess-like (enemies don't block vision)"
               : "Strict LOS (any piece blocks)")
       << "\n\n";

  while (true) {
    Color viewer = bd.stm;
    Color opp = (viewer == White ? Black : White);
    // Print unobstructed board to stdout for logging
    cout << (viewer == White ? "White" : "Black") << " to move\n";
    bd.print(NoColor, fogRule); // NoColor: show all pieces
    // Prepare fogged board for player
    stringstream ss;
    ss << (viewer == White ? "White" : "Black") << " to move\n";
    {
      std::streambuf* old_cout = cout.rdbuf(ss.rdbuf());
      bd.print(viewer, fogRule);
      cout.rdbuf(old_cout);
    }
    ss << "Enter move (e.g., e2e4) or 'q': ";
    string outstr = ss.str();
    int client_fd = (viewer == White) ? client_fd1 : client_fd2;
    send(client_fd, outstr.c_str(), outstr.size(), 0);

    // --- Socket input ---
    char buf[16] = {0};
    ssize_t n = recv(client_fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) {
      cout << "Connection closed or error.\n";
      break;
    }
    string s(buf);
    s.erase(remove(s.begin(), s.end(), '\n'), s.end());
    s.erase(remove(s.begin(), s.end(), '\r'), s.end());
    if (s == "q" || s == "quit")
      break;
    if (s.size() != 4) {
      string msg = "Format: e2e4\n";
      cout << msg;
      send(client_fd, msg.c_str(), msg.size(), 0);
      continue;
    }
    int from = str_to_sq(s.substr(0, 2));
    int to = str_to_sq(s.substr(2, 2));
    if (from < 0 || to < 0) {
      string msg = "Bad squares\n";
      cout << msg;
      send(client_fd, msg.c_str(), msg.size(), 0);
      continue;
    }

    Move req{from, to, false, false, Queen}, pl;
    if (!bd.is_pseudolegal_move(req, bd.stm, pl)) {
      cout << "Illegal (pseudo-legal) move for this variant.\n";
      continue;
    }

    int win = bd.apply_move(pl);
    if (win) {
      cout << ((viewer == White) ? "White" : "Black")
           << " captured the king. Game over!\n";
      break;
    }
    // sanity: if an impossible state happens
    if (!bd.has_king(White) || !bd.has_king(Black)) {
      cout << "King missing—game over.\n";
      break;
    }
    // After move, send updated board and prompt to both players
    for (int i = 0; i < 2; ++i) {
      Color player = (i == 0) ? White : Black;
      int client_fd = (player == White) ? client_fd1 : client_fd2;
      stringstream ss;
      ss << (player == White ? "White" : "Black") << " to move\n";
      {
        std::streambuf* old_cout = cout.rdbuf(ss.rdbuf());
        bd.print(player, fogRule);
        cout.rdbuf(old_cout);
      }
      ss << "Enter move (e.g., e2e4) or 'q': ";
      string outstr = ss.str();
      send(client_fd, outstr.c_str(), outstr.size(), 0);
    }
    cout << "\n";
  }
  return 0;
}
