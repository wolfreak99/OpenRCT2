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

#ifndef _PEEP_H_
#define _PEEP_H_

#include "../common.h"
#include "../world/map.h"

#define PEEP_MAX_THOUGHTS 5

#define PEEP_HUNGER_WARNING_THRESHOLD 25
#define PEEP_THIRST_WARNING_THRESHOLD 25
#define PEEP_BATHROOM_WARNING_THRESHOLD 28
#define PEEP_LITTER_WARNING_THRESHOLD 23
#define PEEP_DISGUST_WARNING_THRESHOLD 22
#define PEEP_VANDALISM_WARNING_THRESHOLD 15
#define PEEP_NOEXIT_WARNING_THRESHOLD 8
#define PEEP_LOST_WARNING_THRESHOLD 8

enum PEEP_TYPE {
    PEEP_TYPE_GUEST,
    PEEP_TYPE_STAFF
};

enum PEEP_THOUGHT_TYPE {
    PEEP_THOUGHT_TYPE_CANT_AFFORD_0 = 0, // "I can't afford"
    PEEP_THOUGHT_TYPE_SPENT_MONEY = 1, // "I've spent all my money"
    PEEP_THOUGHT_TYPE_SICK = 2, // "I feel sick"
    PEEP_THOUGHT_TYPE_VERY_SICK = 3, // "I feel very sick"
    PEEP_THOUGHT_TYPE_MORE_THRILLING = 4, // "I want to go on something more thrilling than X"
    PEEP_THOUGHT_TYPE_INTENSE = 5, // "X looks too intense for me"
    PEEP_THOUGHT_TYPE_HAVENT_FINISHED = 6, // "I haven't finished my X yet"
    PEEP_THOUGHT_TYPE_SICKENING = 7, // "Just looking at X makes me feel sick"
    PEEP_THOUGHT_TYPE_BAD_VALUE = 8, // "I'm not paying that much to go on X"
    PEEP_THOUGHT_TYPE_GO_HOME = 9, // "I want to go home"
    PEEP_THOUGHT_TYPE_GOOD_VALUE = 10, // "X is really good value"
    PEEP_THOUGHT_TYPE_ALREADY_GOT = 11, // "I've already got"
    PEEP_THOUGHT_TYPE_CANT_AFFORD = 12, // "I can't afford"
    PEEP_THOUGHT_TYPE_NOT_HUNGRY = 13, // "I'm not hungry"
    PEEP_THOUGHT_TYPE_NOT_THIRSTY = 14, // "I'm not thirsty"
    PEEP_THOUGHT_TYPE_DROWNING = 15, // "Help! I'm drowning!"
    PEEP_THOUGHT_TYPE_LOST = 16, // "I'm lost!"
    PEEP_THOUGHT_TYPE_WAS_GREAT = 17, // "X was great"
    PEEP_THOUGHT_TYPE_QUEUING_AGES = 18, // "I've been queuing for X for ages"
    PEEP_THOUGHT_TYPE_TIRED = 19, // "I'm tired"
    PEEP_THOUGHT_TYPE_HUNGRY = 20, // "I'm hungry"
    PEEP_THOUGHT_TYPE_THIRSTY = 21, // "I'm thirsty"
    PEEP_THOUGHT_TYPE_BATHROOM = 22, // "I need to go to the bathroom"
    PEEP_THOUGHT_TYPE_CANT_FIND = 23, // "I can't find X"
    PEEP_THOUGHT_TYPE_NOT_PAYING = 24, // "I'm not paying that much to use X"
    PEEP_THOUGHT_TYPE_NOT_WHILE_RAINING = 25, // "I'm not going on X while it's raining"
    PEEP_THOUGHT_TYPE_BAD_LITTER = 26, // "The litter here is really bad"
    PEEP_THOUGHT_TYPE_CANT_FIND_EXIT = 27, // "I can't find the exit"
    PEEP_THOUGHT_TYPE_GET_OFF = 28, // "I want to get off X"
    PEEP_THOUGHT_TYPE_GET_OUT = 29, // "I want to get out of X"
    PEEP_THOUGHT_TYPE_NOT_SAFE = 30, // "I'm not going on X - it isn't safe"
    PEEP_THOUGHT_TYPE_PATH_DISGUSTING = 31, // "This path is disgusting"
    PEEP_THOUGHT_TYPE_CROWDED = 32, // "It's too crowded here"
    PEEP_THOUGHT_TYPE_VANDALISM = 33, // "The vandalism here is really bad"
    PEEP_THOUGHT_TYPE_SCENERY = 34, // "Great scenery!"
    PEEP_THOUGHT_TYPE_VERY_CLEAN = 35, // "This park is very clean and tidy"
    PEEP_THOUGHT_TYPE_FOUNTAINS = 36, // "The jumping fountains are great"
    PEEP_THOUGHT_TYPE_MUSIC = 37, // "The music is nice here"
    PEEP_THOUGHT_TYPE_BALLOON = 38, // "This balloon from X is really good value"
    PEEP_THOUGHT_TYPE_TOY = 39, // "This cuddly toy from X is really good value"
    PEEP_THOUGHT_TYPE_MAP = 40,
    PEEP_THOUGHT_TYPE_PHOTO = 41, // "This on ride photo from X is really good value"
    PEEP_THOUGHT_TYPE_UMBRELLA = 42,
    PEEP_THOUGHT_TYPE_DRINK = 43,
    PEEP_THOUGHT_TYPE_BURGER = 44,
    PEEP_THOUGHT_TYPE_CHIPS = 45,
    PEEP_THOUGHT_TYPE_ICE_CREAM = 46,
    PEEP_THOUGHT_TYPE_CANDYFLOSS = 47,

    PEEP_THOUGHT_TYPE_PIZZA = 51,

