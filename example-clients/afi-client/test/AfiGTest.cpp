//
// AfiGTest.cpp - Afi APIs GTest
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

#include <boost/thread.hpp>
#include <limits.h>
#include "gtest/gtest.h"
#include "TestPacket.h"
#include "TestUtils.h"
#include "../AfiClient.h"
#include <iostream>
#include <iomanip>
#include <ctime>

//
// Test setup
//
//  +-------------------------------------------------------------------+
//  | mx86 server                                                       |
//  |   +-----------------------------------------------------------+   |
//  |   | Docker container                                          |   |
//  |   |                                                           |   |
//  |   |   +---------------------+                                 |   |
//  |   |   |        VCP          |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   +---------------------+                                 |   |
//  |   |                                                           |   |
//  |   |   +---------------------+                                 |   |
//  |   |   |        VFP          |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   |    {afi-server}<----grpc/tcp----->{afi-client}        |   |
//  |   |   |                     |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   |      {vmxt}         |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   |      {riot}         |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   |                     |                                 |   |
//  |   |   |   p0    ...     p7  |                                 |   |
//  |   |   |ge-0/0/0 ... ge-0/0/7|                                 |   |
//  |   |   +---o-o-o-o-o-o-o-o---+                                 |   |
//  |   |       |             |                                     |   |
//  |   |       |     ...     |                                     |   |
//  |   |       |             |                                     |   |
//  |   |       |             |        ,-----.                      |   |
//  |   |       |             |        |     | veth7                |   |
//  |   |       |             `--------o     o----                  |   |
//  |   |       |                      |_____|                      |   |
//  |   |       |                     vmx_link7                     |   |
//  |   |       |                        .                          |   |
//  |   |       |                        .                          |   |
//  |   |       |                        .                          |   |
//  |   |       |                      ,-----.                      |   |
//  |   |       |                      |     | veth0                |   |
//  |   |       `----------------------o     o----                  |   |
//  |   |                              |_____|                      |   |
//  |   |                             vmx_link0                     |   |
//  |   |                                                           |   |
//  |   +-----------------------------------------------------------+   |
//  |                                                                   |
//  +-------------------------------------------------------------------+
//

//
// Sandbox CLI configuration
// =========================
//
// root@vcp-vm# show forwarding-options forwarding-sandbox green
// size medium;
// port p0 {
//     interface ge-0/0/0;
// }
// port p1 {
//     interface ge-0/0/1;
// }
// port p2 {
//     interface ge-0/0/2;
// }
// port p3 {
//     interface ge-0/0/3;
// }
// port p4 {
//     interface ge-0/0/4;
// }
// port p5 {
//     interface ge-0/0/5;
// }
// port p6 {
//     interface ge-0/0/6;
// }
// port p7 {
//     interface ge-0/0/7;
// }
//
// [edit]
// root@vcp-vm#
//
// root@de5cf35cd169:~# for i in $(seq 0 7) ; do ip netns exec vrouter$i ifconfig veth | grep HW ; done
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f0
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f1
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f2
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f3
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f4
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f5
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f6
// veth      Link encap:Ethernet  HWaddr 32:26:0a:2e:cc:f7
// root@de5cf35cd169:~#
//
// root@de5cf35cd169:~# ifconfig | grep vmx_link | grep HW
// vmx_link0 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f0
// vmx_link1 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f1
// vmx_link2 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f2
// vmx_link3 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f3
// vmx_link4 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f4
// vmx_link5 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f5
// vmx_link6 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f6
// vmx_link7 Link encap:Ethernet  HWaddr 32:26:0a:2e:bb:f7
// root@de5cf35cd169:~#
//


const std::string afiServerAddr   = "128.0.0.16:50051"; // grpc/tcp
const std::string afiHospathAddr  = "128.0.0.16:9002";  // udp
const std::string sbName          = "green";
const std::string sbIPv4RttName   = "rtt0";

#define SB_INDEX               1

#define SB_P0_PORT_INDEX       0
#define SB_P1_PORT_INDEX       1
#define SB_P2_PORT_INDEX       2
#define SB_P3_PORT_INDEX       3
#define SB_P4_PORT_INDEX       4
#define SB_P5_PORT_INDEX       5
#define SB_P6_PORT_INDEX       6
#define SB_P7_PORT_INDEX       7

