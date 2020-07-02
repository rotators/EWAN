#pragma once

#include "Log.hpp"

using namespace EWAN;

#include <cstdlib>
#include <string>
#include <vector>

// Must be defined by CMake
#if !defined(TEST_NAME)
 #error TEST_NAME not defined
#endif

//#if defined(_MSC_VER)
// #define TEST_MAIN          extern "C" __declspec(dllexport) int __cdecl TEST_NAME(int, const char**)
//#else
 #define TEST_MAIN          int TEST_NAME(int, char*[])
//#endif

#define TEST_ASSERT(x)     if(!(x)){ Log::Raw(std::string("ASSERT ") + #x); return EXIT_FAILURE; }

#define CONTENT_CACHE_LIST(c) {&c.Font, &c.Image, &c.SoundBuffer, &c.Sprite}

// Attempt to use sf::Texture in test will always generate failure under GitHub Actions + Linux
#if defined(__GNUC__)
 #pragma GCC poison Texture
#endif
