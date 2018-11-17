#include <pch.h>

#include "Game.hpp"
#include <chrono>
#include <iostream>
#include <thread>

#define CHECK_MEMORY_LEAKS 0
constexpr bool OUTPUT_FRAME_TIME = true;

#if CHECK_MEMORY_LEAKS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

int main(int argc, char** argv) {

#if CHECK_MEMORY_LEAKS
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // OUTPUT_FRAME_TIME (noop otherwise)
    std::chrono::high_resolution_clock::time_point t1, t2, t3;
    std::chrono::duration<double> time_span;
    double se;

    if constexpr (OUTPUT_FRAME_TIME) {
        t1 = std::chrono::high_resolution_clock::now();
    }

    GameInit();

    // This is just temp - you must place a game called "game.exe" in the project directory (or in the same directory as your built exe) to load it.
    // This can easily be changed to load from anywhere when the project is done.
    if (!GameLoad("game.exe")) {
        // Load failed
        GameTerminate();
        return 2;
    }

    if constexpr (OUTPUT_FRAME_TIME) {
        t2 = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        se = time_span.count();
        std::cout << "Successful load in " << se << " seconds" << std::endl;
    }

    if (!GameStart()) {
        // Starting game failed
        const char* err;
        if (GameGetError(&err)) {
            std::cout << "RUNTIME ERROR: " << err << std::endl;
        }

        GameTerminate();
        return 3;
    }

    if constexpr (OUTPUT_FRAME_TIME) {
        t3 = std::chrono::high_resolution_clock::now();
        time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t3 - t1);
        se = time_span.count();
        std::cout << "Successful game start in " << se << " seconds" << std::endl;
    }

    unsigned int a = 0;
    double totMus = 0;
    while (true) {
        t1 = std::chrono::high_resolution_clock::now();
        if (!GameFrame()) {
            const char* err;
            if (GameGetError(&err)) {
                std::cout << "RUNTIME ERROR: " << err << std::endl;
            }
            break;
        }

        if constexpr (OUTPUT_FRAME_TIME) {
            t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
            double mus = time_span.count() * 1000000.0;
            std::cout << "Frame took " << ( int )mus << " microseconds" << std::endl;
        }

        while (true) {
            t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
            double mus2 = time_span.count() * 1000000.0;
            long long waitMus = ( long long )(((( double )1000000.0) / GameGetRoomSpeed()) - mus2);
            if (waitMus <= 0) {
                break;
            }
        }
    }

    // Natural end of application
    GameTerminate();
    return 0;
}