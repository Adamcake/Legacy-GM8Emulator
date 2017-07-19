# GM8Emulator
An emulator for games created with GameMaker 8.

## Eh?
GameMaker 8 is an IDE for creating Windows games developed by YoYo Games. It was released on December 22nd 2009 (surpassing GameMaker 7) and went end-of-life in 2011, having been succeeded by GameMaker: Studio. Despite this, it is still widely used and there exist thousands of games created with GameMaker 8.

First, let's talk about how GameMaker 8 executables work. If you compile a game using GameMaker 8 or 8.1 ("GM8"), you will get a standalone exe file which contains the entirety of the game, including logic and assets. A GM8 executable can be broken down into two parts: the runner, and the gamedata.
### Runner
The runner is essentially what you would expect to find in a windows executable: it contains the compiled machine code for running the game. It is pretty much identical for any two games compiled with the same GM8 build, ie. the runner is simply the logic used to interpret the gamedata and put a game on the screen.
### Gamedata
Appended to the runner is the gamedata. The gamedata is where you will find all the game's assets - all the sprites, objects, scripts, sounds, and so on. The runner loads the entire exe file into memory, parses the gamedata and uses it to play the game. The entirety of your game is contained within the gamedata: if you wished to recover any or all of your assets, you would do so from the gamedata.

## So what's a GM8 Emulator?
Put simply, the aim of this project is to emulate the GM8 runner. We are creating an application will be able to parse GM8-format gamedata and run the game for which it was created, in an identical manner to the existing runner described above.