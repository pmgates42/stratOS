import socket
import time

# Configuration for UDP
UDP_IP = "192.168.1.100"  # Target IP (Server IP)
UDP_PORT = 8000           # Target Port
PACKET_DATA = "Hello, UDP!"
BUFFER_SIZE = 1024        # Buffer size for receiving data

def send_and_receive():
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    while True:
        # Send the UDP packet
        sock.sendto(PACKET_DATA.encode(), (UDP_IP, UDP_PORT))
        print(f"Sent '{PACKET_DATA}' to {UDP_IP}:{UDP_PORT}")

        # Set a timeout for the receive call
        sock.settimeout(2.0)  # Timeout after 2 seconds

        try:
            # Receive a response from the server
            data, addr = sock.recvfrom(BUFFER_SIZE)
            print(f"Received message: {data.decode()} from {addr}")
        except socket.timeout:
            print("No response received within timeout period")

        # Wait for 1 second before sending the next packet
        time.sleep(1)

if __name__ == "__main__":
    send_and_receive()
