//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#ifndef __FIFO_H
#define __FIFO_H

#include <omnetpp.h>

namespace BranchComp {

/**
 * Single-server queue with a given service time.
 */
class Fifo : public cSimpleModule
{
  protected:
     cMessage *msgServiced;
     cMessage *endServiceMsg;
     cQueue queue;
     simsignal_t qlenSignal;
     simsignal_t busySignal;
     simsignal_t queueingTimeSignal;
     int capacity;
     bool blocked;

  public:
    Fifo();
    virtual ~Fifo();

   protected:
     virtual void initialize();
     virtual void handleMessage(cMessage *msg);

     virtual void arrival(cMessage *msg) {}
     virtual simtime_t startService(cMessage *msg);
     virtual void endService(cMessage *msg);


};

}; //namespace

#endif
