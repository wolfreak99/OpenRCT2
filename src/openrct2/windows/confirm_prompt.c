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

#include "../game.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "confirm_prompt.h"

#define WW 250
#define WH 100
#define BUTTON_OFFSET 10
#define BUTTON_SPACE 4

enum WINDOW_CONFIRM_PROMPT_WIDGET_IDX {
    WIDX_BACKGROUND,
    WIDX_TITLE,
    WIDX_CLOSE,
    WIDX_YES,
    WIDX_NO,
    WIDX_CANCEL
};

// 0x9AFB4C
static rct_widget window_confirm_prompt_widgets[] = {
    { WWT_FRAME,            0,  0,          WW - 1,     0,          WH - 1,     STR_NONE,               STR_NONE },
    { WWT_CAPTION,          0,  1,          WW - 2,     1,          14,         STR_NONE,               STR_WINDOW_TITLE_TIP },
    { WWT_CLOSEBOX,         0,  WW - 13,    WW - 3,     2,          13,         STR_CLOSE_X,            STR_CLOSE_WINDOW_TIP },
    { WWT_DROPDOWN_BUTTON,  0,  10,         94,         WH - 20,    WH - 9,     STR_YES,                STR_NONE },
    { WWT_DROPDOWN_BUTTON,  0,  0,          0,          WH - 20,    WH - 9,     STR_NONE,               STR_NONE },
    { WWT_DROPDOWN_BUTTON,  0,  WW - 95,    WW - 11,    WH - 20,    WH - 9,     STR_CANCEL,             STR_NONE },
    { WIDGETS_END }
};

static void window_confirm_prompt_close(rct_window *w);
static void window_confirm_prompt_mouseup(rct_window *w, rct_widgetindex widgetIndex);
static void window_confirm_prompt_paint(rct_window *w, rct_drawpixelinfo *dpi);

static rct_window_event_list window_confirm_prompt_events = {
    window_confirm_prompt_close,
    window_confirm_prompt_mouseup,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    window_confirm_prompt_paint,
    NULL
};

static void window_confirm_prompt_update_button_size(bool useThreeButtons);

rct_string_id gConfirmPromptTitle;
rct_string_id gConfirmPromptDescription;
rct_string_id gConfirmPromptButtonYes = STR_YES;
// Set gConfirmPromptButtonNo to STR_NONE before window_confirm_prompt_open to not include 3 buttons.
rct_string_id gConfirmPromptButtonNo = STR_NONE;
rct_string_id gConfirmPromptButtonCancel = STR_CANCEL;
sint64 gConfirmPromptDescriptionArgs[4];

rct_widgetindex _confirmPromptCallingWidgetIndex;
confirm_prompt_callback * _confirmPromptCallback;
rct_window * _confirmPromptCallingWindow;
confirm_prompt_args * _confirmPromptArgs;

void window_confirm_prompt_open(rct_window * callingWindow, rct_widgetindex callingWidgetIndex, confirm_prompt_callback callback, confirm_prompt_args * args, uint16 flags)
{
    // TODO allow multiple confirm prompts to be open
    // An important thing to keep in mind will be to label not only the number
    // of a confirm prompt, but also the class. That way if a fire staff prompt for staff #4 is open,
    // and a demolish ride prompt for ride #4 gets opened, they'll still be distinguishable.
    window_close_by_class(WC_CONFIRM_PROMPT);
    
    rct_window* w = window_create_centred(WW, WH, &window_confirm_prompt_events, WC_CONFIRM_PROMPT, flags);
    w->widgets = window_confirm_prompt_widgets;
    w->enabled_widgets |= (1 << WIDX_CLOSE) | (1 << WIDX_YES) | (1 << WIDX_CANCEL);

    if (gConfirmPromptButtonNo == STR_NONE) {
        // Only use 2 buttons
        window_confirm_prompt_update_button_size(false);
    }
    else {
        // use 3 buttons
        window_confirm_prompt_update_button_size(true);
        w->enabled_widgets |= (1 << WIDX_NO);
    }

    window_init_scroll_widgets(w);

    w->number = callingWindow->number;
    _confirmPromptCallback = callback;
    _confirmPromptCallingWindow = callingWindow;
    _confirmPromptCallingWidgetIndex = callingWidgetIndex;
    _confirmPromptArgs = args;
}

static void window_confirm_prompt_mouseup(rct_window *w, rct_widgetindex widgetIndex)
{
    switch (widgetIndex) {
    case WIDX_YES:
        if (_confirmPromptCallback != NULL)
            _confirmPromptCallback(_confirmPromptCallingWindow, _confirmPromptCallingWidgetIndex, CONFIRM_PROMPT_RESPONSE_IDX_YES, _confirmPromptArgs);
        window_close(w);
        break;
    case WIDX_NO:
        if (_confirmPromptCallback != NULL)
            _confirmPromptCallback(_confirmPromptCallingWindow, _confirmPromptCallingWidgetIndex, CONFIRM_PROMPT_RESPONSE_IDX_NO, _confirmPromptArgs);
        window_close(w);
        break;
    case WIDX_CANCEL:
    case WIDX_CLOSE:
        if (_confirmPromptCallback != NULL)
            _confirmPromptCallback(_confirmPromptCallingWindow, _confirmPromptCallingWidgetIndex, CONFIRM_PROMPT_RESPONSE_IDX_CANCEL, _confirmPromptArgs);
        window_close(w);
        break;
    }
}

static void window_confirm_prompt_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
    window_draw_widgets(w, dpi);

    sint32 x = w->x + WW / 2;
    sint32 y = w->y + (WH / 2) - 3;

    gfx_draw_string_centred_wrapped(dpi, gConfirmPromptDescriptionArgs, x, y, WW - 4, gConfirmPromptDescription, COLOUR_BLACK);
}

static void window_confirm_prompt_update_button_size(bool useThreeButtons)
{
    // For 2 buttons, there are 2 offsets from the window border, and 1 space in between the buttons.
    // For 3 buttons, there are two spaces.
    sint16 buttonWidth = WW - ((BUTTON_OFFSET * 2) + (BUTTON_SPACE * (useThreeButtons ? 2 : 1)));

    window_confirm_prompt_widgets[WIDX_NO].type = useThreeButtons ? WWT_DROPDOWN_BUTTON : WWT_EMPTY;

    window_confirm_prompt_widgets[WIDX_YES].left = BUTTON_OFFSET;
    window_confirm_prompt_widgets[WIDX_YES].right = window_confirm_prompt_widgets[WIDX_YES].left + buttonWidth;

    if (useThreeButtons) {
        window_confirm_prompt_widgets[WIDX_NO].left = window_confirm_prompt_widgets[WIDX_YES].right + BUTTON_SPACE;
        window_confirm_prompt_widgets[WIDX_NO].right = window_confirm_prompt_widgets[WIDX_NO].left + buttonWidth;
    }

    window_confirm_prompt_widgets[WIDX_CANCEL].left = window_confirm_prompt_widgets[(useThreeButtons ? WIDX_NO : WIDX_YES)].right + BUTTON_SPACE;
    window_confirm_prompt_widgets[WIDX_CANCEL].right = window_confirm_prompt_widgets[WIDX_CANCEL].left + buttonWidth;
}
