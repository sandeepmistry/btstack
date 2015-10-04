/*
 *  hci_transport_kernel.c
 *
 *  HCI Transport API implementation for the Linux kernel
 *
 *  Created by Sandeep Mistry on 4/10/15.
 */

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "btstack-config.h"

#include "debug.h"
#include "hci.h"
#include "hci_transport.h"

#define BTPROTO_HCI 1

// #define HCI_CHANNEL_RAW     0
#define HCI_CHANNEL_USER    1

struct sockaddr_hci {
    sa_family_t     hci_family;
    unsigned short  hci_dev;
    unsigned short  hci_channel;
};

static int linux_kernel_sock_process(struct data_source *ds);

// single instance
static hci_transport_t * hci_transport_linux_kernel = NULL;

static void (*packet_handler)(uint8_t packet_type, uint8_t *packet, uint16_t size) = NULL;

int sock;
struct data_source ds;

static void linux_kernel_register_packet_handler(void (*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size)){
    log_info("registering packet handler");
    packet_handler = handler;
}

static const char * linux_kernel_get_transport_name(void){
    return "LINUX-KERNEL";
}

static int linux_kernel_open(void *transport_config) {
    struct sockaddr_hci a;

    sock = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);

    if (sock < 0) {
        return -1;
    }

    memset(&a, 0, sizeof(a));
    a.hci_family = AF_BLUETOOTH;
    a.hci_dev = 0;
    a.hci_channel = HCI_CHANNEL_USER;

    if (bind(sock, (struct sockaddr *) &a, sizeof(a)) < 0) {
        close(sock);
        return -1;
    }

    ds.fd = sock;
    ds.process = linux_kernel_sock_process;

    run_loop_add_data_source(&ds);

    return 0;
}

static int linux_kernel_close(void *transport_config) {
    run_loop_remove_data_source(&ds);

    close(sock);

    return 0;
}

static int linux_kernel_send_packet(uint8_t packet_type, uint8_t * packet, int size){
    uint8_t packet_buffer[256 + 1];

    packet_buffer[0] = packet_type;
    memcpy(&packet_buffer[1], packet, size);

    if (write(sock, packet_buffer, size + 1) < 0) {
        return -1;
    }

    return 0;
}

static int linux_kernel_sock_process(struct data_source *ds) {
    uint8_t packet_buffer[256];
    int size;

    size = read(sock, packet_buffer, sizeof(packet_buffer));

    if (size < 0) {
        return -1;
    }

    if (packet_handler) {
        packet_handler(packet_buffer[0], &packet_buffer[1], size - 1);
    }

    return 0;
}

// get linux kernel singleton
hci_transport_t * hci_transport_linux_kernel_instance() {
    if (!hci_transport_linux_kernel) {
        hci_transport_linux_kernel = (hci_transport_t*) malloc( sizeof(hci_transport_t));
        hci_transport_linux_kernel->open                          = linux_kernel_open;
        hci_transport_linux_kernel->close                         = linux_kernel_close;
        hci_transport_linux_kernel->send_packet                   = linux_kernel_send_packet;
        hci_transport_linux_kernel->register_packet_handler       = linux_kernel_register_packet_handler;
        hci_transport_linux_kernel->get_transport_name            = linux_kernel_get_transport_name;
        hci_transport_linux_kernel->set_baudrate                  = NULL;
        hci_transport_linux_kernel->can_send_packet_now           = NULL;
    }
    return hci_transport_linux_kernel;
}
