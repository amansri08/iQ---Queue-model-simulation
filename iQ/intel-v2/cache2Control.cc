//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "cache2Control.h"
#include "instruction_m.h"

namespace BranchComp {

Define_Module(Cache2Control);

void Cache2Control::initialize()
{
    missL2=par("missL2");
    exp=0.0;
    //divided by 100 because the miss rate is a percentage %
    x = -log(1-missL2/100);
}

void Cache2Control::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("in") ){
        //EV << " missL2 " << missL2 << endl;
        //determine if the message has to go to DRAM or to sink

        //Perform the casting to the correspondent type, Instruction, to deal with it
        Instruction *z = dynamic_cast<Instruction *>(msg);

        //Now the behavior for loads and for stores is completely different
        //the stores are sent to the sink, we supose write-back to the dram
        //the loads have the miss ratio, reason to apply the correspondent check
        if(strcmp(z->getType(),"Load")==0){
            specialSend(z);
        }else if(strcmp(z->getType(),"Store")==0){
       //     EV << " Store sent to sink   " << endl;
            send(z,"out");
        }
    }else{
        //The case to control the queue length is not done now
        delete msg;
    }
}

void Cache2Control::specialSend(Instruction *p){
    exp=exponential(1);
 //   EV << " exp   " << exp << endl;
    if(exp<x){
        //instruction to DRAM due to MISS in L2
  //      EV << " Load sent to DRAM   MISS  " << endl;
        send(p,"out1");
    }else{
        //instruction to sink due to HIT in L2
  //      EV << " Load sent to sink   HIT   " << endl;
        send(p,"out");
    }
}

} //namespace
