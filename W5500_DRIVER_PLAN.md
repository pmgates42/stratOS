# W5500 Driver Implementation Plan

Implements Stage 7 of [NETWORKING_PLAN.md](./NETWORKING_PLAN.md): a WIZnet W5500
hardware driver for the Raspberry Pi 3B+ (`bcm2xxx`) that plugs into the existing
`STRAT_OS_net_hw_driver_interface_t` and feeds the hardware-independent network
stack. Bench state and wiring are in [W5500_BRINGUP_NOTES.md](./W5500_BRINGUP_NOTES.md)
and [apps/w5500_test/README.md](./apps/w5500_test/README.md).

This plan is written for an implementing agent. Follow the repo conventions in
`.github/copilot-instructions.md` and `.github/instructions/`. Stages are ordered
so every stage builds on `win-sim` and has host-side tests before anything is
flashed. **Hardware gates** are points where the user must run `test.sh w5500_test`
on the Pi; the agent must stop and report at each gate rather than run `test.sh`
itself.

---

## Ground truth the agent must know

### What exists

| Piece | Location | State |
|---|---|---|
| Hardware-driver callback contract | `include/uapi/strat_os_net_hw_driver.h` | `receive(ctx, buf, cap, desc)` / `release(ctx, desc)`; `receive` returns `ERR_NO_DATA` when idle. **No transmit callback yet.** |
| RX descriptor / packet view | `include/uapi/strat_os_net_hw.h` | `STRAT_OS_net_hw_rx_descriptor_t { buffer, capacity, length, complete }` |
| Network driver + RX pool | `include/uapi/strat_os_net_driver.h`, `common/uapi/strat_os_net_driver.c` | `STRAT_OS_NET_DRIVER_init/poll/receive/release_rx_item`; fixed caller-owned pool |
| Model hardware driver | `platform/sim/net/sim_net_hw_driver.c` | Shape to copy: static callbacks, `_init()` fills the interface struct |
| Core SPI driver | `drivers/core/spi/spi_mngr.c`, `spi_tx.c`, `spi_lcl.h` | Pin lookup via `config_lookup_pin()`. `spi_tx_periodic()` is a **write-only** bit-bang with 500 us delays, never reads MISO, no transaction API, and is **not scheduled anywhere**. Not usable for the W5500 as-is. |
| Pin table (bcm2xxx) | `platform/bcm2xxx/bcm_2xxx_spi.c` | CS0=GPIO16, SCLK=21, MOSI=20, MISO=19, RST=26 (`NET_HW_MODULE_PIN_ID__RST`), INT=13 (`NET_HW_MODULE_PIN_ID__INT`) |
| GPIO primitives | `include/peripherals/gpio.h` | `gpio_set/clr/get(pin)`, `gpio_pin_set_func`, `gpio_pin_enable` |
| Timing | `include/utils.h`, `include/peripherals/timer.h` | `delay_us()`, `delay_ms()`, `timer_get_time_us()` |
| Errors | `include/generic.h` | `ERR_NO_ERR, ERR_INVALID, ERR_NO_MEM, ERR_NO_DATA` only |
| Test app scaffold | `apps/w5500_test/w5500_test.c`, `test_apps/apps.json` | Blank Unity app, `bcm2xxx` only, builds clean |
| Driver layout model | `drivers/hc_sr04/`, `build.json` module `hc_sr04` | One dir per device under `drivers/`, one `build.json` module, listed per platform |

### Known conflicts to fix first

- `include/generic.h:67` defines `ERROR_PIN 16`. `boot/boot.c:57-59` drives it as an
  output and clears it at boot; `drivers/core/spi/spi_tx.c:123` sets it. **GPIO 16
  is now SPI CS0.** Move `ERROR_PIN` to an unused GPIO (GPIO 12 / header pin 32 is
  free) or remove the boot-time toggling. Do not leave CS0 shared.
- `spi_init()` (`spi_mngr.c`) copies test bytes into the TX buffer. Harmless, leave it.
- `BCM2XXX_DEBUG_LED_PIN=23` (header pin 16) does not collide with anything.

### W5500 facts the code depends on

- SPI mode 0 or 3; MSB first; CS active low and must stay low for the whole
  frame. Chip tolerates up to 80 MHz, so the bit-bang needs no inter-bit delay.
