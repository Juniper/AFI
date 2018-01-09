//
// TapIf.cpp
//
// Advanced Forwarding Interface : AFI client examples
//
// Created by Sandesh Kumar Sodhi, January 2017
// Copyright (c) [2017] Juniper Networks, Inc. All rights reserved.
//
// All rights reserved.
//
// Notice and Disclaimer: This code is licensed to you under the Apache
// License 2.0 (the "License"). You may not use this code except in compliance
// with the License. This code is not an official Juniper product. You can
// obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Third-Party Code: This code may depend on other components under separate
// copyright notice and license terms. Your use of the source code for those
// components is subject to the terms and conditions of the respective license
// as noted in the Third-Party source code file.
//

#include <memory>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <thread>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

#include <string.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>

#include "TapIf.h"

//
// @fn
// tapAlloc
//
// @brief
// allocates or reconnects to a tun/tap device. The caller
// The caller must reserve enough space in *dev.
//
// @param[in]
//     dev Device name
// @param[in]
//     flags Flags
// @return  file descriptor
//

int
TapIf::tapAlloc(std::string ifName, int flags)
{

  struct ifreq ifr;
  int fd, err;
#define DEVICE_FILE_NAME_MAX_LEN 50
  char clonedev[DEVICE_FILE_NAME_MAX_LEN];

  snprintf(clonedev, DEVICE_FILE_NAME_MAX_LEN, "%s", "/dev/net/tun");

  if( (fd = open(clonedev , O_RDWR)) < 0 ) {
    perror("Opening /dev/net/tun");
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;
  strncpy(ifr.ifr_name, ifName.c_str(), IFNAMSIZ);

  if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    perror("ioctl(TUNSETIFF)");
    close(fd);
    return err;
  }

  return fd;
}

//
// @fn
// initialized
//
// @brief
// Check if the interface is initialized or not.
//
// @param[in] void
// @return  1 - Yes, 0 - No
//

int
TapIf::initialized(void)
{
    return _tapFd != -1;
}

//
// @fn
// ifRead
//
// @brief
// Reads interface
//
// @param[in] void
// @return  0 - Success, -1 - Error
//

int
TapIf::ifRead(void)
{
    uint16_t n;
    enum { max_length = 2000 };
    char data[max_length];
    int raw_socket;
    struct sockaddr_ll sa;

    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        perror("socket");
        exit(1);
    }

    std::stringstream outputIfName;
    outputIfName << "veth" << _portIndex;

    memset(&sa, 0, sizeof (struct sockaddr_ll));
    sa.sll_family = AF_PACKET;
    sa.sll_halen = ETH_ALEN;
    sa.sll_ifindex = if_nametoindex(outputIfName.str().c_str());

    while (1) {
        n = read(_tapFd, data, max_length);
        if (n < 0){
            perror("read");
            exit(1);
        }

        std::cout << "Read " << std::dec << n << " bytes from the interface " ;
        std::cout <<  "(fd " << std::dec<< _tapFd << " name "<< _ifName << ")"<< std::endl;
        pktTrace("Packet data", data, n);

        n = sendto(raw_socket, data, n, 0, (struct sockaddr *) &sa,
                   sizeof (struct sockaddr_ll));
        if (n < 0){
            perror("sendto");
            exit(1);
        }
    }

    return 0;
}

//
// @fn
// ifWrite
//
// @brief
// Writes data to interface
//
// @param[in]
//     size Number of bytes to write
// @param[in]
//     data Buffer with actual data
// @return  0 - Success, -1 - Error
//

int
TapIf::ifWrite(int size, uint8_t *data)
{
    int n = write(_tapFd, data, size);
    if (n < 0) {
        perror("write");
        return -1;
    }

    std::cout << "Wrote " << std::dec << n << " bytes to the interface " ;
    std::cout <<  "(fd " << std::dec<< _tapFd << " name "<< _ifName << ")"<< std::endl;
    pktTrace("Packet data", (char *)data, n);

    return 0;
}
