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

#include "../rct2/addresses.h"
#include "../audio/audio.h"
#include "../cheats.h"
#include "../config/Config.h"
#include "../Context.h"
#include "../game.h"
#include "../interface/Cursors.h"
#include "../interface/window.h"
#include "../localisation/date.h"
#include "../localisation/localisation.h"
#include "../management/finance.h"
#include "../network/network.h"
#include "../OpenRCT2.h"
#include "../ride/ride_data.h"
#include "../ride/track.h"
#include "../ride/track_data.h"
#include "../scenario/scenario.h"
#include "../util/util.h"
#include "banner.h"
#include "Climate.h"
#include "footpath.h"
#include "map.h"
#include "map_animation.h"
#include "park.h"
#include "scenery.h"
#include "tile_inspector.h"

/**
 * Replaces 0x00993CCC, 0x00993CCE
 */
const rct_xy16 TileDirectionDelta[] = {
    { -32,   0 },
    {   0, +32 },
    { +32,   0 },
    {   0, -32 },
    { -32, +32 },
    { +32, +32 },
    { +32, -32 },
    { -32, -32 }
};

/** rct2: 0x0097B8B8 */
const money32 TerrainPricing[] = {
    300,    // TERRAIN_GRASS
    100,    // TERRAIN_SAND
    80,     // TERRAIN_DIRT
    120,    // TERRAIN_ROCK
    100,    // TERRAIN_MARTIAN
    100,    // TERRAIN_CHECKERBOARD
    110,    // TERRAIN_GRASS_CLUMPS
    130,    // TERRAIN_ICE
    110,    // TERRAIN_GRID_RED
    110,    // TERRAIN_GRID_YELLOW
    110,    // TERRAIN_GRID_BLUE
    110,    // TERRAIN_GRID_GREEN
    110,    // TERRAIN_SAND_DARK
    110,    // TERRAIN_SAND_LIGHT
};

uint16          gMapSelectFlags;
uint16          gMapSelectType;
rct_xy16        gMapSelectPositionA;
rct_xy16        gMapSelectPositionB;
rct_xyz16       gMapSelectArrowPosition;
uint8           gMapSelectArrowDirection;

uint8 gMapGroundFlags;

uint16 gWidePathTileLoopX;
uint16 gWidePathTileLoopY;
uint16 gGrassSceneryTileLoopPosition;

sint16 gMapSizeUnits;
sint16 gMapSizeMinus2;
sint16 gMapSize;
sint16 gMapSizeMaxXY;
sint16 gMapBaseZ;

#if defined(NO_RCT2)
rct_map_element gMapElements[MAX_TILE_MAP_ELEMENT_POINTERS * 3];
rct_map_element *gMapElementTilePointers[MAX_TILE_MAP_ELEMENT_POINTERS];
#else
rct_map_element *gMapElements = RCT2_ADDRESS(RCT2_ADDRESS_MAP_ELEMENTS, rct_map_element);
rct_map_element **gMapElementTilePointers = RCT2_ADDRESS(RCT2_ADDRESS_TILE_MAP_ELEMENT_POINTERS, rct_map_element*);
#endif
rct_xy16 gMapSelectionTiles[300];
rct2_peep_spawn gPeepSpawns[MAX_PEEP_SPAWNS];

rct_map_element *gNextFreeMapElement;
uint32 gNextFreeMapElementPointerIndex;

bool gLandMountainMode;
bool gLandPaintMode;
bool gClearSmallScenery;
bool gClearLargeScenery;
bool gClearFootpath;

uint16 gLandRemainingOwnershipSales;
uint16 gLandRemainingConstructionSales;

rct_xyz16 gCommandPosition;

bool gMapLandRightsUpdateSuccess;

static void map_update_grass_length(sint32 x, sint32 y, rct_map_element *mapElement);
static void map_set_grass_length(sint32 x, sint32 y, rct_map_element *mapElement, sint32 length);
static void clear_elements_at(sint32 x, sint32 y);
static void translate_3d_to_2d(sint32 rotation, sint32 *x, sint32 *y);

void rotate_map_coordinates(sint16 *x, sint16 *y, sint32 rotation)
{
    sint32 temp;

    switch (rotation) {
    case MAP_ELEMENT_DIRECTION_WEST:
        break;
    case MAP_ELEMENT_DIRECTION_NORTH:
        temp = *x;
        *x = *y;
        *y = -temp;
        break;
    case MAP_ELEMENT_DIRECTION_EAST:
        *x = -*x;
        *y = -*y;
        break;
    case MAP_ELEMENT_DIRECTION_SOUTH:
        temp = *y;
        *y = *x;
        *x = -temp;
        break;
    }
}

rct_xy16 coordinate_3d_to_2d(const rct_xyz16* coordinate_3d, sint32 rotation){
    rct_xy16 coordinate_2d;

    switch (rotation){
    default:
    case 0:
        coordinate_2d.x = coordinate_3d->y - coordinate_3d->x;
        coordinate_2d.y = ((coordinate_3d->y + coordinate_3d->x) >> 1) - coordinate_3d->z;
        break;
    case 1:
        coordinate_2d.x = -coordinate_3d->y - coordinate_3d->x;
        coordinate_2d.y = ((coordinate_3d->y - coordinate_3d->x) >> 1) - coordinate_3d->z;
        break;
    case 2:
        coordinate_2d.x = -coordinate_3d->y + coordinate_3d->x;
        coordinate_2d.y = ((-coordinate_3d->y - coordinate_3d->x) >> 1) - coordinate_3d->z;
        break;
    case 3:
        coordinate_2d.x = coordinate_3d->y + coordinate_3d->x;
        coordinate_2d.y = ((-coordinate_3d->y + coordinate_3d->x) >> 1) - coordinate_3d->z;
        break;
    }
    return coordinate_2d;
}

void map_element_iterator_begin(map_element_iterator *it)
{
    it->x = 0;
    it->y = 0;
    it->element = map_get_first_element_at(0, 0);
}

sint32 map_element_iterator_next(map_element_iterator *it)
{
    if (it->element == NULL) {
        it->element = map_get_first_element_at(it->x, it->y);
        return 1;
    }

    if (!map_element_is_last_for_tile(it->element)) {
        it->element++;
        return 1;
    }

    if (it->x < (MAXIMUM_MAP_SIZE_TECHNICAL - 1)) {
        it->x++;
        it->element = map_get_first_element_at(it->x, it->y);
        return 1;
    }

    if (it->y < (MAXIMUM_MAP_SIZE_TECHNICAL - 1)) {
        it->x = 0;
        it->y++;
        it->element = map_get_first_element_at(it->x, it->y);
        return 1;
    }

    return 0;
}

void map_element_iterator_restart_for_tile(map_element_iterator *it)
{
    it->element = NULL;
}

rct_map_element *map_get_first_element_at(sint32 x, sint32 y)
{
    if (x < 0 || y < 0 || x > (MAXIMUM_MAP_SIZE_TECHNICAL - 1) || y > (MAXIMUM_MAP_SIZE_TECHNICAL - 1)) {
        log_error("Trying to access element outside of range");
        return NULL;
    }
    return gMapElementTilePointers[x + y * MAXIMUM_MAP_SIZE_TECHNICAL];
}

rct_map_element *map_get_nth_element_at(sint32 x, sint32 y, sint32 n)
{
    rct_map_element * mapElement = map_get_first_element_at(x, y);
    if (mapElement == NULL) {
        return NULL;
    }
    // Iterate through elements on this tile. This has to be walked, rather than
    // jumped directly to, because n may exceed element count for given tile,
    // and the order of tiles (unlike elements) is not synced over multiplayer.
    while (n >= 0) {
        if (n == 0) {
            return mapElement;
        }
        if (map_element_is_last_for_tile(mapElement)) {
            break;
        }
        mapElement++;
        n--;
    }
    // The element sought for is not within given tile.
    return NULL;
}

void map_set_tile_elements(sint32 x, sint32 y, rct_map_element *elements)
{
    if (x < 0 || y < 0 || x > (MAXIMUM_MAP_SIZE_TECHNICAL - 1) || y > (MAXIMUM_MAP_SIZE_TECHNICAL - 1)) {
        log_error("Trying to access element outside of range");
        return;
    }
    gMapElementTilePointers[x + y * MAXIMUM_MAP_SIZE_TECHNICAL] = elements;
}

sint32 map_element_is_last_for_tile(const rct_map_element *element)
{
    return element->flags & MAP_ELEMENT_FLAG_LAST_TILE;
}

bool map_element_is_ghost(const rct_map_element *element)
{
    return element->flags & MAP_ELEMENT_FLAG_GHOST;
}

uint8 map_element_get_scenery_quadrant(const rct_map_element *element)
{
    return (element->type & MAP_ELEMENT_QUADRANT_MASK) >> 6;
}

sint32 map_element_get_type(const rct_map_element *element)
{
    return element->type & MAP_ELEMENT_TYPE_MASK;
}

sint32 map_element_get_direction(const rct_map_element *element)
{
    return element->type & MAP_ELEMENT_DIRECTION_MASK;
}

sint32 map_element_get_direction_with_offset(const rct_map_element *element, uint8 offset)
{
    return ((element->type & MAP_ELEMENT_DIRECTION_MASK) + offset) & MAP_ELEMENT_DIRECTION_MASK;
}

sint32 map_element_get_terrain(const rct_map_element *element)
{
    sint32 terrain = (element->properties.surface.terrain >> 5) & 7;
    if (element->type & 1)
        terrain |= (1 << 3);
    return terrain;
}

sint32 map_element_get_terrain_edge(const rct_map_element *element)
{
    sint32 terrain_edge = (element->properties.surface.slope >> 5) & 7;
    if (element->type & 128)
        terrain_edge |= (1 << 3);
    return terrain_edge;
}

void map_element_set_terrain(rct_map_element *element, sint32 terrain)
{
    // Bit 3 for terrain is stored in element.type bit 0
    if (terrain & 8)
        element->type |= 1;
    else
        element->type &= ~1;

    // Bits 0, 1, 2 for terrain are stored in element.terrain bit 5, 6, 7
    element->properties.surface.terrain &= ~0xE0;
    element->properties.surface.terrain |= (terrain & 7) << 5;
}

void map_element_set_terrain_edge(rct_map_element *element, sint32 terrain)
{
    // Bit 3 for terrain is stored in element.type bit 7
    if (terrain & 8)
        element->type |= 128;
    else
        element->type &= ~128;

    // Bits 0, 1, 2 for terrain are stored in element.slope bit 5, 6, 7
    element->properties.surface.slope &= ~0xE0;
    element->properties.surface.slope |= (terrain & 7) << 5;
}

rct_map_element *map_get_surface_element_at(sint32 x, sint32 y)
{
    rct_map_element *mapElement = map_get_first_element_at(x, y);

    if (mapElement == NULL)
        return NULL;

    // Find the first surface element
    while (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SURFACE) {
        if (map_element_is_last_for_tile(mapElement))
            return NULL;

        mapElement++;
    }

    return mapElement;
}

rct_map_element* map_get_path_element_at(sint32 x, sint32 y, sint32 z){
    rct_map_element *mapElement = map_get_first_element_at(x, y);

    if (mapElement == NULL)
        return NULL;

    // Find the path element at known z
    do {
        if (mapElement->flags & MAP_ELEMENT_FLAG_GHOST)
            continue;
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_PATH)
            continue;
        if (mapElement->base_height != z)
            continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
}

rct_map_element* map_get_banner_element_at(sint32 x, sint32 y, sint32 z, uint8 position) {
    rct_map_element *mapElement = map_get_first_element_at(x, y);

    if (mapElement == NULL)
        return NULL;

    // Find the banner element at known z and position
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_BANNER)
            continue;
        if (mapElement->base_height != z)
            continue;
        if (mapElement->properties.banner.position != position)
            continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
}

/**
 *
 *  rct2: 0x0068AB4C
 */
void map_init(sint32 size)
{
    gNumMapAnimations = 0;
    gNextFreeMapElementPointerIndex = 0;

    for (sint32 i = 0; i < MAX_TILE_MAP_ELEMENT_POINTERS; i++) {
        rct_map_element *map_element = &gMapElements[i];
        map_element->type = (MAP_ELEMENT_TYPE_SURFACE << 2);
        map_element->flags = MAP_ELEMENT_FLAG_LAST_TILE;
        map_element->base_height = 14;
        map_element->clearance_height = 14;
        map_element->properties.surface.slope = 0;
        map_element->properties.surface.grass_length = GRASS_LENGTH_CLEAR_0;
        map_element->properties.surface.ownership = 0;
        map_element->properties.surface.terrain = 0;

        map_element_set_terrain(map_element, TERRAIN_GRASS);
        map_element_set_terrain_edge(map_element, TERRAIN_EDGE_ROCK);
    }

    gGrassSceneryTileLoopPosition = 0;
    gWidePathTileLoopX = 0;
    gWidePathTileLoopY = 0;
    gMapSizeUnits = size * 32 - 32;
    gMapSizeMinus2 = size * 32 - 2;
    gMapSize = size;
    gMapSizeMaxXY = size * 32 - 33;
    gMapBaseZ = 7;
    map_update_tile_pointers();
    map_remove_out_of_range_elements();

    window_map_reset();
}

/**
 * Counts the number of surface tiles that offer land ownership rights for sale,
 * but haven't been bought yet. It updates gLandRemainingOwnershipSales and
 * gLandRemainingConstructionSales.
*/
void map_count_remaining_land_rights()
{
    gLandRemainingOwnershipSales = 0;
    gLandRemainingConstructionSales = 0;

    for (sint32 x = 0; x < MAXIMUM_MAP_SIZE_TECHNICAL; x++) {
        for (sint32 y = 0; y < MAXIMUM_MAP_SIZE_TECHNICAL; y++) {
            rct_map_element *element = map_get_surface_element_at(x, y);
            // Surface elements are sometimes hacked out to save some space for other map elements
            if (element == NULL) {
                continue;
            }

            uint8 flags = element->properties.surface.ownership;

            if ((flags & OWNERSHIP_AVAILABLE) && (flags & OWNERSHIP_OWNED) == 0) {
                gLandRemainingOwnershipSales++;
            } else if ((flags & OWNERSHIP_CONSTRUCTION_RIGHTS_AVAILABLE) && (flags & OWNERSHIP_CONSTRUCTION_RIGHTS_OWNED) == 0) {
                gLandRemainingConstructionSales++;
            }
        }
    }
}

/**
 * This is meant to strip MAP_ELEMENT_FLAG_GHOST flag from all elements when
 * importing a park.
 *
 * This can only exist in hacked parks, as we remove ghost elements while saving.
 *
 * This is less invasive than removing ghost elements themselves, as they can
 * contain valid data.
 */
void map_strip_ghost_flag_from_elements()
{
    rct_map_element *mapElement = gMapElements;
    do {
        mapElement->flags &= ~MAP_ELEMENT_FLAG_GHOST;
    } while (++mapElement < gMapElements + MAX_MAP_ELEMENTS);
}

/**
 *
 *  rct2: 0x0068AFFD
 */
void map_update_tile_pointers()
{
    sint32 i, x, y;

    for (i = 0; i < MAX_TILE_MAP_ELEMENT_POINTERS; i++) {
        gMapElementTilePointers[i] = TILE_UNDEFINED_MAP_ELEMENT;
    }

    rct_map_element *mapElement = gMapElements;
    rct_map_element **tile = gMapElementTilePointers;
    for (y = 0; y < MAXIMUM_MAP_SIZE_TECHNICAL; y++) {
        for (x = 0; x < MAXIMUM_MAP_SIZE_TECHNICAL; x++) {
            *tile++ = mapElement;
            while (!map_element_is_last_for_tile(mapElement++));
        }
    }

    gNextFreeMapElement = mapElement;
}

/**
 * Return the absolute height of an element, given its (x,y) coordinates
 *
 * ax: x
 * cx: y
 * dx: return remember to & with 0xFFFF if you don't want water affecting results
 *  rct2: 0x00662783
 */
sint32 map_element_height(sint32 x, sint32 y)
{
    rct_map_element *mapElement;

    // Off the map
    if ((unsigned)x >= 8192 || (unsigned)y >= 8192)
        return 16;

    // Truncate subtile coordinates
    sint32 x_tile = x & 0xFFFFFFE0;
    sint32 y_tile = y & 0xFFFFFFE0;

    // Get the surface element for the tile
    mapElement = map_get_surface_element_at(x_tile / 32, y_tile / 32);

    if (mapElement == NULL) {
        return 16;
    }

    uint32 height =
        (map_get_water_height(mapElement) << 20) |
        (mapElement->base_height << 3);

    uint32 slope = (mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK);
    uint8 extra_height = (slope & 0x10) >> 4; // 0x10 is the 5th bit - sets slope to double height
    // Remove the extra height bit
    slope &= 0xF;

    sint8 quad = 0, quad_extra = 0; // which quadrant the element is in?
                            // quad_extra is for extra height tiles

    uint8 xl, yl;       // coordinates across this tile

    uint8 TILE_SIZE = 31;

    xl = x & 0x1f;
    yl = y & 0x1f;

    // Slope logic:
    // Each of the four bits in slope represents that corner being raised
    // slope == 15 (all four bits) is not used and slope == 0 is flat
    // If the extra_height bit is set, then the slope goes up two z-levels

    // We arbitrarily take the SW corner to be closest to the viewer

    // One corner up
    if ((slope == 1) || (slope == 2) || (slope == 4) || (slope == 8)) {
        switch (slope) {
        case 1:    // NE corner up
            quad = xl + yl - TILE_SIZE;
            break;
        case 2:    // SE corner up
            quad = xl - yl;
            break;
        case 4:   // SW corner up
            quad = TILE_SIZE - yl - xl;
            break;
        case 8:   // NW corner up
            quad = yl - xl;
            break;
        }
        // If the element is in the quadrant with the slope, raise its height
        if (quad > 0) {
            height += quad / 2;
        }
    }

    // One side up
    switch (slope) {
    case 3:   // E side up
        height += xl / 2 + 1;
        break;
    case 6:   // S side up
        height += (TILE_SIZE - yl) / 2;
        break;
    case 9:    // N side up
        height += yl / 2;
        height++;
        break;
    case 12:  // W side up
        height += (TILE_SIZE - xl) / 2;
        break;
    }

    // One corner down
    if ((slope == 7) || (slope == 11) || (slope == 13) || (slope == 14)) {
        switch (slope) {
        case 7:   // NW corner down
            quad_extra = xl + TILE_SIZE - yl;
            quad = xl - yl;
            break;
        case 11:  // SW corner down
            quad_extra = xl + yl;
            quad = xl + yl - TILE_SIZE - 1;
            break;
        case 13:  // SE corner down
            quad_extra = TILE_SIZE - xl + yl;
            quad = yl - xl;
            break;
        case 14:  // NE corner down
            quad_extra = (TILE_SIZE - xl) + (TILE_SIZE - yl);
            quad = TILE_SIZE - yl - xl - 1;
            break;
        }

        if (extra_height) {
            height += quad_extra / 2;
            height++;
            return height;
        }
        // This tile is essentially at the next height level
        height += 0x10;
        // so we move *down* the slope
        if (quad < 0) {
            height += quad / 2;
        }
    }

    // Valleys
    if ((slope == 5) || (slope == 10)) {
        switch (slope) {
        case 5:  // NW-SE valley
            if (xl + yl <= TILE_SIZE + 1) {
                return height;
            }
            quad = TILE_SIZE - xl - yl;
            break;
        case 10: // NE-SW valley
            quad = xl - yl;
            break;
        }
        if (quad > 0) {
            height += quad / 2;
        }
    }

    return height;
}

