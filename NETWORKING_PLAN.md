# StratOS Networking Plan

This plan develops networking from simulator-tested, hardware-independent layers toward a real ENC28J60 driver. Each layer should be complete and tested before the next layer depends on it.

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
ENC28J60 or simulator. The network driver knows only the generic receive,
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

- [ ] Implement packet initialization.
- [ ] Reject null packet, null buffer, and zero-capacity inputs.
- [ ] Implement descriptor-to-packet conversion.
- [ ] Reject incomplete descriptors.
- [ ] Reject descriptor lengths larger than descriptor capacity.
- [ ] Set `offset` to zero for a newly received frame.
- [ ] Set `length` to the received byte count.
- [ ] Add tests for zero-length frames and boundary-sized frames.
- [ ] Add tests proving `offset + length <= capacity`.
- [ ] Decide whether packet initialization clears the buffer or only initializes metadata.

## Stage 2: Implement Ethernet Frames

Target: `net_ethernet` and `net_ethernet_test`.

- [ ] Decode destination MAC address.
- [ ] Decode source MAC address.
- [ ] Decode EtherType in network byte order.
- [ ] Reject packets shorter than the Ethernet header.
- [ ] Reject packets whose active range exceeds `capacity`.
- [ ] Remove or advance past the Ethernet header without copying payload data.
- [ ] Preserve the packet payload as the active `data + offset` range.
- [ ] Add tests for IPv4 EtherType.
- [ ] Add tests for unsupported EtherTypes.
- [ ] Add tests with nonzero initial packet offsets.
- [ ] Add tests for malformed packet lengths.

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

## Stage 7: Add the ENC28J60 Hardware Driver Last

The ENC28J60 driver should implement the existing hardware-driver interface. It
should not change the network-driver queue, Ethernet, IPv4, UDP, or socket
semantics.

- [ ] Obtain and review the ENC28J60 datasheet and module pinout.
- [ ] Verify module voltage and logic levels before connecting it.
- [ ] Confirm SPI pin wiring, chip select, interrupt, and reset wiring.
- [ ] Implement the minimum StratOS SPI transaction needed for register reads.
- [ ] Read and verify the ENC28J60 revision or identifier register.
- [ ] Implement hardware reset and oscillator/link initialization.
- [ ] Configure receive buffer boundaries.
- [ ] Read one received frame length from the ENC28J60.
- [ ] Copy one received frame into a StratOS-owned packet buffer.
- [ ] Populate `data`, `capacity`, `offset`, and `length` correctly.
- [ ] Implement receive-buffer release and pointer advancement.
- [ ] Implement one-frame transmission.
- [ ] Add interrupt or polling-based receive detection.
- [ ] Bind one network-driver instance to the ENC28J60 hardware-driver instance.
- [ ] Connect the driver to the existing Ethernet layer without changing its public API.
- [ ] Test a real Ethernet frame between StratOS and a host computer.
- [ ] Test real UDP traffic after the simulator path is passing.

## Completion Criteria

- [ ] Every protocol layer has tests that use only its public interface.
- [ ] Malformed input is rejected without out-of-bounds reads or invalid packet views.
- [ ] Packet ownership is documented and enforced at every queue boundary.
- [ ] Simulator loopback can send and receive multiple UDP datagrams.
- [ ] The ENC28J60 driver can exchange raw Ethernet frames without protocol logic leaking into the hardware layer.
- [ ] Hardware-specific code is isolated from Ethernet, IPv4, UDP, and socket code.
