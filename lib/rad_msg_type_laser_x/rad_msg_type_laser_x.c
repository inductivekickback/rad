/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <sys/util.h>
#include <logging/log.h>

#if CONFIG_RAD_MSG_TYPE_DYNASTY
LOG_MODULE_REGISTER(rad_message_type_laser_x, CONFIG_RAD_MSG_TYPE_LASER_X_LOG_LEVEL);

#if CONFIG_RAD_RX_ACCEPT_LASER_X
#include <drivers/rad_rx.h>

rad_parse_state_t rad_msg_type_laser_x_parse(uint32_t          *message,
                                             uint32_t           len,
                                             rad_msg_laser_x_t *msg)
{
    /**
     * NOTE: The start pulse length is validated before this function is called so
     *       parsing effectively starts at index 1.
     *
     * NOTE: Message len is validated before calling this function.
     *
     * Each message is a start pulse followed by eight bits:
     *     START: Active pulse of ~5.95ms
     *     0:     Inactive for ~0.45ms followed by an active pulse of ~0.55ms
     *     1:     Inactive for ~0.45ms followed by an active pulse of ~1.5ms
     */
    msg->team_id = 0;
    for (int i=1,j=7; j>=0; i+=2,j--) {
        if (IS_VALID_BIT_PULSE(message[i], RAD_MSG_TYPE_LASER_X_SPACE_PULSE_LEN_US)) {
            if (IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_LASER_X_0_PULSE_LEN_US)) {
                // This is a zero bit.
                continue;
            } else if (IS_VALID_BIT_PULSE(message[i+1], RAD_MSG_TYPE_LASER_X_1_PULSE_LEN_US)) {
                // This is a one bit.
                msg->team_id |= (1<<j);
                continue;
            }
        }
        return RAD_PARSE_STATE_INVALID;
    }

    switch (msg->team_id) {
    case TEAM_ID_LASER_X_BLUE:
    case TEAM_ID_LASER_X_RED:
    case TEAM_ID_LASER_X_NEUTRAL:
        return RAD_PARSE_STATE_VALID;
    default:
        return RAD_PARSE_STATE_VALID;
    }
}
#endif /* CONFIG_RAD_RX_ACCEPT_LASER_X */

#if CONFIG_RAD_TX_LASER_X
#include <drivers/rad_tx.h>

#define ADD_0_BIT(p_values) do { \
for (int i=0; i < RAD_TX_LASER_X_SPACE_PULSE_PWM_VALUES; i++) \
{ \
    *p_values++ = RAD_TX_DUTY_CYCLE_0; \
} \
for (int i=0; i < RAD_TX_LASER_X_0_PULSE_LEN_PWM_VALUES; i++) { \
    *p_values++ = RAD_TX_DUTY_CYCLE_50; \
} \
} while (0)

#define ADD_1_BIT(p_values) do { \
for (int i=0; i < RAD_TX_LASER_X_SPACE_PULSE_PWM_VALUES; i++) \
{ \
    *p_values++ = RAD_TX_DUTY_CYCLE_0; \
} \
for (int i=0; i < RAD_TX_LASER_X_1_PULSE_LEN_PWM_VALUES; i++) { \
    *p_values++ = RAD_TX_DUTY_CYCLE_50; \
} \
} while (0)

int rad_msg_type_laser_x_encode(rad_msg_laser_x_t *msg,
                                  nrf_pwm_values_common_t *values,
                                  uint32_t *len)
{
    nrf_pwm_values_common_t *p_values = values;

    if (*len < RAD_TX_LASER_X_MAX_MSG_LEN_PWM_VALUES) {
        return -ENOMEM;
    }

    for (int i=0; i < RAD_TX_LASER_X_START_PULSE_LEN_PWM_VALUES; i++) {
        *p_values = RAD_TX_DUTY_CYCLE_50;
        p_values++;
    }

    // Add the common prefix.
    ADD_0_BIT(p_values);
    ADD_1_BIT(p_values);
    ADD_0_BIT(p_values);
    ADD_1_BIT(p_values);
    ADD_0_BIT(p_values);
    ADD_0_BIT(p_values);

    switch (msg->team_id) {
    case TEAM_ID_LASER_X_RED:
        ADD_1_BIT(p_values);
        ADD_0_BIT(p_values);
        break;
    case TEAM_ID_LASER_X_BLUE:
        ADD_0_BIT(p_values);
        ADD_1_BIT(p_values);
        break;
    case TEAM_ID_LASER_X_NEUTRAL:
        ADD_1_BIT(p_values);
        ADD_1_BIT(p_values);
        break;
    default:
        return -1;
    }

    *p_values = RAD_TX_DUTY_CYCLE_0;
    p_values++;

    *len = (p_values - values);
    return 0;
}

#endif /* CONFIG_RAD_TX_LASER_X */

#endif /* CONFIG_RAD_MSG_TYPE_DYNASTY */