#define SB_NUM_CFG_PORTS       8
#define SB_PUNT_PORT_INDEX     SB_NUM_CFG_PORTS

//
//                  |
//       ,-----.    |
//  vethX|     |    |
//   ----o     o----o----------
//       |_____|    |pX / ge-0/0/X
//      vmx_linkX   |
//                  |
//

const std::string GE_0_0_0_MAC_STR = "32:26:0a:2e:aa:f0";
const std::string GE_0_0_1_MAC_STR = "32:26:0a:2e:aa:f1";
const std::string GE_0_0_2_MAC_STR = "32:26:0a:2e:aa:f2";
const std::string GE_0_0_3_MAC_STR = "32:26:0a:2e:aa:f3";
const std::string GE_0_0_4_MAC_STR = "32:26:0a:2e:aa:f4";
const std::string GE_0_0_5_MAC_STR = "32:26:0a:2e:aa:f5";
const std::string GE_0_0_6_MAC_STR = "32:26:0a:2e:aa:f6";
const std::string GE_0_0_7_MAC_STR = "32:26:0a:2e:aa:f7";

const std::string GE_0_0_0_VMX_IF_NAME = "ge-0.0.0-vmx1";
const std::string GE_0_0_1_VMX_IF_NAME = "ge-0.0.1-vmx1";
const std::string GE_0_0_2_VMX_IF_NAME = "ge-0.0.2-vmx1";
const std::string GE_0_0_3_VMX_IF_NAME = "ge-0.0.3-vmx1";
const std::string GE_0_0_4_VMX_IF_NAME = "ge-0.0.4-vmx1";
const std::string GE_0_0_5_VMX_IF_NAME = "ge-0.0.5-vmx1";
const std::string GE_0_0_6_VMX_IF_NAME = "ge-0.0.6-vmx1";
const std::string GE_0_0_7_VMX_IF_NAME = "ge-0.0.7-vmx1";

const std::string GE_0_0_0_IP_ADDR_STR = "103.30.00.1";
const std::string GE_0_0_1_IP_ADDR_STR = "103.30.10.1";
const std::string GE_0_0_2_IP_ADDR_STR = "103.30.20.1";
const std::string GE_0_0_3_IP_ADDR_STR = "103.30.30.1";
const std::string GE_0_0_4_IP_ADDR_STR = "103.30.40.1";
const std::string GE_0_0_5_IP_ADDR_STR = "103.30.50.1";
const std::string GE_0_0_6_IP_ADDR_STR = "103.30.60.1";
const std::string GE_0_0_7_IP_ADDR_STR = "103.30.70.1";

const std::string VMX_LINK0_NAME_STR = "vmx_link0";
const std::string VMX_LINK1_NAME_STR = "vmx_link1";
const std::string VMX_LINK2_NAME_STR = "vmx_link2";
const std::string VMX_LINK3_NAME_STR = "vmx_link3";
const std::string VMX_LINK4_NAME_STR = "vmx_link4";
const std::string VMX_LINK5_NAME_STR = "vmx_link5";
const std::string VMX_LINK6_NAME_STR = "vmx_link6";
const std::string VMX_LINK7_NAME_STR = "vmx_link7";

const std::string VMX_LINK0_MAC_STR = "32:26:0a:2e:bb:f0";
const std::string VMX_LINK1_MAC_STR = "32:26:0a:2e:bb:f1";
const std::string VMX_LINK2_MAC_STR = "32:26:0a:2e:bb:f2";
const std::string VMX_LINK3_MAC_STR = "32:26:0a:2e:bb:f3";
const std::string VMX_LINK4_MAC_STR = "32:26:0a:2e:bb:f4";
const std::string VMX_LINK5_MAC_STR = "32:26:0a:2e:bb:f5";
const std::string VMX_LINK6_MAC_STR = "32:26:0a:2e:bb:f6";
const std::string VMX_LINK7_MAC_STR = "32:26:0a:2e:bb:f7";

const std::string VMX_LINK0_IP_ADDR_STR = "103.30.00.2";
const std::string VMX_LINK1_IP_ADDR_STR = "103.30.10.2";
const std::string VMX_LINK2_IP_ADDR_STR = "103.30.20.2";
const std::string VMX_LINK3_IP_ADDR_STR = "103.30.30.2";
const std::string VMX_LINK4_IP_ADDR_STR = "103.30.40.2";
const std::string VMX_LINK5_IP_ADDR_STR = "103.30.50.2";
const std::string VMX_LINK6_IP_ADDR_STR = "103.30.60.2";
const std::string VMX_LINK7_IP_ADDR_STR = "103.30.70.2";

