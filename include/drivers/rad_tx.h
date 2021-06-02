/**
 * @file rad_tx.h
 *
 * @brief Public API for the Rad transmitter driver
 */

/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#ifndef ZEPHYR_INCLUDE_RAD_TX_H_
#define ZEPHYR_INCLUDE_RAD_TX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr.h>
#include <device.h>
#include <stdbool.h>

#include <rad.h>
#include <nrfx_pwm.h>

/**
 * With a 16MHz clock the 37.9KHz period is ~422 ticks.
 */
#define RAD_TX_TICKS_PER_PERIOD                   422
#define RAD_TX_DUTY_CYCLE_0                       RAD_TX_TICKS_PER_PERIOD
#define RAD_TX_DUTY_CYCLE_50                      (RAD_TX_TICKS_PER_PERIOD / 2)
#define RAD_TX_PWM_VALUE_LEN_US                   26

#define RAD_TX_RAD_START_PULSE_LEN_PWM_VALUES     (RAD_MSG_TYPE_RAD_START_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_RAD_ACTIVE_PULSE_LEN_PWM_VALUES    (RAD_MSG_TYPE_RAD_ACTIVE_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_RAD_0_PULSE_LEN_PWM_VALUES         (RAD_MSG_TYPE_RAD_0_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_RAD_1_PULSE_LEN_PWM_VALUES         (RAD_MSG_TYPE_RAD_1_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_RAD_0_BIT_LEN_PWM_VALUES           (RAD_MSG_TYPE_RAD_0_BIT_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_RAD_1_BIT_LEN_PWM_VALUES           (RAD_MSG_TYPE_RAD_1_BIT_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_RAD_MAX_BIT_LEN_PWM_VALUES         MAX(RAD_TX_RAD_0_BIT_LEN_PWM_VALUES, \
                                                    RAD_TX_RAD_1_BIT_LEN_PWM_VALUES)
#define RAD_TX_RAD_MAX_MSG_LEN_PWM_VALUES         (RAD_TX_RAD_START_PULSE_LEN_PWM_VALUES + \
                                                    (RAD_TX_RAD_MAX_BIT_LEN_PWM_VALUES * \
                                                    RAD_MSG_TYPE_RAD_LEN_IR_BITS) + 1)

#define RAD_TX_DYNASTY_START_PULSE_LEN_PWM_VALUES (RAD_MSG_TYPE_DYNASTY_START_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_DYNASTY_0_PULSE_LEN_PWM_VALUES     (RAD_MSG_TYPE_DYNASTY_0_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_DYNASTY_1_PULSE_LEN_PWM_VALUES     (RAD_MSG_TYPE_DYNASTY_1_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_DYNASTY_MAX_BIT_LEN_PWM_VALUES     MAX(RAD_TX_DYNASTY_0_PULSE_LEN_PWM_VALUES, \
                                                    RAD_TX_DYNASTY_1_PULSE_LEN_PWM_VALUES)
#define RAD_TX_DYNASTY_MAX_MSG_LEN_PWM_VALUES     (RAD_TX_DYNASTY_START_PULSE_LEN_PWM_VALUES + \
                                                    (RAD_TX_DYNASTY_MAX_BIT_LEN_PWM_VALUES * \
                                                    RAD_MSG_TYPE_DYNASTY_LEN_IR_BITS) + 1)

#define RAD_TX_LASER_X_START_PULSE_LEN_PWM_VALUES (RAD_MSG_TYPE_LASER_X_START_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_LASER_X_SPACE_PULSE_PWM_VALUES     (RAD_MSG_TYPE_LASER_X_SPACE_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_LASER_X_0_PULSE_LEN_PWM_VALUES     (RAD_MSG_TYPE_LASER_X_0_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_LASER_X_1_PULSE_LEN_PWM_VALUES     (RAD_MSG_TYPE_LASER_X_1_PULSE_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_LASER_X_0_BIT_LEN_PWM_VALUES       (RAD_MSG_TYPE_LASER_X_0_BIT_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_LASER_X_1_BIT_LEN_PWM_VALUES       (RAD_MSG_TYPE_LASER_X_1_BIT_LEN_US / \
                                                    RAD_TX_PWM_VALUE_LEN_US)
#define RAD_TX_LASER_X_MAX_BIT_LEN_PWM_VALUES     MAX(RAD_TX_LASER_X_0_BIT_LEN_PWM_VALUES, \
                                                    RAD_TX_LASER_X_1_BIT_LEN_PWM_VALUES)
#define RAD_TX_LASER_X_MAX_MSG_LEN_PWM_VALUES     (RAD_TX_LASER_X_START_PULSE_LEN_PWM_VALUES + \
                                                    (RAD_TX_LASER_X_MAX_BIT_LEN_PWM_VALUES * \
                                                    RAD_MSG_TYPE_LASER_X_LEN_IR_BITS) + 1)

#define RAD_TX_MSG_MAX_LEN_PWM_VALUES 0

#if CONFIG_RAD_TX_RAD
int rad_msg_type_rad_encode(rad_msg_rad_t *msg,
                              nrf_pwm_values_common_t *values,
                              uint32_t *len);
