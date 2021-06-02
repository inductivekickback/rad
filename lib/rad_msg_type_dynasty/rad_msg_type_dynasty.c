/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#define PISTOL_CHECKSUM_ADD	     5
#define SHOTGUN_CHECKSUM_ADD     6
#define SHOTGUN_IRR_CHECKSUM_ADD 7
#define ROCKET_CHECKSUM_ADD      7
#define ROCKET_IRR_CHECKSUM_ADD  8

#define COMMON_PREAMBLE          170
#define INVALID_CHECKSUM         0xFF

#if CONFIG_RAD_MSG_TYPE_DYNASTY
LOG_MODULE_REGISTER(rad_message_type_dynasty, CONFIG_RAD_MSG_TYPE_DYNASTY_LOG_LEVEL);

#if CONFIG_RAD_RX_ACCEPT_DYNASTY
#include <drivers/rad_rx.h>

static uint8_t checksum_calc(team_id_dynasty_t team_id, weapon_id_dynasty_t weapon_id)
{
    switch (weapon_id) {
    case WEAPON_ID_DYNASTY_PISTOL:
        return (team_id + PISTOL_CHECKSUM_ADD);
    case WEAPON_ID_DYNASTY_SHOTGUN_SMG:
        switch (team_id) {
        case TEAM_ID_DYNASTY_BLUE:
        case TEAM_ID_DYNASTY_RED:
        case TEAM_ID_DYNASTY_GREEN:         
            return (team_id + SHOTGUN_CHECKSUM_ADD);
        case TEAM_ID_DYNASTY_WHITE:
            return (team_id + SHOTGUN_IRR_CHECKSUM_ADD);
        default:
            break;
        }
        break;
    case WEAPON_ID_DYNASTY_ROCKET:
        switch (team_id) {
        case TEAM_ID_DYNASTY_BLUE:
        case TEAM_ID_DYNASTY_RED:
            return (team_id + ROCKET_CHECKSUM_ADD);
        case TEAM_ID_DYNASTY_GREEN:
        case TEAM_ID_DYNASTY_WHITE:
            return (team_id + ROCKET_IRR_CHECKSUM_ADD);
        default:
            break;
        }
        break;
    default:
        break;
    }
    return INVALID_CHECKSUM;
}

rad_parse_state_t rad_msg_type_dynasty_parse(uint32_t          *message,
	                                         uint32_t           len,
	                                         rad_msg_dynasty_t *msg)
{
    /**
     * NOTE: The start pulse length is validated before this function is called so
     *       parsing effectively starts at index 1.
     *
     * NOTE: Message len is validated before calling this function.
     *
     * Each message is a start pulse followed by a 16-bit preamble and then twenty-four bits:
     *     START:    Active pulse of ~1.66ms
     *     PREAMBLE: 0b0000000010101010
     *     0:        Inactive or active for ~0.4ms (can be as long as 0.58ms)
     *     1:        Inactive or active for ~0.75ms (can be as short as 0.61ms)
     *
     *     This blaster's pulse lengths are quite sloppy so parsing is relaxed.
     */
    int      i       =1;
    uint16_t preamble=0;

    for (int j=15; j>=0; i++,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_1_PULSE_LEN_US)) {
            // This is a one bit.
            preamble |= (1<<j);
        }
    }

    if (COMMON_PREAMBLE != preamble) {
    	return RAD_PARSE_STATE_INVALID;
    }

    msg->team_id = 0;
    for (int j=7; j>=0; i++,j--) {
		if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->team_id |= (1<<j);
        }
    }

    switch (msg->team_id) {
    case TEAM_ID_DYNASTY_BLUE:
    case TEAM_ID_DYNASTY_RED:
    case TEAM_ID_DYNASTY_GREEN:
    case TEAM_ID_DYNASTY_WHITE:
        break;
    default:
        return RAD_PARSE_STATE_INVALID;
    }

    msg->weapon_id = 0;
    for (int j=7; j>=0; i++,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->weapon_id |= (1<<j);
        }
    }

    msg->checksum = 0;
    for (int j=7; j>=0; i++,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_DYNASTY_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->checksum |= (1<<j);
        }
    }

    if (msg->checksum == checksum_calc(msg->team_id, msg->weapon_id)) {
        return RAD_PARSE_STATE_VALID;
    } else {
        return RAD_PARSE_STATE_INVALID;
    }
}

#endif /* CONFIG_RAD_RX_ACCEPT_DYNASTY */

#if CONFIG_RAD_TX_DYNASTY
#include <drivers/rad_tx.h>

#define ADD_0_BIT(p_values, duty_cycle) do { \
for (int i=0; i < RAD_TX_DYNASTY_0_PULSE_LEN_PWM_VALUES; i++) { \
    *p_values++ = duty_cycle; \
} \
} while (0)

#define ADD_1_BIT(p_values, duty_cyle) do { \
for (int i=0; i < RAD_TX_DYNASTY_1_PULSE_LEN_PWM_VALUES; i++) { \
    *p_values++ = duty_cyle; \
} \
} while (0)

int rad_msg_type_dynasty_encode(rad_msg_dynasty_t *msg,
                                  nrf_pwm_values_common_t *values,
                                  uint32_t *len)
{
    nrf_pwm_values_common_t *p_values = values;

    if (*len < RAD_TX_DYNASTY_MAX_MSG_LEN_PWM_VALUES) {
        return -ENOMEM;
    }

    for (int i=0; i < RAD_TX_DYNASTY_START_PULSE_LEN_PWM_VALUES; i++) {
        *p_values = RAD_TX_DUTY_CYCLE_50;
        p_values++;
    }

    /* Add the common prefix: 0b0000000010101010 */
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);

    /* Add the team_id byte. */
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);

    switch (msg->team_id) {
    case TEAM_ID_DYNASTY_BLUE:
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        break;
    case TEAM_ID_DYNASTY_RED:
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        break;
    case TEAM_ID_DYNASTY_GREEN:
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        break;
    case TEAM_ID_DYNASTY_WHITE:
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        break;
    default:
        return -1;
    }

    /* Add the weapon_id byte. */
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);

    switch (msg->weapon_id) {
    case WEAPON_ID_DYNASTY_PISTOL:
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        break;
    case WEAPON_ID_DYNASTY_SHOTGUN_SMG:
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        break;
    case WEAPON_ID_DYNASTY_ROCKET:
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
        break;
    default:
        return -1;
    }

    /* Add the checksum byte. */
    uint8_t checksum = checksum_calc(msg->team_id, msg->weapon_id);

    if (INVALID_CHECKSUM == checksum) {
        return -1;
    }

    if (checksum & (1<<7)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    }

    if (checksum & (1<<6)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    }

    if (checksum & (1<<5)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    }

    if (checksum & (1<<4)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    }

    if (checksum & (1<<3)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    }

    if (checksum & (1<<2)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    }

    if (checksum & (1<<1)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_0);
    }

    if (checksum & (1<<0)) {
        ADD_1_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    } else {
        ADD_0_BIT(p_values, RAD_TX_DUTY_CYCLE_50);
    }

    *p_values = RAD_TX_DUTY_CYCLE_0;
    p_values++;

    *len = (p_values - values);

    return 0;
}

#endif /* CONFIG_RAD_TX_DYNASTY */

#endif /* CONFIG_RAD_MSG_TYPE_DYNASTY */