- **SPI frame (VDM mode):** `addr[15:8]`, `addr[7:0]`, `control`, then N data bytes.
  `control = (BSB << 3) | (RWB << 2) | OM`, with `OM = 0b00` (variable length),
  `RWB = 0` read / `1` write.
  - BSB common registers = `0x00`.
  - BSB socket *n* registers = `n*4 + 1`; socket *n* TX buffer = `n*4 + 2`; socket *n* RX buffer = `n*4 + 3`.
- Multi-byte registers are **big-endian**. Read 16-bit free-running counters
  (`Sn_TX_FSR`, `Sn_RX_RSR`) twice until two reads agree.
- Buffer pointers (`Sn_RX_RD`, `Sn_TX_WR`) are 16-bit free-running; write back
  `old + len` without masking. The chip masks the address to the buffer size, so
  a read/write that crosses the 16 KB boundary wraps automatically.
- Common registers: `MR 0x0000` (bit7 RST), `SHAR 0x0009..0x000E` (MAC),
  `IR 0x0015`, `IMR 0x0016`, `SIMR 0x0018`, `PHYCFGR 0x002E` (bit0 LNK), `VERSIONR 0x0039` (= `0x04`).
- Socket registers (offset within socket BSB): `Sn_MR 0x0000`, `Sn_CR 0x0001`,
  `Sn_IR 0x0002`, `Sn_SR 0x0003`, `Sn_RXBUF_SIZE 0x001E`, `Sn_TXBUF_SIZE 0x001F`,
  `Sn_TX_FSR 0x0020`, `Sn_TX_RD 0x0022`, `Sn_TX_WR 0x0024`, `Sn_RX_RSR 0x0026`,
  `Sn_RX_RD 0x0028`, `Sn_RX_WR 0x002A`, `Sn_IMR 0x002C`.
- `Sn_MR`: MACRAW = `0x04`; set bit7 `MFEN` (`0x84`) so the chip filters to our
  MAC + broadcast instead of flooding us with every frame on the wire.
- `Sn_CR`: `OPEN 0x01`, `CLOSE 0x10`, `SEND 0x20`, `RECV 0x40`. `Sn_CR` reads back
  `0x00` once the command is accepted.
- `Sn_SR`: `SOCK_CLOSED 0x00`, `SOCK_MACRAW 0x42`.
- `Sn_IR` bits: `SEND_OK 0x10`, `TIMEOUT 0x08`, `RECV 0x04`. Write 1 to clear.
- **MACRAW RX layout:** each frame in the RX buffer is prefixed by a 2-byte
  big-endian length that **includes the 2 prefix bytes**. Frame bytes follow; the
  FCS is stripped. Max frame = 1514 bytes, so pool buffers should be >= 1536.
- Buffer sizes are in KB: `Sn_RXBUF_SIZE` / `Sn_TXBUF_SIZE` in {0,1,2,4,8,16};
  the sum across all 8 sockets must be <= 16 for RX and <= 16 for TX.
  Give socket 0 all 16 KB of each and set sockets 1-7 to 0.
- Hardware reset: hold `RST` low >= 500 us, release, wait >= 1 ms before SPI.
  Then software reset with `MR = 0x80` and poll until bit7 clears.

---

## Architecture

```text
apps/*            STRAT_OS_NET_DRIVER_poll() -> rx queue -> Ethernet/IPv4/UDP
                       |
                       v  STRAT_OS_net_hw_driver_interface_t (receive / release [/ transmit])
drivers/w5500/    w5500.c   : reset, init, MACRAW socket 0, RX/TX, net_hw adapter
                  w5500_regs.h : register map + bit constants
                       |
                       v  w5500_bus_t { ctx, begin(), xfer(tx, rx, len), end() }
drivers/core/spi/ spi_transfer API (new, synchronous, full duplex)
                       |
                       v  gpio_set/clr/get on CS0 / SCLK / MOSI / MISO
platform/bcm2xxx  bcm_2xxx_spi.c pin table (already done)
```

The W5500 driver talks to the bus **only** through `w5500_bus_t`. On hardware the
bus is backed by the core SPI transfer API; on `win-sim` the test app supplies a
fake bus that models enough of the W5500 to exercise every code path. This is what
makes the driver testable before any hardware gate.

**Layout decision:** the W5500 is a device driver, so it lives in `drivers/w5500/`
mirroring `drivers/hc_sr04/`. `tools/scripts/create_strat_os_component.py` only
knows the `include/uapi` + `common/` layouts, so for this one component the
`build.json` module is added by hand (copy the `hc_sr04` entry). Everything else
still goes through the scaffold scripts.

