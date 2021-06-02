/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#define DT_DRV_COMPAT dmv_rad_rx

#include <kernel.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#include <logging/log.h>

#include <drivers/rad_rx.h>

LOG_MODULE_REGISTER(rad_rx, CONFIG_RAD_RX_LOG_LEVEL);

typedef enum
{
    MSG_STATE_WAIT_FOR_LINE_CLEAR,
    MSG_STATE_WAIT_FOR_PREAMBLE,
    MSG_STATE_WAIT_FOR_1,
    MSG_STATE_WAIT_FOR_0,
    MSG_STATE_COMPLETE,
    MSG_STATE_COUNT
} msg_state_t;

struct rad_rx_data {
    bool                  ready;

    const struct device  *dev;
    struct gpio_callback  cb_data;
    uint8_t               pin;

    rad_rx_callback_t     cb;

    struct k_timer        timer;
    struct k_work         work;

    uint32_t              message[RAD_RX_MSG_MAX_LEN];
    uint32_t              timestamp;
    atomic_t              index;
    msg_state_t           state;

#if CONFIG_RAD_RX_ACCEPT_RAD
    rad_parse_state_t     rad_parse_state;
#endif
#if CONFIG_RAD_RX_ACCEPT_DYNASTY
    rad_parse_state_t     dynasty_parse_state;
#endif
#if CONFIG_RAD_RX_ACCEPT_LASER_X
    rad_parse_state_t     laser_x_parse_state;
#endif
};

struct rad_rx_cfg {
    const char * const port;
    const uint8_t      pin;
    const uint32_t     flags;
};

static void line_clear(struct rad_rx_data *p_data)
{
    p_data->state = MSG_STATE_WAIT_FOR_PREAMBLE;
    atomic_set(&p_data->index, 0);
#if CONFIG_RAD_RX_ACCEPT_RAD
    p_data->rad_parse_state     = RAD_PARSE_STATE_WAIT_FOR_START_PULSE;
#endif
#if CONFIG_RAD_RX_ACCEPT_DYNASTY
    p_data->dynasty_parse_state = RAD_PARSE_STATE_WAIT_FOR_START_PULSE;
#endif
#if CONFIG_RAD_RX_ACCEPT_LASER_X
    p_data->laser_x_parse_state = RAD_PARSE_STATE_WAIT_FOR_START_PULSE;
#endif
}

static void line_clear_timer_expire(struct k_timer *timer_id)
{
    // NOTE: Ensure that this interrupt has a higher priority than the GPIO interrupt
    //       to preclude a race condition.
    struct rad_rx_data *p_data = CONTAINER_OF(timer_id, struct rad_rx_data, timer);
    line_clear(p_data);
}

