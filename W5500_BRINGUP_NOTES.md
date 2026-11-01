# W5500 Lite Bring-Up Notes

Progress log for Stage 7 of [NETWORKING_PLAN.md](./NETWORKING_PLAN.md): connecting
a WIZnet W5500 Lite Ethernet module to the Raspberry Pi 3B+ target.

## 2026-10-31: Hardware identification and wiring review

### Module identification

- The module on the breadboard was originally assumed to be an ENC28J60. The
  PCB silkscreen reads **"W5500 Lite"** and the header labels match the WIZnet
  W5500 Lite board, so the plan and driver comments now target the W5500.
- Consequences for the driver:
  - Different register map and SPI frame format (16-bit address + control byte
    + data) from the ENC28J60.
  - The W5500 is used in **MACRAW mode on socket 0**, which delivers raw
    Ethernet frames. The network-driver queue, Ethernet, IPv4, UDP, and socket
    layers are unaffected.
  - The Lite board is **3.3 V only** (no onboard regulator). Never feed it 5 V.
    Expect roughly 130-180 mA with link up.

### Module pinout (read from the silkscreen)

`G` is at the RJ45 end of both headers. Both `G` pins are the same net.

| J1  | Function              | J2  | Function              |
|-----|-----------------------|-----|-----------------------|
| G   | GND                   | G   | GND                   |
| MO  | MOSI (Pi -> W5500)    | V   | 3.3 V                 |
| SCK | SPI clock             | NC  | not connected         |
| CS  | chip select, active-low | RST | reset, active-low   |
| INT | interrupt out, active-low (optional) | MI | MISO (W5500 -> Pi) |

### Pi 3B+ wiring (matches `platform/bcm2xxx/bcm_2xxx_spi.c`)

SPI is bit-banged, so any GPIO works. The pin table was moved from the SPI0
pins to the USB end of the header (SPI1 layout) to match the existing bench
wiring. `RST`/`INT` are registered under `CONFIG_MODULE_ID__NET_HW` as
`NET_HW_MODULE_PIN_ID__RST` / `NET_HW_MODULE_PIN_ID__INT`.

| W5500 Lite | Pi header pin | Pi signal                       |
|------------|---------------|---------------------------------|
| V          | 1 or 17       | 3.3 V                           |
| G          | 39            | GND                             |
| SCK        | 40            | GPIO 21 / SPI1 SCLK             |
| MO         | 38            | GPIO 20 / SPI1 MOSI             |
| MI         | 35            | GPIO 19 / SPI1 MISO             |
| CS         | 36            | GPIO 16 / SPI1 CE2              |
| RST        | 37            | GPIO 26                         |
| INT        | 33            | GPIO 13 (optional; polling ok)  |

### Current bench state (from the setup video)

- Power (`V`, `G`) is **not yet connected**; the red and black leads on J2 have
  free ends.
- The six signal jumpers are plugged into the **USB end of the 40-pin header**.
  The driver pin table originally targeted SPI0 (GPIO 8/9/10/11); it was
  changed to the USB-end GPIOs above instead of moving the wires. Verify each
  jumper lands on the pin listed in the table.
- UART debugger is on the pin-1 end of the header (approximately pins 6/8/10,
  GND/TXD/RXD) and looks correct.
- The breadboard has leftover short jumpers in rows 2-19 and a LAFVIN Arduino
  nearby. Clear them so nothing shorts a W5500 signal.
- Neither the UART adapter nor the module's Ethernet cable is connected to the
  PC yet.

### Wiring verification (photos, same day)

Module side confirmed from still photos:

- J1: `G` empty, `MO` blue, `SCK` orange, `CS` purple, `INT` empty (polling).
- J2: `G` black, `V` red, `NC` empty, `RST` brown/tan, `MI` green.

Pi side: in the pins 33-40 block, purple/blue/orange sit on the edge column
(36/38/40) and green/brown/black on the inner column (35/37/39), matching the
table. The red 3V3 lead is on pin 17 (inner column, 9th pin up from pin 1);
the photo angle made its column ambiguous, so it was confirmed by counting.
A stray black jumper is plugged into the breadboard's right (blue) power rail
near row 18-19 and is not part of the W5500 wiring.

Sanity check before first power-on: ~3.3 V across module `V`/`G` with the Pi
running.

### Repo changes

- `NETWORKING_PLAN.md`: ENC28J60 references renamed to W5500; Stage 7 rewritten
  with the hardware section, pin tables, and W5500-specific checklist
  (`VERSIONR == 0x04`, MACRAW socket 0, `Sn_RX_RD`/`RECV`, `Sn_TX_WR`/`SEND`).
- `include/uapi/strat_os_net_hw_driver.h`: comment updated to reference the
  W5500.
- `include/config.h`: added `CONFIG_MODULE_ID__NET_HW` and
  `NET_HW_MODULE_PIN_ID__RST` / `NET_HW_MODULE_PIN_ID__INT`.
- `platform/bcm2xxx/bcm_2xxx_spi.c`: pin table moved to the USB-end GPIOs
  (CS0=16, SCLK=21, MOSI=20, MISO=19) and gained a module column; registers
  RST (GPIO 26, output) and INT (GPIO 13, input) under `NET_HW`.
- `apps/w5500_test/`: scaffolded on-target test app (bcm2xxx only) via
  `tools/scripts/create_test_app.py`. Wiring diagrams live in
  `apps/w5500_test/docs/` (`wiring_bench.png` is the bench orientation).
- Driver plan Stages 0-3 (same day, see `W5500_DRIVER_PLAN.md` progress log):
  `ERROR_PIN` moved to GPIO 12 (was GPIO 16 = CS0); synchronous
  `spi_begin/spi_xfer/spi_end` in `drivers/core/spi/spi_xfer.c`; W5500 driver in
  `drivers/w5500/`; `w5500_test` now also builds for `win-sim` with a fake bus
  and carries the gate-1 hardware tests (`VERSIONR`, MACRAW open, PHY link).

## Next steps

- [x] Verify each signal jumper lands on the header pin listed above
      (compare against `apps/w5500_test/docs/wiring_bench.png`).
- [x] Connect `V` to 3.3 V (pin 17) and `G` to pin 39.
- [ ] Measure ~3.3 V across module `V`/`G` with the Pi powered.
- [ ] Remove or account for the stray black jumper on the breadboard power rail.
- [ ] Connect the UART adapter to the PC and confirm boot output.
- [ ] Write the first SPI register read and check `VERSIONR` returns `0x04`.
      (Code done: `test_version_register_reads_0x04` in `apps/w5500_test`;
      needs `test.sh w5500_test` on the Pi.)
- [ ] Continue with [W5500_DRIVER_PLAN.md](./W5500_DRIVER_PLAN.md) (stage-by-stage
      driver implementation with hardware gates), which tracks the Stage 7
      checklist in `NETWORKING_PLAN.md`.
