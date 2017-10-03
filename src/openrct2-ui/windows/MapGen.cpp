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

#include <openrct2/Context.h>
#include <openrct2/core/Math.hpp>
#include <openrct2-ui/windows/Window.h>

#include <openrct2/localisation/localisation.h>
#include <openrct2/input.h>
#include <openrct2/interface/land_tool.h>
#include <openrct2/interface/widget.h>
#include <openrct2/util/util.h>
#include <openrct2/world/mapgen.h>
#include <openrct2/windows/dropdown.h>

enum {
    WINDOW_MAPGEN_PAGE_BASE,
    WINDOW_MAPGEN_PAGE_TREES,
    WINDOW_MAPGEN_PAGE_SIMPLEX,
    WINDOW_MAPGEN_PAGE_HEIGHTMAP,
    WINDOW_MAPGEN_PAGE_COUNT
};

enum {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_PAGE_BACKGROUND,
    WIDX_TAB_1,
    WIDX_TAB_2,
    WIDX_TAB_3,
    WIDX_TAB_4,


    TAB_BEGIN,

    WIDX_MAP_GENERATE = TAB_BEGIN,
    WIDX_MAP_SIZE,
    WIDX_MAP_SIZE_UP,
    WIDX_MAP_SIZE_DOWN,
    WIDX_BASE_HEIGHT,
    WIDX_BASE_HEIGHT_UP,
    WIDX_BASE_HEIGHT_DOWN,
    WIDX_WATER_LEVEL,
    WIDX_WATER_LEVEL_UP,
    WIDX_WATER_LEVEL_DOWN,
    WIDX_FLOOR_TEXTURE,
    WIDX_WALL_TEXTURE,

    WIDX_TREES_GENERATE = TAB_BEGIN,
    WIDX_TREES_PLACE_TREES,
    WIDX_TREES_HIGH,
    WIDX_TREES_HIGH_UP,
    WIDX_TREES_HIGH_DOWN,
    WIDX_TREES_BASE_FREQ,
    WIDX_TREES_BASE_FREQ_UP,
    WIDX_TREES_BASE_FREQ_DOWN,
    WIDX_TREES_OCTAVES,
    WIDX_TREES_OCTAVES_UP,
    WIDX_TREES_OCTAVES_DOWN,

    WIDX_SIMPLEX_GENERATE = TAB_BEGIN,
    WIDX_SIMPLEX_LABEL,
    WIDX_SIMPLEX_LOW,
    WIDX_SIMPLEX_LOW_UP,
    WIDX_SIMPLEX_LOW_DOWN,
    WIDX_SIMPLEX_HIGH,
    WIDX_SIMPLEX_HIGH_UP,
    WIDX_SIMPLEX_HIGH_DOWN,
    WIDX_SIMPLEX_BASE_FREQ,
    WIDX_SIMPLEX_BASE_FREQ_UP,
    WIDX_SIMPLEX_BASE_FREQ_DOWN,
    WIDX_SIMPLEX_OCTAVES,
    WIDX_SIMPLEX_OCTAVES_UP,
    WIDX_SIMPLEX_OCTAVES_DOWN,
    WIDX_SIMPLEX_MAP_SIZE,
    WIDX_SIMPLEX_MAP_SIZE_UP,
    WIDX_SIMPLEX_MAP_SIZE_DOWN,
    WIDX_SIMPLEX_WATER_LEVEL,
    WIDX_SIMPLEX_WATER_LEVEL_UP,
    WIDX_SIMPLEX_WATER_LEVEL_DOWN,
    WIDX_SIMPLEX_RANDOM_TERRAIN_CHECKBOX,
    WIDX_SIMPLEX_FLOOR_TEXTURE,
    WIDX_SIMPLEX_WALL_TEXTURE,
    WIDX_SIMPLEX_PLACE_TREES_CHECKBOX,

    WIDX_HEIGHTMAP_SELECT = TAB_BEGIN,
    WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP,
    WIDX_HEIGHTMAP_STRENGTH,
    WIDX_HEIGHTMAP_STRENGTH_UP,
    WIDX_HEIGHTMAP_STRENGTH_DOWN,
    WIDX_HEIGHTMAP_NORMALIZE,
    WIDX_HEIGHTMAP_SMOOTH_TILES,
    WIDX_HEIGHTMAP_LOW,
    WIDX_HEIGHTMAP_LOW_UP,
    WIDX_HEIGHTMAP_LOW_DOWN,
    WIDX_HEIGHTMAP_HIGH,
    WIDX_HEIGHTMAP_HIGH_UP,
    WIDX_HEIGHTMAP_HIGH_DOWN,
    WIDX_HEIGHTMAP_WATER_LEVEL,
    WIDX_HEIGHTMAP_WATER_LEVEL_UP,
    WIDX_HEIGHTMAP_WATER_LEVEL_DOWN,
};

#pragma region Widgets

#define WW 250
#define WH 273

#define SHARED_WIDGETS \
    { WWT_FRAME,    0,  0,          WW - 1, 0,  WH - 1, 0xFFFFFFFF,                 STR_NONE },             /* WIDX_BACKGROUND */ \
    { WWT_CAPTION,  0,  1,          WW - 2, 1,  14,     STR_MAPGEN_WINDOW_TITLE,    STR_WINDOW_TITLE_TIP }, /* WIDX_TITLE */ \
    { WWT_CLOSEBOX, 0,  WW - 13,    WW - 3, 2,  13,     STR_CLOSE_X,                STR_CLOSE_WINDOW_TIP }, /* WIDX_CLOSE */ \
    { WWT_RESIZE,   1,  0,          WW - 1, 43, WH - 2, 0xFFFFFFFF,                 STR_NONE },             /* WIDX_PAGE_BACKGROUND */ \
    { WWT_TAB,      1,  3,          33,     17, 43,     IMAGE_TYPE_REMAP | SPR_TAB,       STR_NONE },             /* WIDX_TAB_1 */ \
    { WWT_TAB,      1,  34,         64,     17, 43,     IMAGE_TYPE_REMAP | SPR_TAB,       STR_NONE },             /* WIDX_TAB_2 */ \
    { WWT_TAB,      1,  65,         95,     17, 43,     IMAGE_TYPE_REMAP | SPR_TAB,       STR_NONE },             /* WIDX_TAB_3 */ \
    { WWT_TAB,      1,  96,         126,    17, 43,     IMAGE_TYPE_REMAP | SPR_TAB,       STR_NONE }              /* WIDX_TAB_4 */

static rct_widget MapWidgets[] = {
    SHARED_WIDGETS,

    { WWT_DROPDOWN_BUTTON,  1, WW - 95, WW - 6, WH - 17, WH - 6, STR_MAPGEN_ACTION_GENERATE, STR_NONE },

    { WWT_SPINNER,          1,  104,    198,    52,     63,     STR_NONE,                   STR_NONE },
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    53,     57,     STR_NUMERIC_UP,             STR_NONE },
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    58,     62,     STR_NUMERIC_DOWN,           STR_NONE },
    { WWT_SPINNER,          1,  104,    198,    70,     81,     STR_NONE,                   STR_NONE },
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    71,     75,     STR_NUMERIC_UP,             STR_NONE },
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    76,     80,     STR_NUMERIC_DOWN,           STR_NONE },
    { WWT_SPINNER,          1,  104,    198,    88,     99,     STR_NONE,                   STR_NONE },
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    89,     93,     STR_NUMERIC_UP,             STR_NONE },
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    94,     98,     STR_NUMERIC_DOWN,           STR_NONE },
    { WWT_FLATBTN,          1,  104,    150,    106,    141,    0xFFFFFFFF,                 STR_CHANGE_BASE_LAND_TIP },
    { WWT_FLATBTN,          1,  151,    197,    106,    141,    0xFFFFFFFF,                 STR_CHANGE_VERTICAL_LAND_TIP },
    { WIDGETS_END },
};