/**
 *
 *  rct2: 0x0068B089
 */
void sub_68B089()
{
    sint32 i;
    rct_map_element *mapElementFirst, *mapElement;

    if (gTrackDesignSaveMode)
        return;

    i = gNextFreeMapElementPointerIndex;
    do {
        i++;
        if (i >= MAX_TILE_MAP_ELEMENT_POINTERS)
            i = 0;
    } while (gMapElementTilePointers[i] == TILE_UNDEFINED_MAP_ELEMENT);
    gNextFreeMapElementPointerIndex = i;

    mapElementFirst = mapElement = gMapElementTilePointers[i];
    do {
        mapElement--;
        if (mapElement < gMapElements)
            break;
    } while (mapElement->base_height == 255);
    mapElement++;

    if (mapElement == mapElementFirst)
        return;

    //
    gMapElementTilePointers[i] = mapElement;
    do {
        *mapElement = *mapElementFirst;
        mapElementFirst->base_height = 255;

        mapElementFirst++;
    } while (!map_element_is_last_for_tile(mapElement++));

    mapElement = gNextFreeMapElement;
    do {
        mapElement--;
    } while (mapElement->base_height == 255);
    mapElement++;
    gNextFreeMapElement = mapElement;
}


/**
 * Checks if the tile at coordinate at height counts as connected.
 * @return 1 if connected, 0 otherwise
 */
sint32 map_coord_is_connected(sint32 x, sint32 y, sint32 z, uint8 faceDirection)
{
    rct_map_element *mapElement = map_get_first_element_at(x, y);

    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_PATH)
            continue;

        rct_map_element_path_properties props = mapElement->properties.path;
        uint8 pathType = props.type >> 2;
        uint8 pathDirection = props.type & 3;

        if (pathType & 1) {
            if (pathDirection == faceDirection) {
                if (z == mapElement->base_height + 2)
                    return 1;
            } else if ((pathDirection ^ 2) == faceDirection && z == mapElement->base_height) {
                return 1;
            }
        } else {
            if (z == mapElement->base_height)
                return 1;
        }
    } while (!map_element_is_last_for_tile(mapElement++));

    return 0;
}

/**
 *
 *  rct2: 0x006A876D
 */
void map_update_path_wide_flags()
{
    if (gScreenFlags & (SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER)) {
        return;
    }

    // Presumably update_path_wide_flags is too computationally expensive to call for every
    // tile every update, so gWidePathTileLoopX and gWidePathTileLoopY store the x and y
    // progress. A maximum of 128 calls is done per update.
    uint16 x = gWidePathTileLoopX;
    uint16 y = gWidePathTileLoopY;
    for (sint32 i = 0; i < 128; i++) {
        footpath_update_path_wide_flags(x, y);

        // Next x, y tile
        x += 32;
        if (x >= 8192) {
            x = 0;
            y += 32;
            if (y >= 8192) {
                y = 0;
            }
        }
    }
    gWidePathTileLoopX = x;
    gWidePathTileLoopY = y;
}

/**
 *
 *  rct2: 0x006A7B84
 */
sint32 map_height_from_slope(sint32 x, sint32 y, sint32 slope)
{
    if (!(slope & 4))
        return 0;

    switch (slope & 3) {
    case 0:
        return (31 - (x & 31)) / 2;
    case 1:
        return (y & 31) / 2;
    case 2:
        return (x & 31) / 2;
    case 3:
        return (31 - (y & 31)) / 2;
    }
    return 0;
}

bool map_is_location_valid(sint32 x, sint32 y)
{
    if (x < (MAXIMUM_MAP_SIZE_TECHNICAL * 32) && x >= 0 && y < (MAXIMUM_MAP_SIZE_TECHNICAL * 32) && y >= 0) {
        return true;
    }
    return false;
}

bool map_can_build_at(sint32 x, sint32 y, sint32 z)
{
    if (gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR)
        return true;
    if (gCheatsSandboxMode)
        return true;
    if (map_is_location_owned(x, y, z))
        return true;
    return false;
}

/**
 *
 *  rct2: 0x00664F72
 */
bool map_is_location_owned(sint32 x, sint32 y, sint32 z)
{
    // This check is to avoid throwing lots of messages in logs.
    if (map_is_location_valid(x, y)) {
        rct_map_element *mapElement = map_get_surface_element_at(x / 32, y / 32);
        if (mapElement != NULL) {
            if (mapElement->properties.surface.ownership & OWNERSHIP_OWNED)
                return true;

            if (mapElement->properties.surface.ownership & OWNERSHIP_CONSTRUCTION_RIGHTS_OWNED) {
                z /= 8;
                if (z < mapElement->base_height || z - 2 > mapElement->base_height)
                    return true;
            }
        }
    }

    gGameCommandErrorText = STR_LAND_NOT_OWNED_BY_PARK;
    return false;
}

/**
 *
 *  rct2: 0x00664F2C
 */
bool map_is_location_in_park(sint32 x, sint32 y)
{
    if (map_is_location_valid(x, y)) {
        rct_map_element *mapElement = map_get_surface_element_at(x / 32, y / 32);
        if (mapElement == NULL)
            return false;
        if (mapElement->properties.surface.ownership & OWNERSHIP_OWNED)
            return true;
    }

    gGameCommandErrorText = STR_LAND_NOT_OWNED_BY_PARK;
    return false;
}

bool map_is_location_owned_or_has_rights(sint32 x, sint32 y)
{
    if (map_is_location_valid(x, y)) {
        rct_map_element *mapElement = map_get_surface_element_at(x / 32, y / 32);
        if (mapElement == NULL) {
            return false;
        }
        if (mapElement->properties.surface.ownership & OWNERSHIP_OWNED) return true;
        if (mapElement->properties.surface.ownership & OWNERSHIP_CONSTRUCTION_RIGHTS_OWNED) return true;
    }
    return false;
}

/**
 *
 *  rct2: 0x006B8E1B
 */
void game_command_remove_large_scenery(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    uint8 base_height = *edx;
    uint8 tileIndex = *edx >> 8;
    uint8 map_element_direction = *ebx >> 8;
    sint32 x = *eax;
    sint32 y = *ecx;
    sint32 z = map_element_height(x, y);
    uint8 flags = *ebx & 0xFF;
    gCommandPosition.x = x + 16;
    gCommandPosition.y = y + 16;
    gCommandPosition.z = z;
    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;

    if (!(flags & GAME_COMMAND_FLAG_GHOST) && game_is_paused() && !gCheatsBuildInPauseMode) {
        gGameCommandErrorText = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    bool element_found = false;
    rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
    if (map_element == NULL)
    {
        log_warning("Invalid game command for scenery removal, x = %d, y = %d", x, y);
        *ebx = MONEY32_UNDEFINED;
        return;
    }
    do {
        if (map_element_get_type(map_element) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE)
            continue;

        if (map_element->base_height != base_height)
            continue;

        if ((map_element->properties.scenerymultiple.type >> 10) != tileIndex)
            continue;

        if ((map_element_get_direction(map_element)) != map_element_direction)
            continue;

        // If we are removing ghost elements
        if((flags & GAME_COMMAND_FLAG_GHOST) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST))
            continue;

        element_found = true;
        break;
    } while (!map_element_is_last_for_tile(map_element++));

    if (element_found == false){
        *ebx = 0;
        return;
    }

    if (flags & GAME_COMMAND_FLAG_APPLY) {
        map_element_remove_banner_entry(map_element);
    }

    rct_scenery_entry* scenery_entry = get_large_scenery_entry(map_element->properties.scenerymultiple.type & 0x3FF);
    rct_xyz16 firstTile = {
        .x = scenery_entry->large_scenery.tiles[tileIndex].x_offset,
        .y = scenery_entry->large_scenery.tiles[tileIndex].y_offset,
        .z = (base_height * 8) - scenery_entry->large_scenery.tiles[tileIndex].z_offset
    };

    rotate_map_coordinates(&firstTile.x, &firstTile.y, map_element_direction);

    firstTile.x = x - firstTile.x;
    firstTile.y = y - firstTile.y;

    bool calculate_cost = true;
    for (sint32 i = 0; scenery_entry->large_scenery.tiles[i].x_offset != -1; i++){

        rct_xyz16 currentTile = {
            .x = scenery_entry->large_scenery.tiles[i].x_offset,
            .y = scenery_entry->large_scenery.tiles[i].y_offset,
            .z = scenery_entry->large_scenery.tiles[i].z_offset
        };

        rotate_map_coordinates(&currentTile.x, &currentTile.y, map_element_direction);

        currentTile.x += firstTile.x;
        currentTile.y += firstTile.y;
        currentTile.z += firstTile.z;

        if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode){
            if (!map_is_location_owned(currentTile.x, currentTile.y, currentTile.z)){
                *ebx = MONEY32_UNDEFINED;
                return;
            }
        }

        // If not applying then no need to delete the actual element
        if (!(flags & GAME_COMMAND_FLAG_APPLY)) {
            if (flags & (1 << 7)) {
                if (map_element->flags & (1 << 6))
                    calculate_cost = false;
                map_element->flags |= (1 << 6);
            }
            continue;
        }

        rct_map_element* sceneryElement = map_get_first_element_at(currentTile.x / 32, currentTile.y / 32);
        element_found = false;
        do
        {
            if (map_element_get_type(sceneryElement) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE)
                continue;

            if (map_element_get_direction(sceneryElement) != map_element_direction)
                continue;

            if ((sceneryElement->properties.scenerymultiple.type >> 10) != i)
                continue;

            if (sceneryElement->base_height != currentTile.z / 8)
                continue;

            // If we are removing ghost elements
            if ((flags & GAME_COMMAND_FLAG_GHOST) && !(sceneryElement->flags & MAP_ELEMENT_FLAG_GHOST))
                continue;

            map_invalidate_tile_full(currentTile.x, currentTile.y);
            map_element_remove(sceneryElement);
            element_found = true;
            break;
        } while (!map_element_is_last_for_tile(sceneryElement++));

        if (element_found == false){
            log_error("Tile not found when trying to remove element!");
        }
    }

    if (flags & GAME_COMMAND_FLAG_APPLY && gGameCommandNestLevel == 1 && !(flags & GAME_COMMAND_FLAG_GHOST)) {
        rct_xyz16 coord;
        coord.x = x + 16;
        coord.y = y + 16;
        coord.z = map_element_height(coord.x, coord.y);
        network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);
    }

    *ebx = scenery_entry->large_scenery.removal_price * 10;
    if (gParkFlags & PARK_FLAGS_NO_MONEY ||
        calculate_cost == false){
        *ebx = 0;
    }
    return;
}

/**
 *
 *  rct2: 0x006B909A
 */
void game_command_set_large_scenery_colour(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;
    sint32 x = *eax;
    sint32 y = *ecx;
    uint8 map_element_direction = *ebx >> 8;
    uint8 flags = *ebx & 0xFF;
    uint8 base_height = *edx;
    uint8 tileIndex = *edx >> 8;
    uint8 colour1 = *ebp;
    uint8 colour2 = *ebp >> 8;
    sint32 z = map_element_height(x, y);
    gCommandPosition.x = x + 16;
    gCommandPosition.y = y + 16;
    gCommandPosition.z = z;

    rct_map_element *map_element = map_get_large_scenery_segment(x, y, base_height, map_element_direction, tileIndex);

    if(map_element == NULL){
        *ebx = 0;
        return;
    }

    if((flags & GAME_COMMAND_FLAG_GHOST) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST)){
        *ebx = 0;
        return;
    }

    rct_scenery_entry *scenery_entry = get_large_scenery_entry(map_element->properties.scenerymultiple.type & 0x3FF);

    // Work out the base tile coordinates (Tile with index 0)
    rct_xyz16 baseTile = {
        .x = scenery_entry->large_scenery.tiles[tileIndex].x_offset,
        .y = scenery_entry->large_scenery.tiles[tileIndex].y_offset,
        .z = (base_height * 8) - scenery_entry->large_scenery.tiles[tileIndex].z_offset
    };
    rotate_map_coordinates(&baseTile.x, &baseTile.y, map_element_direction);
    baseTile.x = x - baseTile.x;
    baseTile.y = y - baseTile.y;

    for (sint32 i = 0; scenery_entry->large_scenery.tiles[i].x_offset != -1; ++i) {
        assert(i < MAXIMUM_MAP_SIZE_TECHNICAL);

        // Work out the current tile coordinates
        rct_xyz16 currentTile = {
            .x = scenery_entry->large_scenery.tiles[i].x_offset,
            .y = scenery_entry->large_scenery.tiles[i].y_offset,
            .z = scenery_entry->large_scenery.tiles[i].z_offset
        };
        rotate_map_coordinates(&currentTile.x, &currentTile.y, map_element_direction);
        currentTile.x += baseTile.x;
        currentTile.y += baseTile.y;
        currentTile.z += baseTile.z;

        if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode){
            if (!map_is_location_owned(currentTile.x, currentTile.y, currentTile.z)){
                *ebx = MONEY32_UNDEFINED;
                return;
            }
        }

        if(flags & GAME_COMMAND_FLAG_APPLY){
            rct_map_element* mapElement = map_get_large_scenery_segment(
                currentTile.x,
                currentTile.y,
                base_height,
                map_element_direction,
                i);

            mapElement->properties.scenerymultiple.colour[0] &= 0xE0;
            mapElement->properties.scenerymultiple.colour[0] |= colour1;
            mapElement->properties.scenerymultiple.colour[1] &= 0xE0;
            mapElement->properties.scenerymultiple.colour[1] |= colour2;

            map_invalidate_tile_full(currentTile.x, currentTile.y);
        }
    }
    *ebx = 0;
}

// This will cause clear scenery to remove paths
// This should be a flag for the game command which can be set via a checkbox on the clear scenery window.
// #define CLEAR_SCENERY_REMOVES_PATHS

/**
 *
 *  rct2: 0x0068DFE4
 */
static money32 map_clear_scenery_from_tile(sint32 x, sint32 y, sint32 clear, sint32 flags)
{
    sint32 type;
    money32 cost, totalCost;
    rct_map_element *mapElement;

    totalCost = 0;

restart_from_beginning:
    mapElement = map_get_first_element_at(x, y);
    do {
        type = map_element_get_type(mapElement);
        switch (type) {
        case MAP_ELEMENT_TYPE_PATH:
            if (clear & (1 << 2)) {
                sint32 eax = x * 32;
                sint32 ebx = flags;
                sint32 ecx = y * 32;
                sint32 edx = mapElement->base_height;
                sint32 edi = 0, ebp = 0;
                cost = game_do_command(eax, ebx, ecx, edx, GAME_COMMAND_REMOVE_PATH, edi, ebp);

                if (cost == MONEY32_UNDEFINED)
                    return MONEY32_UNDEFINED;

                totalCost += cost;
                if (flags & 1)
                    goto restart_from_beginning;
            } break;
        case MAP_ELEMENT_TYPE_SCENERY:
            if (clear & (1 << 0)) {
                sint32 eax = x * 32;
                sint32 ebx = (mapElement->type << 8) | flags;
                sint32 ecx = y * 32;
                sint32 edx = (mapElement->properties.scenery.type << 8) | (mapElement->base_height);
                sint32 edi = 0, ebp = 0;
                cost = game_do_command(eax, ebx, ecx, edx, GAME_COMMAND_REMOVE_SCENERY, edi, ebp);

                if (cost == MONEY32_UNDEFINED)
                    return MONEY32_UNDEFINED;

                totalCost += cost;
                if (flags & 1)
                    goto restart_from_beginning;

            } break;
        case MAP_ELEMENT_TYPE_WALL:
            if (clear & (1 << 0)) {
                sint32 eax = x * 32;
                sint32 ebx = flags;
                sint32 ecx = y * 32;
                sint32 edx = (mapElement->base_height << 8) | (map_element_get_direction(mapElement));
                sint32 edi = 0, ebp = 0;
                cost = game_do_command(eax, ebx, ecx, edx, GAME_COMMAND_REMOVE_WALL, edi, ebp);

                if (cost == MONEY32_UNDEFINED)
                    return MONEY32_UNDEFINED;

                totalCost += cost;
                if (flags & 1)
                    goto restart_from_beginning;

            } break;
        case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
            if (clear & (1 << 1)) {
                sint32 eax = x * 32;
                sint32 ebx = flags | ((map_element_get_direction(mapElement)) << 8);
                sint32 ecx = y * 32;
                sint32 edx = mapElement->base_height | ((mapElement->properties.scenerymultiple.type >> 10) << 8);
                sint32 edi = 0, ebp = 0;
                cost = game_do_command(eax, ebx | (1 << 7), ecx, edx, GAME_COMMAND_REMOVE_LARGE_SCENERY, edi, ebp);

                if (cost == MONEY32_UNDEFINED)
                    return MONEY32_UNDEFINED;

                totalCost += cost;
                if (flags & 1)
                    goto restart_from_beginning;

            } break;
        }
    } while (!map_element_is_last_for_tile(mapElement++));

    return totalCost;
}

/**
 * Function to clear the flag that is set to prevent cost duplication
 * when using the clear scenery tool with large scenery.
 */
static void map_reset_clear_large_scenery_flag(){
    rct_map_element* mapElement;
    // TODO: Improve efficiency of this
    for (sint32 y = 0; y < MAXIMUM_MAP_SIZE_TECHNICAL; y++) {
        for (sint32 x = 0; x < MAXIMUM_MAP_SIZE_TECHNICAL; x++) {
            mapElement = map_get_first_element_at(x, y);
            do {
                if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_SCENERY_MULTIPLE) {
                    mapElement->flags &= ~(1 << 6);
                }
            } while (!map_element_is_last_for_tile(mapElement++));
        }
    }
}

money32 map_clear_scenery(sint32 x0, sint32 y0, sint32 x1, sint32 y1, sint32 clear, sint32 flags)
{
    sint32 x, y, z;
    money32 totalCost, cost;
    bool noValidTiles;

    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;

    x = (x0 + x1) / 2 + 16;
    y = (y0 + y1) / 2 + 16;
    z = map_element_height(x, y);
    gCommandPosition.x = x;
    gCommandPosition.y = y;
    gCommandPosition.z = z;

    x0 = max(x0, 32);
    y0 = max(y0, 32);
    x1 = min(x1, gMapSizeMaxXY);
    y1 = min(y1, gMapSizeMaxXY);

    noValidTiles = true;
    totalCost = 0;
    for (y = y0; y <= y1; y += 32) {
        for (x = x0; x <= x1; x += 32) {
            if ((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) || gCheatsSandboxMode || map_is_location_owned_or_has_rights(x, y)) {
                cost = map_clear_scenery_from_tile(x / 32, y / 32, clear, flags);
                if (cost != MONEY32_UNDEFINED) {
                    noValidTiles = false;
                    totalCost += cost;
                }
            } else {
                gGameCommandErrorText = STR_LAND_NOT_OWNED_BY_PARK;
            }
        }
    }

    if (gGameCommandNestLevel == 1 && flags & GAME_COMMAND_FLAG_APPLY) {
        rct_xyz16 coord;
        coord.x = ((x0 + x1) / 2) + 16;
        coord.y = ((y0 + y1) / 2) + 16;
        coord.z = map_element_height(coord.x, coord.y);
        network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);
    }

    if (clear & (1 << 1)) {
        map_reset_clear_large_scenery_flag();
    }

    return noValidTiles ? MONEY32_UNDEFINED : totalCost;
}