const std::string PEER0_MAC_STR = "32:26:0a:2e:cc:f0";
const std::string PEER1_MAC_STR = "32:26:0a:2e:cc:f1";
const std::string PEER2_MAC_STR = "32:26:0a:2e:cc:f2";
const std::string PEER3_MAC_STR = "32:26:0a:2e:cc:f3";
const std::string PEER4_MAC_STR = "32:26:0a:2e:cc:f4";
const std::string PEER5_MAC_STR = "32:26:0a:2e:cc:f5";
const std::string PEER6_MAC_STR = "32:26:0a:2e:cc:f6";
const std::string PEER7_MAC_STR = "32:26:0a:2e:cc:f7";

const std::string PEER0_IP_ADDR_STR = "103.30.00.3";
const std::string PEER1_IP_ADDR_STR = "103.30.10.3";
const std::string PEER2_IP_ADDR_STR = "103.30.20.3";
const std::string PEER3_IP_ADDR_STR = "103.30.30.3";
const std::string PEER4_IP_ADDR_STR = "103.30.40.3";
const std::string PEER5_IP_ADDR_STR = "103.30.50.3";
const std::string PEER6_IP_ADDR_STR = "103.30.60.3";
const std::string PEER7_IP_ADDR_STR = "103.30.70.3";

AfiClient *aficlient;
std::string gTestTimeStr;
std::string gtestOutputDirName;
std::string gtestExpectedDirName = "GTEST_EXPECTED";
std::string tsharkBinary = "/usr/bin/tshark";
std::atomic<bool> test_complete(false);

void getTimeStr(std::string &timeStr);
static void tStartTsharkCapture(std::string &tcName,
                                std::string &tName,
                                std::vector<std::string> &interfaces);
static void tVerifyPackets(std::string &tcName,
                         std::string &tName,
                         std::vector<std::string> &interfaces);

static void stopTsharkCapture(void);
//
// Sandbox open
//
//  +------------------------------------------------+
//  |                                                |
//  o p0 ge-0/0/0 In                 Out ge-0/0/0 p0 o
//  |                                                |
//  o p1 ge-0/0/1 In                 Out ge-0/0/1 p1 o
//  |                                                |
//  o p2 ge-0/0/2 In                 Out ge-0/0/2 p2 o
//  |                                                |
//  o p3 ge-0/0/3 In                 Out ge-0/0/3 p3 o
//  |                                                |
//  o p4 ge-0/0/4 In                 Out ge-0/0/4 p4 o
//  |                                                |
//  o p5 ge-0/0/5 In                 Out ge-0/0/5 p5 o
//  |                                                |
//  o p6 ge-0/0/6 In                 Out ge-0/0/6 p6 o
//  |                                                |
//  o p7 ge-0/0/7 In                 Out ge-0/0/7 p7 o
//  |                   Sandbox                      |
//  +------------------------------------------------+
//

TEST(AFI, SandboxOpen)
{
    int ret = 0;

    boost::asio::io_service io_service;

    aficlient = new AfiClient(io_service,
                              afiServerAddr,
                              afiHospathAddr,
                              AFT_CLIENT_HOSTPATH_PORT,
                              false, false);

    ASSERT_TRUE(aficlient != NULL);

    ret = aficlient->openSandbox(sbName, SB_NUM_CFG_PORTS);

    EXPECT_EQ(ret, 0);
}

//
// Counter Node
//
//                  +-------------------------------+
//                  |                               |
//       ,-----.    |                               |
//  veth0|     |    |              list             |
//   ----o     o----o--------->+----------+         |
//       |_____|    |p0        | counter  |         |
//      vmx_link0   |ge-0/0/0  +----------+         |
//         /\       |          |       ---|-------->o
//         ||       |          +----------+       p1|
//         ||       |                       ge-0/0/1|
//         ||       |      Sandbox                  |
//         ||       +-------------------------------+
//    Input Packet
//

