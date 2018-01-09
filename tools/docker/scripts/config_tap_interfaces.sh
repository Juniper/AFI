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

NETNS_NAME="vrouter"
MAC_ADDR_VMX="32:26:0A:2E:AA:FX"
MAC_ADDR_PEER="32:26:0A:2E:CC:FX"
IPV4_ADDR_VMX="103.30.X0.1"
IPV4_ADDR_PEER="103.30.X0.3"
IPV4_PREFIXLEN="24"

COUNTER=0
while [  $COUNTER -lt 8 ]; do
    netns=$NETNS_NAME$COUNTER

    # Remove old network namespace and associated virtual interfaces
    run_command "ip netns del $netns 2>/dev/null || true"
    run_command "ip link del tap$COUNTER 2>/dev/null || true"
    let COUNTER+=1
done

COUNTER=0
while [  $COUNTER -lt 8 ]; do
    netns=$NETNS_NAME$COUNTER
    mac_vmx=${MAC_ADDR_VMX/X/$COUNTER}
    mac_peer=${MAC_ADDR_PEER/X/$COUNTER}
    ipv4_vmx=${IPV4_ADDR_VMX/X/$COUNTER}
    ipv4_peer=${IPV4_ADDR_PEER/X/$COUNTER}

    # Create new network namespace
    run_command "ip netns add $netns"
    run_command "ip netns exec $netns ip link set dev lo up"

    # Add veth interface pair
    run_command "ip link add veth$COUNTER type veth peer name veth"
    run_command "ip link set dev veth$COUNTER up"
    run_command "ip link set dev veth netns $netns up"

    # Add detached tap interface for punting control packets to Linux
    run_command "ip tuntap add tap$COUNTER mode tap"
    run_command "ip link set tap$COUNTER up"

    # Set MAC addresses
    run_command "ip link set tap$COUNTER address $mac_vmx"
    run_command "ip netns exec $netns ip link set veth address $mac_peer"

    # Set IP addresses
    run_command "ip addr add $ipv4_vmx/$IPV4_PREFIXLEN dev tap$COUNTER"
    run_command "ip netns exec $netns ip addr add $ipv4_peer/$IPV4_PREFIXLEN dev veth"

    # Add default route in the vrouters
    run_command "ip netns exec $netns ip route add default via $ipv4_vmx"

    let COUNTER+=1
done

exit 0
