
#include <omnetpp.h>
#include "instruction_m.h"

namespace BranchComp {

/**
 * Generates instructions, with all its correspondent fiels like type
 * or id (we use the id of the messages to avoid another implementation)
 */

class Source : public cSimpleModule
{

  private:
    //This message is used to send a self-event to the module in order to start all the simulation
    cMessage *sendMessageEvent;
    //Pointer used to cntrol the instructions generated before send them to the control module
    Instruction *p;
    double prob, prob_branch,prob_load,prob_int, prob_store, x, e;
    long inst_id;
    long fetch;
    double icache_penalty,icache_miss_prob;
    bool icache_active,lock;
    double time;

    cMessage *icache;

//contructor and destructor methods, just definition because later they will be implemented
  public:
     Source();
     virtual ~Source();
//typical methods used in omnet to perform actions when a module is started and also to execute when the module receives an action
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Source);

Source::Source()
{
    sendMessageEvent = NULL;
}

Source::~Source()
{
    //This routine eliminates the messages owned by this module when the simulation finishes.
    //The cancelanddelete not works properly in some cases when we stop the simulation with messages onfly.
    //cancelAndDelete(sendMessageEvent);
    cOwnedObject *Del=NULL;
    int OwnedSize=this->defaultListSize();
    for(int i=0;i<OwnedSize;i++){
        Del=this->defaultListGet(0);
        this->drop(Del);
        delete Del;
    }
}

void Source::initialize()
{
    //declare the message to start everything
    sendMessageEvent = new cMessage("sendMessageEvent");
    //ScheduleAt is used to send a selfmessage to provoke the execution of the handleMessage function of the same module that has done the schedule
    scheduleAt(simTime(), sendMessageEvent);
    //initilize the variables
    prob=x=e=0.0;
    //DEFINE THE PROB OF INST AS A PARAMETER!!!!!!!!!!!!!!!!!!
    prob_branch=par("prob_branch");
    prob_load=par("prob_load");
    prob_store=par("prob_store");
    prob_int=par("prob_int");
    fetch=(long)par("fetch");

    icache_active=par("icache_active");
    icache_miss_prob=par("icache_miss_prob");
    icache_penalty=par("icache_penalty");

    time=0.0;
    inst_id=0;

    icache = new cMessage("icache");

}

void Source::handleMessage(cMessage *msg)
{
    //each time this module receives a request for more instructions (less the first time the others came from the control module) sends three instructions to the control module
    //we send three instructions because the A15 is a 3-way issue processor.
    //following the exponential distribution

    //This is the good probabilistic generator
    /*x = -log(1-prob_branch);
    EV << "   threshold   " << x << endl;
    for(int i=0;i<3;i++){
        e = exponential(1);
        EV << " value of the exponential  " << e << endl;
        if(e<x){
            p=new Instruction("Branch");
            p->setType("Branch");
        }else{
            p=new Instruction("Integer");
            p->setType("Integer");
        }
        send(p, "out");
    }*/

    double a =-log(1-icache_miss_prob/100);
    double b = exponential(1);

    //Generator with different types and areas to explore
    //to stablish the first cut on the area
    double x = -log(1-prob_load/100);
    //to stablish the second cut on the probabilities
    double x2 = -log(1-(prob_load/100+prob_int/100));
    //to stablish the second cut on the probabilities
    double x3 = -log(1-(prob_load/100+prob_int/100+prob_branch/100));
    //We use the exponential to determine where we are in the three possible areas
    double e;


    if(msg->isSelfMessage()){
        if(simTime()==time){
            lock=false;
            EV << "  UNLOCK  " << endl;
        }
        if(msg->isName("sendMessageEvent")){
            EV << "  NOOO  ICACHE MISS  " << endl;
            for(int i=0;i<fetch;i++){
                e = exponential(1);
                inst_id++;
                if(e<=x){
                    //We have a load
                    p=new Instruction("Load");
                    p->setType("Load");
                    p->setId(inst_id);
                    send(p, "out");
                    //the condition x<e<x2 cannot be used.
                }else if(e<=x2){
                    //We have an int
                    p=new Instruction("Integer");
                    p->setType("Integer");
                    p->setId(inst_id);

                    send(p, "out");
                }else if(e<=x3){
                    //We have a branch
                    p=new Instruction("Branch");
                    p->setType("Branch");
                    p->setId(inst_id);
                    send(p, "out");
                }else{
                    //we have a store
                    p=new Instruction("Store");
                    p->setType("Store");
                    p->setId(inst_id);
                    send(p, "out");
                }
            }
        }
    }else{
        if(!lock){
            if(b<=a && icache_active){
                //we have an icache miss
                EV << "  ICACHE MISS  " << endl;
                //to convert simtime_t to double
                time=SIMTIME_DBL(simTime());
                time= time + icache_penalty;
                lock=true;
                EV << "  LOCKED  " << endl;

                for(int i=0;i<fetch;i++){
                    e = exponential(1);
                    inst_id++;
                    if(e<=x){
                        //We have a load
                        p=new Instruction("Load");
                        p->setType("Load");
                        p->setId(inst_id);
                        sendDelayed(p,icache_penalty,"out");
                        //the condition x<e<x2 cannot be used.
                    }else if(e<=x2){
                        //We have an int
                        p=new Instruction("Integer");
                        p->setType("Integer");
                        p->setId(inst_id);
                        sendDelayed(p,icache_penalty,"out");
                    }else if(e<=x3){
                        //We have a branch
                        p=new Instruction("Branch");
                        p->setType("Branch");
                        p->setId(inst_id);
                        sendDelayed(p,icache_penalty,"out");
                    }else{
                        //we have a store
                        p=new Instruction("Store");
                        p->setType("Store");
                        p->setId(inst_id);
                        sendDelayed(p,icache_penalty,"out");
                    }
                }
            }else{
                //normal behavior
                EV << "  NOOO  ICACHE MISS  " << endl;
                for(int i=0;i<fetch;i++){
                    e = exponential(1);
                    inst_id++;
                    if(e<=x){
                        //We have a load
                        p=new Instruction("Load");
                        p->setType("Load");
                        p->setId(inst_id);
                        send(p, "out");
                        //the condition x<e<x2 cannot be used.
                    }else if(e<=x2){
                        //We have an int
                        p=new Instruction("Integer");
                        p->setType("Integer");
                        p->setId(inst_id);

                        send(p, "out");
                    }else if(e<=x3){
                        //We have a branch
                        p=new Instruction("Branch");
                        p->setType("Branch");
                        p->setId(inst_id);
                        send(p, "out");
                    }else{
                        //we have a store
                        p=new Instruction("Store");
                        p->setType("Store");
                        p->setId(inst_id);
                        send(p, "out");
                    }
                }
            }
            //EV << " Instruction sent  " << p->getType() << " id  " << p->getId()  << "  " <<  p->getId() << endl;
        }
        //important to delete the message received to avoid memory and scalability problems during simulations
        delete msg;
    }
    if(!icache->isScheduled()){
        scheduleAt(simTime()+1.0, icache);
    }
}
}; //namespace