static rct_widget TreesWidgets[] = {
    SHARED_WIDGETS,

    { WWT_DROPDOWN_BUTTON,  1, WW - 95, WW - 6, WH - 17, WH - 6, STR_MAPGEN_ACTION_GENERATE,        STR_NONE },

    { WWT_CHECKBOX,         1,  4,      198,    52,     63,     STR_MAPGEN_OPTION_PLACE_TREES,      STR_NONE }, // WIDX_TREES_PLACE_TREES

    { WWT_SPINNER,          1,  104,    198,    88,     99,     STR_NONE,                         STR_NONE }, // WIDX_TREES_HIGH
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    89,     93,     STR_NUMERIC_UP,                   STR_NONE }, // WIDX_TREES_HIGH_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    94,     98,     STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_TREES_HIGH_DOWN

    { WWT_SPINNER,          1,  104,    198,    106,    117,    STR_NONE,                         STR_NONE }, // WIDX_TREES_BASE_FREQ
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    107,    111,    STR_NUMERIC_UP,                   STR_NONE }, // WIDX_TREES_BASE_FREQ_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    112,    116,    STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_TREES_BASE_FREQ_DOWN

    { WWT_SPINNER,          1,  104,    198,    124,    135,    STR_NONE,                         STR_NONE }, // WIDX_TREES_OCTAVES
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    125,    129,    STR_NUMERIC_UP,                   STR_NONE }, // WIDX_TREES_OCTAVES_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    130,    134,    STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_TREES_OCTAVES_DOWN


    { WIDGETS_END },
};

static rct_widget SimplexWidgets[] = {
    SHARED_WIDGETS,

    { WWT_DROPDOWN_BUTTON,  1, WW - 95, WW - 6, WH - 17, WH - 6, STR_MAPGEN_ACTION_GENERATE,      STR_NONE }, // WIDX_SIMPLEX_GENERATE

    { WWT_12,               1,  4,      198,    52,     63,     STR_MAPGEN_SIMPLEX_NOISE,         STR_NONE }, // WIDX_SIMPLEX_LABEL

    { WWT_SPINNER,          1,  104,    198,    70,     81,     STR_NONE,                         STR_NONE }, // WIDX_SIMPLEX_LOW
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    71,     75,     STR_NUMERIC_UP,                   STR_NONE }, // WIDX_SIMPLEX_LOW_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    76,     80,     STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_SIMPLEX_LOW_DOWN

    { WWT_SPINNER,          1,  104,    198,    88,     99,     STR_NONE,                         STR_NONE }, // WIDX_SIMPLEX_HIGH
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    89,     93,     STR_NUMERIC_UP,                   STR_NONE }, // WIDX_SIMPLEX_HIGH_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    94,     98,     STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_SIMPLEX_HIGH_DOWN

    { WWT_SPINNER,          1,  104,    198,    106,    117,    STR_NONE,                         STR_NONE }, // WIDX_SIMPLEX_BASE_FREQ
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    107,    111,    STR_NUMERIC_UP,                   STR_NONE }, // WIDX_SIMPLEX_BASE_FREQ_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    112,    116,    STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_SIMPLEX_BASE_FREQ_DOWN

    { WWT_SPINNER,          1,  104,    198,    124,    135,    STR_NONE,                         STR_NONE }, // WIDX_SIMPLEX_OCTAVES
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    125,    129,    STR_NUMERIC_UP,                   STR_NONE }, // WIDX_SIMPLEX_OCTAVES_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    130,    134,    STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_SIMPLEX_OCTAVES_DOWN

    { WWT_SPINNER,          1,  104,    198,    148,    159,    STR_NONE,                         STR_NONE }, // WIDX_SIMPLEX_MAP_SIZE
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    149,    153,    STR_NUMERIC_UP,                   STR_NONE }, // WIDX_SIMPLEX_MAP_SIZE_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    154,    158,    STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_SIMPLEX_MAP_SIZE_DOWN

    { WWT_SPINNER,          1,  104,    198,    166,    177,    STR_NONE,                         STR_NONE }, // WIDX_SIMPLEX_WATER_LEVEL
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    167,    171,    STR_NUMERIC_UP,                   STR_NONE }, // WIDX_SIMPLEX_WATER_LEVEL_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    172,    176,    STR_NUMERIC_DOWN,                 STR_NONE }, // WIDX_SIMPLEX_WATER_LEVEL_DOWN

    { WWT_CHECKBOX,         1,  104,    198,    190,    201,    STR_MAPGEN_OPTION_RANDOM_TERRAIN, STR_NONE }, // WIDX_SIMPLEX_RANDOM_TERRAIN_CHECKBOX
    { WWT_FLATBTN,          1,  102,    148,    202,    237,    0xFFFFFFFF,                       STR_CHANGE_BASE_LAND_TIP }, // WIDX_SIMPLEX_FLOOR_TEXTURE
    { WWT_FLATBTN,          1,  150,    196,    202,    237,    0xFFFFFFFF,                       STR_CHANGE_VERTICAL_LAND_TIP }, // WIDX_SIMPLEX_WALL_TEXTURE

    { WWT_CHECKBOX,         1,  104,    198,    239,    250,    STR_NONE,                         STR_NONE }, // WIDX_SIMPLEX_PLACE_TREES_CHECKBOX

    { WIDGETS_END },
};

static rct_widget HeightmapWidgets[] = {
    SHARED_WIDGETS,

    { WWT_DROPDOWN_BUTTON,  1, WW - 155, WW - 6, WH - 17, WH - 6, STR_MAPGEN_SELECT_HEIGHTMAP, STR_NONE }, // WIDX_HEIGHTMAP_SELECT

    { WWT_CHECKBOX,         1,  4,      103,    52,     63,     STR_MAPGEN_SMOOTH_HEIGHTMAP,STR_NONE }, // WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP
    { WWT_SPINNER,          1,  104,    198,    70,     81,     STR_NONE,                   STR_NONE }, // WIDX_HEIGHTMAP_STRENGTH
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    71,     75,     STR_NUMERIC_UP,             STR_NONE }, // WIDX_HEIGHTMAP_STRENGTH_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    76,     80,     STR_NUMERIC_DOWN,           STR_NONE }, // WIDX_HEIGHTMAP_STRENGTH_DOWN

    { WWT_CHECKBOX,         1,  4,      103,    88,     99,     STR_MAPGEN_NORMALIZE,       STR_NONE }, // WIDX_HEIGHTMAP_NORMALIZE

    { WWT_CHECKBOX,         1,  4,      103,    106,    117,    STR_MAPGEN_SMOOTH_TILE,     STR_NONE }, // WIDX_HEIGHTMAP_SMOOTH_TILES

    { WWT_SPINNER,          1,  104,    198,    124,    135,    STR_NONE,                   STR_NONE }, // WIDX_HEIGHTMAP_LOW
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    125,    129,    STR_NUMERIC_UP,             STR_NONE }, // WIDX_HEIGHTMAP_LOW_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    130,    134,    STR_NUMERIC_DOWN,           STR_NONE }, // WIDX_HEIGHTMAP_LOW_DOWN

    { WWT_SPINNER,          1,  104,    198,    142,    153,    STR_NONE,                   STR_NONE }, // WIDX_HEIGHTMAP_HIGH
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    143,    147,    STR_NUMERIC_UP,             STR_NONE }, // WIDX_HEIGHTMAP_HIGH_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    148,    152,    STR_NUMERIC_DOWN,           STR_NONE }, // WIDX_HEIGHTMAP_HIGH_DOWN

    { WWT_SPINNER,          1,  104,    198,    160,    171,    STR_NONE,                   STR_NONE }, // WIDX_HEIGHTMAP_WATER_LEVEL
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    161,    165,    STR_NUMERIC_UP,             STR_NONE }, // WIDX_HEIGHTMAP_WATER_LEVEL_UP
    { WWT_DROPDOWN_BUTTON,  1,  187,    197,    166,    170,    STR_NUMERIC_DOWN,           STR_NONE }, // WIDX_HEIGHTMAP_WATER_LEVEL_DOWN

    { WIDGETS_END },
};

static rct_widget *PageWidgets[WINDOW_MAPGEN_PAGE_COUNT] = {
    MapWidgets,
    TreesWidgets,
    SimplexWidgets,
    HeightmapWidgets
};

#pragma endregion

#pragma region Events

static void window_mapgen_shared_close(rct_window *w);
static void window_mapgen_shared_mouseup(rct_window *w, rct_widgetindex widgetIndex);

static void window_mapgen_base_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_mapgen_base_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_mapgen_base_dropdown(rct_window *w, rct_widgetindex widgetIndex, sint32 dropdownIndex);
static void window_mapgen_base_update(rct_window *w);
static void window_mapgen_textinput(rct_window *w, rct_widgetindex widgetIndex, char *text);
static void window_mapgen_base_invalidate(rct_window *w);
static void window_mapgen_base_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_mapgen_trees_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_mapgen_trees_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_mapgen_trees_update(rct_window *w);
static void window_mapgen_trees_invalidate(rct_window *w);
static void window_mapgen_trees_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_mapgen_simplex_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_mapgen_simplex_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_mapgen_simplex_dropdown(rct_window *w, rct_widgetindex widgetIndex, sint32 dropdownIndex);
static void window_mapgen_simplex_update(rct_window *w);
static void window_mapgen_simplex_invalidate(rct_window *w);
static void window_mapgen_simplex_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_mapgen_heightmap_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_mapgen_heightmap_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget);
static void window_mapgen_heightmap_invalidate(rct_window *w);
static void window_mapgen_heightmap_paint(rct_window *w, rct_drawpixelinfo *dpi);

