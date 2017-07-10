#
# Makefile.inc -- Makefile to build Afi Client
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

AFI_VERSION = afi-1.0
AFI_LIB = ../../../$(AFI_VERSION)/lib
AFI_INCLUDE = ../../../$(AFI_VERSION)/include

CXX = g++
PROG = afi-client

SRCS = Main.cpp AfiClient.cpp Utils.cpp
OBJS=$(subst .cc,.o, $(subst .cpp,.o, $(SRCS)))

CXXFLAGS += -g -O0 -std=c++11

CPPFLAGS += -I. -I$(AFI_INCLUDE)/afi-transport -I$(AFI_INCLUDE)/aft-client

LDLIBS = -lafi-transport \
         -lprotobuf \
		 -lgrpc++ \
		 -laft-client \
		 -lprotoc \
		 -lprotobuf \
		 -lgrpc++ \
		 -lgrpc++_unsecure \
		 -lgrpc_unsecure \
		 -lboost_system \
		 -lpthread

all:    $(PROG)
	@echo $(PROG) compilation success!

$(PROG): $(OBJS)
	LIBRARY_PATH=$(AFI_LIB) $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(PROG) $(OBJS) $(LDLIBS)

clean:
	rm -f *.o $(PROG) ./.depend

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $^ >  ./.depend;

include .depend
