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

#ifndef _WINDOW_CONFIRM_PROMPT_H_
#define _WINDOW_CONFIRM_PROMPT_H_

#include "../common.h"

typedef struct confirm_prompt_args {
    const utf8 * path;
} confirm_prompt_args;

typedef void (*confirm_prompt_callback(rct_windowclass callingWindowClass, rct_windownumber callingWindowNumber, rct_widgetindex callingWidgetIndex, sint32 responseIndex, confirm_prompt_args * args));

typedef enum CONFIRM_PROMPT_RESPONSE_IDX {
    CONFIRM_PROMPT_RESPONSE_IDX_YES,
    CONFIRM_PROMPT_RESPONSE_IDX_NO,
    CONFIRM_PROMPT_RESPONSE_IDX_CANCEL,
} CONFIRM_PROMPT_RESPONSE_IDX;

extern rct_string_id gConfirmPromptTitle;
extern rct_string_id gConfirmPromptDescription;
extern rct_string_id gConfirmPromptButtonYes;
// Set gConfirmPromptButtonNo to STR_NONE to not include 3 buttons.
extern rct_string_id gConfirmPromptButtonNo;
extern rct_string_id gConfirmPromptButtonCancel;
extern sint64 gConfirmPromptDescriptionArgs[4];

void window_confirm_prompt_open_raw(rct_windowclass callingWindowClass, rct_windownumber callingWindowNumber, rct_widgetindex callingWidgetIndex, confirm_prompt_callback * callback, confirm_prompt_args * args, uint16 flags);
void window_confirm_prompt_open(rct_window * callingWindow, rct_widgetindex callingWidgetIndex, confirm_prompt_callback * callback, confirm_prompt_args * args, uint16 flags)

#endif
