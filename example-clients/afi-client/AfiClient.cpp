//
// AfiClient.cpp
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

#include "AfiClient.h"

//
// @fn
// openSandbox
//
// @brief
// Open AFI sandbox created via Junos CLI configuration
//
// @param[in]
//     sandbox_name Sandbox name
// @param[in]
//     numCfgPorts Number of configured ports
// @return 0 - Success, -1 - Failure
//

int
AfiClient::openSandbox (const std::string &sandbox_name,
                        u_int32_t          numCfgPorts)
{
    u_int32_t numStdPorts = 1;
    bool status = _transport->alloc("trio", sandbox_name,
                                    (numCfgPorts + numStdPorts),
                                    (numCfgPorts + numStdPorts));
    if (status == 0) {
        std::cout << "_transport->alloc failed!" << std::endl;
        return -1;
    }

    if (_transport->open(sandbox_name, _sandbox)) {
        std::cout << "Got Sandbox handle!" << std::endl;
    } else {
        std::cout << "Open Sandbox failed!" << std::endl;
        return -1;
    }

    AftPortTablePtr inputPorts = _sandbox->inputPortTable();

    std::cout << "InputPorts: " << std::endl;

    AftPortEntryPtr port;
    for (AftIndex  i = 0 ; i < (inputPorts->maxIndex() - 1);  i++) {
        if (inputPorts->portForIndex(i, port)) {
            std::cout << "index: " << i ;
            std::cout << " port name: " << port->portName();
            std::cout << " token: " << port->nodeToken();
            std::cout << std::endl;
        } else {
            std::cout << "Error getting port for port index " << i << std::endl;
        }
    }

    AftPortTablePtr outputPorts = _sandbox->outputPortTable();
    std::cout << "OutputPorts: " << std::endl;

    for (AftIndex  i = 0 ; i < outputPorts->maxIndex();  i++) {
        if (outputPorts->portForIndex(i, port)) {
            std::cout << "index: " << i ;
            std::cout << " port name: " << port->portName();
            std::cout << " token: " << port->nodeToken();
            std::cout << std::endl;
        } else {
            std::cout << "Error getting port for port index " << i << std::endl;
        }
    }

    return 0;
}

//
// @fn
// addRouteTable
//
// @brief
// Add routing table
//
// @param[in]
//     rttName Routing table name
// @param[in]
//     defaultTragetToken  Default route target token
// @return routing table node token
//

AftNodeToken
AfiClient::addRouteTable (const std::string &rttName,
                          AftNodeToken defaultTragetToken)
{
    AftNodeToken        rttNodeToken;
    AftNodeToken        discardNodeToken;
    AftInsertPtr        insert;

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);

    //
    // Create a route lookup tree
    //
    auto treePtr = AftTree::create(AftField("packet.lookupkey"),
                                   defaultTragetToken);
    //
    // Stich the optional params
    //
    uint64_t skipBits = 16;
    treePtr->setNodeParameter("rt.app", AftDataString::create("NH"));
    treePtr->setNodeParameter("rt.nhType", AftDataString::create("route"));
    treePtr->setNodeParameter("rt.skipBits", AftDataInt::create(skipBits));

    rttNodeToken = insert->push(treePtr, rttName);

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);

    return rttNodeToken;
}

//
// @fn
// addRoute
//
// @brief
// Add route to a routing table
//
// @param[in]
//     rttNodeToken Routing table node token
// @param[in]
//     prefix Route prefix
// @param[in]
//     routeTragetToken Route target token
// @return 0 - Success, -1 - Error
//