/**
 *
 *  rct2: 0x0068DF91
 */
void game_command_clear_scenery(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    *ebx = map_clear_scenery(
        (sint16)(*eax & 0xFFFF),
        (sint16)(*ecx & 0xFFFF),
        (sint16)(*edi & 0xFFFF),
        (sint16)(*ebp & 0xFFFF),
        *edx,
        *ebx & 0xFF
    );
}

/**
 *
 *  rct2: 0x00663CCD
 */
static money32 map_change_surface_style(sint32 x0, sint32 y0, sint32 x1, sint32 y1, uint8 surfaceStyle, uint8 edgeStyle, uint8 flags)
{
    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;

    x0 = max(x0, 32);
    y0 = max(y0, 32);
    x1 = min(x1, gMapSizeMaxXY);
    y1 = min(y1, gMapSizeMaxXY);

    sint32 xMid, yMid;

    xMid = (x0 + x1) / 2 + 16;
    yMid = (y0 + y1) / 2 + 16;

    sint32 heightMid = map_element_height(xMid, yMid);

    gCommandPosition.x = xMid;
    gCommandPosition.y = yMid;
    gCommandPosition.z = heightMid;

    // Do nothing during pause
    if (game_is_paused() && !gCheatsBuildInPauseMode) {
        return 0;
    }

    // Do nothing if not in editor, sandbox mode or landscaping is forbidden
    if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) &&
        !gCheatsSandboxMode &&
        (gParkFlags & PARK_FLAGS_FORBID_LANDSCAPE_CHANGES)
    ) {
        return 0;
    }

    money32 surfaceCost = 0;
    money32 edgeCost = 0;
    for (sint32 x = x0; x <= x1; x += 32) {
        for (sint32 y = y0; y <= y1; y += 32) {
            if (x > 0x1FFF) continue;
            if (y > 0x1FFF) continue;

            if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode) {
                if (!map_is_location_in_park(x, y)) continue;
            }

            rct_map_element* mapElement = map_get_surface_element_at(x / 32, y / 32);

            if (surfaceStyle != 0xFF){
                uint8 cur_terrain = (
                    map_element_get_direction(mapElement) << 3) |
                    (mapElement->properties.surface.terrain >> 5);

                if (surfaceStyle != cur_terrain) {
                    // Prevent network-originated value of surfaceStyle from causing
                    // invalid access.
                    uint8 style = surfaceStyle & 0x1F;
                    if (style >= countof(TerrainPricing)) {
                        return MONEY32_UNDEFINED;
                    }
                    surfaceCost += TerrainPricing[style];
                    if (flags & 1){
                        mapElement->properties.surface.terrain &= MAP_ELEMENT_WATER_HEIGHT_MASK;
                        mapElement->type &= MAP_ELEMENT_QUADRANT_MASK | MAP_ELEMENT_TYPE_MASK;

                        //Save the new terrain
                        mapElement->properties.surface.terrain |= surfaceStyle << 5;

                        //Save the new direction mask
                        mapElement->type |= (surfaceStyle >> 3) & MAP_ELEMENT_DIRECTION_MASK;

                        map_invalidate_tile_full(x, y);
                        footpath_remove_litter(x, y, map_element_height(x, y));
                    }
                }
            }

            if (edgeStyle != 0xFF) {
                uint8 currentEdge =
                    ((mapElement->type & 0x80) >> 4) |
                    (mapElement->properties.surface.slope >> 5);

                if (edgeStyle != currentEdge){
                    edgeCost += 100;

                    if (flags & 1){
                        mapElement->properties.surface.slope &= MAP_ELEMENT_SLOPE_MASK;
                        mapElement->type &= 0x7F;

                        //Save edge style
                        mapElement->properties.surface.slope |= edgeStyle << 5;

                        //Save ???
                        mapElement->type |= (edgeStyle << 4) & 0x80;
                        map_invalidate_tile_full(x, y);
                    }
                }
            }

            if (flags & 1)
            {
                if (!(mapElement->properties.surface.terrain & MAP_ELEMENT_SURFACE_TERRAIN_MASK))
                {
                    if (!(map_element_get_direction(mapElement)))
                    {
                        if ((mapElement->properties.surface.grass_length & 7) != GRASS_LENGTH_CLEAR_0) {
                            mapElement->properties.surface.grass_length = GRASS_LENGTH_CLEAR_0;
                            map_invalidate_tile_full(x, y);
                        }
                    }
                }
            }
        }
    }

    if (flags & GAME_COMMAND_FLAG_APPLY && gGameCommandNestLevel == 1) {
        rct_xyz16 coord;
        coord.x = ((x0 + x1) / 2) + 16;
        coord.y = ((y0 + y1) / 2) + 16;
        coord.z = map_element_height(coord.x, coord.y);
        network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);
    }

    return (gParkFlags & PARK_FLAGS_NO_MONEY) ? 0 : surfaceCost + edgeCost;
}

/**
 *
 *  rct2: 0x00663CCD
 */
void game_command_change_surface_style(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    *ebx = map_change_surface_style(
        (sint16)(*eax & 0xFFFF),
        (sint16)(*ecx & 0xFFFF),
        (sint16)(*edi & 0xFFFF),
        (sint16)(*ebp & 0xFFFF),
        *edx & 0xFF,
        (*edx & 0xFF00) >> 8,
        *ebx & 0xFF
    );
}

//0x00981A1E
const uint8 map_element_raise_styles[5][32] = {
    { 0x01, 0x1B, 0x03, 0x1B, 0x05, 0x21, 0x07, 0x21, 0x09, 0x1B, 0x0B, 0x1B, 0x0D, 0x21, 0x20, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x23, 0x18, 0x19, 0x1A, 0x3B, 0x1C, 0x29, 0x24, 0x1F },
    { 0x02, 0x03, 0x17, 0x17, 0x06, 0x07, 0x17, 0x17, 0x0A, 0x0B, 0x22, 0x22, 0x0E, 0x20, 0x22, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x37, 0x18, 0x19, 0x1A, 0x23, 0x1C, 0x28, 0x26, 0x1F },
    { 0x04, 0x05, 0x06, 0x07, 0x1E, 0x24, 0x1E, 0x24, 0x0C, 0x0D, 0x0E, 0x20, 0x1E, 0x24, 0x1E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x26, 0x18, 0x19, 0x1A, 0x21, 0x1C, 0x2C, 0x3E, 0x1F },
    { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x20, 0x1D, 0x1D, 0x28, 0x28, 0x1D, 0x1D, 0x28, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x22, 0x18, 0x19, 0x1A, 0x29, 0x1C, 0x3D, 0x2C, 0x1F },
    { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x22, 0x20, 0x20, 0x20, 0x21, 0x20, 0x28, 0x24, 0x20 },
};

//0x00981ABE
const uint8 map_element_lower_styles[5][32] = {
    { 0x2E, 0x00, 0x2E, 0x02, 0x3E, 0x04, 0x3E, 0x06, 0x2E, 0x08, 0x2E, 0x0A, 0x3E, 0x0C, 0x3E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x06, 0x18, 0x19, 0x1A, 0x0B, 0x1C, 0x0C, 0x3E, 0x1F },
    { 0x2D, 0x2D, 0x00, 0x01, 0x2D, 0x2D, 0x04, 0x05, 0x3D, 0x3D, 0x08, 0x09, 0x3D, 0x3D, 0x0C, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x07, 0x18, 0x19, 0x1A, 0x09, 0x1C, 0x3D, 0x0C, 0x1F },
    { 0x2B, 0x3B, 0x2B, 0x3B, 0x00, 0x01, 0x02, 0x03, 0x2B, 0x3B, 0x2B, 0x3B, 0x08, 0x09, 0x0A, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x03, 0x18, 0x19, 0x1A, 0x3B, 0x1C, 0x09, 0x0E, 0x1F },
    { 0x27, 0x27, 0x37, 0x37, 0x27, 0x27, 0x37, 0x37, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x37, 0x18, 0x19, 0x1A, 0x03, 0x1C, 0x0D, 0x06, 0x1F },
    { 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x0D, 0x0E, 0x00 },
};

/**
 *
 *  rct2: 0x00663CB9
 */
static sint32 map_set_land_height_clear_func(rct_map_element** map_element, sint32 x, sint32 y, uint8 flags, money32* price) {
    if (map_element_get_type(*map_element) == MAP_ELEMENT_TYPE_SURFACE)
        return 0;

    if (map_element_get_type(*map_element) == MAP_ELEMENT_TYPE_SCENERY)
        return 0;

    return 1;
}

static sint32 map_get_corner_height(sint32 z, sint32 slope, sint32 direction)
{
    switch (direction) {
    case 0:
        if (slope & 1) {
            z += 2;
            if (slope == 27) {
                z += 2;
            }
        }
        break;
    case 1:
        if (slope & 2) {
            z += 2;
            if (slope == 23) {
                z += 2;
            }
        }
        break;
    case 2:
        if (slope & 4) {
            z += 2;
            if (slope == 30) {
                z += 2;
            }
        }
        break;
    case 3:
        if (slope & 8) {
            z += 2;
            if (slope == 29) {
                z += 2;
            }
        }
        break;
    }
    return z;
}

static sint32 map_element_get_corner_height(rct_map_element *mapElement, sint32 direction)
{
    sint32 z = mapElement->base_height;
    sint32 slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
    return map_get_corner_height(z, slope, direction);
}

static money32 map_set_land_height(sint32 flags, sint32 x, sint32 y, sint32 height, sint32 style, sint32 selectionType)
{
    rct_map_element *mapElement;

    if (game_is_paused() && !gCheatsBuildInPauseMode) {
        gGameCommandErrorText = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
        return MONEY32_UNDEFINED;
    }

    if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode) {
        if (gParkFlags & PARK_FLAGS_FORBID_LANDSCAPE_CHANGES) {
            gGameCommandErrorText = STR_FORBIDDEN_BY_THE_LOCAL_AUTHORITY;
            return MONEY32_UNDEFINED;
        }
    }

    if (x > gMapSizeMaxXY || y > gMapSizeMaxXY) {
        gGameCommandErrorText = STR_OFF_EDGE_OF_MAP;
        return MONEY32_UNDEFINED;
    }

    if (height < MINIMUM_LAND_HEIGHT) {
        gGameCommandErrorText = STR_TOO_LOW;
        return MONEY32_UNDEFINED;
    }

    // Divide by 2 and subtract 7 to get the in-game units.
    if (height > MAXIMUM_LAND_HEIGHT) {
        gGameCommandErrorText = STR_TOO_HIGH;
        return MONEY32_UNDEFINED;
    } else if (height > MAXIMUM_LAND_HEIGHT - 2 && (style & 0x1F) != 0) {
        gGameCommandErrorText = STR_TOO_HIGH;
        return MONEY32_UNDEFINED;
    }

    if (height == MAXIMUM_LAND_HEIGHT - 2 && (style & 0x10)) {
        gGameCommandErrorText = STR_TOO_HIGH;
        return MONEY32_UNDEFINED;
    }

    if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode) {
        if (!map_is_location_in_park(x, y)) {
            return MONEY32_UNDEFINED;
        }
    }

    money32 cost = MONEY(0, 0);
    if(flags & GAME_COMMAND_FLAG_APPLY)
    {
        footpath_remove_litter(x, y, map_element_height(x, y));
        if(!gCheatsDisableClearanceChecks)
            wall_remove_at(x, y, height * 8 - 16, height * 8 + 32);
    }

    if (!gCheatsDisableClearanceChecks) {
        //Check for obstructing scenery
        mapElement = map_get_first_element_at(x / 32, y / 32);
        do {
            if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY)
                continue;
            if (height > mapElement->clearance_height)
                continue;
            if (height + 4 < mapElement->base_height)
                continue;
            rct_scenery_entry *sceneryEntry = get_small_scenery_entry(mapElement->properties.scenery.type);
            if (sceneryEntry->small_scenery.height > 64 && gParkFlags & PARK_FLAGS_FORBID_TREE_REMOVAL)
            {
                map_obstruction_set_error_text(mapElement);
                return MONEY32_UNDEFINED;
            }
            cost += MONEY(sceneryEntry->small_scenery.removal_price, 0);
            if (flags & GAME_COMMAND_FLAG_APPLY)
                map_element_remove(mapElement--);
        } while (!map_element_is_last_for_tile(mapElement++));
    }

    //Check for ride support limits
    if(gCheatsDisableSupportLimits==false)
    {
        mapElement = map_get_first_element_at(x / 32, y / 32);
        do{
            if(map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK)
                continue;
            sint32 rideIndex = mapElement->properties.track.ride_index;
            sint32 maxHeight = get_ride_entry_by_ride(get_ride(rideIndex))->max_height;
            if(maxHeight == 0)
                maxHeight = RideData5[get_ride(rideIndex)->type].max_height;
            sint32 zDelta = mapElement->clearance_height - height;
            if(zDelta >= 0 && zDelta/2 > maxHeight)
            {
                gGameCommandErrorText = STR_SUPPORTS_CANT_BE_EXTENDED;
                return MONEY32_UNDEFINED;
            }
        }while(!map_element_is_last_for_tile(mapElement++));
    }

    uint8 zCorner = height; //z position of highest corner of tile
    rct_map_element *surfaceElement = map_get_surface_element_at(x / 32, y / 32);
    if(surfaceElement->type & MAP_ELEMENT_TYPE_FLAG_HIGHLIGHT)
    {
        sint32 waterHeight = map_get_water_height(surfaceElement);
        if(waterHeight != 0)
        {
            if(style & 0x1F)
            {
                zCorner += 2;
                if(style & 0x10)
                {
                    zCorner += 2;
                }
            }
            if(zCorner > waterHeight * 2 - 2)
            {
                surfaceElement++;
                map_obstruction_set_error_text(surfaceElement);
                return MONEY32_UNDEFINED;
            }
        }
    }

    zCorner = height;
    if(style & 0xF)
    {
        zCorner += 2;
        if(style & 0x10)
        {
            zCorner += 2;
        }
    }

    if (!gCheatsDisableClearanceChecks) {
        if (!map_can_construct_with_clear_at(x, y, height, zCorner, &map_set_land_height_clear_func, 0xF, 0, NULL)) {
            return MONEY32_UNDEFINED;
        }
    }

    if (!gCheatsDisableClearanceChecks) {
        mapElement = map_get_first_element_at(x / 32, y / 32);
        do {
            sint32 elementType = map_element_get_type(mapElement);

            if (elementType == MAP_ELEMENT_TYPE_WALL)
                continue;
            if (elementType == MAP_ELEMENT_TYPE_SCENERY)
                continue;
            if (mapElement->flags & 0x10)
                continue;
            if (mapElement == surfaceElement)
                continue;
            if (mapElement > surfaceElement)
            {
                if (zCorner > mapElement->base_height)
                {
                    map_obstruction_set_error_text(mapElement);
                    return MONEY32_UNDEFINED;
                }
                continue;
            }
            if (height < mapElement->clearance_height)
            {
                map_obstruction_set_error_text(mapElement);
                return MONEY32_UNDEFINED;
            }
        } while (!map_element_is_last_for_tile(mapElement++));
    }

    for (sint32 i = 0; i < 4; i += 1) {
        sint32 cornerHeight = map_element_get_corner_height(surfaceElement, i);
        cornerHeight -= map_get_corner_height(height, style & MAP_ELEMENT_SLOPE_MASK, i);
        cost += MONEY(abs(cornerHeight) * 5 / 2, 0);
    }

    if(flags & GAME_COMMAND_FLAG_APPLY)
    {
        if (gGameCommandNestLevel == 1) {
            rct_xyz16 coord;
            coord.x = x + 16;
            coord.y = y + 16;
            coord.z = map_element_height(coord.x, coord.y);
            network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);
        }

        surfaceElement = map_get_surface_element_at(x / 32, y / 32);
        surfaceElement->base_height = height;
        surfaceElement->clearance_height = height;
        surfaceElement->properties.surface.slope &= MAP_ELEMENT_SLOPE_EDGE_STYLE_MASK;
        surfaceElement->properties.surface.slope |= style;
        sint32 slope = surfaceElement->properties.surface.terrain & MAP_ELEMENT_SLOPE_MASK;
        if(slope != 0 && slope <= height / 2)
            surfaceElement->properties.surface.terrain &= MAP_ELEMENT_SURFACE_TERRAIN_MASK;
        map_invalidate_tile_full(x, y);
    }
    if(gParkFlags & PARK_FLAGS_NO_MONEY)
        return 0;
    return cost;
}

void game_command_set_land_height(sint32 *eax, sint32 *ebx, sint32 *ecx, sint32 *edx, sint32 *esi, sint32 *edi, sint32 *ebp)
{
    *ebx = map_set_land_height(
        *ebx & 0xFF,
        *eax & 0xFFFF,
        *ecx & 0xFFFF,
        *edx & 0xFF,
        (*edx >> 8) & 0xFF,
        *edi >> 5
    );
}

static money32 map_set_land_ownership(uint8 flags, sint16 x1, sint16 y1, sint16 x2, sint16 y2, uint8 newOwnership) {
    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LAND_PURCHASE;

    if (!(flags & GAME_COMMAND_FLAG_APPLY))
        return 0;

    // Clamp to maximum addressable element to prevent long loop spamming the log
    x1 = clamp(32, x1, gMapSizeUnits - 32);
    y1 = clamp(32, y1, gMapSizeUnits - 32);
    x2 = clamp(32, x2, gMapSizeUnits - 32);
    y2 = clamp(32, y2, gMapSizeUnits - 32);
    gMapLandRightsUpdateSuccess = false;
    map_buy_land_rights(x1, y1, x2, y2, BUY_LAND_RIGHTS_FLAG_SET_OWNERSHIP_WITH_CHECKS, flags | (newOwnership << 8));

    if (!gMapLandRightsUpdateSuccess)
        return 0;

    sint16 x = clamp(32, x1, gMapSizeUnits - 32);
    sint16 y = clamp(32, y1, gMapSizeUnits - 32);

    x += 16;
    y += 16;

    sint16 z = map_element_height(x, y) & 0xFFFF;
    audio_play_sound_at_location(SOUND_PLACE_ITEM, x, y, z);
    return 0;
}

/**
 *
 *  rct2: 0x006648E3
 */
void game_command_set_land_ownership(sint32 *eax, sint32 *ebx, sint32 *ecx, sint32 *edx, sint32 *esi, sint32 *edi, sint32 *ebp)
{
    sint32 flags = *ebx & 0xFF;

    *ebx = map_set_land_ownership(
        flags,
        *eax & 0xFFFF,
        *ecx & 0xFFFF,
        *edi & 0xFFFF,
        *ebp & 0xFFFF,
        *edx & 0xFF
        );

    if (flags & GAME_COMMAND_FLAG_APPLY) {
        map_count_remaining_land_rights();
    }
}

static uint8 map_get_lowest_land_height(sint32 xMin, sint32 xMax, sint32 yMin, sint32 yMax)
{
    xMin = max(xMin, 32);
    yMin = max(yMin, 32);
    xMax = min(xMax, gMapSizeMaxXY);
    yMax = min(yMax, gMapSizeMaxXY);

    uint8 min_height = 0xFF;
    for (sint32 yi = yMin; yi <= yMax; yi += 32) {
        for (sint32 xi = xMin; xi <= xMax; xi += 32) {
            rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL && min_height > map_element->base_height) {
                min_height = map_element->base_height;
            }
        }
    }
    return min_height;
}

