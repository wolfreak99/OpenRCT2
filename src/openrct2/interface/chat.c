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

#include "../audio/audio.h"
#include "../audio/AudioMixer.h"
#include "../Context.h"
#include "../input.h"
#include "../interface/themes.h"
#include "../localisation/localisation.h"
#include "../network/network.h"
#include "../platform/platform.h"
#include "../util/util.h"
#include "chat.h"
#include "Cursors.h"

bool gChatOpen = false;
static char _chatCurrentLine[CHAT_MAX_MESSAGE_LENGTH];
static char _chatHistory[CHAT_HISTORY_SIZE][CHAT_INPUT_SIZE];
static uint32 _chatHistoryTime[CHAT_HISTORY_SIZE];
static uint32 _chatHistoryIndex = 0;
static uint32 _chatCaretTicks = 0;
static sint32 _chatLeft;
static sint32 _chatTop;
static sint32 _chatRight;
static sint32 _chatBottom;
static sint32 _chatWidth;
static sint32 _chatHeight;
static TextInputSession * _chatTextInputSession;
static sint32 _chatMouseOver = -1;

static const char* chat_history_get(uint32 index);
static uint32 chat_history_get_time(uint32 index);
static void chat_clear_input();

void chat_handle_hover_msg(char* msg, sint32 msgIndex, const CursorState* curState, sint32 rMouseX, sint32 rMouseY);
sint32 chat_string_wrapped_get_height_ext(void *args, sint32 width, sint32 *numLines, sint32 *lineHeight);

void chat_open()
{
    gChatOpen = true;
    _chatTextInputSession = context_start_text_input(_chatCurrentLine, sizeof(_chatCurrentLine));
}

void chat_close()
{
    gChatOpen = false;
    context_stop_text_input();
}

void chat_toggle()
{
    if (gChatOpen) {
        chat_close();
    } else {
        chat_open();
    }
}

void chat_init()
{
    memset(_chatHistory, 0, sizeof(_chatHistory));
    memset(_chatHistoryTime, 0, sizeof(_chatHistoryTime));
}

// TODO remove this or revert "(DEBUG) create debug_ui_box" when finished
bool debug_ui_box_show = false;
sint32 debug_ui_box_x1 = 0, debug_ui_box_y1 = 0, debug_ui_box_x2 = 0, debug_ui_box_y2 = 0;
sint32 debug_mouse_x = 0, debug_mouse_y = 0;

void debug_ui_box_set(sint32 x1, sint32 y1, sint32 x2, sint32 y2)
{
    debug_ui_box_show = true;
    debug_ui_box_x1 = x1;
    debug_ui_box_y1 = y1;
    debug_ui_box_x2 = x2;
    debug_ui_box_y2 = y2;
}

void debug_ui_box_hide()
{
    debug_ui_box_show = false;
}

void chat_update()
{
    // Flash the caret
    _chatCaretTicks = (_chatCaretTicks + 1) % 30;

    // Clicking the chat
    if (!input_test_flag(INPUT_FLAG_5))
    {
        char* inputLine = _chatCurrentLine;
        sint32 stringHeight = chat_string_wrapped_get_height((void*)&inputLine, _chatWidth - 10) + 5;

        sint32 y = _chatBottom - stringHeight - 15;

        const CursorState * cursorState = context_get_cursor_state();
        sint32 mX = cursorState->x;
        sint32 mY = cursorState->y;

        if (_chatMouseOver != -1) {
            _chatMouseOver = -1;
            // To prevent blocking visual feedback on items below chat, cursor should
            // only reset when a message was previously hovered over.
            input_set_cursor(CURSOR_ARROW);
            debug_ui_box_hide();
            debug_mouse_x = 0;
            debug_mouse_y = 0;
        }

        if (mX > _chatLeft && mX < _chatRight && mY > _chatTop && mY < _chatBottom) {
            debug_mouse_x = mX;
            debug_mouse_y = mY;
            y -= 2; // Shift Y up a hair, that way boundary won't ride exactly on top of text
            for (sint32 i = 0; i < CHAT_HISTORY_SIZE; i++, y -= stringHeight) {
                uint32 expireTime = chat_history_get_time(i) + 10000;
                if (!gChatOpen && platform_get_ticks() > expireTime) {
                    break;
                }

                char lineBuffer[CHAT_INPUT_SIZE + 10];
                char* lineCh = lineBuffer;
                safe_strcpy(lineBuffer, chat_history_get(i), CHAT_INPUT_SIZE + 10);

                stringHeight = chat_string_wrapped_get_height((void*)&lineCh, _chatWidth - 10) + 5;
                // If a message has more than 1 line, y is shifted up
                sint32 shiftedY = y - (stringHeight > 15 ? (stringHeight - 15) : 0);
                
                if (mY > shiftedY && mY < shiftedY + stringHeight && !str_is_null_or_empty(lineCh)) {
                    debug_ui_box_set(_chatLeft, shiftedY, _chatRight, shiftedY + stringHeight);
                    chat_handle_hover_msg(lineCh, i, cursorState, mX - _chatLeft, mY - shiftedY);
                }
            }
        }
    }
}

