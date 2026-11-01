# StratOS Networking Plan

This plan develops networking from simulator-tested, hardware-independent layers toward a real WIZnet W5500 driver. Each layer should be complete and tested before the next layer depends on it.

## Architecture

```text
hardware driver instance
    -> hardware-driver interface
    -> network-driver instance and RX queue
    -> STRAT_OS_net_packet_t
    -> Ethernet frame layer
    -> IPv4 layer
    -> UDP layer
    -> UDP socket queues
    -> application API
```

The packet buffer is a view over storage:

```text
0 <= offset
0 <= length
offset + length <= capacity
active bytes = data + offset, length bytes
```

The driver owns the packet while it is receiving or transmitting. Ownership moves between queues and layers explicitly; a packet must have one owner at a time.

Each network-driver instance is bound to one hardware-driver instance during
system initialization. The hardware driver knows the device, such as the
W5500 or simulator. The network driver knows only the generic receive,
release, and queue operations.

## Stage 0: Stabilize the Scaffold

- [x] Create the `net_hw` public component and simulator test app.
- [x] Create the `net_ethernet` public component and simulator test app.
- [x] Create the `net_ipv4` public component and simulator test app.
- [x] Create the `net_udp` public component and simulator test app.
- [x] Create the `net_driver` public component and simulator test app.
- [x] Define `STRAT_OS_net_packet_t` with `data`, `capacity`, `offset`, and `length`.
- [x] Define public hardware descriptor, Ethernet frame, and UDP datagram types.
- [x] Define the hardware-driver callback interface.
- [x] Define fixed RX-item and packet-buffer pool storage.
- [x] Define simulator hardware callbacks that fill caller-provided packet buffers.
- [x] Define the public IPv4 packet view.
- [x] Add linkable function stubs.
- [x] Add black-box tests for each public layer.
- [x] Confirm the networking simulator test applications compile and link.

## Stage 1: Implement Packet and Descriptor Handling

Target: `net_hw` and `net_hw_test`.

- [x] Implement packet initialization.
- [x] Reject null packet, null buffer, and zero-capacity inputs.
- [x] Implement descriptor-to-packet conversion.
- [x] Reject incomplete descriptors.
- [x] Reject descriptor lengths larger than descriptor capacity.
- [x] Set `offset` to zero for a newly received frame.
- [x] Set `length` to the received byte count.
- [x] Add tests for zero-length frames and boundary-sized frames.
- [x] Add tests proving `offset + length <= capacity`.
- [x] Decide whether packet initialization clears the buffer or only initializes metadata. (Decision: metadata only.)

## Stage 2: Implement Ethernet Frames

Target: `net_ethernet` and `net_ethernet_test`.

- [x] Decode destination MAC address.
- [x] Decode source MAC address.
- [x] Decode EtherType in network byte order.
- [x] Reject packets shorter than the Ethernet header.
- [x] Reject packets whose active range exceeds `capacity`.
- [x] Remove or advance past the Ethernet header without copying payload data.
- [x] Preserve the packet payload as the active `data + offset` range.
- [x] Add tests for IPv4 EtherType.
- [x] Add tests for unsupported EtherTypes. (Decode accepts them; the caller dispatches.)
- [x] Add tests with nonzero initial packet offsets.
- [x] Add tests for malformed packet lengths.

## Stage 3: Add an IPv4 Layer

The IPv4 component and simulator test app are scaffolded. Implement the non-fragmented path first; fragmentation remains a later design and implementation task.

- [x] Define an IPv4 header view.
- [ ] Decode source and destination addresses.
- [ ] Decode protocol, total length, identification, flags, and fragment offset.
- [ ] Validate IPv4 header length and total length.
- [ ] Validate the IPv4 header checksum.
- [ ] Reject malformed packets without modifying the packet view.
- [ ] Support non-fragmented IPv4 packets first.
- [ ] Decide whether unsupported fragments are rejected initially.
- [ ] Add an explicit fragment reassembly design before implementing fragmentation.
- [ ] Add tests for valid IPv4/UDP packets.
- [ ] Add tests for invalid checksum and invalid length fields.

## Stage 4: Implement UDP Datagrams

Target: `net_udp` and `net_udp_test`.

- [ ] Decode source port and destination port.
- [ ] Decode UDP length and checksum in network byte order.
- [ ] Validate that UDP length is at least the UDP header size.
- [ ] Validate that UDP length fits inside the active packet range.
- [ ] Validate the UDP checksum when IPv4 pseudo-header information is available.
- [ ] Expose the payload as `data + offset` and `payload_length`.
- [ ] Advance the packet view past the UDP header.
- [ ] Preserve one UDP datagram as one packet boundary.
- [ ] Add tests for empty payloads.
- [ ] Add tests for multiple payload sizes.
- [ ] Add tests for invalid length and checksum fields.

## Stage 5: Add Queues and Ownership

The network-driver RX queue, fixed buffer pool, and hardware-driver boundary
are scaffolded. Build the reusable queue behavior and ownership rules needed
by receive and transmit paths.

- [ ] Define a packet queue using the existing intrusive linked-list node.
- [ ] Implement queue initialization.
- [ ] Implement push and pop operations.
- [ ] Track queue length and enforce optional limits.
- [ ] Define ownership transfer rules for push, pop, release, and drop.
- [ ] Test FIFO ordering with multiple packets.
- [ ] Test empty-queue behavior.
- [ ] Test queue-full behavior.
- [ ] Test packet reuse after release.
- [ ] Decide whether queues use fixed packet storage, a data pool, or caller-owned packets.
- [ ] Implement `STRAT_OS_NET_DRIVER_init` with one hardware interface per network-driver instance.
- [ ] Implement `STRAT_OS_NET_DRIVER_poll` to obtain one descriptor from the hardware driver and enqueue it.
- [ ] Implement `STRAT_OS_NET_DRIVER_receive` to dequeue the oldest completed descriptor.
- [ ] Implement `STRAT_OS_NET_DRIVER_release_rx_item` to return ownership to the hardware driver.
- [x] Add simulator hardware callbacks that provide a pending frame through the hardware-driver interface.