static uint8 map_get_highest_land_height(sint32 xMin, sint32 xMax, sint32 yMin, sint32 yMax)
{
    xMin = max(xMin, 32);
    yMin = max(yMin, 32);
    xMax = min(xMax, gMapSizeMaxXY);
    yMax = min(yMax, gMapSizeMaxXY);

    uint8 max_height = 0;
    for (sint32 yi = yMin; yi <= yMax; yi += 32) {
        for (sint32 xi = xMin; xi <= xMax; xi += 32) {
            rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL) {
                uint8 base_height = map_element->base_height;
                if (map_element->properties.surface.slope & 0xF)
                    base_height += 2;
                if (map_element->properties.surface.slope & 0x10)
                    base_height += 2;
                if (max_height < base_height)
                    max_height = base_height;
            }
        }
    }
    return max_height;
}

static money32 raise_land(sint32 flags, sint32 x, sint32 y, sint32 z, sint32 ax, sint32 ay, sint32 bx, sint32 by, sint32 selectionType)
{
    money32 cost = 0;

    if (selectionType < 0 || selectionType >= countof(map_element_raise_styles))
    {
        log_warning("Invalid selection type %d for raising land", selectionType);
        return MONEY32_UNDEFINED;
    }

    if ((flags & GAME_COMMAND_FLAG_APPLY) && gGameCommandNestLevel == 1) {
        audio_play_sound_at_location(SOUND_PLACE_ITEM, x, y, z);
    }

    uint8 min_height = map_get_lowest_land_height(ax, bx, ay, by);

    for (sint32 yi = ay; yi <= by; yi += 32) {
        for (sint32 xi = ax; xi <= bx; xi += 32) {
            rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL) {
                uint8 height = map_element->base_height;
                if (height <= min_height){
                    uint8 newStyle = map_element_raise_styles[selectionType][map_element->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK];
                    if (newStyle & 0x20) { // needs to be raised
                        height += 2;
                        newStyle &= ~0x20;
                    }
                    money32 tileCost = map_set_land_height(flags, xi, yi, height, newStyle, selectionType);
                    if (tileCost == MONEY32_UNDEFINED)
                        return MONEY32_UNDEFINED;

                    cost += tileCost;
                }
            }
        }
    }

    // Force ride construction to recheck area
    _currentTrackSelectionFlags |= TRACK_SELECTION_FLAG_RECHECK;

    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;
    gCommandPosition.x = x;
    gCommandPosition.y = y;
    gCommandPosition.z = z;
    return cost;
}

static money32 lower_land(sint32 flags, sint32 x, sint32 y, sint32 z, sint32 ax, sint32 ay, sint32 bx, sint32 by, sint32 selectionType)
{
    money32 cost = 0;

    if ((flags & GAME_COMMAND_FLAG_APPLY) && gGameCommandNestLevel == 1) {
        audio_play_sound_at_location(SOUND_PLACE_ITEM, x, y, z);
    }

    if (selectionType < 0 || selectionType >= 5)
    {
        log_warning("Improper selection type %d", selectionType);
        return MONEY32_UNDEFINED;
    }

    uint8 max_height = map_get_highest_land_height(ax, bx, ay, by);

    for (sint32 yi = ay; yi <= by; yi += 32) {
        for (sint32 xi = ax; xi <= bx; xi += 32) {
            rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL) {
                uint8 height = map_element->base_height;
                if (map_element->properties.surface.slope & 0xF)
                    height += 2;
                if (map_element->properties.surface.slope & 0x10)
                    height += 2;
                if (height >= max_height) {
                    height =  map_element->base_height;
                    uint8 newStyle = map_element_lower_styles[selectionType][map_element->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK];
                    if (newStyle & 0x20) { // needs to be lowered
                        height -= 2;
                        newStyle &= ~0x20;
                    }
                    money32 tileCost = map_set_land_height(flags, xi, yi, height, newStyle, selectionType);
                    if (tileCost == MONEY32_UNDEFINED)
                        return MONEY32_UNDEFINED;

                    cost += tileCost;
                }
            }
        }
    }

    // Force ride construction to recheck area
    _currentTrackSelectionFlags |= TRACK_SELECTION_FLAG_RECHECK;

    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;
    gCommandPosition.x = x;
    gCommandPosition.y = y;
    gCommandPosition.z = z;
    return cost;
}

sint32 map_get_water_height(const rct_map_element * mapElement)
{
    return mapElement->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK;
}

money32 raise_water(sint16 x0, sint16 y0, sint16 x1, sint16 y1, uint8 flags)
{
    money32 cost = 0;
    bool waterHeightChanged = false;

    uint8 max_height = 0xFF;

    x0 = max(x0, 32);
    y0 = max(y0, 32);
    x1 = min(x1, gMapSizeMaxXY);
    y1 = min(y1, gMapSizeMaxXY);

    for (sint32 yi = y0; yi <= y1; yi += 32) {
        for (sint32 xi = x0; xi <= x1; xi += 32) {
            rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL) {
                uint8 height = map_element->base_height;
                if (map_get_water_height(map_element) > 0)
                    height = map_get_water_height(map_element) * 2;
                if (max_height > height)
                    max_height = height;
            }
        }
    }

    for (sint32 yi = y0; yi <= y1; yi += 32) {
        for (sint32 xi = x0; xi <= x1; xi += 32) {
            rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL) {
                if (map_element->base_height <= max_height){
                    uint8 height = map_get_water_height(map_element);
                    if (height != 0) {
                        height *= 2;
                        if (height > max_height)
                            continue;
                        height += 2;
                    } else {
                        height = map_element->base_height + 2;
                    }

                    money32 tileCost = game_do_command(xi, flags, yi, (max_height << 8) + height, GAME_COMMAND_SET_WATER_HEIGHT, 0, 0);
                    if (tileCost == MONEY32_UNDEFINED)
                        return MONEY32_UNDEFINED;

                    cost += tileCost;
                    waterHeightChanged = true;
                }
            }
        }
    }

    if (flags & GAME_COMMAND_FLAG_APPLY) {
        sint32 x = ((x0 + x1) / 2) + 16;
        sint32 y = ((y0 + y1) / 2) + 16;
        sint32 z = map_element_height(x, y);
        sint16 water_height_z = z >> 16;
        sint16 base_height_z = z;
        z = water_height_z;
        if (z != 0)
            z = base_height_z;

        rct_xyz16 coord;
        coord.x = x;
        coord.y = y;
        coord.z = z;
        network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);

        gCommandPosition.x = x;
        gCommandPosition.y = y;
        gCommandPosition.z = z;
        if (waterHeightChanged) {
            audio_play_sound_at_location(SOUND_LAYING_OUT_WATER, x, y, z);
        }
    }

    // Force ride construction to recheck area
    _currentTrackSelectionFlags |= TRACK_SELECTION_FLAG_RECHECK;

    return cost;
}

money32 lower_water(sint16 x0, sint16 y0, sint16 x1, sint16 y1, uint8 flags)
{
    money32 cost = 0;
    bool waterHeightChanged = false;

    uint8 min_height = 0;

    x0 = max(x0, 32);
    y0 = max(y0, 32);
    x1 = min(x1, gMapSizeMaxXY);
    y1 = min(y1, gMapSizeMaxXY);

    for (sint32 yi = y0; yi <= y1; yi += 32){
        for (sint32 xi = x0; xi <= x1; xi += 32){
            rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL) {
                uint8 height = map_get_water_height(map_element);
                if (height != 0) {
                    height *= 2;
                    if (height > min_height)
                        min_height = height;
                }
            }
        }
    }

    for (sint32 yi = y0; yi <= y1; yi += 32) {
        for (sint32 xi = x0; xi <= x1; xi += 32) {
            rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
            if (map_element != NULL) {
                uint8 height = map_get_water_height(map_element);
                if (height != 0) {
                    height *= 2;
                    if (height < min_height)
                        continue;
                    height -= 2;
                    sint32 tileCost = game_do_command(xi, flags, yi, (min_height << 8) + height, GAME_COMMAND_SET_WATER_HEIGHT, 0, 0);
                    if (tileCost == MONEY32_UNDEFINED)
                        return MONEY32_UNDEFINED;
                    cost += tileCost;
                    waterHeightChanged = true;
                }
            }
        }
    }

    if (flags & GAME_COMMAND_FLAG_APPLY) {
        sint32 x = ((x0 + x1) / 2) + 16;
        sint32 y = ((y0 + y1) / 2) + 16;
        sint32 z = map_element_height(x, y);
        sint16 water_height_z = z >> 16;
        sint16 base_height_z = z;
        z = water_height_z;
        if (z == 0)
            z = base_height_z;

        rct_xyz16 coord;
        coord.x = x;
        coord.y = y;
        coord.z = z;
        network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);

        gCommandPosition.x = x;
        gCommandPosition.y = y;
        gCommandPosition.z = z;
        if (waterHeightChanged) {
            audio_play_sound_at_location(SOUND_LAYING_OUT_WATER, x, y, z);
        }
    }

    // Force ride construction to recheck area
    _currentTrackSelectionFlags |= TRACK_SELECTION_FLAG_RECHECK;

    return cost;
}

/**
 *
 *  rct2: 0x0068C542
 */
void game_command_raise_land(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    *ebx = raise_land(
        *ebx,
        *eax,
        *ecx,
        map_element_height(*eax, *ecx),
        (sint16)(*edx & 0xFFFF),
        (sint16)(*ebp & 0xFFFF),
        *edx >> 16,
        *ebp >> 16,
        *edi & 0xFFFF
    );
}

/**
 *
 *  rct2: 0x0068C6D1
 */
void game_command_lower_land(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    *ebx = lower_land(
        *ebx,
        *eax,
        *ecx,
        map_element_height(*eax, *ecx),
        (sint16)(*edx & 0xFFFF),
        (sint16)(*ebp & 0xFFFF),
        *edx >> 16,
        *ebp >> 16,
        *edi & 0xFFFF
    );
}

static money32 smooth_land_tile(sint32 direction, uint8 flags, sint32 x, sint32 y, rct_map_element * mapElement, bool raiseLand)
{
    sint32 targetBaseZ = mapElement->base_height;
    sint32 slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
    if (raiseLand) {
        slope = map_element_raise_styles[direction][slope];
        if (slope & 0x20) {
            targetBaseZ += 2;
            slope &= ~0x20;
        }
    }
    else {
        slope = map_element_lower_styles[direction][slope];
        if (slope & 0x20) {
            targetBaseZ -= 2;
            slope &= ~0x20;
        }
    }
    return game_do_command(x, flags, y, targetBaseZ | (slope << 8), GAME_COMMAND_SET_LAND_HEIGHT, 0, 0);
}

static money32 smooth_land_row_by_edge(sint32 flags, sint32 x, sint32 y, sint32 expectedLandHeight1, sint32 expectedLandHeight2, sint32 stepX, sint32 stepY, sint32 direction1, sint32 direction2, sint32 checkDirection1, sint32 checkDirection2, bool raiseLand)
{
    uint8 shouldContinue = 0xF;
    sint32 landChangePerTile = raiseLand ? -2 : 2;
    rct_map_element *mapElement, *nextMapElement;
    money32 totalCost = 0;
    money32 result;

    // check if we need to start at all
    if (!map_is_location_valid(x, y) || !map_is_location_valid(x + stepX, y + stepY)) {
        return 0;
    }
    mapElement = map_get_surface_element_at(x >> 5, y >> 5);
    nextMapElement = map_get_surface_element_at((x + stepX) >> 5, (y + stepY) >> 5);
    if (mapElement == NULL || nextMapElement == NULL) {
        return 0;
    }
    if (map_element_get_corner_height(mapElement, checkDirection1) != expectedLandHeight1 + (raiseLand ? -2 : 2)) {
        shouldContinue &= ~0x1;
    }
    if (map_element_get_corner_height(mapElement, checkDirection2) != expectedLandHeight2 + (raiseLand ? -2 : 2)) {
        shouldContinue &= ~0x2;
    }
    if (map_element_get_corner_height(mapElement, checkDirection1) != map_element_get_corner_height(nextMapElement, direction1)) {
        shouldContinue &= ~0x1;
    }
    if (map_element_get_corner_height(mapElement, checkDirection2) != map_element_get_corner_height(nextMapElement, direction2)) {
        shouldContinue &= ~0x2;
    }
    while ((shouldContinue & 0x3) != 0)
    {
        shouldContinue = ((shouldContinue << 2) | 0x3) & shouldContinue;
        x += stepX;
        y += stepY;
        // check if we need to continue after raising the current tile
        // this needs to be checked before the tile is changed
        if (!map_is_location_valid(x + stepX, y + stepY)) {
            shouldContinue &= ~0x3;
        }
        else
        {
            mapElement = nextMapElement;
            nextMapElement = map_get_surface_element_at((x + stepX) >> 5, (y + stepY) >> 5);
            if (nextMapElement == NULL) {
                shouldContinue &= ~0x3;
            }
            if (map_element_get_corner_height(mapElement, direction1) + landChangePerTile != map_element_get_corner_height(mapElement, checkDirection1)) {
                shouldContinue &= ~0x1;
            }
            if (map_element_get_corner_height(mapElement, direction2) + landChangePerTile != map_element_get_corner_height(mapElement, checkDirection2)) {
                shouldContinue &= ~0x2;
            }
            if ((shouldContinue & 0x1) && map_element_get_corner_height(mapElement, checkDirection1) != map_element_get_corner_height(nextMapElement, direction1)) {
                shouldContinue &= ~0x1;
            }
            if ((shouldContinue & 0x2) && map_element_get_corner_height(mapElement, checkDirection2) != map_element_get_corner_height(nextMapElement, direction2)) {
                shouldContinue &= ~0x2;
            }
        }
        expectedLandHeight1 += landChangePerTile;

        // change land of current tile
        sint32 targetBaseZ = mapElement->base_height;
        sint32 slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
        sint32 oldSlope = slope;
        if (raiseLand) {
            if (shouldContinue & 0x4) {
                slope = map_element_raise_styles[direction1][slope];
                if (slope & 0x20) {
                    targetBaseZ += 2;
                    slope &= ~0x20;
                }
            }
            if ((shouldContinue & 0x8) &&
                    map_get_corner_height(mapElement->base_height, oldSlope, direction2) ==
                    map_get_corner_height(targetBaseZ, slope, direction2))
            {
                slope = map_element_raise_styles[direction2][slope];
                if (slope & 0x20) {
                    targetBaseZ += 2;
                    slope &= ~0x20;
                }
            }
        }
        else
        {
            if (shouldContinue & 0x4) {
                slope = map_element_lower_styles[direction1][slope];
                if (slope & 0x20) {
                    targetBaseZ -= 2;
                    slope &= ~0x20;
                }
            }
            if ((shouldContinue & 0x8) &&
                    map_get_corner_height(mapElement->base_height, oldSlope, direction2) ==
                    map_get_corner_height(targetBaseZ, slope, direction2))
            {
                slope = map_element_lower_styles[direction2][slope];
                if (slope & 0x20) {
                    targetBaseZ -= 2;
                    slope &= ~0x20;
                }
            }
        }
        result = game_do_command(x, flags, y, targetBaseZ | (slope << 8), GAME_COMMAND_SET_LAND_HEIGHT, 0, 0);
        if (result != MONEY32_UNDEFINED) {
            totalCost += result;
        }
    }
    return totalCost;
}

static money32 smooth_land_row_by_corner(sint32 flags, sint32 x, sint32 y, sint32 expectedLandHeight, sint32 stepX, sint32 stepY, sint32 direction, sint32 checkDirection, bool raiseLand)
{
    bool shouldContinue = true;
    rct_map_element *mapElement, *nextMapElement;
    money32 totalCost = 0;
    money32 result;
    sint32 landChangePerTile;
    if (stepX == 0 || stepY == 0)
    {
        landChangePerTile = raiseLand ? -2 : 2;
    }
    else
    {
        landChangePerTile = raiseLand ? -4 : 4;
    }

    // check if we need to start at all
    if (!map_is_location_valid(x, y) || !map_is_location_valid(x + stepX, y + stepY)) {
        return 0;
    }
    mapElement = map_get_surface_element_at(x >> 5, y >> 5);
    nextMapElement = map_get_surface_element_at((x + stepX) >> 5, (y + stepY) >> 5);
    if (mapElement == NULL || nextMapElement == NULL) {
        return 0;
    }
    if (map_element_get_corner_height(mapElement, checkDirection) != expectedLandHeight + (raiseLand ? -2 : 2)) {
        return 0;
    }
    if (map_element_get_corner_height(mapElement, checkDirection) != map_element_get_corner_height(nextMapElement, direction)) {
        return 0;
    }
    while (shouldContinue)
    {
        x += stepX;
        y += stepY;
        // check if we need to continue after raising the current tile
        // this needs to be checked before the tile is changed
        if (!map_is_location_valid(x + stepX, y + stepY)) {
            shouldContinue = false;
        }
        else
        {
            mapElement = nextMapElement;
            nextMapElement = map_get_surface_element_at((x + stepX) >> 5, (y + stepY) >> 5);
            if (nextMapElement == NULL) {
                shouldContinue = false;
            }
            if (map_element_get_corner_height(mapElement, direction) + landChangePerTile != map_element_get_corner_height(mapElement, checkDirection)) {
                shouldContinue = false;
            }
            if (shouldContinue && map_element_get_corner_height(mapElement, checkDirection) != map_element_get_corner_height(nextMapElement, direction)) {
                shouldContinue = false;
            }
        }
        if (stepX*stepY != 0)
        {
            totalCost += smooth_land_row_by_corner(flags, x, y, expectedLandHeight + (landChangePerTile / 2), 0, stepY, direction, checkDirection ^ 3, raiseLand);
            totalCost += smooth_land_row_by_corner(flags, x, y, expectedLandHeight + (landChangePerTile / 2), stepX, 0, direction, checkDirection ^ 1, raiseLand);
        }
        expectedLandHeight += landChangePerTile;
        // change land of current tile
        result = smooth_land_tile(direction, flags, x, y, mapElement, raiseLand);
        if (result != MONEY32_UNDEFINED) {
            totalCost += result;
        }
    }
    return totalCost;
}