    PEEP_THOUGHT_TYPE_POPCORN = 53,
    PEEP_THOUGHT_TYPE_HOT_DOG = 54,
    PEEP_THOUGHT_TYPE_TENTACLE = 55,
    PEEP_THOUGHT_TYPE_HAT = 56,
    PEEP_THOUGHT_TYPE_CANDY_APPLE = 57,
    PEEP_THOUGHT_TYPE_TSHIRT = 58,
    PEEP_THOUGHT_TYPE_DONUT = 59,
    PEEP_THOUGHT_TYPE_COFFEE = 60,

    PEEP_THOUGHT_TYPE_CHICKEN = 62,
    PEEP_THOUGHT_TYPE_LEMONADE = 63,

    PEEP_THOUGHT_TYPE_WOW = 67, // "Wow!"

    PEEP_THOUGHT_TYPE_WOW2 = 70,// "Wow!"
    PEEP_THOUGHT_TYPE_WATCHED = 71, // "I have the strangest feeling someone is watching me"
    PEEP_THOUGHT_TYPE_BALLOON_MUCH = 72, // "I'm not paying that much to get a balloon from X"
    PEEP_THOUGHT_TYPE_TOY_MUCH = 73,
    PEEP_THOUGHT_TYPE_MAP_MUCH = 74,
    PEEP_THOUGHT_TYPE_PHOTO_MUCH = 75,
    PEEP_THOUGHT_TYPE_UMBRELLA_MUCH = 76,
    PEEP_THOUGHT_TYPE_DRINK_MUCH = 77,
    PEEP_THOUGHT_TYPE_BURGER_MUCH = 78,
    PEEP_THOUGHT_TYPE_CHIPS_MUCH = 79,
    PEEP_THOUGHT_TYPE_ICE_CREAM_MUCH = 80,
    PEEP_THOUGHT_TYPE_CANDYFLOSS_MUCH = 81,

    PEEP_THOUGHT_TYPE_PIZZA_MUCH = 85,

    PEEP_THOUGHT_TYPE_POPCORN_MUCH = 87,
    PEEP_THOUGHT_TYPE_HOT_DOG_MUCH = 88,
    PEEP_THOUGHT_TYPE_TENTACLE_MUCH = 89,
    PEEP_THOUGHT_TYPE_HAT_MUCH = 90,
    PEEP_THOUGHT_TYPE_CANDY_APPLE_MUCH = 91,
    PEEP_THOUGHT_TYPE_TSHIRT_MUCH = 92,
    PEEP_THOUGHT_TYPE_DONUT_MUCH = 93,
    PEEP_THOUGHT_TYPE_COFFEE_MUCH = 94,

    PEEP_THOUGHT_TYPE_CHICKEN_MUCH = 96,
    PEEP_THOUGHT_TYPE_LEMONADE_MUCH = 97,

    PEEP_THOUGHT_TYPE_PHOTO2 = 104, // "This on-ride photo from X is really good value"
    PEEP_THOUGHT_TYPE_PHOTO3 = 105,
    PEEP_THOUGHT_TYPE_PHOTO4 = 106,
    PEEP_THOUGHT_TYPE_PRETZEL = 107,
    PEEP_THOUGHT_TYPE_HOT_CHOCOLATE = 108,
    PEEP_THOUGHT_TYPE_ICED_TEA = 109,
    PEEP_THOUGHT_TYPE_FUNNEL_CAKE = 110,
    PEEP_THOUGHT_TYPE_SUNGLASSES = 111,
    PEEP_THOUGHT_TYPE_BEEF_NOODLES = 112,
    PEEP_THOUGHT_TYPE_FRIED_RICE_NOODLES = 113,
    PEEP_THOUGHT_TYPE_WONTON_SOUP = 114,
    PEEP_THOUGHT_TYPE_MEATBALL_SOUP = 115,
    PEEP_THOUGHT_TYPE_FRUIT_JUICE = 116,
    PEEP_THOUGHT_TYPE_SOYBEAN_MILK = 117,
    PEEP_THOUGHT_TYPE_SU_JONGKWA = 118,
    PEEP_THOUGHT_TYPE_SUB_SANDWICH = 119,
    PEEP_THOUGHT_TYPE_COOKIE = 120,

    PEEP_THOUGHT_TYPE_ROAST_SAUSAGE = 124,

    PEEP_THOUGHT_TYPE_PHOTO2_MUCH = 136,
    PEEP_THOUGHT_TYPE_PHOTO3_MUCH = 137,
    PEEP_THOUGHT_TYPE_PHOTO4_MUCH = 138,
    PEEP_THOUGHT_TYPE_PRETZEL_MUCH = 139,
    PEEP_THOUGHT_TYPE_HOT_CHOCOLATE_MUCH = 140,
    PEEP_THOUGHT_TYPE_ICED_TEA_MUCH = 141,
    PEEP_THOUGHT_TYPE_FUNNEL_CAKE_MUCH = 142,
    PEEP_THOUGHT_TYPE_SUNGLASSES_MUCH = 143,
    PEEP_THOUGHT_TYPE_BEEF_NOODLES_MUCH = 144,
    PEEP_THOUGHT_TYPE_FRIED_RICE_NOODLES_MUCH = 145,
    PEEP_THOUGHT_TYPE_WONTON_SOUP_MUCH = 146,
    PEEP_THOUGHT_TYPE_MEATBALL_SOUP_MUCH = 147,
    PEEP_THOUGHT_TYPE_FRUIT_JUICE_MUCH = 148,
    PEEP_THOUGHT_TYPE_SOYBEAN_MILK_MUCH = 149,
    PEEP_THOUGHT_TYPE_SU_JONGKWA_MUCH = 150,
    PEEP_THOUGHT_TYPE_SUB_SANDWICH_MUCH = 151,
    PEEP_THOUGHT_TYPE_COOKIE_MUCH = 152,

    PEEP_THOUGHT_TYPE_ROAST_SAUSAGE_MUCH = 156,

