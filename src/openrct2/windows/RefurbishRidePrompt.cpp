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

extern "C" {
    #include "../game.h"
    #include "../interface/widget.h"
    #include "../localisation/localisation.h"
}

#define WW 200
#define WH 100

money32 gRefurbishRideCost;

enum WINDOW_RIDE_REFURBISH_WIDGET_IDX {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_REFURBISH,
    WIDX_CANCEL
};

// 0x009AEBA0
static rct_widget window_ride_refurbish_widgets[] = {
    { WWT_FRAME,            0, 0,       WW - 1,     0,          WH - 1, STR_NONE,               STR_NONE },
    { WWT_CAPTION,          0, 1,       WW - 2,     1,          14,     STR_REFURBISH_RIDE,     STR_WINDOW_TITLE_TIP },
    { WWT_CLOSEBOX,         0, WW - 13, WW - 3,     2,          13,     STR_CLOSE_X,            STR_CLOSE_WINDOW_TIP },
    { WWT_DROPDOWN_BUTTON,  0, 10,      94,         WH - 20,    WH - 9, STR_REFURBISH,          STR_NONE },
    { WWT_DROPDOWN_BUTTON,  0, WW - 95, WW - 11,    WH - 20,    WH - 9, STR_SAVE_PROMPT_CANCEL, STR_NONE },
    { WIDGETS_END }
};

static void window_ride_refurbish_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_ride_refurbish_paint(rct_window *w, rct_drawpixelinfo *dpi);

//0x0098E2E4
static rct_window_event_list window_ride_refurbish_events = {
    nullptr,
    window_ride_refurbish_mouseup,
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
    nullptr,
    nullptr,
    nullptr,
    window_ride_refubish_paint,
    nullptr
};

/** Based off of rct2: 0x006B486A */
static void _window_ride_refurbish_prompt_open(sint32 rideIndex)
{
    rct_window *w;

    w = window_bring_to_front_by_number(WC_REFURBISH_RIDE_PROMPT, rideIndex);
    if (w != nullptr)
        return;

    w = window_create_centred(WW, WH, &window_ride_refurbish_events, WC_REFURBISH_RIDE_PROMPT, WF_TRANSPARENT);
    w->widgets = window_ride_refurbish_mouseup();
    w->enabled_widgets = (1 << WIDX_CLOSE) | (1 << WIDX_CANCEL) | (1 << WIDX_REFURBISH);
    window_init_scroll_widgets(w);
    w->number = rideIndex;
}


/**
*
*  rct2: 0x006B4933
*/
static void window_ride_refurbish_mouseup(rct_window *w, rct_widgetindex widgetIndex)
{
    rct_window* window;
    switch (widgetIndex) {
    case WIDX_REFURBISH:
        gGameCommandErrorTitle = STR_CANT_REFURBISH_RIDE;
        game_do_command(0, GAME_COMMAND_FLAG_APPLY, RIDE_MODIFY_RENEW, w->number, GAME_COMMAND_DEMOLISH_RIDE, 0, 1); // Set ebp to 1 to be used to log refurbish from window prompt

        // Prevents old ride info sticking around in the ride list window
        window = window_find_by_class(WC_RIDE_LIST);
        if (window != nullptr)
        {
            window_ride_list_refresh_list(window);
        }

        break;
    case WIDX_CANCEL:
    case WIDX_CLOSE:
        window_close(w);
        break;
    }
}

/**
*
*  rct2: 0x006B48E5
*/
static void window_ride_refurbish_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
    window_draw_widgets(w, dpi);

    rct_ride* ride = get_ride(w->number);

    set_format_arg(0, rct_string_id, ride->name);
    set_format_arg(2, uint32, ride->name_arguments);
    set_format_arg(6, uint32, gRefurbishRideCost);

    sint32 x = w->x + WW / 2;
    sint32 y = w->y + (WH / 2) - 3;

    rct_string_id stringId;
    if (gParkFlags & PARK_FLAGS_NO_MONEY) {
        stringId = STR_REFURBISH_RIDE_ID_NO_MONEY;
    }
    else {
        stringId = STR_REFURBISH_RIDE_ID_MONEY;
    }

    gfx_draw_string_centred_wrapped(dpi, gCommonFormatArgs, x, y, WW - 4, stringId, COLOUR_BLACK);
}

extern "C" {
    void window_ride_refurbish_prompt_open(sint32 rideIndex)
    {
        _window_ride_refurbish_prompt_open(rideIndex);
    }
}