---

## Stage 0: Pre-flight fixes

- [x] Move `ERROR_PIN` off GPIO 16 (`include/generic.h`); confirm `boot/boot.c` and
      `drivers/core/spi/spi_tx.c` no longer touch CS0. (Now GPIO 12.)
- [x] Confirm `spi_tx_periodic()` is not registered with the scheduler on any app
      (grep; it is not today). Leave the function in place but it must not run.
      **Finding:** it *is* scheduled by `apps/default/app_default.c` (the
      `default` / `default_bcm` product apps, 30-tick period). It is not scheduled
      by `w5500_test`, so gates 1-3 are unaffected. It must be removed from the
      default app before Stage 6 integration or it will clobber CS0/SCLK/MOSI.
      Left untouched pending user decision.
- [x] Add `"w5500"` module to `build.json` (`drivers/w5500/*.c`, include
      `drivers/w5500`) and list it in **both** platforms' `modules` so the driver
      compiles on `win-sim`.
- [x] Add a `win-sim` profile to `w5500_test` in `test_apps/apps.json` (copy the
      `platform_config.win-sim` block from `net_driver_test`). The hardware-only
      tests will `TEST_IGNORE` on `win-sim`.
- [x] Build `default_bcm` and `w5500_test` for both platforms; zero new warnings.

## Stage 1: Synchronous SPI transfer API

**Status: done** (`drivers/core/spi/spi_xfer.c`, declared in
`include/peripherals/spi.h`; `spi_lcl_is_initialized()` added to `spi_mngr.c`;
`spi_init()` now also drives SCLK low). `test/unit/spi/run.sh` does not exist
(only `test/unit/ssched/` has one), so there was nothing to keep passing there.

Add to `include/peripherals/spi.h` / `drivers/core/spi/`:

```c
error_type spi_begin(uint8_t cs_id);                               /* CS low   */
error_type spi_xfer(const uint8_t *tx, uint8_t *rx, size_t len);   /* either may be NULL */
error_type spi_end(uint8_t cs_id);                                 /* CS high  */
```

- Mode 0 bit-bang per byte, MSB first: set MOSI, (optional `delay_us`), SCLK high,
  sample MISO, SCLK low. Start with a `SPI_BITBANG_HALF_PERIOD_US` of 1 and make it
  a `#define` so it can drop to 0 once the hardware gate passes.
- `tx == NULL` shifts out `0x00`; `rx == NULL` discards input.
- Pins come from `spi_lcl_get_hw_intf_data()` exactly as `spi_tx_periodic()` does.
  Only `cs_id == 0` is supported; return `ERR_INVALID` otherwise.
- `spi_init()` must leave SCLK low and CS0 high (mode 0 idle).
- Keep `test/unit/spi/run.sh` passing (it has its own make-based build).
- Tests (`win-sim`, in `w5500_test` or a small new `spi_xfer_test`): `spi_xfer`
  with NULL tx/rx combinations returns `ERR_NO_ERR`; `spi_begin(1)` returns
  `ERR_INVALID`. Bit-level behaviour is covered by the fake bus in Stage 2 and the
  hardware gate in Stage 3.

## Stage 2: W5500 register layer, reset, init

**Status: done.** Files: `drivers/w5500/w5500.h`, `drivers/w5500/w5500_regs.h`,
`drivers/w5500/w5500.c`; fake bus in `apps/w5500_test/w5500_fake_bus.{c,h}`.
`w5500_init` resolves `rst_pin` itself via
`config_lookup_pin(NET_HW_MODULE_PIN_ID__RST)` (unregistered on `win-sim`, so
`W5500_PIN_NOT_WIRED` there). All Stage 2 tests below pass on `win-sim`.

Files: `drivers/w5500/w5500.h`, `drivers/w5500/w5500_regs.h`, `drivers/w5500/w5500.c`.

Public (internal-driver) API, lowercase `snake_case`, all returning `error_type`:

```c
typedef struct
{
    void *context;
    error_type (*begin)(void *context);
    error_type (*xfer)(void *context, const uint8_t *tx, uint8_t *rx, size_t len);
    error_type (*end)(void *context);
} w5500_bus_t;

typedef struct
{
    w5500_bus_t bus;
    uint32_t    rst_pin;        /* from NET_HW_MODULE_PIN_ID__RST; 0xFFFFFFFF = not wired */
    uint8_t     mac[6];
    boolean     initialized;
} w5500_t;

error_type w5500_bus_from_spi(w5500_bus_t *bus);                 /* wraps spi_begin/xfer/end */
error_type w5500_read (w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t *data, size_t len);
error_type w5500_write(w5500_t *dev, uint8_t bsb, uint16_t addr, const uint8_t *data, size_t len);
error_type w5500_read8 / write8 / read16 / write16 (...);        /* big-endian helpers */
error_type w5500_read16_stable(...);                             /* read until two agree */
error_type w5500_hw_reset(w5500_t *dev);                         /* RST pulse via gpio, or no-op */
error_type w5500_sw_reset(w5500_t *dev);                         /* MR.RST, poll clear, timeout */
error_type w5500_read_version(w5500_t *dev, uint8_t *version);
error_type w5500_init(w5500_t *dev, const w5500_bus_t *bus, const uint8_t mac[6]);
error_type w5500_link_up(w5500_t *dev, boolean *up);             /* PHYCFGR.LNK */
```

`w5500_init` sequence: `hw_reset` -> `sw_reset` -> verify `VERSIONR == 0x04`
(return `ERR_INVALID` and `debug_printf("\n[W5500] bad version 0x%02x")` if not)
-> write `SHAR` -> set `Sn_RXBUF_SIZE/Sn_TXBUF_SIZE` = 16 for socket 0, 0 for 1-7
-> `Sn_MR(0) = 0x84` -> `Sn_CR(0) = OPEN` -> poll `Sn_SR(0) == 0x42` with timeout
-> `initialized = TRUE`.

Default MAC: locally administered `02:53:54:52:41:54`. Expose it as a `#define`
so apps can override.

Timeouts use `timer_get_time_us()`; never spin unbounded. Command acceptance:
after writing `Sn_CR`, poll until it reads `0x00`.