static rct_window_event_list BaseEvents = {
    window_mapgen_shared_close,
    window_mapgen_base_mouseup,
    nullptr,
    window_mapgen_base_mousedown,
    window_mapgen_base_dropdown,
    nullptr,
    window_mapgen_base_update,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_mapgen_textinput,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_mapgen_base_invalidate,
    window_mapgen_base_paint,
    nullptr
};

static rct_window_event_list TreesEvents = {
    window_mapgen_shared_close,
    window_mapgen_trees_mouseup,
    nullptr,
    window_mapgen_trees_mousedown,
    nullptr,
    nullptr,
    window_mapgen_trees_update,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_mapgen_trees_invalidate,
    window_mapgen_trees_paint,
    nullptr
};

static rct_window_event_list SimplexEvents = {
    window_mapgen_shared_close,
    window_mapgen_simplex_mouseup,
    nullptr,
    window_mapgen_simplex_mousedown,
    window_mapgen_simplex_dropdown,
    nullptr,
    window_mapgen_simplex_update,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_mapgen_textinput,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_mapgen_simplex_invalidate,
    window_mapgen_simplex_paint,
    nullptr
};

static rct_window_event_list HeightmapEvents = {
    window_mapgen_shared_close,
    window_mapgen_heightmap_mouseup,
    nullptr,
    window_mapgen_heightmap_mousedown,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    window_mapgen_heightmap_invalidate,
    window_mapgen_heightmap_paint,
    nullptr
};

static rct_window_event_list *PageEvents[] = {
    &BaseEvents,
    &TreesEvents,
    &SimplexEvents,
    &HeightmapEvents
};

#pragma endregion

#pragma region Enabled widgets

static uint64 PageEnabledWidgets[WINDOW_MAPGEN_PAGE_COUNT] = {
    (1ULL << WIDX_CLOSE) |
    (1ULL << WIDX_TAB_1) |
    (1ULL << WIDX_TAB_2) |
    (1ULL << WIDX_TAB_3) |
    (1ULL << WIDX_TAB_4) |
    (1ULL << WIDX_MAP_GENERATE) |
    (1ULL << WIDX_MAP_SIZE) |
    (1ULL << WIDX_MAP_SIZE_UP) |
    (1ULL << WIDX_MAP_SIZE_DOWN) |
    (1ULL << WIDX_BASE_HEIGHT) |
    (1ULL << WIDX_BASE_HEIGHT_UP) |
    (1ULL << WIDX_BASE_HEIGHT_DOWN) |
    (1ULL << WIDX_WATER_LEVEL) |
    (1ULL << WIDX_WATER_LEVEL_UP) |
    (1ULL << WIDX_WATER_LEVEL_DOWN) |
    (1ULL << WIDX_FLOOR_TEXTURE) |
    (1ULL << WIDX_WALL_TEXTURE),

    (1ULL << WIDX_CLOSE) |
    (1ULL << WIDX_TAB_1) |
    (1ULL << WIDX_TAB_2) |
    (1ULL << WIDX_TAB_3) |
    (1ULL << WIDX_TAB_4) |
    (1ULL << WIDX_TREES_GENERATE) |
    (1ULL << WIDX_TREES_PLACE_TREES) |
    (1ULL << WIDX_TREES_HIGH) |
    (1ULL << WIDX_TREES_HIGH_UP) |
    (1ULL << WIDX_TREES_HIGH_DOWN) |
    (1ULL << WIDX_TREES_BASE_FREQ) |
    (1ULL << WIDX_TREES_BASE_FREQ_UP) |
    (1ULL << WIDX_TREES_BASE_FREQ_DOWN) |
    (1ULL << WIDX_TREES_OCTAVES) |
    (1ULL << WIDX_TREES_OCTAVES_UP) |
    (1ULL << WIDX_TREES_OCTAVES_DOWN),

    (1ULL << WIDX_CLOSE) |
    (1ULL << WIDX_TAB_1) |
    (1ULL << WIDX_TAB_2) |
    (1ULL << WIDX_TAB_3) |
    (1ULL << WIDX_TAB_4) |
    (1ULL << WIDX_SIMPLEX_GENERATE) |
    (1ULL << WIDX_SIMPLEX_LABEL) |
    (1ULL << WIDX_SIMPLEX_LOW) |
    (1ULL << WIDX_SIMPLEX_LOW_UP) |
    (1ULL << WIDX_SIMPLEX_LOW_DOWN) |
    (1ULL << WIDX_SIMPLEX_HIGH) |
    (1ULL << WIDX_SIMPLEX_HIGH_UP) |
    (1ULL << WIDX_SIMPLEX_HIGH_DOWN) |
    (1ULL << WIDX_SIMPLEX_BASE_FREQ) |
    (1ULL << WIDX_SIMPLEX_BASE_FREQ_UP) |
    (1ULL << WIDX_SIMPLEX_BASE_FREQ_DOWN) |
    (1ULL << WIDX_SIMPLEX_OCTAVES) |
    (1ULL << WIDX_SIMPLEX_OCTAVES_UP) |
    (1ULL << WIDX_SIMPLEX_OCTAVES_DOWN) |
    (1ULL << WIDX_SIMPLEX_MAP_SIZE) |
    (1ULL << WIDX_SIMPLEX_MAP_SIZE_UP) |
    (1ULL << WIDX_SIMPLEX_MAP_SIZE_DOWN) |
    (1ULL << WIDX_SIMPLEX_WATER_LEVEL) |
    (1ULL << WIDX_SIMPLEX_WATER_LEVEL_UP) |
    (1ULL << WIDX_SIMPLEX_WATER_LEVEL_DOWN) |
    (1ULL << WIDX_SIMPLEX_RANDOM_TERRAIN_CHECKBOX) |
    (1ULL << WIDX_SIMPLEX_FLOOR_TEXTURE) |
    (1ULL << WIDX_SIMPLEX_WALL_TEXTURE) |
    (1ULL << WIDX_SIMPLEX_PLACE_TREES_CHECKBOX),

    (1ULL << WIDX_CLOSE) |
    (1ULL << WIDX_TAB_1) |
    (1ULL << WIDX_TAB_2) |
    (1ULL << WIDX_TAB_3) |
    (1ULL << WIDX_TAB_4) |
    (1ULL << WIDX_HEIGHTMAP_SELECT)
};

static uint64 PageDisabledWidgets[WINDOW_MAPGEN_PAGE_COUNT] = {
    0,

    0,

    0,

    (1ULL << WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP) |
    (1ULL << WIDX_HEIGHTMAP_STRENGTH) |
    (1ULL << WIDX_HEIGHTMAP_STRENGTH_UP) |
    (1ULL << WIDX_HEIGHTMAP_STRENGTH_DOWN) |
    (1ULL << WIDX_HEIGHTMAP_NORMALIZE) |
    (1ULL << WIDX_HEIGHTMAP_SMOOTH_TILES) |
    (1ULL << WIDX_HEIGHTMAP_HIGH) |
    (1ULL << WIDX_HEIGHTMAP_HIGH_UP) |
    (1ULL << WIDX_HEIGHTMAP_HIGH_DOWN) |
    (1ULL << WIDX_HEIGHTMAP_LOW) |
    (1ULL << WIDX_HEIGHTMAP_LOW_UP) |
    (1ULL << WIDX_HEIGHTMAP_LOW_DOWN) |
    (1ULL << WIDX_HEIGHTMAP_WATER_LEVEL) |
    (1ULL << WIDX_HEIGHTMAP_WATER_LEVEL_UP) |
    (1ULL << WIDX_HEIGHTMAP_WATER_LEVEL_DOWN)
};

