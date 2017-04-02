//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include "Fifo.h"

namespace BranchComp {

Define_Module(Fifo);

Fifo::Fifo()
{
    msgServiced = endServiceMsg = NULL;
}

Fifo::~Fifo()
{
    delete msgServiced;
    cancelAndDelete(endServiceMsg);
}

void Fifo::initialize()
{
    endServiceMsg = new cMessage("end-service");
    queue.setName("queue");
    qlenSignal = registerSignal("qlen");
    busySignal = registerSignal("busy");
    queueingTimeSignal = registerSignal("queueingTime");
    emit(qlenSignal, queue.length());
    emit(busySignal, 0);
    //variables declared in Fifo.h
    //capacity is the parameter defined by the user in omnet.ini  --> Remember that these parameters should be defined in .ned file. It has a default value
    capacity=par("capacity");
    //we use this variable as a control to change the state just for the module/s that are blocked. Initially no blocked
    blocked=false;
}

void Fifo::handleMessage(cMessage *msg)
{
    if (msg==endServiceMsg)
    {
        endService( msgServiced );
        if (queue.empty())
        {
            msgServiced = NULL;
            emit(busySignal, 0);

            //Let the picture like in the beginning. The if is to check id we are using the GUI, toa void useless calls
            if (ev.isGUI()){ getDisplayString().setTagArg("i",1,"");
            getParentModule()->getDisplayString().setTagArg("i",1,"");}
        }
        else
        {
            msgServiced = (cMessage *) queue.pop();
            emit(qlenSignal, queue.length());
            emit(queueingTimeSignal, simTime() - msgServiced->getTimestamp());
            simtime_t serviceTime = startService( msgServiced );
            scheduleAt( simTime()+serviceTime, endServiceMsg );
        }
    }
    else if (!msgServiced)
    {
        //enters here if we can process a message at the server
        arrival( msg );
        msgServiced = msg;
        emit(queueingTimeSignal, 0.0);
        simtime_t serviceTime = startService( msgServiced );
        scheduleAt( simTime()+serviceTime, endServiceMsg );
        emit(busySignal, 1);
        //Change the color of the picture while we are processing a packet
        if (ev.isGUI()){ getDisplayString().setTagArg("i",1,"cyan");
        getParentModule()->getDisplayString().setTagArg("i",1,"cyan");}
    }
    else
    {
        //Enters here if the server is busy and the message is going to be stored in the queue
        arrival( msg );
        queue.insert( msg );
        msg->setTimestamp();
        emit(qlenSignal, queue.length());

        //Here we check the current capacity of the queue. If it reaches the limit then we put the bool to true and send the msg to stop the control
        //the name of the msg is defined with the name of the instruction because the module fifo is generic and we need to differentiate between instruction types
        //Note that here we do not do the release of this block because once the control is blocked this part of the code will not be executed until new packets arrive.
        if(!blocked && queue.length()==capacity){
            //in this case we have to send the message to block the control
            blocked=true;
            EV << "  --------------------------------------     BLOCK ------------------------------------   "  << endl;
            std::string s="control_ON_";
            s.append(msg->getName());
            cMessage *trigger = new cMessage(s.data());
            send( trigger, "out1" );
        }
    }
}

simtime_t Fifo::startService(cMessage *msg)
{
    //EV << "Start service of " << msg->getName() << endl;

    //Each time we start the process of an instruction we check the current capacity to activate again the control module.
    //try to avoid in the condition <value to avoid a lot of msgs. if it is fixed we just send one packet to re-activate control
    //thanks to the blocked variable that belongs to each module, just the blocked module will send the reactivation msg
    if(blocked && queue.length()==0){
        //we have to send the message to unblock the control
        //By now we have put length=0 to test the system, but we can change this to adjust to the real behavior.
        blocked=false;
        EV << " --------------------   UNBLOCK   -----------------------------  "  << endl;
        std::string s="control_OFF_";
        s.append(msg->getName());
        //we have to put s.data to obtain the required input for the constructor method of cmsg.
        cMessage *trigger = new cMessage(s.data());
        send( trigger, "out1" );
    }

    return par("serviceTime");
}

void Fifo::endService(cMessage *msg)
{
    //EV << "Completed service of " << msg->getName() << endl;
    send( msg, "out" );
}

}; //namespace