**Fake bus for `win-sim` tests** (`apps/w5500_test/w5500_fake_bus.{c,h}`, listed
in the test app's `sources`): models a 16-bit address space per BSB for common and
socket-0 registers, a 16 KB RX buffer and 16 KB TX buffer, and reacts to writes:
`MR.RST` clears itself; `Sn_CR=OPEN` sets `Sn_SR=0x42` when `Sn_MR&0x0F==0x04`;
`Sn_CR=RECV`/`SEND` are recorded and clear `Sn_CR`; `SEND` sets `Sn_IR|=SEND_OK`
and advances `Sn_TX_RD`. Helpers let tests inject a frame into the RX buffer
(writing the 2-byte length prefix, updating `Sn_RX_WR`/`Sn_RX_RSR`) and read back
what was written to the TX buffer. It also logs every frame header so tests can
assert the exact control byte and address.

Stage 2 tests (`win-sim`):
- control byte encoding for common read, socket-0 reg write, RX-buffer read, TX-buffer write
- `w5500_read16` returns big-endian value; `read16_stable` retries when the fake
  returns two different values then settles
- `w5500_init` on a fake with `VERSIONR=0x04` succeeds and leaves `Sn_SR=0x42`,
  `SHAR` = MAC, buffer sizes 16/0/0/...; with `VERSIONR=0x00` returns `ERR_INVALID`
- NULL-pointer rejection on every public function

## Stage 3: Hardware gate 1 — VERSIONR on the Pi

**Status: tests written, waiting on the user to flash.** See "Progress log" at
the end of this document for exactly what to run and what to expect.

Agent adds to `w5500_test.c`, wrapped in `#ifdef EMBEDDED_BUILD` (else `TEST_IGNORE`):
- `test_version_register_reads_0x04` — real bus via `w5500_bus_from_spi`, hw reset,
  read `VERSIONR`, assert `0x04`. Print the raw value with `debug_printf` on failure.
- `test_init_opens_macraw_socket` — `w5500_init`, assert `Sn_SR(0) == 0x42`.
- `test_phy_reports_link_when_cable_connected` — `TEST_IGNORE_MESSAGE` if LNK=0
  (cable may be unplugged), else pass.

**Stop and report.** The user runs `test.sh w5500_test`, flashes, and reads UART.
If `VERSIONR` is wrong: `0xFF` = MISO stuck high (wiring/CS), `0x00` = no power or
MISO stuck low, other = clock/phase problem. Do not continue to Stage 4 until this
passes; a failing read is a wiring or SPI-mode bug, not a driver bug.

## Stage 4: MACRAW receive + net_hw adapter

**Blocker for the end-to-end test:** `common/uapi/strat_os_net_driver.c` is still
a stub (every `STRAT_OS_NET_DRIVER_*` returns `ERR_INVALID`; NETWORKING_PLAN
Stage 5 is unticked) and `net_driver_test` already fails 2 of 3 tests at
baseline. `w5500_recv_frame`, the adapter, and the fake-bus tests can all be
built and tested without it; only the
`STRAT_OS_NET_DRIVER_init/poll/receive` test must wait for Stage 5 of
NETWORKING_PLAN or be written against the adapter callbacks directly.

Add to `w5500.c`:

```c
error_type w5500_recv_frame(w5500_t *dev, uint8_t *buf, size_t cap, size_t *len);
error_type w5500_get_net_hw_interface(w5500_t *dev, STRAT_OS_net_hw_driver_interface_t *iface);
```

`w5500_recv_frame`:
1. `rsr = read16_stable(Sn_RX_RSR)`; if `0` -> `ERR_NO_DATA`.
2. `rd = read16(Sn_RX_RD)`; read 2 bytes at RX-buffer BSB offset `rd` -> `total`.
3. If `total < 2` or `total - 2 > rsr` -> corrupt; recover by `sw_reset`+re-init
   path or by consuming `rsr` bytes; return `ERR_INVALID`.
4. `frame_len = total - 2`. If `frame_len > cap`: consume it anyway
   (`Sn_RX_RD = rd + total`, `Sn_CR = RECV`) and return `ERR_NO_MEM`. Never leave
   a frame half-consumed.
5. Read `frame_len` bytes at `rd + 2`; `Sn_RX_RD = rd + total`; `Sn_CR = RECV`;
   `*len = frame_len`.

Adapter (`static` functions in `w5500.c`, same shape as `sim_net_hw_driver.c`):
- `receive(ctx, buf, cap, desc)` -> `w5500_recv_frame`; on success fill
  `desc->{buffer, capacity, length, complete=TRUE}`. Map `ERR_NO_DATA` straight through.
- `release(ctx, desc)` -> validate pointers, return `ERR_NO_ERR`. Hardware memory
  was already released inside `receive` (the frame was copied out), so the
  descriptor's buffer belongs to the network-driver pool and nothing else is owed.
  Document this ownership rule in the function comment.

Stage 4 tests (`win-sim`, fake bus): one injected frame is returned with correct
length and bytes and `Sn_RX_RD`/`RECV` advance by `total`; two injected frames
come out in order; oversize frame returns `ERR_NO_MEM` and is consumed so the next
call returns `ERR_NO_DATA`; frame placed to straddle the 16 KB boundary is read
correctly (fake models the wrap); empty RSR returns `ERR_NO_DATA`; end-to-end
through `STRAT_OS_NET_DRIVER_init/poll/receive` with the adapter, then
`STRAT_OS_NET_ETHERNET_decode` (or the existing Ethernet API) parses the injected
frame's dst/src/ethertype.

**Hardware gate 2:** test that polls `w5500_recv_frame` for up to 10 s and asserts
at least one frame arrives while the host generates broadcast traffic (ARP from a
`ping` to an unused address on the same link is enough; `tools/scripts/udp_spam.py`
also works once IPv4 is bound). Print dst/src MAC and ethertype over UART.
`TEST_IGNORE` if LNK=0. **Stop and report.**

## Stage 5: Transmit

Add to `w5500.c`:

```c
error_type w5500_send_frame(w5500_t *dev, const uint8_t *frame, size_t len);
```

1. Reject `len < 14` or `len > 1514`.
2. `fsr = read16_stable(Sn_TX_FSR)`; if `len > fsr` -> `ERR_NO_MEM`.
3. `wr = read16(Sn_TX_WR)`; write `len` bytes at TX-buffer BSB offset `wr`;
   `Sn_TX_WR = wr + len`; `Sn_CR = SEND`.
4. Poll `Sn_IR` for `SEND_OK` or `TIMEOUT` with a bounded wait; write the bit back
   to clear it. `TIMEOUT` -> `ERR_INVALID` (no better code exists; log it).

**Interface decision (needs user sign-off before implementing):** the hardware
driver interface has no transmit callback. Proposed additive change to
`include/uapi/strat_os_net_hw_driver.h`:

```c
error_type (*transmit)(void *context, const uint8_t *frame, size_t length);
```

`sim_net_hw_driver.c` implements it by recording the last frame; `net_driver_test`
gains one test; W5500 adapter maps it to `w5500_send_frame`. The field may be
NULL for receive-only hardware; callers must check. This is the only public
interface change in the plan.

Stage 5 tests (`win-sim`, fake bus): sent bytes land at `Sn_TX_WR`, pointer
advances by `len`, `SEND` issued, `SEND_OK` cleared; `len > FSR` -> `ERR_NO_MEM`
with no pointer change; wrap at 16 KB; bad lengths rejected.

**Hardware gate 3:** send one broadcast frame (dst `ff:ff:ff:ff:ff:ff`, src = our
MAC, ethertype `0x88B5` experimental, payload "STRATOS") once per second for 10 s;
user captures with Wireshark on the PC NIC filtered to `eth.type == 0x88b5`.
**Stop and report.**

## Stage 6: Integration

- [ ] Bind `w5500` to one `STRAT_OS_net_driver_t` in a bcm2xxx app: init in a
      setup hook, `STRAT_OS_NET_DRIVER_poll()` from a scheduler task (10-20 ms
      period is fine for bit-banged SPI), drain `receive()` into the Ethernet layer.
- [ ] Decide where device init belongs: `boot.c::setup_drivers()` behind a
      `HW_DRIVER_W5500` define (mirrors the `HW_DRIVER_HC_SR04` pattern) or app-level.
      Prefer app-level until the stack is stable.
- [ ] Complete NETWORKING_PLAN Stage 7's last two items (real Ethernet frame with
      a host, then real UDP) and tick them off.
