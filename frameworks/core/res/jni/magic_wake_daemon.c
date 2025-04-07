/**
 * Magic Wake Packet Daemon
 *
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * Listens on UDP port 9 for magic packets to wake up the system
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/netdevice.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include <ifaddrs.h>
#include <linux/capability.h>
#include <sys/capability.h>
#include <time.h>
#include <log/log.h>
#include <pthread.h>

#define TAG "magic_wake_daemon"

#define WAKE_ON_LAN_PORT 9
#define MAC_SIZE 6
#define MAGIC_WAKE_PROPERTY "vendor.sys.magic_wake.triggered"
#define MAGIC_WAKE_CONTROL_PROPERTY "vendor.sys.magic_wake.control"

int get_mac_address(unsigned char *mac);
void* listen_thread_func(void *arg);
void check_control_property(void);
void signal_handler(int sig);

// Global variables indicating whether listening is enabled
static volatile int g_is_listening = 0;
static volatile int g_socket_fd = -1;

// Get MAC address of current active network interface
int get_mac_address(unsigned char *mac) {
    FILE *fp;
    char line[100];
    int values[MAC_SIZE];
    fp = fopen("/sys/class/net/wlan0/address", "r");
    if (fp != NULL) {
        if (fgets(line, sizeof(line), fp) != NULL) {
            if (sscanf(line, "%02x:%02x:%02x:%02x:%02x:%02x",
                        &values[0], &values[1], &values[2],
                        &values[3], &values[4], &values[5]) == MAC_SIZE) {

                for (int i = 0; i < MAC_SIZE; i++) {
                    mac[i] = (unsigned char)values[i];
                }

                ALOGI("%s: Using active wlan0 MAC: %02X:%02X:%02X:%02X:%02X:%02X", TAG,
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                fclose(fp);
                return 0;
            }
        }
        fclose(fp);
    }
    return -1;
}

// Listen thread function
void* listen_thread_func(void *arg) {
    (void)arg;

    int sockfd = -1;
    struct sockaddr_in server_addr;
    unsigned char local_mac[MAC_SIZE];

    ALOGI("%s: Starting to listen for magic packets", TAG);

    // Get local MAC address
    if (get_mac_address(local_mac) != 0) {
        ALOGE("%s: Failed to get MAC address", TAG);
        return NULL;
    }

    ALOGI("%s: Local MAC: %02X:%02X:%02X:%02X:%02X:%02X", TAG,
           local_mac[0], local_mac[1], local_mac[2],
           local_mac[3], local_mac[4], local_mac[5]);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ALOGE("%s: Socket creation failed: %s", TAG, strerror(errno));
        return NULL;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        ALOGE("%s: setsockopt failed: %s", TAG, strerror(errno));
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(WAKE_ON_LAN_PORT);

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ALOGE("%s: Bind failed: %s (errno=%d)", TAG, strerror(errno), errno);
        close(sockfd);
        return NULL;
    }

    ALOGI("%s: Listening on port %d for magic packets", TAG, WAKE_ON_LAN_PORT);
    g_socket_fd = sockfd;
    // Set g_is_listening flag for signal handling
    g_is_listening = 1;

    // Receive and process Wake-on-LAN magic packets
    while (g_is_listening) {
        unsigned char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        fd_set readfds;
        struct timeval tv;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Set timeout to allow periodic checking of g_is_listening flag
        tv.tv_sec = 1;  // 1 second timeout
        tv.tv_usec = 0;

        int ret = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        if (ret == 0) {
            // Timeout, continue to next loop
            continue;
        } else if (ret < 0) {
            if (errno == EINTR) {
                // Interrupted by signal, check g_is_listening
                ALOGI("%s: select interrupted by signal", TAG);
                continue;
            }
            ALOGE("%s: Select error: %s", TAG, strerror(errno));
            continue;
        }

        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                        (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            ALOGE("%s: Recvfrom failed: %s", TAG, strerror(errno));
            continue;
        }

        ALOGI("%s: Received UDP packet of size %d from %s:%d", TAG,
               n, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Check magic packet format
        if (n >= 102) {  // Magic packet minimum length is 6+96=102 bytes
            // Verify first 6 bytes are 0xFF
            int i;
            for (i = 0; i < 6; i++) {
                if (buffer[i] != 0xFF) {
                    break;
                }
            }

            if (i == 6) {
                ALOGI("%s: Magic packet header validated", TAG);

                // Check for 16 repetitions of the MAC address
                int match = 1;
                for (i = 6; i < 102; i += 6) {
                    if (memcmp(&buffer[i], local_mac, MAC_SIZE) != 0) {
                        match = 0;
                        break;
                    }
                }

                if (match) {
                    ALOGI("%s: Received VALID Magic Packet for this device!", TAG);
                    // Set property to notify Java layer
                    property_set(MAGIC_WAKE_PROPERTY, "1");
                    ALOGI("%s: Set trigger property", TAG);

                    // Stop listening after receiving a valid magic packet
                    g_is_listening = 0;
                    ALOGI("%s: Valid magic packet received, stopping listen", TAG);
                    break;  // Exit listen loop
                } else {
                    ALOGW("%s: Magic packet MAC mismatch", TAG);
                }
            } else {
                ALOGW("%s: Not a magic packet (header mismatch)", TAG);
            }
        } else {
            ALOGW("%s: Packet too small to be a magic packet (%d bytes)", TAG, n);
        }
    }

    ALOGI("%s: Listen function exiting", TAG);
    if (sockfd >= 0) {
        close(sockfd);
        g_socket_fd = -1;  // Ensure global variable is reset
    }
    g_is_listening = 0;

    return NULL;
}

// Check for control property changes
void check_control_property() {
    int last_command = 0;
    pthread_t listen_thread;

    // Start monitoring loop
    ALOGI("%s: Starting control property monitoring loop", TAG);

    // Initialize by reading property value
    char buffer[PROPERTY_VALUE_MAX] = {0};
    property_get(MAGIC_WAKE_CONTROL_PROPERTY, buffer, "0");
    last_command = atoi(buffer);
    ALOGI("%s: Initial control property value: %d", TAG, last_command);

    // If initial value is 1, start listening in a new thread
    if (last_command == 1) {
        ALOGI("%s: Starting listen thread based on initial property", TAG);
        if (pthread_create(&listen_thread, NULL, listen_thread_func, NULL) != 0) {
            ALOGE("%s: Failed to create listen thread", TAG);
        } else {
            pthread_detach(listen_thread);
            ALOGI("%s: Listen thread created and detached", TAG);
        }
    }

    while (1) {
        // Read control property
        memset(buffer, 0, sizeof(buffer));
        property_get(MAGIC_WAKE_CONTROL_PROPERTY, buffer, "0");

        int command = atoi(buffer);

        // If command changes to 1 (start listening)
        if (command == 1 && last_command == 0) {
            ALOGI("%s: Control command changed: %d -> %d", TAG, last_command, command);
            last_command = command;

            // Ensure previous thread has stopped
            g_is_listening = 0;
            if (g_socket_fd >= 0) {
                close(g_socket_fd);
                g_socket_fd = -1;
                ALOGI("%s: Closed any existing sockets", TAG);
            }

            // Start listening in a new thread
            ALOGI("%s: Starting new listen thread", TAG);
            if (pthread_create(&listen_thread, NULL, listen_thread_func, NULL) != 0) {
                ALOGE("%s: Failed to create listen thread", TAG);
            } else {
                pthread_detach(listen_thread);
                ALOGI("%s: Listen thread created and detached", TAG);
            }
        }
        // If command is 0 (stop listening)
        else if (command == 0) {
            // Only perform stop operation if currently listening
            if (g_is_listening) {
                ALOGI("%s: Command is 0, stopping listen thread", TAG);
                // Set flag to stop listening
                g_is_listening = 0;
                // Close socket if open
                if (g_socket_fd >= 0) {
                    close(g_socket_fd);
                    g_socket_fd = -1;
                    ALOGI("%s: Socket closed", TAG);
                }
            }
            last_command = command;
        }
        // Log command changes even if no action is required
        else if (command != last_command) {
            ALOGI("%s: Control command changed: %d -> %d (no action required)", TAG, last_command, command);
            last_command = command;
        }
        sleep(1);  // Check every 1 seconds
    }
}

// Signal handler function
void signal_handler(int sig) {
    ALOGI("%s: Received signal %d, exiting...", TAG, sig);
    // Stop listening
    g_is_listening = 0;
    // Close socket
    if (g_socket_fd >= 0) {
        close(g_socket_fd);
        g_socket_fd = -1;
    }
    // Only reset wake property, not control property
    property_set(MAGIC_WAKE_PROPERTY, "0");

    exit(0);
}

// Exit handler function
void exit_handler(void) {
    ALOGI("%s: Process exiting, cleaning up", TAG);
    // Stop listening
    g_is_listening = 0;
    // Close socket
    if (g_socket_fd >= 0) {
        close(g_socket_fd);
        g_socket_fd = -1;
    }
    // Only reset wake property, not control property
    property_set(MAGIC_WAKE_PROPERTY, "0");
}

int main(void) {
    ALOGI("%s: Starting up (pid=%d)", TAG, getpid());
    // Set signal handling
    signal(SIGTERM, signal_handler);  // Termination signal
    signal(SIGINT, signal_handler);   // Interrupt signal
    signal(SIGHUP, signal_handler);   // Hangup signal, usually sent when session is closed
    // Initialize property value
    property_set(MAGIC_WAKE_PROPERTY, "0");
    // Check network interface
    unsigned char mac[MAC_SIZE];
    if (get_mac_address(mac) == 0) {
        ALOGI("%s: Successfully got MAC address: %02X:%02X:%02X:%02X:%02X:%02X", TAG,
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        ALOGE("%s: Failed to get MAC address", TAG);
    }

    // Register exit handler function
    atexit(exit_handler);
    ALOGI("%s: Starting control property monitoring loop", TAG);
    // Check for control property changes
    check_control_property();

    ALOGI("%s: Exiting (this should not happen)", TAG);
    return 0;
}