void draw_rect(rct_drawpixelinfo* dpi, sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 col)
{
    gfx_draw_line(dpi, x1, y1, x1, y2, COLOUR_BRIGHT_RED);
    gfx_draw_line(dpi, x1, y1, x2, y1, COLOUR_BRIGHT_RED);
    gfx_draw_line(dpi, x2, y1, x2, y2, COLOUR_BRIGHT_RED);
    gfx_draw_line(dpi, x1, y2, x2, y2, COLOUR_BRIGHT_RED);
}

void chat_draw(rct_drawpixelinfo * dpi)
{
    if (network_get_mode() == NETWORK_MODE_NONE || network_get_status() != NETWORK_STATUS_CONNECTED || network_get_authstatus() != NETWORK_AUTH_OK) {
        gChatOpen = false;
        return;
    }

    _chatLeft = 10;
    _chatRight = min((context_get_width() - 10), CHAT_MAX_WINDOW_WIDTH);
    _chatWidth = _chatRight - _chatLeft;
    _chatBottom = context_get_height() - 45;
    _chatTop = _chatBottom - 10;

    char lineBuffer[CHAT_INPUT_SIZE + 10];
    char* lineCh = lineBuffer;
    char* inputLine = _chatCurrentLine;
    sint32 inputLineHeight = 10;
    uint8 chatBackgroundColor = theme_get_colour(WC_CHAT, 0);

    // Draw chat window
    if (gChatOpen) {
        inputLineHeight = chat_string_wrapped_get_height((void*)&inputLine, _chatWidth - 10);
        _chatTop -= inputLineHeight;

        for (sint32 i = 0; i < CHAT_HISTORY_SIZE; i++) {
            if (strlen(chat_history_get(i)) == 0) {
                continue;
            }

            safe_strcpy(lineBuffer, chat_history_get(i), sizeof(lineBuffer));

            sint32 lineHeight = chat_string_wrapped_get_height((void*)&lineCh, _chatWidth - 10);
            _chatTop -= (lineHeight + 5);
        }

        _chatHeight = _chatBottom - _chatTop;

        if (_chatTop < 50) {
            _chatTop = 50;
        } else if (_chatHeight < 150) { // Min height
            _chatTop = _chatBottom - 150;
            _chatHeight = 150;
        }

        gfx_set_dirty_blocks(_chatLeft, _chatTop - 5, _chatRight, _chatBottom + 5); // Background area + Textbox
        gfx_filter_rect(dpi, _chatLeft, _chatTop - 5, _chatRight, _chatBottom + 5, PALETTE_51); // Opaque gray background
        gfx_fill_rect_inset(dpi, _chatLeft, _chatTop - 5, _chatRight, _chatBottom + 5, chatBackgroundColor, INSET_RECT_FLAG_FILL_NONE);
        gfx_fill_rect_inset(dpi, _chatLeft + 1, _chatTop - 4, _chatRight - 1, _chatBottom - inputLineHeight - 6, chatBackgroundColor, INSET_RECT_FLAG_BORDER_INSET);
        gfx_fill_rect_inset(dpi, _chatLeft + 1, _chatBottom - inputLineHeight - 5, _chatRight - 1, _chatBottom + 4, chatBackgroundColor, INSET_RECT_FLAG_BORDER_INSET); // Textbox
    }

    sint32 x = _chatLeft + 5;
    sint32 y = _chatBottom - inputLineHeight - 20;
    sint32 stringHeight = 0;

    // Draw chat history
    for (sint32 i = 0; i < CHAT_HISTORY_SIZE; i++, y -= stringHeight) {
        uint32 expireTime = chat_history_get_time(i) + 10000;
        if (!gChatOpen && platform_get_ticks() > expireTime) {
            break;
        }

        safe_strcpy(lineBuffer, chat_history_get(i), sizeof(lineBuffer));

        // TODO remove this code (or revert "(DEBUG) Shift text to right when hovered over") when finished
        sint32 _x = x;
        if (_chatMouseOver == i) {
            _x += 5;
        }

        stringHeight = chat_history_draw_string(dpi, (void*) &lineCh, _x, y, _chatWidth - 10) + 5;

        gfx_set_dirty_blocks(x, y - stringHeight, _x + _chatWidth, y + 20);

        if ((y - stringHeight) < 50) {
            break;
        }
    }

    // Draw current chat input
    if (gChatOpen) {
        lineCh = utf8_write_codepoint(lineCh, FORMAT_OUTLINE);
        lineCh = utf8_write_codepoint(lineCh, FORMAT_CELADON);

        safe_strcpy(lineCh, _chatCurrentLine, sizeof(_chatCurrentLine));
        y = _chatBottom - inputLineHeight - 5;

        lineCh = lineBuffer;
        inputLineHeight = gfx_draw_string_left_wrapped(dpi, (void*)&lineCh, x, y + 3, _chatWidth - 10, STR_STRING, TEXT_COLOUR_255);
        gfx_set_dirty_blocks(x, y, x + _chatWidth, y + inputLineHeight + 15);

        // TODO: Show caret if the input text has multiple lines
        if (_chatCaretTicks < 15 && gfx_get_string_width(lineBuffer) < (_chatWidth - 10)) {
            memcpy(lineBuffer, _chatCurrentLine, _chatTextInputSession->SelectionStart);
            lineBuffer[_chatTextInputSession->SelectionStart] = 0;
            sint32 caretX = x + gfx_get_string_width(lineBuffer);
            sint32 caretY = y + 14;

            gfx_fill_rect(dpi, caretX, caretY, caretX + 6, caretY + 1, PALETTE_INDEX_56);
        }
    }

    // TODO remove this or revert "(DEBUG) create debug_ui_box" when finished
    if (debug_ui_box_show) {
        draw_rect(dpi, debug_ui_box_x1, debug_ui_box_y1, debug_ui_box_x2, debug_ui_box_y2, COLOUR_BRIGHT_RED);
    }
    draw_rect(dpi, debug_mouse_x - 1, debug_mouse_y - 1, debug_mouse_x + 1, debug_mouse_y + 1, COLOUR_MOSS_GREEN);
}