- [ ] Once hardware gate 1 passes, try `SPI_BITBANG_HALF_PERIOD_US 0` and re-run
      the gates; record the achieved throughput in `W5500_BRINGUP_NOTES.md`.
- [ ] Update `NETWORKING_PLAN.md` Stage 7 checkboxes and `W5500_BRINGUP_NOTES.md`
      as each gate passes.

---

## Rules for the implementing agent

- Follow `.github/instructions/c-conventions.instructions.md`: `generic.h` first, no
  libc in `EMBEDDED_BUILD`, `error_type` returns, validate pointers first, `static`
  + `s_` prefix for module state, forward-declare statics, `debug_printf("\n[W5500] ...")`,
  no `%ll`/`%p`.
- Match `drivers/hc_sr04/` style (4 spaces, Allman) in `drivers/w5500/`.
- Build `w5500_test` for `win-sim` after every stage and run it; build `default_bcm`
  and `w5500_test` for `bcm2xxx` to make sure the cross build stays clean.
  Zero new warnings.
- Never run `test.sh`; it needs the SD card. Stop at each hardware gate and report
  exactly what to flash and what UART output to expect.
- Do not modify `test/unit/libs/`.
- Do not change Ethernet, IPv4, UDP, or socket code. The only permitted public
  interface change is the optional `transmit` callback in Stage 5, after sign-off.
- Keep the fake bus in the test app, not in the driver.

## Definition of done

- `w5500_test` passes on `win-sim` (register layer, RX, TX against the fake bus)
  and on the Pi (`VERSIONR`, MACRAW open, receive broadcast, transmit visible in
  Wireshark).
- A bcm2xxx app receives a real UDP datagram through the existing
  `STRAT_OS_NET_*` layers with no hardware-specific code above `drivers/w5500/`.
- `NETWORKING_PLAN.md` Stage 7 and `W5500_BRINGUP_NOTES.md` are updated.

---

## Progress log

### 2026-10-31 — Stages 0-3 implemented, stopped at hardware gate 1

**What was done**

