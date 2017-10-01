#pragma region Copyright (c) 2014-2017 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#ifndef _MAPGEN_H_
#define _MAPGEN_H_

#include "../common.h"

typedef struct mapgen_settings {
    // Base
    sint32 mapSize;
    sint32 height;
    sint32 water_level;
    sint32 floor;
    sint32 wall;

    // Features (e.g. tree, rivers, lakes etc.)
    sint32 trees;

    // Simplex Noise Parameters
    sint32 simplex_low;
    sint32 simplex_high;
    float simplex_base_freq;
    sint32 simplex_octaves;

    // Height map settings
    bool smooth;
    bool smooth_height_map;
    uint32 smooth_strength;
    bool normalize_height;

    // Forest settings
    bool trees_place;
    // default was 1.75f;
    float trees_frequency;
    // default was _mapHeight, but 10 works pretty nice.
    sint32 trees_amplitude;
    // default was 6
    sint32 trees_octaves;
    // default was 2.0f
    float trees_lacunarity;
    // default was 0.65f
    float trees_persistance;
    // default was 0.75f
    float trees_threshold;
} mapgen_settings;

#ifdef __cplusplus
extern "C" {
#endif

void mapgen_generate_blank(mapgen_settings *settings);
void mapgen_generate(mapgen_settings *settings);
void mapgen_generate_custom_simplex(mapgen_settings *settings);
void mapgen_generate_forest(mapgen_settings *settings);
bool mapgen_load_heightmap(const utf8 *path);
void mapgen_unload_heightmap();
void mapgen_generate_from_heightmap(mapgen_settings *settings);

#ifdef __cplusplus
}
#endif

#endif
