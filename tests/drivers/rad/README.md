This is a functional test to ensure compatibility between the rad_tx and rad_rx drivers. It generates every possible message, transmits it with an [IR LED](https://www.digikey.com/en/products/detail/w%C3%BCrth-elektronik/15400394A3590/7315793?s=N4IgTCBcDaIOwGYwFoCMqAs6QF0C%2BQA), and ensures that it is received correctly. Obviously this requires having a simple fixture that includes an IR LED and [receiver](https://www.digikey.com/en/products/detail/vishay-semiconductor-opto-division/TSOP38238/1681362).

The FreeCAD project files for an example housing to use to create a fixture are available in the 'cad' directory. 
<p align="center"><img src="https://user-images.githubusercontent.com/6494431/151682075-9c770fee-805d-4c00-8feb-29915d1e1640.png" width="512"></p>
<p align="center"><img src="https://user-images.githubusercontent.com/6494431/151682073-90c98325-bb3a-4a7d-a69d-e6958239a649.JPG" width="512"></p>
<p align="center"><img src="https://user-images.githubusercontent.com/6494431/151682071-3cc23205-c02e-4014-9676-41f723e560ef.JPG" width="512"></p>

---
### Running the test
Zephyr's twister script can be used to run this test on an [nRF52840-DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52840-DK). It should run fine on any device in the nRF52 family with a few modifications (e.g. board overlay file and platform_allow). First generate a hardware-map named **map.yaml** and add a 'rad_fixture' to it like this:
```
- connected: true
  id: 000XXXXXXXXX
  platform: nrf52840dk_nrf52840
  product: J-Link
  runner: nrfjprog
  serial: /dev/ttyACM0
  fixtures:
    - rad_fixture
```
Ensure that the IR LED can be controlled via P0.04 (GPIO_ACTIVE_HIGH) and the receiver's output is on P0.03 (GPIO_ACTIVE_LOW). These pins can be modified in the board overlay if necessary.

Then execute the test:
```
zephyr/scripts/twister -T nrf/tests/drivers/rad/ --device-testing --hardware-map map.yaml
```