    PEEP_THOUGHT_TYPE_HELP = 168, // "Help! Put me down!"
    PEEP_THOUGHT_TYPE_RUNNING_OUT = 169, // "I'm running out of cash!"
    PEEP_THOUGHT_TYPE_NEW_RIDE = 170, // "Wow! A new ride being built!"
    PEEP_THOUGHT_TYPE_NICE_RIDE_DEPRECATED = 171, // "Nice ride! But not as good as the Phoenix..."
    PEEP_THOUGHT_TYPE_EXCITED_DEPRECATED = 172, // "I'm so excited - It's an Intamin ride!"
    PEEP_THOUGHT_TYPE_HERE_WE_ARE = 173, // "...and here we are on X!"

    PEEP_THOUGHT_TYPE_NONE = 255
};

enum PEEP_STATE {
    PEEP_STATE_FALLING = 0, // Drowning is part of falling
    PEEP_STATE_1 = 1,
    PEEP_STATE_QUEUING_FRONT = 2,
    PEEP_STATE_ON_RIDE = 3,
    PEEP_STATE_LEAVING_RIDE = 4,
    PEEP_STATE_WALKING = 5,
    PEEP_STATE_QUEUING = 6,
    PEEP_STATE_ENTERING_RIDE = 7,
    PEEP_STATE_SITTING = 8,
    PEEP_STATE_PICKED = 9,
    PEEP_STATE_PATROLLING = 10, // Not sure
    PEEP_STATE_MOWING = 11,
    PEEP_STATE_SWEEPING = 12,
    PEEP_STATE_ENTERING_PARK = 13,
    PEEP_STATE_LEAVING_PARK = 14,
    PEEP_STATE_ANSWERING = 15,
    PEEP_STATE_FIXING = 16,
    PEEP_STATE_BUYING = 17,
    PEEP_STATE_WATCHING = 18,
    PEEP_STATE_EMPTYING_BIN = 19,
    PEEP_STATE_USING_BIN = 20,
    PEEP_STATE_WATERING = 21,
    PEEP_STATE_HEADING_TO_INSPECTION = 22,
    PEEP_STATE_INSPECTING = 23
};

enum PEEP_ACTION_EVENTS {
    PEEP_ACTION_CHECK_TIME = 0,
    // If no food then check watch
    PEEP_ACTION_EAT_FOOD = 1,
    PEEP_ACTION_SHAKE_HEAD = 2,
    PEEP_ACTION_EMPTY_POCKETS = 3,
    PEEP_ACTION_SITTING_EAT_FOOD = 4,
    PEEP_ACTION_SITTING_CHECK_WATCH = 4,
    PEEP_ACTION_SITTING_LOOK_AROUND_LEFT = 5,
    PEEP_ACTION_SITTING_LOOK_AROUND_RIGHT = 6,
    PEEP_ACTION_WOW = 7,
    PEEP_ACTION_THROW_UP = 8,
    PEEP_ACTION_JUMP = 9,
    PEEP_ACTION_STAFF_SWEEP = 10,
    PEEP_ACTION_DROWNING = 11,
    PEEP_ACTION_STAFF_ANSWER_CALL = 12,
    PEEP_ACTION_STAFF_ANSWER_CALL_2 = 13,
    PEEP_ACTION_STAFF_CHECKBOARD = 14,
    PEEP_ACTION_STAFF_FIX = 15,
    PEEP_ACTION_STAFF_FIX_2 = 16,
    PEEP_ACTION_STAFF_FIX_GROUND = 17,
    PEEP_ACTION_STAFF_FIX_3 = 18,
    PEEP_ACTION_STAFF_WATERING = 19,
    PEEP_ACTION_JOY = 20,
    PEEP_ACTION_READ_MAP = 21,
    PEEP_ACTION_WAVE = 22,
    PEEP_ACTION_STAFF_EMPTY_BIN = 23,
    PEEP_ACTION_WAVE_2 = 24,
    PEEP_ACTION_TAKE_PHOTO = 25,
    PEEP_ACTION_CLAP = 26,
    PEEP_ACTION_DISGUST = 27,
    PEEP_ACTION_DRAW_PICTURE = 28,
    PEEP_ACTION_BEING_WATCHED = 29,
    PEEP_ACTION_WITHDRAW_MONEY = 30,

    PEEP_ACTION_NONE_1 = 254,
    PEEP_ACTION_NONE_2 = 255
};

enum PEEP_ACTION_SPRITE_TYPE {
    PEEP_ACTION_SPRITE_TYPE_NONE = 0,
    PEEP_ACTION_SPRITE_TYPE_CHECK_TIME = 1,
    PEEP_ACTION_SPRITE_TYPE_2 = 2,
    PEEP_ACTION_SPRITE_TYPE_EAT_FOOD = 3,
    PEEP_ACTION_SPRITE_TYPE_SHAKE_HEAD = 4,
    PEEP_ACTION_SPRITE_TYPE_EMPTY_POCKETS = 5,
    PEEP_ACTION_SPRITE_TYPE_HOLD_MAT = 6,
    PEEP_ACTION_SPRITE_TYPE_7 = 7,
    PEEP_ACTION_SPRITE_TYPE_SITTING_EAT_FOOD = 8,
    PEEP_ACTION_SPRITE_TYPE_SITTING_LOOK_AROUND_LEFT = 9,
    PEEP_ACTION_SPRITE_TYPE_SITTING_LOOK_AROUND_RIGHT = 10,
    PEEP_ACTION_SPRITE_TYPE_UI = 11,
    PEEP_ACTION_SPRITE_TYPE_STAFF_MOWER = 12,
    PEEP_ACTION_SPRITE_TYPE_WOW = 13,
    PEEP_ACTION_SPRITE_TYPE_THROW_UP = 14,
    PEEP_ACTION_SPRITE_TYPE_JUMP = 15,
    PEEP_ACTION_SPRITE_TYPE_STAFF_SWEEP = 16,
    PEEP_ACTION_SPRITE_TYPE_DROWNING = 17,
    PEEP_ACTION_SPRITE_TYPE_STAFF_ANSWER_CALL = 18,
    PEEP_ACTION_SPRITE_TYPE_STAFF_ANSWER_CALL_2 = 19,
    PEEP_ACTION_SPRITE_TYPE_STAFF_CHECKBOARD = 20,
    PEEP_ACTION_SPRITE_TYPE_STAFF_FIX = 21,
    PEEP_ACTION_SPRITE_TYPE_STAFF_FIX_2 = 22,
    PEEP_ACTION_SPRITE_TYPE_STAFF_FIX_GROUND = 23,
    PEEP_ACTION_SPRITE_TYPE_STAFF_FIX_3 = 24,
    PEEP_ACTION_SPRITE_TYPE_STAFF_WATERING = 25,
    PEEP_ACTION_SPRITE_TYPE_JOY = 26,
    PEEP_ACTION_SPRITE_TYPE_READ_MAP = 27,
    PEEP_ACTION_SPRITE_TYPE_WAVE = 28,
    PEEP_ACTION_SPRITE_TYPE_STAFF_EMPTY_BIN = 29,
    PEEP_ACTION_SPRITE_TYPE_WAVE_2 = 30,
    PEEP_ACTION_SPRITE_TYPE_TAKE_PHOTO = 31,
    PEEP_ACTION_SPRITE_TYPE_CLAP = 32,
    PEEP_ACTION_SPRITE_TYPE_DISGUST = 33,
    PEEP_ACTION_SPRITE_TYPE_DRAW_PICTURE = 34,
    PEEP_ACTION_SPRITE_TYPE_BEING_WATCHED = 35,
    PEEP_ACTION_SPRITE_TYPE_WITHDRAW_MONEY = 36
};

