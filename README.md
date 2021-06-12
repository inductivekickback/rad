This project implements Zephyr RTOS receiver and transmitter drivers for laser tag blasters. It was built from the v1.5.0 tag of the [nRF Connect SDK (NCS)](https://github.com/nrfconnect/sdk-nrf) and should be compatible with most devices in Nordic's nRF52 series. It currently supports two popular toy blasters (Laser X and Dynasty/Lightbattle/Kidzlane/JOYMOR) as well as a native message type.

### About the driver
The timing of laser blasters, especially toy ones, is pretty loose so *k_cycle_get_32* is usually good enough for decoding messages in the receiver. Nordic's [PWM peripheral](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fps_nrf52840%2Fpwm.html&cp=4_0_0_5_16) handles transmission with minimal CPU overhead.

This driver automatically assigns PWM peripherals to each transmitter device in the DT. Of course PWM peripherals might be needed for other purposes in the application so access to specific instances can be denied via the CONFIG_RAD_TX_ALLOW_PWM**X** settings.

Both receiver and transmitter devices in the DT only need to specify a pin and whether the pin is active high or low.

Received pulses are measured using a pin-change interrupt. Whenever the receiver's pin becomes inactive a task is added to the System Workqueue and that task attempts to decode the current message using whatever message types are enabled. When a message is successfuly decoded the receiver driver's callback is executed from the System Workqueue's thread. Here is an example of the receiver decoding a ("dynasty") message, sending it to the application via the callback, and then having the transmitter reconstruct and send the message (i.e. it's not just an echo of what was received) -- with only 180us latency.

<p align="center"><img src="https://user-images.githubusercontent.com/6494431/120431571-88bd8b00-c32d-11eb-9712-9b41cf1d6e57.png" width="1024"></p>

#### Laser X
The Laser X blasters that I purchased transmit messages that contain only team IDs:
```
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
...
int rad_tx_laser_x_blast(const struct device *dev, rad_msg_laser_x_t *msg);
```
#### Dynasty
The "Dynasty" blasters have a team ID as well as a weapon type. Two of the weapon types, shotgun and SMG, share the same IR code but differ in their sound effects. These messages also contain a third field that appears to be some kind of checksum (which is handled automatically by the driver and doesn't need to be supplied when transmitting):
```
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
...
int rad_tx_dynasty_blast(const struct device *dev, rad_msg_dynasty_t *msg);
```
#### Rad
A native message type is also defined that provides more flexibility:
```
typedef struct
{
	uint8_t damage		: 4;
	uint8_t special 	: 4;
	uint8_t player_id	: 4;
	uint8_t team_id		: 2;
	uint8_t reserved	: 2;
} rad_msg_rad_t;
...
int rad_tx_rad_blast(const struct device *dev, rad_msg_rad_t *msg);
```

### Using the driver
This is an example DT entry in the project's local overlay (e.g. "nrf52840dk_nrf52840.overlay"):
```
/ {
	rad {
		rad_rx0: dmv-rad-rx0 {
			compatible = "dmv,rad-rx";
			status = "okay";
			gpios = <&gpio0 31 GPIO_ACTIVE_LOW>;
			label = "rad_rx0";
		};
		rad_tx0: dmv-rad-tx0 {
			compatible = "dmv,rad-tx";
			status = "okay";
			gpios = <&gpio0 30 GPIO_ACTIVE_HIGH>;
			label = "rad_tx0";
		};
	};
};
```
Then enable the desired libraries in **prj.conf**:
```
CONFIG_RAD_TX=y
CONFIG_RAD_TX_LASER_X=y
CONFIG_RAD_TX_RAD=y
CONFIG_RAD_TX_DYNASTY=y
CONFIG_RAD_TX_ALLOW_PWM0=n # For example if PWM0 is reserved for something else.

CONFIG_RAD_RX=y
CONFIG_RAD_RX_ACCEPT_LASER_X=y
CONFIG_RAD_RX_ACCEPT_RAD=y
CONFIG_RAD_RX_ACCEPT_DYNASTY=y
```
A callback for the receiver needs to be registered at runtime:
```
void rad_rx_cb(rad_msg_type_t msg_type, void *data)
{
    switch (msg_type) {
    case RAD_MSG_TYPE_LASER_X:
        {
            rad_msg_laser_x_t *msg = (rad_msg_laser_x_t*)data;
            ...
        }
        break;
    case RAD_MSG_TYPE_DYNASTY:
        {
            rad_msg_dynasty_t *msg = (rad_msg_dynasty_t*)data;
            ...
        }
        break;
    case RAD_MSG_TYPE_RAD:
        {
            rad_msg_rad_t *msg = (rad_msg_rad_t*)data;
            ...
        }
        break;
    default:
        LOG_INF("Unhandled Rad message type (%d).", msg_type);
        break;
    }
}
...
const struct device *rx_dev = device_get_binding("rad_rx0");
...
int ret = rad_rx_set_callback(rx_dev, rad_rx_cb);
```
The transmitter can send messages for a particular blaster type:
```
rad_msg_dynasty_t dynasty_msg = {
  .team_id   = TEAM_ID_DYNASTY_GREEN,
  .weapon_id = WEAPON_ID_DYNASTY_ROCKET
};
int ret = rad_tx_dynasty_blast(tx_dev, &dynasty_msg);
```
After a message has been sent it can be sent again with very little CPU overhead:
```
int ret = rad_tx_blast_again(tx_dev);
```