| Area | Files | Notes |
|---|---|---|
| Stage 0 | `include/generic.h`, `build.json`, `test_apps/apps.json` | `ERROR_PIN` -> GPIO 12; `w5500` module on both platforms; `w5500_test` builds for `win-sim` + `bcm2xxx`, sources now include the fake bus, app `includes` adds `drivers/w5500`. |
| Stage 1 | `drivers/core/spi/spi_xfer.c` (new), `spi_lcl.h`, `spi_mngr.c`, `include/peripherals/spi.h` | `spi_begin/spi_xfer/spi_end`, mode 0, MSB first, `SPI_BITBANG_HALF_PERIOD_US` default 1 (`#ifndef`, so it can be set from cflags). Requires `spi_init()`; returns `ERR_INVALID` otherwise instead of asserting. |
| Stage 2 | `drivers/w5500/w5500_regs.h`, `w5500.h`, `w5500.c` (new) | Everything in the Stage 2 API list. Timeouts 100 ms via `timer_get_time_us()`. |
| Stage 2 tests | `apps/w5500_test/w5500_fake_bus.{c,h}` (new), `apps/w5500_test/w5500_test.c` | Fake models common regs, 8 socket reg blocks, 16 KB RX/TX for socket 0, `MR.RST`, `OPEN/CLOSE/RECV/SEND`, W1C `Sn_IR`, RSR/FSR from pointers, header log, counter-read fault injection. |
| Stage 3 | `apps/w5500_test/w5500_test.c` (`#ifdef EMBEDDED_BUILD`) | `test_version_register_reads_0x04`, `test_init_opens_macraw_socket`, `test_phy_reports_link_when_cable_connected`. |

**Verification**

- `python3 strat_build.py w5500_test --apps-config test_apps/apps.json --platform win-sim`
  then `timeout -s INT 10 stdbuf -o0 ./build/win-sim/w5500_test`:
  `19 Tests 0 Failures 3 Ignored` (ignores are the hardware-only tests).
  The binary never exits on its own (`sched_main()` loops), hence `timeout`.
- `w5500_test` (bcm2xxx), `default_bcm`, `default` (win-sim) all build with
  `--rebuild`; only the pre-existing `kernel8.elf ... RWX permissions` linker
  warning and the `platform/sim/**/*.S matched no files` glob warning.

**Things the next agent should know**

- Tests run from `APP_sched_configure()`, which `boot.c` calls *before*
  `setup_drivers()`/`spi_init()`. The test app therefore calls `spi_init()`
  itself (`ensure_spi_ready()`); `boot.c` calls it again afterwards, which only
  re-registers the same pins (duplicates are tolerated, well under
  `CFG_PIN_CFG_MAX_PINS`).
- `delay_ms()` does not exist on `win-sim`; the driver uses `delay_us()` only.
- `spi_tx_periodic()` is scheduled by `apps/default/app_default.c`. Remove it
  (or stop scheduling it) before any bcm2xxx app binds the W5500 (Stage 6).
- `STRAT_OS_NET_DRIVER_*` is a stub; see the Stage 4 blocker note.
- The fake bus is a ~33 KB static in the test app and also compiles/runs on
  `bcm2xxx`, so the on-target run executes the fake-bus tests too.

**Hardware gate 1 — what the user runs**

1. `test.sh w5500_test` (builds bcm2xxx, copies `kernel8.img`/`kernel8.elf` to
   the SD card), boot the Pi with the UART adapter attached.
2. Expected UART output (interleaved with Unity lines):
   - 16 fake-bus / SPI-API tests `PASS`
   - `[W5500] VERSIONR = 0x04 (rst pin 26)` then
     `test_version_register_reads_0x04:PASS`
   - `[W5500] Sn_SR(0) = 0x42` then `test_init_opens_macraw_socket:PASS`
   - `[W5500] PHY link UP` + `PASS`, or `link DOWN` + `IGNORE` if no cable
   - `19 Tests 0 Failures N Ignored` / `OK`
3. If `VERSIONR` is wrong the test prints a hint: `0xFF` = MISO stuck high
   (wiring/CS), `0x00` = no power or MISO stuck low, anything else = clock/phase.
   That is a wiring or SPI-mode problem, not a driver bug; fix it before
   Stage 4. If the SPI bit-bang looks suspicious, try
   `-DSPI_BITBANG_HALF_PERIOD_US=5` in the `bcm2xxx` cflags of `w5500_test`.

**Next steps once gate 1 passes**

- Stage 4: `w5500_recv_frame` + `w5500_get_net_hw_interface` in `w5500.c`,
  fake-bus tests (the fake already supports `W5500_FAKE_BUS_inject_frame`,
  wrap-around via `W5500_BUFFER_MASK`, and counts `recv_commands`).
- Try `SPI_BITBANG_HALF_PERIOD_US 0` and record throughput in
  `W5500_BRINGUP_NOTES.md`.
- Tick `NETWORKING_PLAN.md` Stage 7 items: SPI frame, `VERSIONR`, hw/sw reset +
  `SHAR`, buffer sizes + MACRAW open.
