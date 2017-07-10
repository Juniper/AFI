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
# Setup VMX
# Sandesh Kumar Sodhi
#

MY_DIR="$(dirname "$0")"
source "$MY_DIR/cmn.sh"

me=`basename "$0"`

#
# Start syslog server
#
rsyslog_status=`$PS -A | grep rsyslogd | wc -l`
if [ "$rsyslog_status" == "0" ]; then
    log_debug "starting rsyslog service"
    service rsyslog start
else
    log_debug "rsyslog service already running"
fi

#
# Untar vmx bundle image
#
if [ ! -d "$VMX_DIR" ]; then
    run_command $TAR xf $VMX_BUNDLE_IMAGE
fi
#
# Check
#
if [ ! -d "$VMX_DIR" ]; then
    log_error "VMX bundle untar error"
    exit 1
fi

#
# Chekc if VMX is already running
#
cd $VMX_DIR
vmx_status=`./vmx.sh --status | grep "Check if vMX is running" | grep No | wc -l`
if [ "$vmx_status" == "0" ]; then
    log_debug "VMX already running"
    exit 0
fi

$MOUNT -tsecurityfs securityfs /sys/kernel/security
$APPARMOR_PARSER -q -R /etc/apparmor.d/bin.ping          2> /dev/null
$APPARMOR_PARSER -q -R /etc/apparmor.d/usr.sbin.dnsmasq  2> /dev/null
$APPARMOR_PARSER -q -R /etc/apparmor.d/usr.sbin.libvirtd 2> /dev/null
$APPARMOR_PARSER -q -R /etc/apparmor.d/usr.sbin.rsyslogd 2> /dev/null
$APPARMOR_PARSER -q -R /etc/apparmor.d/usr.sbin.tcpdump  2> /dev/null
$CHOWN root:kvm /dev/kvm

#
# Create/configure tap interfaces before starting VMX
#
run_command $SCRIPTS_DIR/config_tap_interfaces.sh


#
# VMX configuration
#
$CP -f $CFG_FILES_DIR/vmx.conf $VMX_DIR/config/
$CP -f $CFG_FILES_DIR/vmx-junosdev.conf $VMX_DIR/config/

JUNOS_VMX_IMG=`ls -1 $VMX_DIR/images/ | grep qcow2`
echo JUNOS_VMX_IMG=$JUNOS_VMX_IMG

VPFC_IMG=`ls -1 $VMX_DIR/images/ | grep vFPC`
echo VPFC_IMG = $VPFC_IMG

$SED -i -e "s/REPLACE_WITH_JUNOS_VMX_IMAGE_NAME/$JUNOS_VMX_IMG/g" $VMX_DIR/config/vmx.conf
$SED -i -e "s/REPLACE_WITH_VFPC_IMAGE_NAME/$VPFC_IMG/g" $VMX_DIR/config/vmx.conf

#
# Increate sleep in vmx_system_setup_restart_libvirt() to fix following issue
#  Attempt to start libvirt-bin......................[OK]
#  Sleep 2 secs......................................[OK]
#  Check libvirt support for hugepages...............[Failed]
#  ls: cannot access /HugePage_vPFE/libvirt: No such file or directory
#  Error! Try restarting libvirt
#
$SED -i -e "s/Sleep 2 secs/Sleep 10 secs/g" $VMX_DIR/scripts/kvm/common/vmx_kvm_system_setup.sh
$SED -i -e "s/sleep 2/sleep 10/g" $VMX_DIR/scripts/kvm/common/vmx_kvm_system_setup.sh


#
# Start VMX
#
cd $VMX_DIR
./vmx.sh --start
if [ $? -ne 0 ]; then
   log_error "VMX did not start successfully"
  ./vmx.sh --stop
   log_prominent "VMX did not start successfully. Please resolve issue and try again"
   exit 1
fi

#
# Check if VMX is running
#
./vmx.sh --status

#
# Chekc if VMX started successfully
#
vmx_status=`./vmx.sh --status | grep "Check if vMX is running" | grep No | wc -l`
if [ "$vmx_status" == "0" ]; then
    ./vmx.sh --bind-dev
    if [ $? -ne 0 ]; then
       log_error "vmx.sh --bind-dev not successful"
      ./vmx.sh --stop
       exit 1
    fi

    ./vmx.sh --bind-check

    #
    # Show Linux bridges
    #
    $BRCTL show

    log_prominent "!!!!VMX started!!!!"

    log_prominent "VMX links configuration"
    run_command $SCRIPTS_DIR/config_vmx_links.sh

    log_prominent "Junos configuration"
    run_command $SCRIPTS_DIR/config_junos.exp

    log_prominent "Disabling flow cache"
    run_command $SCRIPTS_DIR/disable_flow_cache.exp

    #
    # Restart FPC 0 is needed to complete the
    # flow cache disabling
    log_prominent "Restarting FPC0"
    run_command $SCRIPTS_DIR/restart_fpc0.exp

    #
    # Give some more time FPC to restart
    #
    SLEEP_SECONDS=5
    log_prominent "Sleeping for $SLEEP_SECONDS more seconds"
    sleep $SLEEP_SECONDS

    log_prominent "Config VFP ext interface"
    run_command $SCRIPTS_DIR/config_vfp_ext_if.exp

    log_prominent "Config FPC0"
    run_command $SCRIPTS_DIR/config_fpc0.exp

    log_prominent "Configure br-int-vmx1 IP address"
    run_command $SCRIPTS_DIR/config_br-int-vmx1.sh

    log_prominent "Dump debug info"
    run_command $SCRIPTS_DIR/dump_debug_info.exp

    log_prominent "VMX Setup Complete! Happy AFI client programming!!!"
else
    log_prominent "VMX did not start. Please resolve the issue"
    exit 1
fi

#Normal script exit
exit 0