static uint64 HoldDownWidgets[WINDOW_MAPGEN_PAGE_COUNT] = {
    (1ULL << WIDX_MAP_SIZE_UP) |
    (1ULL << WIDX_MAP_SIZE_DOWN) |
    (1ULL << WIDX_BASE_HEIGHT_UP) |
    (1ULL << WIDX_BASE_HEIGHT_DOWN) |
    (1ULL << WIDX_WATER_LEVEL_UP) |
    (1ULL << WIDX_WATER_LEVEL_DOWN),

    (1ULL << WIDX_TREES_HIGH_UP) |
    (1ULL << WIDX_TREES_HIGH_DOWN) |
    (1ULL << WIDX_TREES_BASE_FREQ_UP) |
    (1ULL << WIDX_TREES_BASE_FREQ_DOWN) |
    (1ULL << WIDX_TREES_OCTAVES_UP) |
    (1ULL << WIDX_TREES_OCTAVES_DOWN),

    (1ULL << WIDX_SIMPLEX_LOW_UP) |
    (1ULL << WIDX_SIMPLEX_LOW_DOWN) |
    (1ULL << WIDX_SIMPLEX_HIGH_UP) |
    (1ULL << WIDX_SIMPLEX_HIGH_DOWN) |
    (1ULL << WIDX_SIMPLEX_BASE_FREQ_UP) |
    (1ULL << WIDX_SIMPLEX_BASE_FREQ_DOWN) |
    (1ULL << WIDX_SIMPLEX_OCTAVES_UP) |
    (1ULL << WIDX_SIMPLEX_OCTAVES_DOWN) |
    (1ULL << WIDX_SIMPLEX_MAP_SIZE_UP) |
    (1ULL << WIDX_SIMPLEX_MAP_SIZE_DOWN) |
    (1ULL << WIDX_SIMPLEX_WATER_LEVEL_UP) |
    (1ULL << WIDX_SIMPLEX_WATER_LEVEL_DOWN),

    (1ULL << WIDX_HEIGHTMAP_STRENGTH_UP) |
    (1ULL << WIDX_HEIGHTMAP_STRENGTH_DOWN) |
    (1ULL << WIDX_HEIGHTMAP_LOW_UP) |
    (1ULL << WIDX_HEIGHTMAP_LOW_DOWN) |
    (1ULL << WIDX_HEIGHTMAP_HIGH_UP) |
    (1ULL << WIDX_HEIGHTMAP_HIGH_DOWN) |
    (1ULL << WIDX_HEIGHTMAP_WATER_LEVEL_UP) |
    (1ULL << WIDX_HEIGHTMAP_WATER_LEVEL_DOWN)
};

static uint64 PressedWidgets[WINDOW_MAPGEN_PAGE_COUNT] = {
    0,
    0,
    0,
    (1ULL << WIDX_HEIGHTMAP_SMOOTH_TILES)
};

#pragma endregion

static const sint32 TabAnimationDivisor[WINDOW_MAPGEN_PAGE_COUNT] = {
    1, 1, 1, 1
};
static const sint32 TabAnimationFrames[WINDOW_MAPGEN_PAGE_COUNT] = {
    1, 1, 1, 1
};
static const sint32 TabAnimationLoops[WINDOW_MAPGEN_PAGE_COUNT] = {
    16, 16, 16, 0
};

#define BASESIZE_MIN 0
#define BASESIZE_MAX 60
#define WATERLEVEL_MIN 0
#define WATERLEVEL_MAX 54
#define MAX_SMOOTH_ITERATIONS 20

static void window_mapgen_set_page(rct_window *w, sint32 page);
static void window_mapgen_set_pressed_tab(rct_window *w);
static void window_mapgen_anchor_border_widgets(rct_window *w);
static void window_mapgen_draw_tab_images(rct_drawpixelinfo *dpi, rct_window *w);

static sint32 _mapSize = 150;
static sint32 _baseHeight = 12;
static sint32 _waterLevel = 6;
static sint32 _floorTexture = TERRAIN_GRASS;
static sint32 _wallTexture = TERRAIN_EDGE_ROCK;

static bool _randomTerrain = true;
static sint32 _placeTrees = 1;

static sint32 _trees_place_trees = 1;
static sint32 _trees_high = 10;
static sint32 _trees_base_freq = 60;
static sint32 _trees_octaves = 4;

static sint32 _simplex_low = 6;
static sint32 _simplex_high = 10;
static sint32 _simplex_base_freq = 60;
static sint32 _simplex_octaves = 4;

static bool _heightmapLoaded = false;
static bool _heightmapSmoothMap = false;
static sint32 _heightmapSmoothStrength = 1;
static bool _heightmapNormalize = false;
static bool _heightmapSmoothTiles = true;
static sint32 _heightmapLow = 2;
static sint32 _heightmapHigh = 70;

rct_window *window_mapgen_open()
{
    rct_window *w = window_bring_to_front_by_class(WC_MAPGEN);
    if (w != nullptr) {
        return w;
    }

    w = window_create_centred(
        WW,
        WH,
        PageEvents[WINDOW_MAPGEN_PAGE_BASE],
        WC_MAPGEN,
        WF_10
    );
    w->number = 0;
    w->frame_no = 0;

    w->page = WINDOW_MAPGEN_PAGE_BASE;
    window_invalidate(w);
    w->widgets = PageWidgets[WINDOW_MAPGEN_PAGE_BASE];
    w->enabled_widgets = PageEnabledWidgets[WINDOW_MAPGEN_PAGE_BASE];
    w->hold_down_widgets = HoldDownWidgets[WINDOW_MAPGEN_PAGE_BASE];
    w->event_handlers = PageEvents[WINDOW_MAPGEN_PAGE_BASE];
    w->pressed_widgets = PressedWidgets[WINDOW_MAPGEN_PAGE_BASE];
    w->disabled_widgets = PageDisabledWidgets[WINDOW_MAPGEN_PAGE_BASE];
    window_init_scroll_widgets(w);

    _heightmapLoaded = false;

    return w;
}

static void window_mapgen_shared_close(rct_window *w)
{
    mapgen_unload_heightmap();
}

static void window_mapgen_shared_mouseup(rct_window *w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex)
    {
    case WIDX_CLOSE:
        window_close(w);
        break;
    case WIDX_TAB_1:
    case WIDX_TAB_2:
    case WIDX_TAB_3:
    case WIDX_TAB_4:
        window_mapgen_set_page(w, widgetIndex - WIDX_TAB_1);
        break;
    }
}

#pragma region Base page

static void window_mapgen_base_mouseup(rct_window *w, rct_widgetindex widgetIndex)
{
    window_mapgen_shared_mouseup(w, widgetIndex);

    mapgen_settings mapgenSettings;

    switch (widgetIndex) {
    case WIDX_MAP_GENERATE:
        mapgenSettings.mapSize = _mapSize;
        mapgenSettings.height = _baseHeight + 2;
        mapgenSettings.water_level = _waterLevel + 2;
        mapgenSettings.floor = _floorTexture;
        mapgenSettings.wall = _wallTexture;

        mapgen_generate_blank(&mapgenSettings);
        gfx_invalidate_screen();
        break;
    case WIDX_MAP_SIZE:
        TextInputDescriptionArgs[0] = MINIMUM_MAP_SIZE_PRACTICAL;
        TextInputDescriptionArgs[1] = MAXIMUM_MAP_SIZE_PRACTICAL;
        // Practical map size is 2 lower than the technical map size
        window_text_input_open(w, WIDX_MAP_SIZE, STR_MAP_SIZE_2, STR_ENTER_MAP_SIZE, STR_FORMAT_INTEGER, _mapSize - 2, 4);
        break;
    case WIDX_BASE_HEIGHT:
        TextInputDescriptionArgs[0] = (BASESIZE_MIN - 12) / 2;
        TextInputDescriptionArgs[1] = (BASESIZE_MAX - 12) / 2;
        window_text_input_open(w, WIDX_BASE_HEIGHT, STR_BASE_HEIGHT, STR_ENTER_BASE_HEIGHT, STR_FORMAT_INTEGER, (_baseHeight - 12) / 2, 3);
        break;
    case WIDX_WATER_LEVEL:
        TextInputDescriptionArgs[0] = (WATERLEVEL_MIN - 12) / 2;
        TextInputDescriptionArgs[1] = (WATERLEVEL_MAX - 12) / 2;
        window_text_input_open(w, WIDX_WATER_LEVEL, STR_WATER_LEVEL, STR_ENTER_WATER_LEVEL, STR_FORMAT_INTEGER, (_waterLevel - 12) / 2, 3);
        break;
    }
}

