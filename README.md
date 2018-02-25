# GM8Emulator
An emulator for games created with GameMaker 8.

## Eh?
GameMaker 8 is an IDE for creating Windows games developed by YoYo Games. It was released on December 22nd 2009 (surpassing GameMaker 7) and went end-of-life in 2011, having been succeeded by GameMaker: Studio. Despite this, it is still widely used and there exist thousands of games created with GameMaker 8.

First, let's talk about how GameMaker 8 executables work. If you compile a game using GameMaker 8 or 8.1 ("GM8"), you will get a standalone exe file which contains the entirety of the game, including logic and assets. A GM8 executable can be broken down into two parts: the runner, and the gamedata.
### Runner
The runner is essentially what you would expect to find in a windows executable: it contains the compiled machine code for running the game. It is pretty much identical for any two games compiled with the same GM8 build, ie. the runner is simply the logic used to interpret the gamedata and put a game on the screen.
### Gamedata
Most Windows executables just contain the machine code necessary for them to run... Not these ones. GameMaker 8 executables have the gamedata appended to the end of the file. The gamedata is where you will find all the game's assets - all the sprites, objects, scripts, sounds, and so on. The runner loads the entire exe file into memory, parses the gamedata and uses it to play the game. The entirety of your game project is contained within the gamedata: if you wished to recover any or all of your assets, or indeed your entire project file, then you would require the gamedata and nothing more.

## So what's a GM8 Emulator?
Put simply, the aim of this project is to emulate the GM8 runner. We are creating an application which will be able to parse GM8-format gamedata and run the game for which it was created, in an identical manner to the existing runner described above.

Strictly speaking, "emulator" is not a correct term. In computing, an emulator is a piece of software on a computer system which emulates the behaviour of a different computer system. We aren't emulating any computer system - just the GM8 engine. A more accurate term would be "sourceport" but emulator sounds cooler.

## Notes
Fortunately, the format of the gamedata can be found "documented" in the behaviour of [WastedMeerkat's Decompiler](https://github.com/WastedMeerkat/gm81decompiler). Documentation on the runner's behaviour, however, is very sparse.

Want to help us? We have an unending amount of research that needs doing! If you have some free time and want to help us document GM8's frame cycle, either by observation or debugging, please do get involved.

If you have any questions or comments you can contact us at gm8emulator@gmail.com.

## Building
- Clone the repository with `git clone --recurse-submodules https://github.com/GM8Emulator/GM8Emulator.git`
- If you're using Visual Studio, all the dependencies are already part of the repository. Simply run `cmake -G "Visual Studio 15 2017 Win64"` (optionally without `Win64` if you're on a 32-bit OS for some reason)
- If you aren't using Visual Studio or building it on a non-windows system, the requirements are the glew and zlib packages installed. This is located with [FindGLEW](https://github.com/Kitware/CMake/blob/master/Modules/FindGLEW.cmake) and [FindZLIB](https://github.com/Kitware/CMake/blob/master/Modules/FindZLIB.cmake) respectively.
