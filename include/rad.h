/**
 * @file rad.h
 *
 * @brief Public API for the Rad libraries
 */

/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef ZEPHYR_INCLUDE_RAD_H_
#define ZEPHYR_INCLUDE_RAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <device.h>

/**
 * A *_LEN_PULSES is the number of elapsed-time measurements required to describe a message,
 * including the start pulse.
 *
 * A *_START_PULSE_LEN_US is the expected length of the start pulse.
 *
 * A *_LINE_CLEAR_LEN_US is the expected amount of time that needs to elapse after the final
 * active pulse in order to decide that the message is finished.
 */
#define RAD_MSG_TYPE_RAD_LEN_PULSES             33
#define RAD_MSG_TYPE_RAD_LEN_IR_BITS            16
#define RAD_MSG_TYPE_RAD_START_PULSE_LEN_US     1580
#define RAD_MSG_TYPE_RAD_LINE_CLEAR_LEN_US      860
#define RAD_MSG_TYPE_RAD_0_PULSE_LEN_US         390
#define RAD_MSG_TYPE_RAD_1_PULSE_LEN_US         780
#define RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US    390
#define RAD_MSG_TYPE_RAD_0_BIT_LEN_US           (RAD_MSG_TYPE_RAD_0_PULSE_LEN_US + \
	                                              RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US)
#define RAD_MSG_TYPE_RAD_1_BIT_LEN_US           (RAD_MSG_TYPE_RAD_1_PULSE_LEN_US + \
                                                  RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US)

#define RAD_MSG_TYPE_DYNASTY_LEN_PULSES         41
#define RAD_MSG_TYPE_DYNASTY_LEN_IR_BITS        40
#define RAD_MSG_TYPE_DYNASTY_START_PULSE_LEN_US 1660
#define RAD_MSG_TYPE_DYNASTY_LINE_CLEAR_LEN_US  900
#define RAD_MSG_TYPE_DYNASTY_0_PULSE_LEN_US     412
#define RAD_MSG_TYPE_DYNASTY_1_PULSE_LEN_US     747

#define RAD_MSG_TYPE_LASER_X_LEN_PULSES         17
#define RAD_MSG_TYPE_LASER_X_LEN_IR_BITS		8
#define RAD_MSG_TYPE_LASER_X_START_PULSE_LEN_US 5950
#define RAD_MSG_TYPE_LASER_X_LINE_CLEAR_LEN_US  600
#define RAD_MSG_TYPE_LASER_X_SPACE_PULSE_LEN_US 450
#define RAD_MSG_TYPE_LASER_X_0_PULSE_LEN_US     550
#define RAD_MSG_TYPE_LASER_X_1_PULSE_LEN_US     1525
#define RAD_MSG_TYPE_LASER_X_0_BIT_LEN_US       (RAD_MSG_TYPE_LASER_X_SPACE_PULSE_LEN_US + \
                                                  RAD_MSG_TYPE_LASER_X_0_PULSE_LEN_US)
#define RAD_MSG_TYPE_LASER_X_1_BIT_LEN_US       (RAD_MSG_TYPE_LASER_X_SPACE_PULSE_LEN_US + \
                                                  RAD_MSG_TYPE_LASER_X_1_PULSE_LEN_US)

typedef enum
{
	RAD_MSG_TYPE_RAD,
	RAD_MSG_TYPE_DYNASTY,
	RAD_MSG_TYPE_LASER_X,
	RAD_MSG_TYPE_COUNT
} rad_msg_type_t;

#if CONFIG_RAD_MSG_TYPE_RAD
typedef struct
{
	uint8_t damage		: 4;
	uint8_t special 	: 4;
	uint8_t player_id	: 4;
	uint8_t team_id		: 2;
	uint8_t reserved	: 2;
} rad_msg_rad_t;
#endif /* #if CONFIG_RAD_MSG_TYPE_RAD */

#if CONFIG_RAD_MSG_TYPE_LASER_X
typedef enum
{
	TEAM_ID_LASER_X_BLUE    = 0x51,
	TEAM_ID_LASER_X_RED     = 0x52,
	TEAM_ID_LASER_X_NEUTRAL = 0x53
} team_id_laser_x_t;

typedef struct
{
	uint8_t team_id;
} rad_msg_laser_x_t;
#endif /* CONFIG_RAD_MSG_TYPE_LASER_X */

#if CONFIG_RAD_MSG_TYPE_DYNASTY
typedef enum
{
	TEAM_ID_DYNASTY_BLUE  = 1,
	TEAM_ID_DYNASTY_RED   = 2,
	TEAM_ID_DYNASTY_GREEN = 3,
    TEAM_ID_DYNASTY_WHITE = 4
} team_id_dynasty_t;

typedef enum
{
	WEAPON_ID_DYNASTY_PISTOL      = 1,
	WEAPON_ID_DYNASTY_SHOTGUN_SMG = 2,
	WEAPON_ID_DYNASTY_ROCKET      = 3
} weapon_id_dynasty_t;

typedef struct
{
	uint8_t team_id;
	uint8_t weapon_id;
	uint8_t checksum;
} rad_msg_dynasty_t;
#endif /* CONFIG_RAD_MSG_TYPE_DYNASTY */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_H_ */
 