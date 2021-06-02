/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#define DT_DRV_COMPAT dmv_rad_tx

#include <kernel.h>
#include <device.h>
#include <drivers/rad_tx.h>
#include <devicetree.h>
#include <irq.h>

#include <hal/nrf_gpio.h>
#include <logging/log.h>

#include <drivers/rad_rx.h>

LOG_MODULE_REGISTER(rad_tx, CONFIG_RAD_TX_LOG_LEVEL);

typedef struct
{
    unsigned int irq_p;
    unsigned int priority_p;
    nrfx_pwm_t   pwm_instance;
    bool         ready;
    void (*isr_p)(void);
} pwm_periph_t;

static pwm_periph_t m_avail_pwms[] = {
#if CONFIG_RAD_TX_ALLOW_PWM0
    {
        .pwm_instance = NRFX_PWM_INSTANCE(0),
        .irq_p        = DT_IRQN(DT_NODELABEL(pwm0)),
        .isr_p        = nrfx_pwm_0_irq_handler,
        .priority_p   = DT_IRQ(DT_NODELABEL(pwm0), priority),
        .ready        = false,
    },
#endif
#if CONFIG_RAD_TX_ALLOW_PWM1
    {
        .pwm_instance = NRFX_PWM_INSTANCE(1),
        .irq_p        = DT_IRQN(DT_NODELABEL(pwm1)),
        .isr_p        = nrfx_pwm_1_irq_handler,
        .priority_p   = DT_IRQ(DT_NODELABEL(pwm1), priority),
        .ready        = false,
    },
#endif
#if CONFIG_RAD_TX_ALLOW_PWM2
    {
        .pwm_instance = NRFX_PWM_INSTANCE(2),
        .irq_p        = DT_IRQN(DT_NODELABEL(pwm2)),
        .isr_p        = nrfx_pwm_2_irq_handler,
        .priority_p   = DT_IRQ(DT_NODELABEL(pwm2), priority),
        .ready        = false,
    },
#endif
#if CONFIG_RAD_TX_ALLOW_PWM3
    {
        .pwm_instance = NRFX_PWM_INSTANCE(3),
        .irq_p        = DT_IRQN(DT_NODELABEL(pwm3)),
        .isr_p        = nrfx_pwm_3_irq_handler,
        .priority_p   = DT_IRQ(DT_NODELABEL(pwm3), priority),
        .ready        = false,
    },
#endif
};

#define NUM_AVAIL_PWMS (sizeof(m_avail_pwms)/sizeof(pwm_periph_t))

struct rad_tx_data {
    struct k_sem            sem;
    nrf_pwm_values_common_t values[RAD_TX_MSG_MAX_LEN_PWM_VALUES];
    uint32_t                len;
	bool                    ready;
};

struct rad_tx_cfg {
    const uint32_t pin;
    const uint8_t  pwm_index;
};

static void pwm_handler(nrfx_pwm_evt_type_t event_type, void *p_context)
{
    struct rad_tx_data *p_data = (struct rad_tx_data*)p_context;

    switch (event_type) {
    case NRFX_PWM_EVT_STOPPED:
        k_sem_give(&p_data->sem);
        break;
    default:
        break;
    }
}

static void tx(nrfx_pwm_t *pwm_inst, const nrf_pwm_values_common_t *values, uint32_t len)
{
    static nrf_pwm_sequence_t seq = {
        .values.p_common = NULL,
        .length          = 0,
        .repeats         = 0,
        .end_delay       = 0,
        .repeats         = 0
    };

    seq.values.p_common = values;
    seq.length          = len;

    nrfx_pwm_simple_playback(pwm_inst, &seq, 1, NRFX_PWM_FLAG_STOP);
}

#if CONFIG_RAD_TX_RAD
static int dmv_rad_tx_rad_blast(const struct device *dev, rad_msg_rad_t *msg)
{
    const struct rad_tx_cfg *p_cfg  = dev->config;
    struct rad_tx_data      *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_ERR("Driver is not initialized");
        return -EBUSY;
    }

    int err = k_sem_take(&p_data->sem, K_FOREVER);
    if (0 != err) {
        return err;
    }

    p_data->len = RAD_TX_MSG_MAX_LEN_PWM_VALUES;

    err = rad_msg_type_rad_encode(msg, p_data->values, &p_data->len);
    if (err) {
        return err;
    }
    tx(&m_avail_pwms[p_cfg->pwm_index].pwm_instance, p_data->values, p_data->len);
    return 0;
}
#endif /* CONFIG_RAD_TX_RAD */

#if CONFIG_RAD_TX_LASER_X
static int dmv_rad_tx_laser_x_blast(const struct device *dev, rad_msg_laser_x_t *msg)
{
    const struct rad_tx_cfg *p_cfg  = dev->config;
    struct rad_tx_data      *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_ERR("Driver is not initialized");
        return -EBUSY;
    }

    int err = k_sem_take(&p_data->sem, K_FOREVER);
    if (0 != err) {
        return err;
    }

    p_data->len = RAD_TX_MSG_MAX_LEN_PWM_VALUES;

    err = rad_msg_type_laser_x_encode(msg, p_data->values, &p_data->len);
    if (err) {
        return err;
    }
    tx(&m_avail_pwms[p_cfg->pwm_index].pwm_instance, p_data->values, p_data->len);
    return 0;
}
#endif /* CONFIG_RAD_TX_LASER_X */