static void window_mapgen_base_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    switch (widgetIndex) {
    case WIDX_MAP_SIZE_UP:
        _mapSize = Math::Min(_mapSize + 1, MAXIMUM_MAP_SIZE_TECHNICAL);
        window_invalidate(w);
        break;
    case WIDX_MAP_SIZE_DOWN:
        _mapSize = Math::Max(_mapSize - 1, MINIMUM_MAP_SIZE_TECHNICAL);
        window_invalidate(w);
        break;
    case WIDX_BASE_HEIGHT_UP:
        _baseHeight = Math::Min(_baseHeight + 2, BASESIZE_MAX);
        window_invalidate(w);
        break;
    case WIDX_BASE_HEIGHT_DOWN:
        _baseHeight = Math::Max(_baseHeight - 2, BASESIZE_MIN);
        window_invalidate(w);
        break;
    case WIDX_WATER_LEVEL_UP:
        _waterLevel = Math::Min(_waterLevel + 2, WATERLEVEL_MAX);
        window_invalidate(w);
        break;
    case WIDX_WATER_LEVEL_DOWN:
        _waterLevel = Math::Max(_waterLevel - 2, WATERLEVEL_MIN);
        window_invalidate(w);
        break;
    case WIDX_FLOOR_TEXTURE:
        land_tool_show_surface_style_dropdown(w, widget, _floorTexture);
        break;
    case WIDX_WALL_TEXTURE:
        land_tool_show_edge_style_dropdown(w, widget, _wallTexture);
        break;
    }
}

static void window_mapgen_base_dropdown(rct_window *w, rct_widgetindex widgetIndex, sint32 dropdownIndex)
{
    sint32 type;

    switch (widgetIndex) {
    case WIDX_FLOOR_TEXTURE:
        if (dropdownIndex == -1)
            dropdownIndex = gDropdownHighlightedIndex;

        type = (dropdownIndex == -1) ?
            _floorTexture :
            (uint32)gDropdownItemsArgs[dropdownIndex] - SPR_FLOOR_TEXTURE_GRASS;

        if (gLandToolTerrainSurface == type) {
            gLandToolTerrainSurface = 255;
        } else {
            gLandToolTerrainSurface = type;
            _floorTexture = type;
        }
        window_invalidate(w);
        break;
    case WIDX_WALL_TEXTURE:
        if (dropdownIndex == -1)
            dropdownIndex = gDropdownHighlightedIndex;

        type = (dropdownIndex == -1) ?
            _wallTexture :
            (uint32)gDropdownItemsArgs[dropdownIndex] - SPR_WALL_TEXTURE_ROCK;

        if (gLandToolTerrainEdge == type) {
            gLandToolTerrainEdge = 255;
        } else {
            gLandToolTerrainEdge = type;
            _wallTexture = type;
        }
        window_invalidate(w);
        break;
    }
}

static void window_mapgen_base_update(rct_window *w)
{
    // Tab animation
    if (++w->frame_no >= TabAnimationLoops[w->page])
        w->frame_no = 0;
    widget_invalidate(w, WIDX_TAB_1);
}

static void window_mapgen_textinput(rct_window *w, rct_widgetindex widgetIndex, char *text)
{
    sint32 value;
    char* end;

    if (text == nullptr)
        return;

    value = strtol(text, &end, 10);

    if (*end != '\0') {
        return;
    }

    switch (widgetIndex) {
    case WIDX_MAP_SIZE:
    case WIDX_SIMPLEX_MAP_SIZE:
        // The practical size is 2 lower than the technical size
        value += 2;
        _mapSize = Math::Clamp(MINIMUM_MAP_SIZE_TECHNICAL, value, MAXIMUM_MAP_SIZE_TECHNICAL);
        break;
    case WIDX_BASE_HEIGHT:
        _baseHeight = Math::Clamp(BASESIZE_MIN, (value * 2) + 12, BASESIZE_MAX);
        break;
    case WIDX_WATER_LEVEL:
        _waterLevel = Math::Clamp(WATERLEVEL_MIN, (value * 2) + 12, WATERLEVEL_MAX);
        break;
    }

    window_invalidate(w);

}

static void window_mapgen_base_invalidate(rct_window *w)
{
    if (w->widgets != PageWidgets[WINDOW_MAPGEN_PAGE_BASE]) {
        w->widgets = PageWidgets[WINDOW_MAPGEN_PAGE_BASE];
        window_init_scroll_widgets(w);
    }

    w->widgets[WIDX_FLOOR_TEXTURE].image = SPR_FLOOR_TEXTURE_GRASS + _floorTexture;
    w->widgets[WIDX_WALL_TEXTURE].image = SPR_WALL_TEXTURE_ROCK + _wallTexture;

    window_mapgen_set_pressed_tab(w);
}

static void window_mapgen_base_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
    uint16 arg;

    window_draw_widgets(w, dpi);
    window_mapgen_draw_tab_images(dpi, w);

    const uint8 textColour = w->colours[1];

    gfx_draw_string_left(dpi, STR_MAP_SIZE, nullptr, textColour, w->x + 4, w->y + w->widgets[WIDX_MAP_SIZE].top + 1);
    gfx_draw_string_left(dpi, STR_BASE_HEIGHT_LABEL, nullptr, textColour, w->x + 4, w->y + w->widgets[WIDX_BASE_HEIGHT].top + 1);
    gfx_draw_string_left(dpi, STR_WATER_LEVEL_LABEL, nullptr, textColour, w->x + 4, w->y + w->widgets[WIDX_WATER_LEVEL].top + 1);
    gfx_draw_string_left(dpi, STR_TERRAIN_LABEL, nullptr, textColour, w->x + 4, w->y + w->widgets[WIDX_FLOOR_TEXTURE].top + 1);

    // The practical map size is 2 lower than the technical map size
    rct_xy16 mapSizeArgs = MakeXY16(_mapSize - 2, _mapSize -2);
    gfx_draw_string_left(dpi, STR_RESOLUTION_X_BY_Y, &mapSizeArgs, w->colours[1], w->x + w->widgets[WIDX_MAP_SIZE].left + 1, w->y + w->widgets[WIDX_MAP_SIZE].top + 1);

    arg = (_baseHeight - 12) / 2;
    gfx_draw_string_left(dpi, STR_COMMA16, &arg, w->colours[1], w->x + w->widgets[WIDX_BASE_HEIGHT].left + 1, w->y + w->widgets[WIDX_BASE_HEIGHT].top + 1);

    arg = (_waterLevel - 12) / 2;
    gfx_draw_string_left(dpi, STR_COMMA16, &arg, w->colours[1], w->x + w->widgets[WIDX_WATER_LEVEL].left + 1, w->y + w->widgets[WIDX_WATER_LEVEL].top + 1);
}

#pragma endregion

#pragma region Trees page

static void window_mapgen_trees_mouseup(rct_window *w, rct_widgetindex widgetIndex)
{
    window_mapgen_shared_mouseup(w, widgetIndex);

    mapgen_settings mapgenSettings;

    switch (widgetIndex) {
    case WIDX_TREES_GENERATE:
        mapgenSettings.mapSize = _mapSize;

        mapgenSettings.height = _baseHeight;
        mapgenSettings.water_level = _waterLevel + 2;
        mapgenSettings.floor = _randomTerrain ? -1 : _floorTexture;
        mapgenSettings.wall = _randomTerrain ? -1 : _wallTexture;
        mapgenSettings.trees = _trees_place_trees ? false : _placeTrees; // For now, use old version if new version isn't enabled

        mapgenSettings.simplex_low = _simplex_low;
        mapgenSettings.simplex_high = _simplex_high;
        mapgenSettings.simplex_base_freq = ((float)_simplex_base_freq) / 100.00f;
        mapgenSettings.simplex_octaves = _simplex_octaves;

        mapgenSettings.trees_place = _trees_place_trees;
        mapgenSettings.trees_amplitude = _trees_high;
        mapgenSettings.trees_frequency = ((float)_trees_base_freq) / 100.00f;
        mapgenSettings.trees_octaves = _trees_octaves;
        mapgenSettings.trees_threshold = 0.5f;
        mapgenSettings.trees_lacunarity = 2.0f;
        mapgenSettings.trees_persistance = 0.65f;

        mapgen_generate(&mapgenSettings);
        gfx_invalidate_screen();
        break;
    }
}

static void window_mapgen_trees_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    switch (widgetIndex) {
    case WIDX_TREES_HIGH_UP:
        _trees_high = Math::Min(_trees_high + 1, 36);
        window_invalidate(w);
        break;
    case WIDX_TREES_HIGH_DOWN:
        _trees_high = Math::Max(_trees_high - 1, 0);
        window_invalidate(w);
        break;
    case WIDX_TREES_BASE_FREQ_UP:
        _trees_base_freq = Math::Min(_trees_base_freq + 5, 1000);
        window_invalidate(w);
        break;
    case WIDX_TREES_BASE_FREQ_DOWN:
        _trees_base_freq = Math::Max(_trees_base_freq - 5, 0);
        window_invalidate(w);
        break;
    case WIDX_TREES_OCTAVES_UP:
        _trees_octaves = Math::Min(_trees_octaves + 1, 10);
        window_invalidate(w);
        break;
    case WIDX_TREES_OCTAVES_DOWN:
        _trees_octaves = Math::Max(_trees_octaves - 1, 1);
        window_invalidate(w);
        break;
    case WIDX_TREES_PLACE_TREES:
        _trees_place_trees ^= 1;
        window_invalidate(w);
        break;
    }
}