## Stage 6: Build a Simulator UDP Path

Target: a complete hardware-independent loopback path.

```text
application
    -> UDP transmit queue
    -> simulated network device
    -> Ethernet/IP/UDP receive path
    -> UDP socket receive queue
    -> application
```

- [ ] Implement the simulator hardware-driver backend using the generic hardware-driver interface.
- [ ] Move packets from simulated TX to simulated RX.
- [ ] Add a UDP socket object with local port and receive queue.
- [ ] Implement `sendto`-style datagram submission.
- [ ] Implement `recvfrom`-style datagram retrieval.
- [ ] Support multiple sockets with different local ports.
- [ ] Test multiple queued datagrams.
- [ ] Test delivery to the correct destination port.
- [ ] Test unknown-port behavior.
- [ ] Test packet loss and queue exhaustion behavior.
- [ ] Test that application reads do not expose Ethernet/IP/UDP headers.

## Stage 7: Add the W5500 Hardware Driver Last

Detailed implementation plan: [W5500_DRIVER_PLAN.md](./W5500_DRIVER_PLAN.md).

The W5500 driver should implement the existing hardware-driver interface. It
should not change the network-driver queue, Ethernet, IPv4, UDP, or socket
semantics. The W5500 is used in **MACRAW mode on socket 0** so that it hands
StratOS raw Ethernet frames; its built-in TCP/IP offload is not used, which
keeps the Ethernet, IPv4, and UDP layers above it unchanged.

### Hardware: W5500 Lite module on the Raspberry Pi 3B+

The W5500 Lite module is **3.3 V only** (no onboard regulator). Power it from
a Pi 3.3 V pin, never 5 V. It draws roughly 130-180 mA with link up. Both `G`
pins on the module are the same net; one is enough.

Module silkscreen: J1 = `G MO SCK CS INT`, J2 = `G V NC RST MI`, with `G` at
the RJ45 end of each header.

The mapping below matches the bit-banged pin table in
`platform/bcm2xxx/bcm_2xxx_spi.c`. It follows the SPI1 layout at the USB end
of the 40-pin header, so it stays valid if the hardware SPI1 peripheral is
used later. `RST` and `INT` are registered under `CONFIG_MODULE_ID__NET_HW`.

| W5500 Lite | Pi header pin | Pi signal |
|------------|---------------|-----------|
| V          | 1 or 17       | 3.3 V     |
| G          | 39            | GND       |
| SCK        | 40            | GPIO 21 / SPI1 SCLK |
| MO         | 38            | GPIO 20 / SPI1 MOSI |
| MI         | 35            | GPIO 19 / SPI1 MISO |
| CS         | 36            | GPIO 16 / SPI1 CE2  |
| RST        | 37            | GPIO 26 (`NET_HW_MODULE_PIN_ID__RST`) |
| INT        | 33            | GPIO 13 (`NET_HW_MODULE_PIN_ID__INT`, optional; polling is fine) |

- [ ] Obtain and review the W5500 datasheet (register map, SPI frame format, MACRAW mode).
- [ ] Verify module voltage and logic levels before connecting it (3.3 V only).
- [ ] Confirm SPI pin wiring, chip select, interrupt, and reset wiring against the table above.
- [ ] Implement the W5500 SPI frame: 16-bit address, control byte (block select, R/W, VDM), data.
- [ ] Read and verify the `VERSIONR` register (expect `0x04`).
- [ ] Implement hardware reset (`RST` low >= 500 us) and software reset (`MR.RST`), then set `SHAR` (MAC address).
- [ ] Configure socket 0 RX/TX buffer sizes (`Sn_RXBUF_SIZE`, `Sn_TXBUF_SIZE`) and open socket 0 in MACRAW mode.
- [ ] Read one received frame: `Sn_RX_RSR`, then the 2-byte length prefix at `Sn_RX_RD`.
- [ ] Copy one received frame into a StratOS-owned packet buffer.
- [ ] Populate `data`, `capacity`, `offset`, and `length` correctly.
- [ ] Implement receive release: advance `Sn_RX_RD` and issue `Sn_CR = RECV`.
- [ ] Implement one-frame transmission: write at `Sn_TX_WR`, advance it, issue `Sn_CR = SEND`, wait for `SEND_OK`.
- [ ] Add interrupt (`INT` / `Sn_IR`) or polling-based receive detection.
- [ ] Bind one network-driver instance to the W5500 hardware-driver instance.
- [ ] Connect the driver to the existing Ethernet layer without changing its public API.
- [ ] Test a real Ethernet frame between StratOS and a host computer.
- [ ] Test real UDP traffic after the simulator path is passing.

## Completion Criteria

- [ ] Every protocol layer has tests that use only its public interface.
- [ ] Malformed input is rejected without out-of-bounds reads or invalid packet views.
- [ ] Packet ownership is documented and enforced at every queue boundary.
- [ ] Simulator loopback can send and receive multiple UDP datagrams.
- [ ] The W5500 driver can exchange raw Ethernet frames without protocol logic leaking into the hardware layer.
- [ ] Hardware-specific code is isolated from Ethernet, IPv4, UDP, and socket code.
