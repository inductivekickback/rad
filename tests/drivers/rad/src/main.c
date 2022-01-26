#include <ztest.h>

#include <drivers/rad_rx.h>
#include <drivers/rad_tx.h>

#define LOOPBACK_LATENCY_MS 50
#define LINE_CLEAR_DELAY_MS 1

const static struct device *rx_dev;
const static struct device *tx_dev;

static rad_msg_type_t cur_msg_type;
static void *cur_data;

static K_SEM_DEFINE(loopback, 0, 1);

void rad_rx_cb(rad_msg_type_t msg_type, void *data)
{
	zassert_equal(msg_type, cur_msg_type,
		            "Unexpected rad_msg_type_t received: %d != %d", msg_type, cur_msg_type);
	switch (msg_type) {
	case RAD_MSG_TYPE_LASER_X:
		zassert_mem_equal(data, cur_data,
			                sizeof(rad_msg_laser_x_t), "Invalid LASER_X message data.");
		break;
	case RAD_MSG_TYPE_DYNASTY:
		{
			rad_msg_dynasty_t *cur_msg = (rad_msg_dynasty_t*)cur_data;
			rad_msg_dynasty_t *msg = (rad_msg_dynasty_t*)data;
			zassert_equal(cur_msg->team_id, msg->team_id, "Invalid DYNASTY team_id.");
			zassert_equal(cur_msg->weapon_id, msg->weapon_id, "Invalid DYNASTY weapon_id.");
		}
		break;
	case RAD_MSG_TYPE_RAD:
		zassert_mem_equal(data, cur_data, sizeof(rad_msg_rad_t),"Invalid RAD message data.");
		break;
	default:
		zassert_unreachable("Unknown rad_msg_type_t received: %d", msg_type);
		break;
	}
	k_sem_give(&loopback);
}

static void blast_and_wait(rad_msg_type_t msg_type, void *data)
{
	int ret;
	cur_msg_type = msg_type;
	cur_data = data;
	switch (msg_type) {
	case RAD_MSG_TYPE_LASER_X:
		ret = rad_tx_laser_x_blast(tx_dev, (rad_msg_laser_x_t*)data);
		zassert_equal(ret, 0, "rad_tx_laser_x_blast failed: %d", ret);
		break;
	case RAD_MSG_TYPE_DYNASTY:
		ret = rad_tx_dynasty_blast(tx_dev, (rad_msg_dynasty_t*)data);
		zassert_equal(ret, 0, "rad_tx_dynasty_blast failed: %d", ret);
		break;
	case RAD_MSG_TYPE_RAD:
		ret = rad_tx_rad_blast(tx_dev, (rad_msg_rad_t*)data);
		zassert_equal(ret, 0, "rad_tx_rad_blast failed: %d", ret);
		break;
	default:
		zassert_unreachable("Unknown rad_msg_type_t received: %d", msg_type);
		break;
	}

	ret = k_sem_take(&loopback, K_MSEC(LOOPBACK_LATENCY_MS));
    zassert_equal(ret, 0, "rad_tx_*_blast loopback timed out.");

    k_msleep(LINE_CLEAR_DELAY_MS);

	ret = rad_tx_blast_again(tx_dev);
	zassert_equal(ret, 0, "rad_tx_blast_again failed: %d", ret);

	ret = k_sem_take(&loopback, K_MSEC(LOOPBACK_LATENCY_MS));
    zassert_equal(ret, 0, "rad_tx_blast_again loopback timed out.");

    k_msleep(LINE_CLEAR_DELAY_MS);
}

static void test_get_binding(void)
{
	int ret;

    rx_dev = device_get_binding("rad_rx0");
    zassert_not_null(rx_dev, "Failed to get RX dev binding");

    ret = rad_rx_set_callback(rx_dev, rad_rx_cb);
    zassert_equal(ret, 0, "Failed to set Rad RX callback");

    tx_dev = device_get_binding("rad_tx0");
    zassert_not_null(tx_dev, "Failed to get TX dev binding");
}

static void test_laser_x_loopback(void)
{
    rad_msg_laser_x_t laser_x_msg;

    laser_x_msg.team_id = TEAM_ID_LASER_X_RED;
    blast_and_wait(RAD_MSG_TYPE_LASER_X, &laser_x_msg);

    laser_x_msg.team_id = TEAM_ID_LASER_X_BLUE;
    blast_and_wait(RAD_MSG_TYPE_LASER_X, &laser_x_msg);

    laser_x_msg.team_id = TEAM_ID_LASER_X_NEUTRAL;
    blast_and_wait(RAD_MSG_TYPE_LASER_X, &laser_x_msg);
}

static void test_dynasty_loopback(void)
{
    rad_msg_dynasty_t dynasty_msg;

    for (int i=TEAM_ID_DYNASTY_BLUE; i <= TEAM_ID_DYNASTY_WHITE; i++) {
    	dynasty_msg.team_id = i;

    	for (int j=WEAPON_ID_DYNASTY_PISTOL; j <= WEAPON_ID_DYNASTY_ROCKET; j++) {
			dynasty_msg.weapon_id = j;
		    blast_and_wait(RAD_MSG_TYPE_DYNASTY, &dynasty_msg);
		}
    }
}

static void test_rad_loopback(void)
{
	/**
	 * NOTE: version must only be set to RAD_MSG_VERSION.
	 * 
	 * typedef struct
	 * {
	 *  	uint8_t damage		: 4;
	 *  	uint8_t special 	: 4;
	 *  	uint8_t player_id	: 4;
	 *  	uint8_t team_id		: 2;
	 *  	uint8_t version 	: 2;
	 * } rad_msg_rad_t;
	 */
    rad_msg_rad_t rad_msg;
    rad_msg.version = RAD_MSG_VERSION;
    for (int damage=0; damage < 0xF; damage++) {
    	rad_msg.damage = damage;

    	for (int special=0; special < 0xF; special++) {
			rad_msg.special = special;

			for (int player_id=0; player_id < 0xF; player_id++) {
				rad_msg.player_id = player_id;

				for (int team_id=0; team_id < 0x3; team_id++) {
				    blast_and_wait(RAD_MSG_TYPE_RAD, &rad_msg);
				}
			}
		}
    }
}

void test_main(void)
{
	ztest_test_suite(test_rad,
		ztest_unit_test(test_get_binding),
    	ztest_unit_test(test_laser_x_loopback),
    	ztest_unit_test(test_dynasty_loopback),
    	ztest_unit_test(test_rad_loopback)
	);

	ztest_run_test_suite(test_rad);
}
