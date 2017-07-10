//
// AfiClient.h
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

#ifndef __AfiClient__
#define __AfiClient__

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

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/algorithm/string.hpp>

#include "jnx/Aft.h"
#include "jnx/AfiTransport.h"
#include "Utils.h"

#define BOOST_UDP boost::asio::ip::udp::udp

//
// @class   AfiClient
// @brief   Implements a sample AFI client
//
class AfiClient
{
public:
    //
    // Constructor
    //
    AfiClient(boost::asio::io_service &ioService,
              const std::string       &afiServerAddr,
              const std::string       &afiHostpathAddr,
              short                    port,
              bool                     startHospathSrvr,
              bool                     tracing)
              : _afiServerAddr(afiServerAddr),
                _afiHostpathAddr(afiHostpathAddr),
                _ioService(ioService),
                _hpUdpSock(ioService, BOOST_UDP::endpoint(BOOST_UDP::v4(), port)),
                _tracing(tracing) {

        BOOST_UDP::resolver resolver(_ioService);

        std::vector<std::string> hostpathAddr_substrings;
        boost::split(hostpathAddr_substrings, _afiHostpathAddr,
                     boost::is_any_of(":"));
        std::string &hpIpStr      =  hostpathAddr_substrings.at(0);
        std::string &hpUDPPortStr =  hostpathAddr_substrings.at(1);

        BOOST_UDP::resolver::query query(BOOST_UDP::v4(), hpIpStr, hpUDPPortStr);
        BOOST_UDP::resolver::iterator iterator =
                         resolver.resolve(query);
        _vmxtHostpathEndpoint = *iterator;

        //
        // Initialize transport connection to AFI server
        //
        _transport = AfiTransport::create(_afiServerAddr);
        assert(_transport != nullptr);

        if (startHospathSrvr) {
            startAfiPktRcvr();
        }
    }

    //
    // Destructor
    //
    ~AfiClient();

    //
    // Create sandbox
    //
    int openSandbox(const std::string &sandbox_name, u_int32_t numPorts);

    //
    // Add a routing table to the sandbox
    //
    AftNodeToken addRouteTable(const std::string &rttName,
                               AftNodeToken defaultTragetToken);

    //
    // Add route to a routing table
    //
    int addRoute(AftNodeToken      rttNodeToken,
                 const std::string &prefix,
                 AftNodeToken       routeTragetToken);

    //
    // Create Index table
    //
    AftNodeToken createIndexTable(const std::string &field_name,
                                  AftIndex iTableSize);

    //
    // Add Index table entry
    //
    int addIndexTableEntry(AftNodeToken iTableToken,
                           u_int32_t    entryIndex,
                           AftNodeToken entryTargetToken);


    //
    // Create list
    //
    AftNodeToken createList (AftTokenVector tokVec);

    //
    // Set next node for an input port
    //
    int setInputPortNextNode(AftIndex inputPortIndex, AftNodeToken nextToken);

    //
    // Get token for an output port
    //
    AftNodeToken getOuputPortToken(AftIndex outputPortIndex);

    //
    // Add ethernet encapsulation node
    //
    AftNodeToken addEtherEncapNode(const std::string &dst_mac,
                                   const std::string &src_mac,
                                   const std::string &ivlanStr,
                                   const std::string &ovlanStr,
                                   AftNodeToken       nextToken);

    //
    // Add MPLS label encapsulation node
    //
    AftNodeToken addLabelEncap(const std::string &outerLabelStr,
                               const std::string &innerLabelStr,
                               AftNodeToken       nextToken);

    //
    // Add MPLS label decapsulation node
    //
    AftNodeToken addLabelDecap(AftNodeToken nextToken);

    //
    // Add counter node
    //
    AftNodeToken addCounterNode(void);

    //
    // Add discard node
    //
    AftNodeToken addDiscardNode(void);

    //
    // Handler hostpath packet from sandbox
    //
    int recvHostPathPacket(AftPacketPtr &pkt);

    //
    // Inject layer 2 packet to a port
    //
    int injectL2Packet(AftSandboxId  sandboxId,
                       AftIndex      portIndex,
                       uint8_t      *l2Packet,
                       int           l2PacketLen);
    //
    // Command line interface for this AFI client
    //
    void cli(void);

private:
    std::string                 _afiServerAddr;   //< AFI server address
    std::string                 _afiHostpathAddr; //< AFI hostpath address
    boost::asio::io_service&    _ioService;
    BOOST_UDP::socket           _hpUdpSock;     //< Hospath UDP socket

    BOOST_UDP::endpoint         _vmxtHostpathEndpoint;

    AftSandboxPtr               _sandbox;
    AftTransportPtr             _transport;

    std::vector<std::string>    _commandHistory;
    bool                        _tracing;  //< True if debug tracing is enabled

    //
    // Handle CLI commands
    //
    void handleCliCommand(std::string const & command_str);

    //
    // Hostpath UDP server
    //
    void hostPathUDPSrvr(void);

    void startAfiPktRcvr(void);
};

#endif // __AfiClient__
