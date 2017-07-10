#!/bin/bash
#
# Advanced Forwarding Interface : AFI client examples
#
# Created by Sandesh Kumar Sodhi, January 2017
# Copyright (c) [2017] Juniper Networks, Inc. All rights reserved.
#
# All rights reserved.
#
# Notice and Disclaimer: This code is licensed to you under the Apache
# License 2.0 (the "License"). You may not use this code except in compliance
# with the License. This code is not an official Juniper product. You can
# obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Third-Party Code: This code may depend on other components under separate
# copyright notice and license terms. Your use of the source code for those
# components is subject to the terms and conditions of the respective license
# as noted in the Third-Party source code file.
#

#
# Configure vmx_link* interfaces
#

MY_DIR="$(dirname "$0")"
source "$MY_DIR/cmn.sh"

me=`basename "$0"`


#
# Configure MAC addresses and IP addresses
# of vmx_link* interfaces
#

MAC_ADDR_PREFIX="32:26:0A:2E:BB:F"
COUNTER=0
while [  $COUNTER -lt 8 ]; do
    #
    # ip link set vmx_link0 address 32:26:0A:2E:BB:F0
    #
    run_command "ip link set vmx_link$COUNTER address $MAC_ADDR_PREFIX$COUNTER"
    let COUNTER+=1
done

IP_ADDR_PREFIX="103.30."
IP_ADDR_POSTFIX="0.2"
IP_ADDR_MASK="255.255.255.0"

COUNTER=0
while [  $COUNTER -lt 8 ]; do
    #
    # ifconfig vmx_link0 103.30.00.2 netmask 255.255.255.0 up
    #
    run_command  "ifconfig vmx_link$COUNTER $IP_ADDR_PREFIX$COUNTER$IP_ADDR_POSTFIX netmask $IP_ADDR_MASK up"
    let COUNTER+=1
done

#
# Static arp: Use when needed during testing
#
# arp -i vmx_link0 -s 103.30.00.1 32:26:0a:2e:bb:f0
# arp -i vmx_link1 -s 103.30.10.1 32:26:0a:2e:bb:f1
# ...
# ...
#
# Display arp entries
#
# arp -n
# arp -a -n
#
#
# Deleting arp entry
#
# arp -d 103.30.00.1
# arp -d 103.30.10.1
# ...
# ...
#

exit 0