static void message_decode(struct k_work *item)
{
    struct rad_rx_data *p_data       = CONTAINER_OF(item, struct rad_rx_data, work);
    uint32_t            len          = (atomic_get(&p_data->index) - 1);
    bool                msg_finished = true;

    if (MSG_STATE_WAIT_FOR_LINE_CLEAR == p_data->state) {
        /* There might be one additional input change after giving up on the message. */
        return;
    }

#if CONFIG_RAD_RX_ACCEPT_RAD
    rad_msg_rad_t rad_msg;
    switch (p_data->rad_parse_state) {
    case RAD_PARSE_STATE_WAIT_FOR_START_PULSE:
        if (!IS_VALID_START_PULSE(p_data->message[0], RAD_MSG_TYPE_RAD_START_PULSE_LEN_US)) {
            p_data->rad_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        } else {
            msg_finished            = false;
            p_data->rad_parse_state = RAD_PARSE_STATE_INCOMPLETE;
            // Fall-through into the next state in case the entire message is received.
        }
    case RAD_PARSE_STATE_INCOMPLETE:
        if (RAD_MSG_TYPE_RAD_LEN_PULSES > len) {
            msg_finished = false;
            break;
        } else if (RAD_MSG_TYPE_RAD_LEN_PULSES < len) {
            p_data->rad_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        }

        p_data->rad_parse_state = rad_msg_type_rad_parse(&p_data->message[0], len, &rad_msg);
        if (RAD_PARSE_STATE_VALID == p_data->rad_parse_state) {
            if (p_data->cb) {
                p_data->cb(RAD_MSG_TYPE_RAD, (void*)&rad_msg);
            }
        }
        break;
    default:
        break;
    }
#endif

#if CONFIG_RAD_RX_ACCEPT_LASER_X
    rad_msg_laser_x_t laser_x_msg;
    switch (p_data->laser_x_parse_state) {
    case RAD_PARSE_STATE_WAIT_FOR_START_PULSE:
        if (!IS_VALID_START_PULSE(p_data->message[0], RAD_MSG_TYPE_LASER_X_START_PULSE_LEN_US)){
            p_data->laser_x_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        } else {
            msg_finished                = false;
            p_data->laser_x_parse_state = RAD_PARSE_STATE_INCOMPLETE;
            // Fall-through into the next state in case the entire message is received.
        }
    case RAD_PARSE_STATE_INCOMPLETE:
        if (RAD_MSG_TYPE_LASER_X_LEN_PULSES > len) {
            msg_finished = false;
            break;
        } else if (RAD_MSG_TYPE_LASER_X_LEN_PULSES < len) {
            p_data->laser_x_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        }

        p_data->laser_x_parse_state = rad_msg_type_laser_x_parse(&p_data->message[0],
                                                                   len,
                                                                   &laser_x_msg);
        if (RAD_PARSE_STATE_VALID == p_data->laser_x_parse_state) {
            if (p_data->cb) {
                p_data->cb(RAD_MSG_TYPE_LASER_X, (void*)&laser_x_msg);
            }
        }
        break;
    default:
        break;
    }
#endif

#if CONFIG_RAD_RX_ACCEPT_DYNASTY
    rad_msg_dynasty_t dynasty_msg;
    switch (p_data->dynasty_parse_state) {
    case RAD_PARSE_STATE_WAIT_FOR_START_PULSE:
        if (!IS_VALID_START_PULSE(p_data->message[0], RAD_MSG_TYPE_DYNASTY_START_PULSE_LEN_US)){
            p_data->dynasty_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        } else {
            msg_finished                = false;
            p_data->dynasty_parse_state = RAD_PARSE_STATE_INCOMPLETE;
            // Fall-through into the next state in case the entire message is received.
        }
    case RAD_PARSE_STATE_INCOMPLETE:
        if (RAD_MSG_TYPE_DYNASTY_LEN_PULSES > len) {
            msg_finished = false;
            break;
        } else if (RAD_MSG_TYPE_DYNASTY_LEN_PULSES < len) {
            p_data->dynasty_parse_state = RAD_PARSE_STATE_INVALID;
            break;
        }

        p_data->dynasty_parse_state = rad_msg_type_dynasty_parse(&p_data->message[0],
                                                                    len,
                                                                    &dynasty_msg);
        if (RAD_PARSE_STATE_VALID == p_data->dynasty_parse_state) {
            if (p_data->cb) {
                p_data->cb(RAD_MSG_TYPE_DYNASTY, (void*)&dynasty_msg);
            }
        }
        break;
    default:
        break;
    }
#endif

    if (msg_finished) {
        p_data->state = MSG_STATE_WAIT_FOR_LINE_CLEAR;
        line_clear(p_data);
    } else {
        k_timer_start(&p_data->timer, K_USEC(RAD_RX_LINE_CLEAR_LEN_US), K_NO_WAIT);
    }
}

