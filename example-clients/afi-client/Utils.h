//
// Utils.h
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

#ifndef __Utils__
#define __Utils__

#include <string>
#include <boost/atomic.hpp>

class spinlock {
private:
  typedef enum {Locked, Unlocked} LockState;
  boost::atomic<LockState> state_;

public:
  spinlock() : state_(Unlocked) {}

  void lock()
  {
    while (state_.exchange(Locked, boost::memory_order_acquire) == Locked) {
      /* busy-wait */
    }
  }
  void unlock()
  {
    state_.store(Unlocked, boost::memory_order_release);
  }
};

int convertHexStringToBinary(const char* source,
                             char* target_buff,
                             int   target_buff_len);
void getRidOfSpacesFromString(char* source);
extern int  convertHexPktStrToPkt(char* hex_pkt_str,
                                  char* pkt_buff,
                                  int pkt_buff_len);
extern void pktTrace(const std::string &ctx, char *pkt, int pkt_len);

#endif // __Utils__