static void window_mapgen_trees_update(rct_window *w)
{
    // Tab animation
    if (++w->frame_no >= TabAnimationLoops[w->page])
        w->frame_no = 0;
    widget_invalidate(w, WIDX_TAB_2);
}

static void window_mapgen_trees_invalidate(rct_window *w)
{
    if (w->widgets != PageWidgets[WINDOW_MAPGEN_PAGE_TREES]) {
        w->widgets = PageWidgets[WINDOW_MAPGEN_PAGE_TREES];
        window_init_scroll_widgets(w);
    }

    widget_set_checkbox_value(w, WIDX_TREES_PLACE_TREES, _trees_place_trees != 0);

    window_mapgen_set_pressed_tab(w);
}

static void window_mapgen_trees_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
    window_draw_widgets(w, dpi);
    window_mapgen_draw_tab_images(dpi, w);

    const uint8 textColour = w->colours[1];

    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_HIGH, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_TREES_HIGH].top + 1);
    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_BASE_FREQUENCY, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_TREES_BASE_FREQ].top + 1);
    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_OCTAVES, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_TREES_OCTAVES].top + 1);
    
    gfx_draw_string_left(dpi, STR_COMMA16, &_trees_high, textColour, w->x + w->widgets[WIDX_TREES_HIGH].left + 1, w->y + w->widgets[WIDX_TREES_HIGH].top + 1);
    gfx_draw_string_left(dpi, STR_WINDOW_OBJECTIVE_VALUE_RATING, &_trees_base_freq, textColour, w->x + w->widgets[WIDX_TREES_BASE_FREQ].left + 1, w->y + w->widgets[WIDX_TREES_BASE_FREQ].top + 1);
    gfx_draw_string_left(dpi, STR_COMMA16, &_trees_octaves, textColour, w->x + w->widgets[WIDX_TREES_OCTAVES].left + 1, w->y + w->widgets[WIDX_TREES_OCTAVES].top + 1);

}

#pragma endregion

#pragma region Simplex page

static void window_mapgen_simplex_mouseup(rct_window *w, rct_widgetindex widgetIndex)
{
    window_mapgen_shared_mouseup(w, widgetIndex);

    mapgen_settings mapgenSettings;

    switch (widgetIndex) {
    case WIDX_SIMPLEX_MAP_SIZE:
        TextInputDescriptionArgs[0] = MINIMUM_MAP_SIZE_PRACTICAL;
        TextInputDescriptionArgs[1] = MAXIMUM_MAP_SIZE_PRACTICAL;
        // Practical map size is 2 lower than the technical map size
        window_text_input_open(w, WIDX_SIMPLEX_MAP_SIZE, STR_MAP_SIZE_2, STR_ENTER_MAP_SIZE, STR_FORMAT_INTEGER, _mapSize - 2, 4);
        break;
    case WIDX_SIMPLEX_GENERATE:
        mapgenSettings.mapSize = _mapSize;

        mapgenSettings.height = _baseHeight;
        mapgenSettings.water_level = _waterLevel + 2;
        mapgenSettings.floor = _randomTerrain ? -1 : _floorTexture;
        mapgenSettings.wall = _randomTerrain ? -1 : _wallTexture;
        // Don't use old tree generation if the simplex trees is enabled.
        mapgenSettings.trees = _trees_place_trees ? false : _placeTrees; 
        mapgenSettings.simplex_low = _simplex_low;
        mapgenSettings.simplex_high = _simplex_high;
        mapgenSettings.simplex_base_freq = ((float)_simplex_base_freq) / 100.00f;
        mapgenSettings.simplex_octaves = _simplex_octaves;

        mapgenSettings.trees_place = _trees_place_trees;
        mapgenSettings.trees_amplitude = _trees_high;
        mapgenSettings.trees_frequency = ((float)_trees_base_freq) / 100.00f;
        mapgenSettings.trees_octaves = _trees_octaves;
        mapgenSettings.trees_threshold = 0.5f;
        mapgenSettings.trees_lacunarity = 2.0f;
        mapgenSettings.trees_persistance = 0.65f;

        mapgen_generate(&mapgenSettings);
        gfx_invalidate_screen();
        break;
    }
}

static void window_mapgen_simplex_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    switch (widgetIndex) {
    case WIDX_SIMPLEX_LOW_UP:
        _simplex_low = Math::Min(_simplex_low + 1, 24);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_LOW_DOWN:
        _simplex_low = Math::Max(_simplex_low - 1, 0);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_HIGH_UP:
        _simplex_high = Math::Min(_simplex_high + 1, 36);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_HIGH_DOWN:
        _simplex_high = Math::Max(_simplex_high - 1, 0);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_BASE_FREQ_UP:
        _simplex_base_freq = Math::Min(_simplex_base_freq + 5, 1000);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_BASE_FREQ_DOWN:
        _simplex_base_freq = Math::Max(_simplex_base_freq - 5, 0);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_OCTAVES_UP:
        _simplex_octaves = Math::Min(_simplex_octaves + 1, 10);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_OCTAVES_DOWN:
        _simplex_octaves = Math::Max(_simplex_octaves - 1, 1);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_MAP_SIZE_UP:
        _mapSize = Math::Min(_mapSize + 1, MAXIMUM_MAP_SIZE_TECHNICAL);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_MAP_SIZE_DOWN:
        _mapSize = Math::Max(_mapSize - 1, MINIMUM_MAP_SIZE_TECHNICAL);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_WATER_LEVEL_UP:
        _waterLevel = Math::Min(_waterLevel + 2, 54);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_WATER_LEVEL_DOWN:
        _waterLevel = Math::Max(_waterLevel - 2, 0);
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_RANDOM_TERRAIN_CHECKBOX:
        _randomTerrain = !_randomTerrain;
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_FLOOR_TEXTURE:
        land_tool_show_surface_style_dropdown(w, widget, _floorTexture);
        break;
    case WIDX_SIMPLEX_WALL_TEXTURE:
        land_tool_show_edge_style_dropdown(w, widget, _wallTexture);
        break;
    case WIDX_SIMPLEX_PLACE_TREES_CHECKBOX:
        _placeTrees ^= 1;
        window_invalidate(w);
        break;
    }
}

static void window_mapgen_simplex_dropdown(rct_window *w, rct_widgetindex widgetIndex, sint32 dropdownIndex)
{
    sint32 type;

    switch (widgetIndex) {
    case WIDX_SIMPLEX_FLOOR_TEXTURE:
        if (dropdownIndex == -1)
            dropdownIndex = gDropdownHighlightedIndex;

        type = (dropdownIndex == -1) ?
        _floorTexture :
                      (uint32)gDropdownItemsArgs[dropdownIndex] - SPR_FLOOR_TEXTURE_GRASS;

        if (gLandToolTerrainSurface == type) {
            gLandToolTerrainSurface = 255;
        }
        else {
            gLandToolTerrainSurface = type;
            _floorTexture = type;
        }
        window_invalidate(w);
        break;
    case WIDX_SIMPLEX_WALL_TEXTURE:
        if (dropdownIndex == -1)
            dropdownIndex = gDropdownHighlightedIndex;

        type = (dropdownIndex == -1) ?
        _wallTexture :
                     (uint32)gDropdownItemsArgs[dropdownIndex] - SPR_WALL_TEXTURE_ROCK;

        if (gLandToolTerrainEdge == type) {
            gLandToolTerrainEdge = 255;
        }
        else {
            gLandToolTerrainEdge = type;
            _wallTexture = type;
        }
        window_invalidate(w);
        break;
    }
}

static void window_mapgen_simplex_update(rct_window *w)
{
    // Tab animation
    if (++w->frame_no >= TabAnimationLoops[w->page])
        w->frame_no = 0;
    widget_invalidate(w, WIDX_TAB_3);
}