static void input_changed(const struct device *dev, struct gpio_callback *cb_data, uint32_t pins)
{
    /**
     * line_clear_timer_expired will preempt message_decode so the timer should only be started
     * when the IR receiver's pin is deasserted AND message_decode has had a chance to run.
     */
    struct rad_rx_data *p_data = CONTAINER_OF(cb_data, struct rad_rx_data, cb_data);

    bool pin_state = gpio_pin_get(dev, p_data->pin);
    if (pin_state) {
        k_timer_stop(&p_data->timer);
    }

    if (MSG_STATE_WAIT_FOR_LINE_CLEAR == p_data->state) {
        if (!pin_state) {
            k_timer_start(&p_data->timer, K_USEC(RAD_RX_LINE_CLEAR_LEN_US), K_NO_WAIT);
        }
        return;
    }

    uint32_t now   = k_cyc_to_us_near32(k_cycle_get_32());
    uint32_t index = atomic_inc(&p_data->index); /* index is set to the pre-incremented valued */

    if (0 < index) {
        if (RAD_RX_MSG_MAX_LEN < index) {
            if (!pin_state) {
                k_timer_start(&p_data->timer, K_USEC(RAD_RX_LINE_CLEAR_LEN_US), K_NO_WAIT);
            }
            return;
        }

        if (p_data->timestamp <= now) {
            p_data->message[index-1] = (now - p_data->timestamp);
        } else {
            p_data->message[index-1] = (0xFFFFFFFF - p_data->timestamp);
            p_data->message[index-1] += now;
        }

        if (!pin_state) {
            k_work_submit(&p_data->work);
        }
    }
    p_data->timestamp = now;
}

static int dmv_rad_rx_init(const struct device *dev)
{
    int err;
    struct rad_rx_data      *p_data = dev->data;
    const struct rad_rx_cfg *p_cfg  = dev->config;

    k_timer_init(&p_data->timer, line_clear_timer_expire, NULL);
    k_work_init(&p_data->work, message_decode);

    p_data->dev = device_get_binding(p_cfg->port);
    if (!p_data->dev) {
        return -ENODEV;
    }

    err = gpio_pin_configure(p_data->dev, p_cfg->pin, (GPIO_INPUT | p_cfg->flags));
    if (err != 0) {
        return err;
    }

    err = gpio_pin_interrupt_configure(p_data->dev, p_cfg->pin, GPIO_INT_EDGE_BOTH);
    if (err != 0) {
        return err;
    }

    gpio_init_callback(&p_data->cb_data, input_changed, BIT(p_cfg->pin));
    gpio_add_callback(p_data->dev, &p_data->cb_data);

    p_data->state = MSG_STATE_WAIT_FOR_LINE_CLEAR;
    p_data->index = ATOMIC_INIT(0);
    p_data->cb    = NULL;
    p_data->ready = true;
    p_data->pin   = p_cfg->pin;

    if (!gpio_pin_get(p_data->dev, p_cfg->pin)) {
        k_timer_start(&p_data->timer, K_USEC(RAD_RX_LINE_CLEAR_LEN_US), K_NO_WAIT);
    }
    return 0;
}

static int dmv_rad_set_callback(const struct device *dev, rad_rx_callback_t cb)
{
    struct rad_rx_data *p_data = dev->data;
    p_data->cb = cb;
    return 0;
}

static const struct rad_rx_driver_api rad_rx_driver_api = {
    .init         = dmv_rad_rx_init,
    .set_callback = dmv_rad_set_callback,
};

#define INST(num) DT_INST(num, dmv_rad_rx)

#define RAD_RX_DEVICE(n) \
    static const struct rad_rx_cfg rad_rx_cfg_##n = { \
        .port  = DT_GPIO_LABEL(INST(n), gpios), \
        .pin   = DT_GPIO_PIN(INST(n),   gpios), \
        .flags = DT_GPIO_FLAGS(INST(n), gpios), \
    }; \
    static struct rad_rx_data rad_rx_data_##n; \
    DEVICE_DEFINE(rad_rx_##n, \
                DT_LABEL(INST(n)), \
                dmv_rad_rx_init, \
                NULL, \
                &rad_rx_data_##n, \
                &rad_rx_cfg_##n, \
                POST_KERNEL, \
                CONFIG_RAD_RX_INIT_PRIORITY, \
                &rad_rx_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RAD_RX_DEVICE)

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Rad laser tag receiver driver enabled without any devices"
#endif