int
AfiClient::addRoute (AftNodeToken       rttNodeToken,
                     const std::string &prefix,
                     AftNodeToken       routeTragetToken)
{
    AftInsertPtr        insert;
    AftNodeToken        outputPortToken;

    std::vector<std::string> prefix_sub_strings;
    boost::split(prefix_sub_strings, prefix, boost::is_any_of("./"));

    if (prefix_sub_strings.size() != 5) {
        std::cout << "Invalid prefix" << std::endl;
        return -1;
    }

    AftDataBytes prefix_bytes;
    for(int t = 0; t < (prefix_sub_strings.size() - 1); ++t){
        uint8_t byte = std::strtoul(prefix_sub_strings.at(t).c_str(), NULL, 0);
        prefix_bytes.push_back(byte);
    }

    AftDataPtr  data_prefix = AftDataPrefix::create(prefix_bytes,
                       (uint32_t)(std::strtoul(prefix_sub_strings.at(4).c_str(),
                        NULL, 0)));
    AftKey      key     = AftKey(AftField("packet.ip4.daddr"), data_prefix);

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);

    std::cout <<"Adding route ";
    std::cout << prefix << " ---> Node token " << routeTragetToken << std::endl;

    //
    // Create a route
    //
    AftEntryPtr entryPtr = AftEntry::create(rttNodeToken, key,
                                            routeTragetToken);

    //
    // Set the optional params for Entry
    //
    uint8_t hwFlush = 0;
    entryPtr->setEntryParameter("route.string",
                                AftDataString::create("IPv4 route"));
    entryPtr->setEntryParameter("route.hwFlush",
                                AftDataInt::create(hwFlush));

    insert->push(entryPtr);

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);

    return 0;
}

//
// @fn
// createIndexTable
//
// @brief
// Create index table
//
// @param[in]
//     field_name Index table lookup field name
// @param[in]
//     iTableSize Index table size
// @return Index table node token
//

AftNodeToken
AfiClient::createIndexTable (const std::string &field_name,
                             AftIndex           iTableSize)
{
    AftInsertPtr        insert;

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);

    AftNodePtr iTablePtr = AftTable::create(AftField(field_name), iTableSize,
                                            AFT_NODE_TOKEN_DISCARD);
    iTablePtr->setNodeParameter("index.app", AftDataString::create("NH"));

    AftNodeToken iTableToken = insert->push(iTablePtr, "IndexTable");

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);

    return iTableToken;
}

//
// @fn
// addIndexTableEntry
//
// @brief
// Add entry into index table
//
// @param[in]
//     iTableToken Index table token
// @param[in]
//     entryIndex Index at which entry is to be added
// @param[in]
//     entryTargetToken Entry target token
// @return 0 - Success, -1 - Error
//

int
AfiClient::addIndexTableEntry (AftNodeToken iTableToken,
                               u_int32_t    entryIndex,
                               AftNodeToken entryTargetToken)
{
    AftInsertPtr        insert;

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);

    AftEntryPtr entry = AftEntry::create(iTableToken,
                                         entryIndex,
                                         entryTargetToken);

    insert->push(entry);
    std::cout << "Index table entry pushed. ";
    std::cout <<"(Index: " << entryIndex << " target token: "<< entryTargetToken << ")" << std::endl;

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);

    return 0;
}


//
// @fn
// createList
//
// @brief
// Create List
//
// @param[in]
//     tokVec Token vector
// @return List token
//

AftNodeToken
AfiClient::createList (AftTokenVector tokVec)
{
    AftInsertPtr        insert;

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);

    //
    // Build a list of provided tokens
    //
    //AftNodePtr list = AftList::create(token1, token2);
    AftNodePtr list = AftList::create(tokVec);

    insert->push(list);

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);

    return list->nodeToken();
}

//
// @fn
// setInputPortNextNode
//
// @brief
// Set next node for an input port
//
// @param[in]
//     inputPortIndex Input port index
// @param[in]
//     nextToken Next node token
// @return 0 - Success, -1 - Error
//

int
AfiClient::setInputPortNextNode (AftIndex     inputPortIndex,
                                 AftNodeToken nextToken)
{
    _sandbox->setInputPortByIndex(inputPortIndex, nextToken);
    return 0;
}

//
// @fn
// getOuputPortToken
//
// @brief
// Get token for an output port
//
// @param[in]
//     outputPortIndex Output port index
// @return Outport port's token
//