TEST(AFI, CounterNode)
{
    int ret = 0;
    std::string tcName = "AFI";
    std::string tName  = "CounterNode";

    std::vector<std::string> capture_ifs;
    capture_ifs.push_back(GE_0_0_0_VMX_IF_NAME);
    capture_ifs.push_back(GE_0_0_1_VMX_IF_NAME);
    tStartTsharkCapture(tcName, tName, capture_ifs);

    AftNodeToken cntrToken  = aficlient->addCounterNode();

    AftNodeToken outputPortToken =
                 aficlient->getOuputPortToken(SB_P1_PORT_INDEX);

    AftTokenVector tokVec;

    tokVec = {cntrToken, outputPortToken};

    AftNodeToken listToken  = aficlient->createList(tokVec);

    ret = aficlient->setInputPortNextNode(SB_P0_PORT_INDEX, listToken);
    EXPECT_EQ(0, ret);


    ret = SendRawEth(VMX_LINK0_NAME_STR,
                     TestPacketLibrary::TEST_PKT_ID_PUNT_ICMP_ECHO);
    EXPECT_EQ(0, ret);

    stopTsharkCapture();
    tVerifyPackets(tcName, tName, capture_ifs);
}

//
// Discard Node
//                  +---------------------+
//                  |                     |
//       ,-----.    |                     |
//  veth0|     |    |                     |
//   ----o     o----o----->[Discard]      |
//       |_____|    |p0                   |
//      vmx_link0   |ge-0/0/0             |
//         /\       |                     |
//         ||       |                     |
//         ||       |      Sandbox        |
//         ||       +---------------------+
//    Input Packet
//

TEST(AFI, DiscardNode)
{
    int ret = 0;
    std::string tcName = "AFI";
    std::string tName  = "DiscardNode";


    AftNodeToken token  = aficlient->addDiscardNode();
    ret = aficlient->setInputPortNextNode(SB_P0_PORT_INDEX, token);
    EXPECT_EQ(0, ret);

    std::vector<std::string> capture_ifs;
    capture_ifs.push_back(GE_0_0_0_VMX_IF_NAME);
    tStartTsharkCapture(tcName, tName, capture_ifs);

    ret = SendRawEth(VMX_LINK0_NAME_STR,
                     TestPacketLibrary::TEST_PKT_ID_PUNT_ICMP_ECHO);
    EXPECT_EQ(0, ret);
    stopTsharkCapture();
    tVerifyPackets(tcName, tName, capture_ifs);
}

//
// Sandbox hostpath: Punt
//                                      AFI Client
//                                          ^
//                                          |
//                                          |
//                  +------------------+    |
//                  |                  |    |
//       ,-----.    |                  |    |
//  veth0|     |    |                  |    |
//   ----o     o----o----------------->o----'
//       |_____|    |p0            punt|
//      vmx_link0   |ge-0/0/0          |
//         /\       |                  |
//         ||       |                  |
//         ||       |      Sandbox     |
//         ||       +------------------+
//    Input Packet
//

void
readPuntedPkts(std::string &ctx, int num_pkts,
               TestPacketLibrary::TestPacketId tcPktNum)
{
    int ret;
    int rcvd_num_pkts = 0;
    AftPacketPtr pkt = AftPacket::createReceive();

    TestPacket* testPkt = testPacketLibrary.getTestPacket(tcPktNum);

#define PKT_BUFF_SIZE 1500
    char pkt_buff[PKT_BUFF_SIZE];

    int pkt_len = testPkt->getEtherPacket(pkt_buff, PKT_BUFF_SIZE);

    while (!test_complete.load()) {
        ret = aficlient->recvHostPathPacket(pkt);
        EXPECT_EQ(0, ret);
        EXPECT_EQ(pkt->dataSize(), pkt_len);

        pktTrace("Sent packet", pkt_buff, pkt_len);
        pktTrace("Received pkt", (char *)(pkt->data()), pkt->dataSize());

        ret = memcmp(pkt_buff, pkt->data(), pkt_len);
        // TBD: FIXME
        //EXPECT_EQ(0, ret);

        rcvd_num_pkts++;
        if (rcvd_num_pkts >= num_pkts) {
            break;
        }
    }
}