enum PEEP_FLAGS {
    PEEP_FLAGS_LEAVING_PARK = (1 << 0),
    PEEP_FLAGS_SLOW_WALK = (1 << 1),
    PEEP_FLAGS_2 = (1 << 2),
    PEEP_FLAGS_TRACKING = (1 << 3),
    PEEP_FLAGS_WAVING = (1 << 4), // Makes the peep wave
    PEEP_FLAGS_HAS_PAID_FOR_PARK_ENTRY = (1 << 5), // Set on paying to enter park
    PEEP_FLAGS_PHOTO = (1 << 6), // Makes the peep take a picture
    PEEP_FLAGS_PAINTING = (1 << 7),
    PEEP_FLAGS_WOW = (1 << 8), // Makes a peep WOW2
    PEEP_FLAGS_LITTER = (1 << 9), // Makes the peep throw litter
    PEEP_FLAGS_LOST = (1 << 10), // Makes the peep feel lost (animation triggered)
    PEEP_FLAGS_HUNGER = (1 << 11), // Makes the peep become hungry quicker
    PEEP_FLAGS_BATHROOM = (1 << 12), // Makes the peep want to go to the bathroom
    PEEP_FLAGS_CROWDED = (1 << 13), // The peep will start feeling crowded
    PEEP_FLAGS_HAPPINESS = (1 << 14), // The peep will start increasing happiness
    PEEP_FLAGS_NAUSEA = (1 << 15), // Makes the peep feel sick (e.g. after an extreme ride)
    PEEP_FLAGS_PURPLE = (1 << 16), // Makes surrounding peeps purple
    PEEP_FLAGS_PIZZA = (1 << 17), // Gives passing peeps pizza
    PEEP_FLAGS_EXPLODE = (1 << 18),
    PEEP_FLAGS_RIDE_SHOULD_BE_MARKED_AS_FAVOURITE = (1 << 19),
    PEEP_FLAGS_PARK_ENTRANCE_CHOSEN = (1 << 20), //Set when the nearest park entrance has been chosen
    PEEP_FLAGS_21 = (1 << 21),
    PEEP_FLAGS_CONTAGIOUS = (1 << 22), // Makes any peeps in surrounding tiles sick
    PEEP_FLAGS_JOY = (1 << 23), // Makes the peep jump in joy
    PEEP_FLAGS_ANGRY = (1 << 24),
    PEEP_FLAGS_ICE_CREAM = (1 << 25), // Gives passing peeps ice cream and they wave back
    PEEP_FLAGS_NICE_RIDE_DEPRECATED = (1 << 26), // Used to make the peep think "Nice ride! But not as good as the Phoenix..." on exiting a ride
    PEEP_FLAGS_INTAMIN_DEPRECATED = (1 << 27), // Used to make the peep think "I'm so excited - It's an Intamin ride!" while riding on a Intamin ride.
    PEEP_FLAGS_HERE_WE_ARE = (1 << 28), // Makes the peep think  "...and here we are on X!" while riding a ride
    PEEP_FLAGS_TWITCH = (1u << 31)      // Added for twitch integration
};

enum PEEP_NAUSEA_TOLERANCE {
    PEEP_NAUSEA_TOLERANCE_NONE,
    PEEP_NAUSEA_TOLERANCE_LOW,
    PEEP_NAUSEA_TOLERANCE_AVERAGE,
    PEEP_NAUSEA_TOLERANCE_HIGH
};