void chat_handle_hover_msg(char* msg, sint32 msgIndex, const CursorState* curState, sint32 rMouseX, sint32 rMouseY)
{
    _chatMouseOver = msgIndex;

    // TODO use relative mouse x/y coordinates to determine the distance
    // into the message to check. This will allow multiple urls in a single
    // message by skipping urls before the hovered url.
    // Take into account user will usually hover halfway into a url.
    char url[256];
    char* urlBuffer = url;
    url_from_string(urlBuffer, msg, sizeof(url));

    if (!str_is_null_or_empty(url)) {
        input_set_cursor(CURSOR_HAND_POINT);
        if (curState->left == CURSOR_RELEASED) {
            platform_open_browser(url);
        }
    }
    // TODO Remove this code or revert "(DEBUG) Allow clearing chat history by clicking non-url messages" once lineHeight is working properly
    else {
        if (curState->left == CURSOR_RELEASED) {
            for (sint32 i = 0; i < CHAT_HISTORY_SIZE; i++) {
                memset(_chatHistory[i], 0, CHAT_INPUT_SIZE);
            }
            _chatHistoryIndex = 0;
        }
    }
}

void chat_history_add(const char * src)
{
    size_t bufferSize = strlen(src) + 64;
    utf8 * buffer = (utf8 *)calloc(1, bufferSize);

    // Find the start of the text (after format codes)
    const char * ch = src;
    const char * nextCh;
    uint32 codepoint;
    while ((codepoint = utf8_get_next(ch, &nextCh)) != 0) {
        if (!utf8_is_format_code(codepoint)) {
            break;
        }
        ch = nextCh;
    }
    const char * srcText = ch;

    // Copy format codes to buffer
    memcpy(buffer, src, min(bufferSize, (size_t)(srcText - src)));

    // Prepend a timestamp
    time_t timer;
    time(&timer);
    struct tm * tmInfo = localtime(&timer);

    strcatftime(buffer, bufferSize, "[%H:%M] ", tmInfo);
    safe_strcat(buffer, srcText, bufferSize);

    // Add to history list
    sint32 index = _chatHistoryIndex % CHAT_HISTORY_SIZE;
    memset(_chatHistory[index], 0, CHAT_INPUT_SIZE);
    memcpy(_chatHistory[index], buffer, min(strlen(buffer), CHAT_INPUT_SIZE - 1));
    _chatHistoryTime[index] = platform_get_ticks();
    _chatHistoryIndex++;

    // Log to file (src only as logging does its own timestamp)
    network_append_chat_log(src);

    free(buffer);

    Mixer_Play_Effect(SOUND_NEWS_ITEM, 0, MIXER_VOLUME_MAX, 0.5f, 1.5f, true);
}

