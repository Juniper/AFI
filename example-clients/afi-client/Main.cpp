//
// Main.cpp
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
// Usage
//
int
displayUsage (void)
{
    std::cout << std::endl;
    std::cout << "\tUsage:"                                      << std::endl;
    std::cout << "\tafi-client <afi-server-address> <afi-hospath-address>";
    std::cout << std::endl;
    std::cout << "\t<afi-server-address>  : "                    << std::endl;
    std::cout << "\t    Address where AFI server is listening "  << std::endl;
    std::cout << "\t    for connection from AFI client/s"        << std::endl;
    std::cout << "\t<afi-hospath-address> : "                    << std::endl;
    std::cout << "\t    Address (UDP server) to send hostpath packets";
    std::cout << std::endl;
    std::cout << "\tExamples:"                                   << std::endl;
    std::cout << "\tafi-client 128.0.0.16:50051 128.0.0.16:9002" << std::endl;
    std::cout << std::endl;
}

//
// AFI client main
//
int
main(int argc, char *argv[])
{
    if (argc != 3) {
        displayUsage();
        exit(1);
    }
    std::string afiServerAddr(argv[1]);
    std::string afiHostpathAddr(argv[2]);

    boost::asio::io_service io_service;

    AfiClient afiClient(io_service,
                        afiServerAddr,            // AFI server address
                        afiHostpathAddr,          // AFI hostpath address
                        AFT_CLIENT_HOSTPATH_PORT, // jnx/AftPacket.h
                        true,                     // start hostpath server
                        false);                   // tracing

    //
    // Start this AFI client's
    // command line interface
    //
    afiClient.cli();
    return 0;
}