static void window_mapgen_simplex_invalidate(rct_window *w)
{
    if (w->widgets != PageWidgets[WINDOW_MAPGEN_PAGE_SIMPLEX]) {
        w->widgets = PageWidgets[WINDOW_MAPGEN_PAGE_SIMPLEX];
        window_init_scroll_widgets(w);
    }

    w->widgets[WIDX_SIMPLEX_FLOOR_TEXTURE].image = SPR_FLOOR_TEXTURE_GRASS + _floorTexture;
    w->widgets[WIDX_SIMPLEX_WALL_TEXTURE].image = SPR_WALL_TEXTURE_ROCK + _wallTexture;
    
    widget_set_checkbox_value(w, WIDX_SIMPLEX_RANDOM_TERRAIN_CHECKBOX, _randomTerrain != 0);
    widget_set_checkbox_value(w, WIDX_SIMPLEX_PLACE_TREES_CHECKBOX, _placeTrees != 0);

    // Only allow floor and wall texture options if random terrain is disabled
    if (!_randomTerrain) {
        widget_set_enabled(w, WIDX_SIMPLEX_FLOOR_TEXTURE, true);
        widget_set_enabled(w, WIDX_SIMPLEX_WALL_TEXTURE, true);
    }
    else {
        widget_set_enabled(w, WIDX_SIMPLEX_FLOOR_TEXTURE, false);
        widget_set_enabled(w, WIDX_SIMPLEX_WALL_TEXTURE, false);
    }

    window_mapgen_set_pressed_tab(w);
}

static void window_mapgen_simplex_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
    uint16 arg;

    window_draw_widgets(w, dpi);
    window_mapgen_draw_tab_images(dpi, w);

    const uint8 textColour = w->colours[1];

    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_LOW_, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_LOW].top + 1);
    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_HIGH, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_HIGH].top + 1);
    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_BASE_FREQUENCY, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_BASE_FREQ].top + 1);
    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_OCTAVES, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_OCTAVES].top + 1);
    gfx_draw_string_left(dpi, STR_MAP_SIZE, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_MAP_SIZE].top + 1);
    gfx_draw_string_left(dpi, STR_WATER_LEVEL_LABEL, nullptr, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_WATER_LEVEL].top + 1);

    gfx_draw_string_left(dpi, STR_COMMA16, &_simplex_low, textColour, w->x + w->widgets[WIDX_SIMPLEX_LOW].left + 1, w->y + w->widgets[WIDX_SIMPLEX_LOW].top + 1);
    gfx_draw_string_left(dpi, STR_COMMA16, &_simplex_high, textColour, w->x + w->widgets[WIDX_SIMPLEX_HIGH].left + 1, w->y + w->widgets[WIDX_SIMPLEX_HIGH].top + 1);
    gfx_draw_string_left(dpi, STR_WINDOW_OBJECTIVE_VALUE_RATING, &_simplex_base_freq, textColour, w->x + w->widgets[WIDX_SIMPLEX_BASE_FREQ].left + 1, w->y + w->widgets[WIDX_SIMPLEX_BASE_FREQ].top + 1);
    gfx_draw_string_left(dpi, STR_COMMA16, &_simplex_octaves, textColour, w->x + w->widgets[WIDX_SIMPLEX_OCTAVES].left + 1, w->y + w->widgets[WIDX_SIMPLEX_OCTAVES].top + 1);
    gfx_draw_string_left(dpi, STR_TERRAIN_LABEL, NULL, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_RANDOM_TERRAIN_CHECKBOX].top + 1);
    gfx_draw_string_left(dpi, STR_MAPGEN_OPTION_PLACE_TREES, NULL, textColour, w->x + 5, w->y + w->widgets[WIDX_SIMPLEX_PLACE_TREES_CHECKBOX].top + 1);

    // The practical map size is 2 lower than the technical map size
    rct_xy16 mapSizeArgs = MakeXY16(_mapSize - 2, _mapSize - 2);
    gfx_draw_string_left(dpi, STR_RESOLUTION_X_BY_Y, &mapSizeArgs, textColour, w->x + w->widgets[WIDX_SIMPLEX_MAP_SIZE].left + 1, w->y + w->widgets[WIDX_SIMPLEX_MAP_SIZE].top + 1);

    arg = (_waterLevel - 12) / 2;
    gfx_draw_string_left(dpi, STR_COMMA16, &arg, textColour, w->x + w->widgets[WIDX_SIMPLEX_WATER_LEVEL].left + 1, w->y + w->widgets[WIDX_SIMPLEX_WATER_LEVEL].top + 1);
}

#pragma endregion

#pragma region Heightmap page

static void window_mapgen_heightmap_mousedown(rct_window *w, rct_widgetindex widgetIndex, rct_widget* widget)
{
    switch (widgetIndex)
    {
    case WIDX_HEIGHTMAP_STRENGTH_UP:
        _heightmapSmoothStrength = Math::Min(_heightmapSmoothStrength + 1, MAX_SMOOTH_ITERATIONS);
        widget_invalidate(w, WIDX_HEIGHTMAP_STRENGTH);
        break;
    case WIDX_HEIGHTMAP_STRENGTH_DOWN:
        _heightmapSmoothStrength = Math::Max(_heightmapSmoothStrength - 1, 1);
        widget_invalidate(w, WIDX_HEIGHTMAP_STRENGTH);
        break;
    case WIDX_HEIGHTMAP_LOW_UP:
        _heightmapLow = Math::Min(_heightmapLow + 1, 142 - 1);
        _heightmapHigh = Math::Max(_heightmapHigh, _heightmapLow + 1);
        widget_invalidate(w, WIDX_HEIGHTMAP_LOW);
        break;
    case WIDX_HEIGHTMAP_LOW_DOWN:
        _heightmapLow = Math::Max(_heightmapLow - 1, 2);
        widget_invalidate(w, WIDX_HEIGHTMAP_LOW);
        break;
    case WIDX_HEIGHTMAP_HIGH_UP:
        _heightmapHigh = Math::Min(_heightmapHigh + 1, 142);
        widget_invalidate(w, WIDX_HEIGHTMAP_HIGH);
        break;
    case WIDX_HEIGHTMAP_HIGH_DOWN:
        _heightmapHigh = Math::Max(_heightmapHigh - 1, 2 + 1);
        _heightmapLow = Math::Min(_heightmapLow, _heightmapHigh - 1);
        widget_invalidate(w, WIDX_HEIGHTMAP_HIGH);
        break;
    case WIDX_HEIGHTMAP_WATER_LEVEL_UP:
        _waterLevel = Math::Min(_waterLevel + 2, 54);
        widget_invalidate(w, WIDX_HEIGHTMAP_WATER_LEVEL);
        break;
    case WIDX_HEIGHTMAP_WATER_LEVEL_DOWN:
        _waterLevel = Math::Max(_waterLevel - 2, 0);
        widget_invalidate(w, WIDX_HEIGHTMAP_WATER_LEVEL);
        break;
    }
}

static void window_mapgen_heightmap_generate_map()
{
    mapgen_settings mapgenSettings;
    mapgenSettings.water_level = _waterLevel;
    mapgenSettings.smooth = _heightmapSmoothTiles;
    mapgenSettings.smooth_height_map = _heightmapSmoothMap;
    mapgenSettings.smooth_strength = _heightmapSmoothStrength;
    mapgenSettings.normalize_height = _heightmapNormalize;
    mapgenSettings.simplex_low = _heightmapLow;
    mapgenSettings.simplex_high = _heightmapHigh;
    mapgen_generate_from_heightmap(&mapgenSettings);
    gfx_invalidate_screen();
}

static void window_mapgen_heightmap_loadsave_callback(sint32 result, const utf8 *path)
{
    if (result == MODAL_RESULT_OK) {
        if (!mapgen_load_heightmap(path)) {
            // TODO: Display error popup
            return;
        }

        // The window needs to be open while using the map
        rct_window *const w = context_open_window(WC_MAPGEN);
        _heightmapLoaded = true;
        window_mapgen_set_page(w, WINDOW_MAPGEN_PAGE_HEIGHTMAP);

        window_mapgen_heightmap_generate_map();
    }
}

