//
// Utils.cpp
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

#include <memory>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include "Utils.h"

//
// @fn
// getHex
//
// @brief
// Converts a char into bunary string
//
// @param[in]
//     buf Value to be converted to hex string
// @param[in]
//     buf_len Length of the buffer
// @param[in]
//     hex_ Pointer to space to put Hex string into
// @param[in]
//     hex_len Length of the hex string space
// @param[in]
//     num_col Number of columns in display hex string
// @param[out]
//     hex_ Contains the hex string
// @return  void
//

void
getHex (char *buf,
        int buf_len,
        char* hex_,
        int hex_len,
        int num_col)
{
    int i;
#define ONE_BYTE_HEX_STRING_SIZE   3
  unsigned int byte_no = 0;

  if (buf_len <= 0) {
      if (hex_len > 0) {
        hex_[0] = '\0';
      }
      return;
  }

  if(hex_len < ONE_BYTE_HEX_STRING_SIZE + 1)
  {
      return;
  }

  do {
         for (i = 0; ((i < num_col) && (buf_len > 0) && (hex_len > 0)); ++i )
         {
            snprintf(hex_, hex_len, "%02X ", buf[byte_no++] & 0xff);
            hex_ += ONE_BYTE_HEX_STRING_SIZE;
            hex_len -=ONE_BYTE_HEX_STRING_SIZE;
            buf_len--;
         }
         if (buf_len > 1)
         {
             snprintf(hex_, hex_len, "\n");
             hex_ += 1;
         }
  } while ((buf_len) > 0 && (hex_len > 0));

}

//
// @fn
// convertCharToInt
//
// @brief
// Converts a char to integer
//
// @param[in]
//     input_char Character to be converted to integer
// @return  integer corresponding to the input character
//

static int
convertCharToInt (char input_char)
{
  if(input_char >= '0' && input_char <= '9') {
    return input_char - '0';
  }
  if(input_char >= 'A' && input_char <= 'F') {
    return input_char - 'A' + 10;
  }
  if(input_char >= 'a' && input_char <= 'f') {
    return input_char - 'a' + 10;
  }
  std::cout<<"Error: Invalid "<< input_char <<std::endl;
  return 0;
}

//
// @fn
// convertHexStringToBinary
//
// @brief
// Converts hex string to corresponding binary string
//
// @param[in]
//     source Hex String - Null terminated with even nuber of [0-9a-f] characters
// @param[Out]
//     target_buff Target buffer where binary string will be stored
// @param[in]
//     target_buff_len Target buffer length
// @return  Nunber of bytes in output binary string
//

int
convertHexStringToBinary (const char* source,
                          char* target_buff,
                          int   target_buff_len)
{
  int num_bytes = 0;
  while(*source && source[1])
  {
    if (num_bytes >= target_buff_len) {
        return -1;
    }

    *(target_buff++) = convertCharToInt(*source)*16 + convertCharToInt(source[1]);
    source += 2;
    num_bytes++;
  }
  return num_bytes;
}

//
// @fn
// getRidOfSpacesFromString
//
// @brief
// Removed spaces from a string
//
// @param[in]
//     source String
// @return  void
//

void
getRidOfSpacesFromString (char* source)
{
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ') {
        i++;
    }
  }
  *i = 0;
}

//
// @fn
// convertHexPktStrToPkt
//
// @brief
// Coverts packet hex string to binary packet.
// Any spaces present in packet hex string will be
// removed before converting to binary packet.
//
// @param[in]
//     hex_pkt_str Packet Hex String -
// @param[Out]
//     pkt_buff Target packet buffer where binary will be stored
// @param[in]
//     pkt_buff_len Target packet buffer length
// @return  Packet length
//

int
convertHexPktStrToPkt (char* hex_pkt_str,
                       char* pkt_buff,
                       int   pkt_buff_len)
{
    int pkt_len;
    getRidOfSpacesFromString(hex_pkt_str);
    pkt_len = convertHexStringToBinary(hex_pkt_str, pkt_buff, pkt_buff_len);
    return pkt_len;
}

//
// @fn
// pktTrace
//
// @brief
// Displays packet hex
//
// @param[in]
//     ctx Context to be displayed along with packet
// @param[Out]
//     pkt Packet
// @param[in]
//     pkt_len Packet length
// @return  void
//

void
pktTrace(const std::string &ctx, char *pkt, int pkt_len)
{
#define DATA_HEX_STR_LEN 10000
    char      data_hex_str[DATA_HEX_STR_LEN];

    getHex(pkt, pkt_len, data_hex_str, DATA_HEX_STR_LEN, 16);
    std::cout << std::endl << ctx << ": " << std::endl;
    std::cout << data_hex_str << std::endl;
}

