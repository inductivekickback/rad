/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#include <drivers/rad_tx.h>

#if CONFIG_RAD_MSG_TYPE_RAD
LOG_MODULE_REGISTER(rad_message_type_rad, CONFIG_RAD_MSG_TYPE_RAD_LOG_LEVEL);

#if CONFIG_RAD_RX_ACCEPT_RAD
#include <drivers/rad_rx.h>

rad_parse_state_t rad_msg_type_rad_parse(uint32_t      *message,
	                                     uint32_t       len,
	                                     rad_msg_rad_t *msg)
{
    /**
     * NOTE: The start pulse length is validated before this function is called so
     *       parsing effectively starts at index 1.
     *
     * NOTE: Message len is validated before calling this function.
     *
     * Each message is a start pulse followed by sixteen bits:
     *     START: Active pulse of ~1.58ms (60 periods @ 38KHz)
     *     0:     Inactive for ~0.39ms (15 periods) followed by an active pulse of ~0.39ms
     *     1:     Inactive for ~0.78ms (30 periods) followed by an active pulse of ~0.39ms
     */
	int i=1;

	msg->reserved = 0;
	for (int j=1; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_0_PULSE_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->reserved |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->team_id = 0;
	for (int j=1; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_0_PULSE_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->team_id |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->player_id = 0;
	for (int j=3; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_0_PULSE_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->player_id |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->special = 0;
	for (int j=3; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_0_PULSE_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->special |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	msg->damage = 0;
	for (int j=3; j>=0; i+=2,j--) {
	    if (!IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US)) {
	    	return RAD_PARSE_STATE_INVALID;
	    }
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_0_PULSE_LEN_US)) {
            // This is a zero bit.
        } else if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_RAD_1_PULSE_LEN_US)) {
            // This is a one bit.
            msg->damage |= (1<<j);
        } else {
        	return RAD_PARSE_STATE_INVALID;
        }
	}

	return RAD_PARSE_STATE_VALID;
}

#endif /* CONFIG_RAD_RX_ACCEPT_RAD */

#if CONFIG_RAD_TX_RAD
#include <drivers/rad_tx.h>

#define ADD_0_BIT(p_values) do { \
for (int i=0; i < RAD_TX_RAD_0_PULSE_LEN_PWM_VALUES; i++) \
{ \
    *p_values++ = RAD_TX_DUTY_CYCLE_0; \
} \
for (int i=0; i < RAD_TX_RAD_ACTIVE_PULSE_LEN_PWM_VALUES; i++) { \
    *p_values++ = RAD_TX_DUTY_CYCLE_50; \
} \
} while (0)

#define ADD_1_BIT(p_values) do { \
for (int i=0; i < RAD_TX_RAD_1_PULSE_LEN_PWM_VALUES; i++) \
{ \
    *p_values++ = RAD_TX_DUTY_CYCLE_0; \
} \
for (int i=0; i < RAD_TX_RAD_ACTIVE_PULSE_LEN_PWM_VALUES; i++) { \
    *p_values++ = RAD_TX_DUTY_CYCLE_50; \
} \
} while (0)

int rad_msg_type_rad_encode(rad_msg_rad_t *msg, nrf_pwm_values_common_t *values, uint32_t *len)
{
    nrf_pwm_values_common_t *p_values = values;

    if (*len < RAD_TX_RAD_MAX_MSG_LEN_PWM_VALUES) {
        return -ENOMEM;
    }

    for (int i=0; i < RAD_TX_RAD_START_PULSE_LEN_PWM_VALUES; i++) {
        *p_values = RAD_TX_DUTY_CYCLE_50;
        p_values++;
    }

    for (int j=1; j>=0; j--) {
        if (msg->reserved & (1<<j)) {
            ADD_1_BIT(p_values);
        } else {
            ADD_0_BIT(p_values);
        }
    }

    for (int j=1; j>=0; j--) {
        if (msg->team_id & (1<<j)) {
            ADD_1_BIT(p_values);
        } else {
            ADD_0_BIT(p_values);
        }
    }

    for (int j=3; j>=0; j--) {
        if (msg->player_id & (1<<j)) {
            ADD_1_BIT(p_values);
        } else {
            ADD_0_BIT(p_values);
        }
    }

    for (int j=3; j>=0; j--) {
        if (msg->special & (1<<j)) {
            ADD_1_BIT(p_values);
        } else {
            ADD_0_BIT(p_values);
        }
    }

    for (int j=3; j>=0; j--) {
        if (msg->damage & (1<<j)) {
            ADD_1_BIT(p_values);
        } else {
            ADD_0_BIT(p_values);
        }
    }

    *p_values = RAD_TX_DUTY_CYCLE_0;
    p_values++;

    *len = (p_values - values);
    return 0;
}

#endif /* CONFIG_RAD_TX_RAD */

#endif /* CONFIG_RAD_MSG_TYPE_RAD */