enum PEEP_ITEM {
    // item_standard_flags
    PEEP_ITEM_BALLOON = (1 << 0),
    PEEP_ITEM_TOY = (1 << 1),
    PEEP_ITEM_MAP = (1 << 2),
    PEEP_ITEM_PHOTO = (1 << 3),
    PEEP_ITEM_UMBRELLA = (1 << 4),
    PEEP_ITEM_DRINK = (1 << 5),
    PEEP_ITEM_BURGER = (1 << 6),
    PEEP_ITEM_CHIPS = (1 << 7),
    PEEP_ITEM_ICE_CREAM = (1 << 8),
    PEEP_ITEM_CANDYFLOSS = (1 << 9),
    PEEP_ITEM_EMPTY_CAN = (1 << 10),
    PEEP_ITEM_RUBBISH = (1 << 11),
    PEEP_ITEM_EMPTY_BURGER_BOX = (1 << 12),
    PEEP_ITEM_PIZZA = (1 << 13),
    PEEP_ITEM_VOUCHER = (1 << 14),
    PEEP_ITEM_POPCORN = (1 << 15),
    PEEP_ITEM_HOT_DOG = (1 << 16),
    PEEP_ITEM_TENTACLE = (1 << 17),
    PEEP_ITEM_HAT = (1 << 18),
    PEEP_ITEM_CANDY_APPLE = (1 << 19),
    PEEP_ITEM_TSHIRT = (1 << 20),
    PEEP_ITEM_DONUT = (1 << 21),
    PEEP_ITEM_COFFEE = (1 << 22),
    PEEP_ITEM_EMPTY_CUP = (1 << 23),
    PEEP_ITEM_CHICKEN = (1 << 24),
    PEEP_ITEM_LEMONADE = (1 << 25),
    PEEP_ITEM_EMPTY_BOX = (1 << 26),
    PEEP_ITEM_EMPTY_BOTTLE = (1 << 27),

    // item_extra_flags
    PEEP_ITEM_PHOTO2 = (1 << 0),
    PEEP_ITEM_PHOTO3 = (1 << 1),
    PEEP_ITEM_PHOTO4 = (1 << 2),
    PEEP_ITEM_PRETZEL = (1 << 3),
    PEEP_ITEM_CHOCOLATE = (1 << 4),
    PEEP_ITEM_ICED_TEA = (1 << 5),
    PEEP_ITEM_FUNNEL_CAKE = (1 << 6),
    PEEP_ITEM_SUNGLASSES = (1 << 7),
    PEEP_ITEM_BEEF_NOODLES = (1 << 8),
    PEEP_ITEM_FRIED_RICE_NOODLES = (1 << 9),
    PEEP_ITEM_WONTON_SOUP = (1 << 10),
    PEEP_ITEM_MEATBALL_SOUP = (1 << 11),
    PEEP_ITEM_FRUIT_JUICE = (1 << 12),
    PEEP_ITEM_SOYBEAN_MILK = (1 << 13),
    PEEP_ITEM_SU_JONGKWA = (1 << 14),
    PEEP_ITEM_SUB_SANDWICH = (1 << 15),
    PEEP_ITEM_COOKIE = (1 << 16),
    PEEP_ITEM_EMPTY_BOWL_RED = (1 << 17),
    PEEP_ITEM_EMPTY_DRINK_CARTON = (1 << 18),
    PEEP_ITEM_EMPTY_JUICE_CUP = (1 << 19),
    PEEP_ITEM_ROAST_SAUSAGE = (1 << 20),
    PEEP_ITEM_EMPTY_BOWL_BLUE = (1 << 21)
};

enum PEEP_SPRITE_TYPE {
    PEEP_SPRITE_TYPE_NORMAL = 0,
    PEEP_SPRITE_TYPE_HANDYMAN = 1,
    PEEP_SPRITE_TYPE_MECHANIC = 2,
    PEEP_SPRITE_TYPE_SECURITY = 3,
    PEEP_SPRITE_TYPE_ENTERTAINER_PANDA = 4,
    PEEP_SPRITE_TYPE_ENTERTAINER_TIGER = 5,
    PEEP_SPRITE_TYPE_ENTERTAINER_ELEPHANT = 6,
    PEEP_SPRITE_TYPE_ENTERTAINER_ROMAN = 7,
    PEEP_SPRITE_TYPE_ENTERTAINER_GORILLA = 8,
    PEEP_SPRITE_TYPE_ENTERTAINER_SNOWMAN = 9,
    PEEP_SPRITE_TYPE_ENTERTAINER_KNIGHT = 10,
    PEEP_SPRITE_TYPE_ENTERTAINER_ASTRONAUT = 11,
    PEEP_SPRITE_TYPE_ENTERTAINER_BANDIT = 12,
    PEEP_SPRITE_TYPE_ENTERTAINER_SHERIFF = 13,
    PEEP_SPRITE_TYPE_ENTERTAINER_PIRATE = 14,
    PEEP_SPRITE_TYPE_ICE_CREAM = 15,
    PEEP_SPRITE_TYPE_CHIPS = 16,
    PEEP_SPRITE_TYPE_BURGER = 17,
    PEEP_SPRITE_TYPE_DRINK = 18,
    PEEP_SPRITE_TYPE_BALLOON = 19,
    PEEP_SPRITE_TYPE_CANDYFLOSS = 20,
    PEEP_SPRITE_TYPE_UMBRELLA = 21,
    PEEP_SPRITE_TYPE_PIZZA = 22,
    PEEP_SPRITE_TYPE_SECURITY_ALT = 23,
    PEEP_SPRITE_TYPE_POPCORN = 24,
    PEEP_SPRITE_TYPE_ARMS_CROSSED = 25,
    PEEP_SPRITE_TYPE_HEAD_DOWN = 26,
    PEEP_SPRITE_TYPE_NAUSEOUS = 27,
    PEEP_SPRITE_TYPE_VERY_NAUSEOUS = 28,
    PEEP_SPRITE_TYPE_REQUIRE_BATHROOM = 29,
    PEEP_SPRITE_TYPE_HAT = 30,
    PEEP_SPRITE_TYPE_HOT_DOG = 31,
    PEEP_SPRITE_TYPE_TENTACLE = 32,
    PEEP_SPRITE_TYPE_TOFFEE_APPLE = 33,
    PEEP_SPRITE_TYPE_DONUT = 34,
    PEEP_SPRITE_TYPE_COFFEE = 35,
    PEEP_SPRITE_TYPE_CHICKEN = 36,
    PEEP_SPRITE_TYPE_LEMONADE = 37,
    PEEP_SPRITE_TYPE_WATCHING = 38,
    PEEP_SPRITE_TYPE_PRETZEL = 39,
    PEEP_SPRITE_TYPE_SUNGLASSES = 40,
    PEEP_SPRITE_TYPE_SU_JONGKWA = 41,
    PEEP_SPRITE_TYPE_JUICE = 42,
    PEEP_SPRITE_TYPE_FUNNEL_CAKE = 43,
    PEEP_SPRITE_TYPE_NOODLES = 44,
    PEEP_SPRITE_TYPE_SAUSAGE = 45,
    PEEP_SPRITE_TYPE_SOUP = 46,
    PEEP_SPRITE_TYPE_SANDWICH = 47,
    PEEP_SPRITE_TYPE_COUNT
};

