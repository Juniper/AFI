//
// TestUtils.cpp
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

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include "TestPacket.h"
#include "../Utils.h"
#include "TestUtils.h"

#define SEND_BUF_SIZE           1500
#define ETHER_PAYLOAD_BUF_SIZE  5000

#define SEND_BUF_STR_SIZE       5000


int SendRawEth (const std::string &ifNameStr,
                TestPacketLibrary::TestPacketId tcPktNum)
{
    int         i;
    int         ether_type;
    char       *ether_payload = NULL;
    int        *dst_mac;

    int         sockfd;
    struct      ifreq if_idx;
    struct      ifreq if_mac;
    int         tx_len = 0;
    char        sendbuf[SEND_BUF_SIZE];
    char        sendbuf_str[SEND_BUF_STR_SIZE];
    struct      ether_header *eh = (struct ether_header *) sendbuf;
    struct      iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
    struct      sockaddr_ll socket_address;
    char        ifName[IFNAMSIZ];
    int         byte_count;

    TestPacket* testPkt = testPacketLibrary.getTestPacket(tcPktNum);

    ether_type    = testPkt->getEtherType();
    dst_mac       = testPkt->getDstMac();
    ether_payload = testPkt->getEtherPayloadStr();

    strncpy(ifName, ifNameStr.c_str(), IFNAMSIZ);
    ifName[IFNAMSIZ - 1] = '\0';

    // RAW socket
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("socket");
    }

    // Get interface index */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ-1);

    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0) {
        perror("SIOCGIFINDEX");
    }

    // Get the MAC address of the interface to send on
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0) {
        perror("SIOCGIFHWADDR");
    }

    //
    // Build Ethernet header
    //
    memset(sendbuf, 0, SEND_BUF_SIZE);

    // Destination MAC
    for (i = 0; i < MAC_ADDR_LEN; i++) {
        eh->ether_dhost[i] = dst_mac[i];
    }

    // Source MAC
    for (i = 0; i < MAC_ADDR_LEN; i++) {
        eh->ether_shost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];
    }

    // Ethertype field
    eh->ether_type = htons(ether_type);
    tx_len        += sizeof(struct ether_header);

    getRidOfSpacesFromString(ether_payload);

    byte_count = convertHexStringToBinary(ether_payload,
                                          &sendbuf[tx_len],
                                          (SEND_BUF_SIZE - tx_len));

    tx_len += byte_count;

    // Index of the network device
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    // Address length
    socket_address.sll_halen = ETH_ALEN;

    // Destination MAC
    for (i = 0; i < MAC_ADDR_LEN; i++) {
        socket_address.sll_addr[i] = dst_mac[i];
    }

    std::cout << "Sending packet on interface " << ifName << "..." << std::endl;
    pktTrace("Raw socket send", sendbuf, tx_len);

    // Send packet
    if (sendto(sockfd, sendbuf, tx_len, 0,
        (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0) {
        std::cout << "Error: Send failed!" << std::endl;
        return -1;
    } else {
        std::cout << "Send success!!!" << std::endl;
        return 0;
    }
}