AftNodeToken
AfiClient::getOuputPortToken(AftIndex outputPortIndex)
{
    AftNodeToken        outputPortToken;

    _sandbox->outputPortByIndex(outputPortIndex, outputPortToken);

    return outputPortToken;
}

//
// @fn
// addEtherEncapNode
//
// @brief
// Add Ethernet encap node
//
// @param[in]
//     dst_mac Destination MAC
// @param[in]
//     src_mac Source MAC
// @param[in]
//     ivlanStr Inner vlan
// @param[in]
//     ovlanStr Outer vlan
// @param[in]
//     nextToken Next node token
// @return Ethernet encap node's token
//

AftNodeToken
AfiClient::addEtherEncapNode(const std::string &dst_mac,
                             const std::string &src_mac,
                             const std::string &ivlanStr,
                             const std::string &ovlanStr,
                             AftNodeToken       nextToken)
{
    AftNodeToken        outListToken;
    AftInsertPtr        insert;

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);
    //
    // Create a key vector of ethernet data
    //

    AftKeyVector encapKeys = { AftKey(AftField("packet.ether.saddr"),
                                      AftDataEtherAddr::create(src_mac)),
                               AftKey(AftField("packet.ether.daddr"),
                                      AftDataEtherAddr::create(dst_mac)) };
    //
    // Create aft encap node
    //
    auto aftEncapPtr = AftEncap::create("ethernet", encapKeys);

    //
    // Stich the optional params
    //
    static u_int64_t nhId = 100; nhId++;
    aftEncapPtr->setNodeParameter("meta.nhid", AftDataInt::create(nhId));
    uint64_t ivlan = std::strtoull(ivlanStr.c_str(),NULL,0);
    uint64_t ovlan = std::strtoull(ovlanStr.c_str(),NULL,0);
    if (ivlan != 0) {
           std::cout << "ivlan: " << ivlan << std::endl;
        aftEncapPtr->setNodeParameter("encap.ether.ivlan",
                                      AftDataInt::create(ivlan));
    }
    if (ovlan != 0) {
           std::cout << "ovlan: " << ovlan << std::endl;
        aftEncapPtr->setNodeParameter("encap.ether.ovlan",
                                      AftDataInt::create(ovlan));
    }

    //
    // Set the List node token as next pointer to encap
    //
    aftEncapPtr->setNodeNext(nextToken);

    AftNodeToken  nhEncapToken = insert->push(aftEncapPtr);

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);

    return nhEncapToken;
}

//
// @fn
// addLabelEncap
//
// @brief
// Add label encap node
//
// @param[in]
//     outerLabelStr Outer label
// @param[in]
//     innerLabelStr Inner label
// @param[in]
//     nextToken Next node token
// @return Label encap node's token
//

AftNodeToken
AfiClient::addLabelEncap(const std::string &outerLabelStr,
                         const std::string &innerLabelStr,
                         AftNodeToken nextToken)
{

    uint64_t outerLabel = std::strtoull(outerLabelStr.c_str(),NULL,0);
    uint64_t innerLabel = std::strtoull(innerLabelStr.c_str(),NULL,0);
    AftNodeToken        listToken;
    AftInsertPtr        insert;
    AftNodeToken nhLabelEncapToken2;

    if (outerLabel == 0) {
        std::cout << "Outer label cannot be 0" << std::endl;
        return -1;
    }

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);

    //
    // Create a key vector of ethernet data
    //
    AftKeyVector encapKeys =
        { AftKey(AftField("packet.ether.saddr"),
                 AftDataEtherAddr::create("00:00:00:01:02:03")),
          AftKey(AftField("packet.ether.daddr"),
                 AftDataEtherAddr::create("00:00:00:04:05:06")) };

    //
    // Create aft encap node
    //
    AftEncap::Ptr aftEncapPtr = AftEncap::create("label", encapKeys);

    //
    // Set the label value
    //
    aftEncapPtr->setNodeParameter("label.value",
                                  AftDataInt::create(outerLabel));


    AftNodeToken nhLabelEncapToken = insert->push(aftEncapPtr);

    if (innerLabel != 0) {
        //
        // Create a key vector of ethernet data
        //
        AftKeyVector encapKeys2 =
            { AftKey(AftField("packet.ether.saddr"),
                     AftDataEtherAddr::create("00:00:00:01:02:03")),
              AftKey(AftField("packet.ether.daddr"),
                     AftDataEtherAddr::create("00:00:00:04:05:06")) };

        //
        // Create aft encap node
        //
        AftEncap::Ptr aftEncapPtr2 = AftEncap::create("label", encapKeys2);

        aftEncapPtr2->setNodeParameter("label.value",
                                       AftDataInt::create(innerLabel));


        nhLabelEncapToken2 = insert->push(aftEncapPtr2);
    }

    AftTokenVector tokVec;

    if (innerLabel != 0) {
        tokVec = {nhLabelEncapToken2, nhLabelEncapToken, nextToken};
    } else {
        tokVec = {nhLabelEncapToken, nextToken};
    }

    AftNodePtr nhList = AftList::create(tokVec);

    u_int64_t set_val = 1;
    nhList->setNodeParameter("list.allocDesc",  AftDataInt::create(set_val));
    listToken = insert->push(nhList);

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);

    return listToken;
}

