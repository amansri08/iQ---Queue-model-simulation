//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "Delay.h"

namespace BranchComp {

Define_Module(Delay);

void Delay::initialize()
{
    currentlyStored = 0;
    delayedJobsSignal = registerSignal("delayedJobs");
    emit(delayedJobsSignal, 0);
    WATCH(currentlyStored);
}

void Delay::handleMessage(cMessage *msg)
{
    //In this module we do not need a queue since the packets are delayed or sent. Hence, no storage is necessary
    if (!msg->isSelfMessage())
    {
        // if it is not a self-message, send it to ourselves with a delay
        currentlyStored++;
        double delay = par("delay").doubleValue();
        scheduleAt(simTime() + delay, msg);
    }
    else
    {
        // if it was a self message (ie. we have already delayed) so we send it out
        currentlyStored--;
        send(msg, "out");
    }
    emit(delayedJobsSignal, currentlyStored);

    //Change the color of the box if some packet is inside the module
    //The if is just to activate the visual stuff if we are using the GUI
    if (ev.isGUI()){
        getDisplayString().setTagArg("i",1, currentlyStored==0 ? "" : "cyan");
        getParentModule()->getDisplayString().setTagArg("i",1, currentlyStored==0 ? "" : "cyan");
    }
}

}; //namespace

