# GM8Emulator
An emulator for games created with GameMaker 8.

## What's GameMaker?
GameMaker is an IDE for creating Windows games, developed by YoYo Games. *GameMaker 8* (GM8) was the last of the numbered releases of GameMaker, released on December 22nd 2009 (surpassing *GameMaker 7*) and succeeded by *GameMaker: Studio* in 2011. Due to the huge behavioral differences between "Numbered GameMaker" and *GameMaker: Studio*, as well as Studio's lack of backward-compatibility, GM8 is still widely used, with thousands of games to its name.

## So what's a GM8 Emulator?
One of GameMaker's strengths as a game engine is its ability to compile an entire project into a single executable. No external dependencies or installers, just compile, send the .exe file to your friend and they will be able to play your game. This is achieved by having the exe contain a phase file for the entire collection of assets required to run the game. In other words, that game.exe file contains not only the game engine, but also the entire game. This behaviour was made optional GameMaker: Studio, giving the creator a choice between standalone executable or .msi installer, but in GM8 this is the only build option.

So with that in mind, our goal with GM8Emulator is to create a C++ application which will be able to parse GM8 exe files and play the game contained within. The emulator should mimic the behaviour of GameMaker 8's engine as closely as possible, down to the sub-frame. Any behavioral discrepancy the official engine and the emulator will be considered a bug.

Strictly speaking, "emulator" is not a correct term. In computing, an emulator is a piece of software on a computer system which emulates the behaviour of a different computer system. We aren't emulating any computer system - just the GM8 engine. A more accurate term would be "sourceport" but emulator sounds cooler.

## Why?
Why not?

## Building
- Clone the repository with `git clone --recurse-submodules https://github.com/GM8Emulator/GM8Emulator.git`
- If you're using Visual Studio, all the dependencies are already part of the repository. Simply run `cmake -G "Visual Studio 15 2017 Win64"` (optionally without `Win64` if you're on a 32-bit OS for some reason)
- If you aren't using Visual Studio or building it on a non-windows system, the requirements are the glew and zlib packages installed. This is located with [FindGLEW](https://github.com/Kitware/CMake/blob/master/Modules/FindGLEW.cmake) and [FindZLIB](https://github.com/Kitware/CMake/blob/master/Modules/FindZLIB.cmake) respectively.

## Contributing
This project has only been worked on by two people so far. If you would like to help then that's great, we have a lot to do - but I would encourage you to get in touch first before diving in.

## Contact
gm8emulator@gmail.com