// Flags used by peep->window_invalidate_flags
enum PEEP_INVALIDATE {
    PEEP_INVALIDATE_PEEP_THOUGHTS = 1,
    PEEP_INVALIDATE_PEEP_STATS = 1 << 1,
    PEEP_INVALIDATE_PEEP_2 = 1 << 2,
    PEEP_INVALIDATE_PEEP_INVENTORY = 1 << 3,
    PEEP_INVALIDATE_STAFF_STATS = 1 << 4,
};

// Flags used by peep_should_go_on_ride()
enum PEEP_RIDE_DECISION {
    PEEP_RIDE_DECISION_AT_QUEUE = 1,
    PEEP_RIDE_DECISION_THINKING = 1 << 2
};

#pragma pack(push, 1)
typedef struct rct_peep_thought {
    uint8 type;     //0
    uint8 item;     //1
    uint8 var_2;    //2
    uint8 var_3;    //3
} rct_peep_thought;
assert_struct_size(rct_peep_thought, 4);

typedef struct rct_peep {
    uint8 sprite_identifier;        // 0x00
    uint8 var_01;
    uint16 next_in_quadrant;        // 0x02
    uint16 next;                    // 0x04
    uint16 previous;                // 0x06
    uint8 linked_list_type_offset;  // 0x08 Valid values are SPRITE_LINKEDLIST_OFFSET_...
    // Height from centre of sprite to bottom
    uint8 sprite_height_negative;   // 0x09
    uint16 sprite_index;            // 0x0A
    uint16 flags;           // 0x0C
    sint16 x;                       // 0x0E
    sint16 y;                       // 0x10
    sint16 z;                       // 0x12
    // Width from centre of sprite to edge
    uint8 sprite_width;             // 0x14
    // Height from centre of sprite to top
    uint8 sprite_height_positive;   // 0x15
    sint16 sprite_left;             // 0x16
    sint16 sprite_top;              // 0x18
    sint16 sprite_right;            // 0x1A
    sint16 sprite_bottom;           // 0x1C
    uint8 sprite_direction;         // 0x1E
    union {
        uint8 pad_1F[3];
#ifdef STOUT_PEEPS_EXPANDED_EXPERIMENT
        struct {
            uint8 peeps_ex_direction_preference;
            uint8 pad_1F_[2];
        };
#endif
    };
    rct_string_id name_string_idx;  // 0x22
    uint16 next_x;                  // 0x24
    uint16 next_y;                  // 0x26
    uint8 next_z;                   // 0x28
    uint8 next_var_29;              // 0x29
    uint8 outside_of_park;          // 0x2A
    uint8 state;                    // 0x2B
    uint8 sub_state;                // 0x2C
    uint8 sprite_type;              // 0x2D
    uint8 type;                     // 0x2E
    union{
        uint8 staff_type;           // 0x2F
        uint8 no_of_rides;          // 0x2F
    };
    uint8 tshirt_colour;            // 0x30
    uint8 trousers_colour;          // 0x31
    uint16 destination_x;           // 0x32 Location that the peep is trying to get to
    uint16 destination_y;           // 0x34
    uint8 destination_tolerence;    // 0x36 How close to destination before next action/state 0 = exact
    uint8 var_37;
    uint8 energy;                   // 0x38
    uint8 energy_growth_rate;       // 0x39
    uint8 happiness;                // 0x3A
    uint8 happiness_growth_rate;    // 0x3B
    uint8 nausea;                   // 0x3C
    uint8 nausea_growth_rate;       // 0x3D
    uint8 hunger;                   // 0x3E
    uint8 thirst;                   // 0x3F
    uint8 bathroom;                 // 0x40
    uint8 var_41;
    uint8 var_42;
    uint8 intensity;                // 0x43 The max intensity is stored in the first 4 bits, and the min intensity in the second 4 bits
    uint8 nausea_tolerance;         // 0x44
    uint8 window_invalidate_flags;  // 0x45
    money16 paid_on_drink;          // 0x46
    uint8 ride_types_been_on[16];   // 0x48
    uint32 item_extra_flags;        // 0x58
    uint8 photo2_ride_ref;          // 0x5C
    uint8 photo3_ride_ref;          // 0x5D
    uint8 photo4_ride_ref;          // 0x5E
    uint8 pad_5F[0x09];             // 0x5F
    uint8 current_ride;             // 0x68
    uint8 current_ride_station;     // 0x69
    uint8 current_train;            // 0x6A
    union{
        struct{
            uint8 current_car;      // 0x6B
            uint8 current_seat;     // 0x6C
        };
        uint16 time_to_sitdown;     //0x6B
        struct{
            uint8 time_to_stand;    //0x6B
            uint8 standing_flags;   //0x6C
        };
    };
    // Normally 0, 1 for carrying sliding board on spiral slide ride, 2 for carrying lawn mower
    uint8 special_sprite;           // 0x6D
    uint8 action_sprite_type;       // 0x6E
    // Seems to be used like a local variable, as it's always set before calling peep_switch_to_next_action_sprite_type, which reads this again
    uint8 next_action_sprite_type;    // 0x6F
    uint8 action_sprite_image_offset; // 0x70
    uint8 action;                   // 0x71
    uint8 action_frame;             // 0x72
    uint8 var_73;
    union {
        uint16 var_74; // time getting to ride to fix
        uint16 next_in_queue;       // 0x74
    };
    uint8 var_76;
    union {
        uint8 pad_77;
#ifdef STOUT_PEEPS_EXPANDED_EXPERIMENT
        uint8 peeps_ex_queue_wait_distance;		// Messy Queue
#endif
    };
    union{
        uint8 maze_last_edge;           // 0x78
        uint8 direction;    //Direction ?
    };
    uint8 interaction_ride_index;
    uint16 time_in_queue;           // 0x7A
    uint8 rides_been_on[32];        // 0x7C
    // 255 bit bitmap of every ride the peep has been on see
    // window_peep_rides_update for how to use.
    uint32 id;                      // 0x9C
    money32 cash_in_pocket;         // 0xA0
    money32 cash_spent;             // 0xA4
    sint32 time_in_park;            // 0xA8
    sint8 var_AC;                   // 0xAC
    uint8 previous_ride;            // 0xAD
    uint16 previous_ride_time_out;  // 0xAE
    rct_peep_thought thoughts[PEEP_MAX_THOUGHTS];   // 0xB0
    uint8 var_C4;                   // 0xC4 has something to do with peep falling, see peep.checkForPath
    union {
        uint8 staff_id;                     // 0xC5
        uint8 guest_heading_to_ride_id;     // 0xC5
    };
    union {
        uint8 staff_orders;             // 0xC6
        uint8 peep_is_lost_countdown;   // 0xC6
    };
    uint8 photo1_ride_ref;          // 0xC7
    uint32 peep_flags;              // 0xC8
    rct_xyzd8 pathfind_goal;        // 0xCC
    rct_xyzd8 pathfind_history[4];  // 0xD0
    uint8 no_action_frame_no;       // 0xE0
    // 0x3F Litter Count split into lots of 3 with time, 0xC0 Time since last recalc
    uint8 litter_count;             // 0xE1
    union{
        uint8 time_on_ride;         // 0xE2
        uint8 var_E2;               // 0xE2
    };
    // 0x3F Sick Count split into lots of 3 with time, 0xC0 Time since last recalc
    uint8 disgusting_count;         // 0xE3
    union{
        money16 paid_to_enter;          // 0xE4
        uint16 staff_lawns_mown;        // 0xE4
        uint16 staff_rides_fixed;       // 0xE4
    };
    union{
        money16 paid_on_rides;          // 0xE6
        uint16 staff_gardens_watered;   // 0xE6
        uint16 staff_rides_inspected;   // 0xE6
    };
    union {
        money16 paid_on_food;           // 0xE8
        uint16 staff_litter_swept;      // 0xE8
    };
    union{
        money16 paid_on_souvenirs;      // 0xEA
        uint16 staff_bins_emptied;      // 0xEA
    };
    uint8 no_of_food;               // 0xEC
    uint8 no_of_drinks;             // 0xED
    uint8 no_of_souvenirs;          // 0xEE
    uint8 var_EF;
    uint8 voucher_type;             // 0xF0
    uint8 voucher_arguments;        // 0xF1 ride_id or string_offset_id
    uint8 var_F2;
    uint8 angriness;
    uint8 var_F4;
    uint8 days_in_queue;            // 0xF5
    uint8 balloon_colour;           // 0xF6
    uint8 umbrella_colour;          // 0xF7
    uint8 hat_colour;               // 0xF8
    uint8 favourite_ride;           // 0xF9
    uint8 favourite_ride_rating;    // 0xFA
    union {
        uint8 pad_FB;
#ifdef STOUT_PEEPS_EXPANDED_EXPERIMENT
        uint8 peeps_ex_crowded_store;			// Store crowdedness
#endif
    };
    uint32 item_standard_flags;     // 0xFC
} rct_peep;
assert_struct_size(rct_peep, 0x100);
#pragma pack(pop)

