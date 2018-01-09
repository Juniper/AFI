//
// TestPacket.h
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

#ifndef __TestPacket__
#define __TestPacket__

#include <netinet/ether.h>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include "../Utils.h"

#define MAC_ADDR_LEN            6
#define ETHER_PAYLOAD_BUF_SIZE  5000

class TestPacket {
public:

    //
    //
    //
    int setSrcMac(const std::string &mac) {
        int i;
        std::vector<std::string> mac_sub_strings;
        boost::split(mac_sub_strings, mac, boost::is_any_of(":"));

        for (i = 0; i < MAC_ADDR_LEN; i++) {
            _srcMac[i] = std::strtoull(mac_sub_strings.at(i).c_str(), NULL, 16);
        }
    }

    int *getSrcMac(void) {
        return _srcMac;
    }

    //
    //
    //
    int setDstMac(const std::string &mac) {
        int i;
        std::vector<std::string> mac_sub_strings;
        boost::split(mac_sub_strings, mac, boost::is_any_of(":"));

        for (i = 0; i < MAC_ADDR_LEN; i++) {
            _dstMac[i] = std::strtoull(mac_sub_strings.at(i).c_str(), NULL, 16);
        }
    }

    int *getDstMac(void) {
        return _dstMac;
    }

    //
    //
    //
    int getEtherType(void) {
        return _etherType;
    }

    void setEtherType(int  etherType) {
        _etherType = etherType;
    }

    //
    //
    //
    int setEtherPayloadStr(const std::string &etherPayloadStr) {
        strncpy(_etherPayload, etherPayloadStr.c_str(), ETHER_PAYLOAD_BUF_SIZE);
        _etherPayload[ETHER_PAYLOAD_BUF_SIZE - 1] = '\0';
    }

    char *getEtherPayloadStr(void) {
          return _etherPayload;
    }


    int getEtherPacket(char *pktData, int pktDataBuffLen) {
#define ETH_HEADER_LEN    14
        int i = 0;

        if (pktDataBuffLen < ETH_HEADER_LEN) {
            return -1;
        }

        for (i = 0; i < MAC_ADDR_LEN; i++) {
            pktData[i] = _dstMac[i];
        }

        for (i = 0; i < MAC_ADDR_LEN; i++) {
            pktData[MAC_ADDR_LEN + i] = _srcMac[i];
        }

        pktData[ETH_HEADER_LEN - 2] = ((_etherType >> 8) & 0xFF);
        pktData[ETH_HEADER_LEN - 1] = ((_etherType) & 0xFF);

        int pktDataLen = convertHexPktStrToPkt(_etherPayload,
                                               &pktData[ETH_HEADER_LEN],
                                               pktDataBuffLen - ETH_HEADER_LEN);
        return (ETH_HEADER_LEN + pktDataLen);
    }

private:
    int  _srcMac[MAC_ADDR_LEN];
    int  _dstMac[MAC_ADDR_LEN];

    //
    // ETH_P_IP =  0x0800
    //
    //
    int  _etherType;
    char _etherPayload[ETHER_PAYLOAD_BUF_SIZE];
};


class TestPacketLibrary {
public:
    //
    // Test Packed Id.
    //
    typedef enum {
        TestPackerIdInvalid = 0,
        TEST_PKT_ID_IPV4_ECHO_REQ_TO_PEER1,
        TEST_PKT_ID_IPV4_ECHO_REQ_TO_PEER2,
        TEST_PKT_ID_IPV4_ROUTER_ICMP_ECHO_TO_PEER3,
        TEST_PKT_ID_PUNT_ICMP_ECHO,
        TEST_PKT_ID_IPV4_VLAN,
        TEST_PKT_ID_MPLS_L2VLAN,
        TestPackerIdMax,
    } TestPacketId;



    TestPacketLibrary() {
        buildTestPacketLibrary();
    }

    ~TestPacketLibrary() {
        destroyTestPacketLibrary();
    }

    //
    //
    //
    TestPacket* getTestPacket(TestPacketId testPktNum) {
        return _testPacketLibrary[testPktNum];

    }

    void buildTestPacketLibrary(void);
    void destroyTestPacketLibrary(void);

private:
    typedef std::map<TestPacketId, TestPacket*>  TestPacketLibraryMap;

    TestPacketLibraryMap _testPacketLibrary;
};


//
// Test Packet Library
//
extern TestPacketLibrary testPacketLibrary;

#endif // __TestPacket__
