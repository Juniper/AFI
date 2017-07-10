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
# Create and configure tap interfaces
#

MY_DIR="$(dirname "$0")"
source "$MY_DIR/cmn.sh"

me=`basename "$0"`

MAC_ADDR_PREFIX="32:26:0A:2E:CC:F"
COUNTER=0
while [  $COUNTER -lt 8 ]; do

    if $ETHTOOL tap$COUNTER | $GREP -q "Link detected"; then
        log_debug "interface tap$COUNTER exists"
    else
        run_command "ip tuntap add mode tap tap$COUNTER"
    fi

    #
    # ip link set tap0 address 32:26:0A:2E:CC:F0
    #
    run_command "ip link set tap$COUNTER address $MAC_ADDR_PREFIX$COUNTER"
    run_command "ip link set tap$COUNTER up"
    run_command "ifconfig tap$COUNTER promisc up"

    let COUNTER+=1
done

IP_ADDR_PREFIX="103.30."
IP_ADDR_POSTFIX="0.3"
IP_ADDR_MASK="255.255.255.0"

COUNTER=0
while [  $COUNTER -lt 8 ]; do
    #
    # ifconfig tap0 103.30.00.3 netmask 255.255.255.0 up
    #
    run_command "ifconfig tap$COUNTER $IP_ADDR_PREFIX$COUNTER$IP_ADDR_POSTFIX netmask $IP_ADDR_MASK up"
    run_command "ifconfig tap$COUNTER"
    let COUNTER+=1
done

exit 0

run_command "brctl show"

#for interface in tap0 tap1 tap2 tap3 tap4 tap5 tap6 tap7
#do
#    echo Deleting interface $interface
#    ip link delete $interface
#done


#for interface in tap0 tap1 tap2 tap3 tap4 tap5 tap6 tap7
#do
#    echo Clear all IP addresses configured on interface $interface
#    ip addr flush dev $interface
#done

exit 0