static money32 smooth_land(sint32 flags, sint32 centreX, sint32 centreY, sint32 mapLeft, sint32 mapTop, sint32 mapRight, sint32 mapBottom, sint32 command)
{
    // Cap bounds to map
    mapLeft = max(mapLeft, 32);
    mapTop = max(mapTop, 32);
    mapRight = clamp(0, mapRight, (MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32);
    mapBottom = clamp(0, mapBottom, (MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32);

    bool raiseLand = command < 0x7FFF;
    sint32 commandType = raiseLand ? GAME_COMMAND_RAISE_LAND : GAME_COMMAND_LOWER_LAND;
    sint32 centreZ = map_element_height(centreX, centreY);
    sint32 mapLeftRight = mapLeft | (mapRight << 16);
    sint32 mapTopBottom = mapTop | (mapBottom << 16);
    bool fullTile = ((command & 0x7FFF) == MAP_SELECT_TYPE_FULL);

    // Play sound (only once)
    if ((flags & GAME_COMMAND_FLAG_APPLY) && gGameCommandNestLevel == 1) {
        audio_play_sound_at_location(SOUND_PLACE_ITEM, centreX, centreY, centreZ);
    }

    money32 totalCost = 0;
    money32 result;

    rct_map_element *mapElement = map_get_surface_element_at(mapLeft >> 5, mapTop >> 5);
    if (mapElement == NULL)
    {
        log_warning("Invalid coordinates for land smoothing, x = %d, y = %d", mapLeft, mapTop);
        return MONEY32_UNDEFINED;
    }

    if (flags & GAME_COMMAND_FLAG_APPLY) {
        rct_xyz16 coord;
        coord.x = centreX + 16;
        coord.y = centreY + 16;
        coord.z = map_element_height(coord.x, coord.y);
        network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);
    }

    uint8 maxHeight = 0;
    uint8 minHeight = 0xFF;
    uint32 newBaseZ = 0;
    uint32 newSlope = 0;

    // Predict the land height for future use
    if (fullTile)
    {
        minHeight = map_get_lowest_land_height(mapLeft, mapRight, mapTop, mapBottom);
        maxHeight = map_get_highest_land_height(mapLeft, mapRight, mapTop, mapBottom);

        if (commandType == GAME_COMMAND_RAISE_LAND) {
            minHeight += 2;
            maxHeight += 2;
        }
        else {
            maxHeight -= 2;
            minHeight -= 2;
        }
    }
    else
    {
        // One corner tile selected
        newBaseZ = mapElement->base_height;
        newSlope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
        if (commandType == GAME_COMMAND_RAISE_LAND) {
            newSlope = map_element_raise_styles[command & 0xFF][newSlope];
            if (newSlope & 0x20) {
                newBaseZ += 2;
                newSlope &= ~0x20;
            }
        }
        else {
            newSlope = map_element_lower_styles[command & 0xFF][newSlope];
            if (newSlope & 0x20) {
                newBaseZ -= 2;
                newSlope &= ~0x20;
            }
        }
    }

    // Then do the smoothing
    if (fullTile) {
        // Smooth the corners
        sint32 z = clamp(minHeight, map_element_get_corner_height(mapElement, 2), maxHeight);
        totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, -32, -32, 0, 2, raiseLand);
        mapElement = map_get_surface_element_at(mapLeft >> 5, mapBottom >> 5);
        z = clamp(minHeight, map_element_get_corner_height(mapElement, 3), maxHeight);
        totalCost += smooth_land_row_by_corner(flags, mapLeft, mapBottom, z, -32, 32, 1, 3, raiseLand);
        mapElement = map_get_surface_element_at(mapRight >> 5, mapBottom >> 5);
        z = clamp(minHeight, map_element_get_corner_height(mapElement, 0), maxHeight);
        totalCost += smooth_land_row_by_corner(flags, mapRight, mapBottom, z, 32, 32, 2, 0, raiseLand);
        mapElement = map_get_surface_element_at(mapRight >> 5, mapTop >> 5);
        z = clamp(minHeight, map_element_get_corner_height(mapElement, 1), maxHeight);
        totalCost += smooth_land_row_by_corner(flags, mapRight, mapTop, z, 32, -32, 3, 1, raiseLand);

        // Smooth the edges
        sint32 x = mapLeft;
        sint32 y, z2;
        for (y = mapTop; y <= mapBottom; y += 32)
        {
            mapElement = map_get_surface_element_at(x >> 5, y >> 5);
            z = clamp(minHeight, map_element_get_corner_height(mapElement, 3), maxHeight);
            z2 = clamp(minHeight, map_element_get_corner_height(mapElement, 2), maxHeight);
            totalCost += smooth_land_row_by_edge(flags, x, y, z, z2, -32, 0, 0, 1, 3, 2, raiseLand);
        }
        x = mapRight;
        for (y = mapTop; y <= mapBottom; y += 32)
        {
            mapElement = map_get_surface_element_at(x >> 5, y >> 5);
            z = clamp(minHeight, map_element_get_corner_height(mapElement, 1), maxHeight);
            z2 = clamp(minHeight, map_element_get_corner_height(mapElement, 0), maxHeight);
            totalCost += smooth_land_row_by_edge(flags, x, y, z, z2, 32, 0, 2, 3, 1, 0, raiseLand);
        }
        y = mapTop;
        for (x = mapLeft; x <= mapRight; x += 32)
        {
            mapElement = map_get_surface_element_at(x >> 5, y >> 5);
            z = clamp(minHeight, map_element_get_corner_height(mapElement, 1), maxHeight);
            z2 = clamp(minHeight, map_element_get_corner_height(mapElement, 2), maxHeight);
            totalCost += smooth_land_row_by_edge(flags, x, y, z, z2, 0, -32, 0, 3, 1, 2, raiseLand);
        }
        y = mapBottom;
        for (x = mapLeft; x <= mapRight; x += 32)
        {
            mapElement = map_get_surface_element_at(x >> 5, y >> 5);
            z = clamp(minHeight, map_element_get_corner_height(mapElement, 0), maxHeight);
            z2 = clamp(minHeight, map_element_get_corner_height(mapElement, 3), maxHeight);
            totalCost += smooth_land_row_by_edge(flags, x, y, z, z2, 0, 32, 1, 2, 0, 3, raiseLand);
        }
    }
    else
    {
        // One corner tile selected
        // Smooth the corners
        sint32 z = map_get_corner_height(newBaseZ, newSlope, 2);
        totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, -32, -32, 0, 2, raiseLand);
        z = map_get_corner_height(newBaseZ, newSlope, 0);
        totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 32, 32, 2, 0, raiseLand);
        z = map_get_corner_height(newBaseZ, newSlope, 3);
        totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, -32, 32, 1, 3, raiseLand);
        z = map_get_corner_height(newBaseZ, newSlope, 1);
        totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 32, -32, 3, 1, raiseLand);

        // Smooth the edges
        switch (command & 0x7FFF) {
        case MAP_SELECT_TYPE_CORNER_0:
            z = map_get_corner_height(newBaseZ, newSlope, 0);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 32, 0, 3, 0, raiseLand);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, 32, 1, 0, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 3);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, -32, 0, 0, 3, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 1);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, -32, 0, 1, raiseLand);
            break;
        case MAP_SELECT_TYPE_CORNER_1:
            z = map_get_corner_height(newBaseZ, newSlope, 1);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 32, 0, 2, 1, raiseLand);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, -32, 0, 1, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 2);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, -32, 0, 1, 2, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 0);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, 32, 1, 0, raiseLand);
            break;
        case MAP_SELECT_TYPE_CORNER_2:
            z = map_get_corner_height(newBaseZ, newSlope, 2);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, -32, 0, 1, 2, raiseLand);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, -32, 3, 2, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 1);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 32, 0, 2, 1, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 3);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, 32, 2, 3, raiseLand);
            break;
        case MAP_SELECT_TYPE_CORNER_3:
            z = map_get_corner_height(newBaseZ, newSlope, 3);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, -32, 0, 0, 3, raiseLand);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, 32, 2, 3, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 0);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 32, 0, 3, 0, raiseLand);
            z = map_get_corner_height(newBaseZ, newSlope, 2);
            totalCost += smooth_land_row_by_corner(flags, mapLeft, mapTop, z, 0, -32, 3, 2, raiseLand);
            break;
        }
    }

    // Finally raise / lower the centre tile
    result = game_do_command(centreX, flags, centreY, mapLeftRight, commandType, command & 0x7FFF, mapTopBottom);
    if (result != MONEY32_UNDEFINED) {
        totalCost += result;
    } else {
        return MONEY32_UNDEFINED;
    }

    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;
    gCommandPosition.x = centreX;
    gCommandPosition.y = centreY;
    gCommandPosition.z = centreZ;
    return totalCost;
}

/**
 *
 *  rct2: 0x0068BC01
 */
void game_command_smooth_land(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    sint32 flags = *ebx & 0xFF;
    sint32 centreX = *eax & 0xFFFF;
    sint32 centreY = *ecx & 0xFFFF;
    sint32 mapLeft = (sint16)(*edx & 0xFFFF);
    sint32 mapTop = (sint16)(*ebp & 0xFFFF);
    sint32 mapRight = (sint16)(*edx >> 16);
    sint32 mapBottom = (sint16)(*ebp >> 16);
    sint32 command = *edi;
    *ebx = smooth_land(flags, centreX, centreY, mapLeft, mapTop, mapRight, mapBottom, command);
}

/**
 *
 *  rct2: 0x006E66A0
 */
void game_command_raise_water(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    *ebx = raise_water(
        (sint16)(*eax & 0xFFFF),
        (sint16)(*ecx & 0xFFFF),
        (sint16)(*edi & 0xFFFF),
        (sint16)(*ebp & 0xFFFF),
        (uint8)*ebx
    );
}

/**
 *
 *  rct2: 0x006E6878
 */
void game_command_lower_water(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    *ebx = lower_water(
        (sint16)(*eax & 0xFFFF),
        (sint16)(*ecx & 0xFFFF),
        (sint16)(*edi & 0xFFFF),
        (sint16)(*ebp & 0xFFFF),
        (uint8)*ebx
    );
}

/**
 *
 *  rct2: 0x006E650F
 */
