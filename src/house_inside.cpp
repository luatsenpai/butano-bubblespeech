#include "map.h"

// This file used to contain the old `map::house_inside` implementation.
// The current codebase was refactored to use `rpg::world_map` in map.cpp,
// so the old class no longer exists in map.h.
//
// Keeping this translation unit minimal avoids build failures while the
// new map system is responsible for loading the inside-house backgrounds.
