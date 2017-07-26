#pragma once

#include "rct2/addresses.h"
#include "ride/ride.h"
#include "world/banner.h"
#include "world/entrance.h"
#include "world/footpath.h"
#include "world/scenery.h"
#include "world/water.h"
#include "common.h"

#define OBJECT_ENTRY_GROUP_COUNT_RIDES         128
#define OBJECT_ENTRY_GROUP_COUNT_SMALL_SCENERY 252
#define OBJECT_ENTRY_GROUP_COUNT_LARGE_SCENERY 128
#define OBJECT_ENTRY_GROUP_COUNT_WALLS         128
#define OBJECT_ENTRY_GROUP_COUNT_BANNERS       128
#define OBJECT_ENTRY_GROUP_COUNT_PATHS         16
#define OBJECT_ENTRY_GROUP_COUNT_PATH_BITS     15
#define OBJECT_ENTRY_GROUP_COUNT_SCENERY_SETS  19
#define OBJECT_ENTRY_GROUP_COUNT_PARK_ENTRANCE 1
#define OBJECT_ENTRY_GROUP_COUNT_WATER         1
#define OBJECT_ENTRY_GROUP_COUNT_SCENARIO_TEXT 1

#if defined(NO_RCT2) && defined(RAISED_LIMITS)
#define OBJECT_ENTRY_GROUP_EXT_COUNT_RIDES         320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_SMALL_SCENERY 320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_LARGE_SCENERY 320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_WALLS         320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_BANNERS       320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_PATHS         320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_PATH_BITS     320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_SCENERY_SETS  320
#define OBJECT_ENTRY_GROUP_EXT_COUNT_PARK_ENTRANCE 1
#define OBJECT_ENTRY_GROUP_EXT_COUNT_WATER         1
#define OBJECT_ENTRY_GROUP_EXT_COUNT_SCENARIO_TEXT 1
#else
#define OBJECT_ENTRY_GROUP_EXT_COUNT_RIDES         OBJECT_ENTRY_GROUP_COUNT_RIDES
#define OBJECT_ENTRY_GROUP_EXT_COUNT_SMALL_SCENERY OBJECT_ENTRY_GROUP_COUNT_SMALL_SCENERY
#define OBJECT_ENTRY_GROUP_EXT_COUNT_LARGE_SCENERY OBJECT_ENTRY_GROUP_COUNT_LARGE_SCENERY
#define OBJECT_ENTRY_GROUP_EXT_COUNT_WALLS         OBJECT_ENTRY_GROUP_COUNT_WALLS
#define OBJECT_ENTRY_GROUP_EXT_COUNT_BANNERS       OBJECT_ENTRY_GROUP_COUNT_BANNERS
#define OBJECT_ENTRY_GROUP_EXT_COUNT_PATHS         OBJECT_ENTRY_GROUP_COUNT_PATHS
#define OBJECT_ENTRY_GROUP_EXT_COUNT_PATH_BITS     OBJECT_ENTRY_GROUP_COUNT_PATH_BITS
#define OBJECT_ENTRY_GROUP_EXT_COUNT_SCENERY_SETS  OBJECT_ENTRY_GROUP_COUNT_SCENERY_SETS
#define OBJECT_ENTRY_GROUP_EXT_COUNT_PARK_ENTRANCE OBJECT_ENTRY_GROUP_COUNT_PARK_ENTRANCE
#define OBJECT_ENTRY_GROUP_EXT_COUNT_WATER         OBJECT_ENTRY_GROUP_COUNT_WATER
#define OBJECT_ENTRY_GROUP_EXT_COUNT_SCENARIO_TEXT OBJECT_ENTRY_GROUP_COUNT_SCENARIO_TEXT
#endif


#ifdef NO_RCT2
    extern rct_ride_entry               *gRideEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_RIDES];
    extern rct_small_scenery_entry      *gSmallSceneryEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_SMALL_SCENERY];
    extern rct_large_scenery_entry      *gLargeSceneryEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_LARGE_SCENERY];
    extern rct_wall_scenery_entry       *gWallSceneryEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_WALLS];
    extern rct_banner                   *gBannerSceneryEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_BANNERS];
    extern rct_footpath_entry           *gFootpathEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_PATHS];
    extern rct_path_bit_scenery_entry   *gFootpathAdditionEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_PATH_BITS];
    extern rct_scenery_set_entry        *gSceneryGroupEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_SCENERY_SETS];
    extern rct_entrance_type            *gParkEntranceEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_PARK_ENTRANCE];
    extern rct_water_type               *gWaterEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_WATER];
    extern rct_stex_entry               *gStexEntries[OBJECT_ENTRY_GROUP_EXT_COUNT_SCENARIO_TEXT];
#else


    #define gRideEntries                RCT2_ADDRESS(RCT2_ADDRESS_RIDE_ENTRIES,             rct_ride_entry*)
    #define gSmallSceneryEntries        RCT2_ADDRESS(RCT2_ADDRESS_SMALL_SCENERY_ENTRIES,    rct_small_scenery_entry*)
    #define gLargeSceneryEntries        RCT2_ADDRESS(RCT2_ADDRESS_LARGE_SCENERY_ENTRIES,    rct_large_scenery_entry*)
    #define gWallSceneryEntries         RCT2_ADDRESS(RCT2_ADDRESS_WALL_SCENERY_ENTRIES,     rct_wall_scenery_entry*)
    #define gBannerSceneryEntries       RCT2_ADDRESS(RCT2_ADDRESS_BANNER_SCENERY_ENTRIES,   rct_banner*)
    #define gFootpathEntries            RCT2_ADDRESS(RCT2_ADDRESS_PATH_TYPES,               rct_footpath_entry*)
    #define gFootpathAdditionEntries    RCT2_ADDRESS(RCT2_ADDRESS_PATH_BIT_SCENERY_ENTRIES, rct_path_bit_scenery_entry*)
    #define gSceneryGroupEntries        RCT2_ADDRESS(RCT2_ADDRESS_SCENERY_SET_ENTRIES,      rct_scenery_set_entry*)
    #define gParkEntranceEntries        RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_ENTRIES,    rct_entrance_type*)
    #define gWaterEntries               RCT2_ADDRESS(RCT2_ADDRESS_WATER_ENTRIES,            rct_water_type*)
    #define gStexEntries                RCT2_ADDRESS(RCT2_ADDRESS_SCENARIO_TEXT_ENTRIES,    rct_stex_entry*)
#endif

void get_type_entry_index(size_t index, uint8 * outObjectType, uint8 * outEntryIndex);
const rct_object_entry * get_loaded_object_entry(size_t index);
void * get_loaded_object_chunk(size_t index);
