
#include <omnetpp.h>
#include <stdio.h>
#include "instruction_m.h"
#include "ack_m.h"

//we have to add the instruction declaration (message kind) and ack is another message kind that complete the circle (acomplishment of instructions)

namespace BranchComp {

/**
 * Instruction Sink. Works as a writeback stage of a pipeline and it sends acks with id to the control module to deal with the onfly list and dependent elements.
 */
class Sink : public cSimpleModule
{
  private:
    //lifetimesignal is used to control the transit time of the instructions in the system
    simsignal_t lifetimeSignal;
    //Count is used to know how much instructions are executed in the processor.
    long count;
    double real_ipc;
    double dep_correction;
    bool dep_f_active;
    double sim_ipc;
    double sim_ipc_factor;
    double error;

    double startCPUclocks;
    double endCPUclocks;
    double cpu_time;

    //output vectors for statistics
    cOutVector ipc_vector;
    cOutVector ipc_vector_factor;

  protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module( Sink );

void Sink::initialize()
{
    //Signals are used to collect data from the simulation
    //It is the preferred way to obtain data.
    lifetimeSignal = registerSignal("lifetime");
    count=0;
    real_ipc=par("real_ipc");
    dep_correction=par("dep_correction");
    dep_f_active=par("dep_f_active");
    sim_ipc=sim_ipc_factor=0.0;
    error=0.0;
    startCPUclocks = clock();

    //necessary so the output vectors appear with the proper name ... by default white
    ipc_vector.setName("Sim_ipc_vector");
    ipc_vector_factor.setName("Sim_ipc_factor_vector");

}

void Sink::handleMessage(cMessage *msg)
{
    //As always, the first thing to implement is to perform the casting to the instruction type
    Instruction *a = dynamic_cast<Instruction *>(msg);
    if(a!=NULL){
        simtime_t lifetime = simTime() - a->getCreationTime();
     //   EV << "Received instruction  " << a->getType()<< " at time "<< a->getArrivalTime()<< "s" << endl;
        emit(lifetimeSignal, lifetime);
        //It does not matter the kind of instruction, we need to send back the id, or at least, notify that an instruction has finished.
        //we also put the type of instruction because their acks will require different treatments.
        Ack *ack= new Ack("ack");
        ack->setRecId(a->getId());
        ack->setType(a->getType());
        EV << "Enviado ack sobre instr " << a->getType() << "  con id    " << ack->getRecId() << endl;
        send( ack, "out" );
        //Now, we proceed to delete the instruction from the system
        delete a;
        //Increment the total number of instructions executed in the processor
        count++;
        //These parameters are used to know how good is our implementation
        //EV << "Total instructions executed    " << count << endl;
        //EV << "Total simulated time           " << simTime() << endl;
        //EV << "IPC:   " << count/simTime() << endl;
        if(!dep_f_active){
            //Without the factor
            //sim_ipc=count/simTime();
            //EV << "IPC_without factor:   " << sim_ipc << endl;
            //EV << "% of error:   " << 100*((sim_ipc-real_ipc)/real_ipc) << endl;
        }else{
            //With the factor
            //sim_ipc_factor=(count/simTime())/dep_correction;
            //EV << "Ifactor   " << dep_correction << endl;
            //EV << "IPC_with factor:   " << sim_ipc_factor << endl;
            //EV << "% of error:   " << 100*((sim_ipc_factor-real_ipc)/real_ipc) << endl;
        }
        EV << "real_ipc:   " << real_ipc << endl;


        sim_ipc=count/simTime();
        sim_ipc_factor=(count/simTime())/dep_correction;

        error=((sim_ipc-real_ipc)/real_ipc)*100;

        EV << "IPC   " << sim_ipc << endl;
        EV << "IPC_factor:   " << sim_ipc_factor << endl;
        EV << "% of error:   " << 100*((sim_ipc-real_ipc)/real_ipc) << endl;

        //Add the data to the vectors we want in statistics files --> output of the simulations
        ipc_vector.record(sim_ipc);
        ipc_vector_factor.record(sim_ipc_factor);
    }
}

void Sink::finish()
{

    endCPUclocks = clock();
    cpu_time=(endCPUclocks - startCPUclocks)/1000;

    // This function is called by OMNeT++ at the end of the simulation.
    //Statistics collection
    recordScalar("Sim_ipc", sim_ipc);
    recordScalar("Sim_ipc_factor", sim_ipc_factor);
    recordScalar("Real_ipc", real_ipc);
    recordScalar("CPU_time", cpu_time);
    recordScalar("Error_ipc", error);
}

}; //namespace