typedef struct rct_sprite_bounds {
    uint8 sprite_width;             // 0x00
    uint8 sprite_height_negative;   // 0x01
    uint8 sprite_height_positive;   // 0x02
} rct_sprite_bounds;

typedef struct rct_peep_animation {
    uint32 base_image;   // 0x00
    uint32 num_frames;
    const uint8* frame_offsets;
} rct_peep_animation;

typedef struct rct_peep_animation_entry {
    const rct_peep_animation *sprite_animation;      // 0x00
    const rct_sprite_bounds *sprite_bounds;    // 0x04
} rct_peep_animation_entry;

// rct2: 0x00982708
extern rct_peep_animation_entry g_peep_animation_entries[PEEP_SPRITE_TYPE_COUNT];

enum {
    EASTEREGG_PEEP_NAME_MICHAEL_SCHUMACHER,
    EASTEREGG_PEEP_NAME_JACQUES_VILLENEUVE,
    EASTEREGG_PEEP_NAME_DAMON_HILL,
    EASTEREGG_PEEP_NAME_MR_BEAN,
    EASTEREGG_PEEP_NAME_CHRIS_SAWYER,
    EASTEREGG_PEEP_NAME_KATIE_BRAYSHAW,
    EASTEREGG_PEEP_NAME_MELANIE_WARN,
    EASTEREGG_PEEP_NAME_SIMON_FOSTER,
    EASTEREGG_PEEP_NAME_JOHN_WARDLEY,
    EASTEREGG_PEEP_NAME_LISA_STIRLING,
    EASTEREGG_PEEP_NAME_DONALD_MACRAE,
    EASTEREGG_PEEP_NAME_KATHERINE_MCGOWAN,
    EASTEREGG_PEEP_NAME_FRANCES_MCGOWAN,
    EASTEREGG_PEEP_NAME_CORINA_MASSOURA,
    EASTEREGG_PEEP_NAME_CAROL_YOUNG,
    EASTEREGG_PEEP_NAME_MIA_SHERIDAN,
    EASTEREGG_PEEP_NAME_KATIE_RODGER,
    EASTEREGG_PEEP_NAME_EMMA_GARRELL,
    EASTEREGG_PEEP_NAME_JOANNE_BARTON,
    EASTEREGG_PEEP_NAME_FELICITY_ANDERSON,
    EASTEREGG_PEEP_NAME_KATIE_SMITH,
    EASTEREGG_PEEP_NAME_EILIDH_BELL,
    EASTEREGG_PEEP_NAME_NANCY_STILLWAGON,
    EASTEREGG_PEEP_NAME_DAVID_ELLIS
};