void chat_input(CHAT_INPUT input)
{
    switch (input) {
    case CHAT_INPUT_SEND:
        if (strlen(_chatCurrentLine) > 0) {
            network_send_chat(_chatCurrentLine);
        }
        chat_clear_input();
        chat_close();
        break;
    case CHAT_INPUT_CLOSE:
        chat_close();
        break;
    default:
        break;
    }
}

static const char* chat_history_get(uint32 index)
{
    return _chatHistory[(_chatHistoryIndex + CHAT_HISTORY_SIZE - index - 1) % CHAT_HISTORY_SIZE];
}

static uint32 chat_history_get_time(uint32 index)
{
    return _chatHistoryTime[(_chatHistoryIndex + CHAT_HISTORY_SIZE - index - 1) % CHAT_HISTORY_SIZE];
}

static void chat_clear_input()
{
    _chatCurrentLine[0] = 0;
}

// This method is the same as gfx_draw_string_left_wrapped.
// But this adjusts the initial Y coordinate depending of the number of lines.
sint32 chat_history_draw_string(rct_drawpixelinfo *dpi, void *args, sint32 x, sint32 y, sint32 width)
{
    sint32 fontSpriteBase, lineHeight, lineY, numLines;

    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;

    gfx_draw_string(dpi, "", TEXT_COLOUR_255, dpi->x, dpi->y);
    char *buffer = gCommonStringFormatBuffer;
    format_string(buffer, 256, STR_STRING, args);

    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;
    gfx_wrap_string(buffer, width, &numLines, &fontSpriteBase);
    lineHeight = font_get_line_height(fontSpriteBase);

    gCurrentFontFlags = 0;

    sint32 expectedY = y - (numLines * lineHeight);
    if (expectedY < 50) {
        return (numLines * lineHeight); // Skip drawing, return total height.
    }

    lineY = y;
    for (sint32 line = 0; line <= numLines; ++line) {
        gfx_draw_string(dpi, buffer, TEXT_COLOUR_254, x, lineY - (numLines * lineHeight));
        buffer = get_string_end(buffer) + 1;
        lineY += lineHeight;
    }
    return lineY - y;
}

// Wrap string without drawing, useful to get the height of a wrapped string.
// Almost the same as gfx_draw_string_left_wrapped
sint32 chat_string_wrapped_get_height(void *args, sint32 width)
{
    sint32 fontSpriteBase, lineHeight, lineY, numLines;

    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;

    char *buffer = gCommonStringFormatBuffer;
    format_string(buffer, 256, STR_STRING, args);

    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;
    gfx_wrap_string(buffer, width, &numLines, &fontSpriteBase);
    lineHeight = font_get_line_height(fontSpriteBase);

    gCurrentFontFlags = 0;

    lineY = 0;
    for (sint32 line = 0; line <= numLines; ++line) {
        buffer = get_string_end(buffer) + 1;
        lineY += lineHeight;
    }

    return lineY;
}

// Wrap string without drawing, useful to get the height of a wrapped string.
// Almost the same as gfx_draw_string_left_wrapped
sint32 chat_string_wrapped_get_height_ext(void *args, sint32 width, sint32 *numLines, sint32 *lineHeight)
{
    sint32 fontSpriteBase, lineY;

    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;

    char *buffer = gCommonStringFormatBuffer;
    format_string(buffer, 256, STR_STRING, args);

    gCurrentFontSpriteBase = FONT_SPRITE_BASE_MEDIUM;
    gfx_wrap_string(buffer, width, numLines, &fontSpriteBase);
    *lineHeight = font_get_line_height(fontSpriteBase);

    gCurrentFontFlags = 0;

    lineY = 0;
    for (sint32 line = 0; line <= *numLines; ++line) {
        buffer = get_string_end(buffer) + 1;
        lineY += *lineHeight;
    }

    return lineY;
}