TEST(AFI, SandboxHostpathPunt)
{
    int ret = 0;
    int num_pkts_to_send = 1;
    std::string tcName = "AFI";
    std::string tName  = "SandboxHostpathPunt";
    ASSERT_TRUE(aficlient != NULL);

    std::vector<std::string> capture_ifs;
    capture_ifs.push_back(GE_0_0_0_VMX_IF_NAME);

    tStartTsharkCapture(tcName, tName, capture_ifs);

    test_complete.store(false);
    boost::thread hpRecvThread(boost::bind(&readPuntedPkts, boost::ref(tName),
             num_pkts_to_send, TestPacketLibrary::TEST_PKT_ID_PUNT_ICMP_ECHO));
    //
    // Set up punt for packets incoming on port p0
    //
    AftNodeToken puntPortToken  = aficlient->getOuputPortToken(SB_PUNT_PORT_INDEX);

    ret = aficlient->setInputPortNextNode(SB_P0_PORT_INDEX, puntPortToken);
    EXPECT_EQ(0, ret);


    for (int i = 0; i < num_pkts_to_send; i++) {
        ret = SendRawEth(VMX_LINK0_NAME_STR,
                         TestPacketLibrary::TEST_PKT_ID_PUNT_ICMP_ECHO);
        EXPECT_EQ(0, ret);
        sleep(1);
    }

    test_complete.store(true);

    if (hpRecvThread.timed_join( boost::posix_time::seconds(5))) {
        std::cout<<"\nDone!\n";
    } else {
        std::cerr<<"\nTimed out!\n";
    }

    stopTsharkCapture();
    tVerifyPackets(tcName, tName, capture_ifs);
}

//
// Sandbox Hostpath: Layer 2 inject
//
//            AFI Client
//            injects
//            L2 packet
//               |              Expected Packet
//               |                       /\
//               |                       ||
//   +-----------|---------+             ||
//   |           |         |             ||
//   |           |         |    ,-----. veth1
//   |           |         |    |     o-----
//   |           `-------->o----o     |
//   |                   p1|    |_____|
//   |             ge-0/0/1|    vmx_link1
//   |                     |
//   |                     |
//   |     Sandbox         |
//   +---------------------+
//
//

TEST(AFI, SandboxL2Inject)
{
    int ret = 0;
    std::string tcName = "AFI";
    std::string tName  = "SandboxL2Inject";
    ASSERT_TRUE(aficlient != NULL);


    std::vector<std::string> capture_ifs;
    capture_ifs.push_back(GE_0_0_1_VMX_IF_NAME);

    tStartTsharkCapture(tcName, tName, capture_ifs);

    test_complete.store(false);

    TestPacket* testPkt = testPacketLibrary.getTestPacket(
                        TestPacketLibrary::TEST_PKT_ID_IPV4_ECHO_REQ_TO_PEER1);


#define PKT_BUFF_SIZE 1500
    char pkt_buff[PKT_BUFF_SIZE];

    int pkt_len = testPkt->getEtherPacket(pkt_buff, PKT_BUFF_SIZE);

    const int num_pkts_to_send = 1;
    for (int i = 0; i < num_pkts_to_send; i++) {
        ret = aficlient->injectL2Packet(SB_INDEX,          // Sandbox Index
                                        SB_P1_PORT_INDEX,  // Port Index
                                        (uint8_t *)pkt_buff, pkt_len);

        EXPECT_EQ(0, ret);
        sleep(1);
    }

    test_complete.store(true);

    stopTsharkCapture();
    tVerifyPackets(tcName, tName, capture_ifs);
}

//
// IPv4 Routing
//                                                                Expected Packet
//                                                                         /\
//                                                                         ||
//                  |                                        |             ||
//                  |                                        |             ||
//       ,-----.    |                /\                      |    ,-----. veth3
//  veth2|     |    |               /  \                     |    |     o-----
//   ----o     o----o------------->/ R1-\---->[EthEncap]---->o----o     |
//       |_____|    |p2           /______\                 p3|    |_____|
//      vmx_link2   |ge-0/0/2   Routing Table        ge-0/0/3|    vmx_link3
//         /\       |         R1 = 103.30.30.0/24            |
//         ||       |                                        |
//         ||       |                                        |
//         ||
//    Input Packet
//

#define SB_ROUTE_PREFIX_R1 "103.30.30.0/24"