/** Helper macro until rides are stored in this module. */
#define GET_PEEP(sprite_index) &(get_sprite(sprite_index)->peep)

/**
 * Helper macro loop for enumerating through all the peeps. To avoid needing a end loop counterpart, statements are
 * applied in tautology if statements.
 */
#define FOR_ALL_PEEPS(sprite_index, peep) \
    for (sprite_index = gSpriteListHead[SPRITE_LIST_PEEP]; sprite_index != SPRITE_INDEX_NULL; sprite_index = peep->next) \
        if ((peep = GET_PEEP(sprite_index)) != NULL || 1)

#define FOR_ALL_GUESTS(sprite_index, peep) \
    FOR_ALL_PEEPS(sprite_index, peep) \
        if (peep->type == PEEP_TYPE_GUEST)

#define FOR_ALL_STAFF(sprite_index, peep) \
    FOR_ALL_PEEPS(sprite_index, peep) \
        if (peep->type == PEEP_TYPE_STAFF)

extern uint8 gGuestChangeModifier;
extern uint16 gNumGuestsInPark;
extern uint16 gNumGuestsInParkLastWeek;
extern uint16 gNumGuestsHeadingForPark;

extern money16 gGuestInitialCash;
extern uint8 gGuestInitialHappiness;
extern uint8 gGuestInitialHunger;
extern uint8 gGuestInitialThirst;

extern uint32 gNextGuestNumber;

extern uint8 gPeepWarningThrottle[16];

extern rct_xyz16 gPeepPathFindGoalPosition;
extern bool gPeepPathFindIgnoreForeignQueues;
extern uint8 gPeepPathFindQueueRideIndex;

rct_peep * try_get_guest(uint16 spriteIndex);
sint32 peep_get_staff_count();
sint32 peep_can_be_picked_up(rct_peep* peep);
void peep_update_all();
void peep_problem_warnings_update();
void peep_stop_crowd_noise();
void peep_update_crowd_noise();
void peep_update_days_in_queue();
void peep_applause();
rct_peep *peep_generate(sint32 x, sint32 y, sint32 z);
void get_arguments_from_action(rct_peep* peep, uint32 *argument_1, uint32* argument_2);
void peep_thought_set_format_args(rct_peep_thought *thought);
sint32 get_peep_face_sprite_small(rct_peep *peep);
sint32 get_peep_face_sprite_large(rct_peep *peep);
sint32 peep_check_easteregg_name(sint32 index, rct_peep *peep);
sint32 peep_get_easteregg_name_id(rct_peep *peep);
sint32 peep_is_mechanic(rct_peep *peep);
bool peep_has_item(rct_peep *peep, sint32 peepItem);
sint32 peep_has_food(rct_peep* peep);
void peep_pickup(rct_peep* peep);
void peep_pickup_abort(rct_peep* peep, sint32 old_x);
bool peep_pickup_place(rct_peep* peep, sint32 x, sint32 y, sint32 z, bool apply);
bool peep_pickup_command(uint32 peepnum, sint32 x, sint32 y, sint32 z, sint32 action, bool apply);
void game_command_pickup_guest(sint32* eax, sint32* ebx, sint32* ecx, sint32* edx, sint32* esi, sint32* edi, sint32* ebp);
void peep_sprite_remove(rct_peep* peep);
void peep_remove(rct_peep* peep);
void peep_update_sprite_type(rct_peep* peep);

void peep_window_state_update(rct_peep* peep);
void peep_decrement_num_riders(rct_peep* peep);
/**
* rct2: 0x699F5A
* al:thought_type
* ah:thought_arguments
* esi: peep
*/
void peep_insert_new_thought(rct_peep *peep, uint8 thought_type, uint8 thought_arguments);

void peep_set_map_tooltip(rct_peep *peep);
void peep_update_current_action_sprite_type(rct_peep* peep);
void remove_peep_from_ride(rct_peep* peep);
void remove_peep_from_queue(rct_peep* peep);

void peep_switch_to_special_sprite(rct_peep* peep, uint8 special_sprite_id);
void peep_update_name_sort(rct_peep *peep);
void peep_sort();
void peep_update_names(bool realNames);

money32 set_peep_name(sint32 flags, sint32 state, uint16 sprite_index, uint8* text_1, uint8* text_2, uint8* text_3);
void game_command_set_guest_name(sint32 *eax, sint32 *ebx, sint32 *ecx, sint32 *edx, sint32 *esi, sint32 *edi, sint32 *ebp);

sint32 peep_pathfind_choose_direction(sint16 x, sint16 y, uint8 z, rct_peep *peep);
void peep_reset_pathfind_goal(rct_peep *peep);

bool is_valid_path_z_and_direction(rct_map_element *mapElement, sint32 currentZ, sint32 currentDirection);

#if defined(DEBUG_LEVEL_1) && DEBUG_LEVEL_1
#define PATHFIND_DEBUG 0 // Set to 0 to disable pathfinding debugging;
             // Set to 1 to enable pathfinding debugging.
// Some variables used for the path finding debugging.
extern bool gPathFindDebug; // Use to guard calls to log messages
extern utf8 gPathFindDebugPeepName[256]; // Use to put the peep name in the log message

// The following calls set the above two variables for a peep.
// ... when PATHFIND_DEBUG is 1 (non zero)
void pathfind_logging_enable(rct_peep* peep);
void pathfind_logging_disable();
#endif // defined(DEBUG_LEVEL_1) && DEBUG_LEVEL_1

void peep_autoposition(rct_peep *newPeep);

void increment_guests_in_park();
void increment_guests_heading_for_park();
void decrement_guests_in_park();
void decrement_guests_heading_for_park();

#endif
