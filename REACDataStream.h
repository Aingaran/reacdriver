/*
 *  REACDataStream.h
 *  REAC
 *
 *  Created by Per Eckerdal on 15/03/2011.
 *  Copyright 2011 Per Eckerdal. All rights reserved.
 *
 */

#ifndef _REACDATASTREAM_H
#define _REACDATASTREAM_H

#include <libkern/OSTypes.h>
#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSArray.h>
#include <IOKit/IOReturn.h>

#include "REACConstants.h"

#define REACPacketHeader        com_pereckerdal_driver_REACPacketHeader
#define REACSplitUnit           com_pereckerdal_driver_REACSplitUnit
#define REACDataStream          com_pereckerdal_driver_REACDataStream
#define REACDeviceInfo          com_pereckerdal_driver_REACDeviceInfo

class  com_pereckerdal_driver_REACConnection;
struct com_pereckerdal_driver_EthernetHeader;

struct REACDeviceInfo {
    UInt8 addr[ETHER_ADDR_LEN];
    UInt32 in_channels;
    UInt32 out_channels;
};

/* REAC packet header */
struct REACPacketHeader {
    UInt8 counter[2];
    UInt8 type[2];
    UInt8 data[32];
    
    UInt16 getCounter() {
        UInt16 ret = counter[0];
        ret += ((UInt16) counter[1]) << 8;
        return ret;
    }
    void setCounter(UInt16 c) {
        counter[0] = c;
        counter[1] = c >> 8;
    }
};

// Handles the data stream part of a REAC stream (both input and output).
// Each REAC connection is supposed to have one of these objects.
//
// This class is not thread safe.
//
// TODO Protected constructor/assignment operator/destructor?
class REACDataStream : public OSObject {
    OSDeclareDefaultStructors(REACDataStream)
    
    struct MasterAnnouncePacket {
        UInt8 unknown1[9];
        UInt8 address[ETHER_ADDR_LEN];
        UInt8 inChannels;
        UInt8 outChannels;
        UInt8 unknown2[4];
    };
    
    enum REACStreamType {
        REAC_STREAM_FILLER = 0,
        REAC_STREAM_CONTROL = 1,
        REAC_STREAM_MASTER_ANNOUNCE = 2,
        REAC_STREAM_SPLIT_ANNOUNCE = 3
    };
    
    static const UInt8 STREAM_TYPE_IDENTIFIERS[][2];
    
    virtual bool initConnection(com_pereckerdal_driver_REACConnection *conn);
    
public:
    
    static REACDataStream *withConnection(com_pereckerdal_driver_REACConnection *conn);
    
public:

    // Return kIOReturnSuccess on success, kIOReturnAborted if no packet should be
    // sent, and anything else on error.
    //
    // This method will return kIOReturnAborted. Classes inheriting this class
    // should overload this method if they wish to do anything else. If this method
    // is overloaded, it must call this method.
    IOReturn processPacket(REACPacketHeader *packet, UInt32 dhostLen, UInt8 *dhost);
    
    // Returns true if the processing is finished and no further processing should
    // be done.
    //
    // If this method is overloaded, it should be called before any other processing.
    // If it returns true, the overloaded method must not do any processing and must
    // return true.
    bool gotPacket(const REACPacketHeader *packet, const com_pereckerdal_driver_EthernetHeader *header);
    
protected:
    
    com_pereckerdal_driver_REACConnection *connection;
    UInt64    lastAnnouncePacket; // The counter of the last announce counter packet
    UInt64    recievedPacketCounter;
    UInt64    counter;
        
    static bool checkChecksum(const REACPacketHeader *packet);
    static UInt8 applyChecksum(REACPacketHeader *packet);
};


#endif
