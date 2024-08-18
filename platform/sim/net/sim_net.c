#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>

/**
 * 
 * <net/route.h> does not appear to be standard on
 * MacOS Monterey?
 * 
 */
#ifndef _NET_ROUTE_H_
struct rtentry {
    struct sockaddr rt_gateway;
    struct sockaddr rt_dst;
    struct sockaddr rt_genmask;
    short rt_flags;
};

#define RTF_UP 0x1          /* Route is up */
#define RTF_GATEWAY 0x2     /* Route is a gateway */
#define SIOCADDRT 0x890B    /* Add route */
#endif

#define INTERFACE_NAME "lo0"        /* Network interface */
#define STATIC_IP "192.168.1.100"   /* This devices static IP */
#define NETMASK "255.255.255.0"     /* Net mask */
#define GATEWAY "192.168.1.1"       /* Gateway */

#define PORT 8000                 /* Port number for listening */
#define BUFFER_SIZE 1024          /* Buffer size for packet data */

static int sock = -1;             /* Socket descriptor */

/**********************************************************
 * 
 * set_static_ip()
 * 
 * DESCRIPTION:
 *      Set the static IP address for this "device".
 * 
 */

void set_static_ip()
{
    struct ifreq ifr;
    struct sockaddr_in *sin;

    /* create UDP sock */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    /* Configure the interface name */
    strncpy(ifr.ifr_name, INTERFACE_NAME, IFNAMSIZ);

    /* Set static IP */
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    sin->sin_family = AF_INET;
    inet_pton(AF_INET, STATIC_IP, &sin->sin_addr);
    if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
        perror("Failed to set IP address");
        close(sock);
        return;
    }

    /* Set the netmask */
    inet_pton(AF_INET, NETMASK, &sin->sin_addr);
    if (ioctl(sock, SIOCSIFNETMASK, &ifr) < 0) {
        perror("Failed to set netmask");
        close(sock);
        return;
    }

    /* Bring up the interface (it's alive!) */
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        perror("Failed to get interface flags");
        close(sock);
        return;
    }
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        perror("Failed to bring up the interface");
        close(sock);
        return;
    }

    //TODO PMG this always seems to fail
    // /* set gateway */
    // struct rtentry route;
    // memset(&route, 0, sizeof(route));
    // sin = (struct sockaddr_in *)&route.rt_gateway;
    // sin->sin_family = AF_INET;
    // inet_pton(AF_INET, GATEWAY, &sin->sin_addr);
    // sin = (struct sockaddr_in *)&route.rt_dst;
    // sin->sin_family = AF_INET;
    // sin->sin_addr.s_addr = 0;
    // sin = (struct sockaddr_in *)&route.rt_genmask;
    // sin->sin_family = AF_INET;
    // sin->sin_addr.s_addr = 0;
    // route.rt_flags = RTF_UP | RTF_GATEWAY;
    // if (ioctl(sock, SIOCADDRT, &route) < 0) {
    //     perror("Failed to set gateway");
    //     close(sock);
    //     return;
    // }

    close(sock);
}

void net_init()
{
    struct sockaddr_in addr;

    /* Create a socket */
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    /* Bind the socket to the static IP and specified port */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, STATIC_IP, &addr.sin_addr);
    addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(sock);
        sock = -1;
        return;
    }
}


/**********************************************************
 * 
 *  get_packet()
 * 
 * 
 *  DESCRIPTION:
 *      Get packet data
 *
 */

int get_packet(char *buffer, size_t buffer_size, int timeout_us) {
    struct timeval tv;

    if (sock < 0) {
        perror("Socket is not initialized");
        return -1;
    }

    tv.tv_sec = 2;  // 0 seconds
    tv.tv_usec = timeout_us;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Failed to set socket timeout");
        return -1;
    }

    int bytes_received = recvfrom(sock, buffer, buffer_size, 0, NULL, NULL);
    if (bytes_received < 0) {
        perror("Packet receive failed");
    }

    return bytes_received;
}