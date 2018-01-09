//
// TestPacket.cpp
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

#include "TestPacket.h"

//
// Test Packet Library
//
TestPacketLibrary testPacketLibrary;


void TestPacketLibrary::buildTestPacketLibrary(void)
{
    TestPacket *testPkt;

    //
    // TEST_PKT_ID_IPV4_ECHO_REQ_TO_PEER1
    //
    // Mac peer1 32:26:0a:2e:cc:f1
    // Src IP : 103.30.10.1
    // Dst IP : 103.30.10.3
    //
    testPkt = new TestPacket();

    //testPkt->setDstMac("32:26:0a:2e:cc:f1");
    testPkt->setDstMac("33:22:0a:2e:ff:f1");
    testPkt->setSrcMac("32:26:0A:2E:aa:F1");
    testPkt->setEtherType(ETH_P_IP); // 0x0800
    testPkt->setEtherPayloadStr(
        "4500 0054 dacc 4000 4001 7d9c"
        "671e 0a01 671e 0a03"
        "0800 5cba 492b 3942 ee7c 5658 0000"
        "0000 0a30 0b00 0000 0000 1011 1213 1415"
        "1617 1819 1a1b 1c1d 1e1f 2021 2223 2425"
        "2627 2829 2a2b 2c2d 2e2f 3031 3233 3435"
        "3637");

    _testPacketLibrary[TEST_PKT_ID_IPV4_ECHO_REQ_TO_PEER1] = testPkt;

    //
    // TEST_PKT_ID_IPV4_ECHO_REQ_TO_PEER2
    //
    // Mac peer2 7a:44:b9:85:3e:10
    // Src IP : 103.30.80.1
    // Dst IP : 103.30.80.3
    testPkt = new TestPacket();

    testPkt->setDstMac("7A:44:B9:85:3E:10");
    testPkt->setSrcMac("32:26:0A:2E:FF:F3");
    testPkt->setEtherType(ETH_P_IP); // 0x0800
    testPkt->setEtherPayloadStr(
        "4500 0054 dacc 4000 4001 f19b"
        "671e 5001 671e 5003 "
        "0800 5cba 492b 3942 ee7c 5658 0000"
        "0000 0a30 0b00 0000 0000 1011 1213 1415"
        "1617 1819 1a1b 1c1d 1e1f 2021 2223 2425"
        "2627 2829 2a2b 2c2d 2e2f 3031 3233 3435"
        "3637");

    _testPacketLibrary[TEST_PKT_ID_IPV4_ECHO_REQ_TO_PEER2] = testPkt;


    // peer2 mac     : 32:26:0a:2e:cc:f2
    // ge-0/0/2 mac : 32:26:0a:2e:aa:f2
    //
    // TEST_PKT_ID_IPV4_ROUTER_ICMP_ECHO_TO_PEER3
    //
    // peer3 mac : 7a:44:b9:85:3e:10
    //
    // Src IP : 103.30.20.2
    // Dst IP : 103.30.30.3
    testPkt = new TestPacket();

    testPkt->setDstMac("32:26:0a:2e:aa:f2");
    testPkt->setSrcMac("32:26:0A:2E:BB:F2");
    testPkt->setEtherType(ETH_P_IP); // 0x0800
    testPkt->setEtherPayloadStr(
        "4500 0054 dacc 4000 4001 5f9c"
        "671e 1401 671e 1E03"
        "0800 5cba 492b 3942 ee7c 5658 0000"
        "0000 0a30 0b00 0000 0000 1011 1213 1415"
        "1617 1819 1a1b 1c1d 1e1f 2021 2223 2425"
        "2627 2829 2a2b 2c2d 2e2f 3031 3233 3435"
        "3637");

    _testPacketLibrary[TEST_PKT_ID_IPV4_ROUTER_ICMP_ECHO_TO_PEER3] = testPkt;


    //
    // TEST_PKT_ID_PUNT_ICMP_ECHO
    //
    // Src IP : 103.30.00.2
    // Dst IP : 103.30.00.1
    testPkt = new TestPacket();

    testPkt->setEtherType(ETH_P_IP); // 0x0800
    testPkt->setDstMac("33:22:0A:2E:AA:F1");
    testPkt->setEtherPayloadStr(
        "4500 0054 a038 4000 4001 cc31"
        "671e 0002 671e 0001 "
        "0800 bf46 3271 0001 06f8 8558"
        "0000 0000 bb23 0000 0000 0000 1011 1213"
        "1415 1617 1819 1a1b 1c1d 1e1f 2021 2223"
        "2425 2627 2829 2a2b 2c2d 2e2f 3031 3233"
        "3435 3637");

    _testPacketLibrary[TEST_PKT_ID_PUNT_ICMP_ECHO] = testPkt;


    //
    // TEST_PKT_ID_IPV4_VLAN
    //
    testPkt = new TestPacket();

    testPkt->setEtherType(ETH_P_8021Q); // 0x8100
    testPkt->setDstMac("33:22:0a:2e:ff:f1");

    testPkt->setEtherPayloadStr(
        "000b 0800 4500 0054 40b3 4000"
        "4001 b3b6 671e 3c02 671e 3c01 0800 fbe6"
        "3edd 8281 0b21 4158 0000 0000 2d6e 0200"
        "0000 0000 1011 1213 1415 1617 1819 1a1b"
        "1c1d 1e1f 2021 2223 2425 2627 2829 2a2b"
        "2c2d 2e2f 3031 3233 3435 3637");
#if 0

    testPkt->setEtherPayloadStr(
        "000c 8847 000c 81ff 3322 0a2e fff1 5ed8 f932"
        "bd85 8100 000b 0800 4500 0054 40b3 4000"
        "4001 b3b6 671e 3c02 671e 3c01 0800 fbe6"
        "3edd 8281 0b21 4158 0000 0000 2d6e 0200"
        "0000 0000 1011 1213 1415 1617 1819 1a1b"
        "1c1d 1e1f 2021 2223 2425 2627 2829 2a2b"
        "2c2d 2e2f 3031 3233 3435 3637");
#endif

    _testPacketLibrary[TEST_PKT_ID_IPV4_VLAN] = testPkt;


    //
    // TEST_PKT_ID_MPLS_L2VLAN
    //
    testPkt = new TestPacket();

    testPkt->setEtherType(ETH_P_MPLS_UC); // 0x8847
    testPkt->setDstMac("33:22:0a:2e:ff:f1");
    testPkt->setEtherPayloadStr(
        "000c 8847 000c 81ff 3322 0a2e fff1 5ed8 f932"
        "bd85 8100 000b 0800 4500 0054 40b3 4000"
        "4001 b3b6 671e 3c02 671e 3c01 0800 fbe6"
        "3edd 8281 0b21 4158 0000 0000 2d6e 0200"
        "0000 0000 1011 1213 1415 1617 1819 1a1b"
        "1c1d 1e1f 2021 2223 2425 2627 2829 2a2b"
        "2c2d 2e2f 3031 3233 3435 3637");

    _testPacketLibrary[TEST_PKT_ID_MPLS_L2VLAN] = testPkt;
}

void TestPacketLibrary::destroyTestPacketLibrary(void)
{
    TestPacket *testPkt;

    TestPacketLibraryMap::iterator itr = _testPacketLibrary.begin();

    while (itr != _testPacketLibrary.end()) {
        TestPacket *testPkt = itr->second;
        itr = _testPacketLibrary.erase(itr);
        ++itr;
    }

}
