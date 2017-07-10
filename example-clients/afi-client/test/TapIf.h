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

#include "../Utils.h"

class TapIf
{
public:
    TapIf (const std::string &ifNameStr) {
        strncpy(_ifName, ifNameStr.c_str(), IFNAMSIZ);
        _ifName[IFNAMSIZ - 1] = '\0';
    }

    ~TapIf () {
    }

    int init(void) {
        int tapFd;
        int flags = IFF_TAP;

        /* initialize tun/tap interface */
        if ( (tapFd = tapAlloc(_ifName, flags | IFF_NO_PI)) < 0 ) {
            std::cout << "Error connecting to tap interface " << _ifName << std::endl;
            return -1;
        }
        _tapFd = tapFd;
        return _tapFd;
    }

    int tapAlloc(char *dev, int flags);
    int ifRead(void);

private:
    char _ifName[IFNAMSIZ];
    int  _tapFd;
};

#endif // __TapIf__
