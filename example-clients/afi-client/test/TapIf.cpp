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
TapIf::tapAlloc(char *dev, int flags)
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

  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    perror("ioctl(TUNSETIFF)");
    close(fd);
    return err;
  }

  strcpy(dev, ifr.ifr_name);

  return fd;
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
    int ret;
    int maxfd;
    uint16_t nread;
    fd_set rd_set;
    enum { max_length = 2000 };
    char data[max_length];

    FD_ZERO(&rd_set);
    FD_SET(_tapFd, &rd_set);
    maxfd = _tapFd;

    struct timeval tv = {1, 0};   // 1 seconds!
    std::cout << "Calling select for fd " << std::dec<< _tapFd ;
    std::cout << "(interface "<< _ifName << ")"<< std::endl;
    ret = select(maxfd + 1, &rd_set, NULL, NULL, &tv);

    while (ret < 0 && errno == EINTR){
        std::cout << " select returned  " << ret << std::endl;
        ret = select(maxfd + 1, &rd_set, NULL, NULL, &tv);
        sleep(1);
    }

    if (FD_ISSET(_tapFd, &rd_set)) {
        /* data from tun/tap: just read it and write it to the network */

        nread = read(_tapFd, data, max_length);
        if (nread < 0){
            perror("Reading data");
            exit(1);
        }

        std::cout << "Read " << std::dec << nread << " bytes from the interface " ;
        std::cout <<  "(fd " << std::dec<< _tapFd << "name "<< _ifName << ")"<< std::endl;
        pktTrace("Packet data", data, nread);

    } else {
        return -1;
    }

    return 0;
}