//
// @fn
// addLabelDecap
//
// @brief
// Add label decap node
//
// @param[in]
//     nextToken Next node token
// @return Label decap node's token
//

AftNodeToken
AfiClient::addLabelDecap(AftNodeToken nextToken)
{
    AftNodeToken        listToken;
    AftInsertPtr        insert;

    //
    // Allocate an insert context
    //
    insert = AftInsert::create(_sandbox);

    auto aftDecapPtr = AftDecap::create("label");

    //
    // Stich the optional params
    //
    static u_int64_t nhId = 1000; nhId++;
    aftDecapPtr->setNodeParameter("meta.nhid", AftDataInt::create(nhId));

    //
    // Set the List node token as next pointer to encap
    //
    //aftDecapPtr->setNodeNext(nextToken);
    AftNodeToken  nhDecapToken = insert->push(aftDecapPtr);

    AftTokenVector tokVec = {nhDecapToken, nextToken};

    AftNodePtr list = AftList::create(tokVec);

    u_int64_t set_val = 1;
    list->setNodeParameter("list.allocDesc",  AftDataInt::create(set_val));
    listToken = insert->push(list);

    //
    // Send all the nodes to the sandbox
    //
    _sandbox->send(insert);
    return listToken;
}

//
// @fn
// addDiscardNode
//
// @brief
// Add counter node
//
// @param[in] void
// @return Counter node token
//

AftNodeToken
AfiClient::addCounterNode (void)
{
    AftNodeToken        counterNodeToken;
    AftInsertPtr        insert;

    insert = AftInsert::create(_sandbox);

    AftNodePtr counter =  AftCounter::create(0, 0, false);

    counterNodeToken = insert->push(counter, "Counter1");
    _sandbox->send(insert);

    return counterNodeToken;
}

//
// @fn
// addDiscardNode
//
// @brief
// Add discard node
//
// @param[in] void
// @return Discard node token
//

AftNodeToken
AfiClient::addDiscardNode (void)
{
    AftNodeToken        discardNodeToken;
    AftInsertPtr        insert;

    insert = AftInsert::create(_sandbox);
    AftNodePtr discard = AftDiscard::create();
    discardNodeToken = insert->push(discard, "Discard");
    _sandbox->send(insert);

    return discardNodeToken;
}

//
// @fn
// recvHostPathPacket
//
// @brief
// Receive hostpath packet
//
// @param[in]
//     pkt Aft packet where received packet will be copied to
// @return 0 - Success, -1 - Error
//