#if RAD_TX_MSG_MAX_LEN_PWM_VALUES < RAD_TX_RAD_MAX_MSG_LEN_PWM_VALUES
#undef RAD_TX_MSG_MAX_LEN_PWM_VALUES
#define RAD_TX_MSG_MAX_LEN_PWM_VALUES RAD_TX_RAD_MAX_MSG_LEN_PWM_VALUES
#endif
#endif /* CONFIG_RAD_TX_RAD */

#if CONFIG_RAD_TX_DYNASTY
int rad_msg_type_dynasty_encode(rad_msg_dynasty_t *msg,
                                  nrf_pwm_values_common_t *values,
                                  uint32_t *len);
#if RAD_TX_MSG_MAX_LEN_PWM_VALUES < RAD_TX_DYNASTY_MAX_MSG_LEN_PWM_VALUES
#undef RAD_TX_MSG_MAX_LEN_PWM_VALUES
#define RAD_TX_MSG_MAX_LEN_PWM_VALUES RAD_TX_DYNASTY_MAX_MSG_LEN_PWM_VALUES
#endif
#endif /* CONFIG_RAD_TX_DYNASTY */

#if CONFIG_RAD_TX_LASER_X
int rad_msg_type_laser_x_encode(rad_msg_laser_x_t *msg,
                                  nrf_pwm_values_common_t *values,
                                  uint32_t *len);
#if RAD_TX_MSG_MAX_LEN_PWM_VALUES < RAD_TX_LASER_X_MAX_MSG_LEN_PWM_VALUES
#undef RAD_TX_MSG_MAX_LEN_PWM_VALUES
#define RAD_TX_MSG_MAX_LEN_PWM_VALUES RAD_TX_LASER_X_MAX_MSG_LEN_PWM_VALUES
#endif
#endif /* CONFIG_RAD_TX_LASER_X */

#if CONFIG_RAD_TX
#if RAD_TX_MSG_MAX_LEN_PWM_VALUES == 0
#error No Rad TX message types enabled
#endif
#endif

typedef int (*rad_tx_init_t)        (const struct device *dev);
typedef int (*rad_tx_blast_again_t) (const struct device *dev); /* Repeat the last blast. */

#if CONFIG_RAD_TX_RAD
typedef int (*rad_tx_rad_blast_t) (const struct device *dev, rad_msg_rad_t *msg);
#endif

#if CONFIG_RAD_TX_LASER_X
typedef int (*rad_tx_laser_x_blast_t) (const struct device *dev, rad_msg_laser_x_t *msg);
#endif

#if CONFIG_RAD_TX_DYNASTY
typedef int (*rad_tx_dynasty_blast_t) (const struct device *dev, rad_msg_dynasty_t *msg);
#endif

/**
 * @brief Rad transmitter driver API
 */
struct rad_tx_driver_api {
    rad_tx_init_t          init;
    rad_tx_blast_again_t   blast_again;
#if CONFIG_RAD_TX_RAD
    rad_tx_rad_blast_t     rad_blast;
#endif
#if CONFIG_RAD_TX_LASER_X
    rad_tx_laser_x_blast_t laser_x_blast;
#endif
#if CONFIG_RAD_TX_DYNASTY
    rad_tx_dynasty_blast_t dynasty_blast;
#endif
};

static inline int rad_tx_init(const struct device *dev)
{
    struct rad_tx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_tx_driver_api*)dev->api;

    if (api->init == NULL) {
        return -ENOTSUP;
    }
    return api->init(dev);
}

static inline int rad_tx_blast_again(const struct device *dev)
{
    struct rad_tx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_tx_driver_api*)dev->api;

    if (api->blast_again == NULL) {
        return -ENOTSUP;
    }
    return api->blast_again(dev);
}

#if CONFIG_RAD_TX_RAD
static inline int rad_tx_rad_blast(const struct device *dev, rad_msg_rad_t *msg)
{
    struct rad_tx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_tx_driver_api*)dev->api;

    if (api->rad_blast == NULL) {
        return -ENOTSUP;
    }
    return api->rad_blast(dev, msg);
}
#endif /* CONFIG_RAD_RAD */

#if CONFIG_RAD_TX_LASER_X
static inline int rad_tx_laser_x_blast(const struct device *dev, rad_msg_laser_x_t *msg)
{
    struct rad_tx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_tx_driver_api*)dev->api;

    if (api->laser_x_blast == NULL) {
        return -ENOTSUP;
    }
    return api->laser_x_blast(dev, msg);
}
#endif /* CONFIG_RAD_TX_LASER_X */

#if CONFIG_RAD_TX_DYNASTY
static inline int rad_tx_dynasty_blast(const struct device *dev, rad_msg_dynasty_t *msg)
{
    struct rad_tx_driver_api *api;

    if (dev == NULL) {
        return -EINVAL;
    }

    api = (struct rad_tx_driver_api*)dev->api;

    if (api->dynasty_blast == NULL) {
        return -ENOTSUP;
    }
    return api->dynasty_blast(dev, msg);
}
#endif /* CONFIG_RAD_TX_DYNASTY */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_RAD_TX_H_ */