TEST(AFI, IPv4Routing)
{
    int ret = 0;
    std::string tcName = "AFI";
    std::string tName  = "IPv4Routing";

    std::vector<std::string> capture_ifs;
    capture_ifs.push_back(GE_0_0_2_VMX_IF_NAME);
    capture_ifs.push_back(GE_0_0_3_VMX_IF_NAME);
    tStartTsharkCapture(tcName, tName, capture_ifs);

    AftNodeToken puntPortToken;
    AftNodeToken rttToken;
    AftNodeToken rtTargetPortToken;
    AftNodeToken etherEncapToken;

    ASSERT_TRUE(aficlient != NULL);

    test_complete.store(false);

    //
    // Create Routing Table
    //
    puntPortToken = aficlient->getOuputPortToken(SB_PUNT_PORT_INDEX);

    rttToken = aficlient->addRouteTable(sbIPv4RttName, puntPortToken);

    ret = aficlient->setInputPortNextNode(SB_P2_PORT_INDEX,
                                          rttToken);

    //
    // Add route to routing table
    //
    rtTargetPortToken  = aficlient->getOuputPortToken(SB_P3_PORT_INDEX);

    etherEncapToken = aficlient->addEtherEncapNode(
                                             PEER3_MAC_STR,    // dst mac
                                             GE_0_0_3_MAC_STR, // src mac
                                             "0",
                                             "0",
                                             rtTargetPortToken);

    ret = aficlient->addRoute(rttToken, SB_ROUTE_PREFIX_R1, etherEncapToken);

    EXPECT_EQ(0, ret);

    const int num_pkts_to_send = 1;
    for (int i = 0; i < num_pkts_to_send; i++) {
        ret = SendRawEth(VMX_LINK2_NAME_STR,
                  TestPacketLibrary::TEST_PKT_ID_IPV4_ROUTER_ICMP_ECHO_TO_PEER3);
        EXPECT_EQ(0, ret);
        sleep(1);
    }

    test_complete.store(true);

    stopTsharkCapture();
    tVerifyPackets(tcName, tName, capture_ifs);
}

//
// MPLS L2VPN Encap
//
//
//                  |                                                   |
//                  |                                                   |
//       ,-----.    |                                                   |
//  veth4|     |    |          Index Table                              |    Expected Packet
//   ----o     o----o--------->+--------+                               |             /\
//       |_____|    |p4        |        |                               |             ||
//      vmx_link4   |ge-0/0/4  +--------+                               |             ||
//         /\       |          .        .                               |             ||
//         ||       |          +--------+                               |    ,-----. veth5
//         ||       |          +--------+                               |    |     o-----
//         ||       |          |   ll   |-->[LableEncap]-->[EthEncap]-->o----o     |
//    Input Packet  |          +--------+                             p5|    |_____|
//                  |          +--------+                       ge-0/0/5|    vmx_link5
//                  |                                                   |
//                  |                                                   |

const int MPLS_L2VPN_VLAN_ID = 11;
const int INDEX_TABLE_NUM_ENTRIES = 25;
const std::string vlan1_field_name ("packet.ether.vlan1");

TEST(AFI, MPLS_L2VPN_Encap)
{
    int ret = 0;
    std::string tcName = "AFI";
    std::string tName  = "MPLS_L2VPN_Encap";

    std::vector<std::string> capture_ifs;
    capture_ifs.push_back(GE_0_0_4_VMX_IF_NAME);
    capture_ifs.push_back(GE_0_0_5_VMX_IF_NAME);
    tStartTsharkCapture(tcName, tName, capture_ifs);

    AftNodeToken targetPortToken;
    AftNodeToken labelEncapToken;

    ASSERT_TRUE(aficlient != NULL);

    test_complete.store(false);

    AftNodeToken iTableToken =  aficlient->createIndexTable(vlan1_field_name,
                                                   INDEX_TABLE_NUM_ENTRIES);

    ret = aficlient->setInputPortNextNode(SB_P4_PORT_INDEX,
                                          iTableToken);

    targetPortToken  = aficlient->getOuputPortToken(SB_P5_PORT_INDEX);



    AftNodeToken etherEncapToken = aficlient->addEtherEncapNode(
                                             PEER5_MAC_STR,    // dst mac
                                             GE_0_0_5_MAC_STR, // src mac
                                             "0",
                                             "0",
                                             targetPortToken);


    //
    // Outer label: 1000002
    // Inner label: 16
    //
    labelEncapToken = aficlient->addLabelEncap("1000002",
                                               "16",
                                               etherEncapToken);

    ret = aficlient->addIndexTableEntry(iTableToken,
                                        MPLS_L2VPN_VLAN_ID,
                                        labelEncapToken);

    const int num_pkts_to_send = 1;
    for (int i = 0; i < num_pkts_to_send; i++) {
        ret = SendRawEth(VMX_LINK4_NAME_STR,
                  TestPacketLibrary::TEST_PKT_ID_IPV4_VLAN);
        EXPECT_EQ(0, ret);
        sleep(1);
    }

    test_complete.store(true);

    stopTsharkCapture();
    tVerifyPackets(tcName, tName, capture_ifs);
}