int
AfiClient::recvHostPathPacket(AftPacketPtr &pkt)
{
    size_t recvlen;
    enum { max_length = 2000 };
    char _data[max_length];
    BOOST_UDP::endpoint sender_endpoint;

    // Block until data has been received successfully or an error occurs.
    recvlen = _hpUdpSock.receive_from(
                boost::asio::buffer(_data, max_length), sender_endpoint);

    pktTrace("Received (hostpath) pkt ", _data, recvlen);

    memcpy(pkt->header(), _data, pkt->headerSize());
    memcpy(pkt->data(), _data + pkt->headerSize(), (recvlen - pkt->headerSize()));

    pktTrace("packet header", (char *)(pkt->header()), pkt->headerSize());

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "pkt->headerSize(): " << pkt->headerSize() << " bytes" << std::endl;
    std::cout << "Header: Received " << recvlen << " bytes" << std::endl;

    pkt->headerParse();

    std::cout << "Received packet:" << std::endl;
    std::cout << "----------------" << std::endl;
    std::cout << "Sandbox Id : " << pkt->sandboxId() << std::endl;
    std::cout << "Port Index : " << pkt->portIndex() << std::endl;
    std::cout << "Data Size  : " << pkt->dataSize()  << std::endl;


    std::cout << "pkt->dataSize(): " << pkt->dataSize() << " bytes" << std::endl;


    std::cout << "Data: Received " << recvlen << "bytes" << std::endl;

    pktTrace("pkt data", (char *)(pkt->data()), pkt->dataSize());

    return 0;
}

//
// @fn
// hostPathUDPSrvr
//
// @brief
// Hostpath UDP server
//
// @param[in] void
// @return void
//

void
AfiClient::hostPathUDPSrvr(void)
{
    if (_tracing) {
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "!!!Hostpath Receiver started!!! ";
        std::cout << std::endl;
        std::cout << std::endl;
    }

    enum { max_length = 2000 };
    char _data[max_length];

    for (;;)
    {
        size_t recvlen;
        int ret;
        BOOST_UDP::endpoint sender_endpoint;

        //
        // Allocate packet context *and* buffer
        //
        AftPacketPtr pkt = AftPacket::createReceive();

        ret = recvHostPathPacket(pkt);
    }
}

//
// @fn
// startAfiPktRcvr
//
// @brief
// Start AFI packer receiver
//
// @param[in] void
// @return void
//

void
AfiClient::startAfiPktRcvr(void)
{
    std::thread udpSrvr( [this] { this->hostPathUDPSrvr(); } );
    udpSrvr.detach();
}

//
// @fn
// injectL2Packet
//
// @brief
// Inject layer 2 packet on specified (output)
// port of specified sandbox
//
// @param[in]
//     sandboxId Sandbox index
// @param[in]
//     portIndex Output port index
// @param[in]
//     l2Packet Pointer to layer 2 packet to be injected
// @param[in]
//     l2PacketLen Length of layer 2 packet
// @return void
//

int
AfiClient::injectL2Packet(AftSandboxId  sandboxId,
                          AftIndex      portIndex,
                          uint8_t      *l2Packet,
                          int           l2PacketLen)
{
    std::cout << "Injecting layer2 packet - " ;
    std::cout << "Sandbox index:" << sandboxId  <<" ";
    std::cout << "Port index: "<< portIndex << std::endl;

    if (!l2Packet) {
        std::cout << "l2Packet NULL" << std::endl;
    }

    AftPacketPtr pkt = AftPacket::createTransmit(
                                       l2PacketLen, sandboxId,
                                       portIndex, AftPacket::PacketTypeL2);

    //
    // Get base of packet data.
    // This is in the buffer immediately after the header.
    //
    uint8_t *pktData = pkt->data();

    //
    // Fill the pktData with 'L2 packet to be injected'
    //
    memcpy(pktData, l2Packet, l2PacketLen);

    pktTrace("xmit pkt ", (char *)(pkt->header()), pkt->size());

    _hpUdpSock.send_to(boost::asio::buffer(pkt->header(), pkt->size()),
                                      _vmxtHostpathEndpoint);

    return 0;
}

//
// @fn
// handleCliCommand
//
// @brief
// Handle CLI command
//
// @param[in]
//     command_str CLI command string
// @return void
//