void game_command_set_water_height(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    sint32 x = *eax;
    sint32 y = *ecx;
    uint8 base_height = *edx;
    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;
    gCommandPosition.x = x + 16;
    gCommandPosition.y = y + 16;
    gCommandPosition.z = base_height * 8;
    if(game_is_paused() && !gCheatsBuildInPauseMode){
        gGameCommandErrorText = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
        *ebx = MONEY32_UNDEFINED;
        return;
    }
    if(!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && gParkFlags & PARK_FLAGS_FORBID_LANDSCAPE_CHANGES){
        gGameCommandErrorText = STR_FORBIDDEN_BY_THE_LOCAL_AUTHORITY;
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    if(base_height < 2){
        gGameCommandErrorText = STR_TOO_LOW;
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    if(base_height >= 58){
        gGameCommandErrorText = STR_TOO_HIGH;
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    if(x >= gMapSizeUnits || y >= gMapSizeUnits){
        gGameCommandErrorText = STR_OFF_EDGE_OF_MAP;
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    if(!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && !map_is_location_in_park(x, y)){
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    if(*ebx & GAME_COMMAND_FLAG_APPLY){
        sint32 element_height = map_element_height(x, y);
        footpath_remove_litter(x, y, element_height);
        if(!gCheatsDisableClearanceChecks)
            wall_remove_at_z(x, y, element_height);
    }

    rct_map_element* map_element = map_get_surface_element_at(x / 32, y / 32);
    sint32 zHigh = map_element->base_height;
    sint32 zLow = base_height;
    if (map_get_water_height(map_element) > 0)
    {
        zHigh = map_get_water_height(map_element) * 2;
    }
    if(zLow > zHigh){
        sint32 temp = zHigh;
        zHigh = zLow;
        zLow = temp;
    }

    if (gCheatsDisableClearanceChecks || map_can_construct_at(x, y, zLow, zHigh, 0xFF)) {
        if(map_element->type & 0x40){
            gGameCommandErrorText = 0;
            *ebx = MONEY32_UNDEFINED;
            return;
        }
        if(*ebx & GAME_COMMAND_FLAG_APPLY){
            sint32 new_terrain = map_element->properties.surface.terrain & 0xE0;
            if(base_height > map_element->base_height){
                new_terrain |= (base_height / 2);
            }
            map_element->properties.surface.terrain = new_terrain;
            map_invalidate_tile_full(x, y);
        }
        *ebx = 250;
        if(gParkFlags & PARK_FLAGS_NO_MONEY){
            *ebx = 0;
        }
    }else{
        *ebx = MONEY32_UNDEFINED;
    }
}

bool map_is_location_at_edge(sint32 x, sint32 y)
{
    return x < 32 || y < 32 || x >= ((MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32) || y >= ((MAXIMUM_MAP_SIZE_TECHNICAL - 1) * 32);
}

/**
 *
 *  rct2: 0x006B893C
 */
void game_command_place_large_scenery(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_LANDSCAPING;
    sint32 x = (sint16)*eax;
    sint32 y = (sint16)*ecx;
    sint32 z = (sint16)*ebp;
    uint8 colour1 = *edx;
    uint8 colour2 = *edx >> 8;
    uint8 flags = *ebx;
    uint8 rotation = *ebx >> 8;
    uint8 entry_index = *edi;
    sint32 base_height = map_element_height(x, y);
    gCommandPosition.x = x + 16;
    gCommandPosition.y = y + 16;
    gCommandPosition.z = base_height;
    gSceneryGroundFlags = 0;
    uint8 banner_id = 0xFF;
    money32 supportsCost = 0;

    if (game_is_paused() && !gCheatsBuildInPauseMode) {
        gGameCommandErrorText = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    if (entry_index >= 128)
    {
        log_warning("Invalid game command for scenery placement, entry_index = %u", entry_index);
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    rct_scenery_entry *scenery_entry = get_large_scenery_entry(entry_index);
    if (scenery_entry == (rct_scenery_entry *)-1)
    {
        log_warning("Invalid game command for scenery placement, entry_index = %u", entry_index);
        *ebx = MONEY32_UNDEFINED;
        return;
    }
    if(scenery_entry->large_scenery.scrolling_mode != 0xFF){
        banner_id = create_new_banner(flags);

        if (banner_id == MAX_BANNERS) {
            *ebx = MONEY32_UNDEFINED;
            return;
        }

        if (flags & GAME_COMMAND_FLAG_APPLY) {
            rct_banner* banner = &gBanners[banner_id];
            banner->flags |= BANNER_FLAG_IS_LARGE_SCENERY;
            banner->type = 0;
            banner->x = x / 32;
            banner->y = y / 32;

            sint32 rideIndex = banner_get_closest_ride_index(x, y, z);
            if (rideIndex != -1) {
                banner->colour = rideIndex;
                banner->flags |= BANNER_FLAG_LINKED_TO_RIDE;
            }
        }
    }

    uint32 num_elements = 0;
    sint16 maxHeight = 0xFFFF;
    for (rct_large_scenery_tile* tile = scenery_entry->large_scenery.tiles;
        tile->x_offset != -1;
        tile++) {
        num_elements++;

        rct_xy16 curTile = {
            .x = tile->x_offset,
            .y = tile->y_offset
        };

        rotate_map_coordinates(&curTile.x, &curTile.y, rotation);

        curTile.x += x;
        curTile.y += y;

        if(curTile.x >= 0x1FFF || curTile.y >= 0x1FFF || curTile.x < 0 || curTile.y < 0){
            continue;
        }

        rct_map_element* map_element = map_get_surface_element_at(curTile.x / 32, curTile.y / 32);
        if(map_element != NULL){
            sint32 height = map_element->base_height * 8;

            if(map_element->properties.scenerymultiple.type & 0xF){
                height += 16;
                if(map_element->properties.scenerymultiple.type & 0x10){
                    height += 16;
                }
            }

            if(height > maxHeight){
                maxHeight = height;
            }
        }
    }

    if(z != 0){
        maxHeight = z;
    }

    if (!map_check_free_elements_and_reorganise(num_elements)) {
        *ebx = MONEY32_UNDEFINED;
        return;
    }

    gCommandPosition.z = maxHeight;
    uint8 tile_num = 0;
    for (rct_large_scenery_tile* tile = scenery_entry->large_scenery.tiles;
    tile->x_offset != -1;
        tile++, tile_num++) {

        rct_xy16 curTile = {
            .x = tile->x_offset,
            .y = tile->y_offset
        };

        rotate_map_coordinates(&curTile.x, &curTile.y, rotation);

        curTile.x += x;
        curTile.y += y;

        sint32 zLow = (tile->z_offset + maxHeight) / 8;
        sint32 zHigh = (tile->z_clearance / 8) + zLow;

        sint32 bx = tile->var_7 >> 12;
        bx <<= rotation;
        uint8 bl = bx;
        uint8 bh = bl >> 4;
        bl &= 0xF;
        bl |= bh;
        uint8 F43887 = bl;

        if (!gCheatsDisableClearanceChecks && !map_can_construct_with_clear_at(curTile.x, curTile.y, zLow, zHigh, &map_place_scenery_clear_func, bl, flags, &supportsCost)) {
            *ebx = MONEY32_UNDEFINED;
            return;
        }

        if ((gMapGroundFlags & ELEMENT_IS_UNDERWATER) || (gMapGroundFlags & ELEMENT_IS_UNDERGROUND)) {
            *ebx = MONEY32_UNDEFINED;
            return;
        }

        sint32 b = gMapGroundFlags & (ELEMENT_IS_ABOVE_GROUND | ELEMENT_IS_UNDERGROUND);
        if (!gCheatsDisableClearanceChecks) {
            if (gSceneryGroundFlags && !(gSceneryGroundFlags & b)) {
                gGameCommandErrorText = STR_CANT_BUILD_PARTLY_ABOVE_AND_PARTLY_BELOW_GROUND;
                *ebx = MONEY32_UNDEFINED;
                return;
            }
        }
        gSceneryGroundFlags = b;

        if (curTile.x >= gMapSizeUnits || curTile.y >= gMapSizeUnits) {
            gGameCommandErrorText = STR_OFF_EDGE_OF_MAP;
            *ebx = MONEY32_UNDEFINED;
            return;
        }

        if (!(gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) &&
            !map_is_location_owned(curTile.x, curTile.y, zLow * 8) &&
            !gCheatsSandboxMode) {
            *ebx = MONEY32_UNDEFINED;
            return;
        }

        if (flags & GAME_COMMAND_FLAG_APPLY) {
            if (!(flags & GAME_COMMAND_FLAG_GHOST)) {
                footpath_remove_litter(curTile.x, curTile.y, zLow * 8);
                if (!gCheatsDisableClearanceChecks) {
                    wall_remove_at(curTile.x, curTile.y, zLow * 8, zHigh * 8);
                }
            }
            if (gGameCommandNestLevel == 1 && !(*ebx & GAME_COMMAND_FLAG_GHOST)) {
                rct_xyz16 coord;
                coord.x = x + 16;
                coord.y = y + 16;
                coord.z = map_element_height(coord.x, coord.y);
                network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);
            }

            rct_map_element *new_map_element = map_element_insert(curTile.x / 32, curTile.y / 32, zLow, F43887);
            assert(new_map_element != NULL);
            map_animation_create(MAP_ANIMATION_TYPE_LARGE_SCENERY, curTile.x, curTile.y, zLow);

            new_map_element->clearance_height = zHigh;
            new_map_element->type = MAP_ELEMENT_TYPE_SCENERY_MULTIPLE | rotation;

            new_map_element->properties.scenerymultiple.type =
                (tile_num << 10) |
                entry_index;

            new_map_element->properties.scenerymultiple.colour[0] = colour1;
            new_map_element->properties.scenerymultiple.colour[1] = colour2;

            if (banner_id != 0xFF) {
                new_map_element->type |= banner_id & 0xC0;
                new_map_element->properties.scenerymultiple.colour[0] |= (banner_id & 0x38) << 2;
                new_map_element->properties.scenerymultiple.colour[1] |= (banner_id & 7) << 5;
            }

            if (flags & GAME_COMMAND_FLAG_GHOST) {
                new_map_element->flags |= MAP_ELEMENT_FLAG_GHOST;
            }

            if (tile_num == 0) {
                gSceneryMapElement = new_map_element;
            }
            map_invalidate_tile_full(curTile.x, curTile.y);
        }
    }

    // Force ride construction to recheck area
    _currentTrackSelectionFlags |= TRACK_SELECTION_FLAG_RECHECK;

    *ebx = (scenery_entry->large_scenery.price * 10) + supportsCost;
    if(gParkFlags & PARK_FLAGS_NO_MONEY){
        *ebx = 0;
    }
}

sint32 map_element_get_station(const rct_map_element * mapElement)
{
    return (mapElement->properties.track.sequence & MAP_ELEM_TRACK_SEQUENCE_STATION_INDEX_MASK) >> 4;
}

void map_element_set_station(rct_map_element * mapElement, uint32 stationIndex)
{
    mapElement->properties.track.sequence &= ~MAP_ELEM_TRACK_SEQUENCE_STATION_INDEX_MASK;
    mapElement->properties.track.sequence |= (stationIndex << 4);
}

sint32 map_element_get_track_sequence(const rct_map_element * mapElement)
{
    return mapElement->properties.track.sequence & MAP_ELEM_TRACK_SEQUENCE_SEQUENCE_MASK;
}

void map_element_set_track_sequence(rct_map_element * mapElement, sint32 trackSequence)
{
    mapElement->properties.track.sequence &= ~MAP_ELEM_TRACK_SEQUENCE_SEQUENCE_MASK;
    mapElement->properties.track.sequence |= (trackSequence & MAP_ELEM_TRACK_SEQUENCE_SEQUENCE_MASK);
}

bool map_element_get_green_light(const rct_map_element * mapElement)
{
    return (mapElement->properties.track.sequence & MAP_ELEM_TRACK_SEQUENCE_GREEN_LIGHT) != 0;
}

void map_element_set_green_light(rct_map_element * mapElement, bool greenLight)
{
    mapElement->properties.track.sequence &= ~MAP_ELEM_TRACK_SEQUENCE_GREEN_LIGHT;
    if (greenLight)
    {
        mapElement->properties.track.sequence |= MAP_ELEM_TRACK_SEQUENCE_GREEN_LIGHT;
    }
}

sint32 map_element_get_brake_booster_speed(const rct_map_element *mapElement)
{
    return (mapElement->properties.track.sequence >> 4) << 1;
}

void map_element_set_brake_booster_speed(rct_map_element *mapElement, sint32 speed)
{
    mapElement->properties.track.sequence = map_element_get_track_sequence(mapElement) | ((speed >> 1) << 4);
}

bool map_element_is_taking_photo(const rct_map_element * mapElement)
{
    return (mapElement->properties.track.sequence & MAP_ELEM_TRACK_SEQUENCE_TAKING_PHOTO_MASK) != 0;
}

void map_element_set_onride_photo_timeout(rct_map_element * mapElement)
{
    mapElement->properties.track.sequence &= MAP_ELEM_TRACK_SEQUENCE_SEQUENCE_MASK;
    mapElement->properties.track.sequence |= (3 << 4);
}

void map_element_decrement_onride_photo_timout(rct_map_element * mapElement)
{
    // We should only touch the upper 4 bits, avoid underflow into the lower 4.
    if (mapElement->properties.track.sequence & MAP_ELEM_TRACK_SEQUENCE_TAKING_PHOTO_MASK)
    {
        mapElement->properties.track.sequence -= (1 << 4);
    }
}

/**
 *
 *  rct2: 0x0068B280
 */
void map_element_remove(rct_map_element *mapElement)
{
    // Replace Nth element by (N+1)th element.
    // This loop will make mapElement point to the old last element position,
    // after copy it to it's new position
    if (!map_element_is_last_for_tile(mapElement)){
        do{
            *mapElement = *(mapElement + 1);
        } while (!map_element_is_last_for_tile(++mapElement));
    }

    // Mark the latest element with the last element flag.
    (mapElement - 1)->flags |= MAP_ELEMENT_FLAG_LAST_TILE;
    mapElement->base_height = 0xFF;

    if ((mapElement + 1) == gNextFreeMapElement){
        gNextFreeMapElement--;
    }
}

/**
 *
 *  rct2: 0x00675A8E
 */
void map_remove_all_rides()
{
    map_element_iterator it;

    map_element_iterator_begin(&it);
    do {
        switch (map_element_get_type(it.element)) {
        case MAP_ELEMENT_TYPE_PATH:
            if (it.element->type & 1) {
                it.element->properties.path.type &= ~8;
                it.element->properties.path.addition_status = 255;
            }
            break;
        case MAP_ELEMENT_TYPE_ENTRANCE:
            if (it.element->properties.entrance.type == ENTRANCE_TYPE_PARK_ENTRANCE)
                break;

            // fall-through
        case MAP_ELEMENT_TYPE_TRACK:
            footpath_queue_chain_reset();
            footpath_remove_edges_at(it.x * 32, it.y * 32, it.element);
            map_element_remove(it.element);
            map_element_iterator_restart_for_tile(&it);
            break;
        }
    } while (map_element_iterator_next(&it));
}

/**
 *
 *  rct2: 0x0068AB1B
 */
void map_invalidate_map_selection_tiles()
{
    rct_xy16 *position;

    if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE_CONSTRUCT))
        return;

    for (position = gMapSelectionTiles; position->x != -1; position++)
        map_invalidate_tile_full(position->x, position->y);
}

static void map_get_bounding_box(sint32 ax, sint32 ay, sint32 bx, sint32 by, sint32 *left, sint32 *top, sint32 *right, sint32 *bottom)
{
    sint32 x, y;
    x = ax;
    y = ay;
    uint32 rotation = get_current_rotation();
    translate_3d_to_2d(rotation, &x, &y);
    *left = x;
    *right = x;
    *top = y;
    *bottom = y;
    x = bx;
    y = ay;
    translate_3d_to_2d(rotation, &x, &y);
    if (x < *left) *left = x;
    if (x > *right) *right = x;
    if (y > *bottom) *bottom = y;
    if (y < *top) *top = y;
    x = bx;
    y = by;
    translate_3d_to_2d(rotation, &x, &y);
    if (x < *left) *left = x;
    if (x > *right) *right = x;
    if (y > *bottom) *bottom = y;
    if (y < *top) *top = y;
    x = ax;
    y = by;
    translate_3d_to_2d(rotation, &x, &y);
    if (x < *left) *left = x;
    if (x > *right) *right = x;
    if (y > *bottom) *bottom = y;
    if (y < *top) *top = y;
}

/**
 *
 *  rct2: 0x0068AAE1
 */
void map_invalidate_selection_rect()
{
    sint32 x0, y0, x1, y1, left, right, top, bottom;

    if (!(gMapSelectFlags & MAP_SELECT_FLAG_ENABLE))
        return;

    x0 = gMapSelectPositionA.x + 16;
    y0 = gMapSelectPositionA.y + 16;
    x1 = gMapSelectPositionB.x + 16;
    y1 = gMapSelectPositionB.y + 16;
    map_get_bounding_box(x0, y0, x1, y1, &left, &top, &right, &bottom);
    left -= 32;
    right += 32;
    bottom += 32;
    top -= 32 + 2080;

    for (sint32 i = 0; i < MAX_VIEWPORT_COUNT; i++) {
        rct_viewport *viewport = &g_viewport_list[i];
        if (viewport->width != 0) {
            viewport_invalidate(viewport, left, top, right, bottom);
        }
    }
}

/**
 *
 *  rct2: 0x0068B111
 */
void map_reorganise_elements()
{
    context_setcurrentcursor(CURSOR_ZZZ);

    rct_map_element* new_map_elements = malloc(3 * (MAXIMUM_MAP_SIZE_TECHNICAL * MAXIMUM_MAP_SIZE_TECHNICAL) * sizeof(rct_map_element));
    rct_map_element* new_elements_pointer = new_map_elements;

    if (new_map_elements == NULL) {
        log_fatal("Unable to allocate memory for map elements.");
        return;
    }

    uint32 num_elements;

    for (sint32 y = 0; y < MAXIMUM_MAP_SIZE_TECHNICAL; y++) {
        for (sint32 x = 0; x < MAXIMUM_MAP_SIZE_TECHNICAL; x++) {
            rct_map_element *startElement = map_get_first_element_at(x, y);
            rct_map_element *endElement = startElement;
            while (!map_element_is_last_for_tile(endElement++));

            num_elements = (uint32)(endElement - startElement);
            memcpy(new_elements_pointer, startElement, num_elements * sizeof(rct_map_element));
            new_elements_pointer += num_elements;
        }
    }

    num_elements = (uint32)(new_elements_pointer - new_map_elements);
    memcpy(gMapElements, new_map_elements, num_elements * sizeof(rct_map_element));
    memset(gMapElements + num_elements, 0, (3 * (MAXIMUM_MAP_SIZE_TECHNICAL * MAXIMUM_MAP_SIZE_TECHNICAL) - num_elements) * sizeof(rct_map_element));

    free(new_map_elements);

    map_update_tile_pointers();
}

/**
 *
 *  rct2: 0x0068B044
 *  Returns true on space available for more elements
 *  Reorganises the map elements to check for space
 */
bool map_check_free_elements_and_reorganise(sint32 num_elements)
{
    if ((gNextFreeMapElement + num_elements) <= gMapElements + MAX_MAP_ELEMENTS)
        return true;

    for (sint32 i = 1000; i != 0; --i)
        sub_68B089();

    if ((gNextFreeMapElement + num_elements) <= gMapElements + MAX_MAP_ELEMENTS)
        return true;

    map_reorganise_elements();

    if ((gNextFreeMapElement + num_elements) <= gMapElements + MAX_MAP_ELEMENTS)
        return true;
    else{
        gGameCommandErrorText = STR_ERR_LANDSCAPE_DATA_AREA_FULL;
        return false;
    }
}

/**
 *
 *  rct2: 0x0068B1F6
 */
rct_map_element *map_element_insert(sint32 x, sint32 y, sint32 z, sint32 flags)
{
    rct_map_element *originalMapElement, *newMapElement, *insertedElement;

    if (!map_check_free_elements_and_reorganise(1)) {
        log_error("Cannot insert new element");
        return NULL;
    }

    newMapElement = gNextFreeMapElement;
    originalMapElement = gMapElementTilePointers[y * MAXIMUM_MAP_SIZE_TECHNICAL + x];

    // Set tile index pointer to point to new element block
    gMapElementTilePointers[y * MAXIMUM_MAP_SIZE_TECHNICAL + x] = newMapElement;

    // Copy all elements that are below the insert height
    while (z >= originalMapElement->base_height) {
        // Copy over map element
        *newMapElement = *originalMapElement;
        originalMapElement->base_height = 255;
        originalMapElement++;
        newMapElement++;

        if ((newMapElement - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE) {
            // No more elements above the insert element
            (newMapElement - 1)->flags &= ~MAP_ELEMENT_FLAG_LAST_TILE;
            flags |= MAP_ELEMENT_FLAG_LAST_TILE;
            break;
        }
    }

    // Insert new map element
    insertedElement = newMapElement;
    newMapElement->base_height = z;
    newMapElement->flags = flags;
    newMapElement->clearance_height = z;
    memset(&newMapElement->properties, 0, sizeof(newMapElement->properties));
    newMapElement++;

    // Insert rest of map elements above insert height
    if (!(flags & MAP_ELEMENT_FLAG_LAST_TILE)) {
        do {
            // Copy over map element
            *newMapElement = *originalMapElement;
            originalMapElement->base_height = 255;
            originalMapElement++;
            newMapElement++;
        } while (!((newMapElement - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE));
    }

    gNextFreeMapElement = newMapElement;
    return insertedElement;
}

/**
 * This function will validate element address. It will only check if element lies within
 * the user-accessible part of map elements, there is some scratch space behind that is not
 * considered valid here.
 */
bool map_element_check_address(const rct_map_element * const element)
{
    if (element >= gMapElements
        && element < gMapElements + MAX_MAP_ELEMENTS
        // condition below checks alignment
        && gMapElements + (((uintptr_t)element - (uintptr_t)gMapElements) / sizeof(rct_map_element)) == element)
    {
        return true;
    }
    return false;
}

/**
 *
 *  rct2: 0x0068BB18
 */
void map_obstruction_set_error_text(rct_map_element *mapElement)
{
    rct_string_id errorStringId;
    rct_ride *ride;
    rct_scenery_entry *sceneryEntry;

    errorStringId = STR_OBJECT_IN_THE_WAY;
    switch (map_element_get_type(mapElement)) {
    case MAP_ELEMENT_TYPE_SURFACE:
        errorStringId = STR_RAISE_OR_LOWER_LAND_FIRST;
        break;
    case MAP_ELEMENT_TYPE_PATH:
        errorStringId = STR_FOOTPATH_IN_THE_WAY;
        break;
    case MAP_ELEMENT_TYPE_TRACK:
        ride = get_ride(mapElement->properties.track.ride_index);
        errorStringId = STR_X_IN_THE_WAY;
        set_format_arg(0, rct_string_id, ride->name);
        set_format_arg(2, uint32, ride->name_arguments);
        break;
    case MAP_ELEMENT_TYPE_SCENERY:
        sceneryEntry = get_small_scenery_entry(mapElement->properties.scenery.type);
        errorStringId = STR_X_IN_THE_WAY;
        set_format_arg(0, rct_string_id, sceneryEntry->name);
        break;
    case MAP_ELEMENT_TYPE_ENTRANCE:
        switch (mapElement->properties.entrance.type) {
        case ENTRANCE_TYPE_RIDE_ENTRANCE:
            errorStringId = STR_RIDE_ENTRANCE_IN_THE_WAY;
            break;
        case ENTRANCE_TYPE_RIDE_EXIT:
            errorStringId = STR_RIDE_EXIT_IN_THE_WAY;
            break;
        case ENTRANCE_TYPE_PARK_ENTRANCE:
            errorStringId = STR_PARK_ENTRANCE_IN_THE_WAY;
            break;
        }
        break;
    case MAP_ELEMENT_TYPE_WALL:
        sceneryEntry = get_wall_entry(mapElement->properties.scenery.type);
        errorStringId = STR_X_IN_THE_WAY;
        set_format_arg(0, rct_string_id, sceneryEntry->name);
        break;
    case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
        sceneryEntry = get_large_scenery_entry(mapElement->properties.scenery.type);
        errorStringId = STR_X_IN_THE_WAY;
        set_format_arg(0, rct_string_id, sceneryEntry->name);
        break;
    }

    gGameCommandErrorText = errorStringId;
}

/**
 *
 *  rct2: 0x0068B932
 *  ax = x
 *  cx = y
 *  dl = zLow
 *  dh = zHigh
 *  ebp = clearFunc
 *  bl = bl
 */
sint32 map_can_construct_with_clear_at(sint32 x, sint32 y, sint32 zLow, sint32 zHigh, CLEAR_FUNC *clearFunc, uint8 bl, uint8 flags, money32 *price)
{
    gMapGroundFlags = ELEMENT_IS_ABOVE_GROUND;
    if (x >= gMapSizeUnits || y >= gMapSizeUnits || x < 32 || y < 32) {
        gGameCommandErrorText = STR_OFF_EDGE_OF_MAP;
        return false;
    }
    rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
    do {
        if (map_element_get_type(map_element) != MAP_ELEMENT_TYPE_SURFACE) {
            if (zLow < map_element->clearance_height && zHigh > map_element->base_height && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST)) {
                if (map_element->flags & (bl & 0x0F)) {
                    goto loc_68BABC;
                }
            }
            continue;
        }
        sint32 water_height = map_get_water_height(map_element) * 2;
        if (water_height && water_height > zLow && map_element->base_height < zHigh) {
            gMapGroundFlags |= ELEMENT_IS_UNDERWATER;
            if (water_height < zHigh) {
                goto loc_68BAE6;
            }
        }
        loc_68B9B7:
        if (gParkFlags & PARK_FLAGS_FORBID_HIGH_CONSTRUCTION) {
            sint32 al = zHigh - map_element->base_height;
            if (al >= 0) {
                if (al > 18) {
                    gGameCommandErrorText = STR_LOCAL_AUTHORITY_WONT_ALLOW_CONSTRUCTION_ABOVE_TREE_HEIGHT;
                    return false;
                }
            }
        }
        if ((bl & 0xF0) != 0xF0) {
            if (map_element->base_height >= zHigh) {
                // loc_68BA81
                gMapGroundFlags |= ELEMENT_IS_UNDERGROUND;
                gMapGroundFlags &= ~ELEMENT_IS_ABOVE_GROUND;
            } else {
                sint32 al = map_element->base_height;
                sint32 ah = al;
                sint32 cl = al;
                sint32 ch = al;
                uint8 slope = map_element->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
                if (slope & 1) {
                    al += 2;
                    if (slope == 0x1B)
                        al += 2;
                }
                if (slope & 2) {
                    ah += 2;
                    if (slope == 0x17)
                        ah += 2;
                }
                if (slope & 4) {
                    cl += 2;
                    if (slope == 0x1E)
                        cl += 2;
                }
                if (slope & 8) {
                    ch += 2;
                    if (slope == 0x1D)
                        ch += 2;
                }
                sint32 bh = zLow + 4;
                if ((!(bl & 1) || ((bl & 0x10 || zLow >= al) && bh >= al)) &&
                    (!(bl & 2) || ((bl & 0x20 || zLow >= ah) && bh >= ah)) &&
                    (!(bl & 4) || ((bl & 0x40 || zLow >= cl) && bh >= cl)) &&
                    (!(bl & 8) || ((bl & 0x80 || zLow >= ch) && bh >= ch))) {
                    continue;
                }
                loc_68BABC:
                if (clearFunc != NULL) {
                    if (!clearFunc(&map_element, x, y, flags, price)) {
                        continue;
                    }
                }
                if (map_element != (rct_map_element*)0xFFFFFFF) {
                    map_obstruction_set_error_text(map_element);
                }
                return false;
                loc_68BAE6:
                if (clearFunc != NULL) {
                    if (!clearFunc(&map_element, x, y, flags, price)) {
                        goto loc_68B9B7;
                    }
                }
                if (map_element != (rct_map_element*)0xFFFFFFF) {
                    gGameCommandErrorText = STR_CANNOT_BUILD_PARTLY_ABOVE_AND_PARTLY_BELOW_WATER;
                }
                return false;
            }
        }
    } while (!map_element_is_last_for_tile(map_element++));
    return true;
}

/**
 *
 *  rct2: 0x0068B93A
 */
sint32 map_can_construct_at(sint32 x, sint32 y, sint32 zLow, sint32 zHigh, uint8 bl)
{
    return map_can_construct_with_clear_at(x, y, zLow, zHigh, NULL, bl, 0, NULL);
}

/**
 * Updates grass length, scenery age and jumping fountains.
 *
 *  rct2: 0x006646E1
 */
void map_update_tiles()
{
    sint32 ignoreScreenFlags = SCREEN_FLAGS_SCENARIO_EDITOR | SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER;
    if (gScreenFlags & ignoreScreenFlags)
        return;

    // Update 43 more tiles
    for (sint32 j = 0; j < 43; j++) {
        sint32 x = 0;
        sint32 y = 0;

        uint16 interleaved_xy = gGrassSceneryTileLoopPosition;
        for (sint32 i = 0; i < 8; i++) {
            x = (x << 1) | (interleaved_xy & 1);
            interleaved_xy >>= 1;
            y = (y << 1) | (interleaved_xy & 1);
            interleaved_xy >>= 1;
        }

        rct_map_element *mapElement = map_get_surface_element_at(x, y);
        if (mapElement != NULL) {
            map_update_grass_length(x * 32, y * 32, mapElement);
            scenery_update_tile(x * 32, y * 32);
        }

        gGrassSceneryTileLoopPosition++;
        gGrassSceneryTileLoopPosition &= 0xFFFF;
    }
}

/**
 *
 *  rct2: 0x006647A1
 */
static void map_update_grass_length(sint32 x, sint32 y, rct_map_element *mapElement)
{
    // Check if tile is grass
    if ((mapElement->properties.surface.terrain & 0xE0) && !(mapElement->type & 3))
        return;

    sint32 grassLength = mapElement->properties.surface.grass_length & 7;

    // Check if grass is underwater or outside park
    sint32 waterHeight = map_get_water_height(mapElement) * 2;
    if (waterHeight > mapElement->base_height || !map_is_location_in_park(x, y)) {
        if (grassLength != GRASS_LENGTH_CLEAR_0)
            map_set_grass_length(x, y, mapElement, GRASS_LENGTH_CLEAR_0);

        return;
    }

    // Grass can't grow any further than CLUMPS_2 but this code also cuts grass
    // if there is an object placed on top of it.

    sint32 z0 = mapElement->base_height;
    sint32 z1 = mapElement->base_height + 2;
    if (mapElement->properties.surface.slope & 0x10)
        z1 += 2;

    // Check objects above grass
    rct_map_element *mapElementAbove = mapElement;
    for (;;) {
        if (mapElementAbove->flags & MAP_ELEMENT_FLAG_LAST_TILE) {
            // Grow grass

            // Check interim grass lengths
            uint8 lengthNibble = (mapElement->properties.surface.grass_length & 0xF0) >> 4;
            if (lengthNibble < 0xF) {
                mapElement->properties.surface.grass_length += 0x10;
            } else {
                // Zeros the length nibble
                mapElement->properties.surface.grass_length += 0x10;
                mapElement->properties.surface.grass_length ^= 8;
                if (mapElement->properties.surface.grass_length & 8) {
                    // Random growth rate (length nibble)
                    mapElement->properties.surface.grass_length |= scenario_rand() & 0x70;
                } else {
                    // Increase length if not at max length
                    if (grassLength != GRASS_LENGTH_CLUMPS_2)
                        map_set_grass_length(x, y, mapElement, grassLength + 1);
                }
            }
        } else {
            mapElementAbove++;
            if (map_element_get_type(mapElementAbove) == MAP_ELEMENT_TYPE_WALL)
                continue;
            // Grass should not be affected by ghost elements.
            if (map_element_is_ghost(mapElementAbove))
                continue;
            if (z0 >= mapElementAbove->clearance_height)
                continue;
            if (z1 < mapElementAbove->base_height)
                continue;

            if (grassLength != GRASS_LENGTH_CLEAR_0)
                map_set_grass_length(x, y, mapElement, GRASS_LENGTH_CLEAR_0);
        }
        break;
    }
}

static void map_set_grass_length(sint32 x, sint32 y, rct_map_element *mapElement, sint32 length)
{
    sint32 z0, z1;

    mapElement->properties.surface.grass_length = length;
    z0 = mapElement->base_height * 8;
    z1 = z0 + 16;
    map_invalidate_tile(x, y, z0, z1);
}

void map_remove_provisional_elements()
{
    if (gFootpathProvisionalFlags & PROVISIONAL_PATH_FLAG_1)
    {
        footpath_provisional_remove();
        gFootpathProvisionalFlags |= PROVISIONAL_PATH_FLAG_1;
    }
    if (window_find_by_class(WC_RIDE_CONSTRUCTION ) != NULL)
    {
        ride_remove_provisional_track_piece();
        ride_entrance_exit_remove_ghost();
    }
}

void map_restore_provisional_elements()
{
    if (gFootpathProvisionalFlags & PROVISIONAL_PATH_FLAG_1)
    {
        gFootpathProvisionalFlags &= ~PROVISIONAL_PATH_FLAG_1;
        footpath_provisional_set(gFootpathProvisionalType,
                gFootpathProvisionalPosition.x,
                gFootpathProvisionalPosition.y,
                gFootpathProvisionalPosition.z,
                gFootpathProvisionalSlope);
    }
    if (window_find_by_class(WC_RIDE_CONSTRUCTION) != NULL)
    {
        ride_restore_provisional_track_piece();
        ride_entrance_exit_place_provisional_ghost();
    }
}

sint32 map_element_get_banner_index(rct_map_element *mapElement)
{
    rct_scenery_entry* sceneryEntry;

    switch (map_element_get_type(mapElement)) {
    case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
        sceneryEntry = get_large_scenery_entry(mapElement->properties.scenerymultiple.type & 0x3FF);
        if (sceneryEntry->large_scenery.scrolling_mode == 0xFF)
            return -1;

        return
            (mapElement->type & MAP_ELEMENT_QUADRANT_MASK) |
            ((mapElement->properties.scenerymultiple.colour[0] & 0xE0) >> 2) |
            ((mapElement->properties.scenerymultiple.colour[1] & 0xE0) >> 5);
    case MAP_ELEMENT_TYPE_WALL:
        sceneryEntry = get_wall_entry(mapElement->properties.wall.type);
        if (sceneryEntry->wall.scrolling_mode == 0xFF)
            return -1;

        return mapElement->properties.wall.banner_index;
    case MAP_ELEMENT_TYPE_BANNER:
        return mapElement->properties.banner.index;
    default:
        return -1;
    }
}

void map_element_remove_banner_entry(rct_map_element *mapElement)
{
    sint32 bannerIndex = map_element_get_banner_index(mapElement);
    if (bannerIndex == -1)
        return;

    rct_banner* banner = &gBanners[bannerIndex];
    if (banner->type != BANNER_NULL) {
        window_close_by_number(WC_BANNER, bannerIndex);
        banner->type = BANNER_NULL;
        user_string_free(banner->string_idx);
    }
}

/**
 * Removes elements that are out of the map size range and crops the park perimeter.
 *  rct2: 0x0068ADBC
 */
void map_remove_out_of_range_elements()
{
    sint32 mapMaxXY = gMapSizeMaxXY;

    for (sint32 y = 0; y < (MAXIMUM_MAP_SIZE_TECHNICAL * 32); y += 32) {
        for (sint32 x = 0; x < (MAXIMUM_MAP_SIZE_TECHNICAL * 32); x += 32) {
            if (x == 0 || y == 0 || x >= mapMaxXY || y >= mapMaxXY) {
                map_buy_land_rights(x, y, x, y, BUY_LAND_RIGHTS_FLAG_UNOWN_TILE, GAME_COMMAND_FLAG_APPLY);
                clear_elements_at(x, y);
            }
        }
    }
}

/**
 * Copies the terrain and slope from the edge of the map to the new tiles. Used when increasing the size of the map.
 *  rct2: 0x0068AC15
 */
void map_extend_boundary_surface()
{
    rct_map_element *existingMapElement, *newMapElement;
    sint32 x, y, z, slope;

    y = gMapSize - 2;
    for (x = 0; x < MAXIMUM_MAP_SIZE_TECHNICAL; x++) {
        existingMapElement = map_get_surface_element_at(x, y - 1);
        newMapElement = map_get_surface_element_at(x, y);

        newMapElement->type = (newMapElement->type & 0x7C) | (existingMapElement->type & 0x83);
        newMapElement->properties.surface.slope = existingMapElement->properties.surface.slope & 0xE0;
        newMapElement->properties.surface.terrain = existingMapElement->properties.surface.terrain;
        newMapElement->properties.surface.grass_length = existingMapElement->properties.surface.grass_length;
        newMapElement->properties.surface.ownership = 0;

        z = existingMapElement->base_height;
        slope = existingMapElement->properties.surface.slope & 9;
        if (slope == 9) {
            z += 2;
            slope = 0;
            if (existingMapElement->properties.surface.slope & 0x10) {
                slope = 1;
                if (existingMapElement->properties.surface.slope & 0x04) {
                    slope = 8;
                    if (existingMapElement->properties.surface.slope & 0x02) {
                        slope = 0;
                    }
                }
            }
        }
        if (slope & 1) slope |= 2;
        if (slope & 8) slope |= 4;

        newMapElement->properties.surface.slope |= slope;
        newMapElement->base_height = z;
        newMapElement->clearance_height = z;

        update_park_fences(x << 5, y << 5);
    }

    x = gMapSize - 2;
    for (y = 0; y < MAXIMUM_MAP_SIZE_TECHNICAL; y++) {
        existingMapElement = map_get_surface_element_at(x - 1, y);
        newMapElement = map_get_surface_element_at(x, y);

        newMapElement->type = (newMapElement->type & 0x7C) | (existingMapElement->type & 0x83);
        newMapElement->properties.surface.slope = existingMapElement->properties.surface.slope & 0xE0;
        newMapElement->properties.surface.terrain = existingMapElement->properties.surface.terrain;
        newMapElement->properties.surface.grass_length = existingMapElement->properties.surface.grass_length;
        newMapElement->properties.surface.ownership = 0;

        z = existingMapElement->base_height;
        slope = existingMapElement->properties.surface.slope & 3;
        if (slope == 3) {
            z += 2;
            slope = 0;
            if (existingMapElement->properties.surface.slope & 0x10) {
                slope = 1;
                if (existingMapElement->properties.surface.slope & 0x04) {
                    slope = 2;
                    if (existingMapElement->properties.surface.slope & 0x08) {
                        slope = 0;
                    }
                }
            }
        }
        if (slope & 1) slope |= 8;
        if (slope & 2) slope |= 4;

        newMapElement->properties.surface.slope |= slope;
        newMapElement->base_height = z;
        newMapElement->clearance_height = z;

        update_park_fences(x << 5, y << 5);
    }
}

/**
 * Clears the provided element properly from a certain tile, and updates
 * the pointer (when needed) passed to this function to point to the next element.
 */
static void clear_element_at(sint32 x, sint32 y, rct_map_element **elementPtr)
{
    rct_map_element *element = *elementPtr;
    switch (map_element_get_type(element)) {
    case MAP_ELEMENT_TYPE_SURFACE:
        element->base_height = 2;
        element->clearance_height = 2;
        element->properties.surface.slope = 0;
        element->properties.surface.terrain = 0;
        element->properties.surface.grass_length = GRASS_LENGTH_CLEAR_0;
        element->properties.surface.ownership = 0;
        // Because this element is not completely removed, the pointer must be updated manually
        // The rest of the elements are removed from the array, so the pointer doesn't need to be updated.
        (*elementPtr)++;
        break;
    case MAP_ELEMENT_TYPE_ENTRANCE:
        viewport_interaction_remove_park_entrance(element, x, y);
        break;
    case MAP_ELEMENT_TYPE_WALL:
        gGameCommandErrorTitle = STR_CANT_REMOVE_THIS;
        game_do_command(
                x,
                GAME_COMMAND_FLAG_APPLY,
                y,
                map_element_get_direction(element) | (element->base_height << 8),
                GAME_COMMAND_REMOVE_WALL,
                0,
                0
        );
        break;
    case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
        gGameCommandErrorTitle = STR_CANT_REMOVE_THIS;
        game_do_command(
                x,
                (GAME_COMMAND_FLAG_APPLY) | (map_element_get_direction(element) << 8),
                y,
                (element->base_height) | (((element->properties.scenerymultiple.type >> 8) >> 2) << 8),
                GAME_COMMAND_REMOVE_LARGE_SCENERY,
                0,
                0
        );
        break;
    case MAP_ELEMENT_TYPE_BANNER:
        gGameCommandErrorTitle = STR_CANT_REMOVE_THIS;
        game_do_command(
                x,
                GAME_COMMAND_FLAG_APPLY,
                y,
                (element->base_height) | ((element->properties.banner.position & 3) << 8),
                GAME_COMMAND_REMOVE_BANNER,
                0,
                0
        );
        break;
    default:
        map_element_remove(element);
        break;
    }
}

/**
 * Clears all elements properly from a certain tile.
 *  rct2: 0x0068AE2A
 */
static void clear_elements_at(sint32 x, sint32 y)
{
    // Remove the spawn point (if there is one in the current tile)
    for (sint32 i = 0; i < MAX_PEEP_SPAWNS; i++) {
        rct2_peep_spawn *peepSpawn = &gPeepSpawns[i];
        if (floor2(peepSpawn->x, 32) == x && floor2(peepSpawn->y, 32) == y) {
            peepSpawn->x = PEEP_SPAWN_UNDEFINED;
        }
    }

    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);

    // Remove all elements except the last one
    // BUG infinite loop may occur from map generator in active park (such as Robin Woods scenario)
    // Fix this by finding out why map_element_is_last_for_tile never ends on some.
    int numberOfIterations = 0;
    while (!map_element_is_last_for_tile(mapElement)) {
        clear_element_at(x, y, &mapElement);
        if (numberOfIterations > 10000) {
            log_warning("possible infinite loop detected in clear_elements_at(%d, %d)", x / 16, y / 16);
            break;
        }
        numberOfIterations++;
    }

    // Remove the last element
    clear_element_at(x, y, &mapElement);
}

sint32 map_get_highest_z(sint32 tileX, sint32 tileY)
{
    rct_map_element *mapElement;
    sint32 z;

    mapElement = map_get_surface_element_at(tileX, tileY);
    if (mapElement == NULL)
        return -1;

    z = mapElement->base_height * 8;

    // Raise z so that is above highest point of land and water on tile
    if ((mapElement->properties.surface.slope & 0x0F) != 0)
        z += 16;
    if ((mapElement->properties.surface.slope & 0x10) != 0)
        z += 16;

    z = max(z, map_get_water_height(mapElement) * 16);
    return z;
}

bool map_element_is_underground(rct_map_element *mapElement)
{
    do {
        mapElement++;
        if (map_element_is_last_for_tile(mapElement - 1))
            return false;
    } while (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SURFACE);
    return true;
}

rct_map_element *map_get_large_scenery_segment(sint32 x, sint32 y, sint32 z, sint32 direction, sint32 sequence)
{
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    if (mapElement == NULL)
    {
        return NULL;
    }
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE)
            continue;
        if (mapElement->base_height != z)
            continue;
        if ((mapElement->properties.scenerymultiple.type >> 10) != sequence)
            continue;
        if ((map_element_get_direction(mapElement)) != direction)
            continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));
    return NULL;
}

rct_map_element *map_get_park_entrance_element_at(sint32 x, sint32 y, sint32 z, bool ghost)
{
    rct_map_element* mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_ENTRANCE)
            continue;

        if (mapElement->base_height != z)
            continue;

        if (mapElement->properties.entrance.type != ENTRANCE_TYPE_PARK_ENTRANCE)
            continue;

        if ((ghost == false) && (mapElement->flags & MAP_ELEMENT_FLAG_GHOST))
            continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));
    return NULL;
}

