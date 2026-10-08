# Blackjack

A graphical Blackjack game written in C using the [Raylib](https://www.raylib.com/) library. Play against the dealer, keep your balance across sessions, and enjoy a simple point-and-click interface.

## Features

- Graphical interface built with Raylib (800x600 window, 60 FPS)
- Main menu, name entry screen, and in-game table
- Animated card dealing and dealer turn
- Dealer's second card stays hidden during the player's turn
- Standard Blackjack rules:
  - Face cards (J, Q, K) count as 10
  - Aces count as 11 or 1, adjusted automatically to avoid busting
  - Natural Blackjack is detected right after the initial deal
  - Dealer draws until reaching 17 or higher
- Fresh Fisher-Yates shuffle at the start of every round
- Persistent player balance saved by name in a local file (`salvos.txt`)

## Scoring

| Result               | Balance change |
|----------------------|----------------|
| Win                  | +100           |
| Win with Blackjack   | +200           |
| Loss                 | -100           |
| Dealer Blackjack     | -200           |
| Tie                  | 0              |

New players start with a balance of 0, and the balance is allowed to go negative.

## Requirements

- GCC (or any C11 compatible compiler)
- Raylib installed on your system
- Linux (developed and tested on WSL with Ubuntu)
- X11 and OpenGL development libraries (needed for linking Raylib)

On Debian/Ubuntu, the system libraries can be installed with:

```bash
sudo apt install build-essential libx11-dev libxrandr-dev libxinerama-dev libxi-dev libxcursor-dev libgl1-mesa-dev
```

See the [Raylib installation guide](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux) for building and installing Raylib itself.

## Build and Run

```bash
make        # compile
make run    # compile and run
make clean  # remove the executable
```

Or run the binary directly after compiling:

```bash
./main
```

## How to Play

1. Click **JOGAR** on the main menu.
2. Type your name and press **Enter** (or click **ENTRAR**). If you have played before, your saved balance is loaded.
3. On your turn, click **COMPRAR** to draw a card or **PARAR** to stand.
4. After the dealer plays, the result is shown and your balance is updated and saved.
5. Click **JOGAR NOVAMENTE** for another round or **MENU PRINCIPAL** to go back.

## Project Structure

```
.
├── main.c      # game logic, rendering, and save system
├── Makefile    # build rules
└── salvos.txt  # player balances (created automatically)
```

## Possible Improvements

- Betting system with custom amounts
- Split and double down
- Card sprites instead of text based cards
- Sound effects