//
// MPLS L2VPN Decap
//                  |                                            |
//                  |                                            |
//       ,-----.    |                                            |
//  veth5|     |    |                        Index Table         |    Expected Packet
//   ----o     o----o--------[LableDecap]--->+--------+          |             /\
//       |_____|    |p5                      |        |          |             ||
//      vmx_link5   |ge-0/0/5                +--------+          |             ||
//         /\       |                        .        .          |             ||
//         ||       |                        +--------+          |    ,-----. veth4
//         ||       |                        +--------+          |    |     o-----
//         ||       |                        |   ll   |--------->o----o     |
//    Input Packet  |                        +--------+        p4|    |_____|
//                  |                        +--------+  ge-0/0/4|    vmx_link4
//                  |                                              |
//                  |                                              |

TEST(AFI, MPLS_L2VPN_Decap)
{
    int ret = 0;
    std::string tcName = "AFI";
    std::string tName  = "MPLS_L2VPN_Decap";

    std::vector<std::string> capture_ifs;
    capture_ifs.push_back(GE_0_0_4_VMX_IF_NAME);
    capture_ifs.push_back(GE_0_0_5_VMX_IF_NAME);
    tStartTsharkCapture(tcName, tName, capture_ifs);

    ASSERT_TRUE(aficlient != NULL);

    test_complete.store(false);

    AftNodeToken iTableToken =  aficlient->createIndexTable(
                                                   vlan1_field_name,
                                                   INDEX_TABLE_NUM_ENTRIES);

    AftNodeToken labelDecapToken = aficlient->addLabelDecap(iTableToken);

    ret = aficlient->setInputPortNextNode(SB_P5_PORT_INDEX,
                                          labelDecapToken);

    AftNodeToken outputPortToken = aficlient->getOuputPortToken(
                                                   SB_P4_PORT_INDEX);

    ret = aficlient->addIndexTableEntry(iTableToken,
                                        MPLS_L2VPN_VLAN_ID,
                                        outputPortToken);

    const int num_pkts_to_send = 1;
    for (int i = 0; i < num_pkts_to_send; i++) {
        ret = SendRawEth(VMX_LINK5_NAME_STR,
                  TestPacketLibrary::TEST_PKT_ID_MPLS_L2VLAN);
        EXPECT_EQ(0, ret);
        sleep(1);
    }

    test_complete.store(true);

    stopTsharkCapture();
    tVerifyPackets(tcName, tName, capture_ifs);
}

void
getTimeStr(std::string &timeStr)
{

  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer,80,"%d%m%Y_%I%M%S",timeinfo);
  timeStr = buffer;
}

static std::string
getShellCmdOutput(std::string &cmd)
{
    std::array<char, 128> buff;

    std::string output;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buff.data(), 128, pipe.get()) != NULL)
            output += buff.data();
    }
    return output;
}

static void
tStartTsharkCapture (std::string &tcName,
                      std::string &tName,
                      std::vector<std::string> &interfaces)
{
    int ret;
    std::string cmd, cmd_output;
    std::string tDir = tcName + "/" + tName;
    std::string tOutputDir = gtestOutputDirName + "/" + tDir;

    cmd = "mkdir -p " + tOutputDir;
    ret = system(cmd.c_str());
    EXPECT_EQ(0, ret);

    for(auto interface : interfaces) {

        std::string ifPcapFile = tOutputDir + "/" + interface  + ".pcap";
        std::string ifstderrFile = tOutputDir + "/" + interface  + ".stderr";
        std::string tsharkCmd = tsharkBinary + " -i " + interface + " -f 'not arp'" +
                             " -w " + ifPcapFile + " 2> " + ifstderrFile + " < /dev/null &";

        ret = system(tsharkCmd.c_str());
        EXPECT_EQ(0, ret);

        cmd = "cat " + ifstderrFile;
        cmd_output = getShellCmdOutput(cmd);
        while (cmd_output.find("Capturing on") != std::string::npos) {
            sleep (1);
            cmd_output = getShellCmdOutput(cmd);
        }
        sleep (1);

        //cmd = "rm -f " + ifstderrFile;
        //cmd_output = getShellCmdOutput(cmd);
    }

    //
    // tshark takes somet to run
    //
    sleep(15);
}