rct_map_element *map_get_small_scenery_element_at(sint32 x, sint32 y, sint32 z, sint32 type, uint8 quadrant)
{
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY)
            continue;
        if (mapElement->type >> 6 != quadrant)
            continue;
        if (mapElement->base_height != z)
            continue;
        if (mapElement->properties.scenery.type != type)
            continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));
    return NULL;
}

bool map_large_scenery_get_origin(
    sint32 x, sint32 y, sint32 z, sint32 direction, sint32 sequence,
    sint32 *outX, sint32 *outY, sint32 *outZ, rct_map_element** outElement
) {
    rct_map_element *mapElement;
    rct_scenery_entry *sceneryEntry;
    rct_large_scenery_tile *tile;
    sint16 offsetX, offsetY;

    mapElement = map_get_large_scenery_segment(x, y, z, direction, sequence);
    if (mapElement == NULL)
        return false;

    sceneryEntry = get_large_scenery_entry(mapElement->properties.scenerymultiple.type & 0x3FF);
    tile = &sceneryEntry->large_scenery.tiles[sequence];

    offsetX = tile->x_offset;
    offsetY = tile->y_offset;
    rotate_map_coordinates(&offsetX, &offsetY, direction);

    *outX = x - offsetX;
    *outY = y - offsetY;
    *outZ = (z * 8) - tile->z_offset;
    if (outElement != NULL)
        *outElement = mapElement;
    return true;
}

/**
 *
 *  rct2: 0x006B9B05
 */
bool sign_set_colour(sint32 x, sint32 y, sint32 z, sint32 direction, sint32 sequence, uint8 mainColour, uint8 textColour)
{
    rct_map_element *mapElement;
    rct_scenery_entry *sceneryEntry;
    rct_large_scenery_tile *sceneryTiles, *tile;
    sint16 offsetX, offsetY;
    sint32 x0, y0, z0;

    if (!map_large_scenery_get_origin(x, y, z, direction, sequence, &x0, &y0, &z0, &mapElement)) {
        return false;
    }

    sceneryEntry = get_large_scenery_entry(mapElement->properties.scenerymultiple.type & 0x3FF);
    sceneryTiles = sceneryEntry->large_scenery.tiles;

    // Iterate through each tile of the large scenery element
    sequence = 0;
    for (tile = sceneryTiles; tile->x_offset != -1; tile++, sequence++) {
        offsetX = tile->x_offset;
        offsetY = tile->y_offset;
        rotate_map_coordinates(&offsetX, &offsetY, direction);

        x = x0 + offsetX;
        y = y0 + offsetY;
        z = (z0 + tile->z_offset) / 8;
        mapElement = map_get_large_scenery_segment(x, y, z, direction, sequence);
        if (mapElement != NULL) {
            mapElement->properties.scenerymultiple.colour[0] &= 0xE0;
            mapElement->properties.scenerymultiple.colour[1] &= 0xE0;
            mapElement->properties.scenerymultiple.colour[0] |= mainColour;
            mapElement->properties.scenerymultiple.colour[1] |= textColour;

            map_invalidate_tile(x, y, mapElement->base_height * 8 , mapElement->clearance_height * 8);
        }
    }

    return true;
}

static void translate_3d_to_2d(sint32 rotation, sint32 *x, sint32 *y)
{
    sint32 rx, ry;

    switch (rotation & 3) {
    default:
    case 0:
        rx = (*y) - (*x);
        ry = (*x) + (*y);
        break;
    case 1:
        rx = -(*x) - (*y);
        ry = (*y) - (*x);
        break;
    case 2:
        rx = (*x) - (*y);
        ry = -(*x) - (*y);
        break;
    case 3:
        rx = (*x) + (*y);
        ry = (*x) - (*y);
        break;
    }
    ry /= 2;

    *x = rx;
    *y = ry;
}

rct_xy32 translate_3d_to_2d_with_z(sint32 rotation, rct_xyz32 pos)
{
    rct_xy32 result;
    switch (rotation & 3) {
    default:
    case 0:
        result.x = pos.y - pos.x;
        result.y = (pos.x + pos.y) / 2 - pos.z;
        break;
    case 1:
        result.x = -pos.x - pos.y;
        result.y = (pos.y - pos.x) / 2 - pos.z;
        break;
    case 2:
        result.x = pos.x - pos.y;
        result.y = (-pos.x - pos.y) / 2 - pos.z;
        break;
    case 3:
        result.x = pos.x + pos.y;
        result.y = (pos.x - pos.y) / 2 - pos.z;
        break;
    }
    return result;
}

