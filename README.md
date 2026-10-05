# Asteroids.c
A complete arcade-style space shooter built from scratch in C using the raylib library. The player controls a ship, shoots lasers at asteroids that break into smaller pieces, and fights enemy UFOs. Includes a start menu, game-over screen, score system and sound effects.
# Asteroids.c

A classic arcade-style space shooter built from scratch in **C** using the **raylib** library. Pilot your ship, blast asteroids into smaller pieces, and survive enemy UFOs that hunt you down.

## Features
- Ship with rotation, thrust and sideways strafing, with screen wrap-around
- Asteroids in 3 sizes that split into smaller pieces when shot
- Pseudo-3D depth effect: asteroids grow and shrink as they drift
- Enemy UFOs that fly across the screen and fire aimed bullets at the player
- 3 lives with a short invulnerability period after each hit
- Difficulty that increases the longer you survive
- Persistent high score saved to `highscore.txt`
- Main menu, High Scores, How to Play and About Us screens
- Laser and crash sound effects, plus background music

## Controls
| Key | Action |
|---|---|
| Left / Right Arrow | Rotate ship |
| Up Arrow | Move forward |
| A / D | Strafe left / right |
| Space | Fire laser |
| B | Back to menu |
| Enter | Restart after game over |

## Scoring
| Target | Points |
|---|---|
| Large asteroid | 20 |
| Medium asteroid | 50 |
| Small asteroid | 90 |
| Enemy UFO | 50 |

## Built With
- C
- [raylib](https://www.raylib.com/)

## How to Run
1. Install raylib (https://github.com/raysan5/raylib)
2. Make sure the `resources` folder is in the same directory as the game. It must contain:
   `crash.wav`, `laser.mp3`, `background.mp3`, `Tahmid.png`, `Hridoy.png`
3. Compile (Windows, MinGW):
```
   gcc asteroids.c -o asteroids.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```
   Linux:
```
   gcc asteroids.c -o asteroids -lraylib -lm -lpthread -ldl
```
4. Run `asteroids.exe` (or `./asteroids`)





## Authors
- Hridoy Banik