static void
tVerifyPackets (std::string &tcName,
                std::string &tName,
                std::vector<std::string> &interfaces)
{
    int ret = 0;
    std::string cmd;
    std::string tDir = tcName + "/" + tName;
    std::string tOutputDir = gtestOutputDirName + "/" + tDir;
    std::string tExpectedDir = gtestExpectedDirName + "/" + tDir;

    for(auto interface : interfaces) {
        std::string ifPcapFile = tOutputDir + "/" + interface  + ".pcap";
        std::string ifPktsFile = tOutputDir + "/" + interface  + ".pkts";
        std::string ifExpectedPcapFile = tExpectedDir + "/" + interface  + ".pcap";
        std::string ifExpectedPktsFile = tOutputDir + "/" + interface  + ".pkts.expected";

        cmd = tsharkBinary + " -nx " + " -r " + ifExpectedPcapFile + " > " + ifExpectedPktsFile + " 2> /dev/null";
        ret = system(cmd.c_str());
        EXPECT_EQ(0, ret);
        cmd = tsharkBinary + " -nx " + " -r " + ifPcapFile + " > " + ifPktsFile + " 2> /dev/null";
        ret = system(cmd.c_str());
        EXPECT_EQ(0, ret);
        cmd = "diff " + ifExpectedPktsFile + " " + ifPktsFile;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        std::cout << cmd << std::endl;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        ret = system(cmd.c_str());
        EXPECT_EQ(0, ret);

        if (ret != 0) {
            // diff present
            std::cout << "Expected packets and actual packets differ" << std::endl;
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            std::cout << "Expected: " << ifExpectedPcapFile << std::endl;
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            cmd = tsharkBinary + " -nVx " + " -r " + ifExpectedPcapFile + " 2> /dev/null";
            std::cout << getShellCmdOutput(cmd);
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            std::cout << "Actual: "<< ifPcapFile << std::endl;
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            cmd = tsharkBinary + " -nVx " + " -r " + ifPcapFile + " 2> /dev/null";
            std::cout << getShellCmdOutput(cmd);
        } else {
            // No diff
            std::cout << "SUCCESS: Actual packets matches with expected packets" << std::endl;
        }
    }
}

static
void stopTsharkCapture (void)
{
    int ret;
    std::string cmd = "ps -auwx | grep tshark | grep -v grep";
    std::string cmd_output = getShellCmdOutput(cmd);
    while (cmd_output.find("tshark") != std::string::npos) {
        sleep (1);
        system("/usr/bin/pkill tshark");
        cmd_output = getShellCmdOutput(cmd);
    }
}

//
// gtest main
//
int main(int argc, char **argv) {

    getTimeStr(gTestTimeStr);
    std::cout<<gTestTimeStr;

    gtestOutputDirName = "GTEST_" + gTestTimeStr;
    std::string mk_gtestOutputDirName_cmd = "mkdir -p " + gtestOutputDirName;

    system(mk_gtestOutputDirName_cmd.c_str());

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::GTEST_FLAG(filter) = "*";
    //::testing::GTEST_FLAG(filter) = "*SandboxOpen*:*CounterNode*";
    //::testing::GTEST_FLAG(filter) = "*SandboxOpen*:*SandboxHostpathPunt*";
    //::testing::GTEST_FLAG(filter) = "*SandboxOpen*:*SandboxL2Inject*";
    //::testing::GTEST_FLAG(filter) = "*SandboxOpen*:*IPv4Routing*";
    //::testing::GTEST_FLAG(filter) = "*SandboxOpen*:*MPLS_L2VPN_Encap*";
    //::testing::GTEST_FLAG(filter) = "*SandboxOpen*:*MPLS_L2VPN_Decap*";
    return RUN_ALL_TESTS();
}