static void map_invalidate_tile_under_zoom(sint32 x, sint32 y, sint32 z0, sint32 z1, sint32 maxZoom)
{
    if (gOpenRCT2Headless) return;

    sint32 x1, y1, x2, y2;

    x += 16;
    y += 16;
    translate_3d_to_2d(get_current_rotation(), &x, &y);

    x1 = x - 32;
    y1 = y - 32 - z1;
    x2 = x + 32;
    y2 = y + 32 - z0;

    for (sint32 i = 0; i < MAX_VIEWPORT_COUNT; i++) {
        rct_viewport *viewport = &g_viewport_list[i];
        if (viewport->width != 0 && (maxZoom == -1 || viewport->zoom <= maxZoom)) {
            viewport_invalidate(viewport, x1, y1, x2, y2);
        }
    }
}

/**
 *
 *  rct2: 0x006EC847
 */
void map_invalidate_tile(sint32 x, sint32 y, sint32 z0, sint32 z1)
{
    map_invalidate_tile_under_zoom(x, y, z0, z1, -1);
}

/**
 *
 *  rct2: 0x006ECB60
 */
void map_invalidate_tile_zoom1(sint32 x, sint32 y, sint32 z0, sint32 z1)
{
    map_invalidate_tile_under_zoom(x, y, z0, z1, 1);
}

/**
 *
 *  rct2: 0x006EC9CE
 */
void map_invalidate_tile_zoom0(sint32 x, sint32 y, sint32 z0, sint32 z1)
{
    map_invalidate_tile_under_zoom(x, y, z0, z1, 0);
}

/**
 *
 *  rct2: 0x006EC6D7
 */
void map_invalidate_tile_full(sint32 x, sint32 y)
{
    map_invalidate_tile(x, y, 0, 2080);
}

void map_invalidate_element(sint32 x, sint32 y, rct_map_element *mapElement)
{
    map_invalidate_tile(x, y, mapElement->base_height * 8, mapElement->clearance_height * 8);
}

sint32 map_get_tile_side(sint32 mapX, sint32 mapY)
{
    sint32 subMapX = mapX & (32 - 1);
    sint32 subMapY = mapY & (32 - 1);
    return (subMapX < subMapY) ?
        ((subMapX + subMapY) < 32 ? 0 : 1):
        ((subMapX + subMapY) < 32 ? 3 : 2);
}

sint32 map_get_tile_quadrant(sint32 mapX, sint32 mapY)
{
    sint32 subMapX = mapX & (32 - 1);
    sint32 subMapY = mapY & (32 - 1);
    return (subMapX > 16) ?
        (subMapY < 16 ? 1 : 0):
        (subMapY < 16 ? 2 : 3);
}

/**
 *
 *  rct2: 0x00693BFF
 */
bool map_surface_is_blocked(sint16 x, sint16 y){
    rct_map_element *mapElement;
    if (x >= 8192 || y >= 8192)
        return true;

    mapElement = map_get_surface_element_at(x / 32, y / 32);

    if (mapElement == NULL) {
        return true;
    }

    sint16 water_height = map_get_water_height(mapElement);
    water_height *= 2;
    if (water_height > mapElement->base_height)
        return true;

    sint16 base_z = mapElement->base_height;
    sint16 clear_z = mapElement->base_height + 2;
    if (mapElement->properties.surface.slope & (1 << 4))
        clear_z += 2;

    while (!map_element_is_last_for_tile(mapElement++)){
        if (clear_z >= mapElement->clearance_height)
            continue;

        if (base_z < mapElement->base_height)
            continue;

        if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH ||
            map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_WALL)
            continue;

        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY)
            return true;

        rct_scenery_entry* scenery = get_small_scenery_entry(mapElement->properties.scenery.type);
        if (scenery->small_scenery.flags & SMALL_SCENERY_FLAG_FULL_TILE)
            return true;
    }
    return false;
}

/* Clears all map elements, to be used before generating a new map */
void map_clear_all_elements()
{
    for (sint32 y = 0; y < (MAXIMUM_MAP_SIZE_TECHNICAL * 32); y += 32) {
        for (sint32 x = 0; x < (MAXIMUM_MAP_SIZE_TECHNICAL * 32); x += 32) {
            clear_elements_at(x, y);
        }
    }
}

void game_command_set_sign_name(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp) {
    static char newName[128];

    if ((*ecx >= MAX_BANNERS) || (*ecx < 0))
    {
        log_warning("Invalid game command for setting sign name, banner id = %d", *ecx);
        *ebx = MONEY32_UNDEFINED;
        return;
    }
    rct_banner* banner = &gBanners[*ecx];
    sint32 x = banner->x << 5;
    sint32 y = banner->y << 5;

    sint32 nameChunkIndex = *eax & 0xFFFF;

    gCommandExpenditureType = RCT_EXPENDITURE_TYPE_RIDE_RUNNING_COSTS;
    sint32 nameChunkOffset = nameChunkIndex - 1;
    if (nameChunkOffset < 0)
        nameChunkOffset = 2;
    nameChunkOffset *= 12;
    nameChunkOffset = min(nameChunkOffset, countof(newName) - 12);
    memcpy(newName + nameChunkOffset + 0, edx, 4);
    memcpy(newName + nameChunkOffset + 4, ebp, 4);
    memcpy(newName + nameChunkOffset + 8, edi, 4);

    if (nameChunkIndex != 0) {
        *ebx = 0;
        return;
    }

    if (!(*ebx & GAME_COMMAND_FLAG_APPLY)) {
        *ebx = 0;
        return;
    }

    if (newName[0] != 0) {
        rct_string_id string_id = user_string_allocate(USER_STRING_DUPLICATION_PERMITTED, newName);
        if (string_id != 0) {
            rct_string_id prev_string_id = banner->string_idx;
            banner->string_idx = string_id;
            user_string_free(prev_string_id);

            banner->flags &= ~(BANNER_FLAG_LINKED_TO_RIDE);
            gfx_invalidate_screen();
        } else {
            gGameCommandErrorText = STR_ERR_CANT_SET_BANNER_TEXT;
            *ebx = MONEY32_UNDEFINED;
            return;
        }
    }
    else{
        sint32 rideIndex = banner_get_closest_ride_index(x, y, 16);
        if (rideIndex == -1) {
            *ebx = 0;
            return;
        }

        banner->colour = rideIndex;
        banner->flags |= BANNER_FLAG_LINKED_TO_RIDE;

        rct_string_id prev_string_id = banner->string_idx;
        banner->string_idx = STR_DEFAULT_SIGN;
        user_string_free(prev_string_id);
        gfx_invalidate_screen();
    }

    *ebx = 0;
}

void game_command_set_sign_style(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp) {
    uint8 bannerId = *ecx & 0xFF;
    if (bannerId > countof(gBanners)) {
        log_warning("Invalid game command for setting sign style, banner id = %d", bannerId);
        *ebx = MONEY32_UNDEFINED;
        return;
    }
    rct_banner *banner = &gBanners[bannerId];
    sint32 x = banner->x << 5;
    sint32 y = banner->y << 5;

    uint8 mainColour = (uint8)*edx;
    uint8 textColour = (uint8)*edi;

    if (*ebp == 0) { // small sign

        rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
        bool wall_found = false;
        do{
            if (map_element_get_type(map_element) != MAP_ELEMENT_TYPE_WALL)
                continue;

            rct_scenery_entry* scenery_entry = get_wall_entry(map_element->properties.wall.type);
            if (scenery_entry->wall.scrolling_mode == 0xFF)
                continue;
            if (map_element->properties.wall.banner_index != bannerId)
                continue;
            wall_found = true;
            break;
        } while (!map_element_is_last_for_tile(map_element++));

        if (wall_found == false) {
            *ebx = MONEY32_UNDEFINED;
            return;
        }

        if (!(*ebx & GAME_COMMAND_FLAG_APPLY)) {
            *ebx = 0;
            return;
        }
        map_element->properties.wall.colour_1 = mainColour;
        wall_element_set_secondary_colour(map_element, textColour);

        map_invalidate_tile(x, y, map_element->base_height * 8, map_element->clearance_height * 8);
    } else { // large sign
        rct_map_element *mapElement = banner_get_map_element(bannerId);
        if (mapElement == NULL || map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE) {
            gGameCommandErrorText = STR_ERR_CANT_SET_BANNER_TEXT;
            *ebx = MONEY32_UNDEFINED;
            return;
        }

        if (!(*ebx & GAME_COMMAND_FLAG_APPLY)) {
            *ebx = 0;
            return;
        }

        if (!sign_set_colour(
            banner->x * 32,
            banner->y * 32,
            mapElement->base_height,
            map_element_get_direction(mapElement),
            mapElement->properties.scenerymultiple.type >> 10,
            mainColour,
            textColour
        )) {
            *ebx = MONEY32_UNDEFINED;
            return;
        }
    }

    rct_window* w = window_bring_to_front_by_number(WC_BANNER, *ecx);
    if (w) {
        window_invalidate(w);
    }

    *ebx = 0;
}

void game_command_modify_tile(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp)
{
    const sint32 flags = *ebx;
    const sint32 x = *ecx & 0xFF;
    const sint32 y = (*ecx >> 8) & 0xFF;
    const tile_inspector_instruction instruction = *eax;

    switch (instruction)
    {
    case TILE_INSPECTOR_ANY_REMOVE:
    {
        const sint16 elementIndex = *edx;
        *ebx = tile_inspector_remove_element_at(x, y, elementIndex, flags);
        break;
    }
    case TILE_INSPECTOR_ANY_SWAP:
    {
        const sint32 firstIndex = *edx;
        const sint32 secondIndex = *edi;
        *ebx = tile_inspector_swap_elements_at(x, y, firstIndex, secondIndex, flags);
        break;
    }
    case TILE_INSPECTOR_ANY_INSERT_CORRUPT:
    {
        const sint16 elementIndex = *edx;
        *ebx = tile_inspector_insert_corrupt_at(x, y, elementIndex, flags);
        break;
    }
    case TILE_INSPECTOR_ANY_ROTATE:
    {
        const sint16 elementIndex = *edx;
        *ebx = tile_inspector_rotate_element_at(x, y, elementIndex, flags);
        break;
    }
    case TILE_INSPECTOR_ANY_PASTE:
    {
        rct_map_element elementToPaste;
        const sint32 data[] = { *edx, *edi };
        assert_struct_size(data, sizeof(elementToPaste));
        memcpy(&elementToPaste, data, 8);
        *ebx = tile_inspector_paste_element_at(x, y, elementToPaste, flags);
        break;
    }
    case TILE_INSPECTOR_ANY_SORT:
    {
        *ebx = tile_inspector_sort_elements_at(x, y, flags);
        break;
    }
    case TILE_INSPECTOR_ANY_BASE_HEIGHT_OFFSET:
    {
        const sint16 elementIndex = *edx;
        const sint8 heightOffset = *edi;
        *ebx = tile_inspector_any_base_height_offset(x, y, elementIndex, heightOffset, flags);
        break;
    }
    case TILE_INSPECTOR_SURFACE_SHOW_PARK_FENCES:
    {
        const bool showFences = *edx;
        *ebx = tile_inspector_surface_show_park_fences(x, y, showFences, flags);
        break;
    }
    case TILE_INSPECTOR_SURFACE_TOGGLE_CORNER:
    {
        const sint32 cornerIndex = *edx;
        *ebx = tile_inspector_surface_toggle_corner(x, y, cornerIndex, flags);
        break;
    }
    case TILE_INSPECTOR_SURFACE_TOGGLE_DIAGONAL:
    {
        *ebx = tile_inspector_surface_toggle_diagonal(x, y, flags);
        break;
    }
    case TILE_INSPECTOR_PATH_SET_SLOPE:
    {
        const sint32 elementIndex = *edx;
        const bool sloped = *edi;
        *ebx = tile_inspector_path_set_sloped(x, y, elementIndex, sloped, flags);
        break;
    }
    case TILE_INSPECTOR_PATH_TOGGLE_EDGE:
    {
        const sint32 elementIndex = *edx;
        const sint32 edgeIndex = *edi;
        *ebx = tile_inspector_path_toggle_edge(x, y, elementIndex, edgeIndex, flags);
        break;
    }
    case TILE_INSPECTOR_WALL_SET_SLOPE:
    {
        const sint32 elementIndex = *edx;
        const sint32 slopeValue = *edi;
        *ebx = tile_inspector_wall_set_slope(x, y, elementIndex, slopeValue, flags);
        break;
    }
    case TILE_INSPECTOR_TRACK_BASE_HEIGHT_OFFSET:
    {
        const sint32 elementIndex = *edx;
        const sint8 heightOffset = *edi;
        *ebx = tile_inspector_track_base_height_offset(x, y, elementIndex, heightOffset, flags);
        break;
    }
    case TILE_INSPECTOR_TRACK_SET_CHAIN:
    {
        const sint32 elementIndex = *edx;
        const bool entireTrackBlock = *edi;
        const bool setChain = *ebp;
        *ebx = tile_inspector_track_set_chain(x, y, elementIndex, entireTrackBlock, setChain, flags);
        break;
    }
    case TILE_INSPECTOR_SCENERY_SET_QUARTER_LOCATION:
    {
        const sint32 elementIndex = *edx;
        const sint32 quarterIndex = *edi;
        *ebx = tile_inspector_scenery_set_quarter_location(x, y, elementIndex, quarterIndex, flags);
        break;
    }
    case TILE_INSPECTOR_SCENERY_SET_QUARTER_COLLISION:
    {
        const sint32 elementIndex = *edx;
        const sint32 quarterIndex = *edi;
        *ebx = tile_inspector_scenery_set_quarter_collision(x, y, elementIndex, quarterIndex, flags);
        break;
    }
    case TILE_INSPECTOR_BANNER_TOGGLE_BLOCKING_EDGE:
    {
        const sint32 elementIndex = *edx;
        const sint32 edgeIndex = *edi;
        *ebx = tile_inspector_banner_toggle_blocking_edge(x, y, elementIndex, edgeIndex, flags);
        break;
    }
    case TILE_INSPECTOR_CORRUPT_CLAMP:
    {
        const sint32 elementIndex = *edx;
        *ebx = tile_inspector_corrupt_clamp(x, y, elementIndex, flags);
        break;
    }
    default:
        log_error("invalid instruction");
        *ebx = MONEY32_UNDEFINED;
        break;
    }

    if (flags & GAME_COMMAND_FLAG_APPLY &&
            gGameCommandNestLevel == 1 &&
            !(flags & GAME_COMMAND_FLAG_GHOST) &&
            *ebx != MONEY32_UNDEFINED)
    {
        rct_xyz16 coord;
        coord.x = (x << 5) + 16;
        coord.y = (y << 5) + 16;
        coord.z = map_element_height(coord.x, coord.y);
        network_set_player_last_action_coord(network_get_player_index(game_command_playerid), coord);
    }
}

/**
 * Gets the track element at x, y, z.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 */
rct_map_element *map_get_track_element_at(sint32 x, sint32 y, sint32 z)
{
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
        if (mapElement->base_height != z) continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
}

/**
 * Gets the track element at x, y, z that is the given track type.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 * @param trackType
 */
rct_map_element *map_get_track_element_at_of_type(sint32 x, sint32 y, sint32 z, sint32 trackType)
{
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
        if (mapElement->base_height != z) continue;
        if (mapElement->properties.track.type != trackType) continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
}

/**
 * Gets the track element at x, y, z that is the given track type and sequence.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 * @param trackType
 * @param sequence
 */
rct_map_element *map_get_track_element_at_of_type_seq(sint32 x, sint32 y, sint32 z, sint32 trackType, sint32 sequence)
{
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (mapElement == NULL) break;
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
        if (mapElement->base_height != z) continue;
        if (mapElement->properties.track.type != trackType) continue;
        if (map_element_get_track_sequence(mapElement) != sequence) continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
}

/**
 * Gets the track element at x, y, z that is the given track type and sequence.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 * @param trackType
 * @param ride index
 */
rct_map_element *map_get_track_element_at_of_type_from_ride(sint32 x, sint32 y, sint32 z, sint32 trackType, sint32 rideIndex) {
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
        if (mapElement->base_height != z) continue;
        if (mapElement->properties.track.ride_index != rideIndex) continue;
        if (mapElement->properties.track.type != trackType) continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
};

/**
 * Gets the track element at x, y, z that is the given track type and sequence.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 * @param ride index
 */
rct_map_element *map_get_track_element_at_from_ride(sint32 x, sint32 y, sint32 z, sint32 rideIndex) {
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
        if (mapElement->base_height != z) continue;
        if (mapElement->properties.track.ride_index != rideIndex) continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
};

/**
 * Gets the track element at x, y, z that is the given track type and sequence.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 * @param direction The direction (0 - 3).
 * @param ride index
 */
rct_map_element *map_get_track_element_at_with_direction_from_ride(sint32 x, sint32 y, sint32 z, sint32 direction, sint32 rideIndex)
{
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
        if (mapElement->base_height != z) continue;
        if (mapElement->properties.track.ride_index != rideIndex) continue;
        if (map_element_get_direction(mapElement) != direction) continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));

    return NULL;
};

void map_offset_with_rotation(sint16 *x, sint16 *y, sint16 offsetX, sint16 offsetY, uint8 rotation)
{
    switch (rotation & 3) {
    case MAP_ELEMENT_DIRECTION_WEST:
        *x += offsetX;
        *y += offsetY;
        break;
    case MAP_ELEMENT_DIRECTION_NORTH:
        *x += offsetY;
        *y -= offsetX;
        break;
    case MAP_ELEMENT_DIRECTION_EAST:
        *x -= offsetX;
        *y -= offsetY;
        break;
    case MAP_ELEMENT_DIRECTION_SOUTH:
        *x -= offsetY;
        *y += offsetX;
        break;
    }
}

rct_map_element *map_get_wall_element_at(sint32 x, sint32 y, sint32 z, sint32 direction)
{
    rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
    do {
        if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_WALL)
            continue;
        if (mapElement->base_height != z)
            continue;
        if (map_element_get_direction(mapElement) != direction)
            continue;

        return mapElement;
    } while (!map_element_is_last_for_tile(mapElement++));
    return NULL;
}

uint32 map_get_available_peep_spawn_index_list(uint32* peepSpawnIndexList)
{
    uint32 numSpawns = 0;
    for (uint8 i = 0; i < MAX_PEEP_SPAWNS; i++) {
        if (gPeepSpawns[i].x != PEEP_SPAWN_UNDEFINED) {
            peepSpawnIndexList[numSpawns++] = i;
        }
    }
    return numSpawns;
}

uint16 check_max_allowable_land_rights_for_tile(uint8 x, uint8 y, uint8 base_z)
{
    rct_map_element * mapElement = map_get_first_element_at(x, y);
    uint16 destOwnership = OWNERSHIP_OWNED;

    // Sometimes done deliberately.
    if (mapElement == NULL)
    {
        return OWNERSHIP_OWNED;
    }

    do
    {
        sint32 type = map_element_get_type(mapElement);
        if (type == MAP_ELEMENT_TYPE_PATH ||
            (type == MAP_ELEMENT_TYPE_ENTRANCE && mapElement->properties.entrance.type == ENTRANCE_TYPE_PARK_ENTRANCE))
        {
            destOwnership = OWNERSHIP_CONSTRUCTION_RIGHTS_OWNED;
            // Do not own construction rights if too high/below surface
            if (mapElement->base_height - 3 > base_z ||
                mapElement->base_height < base_z)
            {
                destOwnership = OWNERSHIP_UNOWNED;
                break;
            }
        }
    }
    while (!map_element_is_last_for_tile(mapElement++));

    return destOwnership;
}
