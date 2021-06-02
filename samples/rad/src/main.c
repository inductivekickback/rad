/*
 * Copyright (c) 2021 Daniel Veilleux
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <string.h>
#include <zephyr.h>
#include <device.h>
#include <stdio.h>
#include <sys/__assert.h>

#include <logging/log.h>

#include <drivers/rad_rx.h>
#include <drivers/rad_tx.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void rad_rx_cb(rad_msg_type_t msg_type, void *data)
{
    switch (msg_type) {
    case RAD_MSG_TYPE_LASER_X:
        {
            rad_msg_laser_x_t *msg = (rad_msg_laser_x_t*)data;
            switch (msg->team_id) {
            case TEAM_ID_LASER_X_BLUE:
                LOG_INF("Laser X team ID: BLUE");
                break;
            case TEAM_ID_LASER_X_RED:
                LOG_INF("Laser X team ID: RED");
                break;
            case TEAM_ID_LASER_X_NEUTRAL:
                LOG_INF("Laser X team ID: NEUTRAL");
                break;
            default:
                LOG_INF("Invalid Laser X team ID.");
                break;
            }
        }
        break;
    case RAD_MSG_TYPE_DYNASTY:
        {
            rad_msg_dynasty_t *msg = (rad_msg_dynasty_t*)data;
            LOG_INF("Dynasty team ID (%d), weapon ID (%d)", msg->team_id, msg->weapon_id);
        }
        break;
    case RAD_MSG_TYPE_RAD:
        LOG_INF("Rad message received");
        break;
    default:
        LOG_INF("Unhandled Rad message type (%d).", msg_type);
        break;
    }
}

void main(void)
{
    int ret;
    const struct device *rx_dev;
    const struct device *tx_dev;

    if (IS_ENABLED(CONFIG_LOG_BACKEND_RTT)) {
        /* Give RTT log time to be flushed before executing tests */
        k_sleep(K_MSEC(500));
    }

    rx_dev = device_get_binding("rad_rx0");

    if (rx_dev == NULL) {
        LOG_ERR("Failed to get RX dev binding");
        return;
    }
    LOG_INF("RX dev is %p, name is %s", rx_dev, rx_dev->name);

    ret = rad_rx_set_callback(rx_dev, rad_rx_cb);
    if (ret) {
        LOG_ERR("Failed to set Rad RX callback");
        return;
    }

    tx_dev = device_get_binding("rad_tx0");

    if (tx_dev == NULL) {
        LOG_ERR("Failed to get TX dev binding");
        return;
    }
    LOG_INF("TX dev is %p, name is %s", tx_dev, tx_dev->name);

    while (1) {
        rad_msg_laser_x_t laser_x_msg = {
            .team_id = TEAM_ID_LASER_X_RED
        };
        ret = rad_tx_laser_x_blast(tx_dev, &laser_x_msg);
        if (ret) {
            LOG_ERR("rad_tx_laser_x_blast failed: %d", ret);
        }
    	k_sleep(K_MSEC(100));

        ret = rad_tx_blast_again(tx_dev);
        if (ret) {
            LOG_ERR("rad_tx_blast again failed: %d", ret);
        }
        k_sleep(K_MSEC(100));

        rad_msg_dynasty_t dynasty_msg = {
            .team_id   = TEAM_ID_DYNASTY_RED,
            .weapon_id = WEAPON_ID_DYNASTY_ROCKET
        };
        ret = rad_tx_dynasty_blast(tx_dev, &dynasty_msg);
        if (ret) {
            LOG_ERR("rad_tx_laser_x_blast failed: %d", ret);
        }
        k_sleep(K_MSEC(100));

        rad_msg_rad_t rad_msg = {
            .damage    = 0,
            .special   = 9,
            .player_id = 4,
            .team_id   = 3,
            .reserved  = 0
        };
        ret = rad_tx_rad_blast(tx_dev, &rad_msg);
        if (ret) {
            LOG_ERR("rad_tx_rad_blast failed: %d", ret);
        }
        k_sleep(K_MSEC(100));
    }
}