#if CONFIG_RAD_TX_DYNASTY
static int dmv_rad_tx_dynasty_blast(const struct device *dev, rad_msg_dynasty_t *msg)
{
    const struct rad_tx_cfg *p_cfg  = dev->config;
    struct rad_tx_data      *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_ERR("Driver is not initialized");
        return -EBUSY;
    }

    int err = k_sem_take(&p_data->sem, K_FOREVER);
    if (0 != err) {
        return err;
    }

    p_data->len = RAD_TX_MSG_MAX_LEN_PWM_VALUES;

    err = rad_msg_type_dynasty_encode(msg, p_data->values, &p_data->len);
    if (err) {
        return err;
    }
    tx(&m_avail_pwms[p_cfg->pwm_index].pwm_instance, p_data->values, p_data->len);
    return 0;
}
#endif /* CONFIG_RAD_TX_DYNASTY */

static int dmv_rad_tx_blast_again(const struct device *dev)
{
    const struct rad_tx_cfg *p_cfg  = dev->config;
    struct rad_tx_data      *p_data = dev->data;

    if (unlikely(!p_data->ready)) {
        LOG_ERR("Driver is not initialized");
        return -EBUSY;
    }

    if (0 == p_data->len) {
        return -1;
    }

    int err = k_sem_take(&p_data->sem, K_FOREVER);
    if (0 != err) {
        return err;
    }
    tx(&m_avail_pwms[p_cfg->pwm_index].pwm_instance, p_data->values, p_data->len);
    return 0;
}

static int dmv_rad_tx_init(const struct device *dev)
{
    nrfx_pwm_t *p_inst;

    const struct rad_tx_cfg *p_cfg  = dev->config;
    struct rad_tx_data      *p_data = dev->data;

    if (unlikely(p_data->ready)) {
        /* Already initialized */
        return 0;
    }

    if (NUM_AVAIL_PWMS <= p_cfg->pwm_index) {
        goto ERR_EXIT;
    }

    if (0 != k_sem_init(&p_data->sem, 1, 1)) {
        goto ERR_EXIT;
    }

    p_inst = &m_avail_pwms[p_cfg->pwm_index].pwm_instance;

    if (!m_avail_pwms[p_cfg->pwm_index].ready) {
        nrfx_pwm_config_t config = NRFX_PWM_DEFAULT_CONFIG(p_cfg->pin,
                                                            NRFX_PWM_PIN_NOT_USED,
                                                            NRFX_PWM_PIN_NOT_USED,
                                                            NRFX_PWM_PIN_NOT_USED);
        config.base_clock = NRF_PWM_CLK_16MHz;
        config.top_value  = RAD_TX_TICKS_PER_PERIOD;
        config.load_mode  = NRF_PWM_LOAD_COMMON;

        /* NOTE: irq_connect_dynamic returns a vector index instead of an error code. */
        irq_connect_dynamic(m_avail_pwms[p_cfg->pwm_index].irq_p,
                              m_avail_pwms[p_cfg->pwm_index].priority_p,
                              nrfx_isr,
                              m_avail_pwms[p_cfg->pwm_index].isr_p,
                              0);

        nrfx_err_t err = nrfx_pwm_init(&m_avail_pwms[p_cfg->pwm_index].pwm_instance,
                                         &config,
                                         pwm_handler,
                                         p_data);
        if (NRFX_SUCCESS != err) {
            goto ERR_EXIT;
        }

        m_avail_pwms[p_cfg->pwm_index].ready = true;
    }

    nrf_gpio_pin_clear(p_cfg->pin);
    nrf_gpio_cfg_output(p_cfg->pin);
    m_avail_pwms[p_cfg->pwm_index].pwm_instance.p_registers->PSEL.OUT[0] = p_cfg->pin;

    p_data->len   = 0;
    p_data->ready = true;
    return 0;

ERR_EXIT:
    return -ENXIO;
}

static const struct rad_tx_driver_api rad_tx_driver_api = {
    .init          = dmv_rad_tx_init,
    .blast_again   = dmv_rad_tx_blast_again,
#if CONFIG_RAD_TX_RAD
    .rad_blast     = dmv_rad_tx_rad_blast,
#endif
#if CONFIG_RAD_TX_RAD
    .laser_x_blast = dmv_rad_tx_laser_x_blast,
#endif
#if CONFIG_RAD_TX_DYNASTY
    .dynasty_blast = dmv_rad_tx_dynasty_blast,
#endif
};

#define INST(num) DT_INST(num, dmv_rad_tx)

#define RAD_TX_DEVICE(n) \
    static const struct rad_tx_cfg rad_tx_cfg_##n = { \
        .pin       = DT_GPIO_PIN(INST(n),   gpios), \
        .pwm_index = (n) \
    }; \
    static struct rad_tx_data rad_tx_data_##n; \
    DEVICE_DEFINE(rad_tx_##n, \
                DT_LABEL(INST(n)), \
                dmv_rad_tx_init, \
                NULL, \
                &rad_tx_data_##n, \
                &rad_tx_cfg_##n, \
                POST_KERNEL, \
                CONFIG_RAD_TX_INIT_PRIORITY, \
                &rad_tx_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RAD_TX_DEVICE)

#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "Rad laser tag transmitter driver enabled without any devices"
#endif
