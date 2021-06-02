/**
 * @file rad_rx.h
 *
 * @brief Public API for the Rad receiver driver
 */

/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef ZEPHYR_INCLUDE_RAD_RX_H_
#define ZEPHYR_INCLUDE_RAD_RX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <device.h>

#include <rad.h>

#define RAD_RX_START_PULSE_MARGIN_US 500 /* A valid start pulse can be +/- this much. */
#define RAD_RX_BIT_MARGIN_US         125 /* A valid bit pulse can be +/- this much. */

#define RAD_RX_MSG_MAX_LEN           0
#define RAD_RX_LINE_CLEAR_LEN_US     0

typedef enum
{
    RAD_PARSE_STATE_WAIT_FOR_START_PULSE, /* A valid start pulse is required before parsing. */
    RAD_PARSE_STATE_INCOMPLETE,           /* Not enough of the message has been received. */
    RAD_PARSE_STATE_INVALID,              /* The message is not going to work out. */
    RAD_PARSE_STATE_VALID,                /* The message is valid. */
    RAD_PARSE_STATE_COUNT
} rad_parse_state_t;

#if CONFIG_RAD_MSG_TYPE_RAD
#if RAD_RX_MSG_MAX_LEN < RAD_MSG_TYPE_RAD_LEN_PULSES
#undef RAD_RX_MSG_MAX_LEN
#define RAD_RX_MSG_MAX_LEN RAD_MSG_TYPE_RAD_LEN_PULSES
#endif
#if CONFIG_RAD_RX_ACCEPT_RAD
rad_parse_state_t rad_msg_type_rad_parse(uint32_t *message,
                                           uint32_t len,
                                           rad_msg_rad_t *msg);
#if RAD_RX_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_RAD_LINE_CLEAR_LEN_US
#undef RAD_RX_LINE_CLEAR_LEN_US
#define RAD_RX_LINE_CLEAR_LEN_US RAD_MSG_TYPE_RAD_LINE_CLEAR_LEN_US
#endif
#endif /* CONFIG_RAD_RX_ACCEPT_RAD */
#endif /* CONFIG_RAD_MSG_TYPE_RAD */


#if CONFIG_RAD_MSG_TYPE_DYNASTY
#if RAD_RX_MSG_MAX_LEN < RAD_MSG_TYPE_DYNASTY_LEN_PULSES
#undef RAD_RX_MSG_MAX_LEN
#define RAD_RX_MSG_MAX_LEN RAD_MSG_TYPE_DYNASTY_LEN_PULSES
#endif
#if CONFIG_RAD_RX_ACCEPT_DYNASTY
rad_parse_state_t rad_msg_type_dynasty_parse(uint32_t *message,
                                               uint32_t len,
                                               rad_msg_dynasty_t *msg);
#if RAD_RX_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_DYNASTY_LINE_CLEAR_LEN_US
#undef RAD_RX_LINE_CLEAR_LEN_US
#define RAD_RX_LINE_CLEAR_LEN_US RAD_MSG_TYPE_DYNASTY_LINE_CLEAR_LEN_US
#endif
#endif /* CONFIG_RAD_RX_ACCEPT_DYNASTY */
#endif /* CONFIG_RAD_MSG_TYPE_DYNASTY */

#if CONFIG_RAD_MSG_TYPE_LASER_X
#if RAD_RX_MSG_MAX_LEN < RAD_MSG_TYPE_LASER_X_LEN_PULSES
#undef RAD_RX_MSG_MAX_LEN
#define RAD_RX_MAX_LEN RAD_MSG_TYPE_LASER_X_LEN_PULSES
#endif
#if CONFIG_RAD_RX_ACCEPT_LASER_X
rad_parse_state_t rad_msg_type_laser_x_parse(uint32_t *message,
                                               uint32_t len,
                                               rad_msg_laser_x_t *msg);
#if RAD_RX_LINE_CLEAR_LEN_US < RAD_MSG_TYPE_LASER_X_LINE_CLEAR_LEN_US
#undef RAD_RX_LINE_CLEAR_LEN_US
#define RAD_RX_LINE_CLEAR_LEN_US RAD_MSG_TYPE_LASER_X_LINE_CLEAR_LEN_US
#endif
#endif /* CONFIG_RAD_RX_ACCEPT_LASER_X */
#endif /* CONFIG_RAD_MSG_TYPE_LASER_X */

#if CONFIG_RAD_RX
#if RAD_RX_LINE_CLEAR_LEN_US == 0
#error No Rad RX message types enabled
#endif
#endif

/* This callback is called from the driver to notify the app that a message was received. */
typedef void (*rad_rx_callback_t) (rad_msg_type_t msg_type, void *data);

typedef int (*rad_rx_init_t)         (const struct device *dev);
typedef int (*rad_rx_set_callback_t) (const struct device *dev, rad_rx_callback_t cb);

/**
 * @brief Rad receiver driver API
 */
struct rad_rx_driver_api {
    rad_rx_init_t         init;
    rad_rx_set_callback_t set_callback;
};

static inline int rad_rx_init(const struct device *dev)
{
    struct rad_rx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_rx_driver_api*)dev->api;

    if (api->init == NULL) {
        return -ENOTSUP;
    }
    return api->init(dev);
}

static inline int rad_rx_set_callback(const struct device *dev, rad_rx_callback_t cb)
{
    struct rad_rx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_rx_driver_api*)dev->api;

    if (api->init == NULL) {
        return -ENOTSUP;
    }
    return api->set_callback(dev, cb);
}

#define IS_VALID_START_PULSE(value, target) ((target)-RAD_RX_START_PULSE_MARGIN_US <= (value) && \
                                                (target)+RAD_RX_START_PULSE_MARGIN_US >= (value))

#define IS_VALID_BIT_PULSE(value, target) ((target)-RAD_RX_BIT_MARGIN_US <= (value) && \
                                                (target)+RAD_RX_BIT_MARGIN_US >= (value))

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_RX_H_ */
