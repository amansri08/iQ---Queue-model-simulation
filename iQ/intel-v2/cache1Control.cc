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

#include "cache1Control.h"
#include "instruction_m.h"

namespace BranchComp {

Define_Module(Cache1Control);

void Cache1Control::initialize()
{
    missL1_ld=par("missL1_ld");
    missL1_st=par("missL1_st");
    exp=0.0;
    //divided by 100 because the miss rate is a percentage %
    x = -log(1-missL1_ld/100);
    y= -log(1-missL1_st/100);
}

void Cache1Control::handleMessage(cMessage *msg)
{
   // EV << " x   " << x << endl;
    if (msg->arrivedOn("in")||msg->arrivedOn("in1") ){
        //determine if the message has to go to L2 or to sink
        //EV << " missL1 " << missL1 << endl;
        //Perform the casting to the correspondent type, Instruction, to deal with it
        Instruction *z = dynamic_cast<Instruction *>(msg);

        //We separate the Load & Store to ensure to apply the same miss ratio on each kind. It could happen that one type has all the
        //misses because it is exponential distribution. By this way we ensure fairness. More loads, more miss for loads.
        //the miss ratio introduced is the combined for both type of instructions, so it works.
        if(strcmp(z->getType(),"Load")==0){
            specialSend(z, x);
        }else if(strcmp(z->getType(),"Store")==0){
            specialSend(z, y);
        }

    }else{
        //The case to control the queue length is not done now
        delete msg;
    }

}

void Cache1Control::specialSend(Instruction *p, double d){
    exp=exponential(1);
    //EV << " exp   " << exp << endl;
    if(exp<d){
        //instruction to L2 due to MISS in L1
    //    EV << " Instruction sent to L2   MISS  " << endl;
        send(p,"out1");
    }else{
        //instruction to sink due to HIT in L1
    //    EV << " Instruction sent to sink   HIT   " << endl;
        send(p,"out");
    }
}

} //namespace