static void window_mapgen_heightmap_mouseup(rct_window *w, rct_widgetindex widgetIndex)
{
    window_mapgen_shared_mouseup(w, widgetIndex);

    switch (widgetIndex)
    {
    case WIDX_CLOSE:
    case WIDX_TAB_1:
    case WIDX_TAB_2:
    case WIDX_TAB_3:
    case WIDX_TAB_4:
        return; // Only widgets that change a setting need to regenerate the map

        // Page widgets
    case WIDX_HEIGHTMAP_SELECT:
    {
        auto intent = Intent(WC_LOADSAVE);
        intent.putExtra(INTENT_EXTRA_LOADSAVE_TYPE, LOADSAVETYPE_LOAD | LOADSAVETYPE_IMAGE);
        intent.putExtra(INTENT_EXTRA_CALLBACK, (void *) window_mapgen_heightmap_loadsave_callback);
        context_open_intent(&intent);
        return;
    }
    case WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP:
        _heightmapSmoothMap = !_heightmapSmoothMap;
        widget_set_checkbox_value(w, WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP, _heightmapSmoothMap);
        widget_set_enabled(w, WIDX_HEIGHTMAP_STRENGTH, _heightmapSmoothMap);
        widget_set_enabled(w, WIDX_HEIGHTMAP_STRENGTH_UP, _heightmapSmoothMap);
        widget_set_enabled(w, WIDX_HEIGHTMAP_STRENGTH_DOWN, _heightmapSmoothMap);
        widget_invalidate(w, WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP);
        widget_invalidate(w, WIDX_HEIGHTMAP_STRENGTH);
        break;
    case WIDX_HEIGHTMAP_NORMALIZE:
        _heightmapNormalize = !_heightmapNormalize;
        widget_set_checkbox_value(w, WIDX_HEIGHTMAP_NORMALIZE, _heightmapNormalize);
        widget_invalidate(w, WIDX_HEIGHTMAP_NORMALIZE);
        break;
    case WIDX_HEIGHTMAP_SMOOTH_TILES:
        _heightmapSmoothTiles = !_heightmapSmoothTiles;
        widget_set_checkbox_value(w, WIDX_HEIGHTMAP_SMOOTH_TILES, _heightmapSmoothTiles);
        widget_invalidate(w, WIDX_HEIGHTMAP_SMOOTH_TILES);
        break;
    }

    // Always regenerate the map after one of the page widgets has been changed
    window_mapgen_heightmap_generate_map();
}

static void window_mapgen_heightmap_invalidate(rct_window *w)
{
    if (w->widgets != PageWidgets[WINDOW_MAPGEN_PAGE_HEIGHTMAP])
    {
        w->widgets = PageWidgets[WINDOW_MAPGEN_PAGE_HEIGHTMAP];
        window_init_scroll_widgets(w);
    }

    widget_set_checkbox_value(w, WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP, _heightmapSmoothMap);
    widget_set_checkbox_value(w, WIDX_HEIGHTMAP_NORMALIZE, _heightmapNormalize);
    widget_set_checkbox_value(w, WIDX_HEIGHTMAP_SMOOTH_TILES, _heightmapSmoothTiles);

    window_mapgen_set_pressed_tab(w);
}

static void window_mapgen_heightmap_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
    window_draw_widgets(w, dpi);
    window_mapgen_draw_tab_images(dpi, w);

    const uint8 enabledColour = w->colours[1];
    const uint8 disabledColour = enabledColour | COLOUR_FLAG_INSET;

    // Smooth strength label and value
    const uint8 strengthColour = _heightmapSmoothMap ? enabledColour : disabledColour;
    sint16 strength = _heightmapSmoothStrength;
    gfx_draw_string_left(dpi, STR_MAPGEN_SMOOTH_STRENGTH, nullptr, strengthColour, w->x + 5, w->y + w->widgets[WIDX_HEIGHTMAP_STRENGTH].top + 1);
    gfx_draw_string_left(dpi, STR_COMMA16, &strength, strengthColour, w->x + w->widgets[WIDX_HEIGHTMAP_STRENGTH].left + 1, w->y + w->widgets[WIDX_HEIGHTMAP_STRENGTH].top + 1);

    // Low label and value
    const uint8 labelColour = _heightmapLoaded ? enabledColour : disabledColour;
    sint16 low = _heightmapLow;
    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_LOW_, nullptr, labelColour, w->x + 5, w->y + w->widgets[WIDX_HEIGHTMAP_LOW].top + 1);
    gfx_draw_string_left(dpi, STR_COMMA16, &low, labelColour, w->x + w->widgets[WIDX_HEIGHTMAP_LOW].left + 1, w->y + w->widgets[WIDX_HEIGHTMAP_LOW].top + 1);

    // High label and value
    sint16 high = _heightmapHigh;
    gfx_draw_string_left(dpi, STR_MAPGEN_SIMPLEX_NOISE_HIGH, nullptr, labelColour, w->x + 5, w->y + w->widgets[WIDX_HEIGHTMAP_HIGH].top + 1);
    gfx_draw_string_left(dpi, STR_COMMA16, &high, labelColour, w->x + w->widgets[WIDX_HEIGHTMAP_HIGH].left + 1, w->y + w->widgets[WIDX_HEIGHTMAP_HIGH].top + 1);

    // Water level label and value
    sint16 waterLevel = _waterLevel;
    gfx_draw_string_left(dpi, STR_WATER_LEVEL_LABEL, nullptr, labelColour, w->x + 5, w->y + w->widgets[WIDX_HEIGHTMAP_WATER_LEVEL].top + 1);
    gfx_draw_string_left(dpi, STR_COMMA16, &waterLevel, labelColour, w->x + w->widgets[WIDX_HEIGHTMAP_WATER_LEVEL].left + 1, w->y + w->widgets[WIDX_HEIGHTMAP_WATER_LEVEL].top + 1);
}

#pragma endregion

#pragma region Common

static void window_mapgen_set_page(rct_window *w, sint32 page)
{
    w->page = page;
    w->frame_no = 0;
    if (w->viewport != nullptr) {
        w->viewport->width = 0;
        w->viewport = nullptr;
    }

    w->enabled_widgets = PageEnabledWidgets[page];
    w->hold_down_widgets = HoldDownWidgets[page];
    w->event_handlers = PageEvents[page];
    w->widgets = PageWidgets[page];
    w->disabled_widgets = PageDisabledWidgets[page];
    w->pressed_widgets = PressedWidgets[page];

    // Enable heightmap widgets if one is loaded
    if (page == WINDOW_MAPGEN_PAGE_HEIGHTMAP && _heightmapLoaded)
    {
        widget_set_enabled(w, WIDX_HEIGHTMAP_SMOOTH_HEIGHTMAP, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_STRENGTH, _heightmapSmoothMap);
        widget_set_enabled(w, WIDX_HEIGHTMAP_STRENGTH_UP, _heightmapSmoothMap);
        widget_set_enabled(w, WIDX_HEIGHTMAP_STRENGTH_DOWN, _heightmapSmoothMap);
        widget_set_enabled(w, WIDX_HEIGHTMAP_NORMALIZE, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_SMOOTH_TILES, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_HIGH, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_HIGH_UP, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_HIGH_DOWN, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_LOW, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_LOW_UP, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_LOW_DOWN, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_WATER_LEVEL, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_WATER_LEVEL_UP, true);
        widget_set_enabled(w, WIDX_HEIGHTMAP_WATER_LEVEL_DOWN, true);
    }

    window_init_scroll_widgets(w);
    window_invalidate(w);
}

static void window_mapgen_set_pressed_tab(rct_window *w)
{
    sint32 i;
    for (i = 0; i < WINDOW_MAPGEN_PAGE_COUNT; i++)
        w->pressed_widgets &= ~(1 << (WIDX_TAB_1 + i));
    w->pressed_widgets |= 1LL << (WIDX_TAB_1 + w->page);
}

static void window_mapgen_draw_tab_image(rct_drawpixelinfo *dpi, rct_window *w, sint32 page, sint32 spriteIndex)
{
    rct_widgetindex widgetIndex = WIDX_TAB_1 + page;

    if (!(w->disabled_widgets & (1LL << widgetIndex))) {
        if (w->page == page) {
            sint32 frame = w->frame_no / TabAnimationDivisor[w->page];
            spriteIndex += (frame % TabAnimationFrames[w->page]);
        }

        gfx_draw_sprite(dpi, spriteIndex, w->x + w->widgets[widgetIndex].left, w->y + w->widgets[widgetIndex].top, 0);
    }
}

static void window_mapgen_draw_tab_images(rct_drawpixelinfo *dpi, rct_window *w)
{
    window_mapgen_draw_tab_image(dpi, w, WINDOW_MAPGEN_PAGE_BASE, SPR_G2_TAB_LAND);
    window_mapgen_draw_tab_image(dpi, w, WINDOW_MAPGEN_PAGE_TREES, SPR_G2_TAB_TREE);
    window_mapgen_draw_tab_image(dpi, w, WINDOW_MAPGEN_PAGE_SIMPLEX, SPR_G2_TAB_PENCIL);
    window_mapgen_draw_tab_image(dpi, w, WINDOW_MAPGEN_PAGE_HEIGHTMAP, SPR_TAB_GRAPH_0);
}

#pragma endregion