void
AfiClient::handleCliCommand(std::string const & command_str)
{
    std::vector<std::string> command_sub_strings;
    boost::split(command_sub_strings, command_str, boost::is_any_of("\t "));

    std::string command = command_sub_strings.at(0);

    std::vector<std::string> command_args;
    for(int t=0; t < command_sub_strings.size(); ++t){
        if ((t != 0) && (!command_sub_strings.at(t).empty())) {
            command_args.push_back(command_sub_strings.at(t));
        }
    }

    if (command_args.size() > 0) {
        std::string command_arg1 = command_args.at(0);
    }
    if (command_args.size() > 1) {
        std::string command_arg1 = command_args.at(1);
    }

    if ((command.compare("help") == 0) ||
        (command.compare("h")    == 0) ||
        (command.compare("?")    == 0)) {
        std::cout << "\tSupported commands:" << std::endl;
        std::cout << "\t open-sb <sandbox-name> <num-configured-ports>" << std::endl;
        std::cout << "\t          num-configured-ports : Number of configured ports through Junos CLI" << std::endl;
        std::cout << "\t create-rtt <rtt-name> <default-target-node-token>" << std::endl;
        std::cout << "\t create-index-table <table-size>" << std::endl;
        std::cout << "\t add-index-table-entry <index-table-token> <entry-index> <entry-target-token>" << std::endl;
        std::cout << "\t set-input-port-next-node <port-index> <next-node-token>" << std::endl;
        std::cout << "\t add-ether-encap <src-mac> <dst-mac> <0 or inner-vlan-id> <0 or outer-vlan-id> <output-port-token>" << std::endl;
        std::cout << "\t add-label-encap <outer-label> <0 or inner-label> <next-node-token>" << std::endl;
        std::cout << "\t add-label-decap <next-node-token>" << std::endl;
        std::cout << "\t get-output-port-token <ouput-port-index>" << std::endl;
        std::cout << "\t add-route <rtt-token> <prefix> <next-node-token>" << std::endl;
        std::cout << "\t inject-l2-pkt <sandbox-index> <port-index>: Inject layer 2 packet" << std::endl;
        std::cout << "\t history " << std::endl;
        std::cout << "\t clear-history " << std::endl;
        std::cout << "\t quit/exit " << std::endl;

    } else  if (command.compare("open-sb") == 0) {
        if (command_args.size() != 2) {
            std::cout << "Please provide sandbox name and number of configured ports" << std::endl;
            std::cout << "Example: open-sb green 4" << std::endl;
            return;
        }

        std::cout << "Opening Sandbox" << std::endl;
        u_int32_t numPorts =  std::strtoull(command_args.at(1).c_str(),NULL,0);

        openSandbox(command_args.at(0), numPorts);

    } else  if (command.compare("create-rtt") == 0) {
        if (command_args.size() != 2) {
            std::cout << "Please provide routing table name and default target node token" << std::endl;
            std::cout << "Example: create-rtt rtt0 10" << std::endl;
            return;
        }

        std::cout << "Creating routing table... " << std::endl;
        AftNodeToken  defaultTragetToken = std::strtoull(command_args.at(1).c_str(), NULL, 0);
        AftNodeToken rttNodeToken = addRouteTable(command_args.at(0), defaultTragetToken);
        std::cout << "Route table node token: " << rttNodeToken << std::endl;

    } else  if (command.compare("create-index-table") == 0) {
        if (command_args.size() != 2) {
            std::cout << "Please provide table size" << std::endl;
            std::cout << "Example: create-index-table packet.ether.vlan1 25" << std::endl;
            return;
        }
        std::cout << "Creating Index table... " << std::endl;
        AftIndex iTableSize =  std::strtoull(command_args.at(1).c_str(), NULL, 0);
        AftNodeToken iTableToken =  createIndexTable(command_args.at(0), iTableSize);
        std::cout << "Index table node token: " << iTableToken << std::endl;

    } else  if (command.compare("add-index-table-entry") == 0) {
        if (command_args.size() != 3) {
            std::cout << "Please provide index table token, entry index and entry targert token " << std::endl;
            std::cout << "Example: add-vlan-table-entry 0 1 5" << std::endl;
            return;
        }
        std::cout << "Adding Index table entry.. " << std::endl;

        AftNodeToken iTableToken      = std::strtoull(command_args.at(0).c_str(),NULL,0);
        u_int32_t    entryIndex       = std::strtoull(command_args.at(1).c_str(),NULL,0);
        AftNodeToken entryTargetToken = std::strtoull(command_args.at(2).c_str(),NULL,0);

        addIndexTableEntry(iTableToken, entryIndex, entryTargetToken);

    } else  if (command.compare("set-input-port-next-node") == 0) {
        if (command_args.size() != 2) {
            std::cout << "Please provide port index and node-token" << std::endl;
            std::cout << "Example: set-all-input-ports-next-node 0 10" << std::endl;
            return;
        }
        std::cout << "Attaching next token to input port" << std::endl;
        AftIndex inputPortIndex = std::strtoull(command_args.at(0).c_str(),NULL,0);
        AftNodeToken nextToken  = std::strtoull(command_args.at(1).c_str(),NULL,0);

        setInputPortNextNode(inputPortIndex, nextToken);

    } else  if (command.compare("add-ether-encap") == 0) {
        if (command_args.size() != 5) {
            std::cout << "Please provide source mac, destination mac, inner-vlan-id and outer-vlan-id  output port token" << std::endl;
            std::cout << "Examples: " << std::endl;
            std::cout << "add-ether-encap 32:26:0a:2e:ff:f1 5e:d8:f9:32:bd:85 100 200 10" << std::endl;
            std::cout << "add-ether-encap 32:26:0a:2e:ff:f1 5e:d8:f9:32:bd:85 0 0 10" << std::endl;
            return;
        }
        std::cout << "Adding ether encap node" << std::endl;
        AftNodeToken nextToken = std::strtoull(command_args.at(4).c_str(),NULL,0);;

        AftNodeToken nhEncapToken = addEtherEncapNode(command_args.at(0),
                                                      command_args.at(1),
                                                      command_args.at(2),
                                                      command_args.at(3),
                                                      nextToken);

        std::cout << "Ether encap nexthop token: " << nhEncapToken << std::endl;

    } else  if (command.compare("add-label-encap") == 0) {
        if (command_args.size() != 3) {
            std::cout << "Please provide outer label, inner label and next-node-token" << std::endl;
            std::cout << " Pass 0 as inner label if only outer label needs to be added" << std::endl;
            std::cout << "Examples: " << std::endl;
            std::cout << "add-label-encap 100 200 10" << std::endl;
            std::cout << "add-label-encap 100 0 10" << std::endl;
            return;
        }
        std::cout << "Adding label encap node " << std::endl;
        AftNodeToken nextToken = std::strtoull(command_args.at(2).c_str(),NULL,0);
        AftNodeToken labelEncapToken = addLabelEncap(command_args.at(0), command_args.at(1), nextToken);
        std::cout << "Node token: " << labelEncapToken << std::endl;

    } else  if (command.compare("add-label-decap") == 0) {
        if (command_args.size() != 1) {
            std::cout << "Please provide next-token" << std::endl;
            std::cout << "Example: add-label-decap-node 10" << std::endl;
            return;
        }
        std::cout << "Adding label decap node " << std::endl;
        AftNodeToken nextToken = std::strtoull(command_args.at(0).c_str(),NULL,0);
        AftNodeToken labelDecapToken = addLabelDecap(nextToken);
        std::cout << "Node token: " << labelDecapToken << std::endl;

    } else  if (command.compare("get-output-port-token") == 0) {
        if (command_args.size() != 1) {
            std::cout << "Please provide output port index" << std::endl;
            std::cout << "Example: get-output-port-token 10" << std::endl;
            return;
        }
        std::cout << "Getting output port token " << std::endl;

        AftIndex outputPortIndex = std::strtoull(command_args.at(0).c_str(),NULL,0);

        AftNodeToken token  = getOuputPortToken(outputPortIndex);
        std::cout << "Token: " << token << std::endl;

    } else  if (command.compare("add-route") == 0) {
        if (command_args.size() != 3) {
            std::cout << "Please provide rtt token, route prefix and next-node-token" << std::endl;
            std::cout << "Example: add-route 10 103.30.60.1 100" << std::endl;
            return;
        }
        AftNodeToken rttToken = std::strtoull(command_args.at(0).c_str(), NULL, 0);
        AftNodeToken routeTragetToken = std::strtoull(command_args.at(2).c_str(), NULL, 0);

        addRoute(rttToken, command_args.at(1), routeTragetToken);

    } else  if ((command.compare("pkt") == 0) ||
                (command.compare("inject-l2-pkt") == 0)) {
        if (command_args.size() != 2) {
            std::cout << "Please provide sandbox-index and port-index" << std::endl;
            std::cout << "Example: inject-l2-pkt 0 0" << std::endl;
            return;
        }
        AftSandboxId  sandboxId = std::strtoull(command_args.at(0).c_str(), NULL, 0); // Sandbox ID
        AftIndex      portIndex = std::strtoull(command_args.at(1).c_str(), NULL, 0);    // Port Index

#define PKT_BUFF_SIZE 1500
#define ETHERNET_PACKET_BUF_SIZE 5000
        // Mac tap1 a2:24:4f:ce:94:b4
        // Src IP : 103.30.70.1
        // Dst IP : 103.30.70.3
        char ethernet_pkt[ETHERNET_PACKET_BUF_SIZE] =
            "a224 4fce 94b4 3226 0a2e fff1 0800 4500"
            "0054 dacc 4000 4001 059c 671e 4601 671e"
            "4603 0800 5cba 492b 3942 ee7c 5658 0000"
            "0000 0a30 0b00 0000 0000 1011 1213 1415"
            "1617 1819 1a1b 1c1d 1e1f 2021 2223 2425"
            "2627 2829 2a2b 2c2d 2e2f 3031 3233 3435"
            "3637";

        char pktBuff[PKT_BUFF_SIZE];
        int pktLen;

        pktLen = convertHexPktStrToPkt(ethernet_pkt,
                                       pktBuff,
                                       PKT_BUFF_SIZE);

        injectL2Packet(sandboxId, portIndex, (uint8_t *)pktBuff, pktLen);

    } else  if (command.compare("history") == 0) {
        std::cout << "Command history: " << std::endl;
        for(int t=0; t < _commandHistory.size(); ++t){
            std::cout << "\t" << _commandHistory.at(t) << std::endl;
        }
    } else  if (command.compare("clear-history") == 0) {
        _commandHistory.clear();
        std::cout << "Cleared history" << std::endl;

    } else  if ((command.compare("quit") == 0) ||
                (command.compare("exit") == 0)) {
        std::cout << "Exiting... " << std::endl;
        exit(1);
    } else {
        std::cout << "Invalid command '" << command << "'" << std::endl;
    }

    //
    // Add commands to command history -
    // skip commands which are not to be added.
    //
    if ((command.compare("history") != 0) &&
        (command.compare("clear-history") != 0)) {
        _commandHistory.push_back(command_str);
    }
}

//
// @fn
// cli
//
// @brief
// This example afi client's command line interface
//
// @param[in] void
// @return void
//

void
AfiClient::cli(void) {
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "\tAFI Client" << std::endl;
    std::cout << "\t==========" << std::endl;
    std::cout << "\tAFI server address: " << _afiServerAddr <<std::endl;
    std::cout << "\tAFI hostpath address: " << _afiHostpathAddr <<std::endl;
    std::cout << "\tEnter 'help' to display list of available commands" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    for (std::string command_str; std::cout << "____AFIClient____ > " &&
         std::getline(std::cin, command_str); )
    {
        if ((!command_str.empty()) &&
            (command_str.find_first_not_of(' ') != std::string::npos)) {
            handleCliCommand(command_str);
        }
    }
}

