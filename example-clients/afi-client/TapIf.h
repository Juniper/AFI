//
// TapIf.h
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

#ifndef __TapIf__
#define __TapIf__

#include <boost/thread.hpp>

#include <linux/if_tun.h>

#include "jnx/Aft.h"
#include "Utils.h"

class TapIf
{
public:
    TapIf () {
        _tapFd = -1;
    }

    ~TapIf () {
        if (!initialized())
            return;

        _tapThread->join();
        delete(_tapThread);
        close(_tapFd);
    }

    int init(AftIndex portIndex, std::string ifName) {
        int tapFd;

        if (ifName.length() >= IFNAMSIZ) {
            std::cout << "Tap interface name is too long"<< std::endl;
            return -1;
        }

        /* initialize tun/tap interface */
        if ((tapFd = tapAlloc(ifName, IFF_TAP | IFF_NO_PI)) < 0) {
            std::cout << "Error connecting to tap interface " << ifName << std::endl;
            return -1;
        }

        _ifName = ifName;
        _portIndex = portIndex;
        _tapFd = tapFd;
        _tapThread = new boost::thread(boost::bind(&TapIf::ifRead, this));

        return _tapFd;
    }

    int tapAlloc(std::string ifName, int flags);
    int initialized(void);
    int ifRead(void);
    int ifWrite(int size, uint8_t *data);

private:
    std::string           _ifName;
    AftIndex              _portIndex;
    int                   _tapFd;
    boost::thread	  *_tapThread;
};

#endif // __TapIf__
