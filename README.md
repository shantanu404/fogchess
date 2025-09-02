FogChess
========
A simple implementation of Fog of War chess variant in C++

How to play
## FogChess

A simple, interactive Fog of War chess variant server in C++.

## Features
- **Fog of War:** Each player only sees their own pieces and squares they control.
- **Socket-based Multiplayer:** Connect as White or Black using TCP sockets.
- **Unobstructed Logging:** The server logs the full board for monitoring.

## Getting Started

### 1. Compile
Use the provided script to build the project:

```bash
./compile.sh
```

### 2. Run the Server
Start the chess server:

```bash
./fogchess
```

### 3. Connect Players
Players connect using `netcat` (or any TCP client):

- **White:**
	```bash
	nc localhost 8001
	```
- **Black:**
	```bash
	nc localhost 8002
	```

Each player will see their own board and prompts. The server logs the full board after every move.

## Game Rules
- **Win Condition:** Capture the opponent's king.
- **Promotions:** Pawns auto-queen.
- **Castling** Not implemented.
- **En Passant** Should work?
- **Fog:** Only squares visible to your pieces are shown.

## License
This project is licensed under the MIT License. See `LICENCE` for details.
