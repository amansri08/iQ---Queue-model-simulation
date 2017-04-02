
#include <omnetpp.h>
#include "instruction_m.h"
#include "ack_m.h"
#include "id_m.h"

/*
 * This module simulates the fetch and the decode (simple mode currently). +
 * It has a buffer to store instructions ready to be executed, and it also controls the amount of instructions there
 *
 * ARM A15 can send up to three instructions per cycle. Before send it it has to check the possible dependencies that exists (determined by probability)
 *
 * In case the instruction is a branch, this method determine by probability if it is taken or not taken. If it is taken, just send the instruction to
 * the branch module and continue with the normal execution. If it is not taken, then we need to block the pipeline for four cycles. We perform this with
 * a delayed send, and an event scheduled with 4 cycles in advance.
 */

//This namespace has to be the same in all the modules and also in the package.ned!!!!!
namespace BranchComp {

class Control : public cSimpleModule
{
  private:
    int numblocks;
    simsignal_t numfunc;
    //control is to send one packet to each alu int or fp.
    //begin is used to start the simulation in this module the first time we receive instructions to be executed
    //sched is used to control the different possibilities to schedule an event of the clock
    bool control,begin,sched, branch,load,store;
    //nInst count the number of instructions sent to the functional units (all of them are counted)
    //x is used to control the number of elements inside the queue, but after three packets. It avoids to send three triggers each time the length is controlled
    //uniform is the result of a random distribution to determine to which element an instruction depends on.
    int nInst,x,uniform,capInt,capLoad;
    int outInt,outLoad;
    //variables used to control and block the sent of more instructions due to problems with capacities at the functional units. We have separated variables for each type
    //if block is true we cannot send instructions.
    bool blockInt,blockBranch,ooo, block;

    double distance;

    //to use the probability value to have dependencies or not
    //branch ones are to determinate if branch is taken or not taken
    double prob,random, random_branch;
    //ARM has 16 registers but two are used as stack pointer and program counter
    double nreg;
    int ooo_window;
    int x_for;
    int i;
    //queue is to store all the instructions that arrive to control, even to control that we always have three instr to send
    //dependency is to store the inst that are waiting through a dependency.

    double prob_dep_1,prob_dep_2,prob_dep_4,prob_dep_8,prob_dep_16,prob_dep_32,prob_dep_64,prob_else;

    double prob_branch_miss;
    double branch_miss_penalty;
    bool branch_miss;

    long old_inst;
    bool empty;

    cLongHistogram load_sent;
    cLongHistogram queue_length;
    cLongHistogram dep_length;
    cLongHistogram onfly_length;

    cOutVector queue_time;
    cOutVector onfly_time;
    cOutVector dep_time;

    cQueue queue, dependency, onfly;
    //Self messages used to start the module (Begin) and to simulate the clock cycles (Event)
    cMessage *Event;
    cMessage *Begin;

  //Destructor method (note the element ~), used to remove messages at the end of the simulation. It is completely necessary to avoid memory problems with large simulations
  public:
     virtual ~Control();

     //specialSend is used to be fair when we send packets to the integer units--> this method will change once the code is cleared
  protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void specialSend(Instruction *p, int b, bool h);
};

Define_Module( Control );

Control::~Control(){
    //Destructor method of the class. Necessary to delete the remain messages of the class Control.
    //We use cancel and delete because this Event msg can be scheduled in advanced (is a selfmessage), so if we just do delete we will have an error
   cancelAndDelete(Event);
}

void Control::initialize()
{
    numfunc = registerSignal("numfunc");

    nreg=par("n_registers");
    ooo_window=par("ooo_window");
    ooo=par("ooo");

    distance=par("distance");

    prob_branch_miss=par("prob_branch_miss");
    branch_miss_penalty=par("branch_miss_penalty");
    branch_miss=par("branch_miss");

    prob_dep_1=par("prob_dep_1");
    prob_dep_2=par("prob_dep_2");
    prob_dep_4=par("prob_dep_4");
    prob_dep_8=par("prob_dep_8");
    prob_dep_16=par("prob_dep_16");
    prob_dep_32=par("prob_dep_32");
    prob_dep_64=par("prob_dep_64");
    prob_else=par("prob_else");

    outInt=outLoad=0;
    //Initialize all the global variables used
    control=begin=true;
    //to block the sent of more instructions due to problems with capacity at the functional units. Initially nothing is blocked
    block=blockInt=blockBranch=false;
    sched=false;
    branch=load=store=false;
    capInt=capLoad=0;
    nInst=0;
    x=0;
    uniform=0;
    prob=0.0;
    random=0.0;
    random_branch=0.0;
    queue.setName("queue");
    dependency.setName("dependency");
    onfly.setName("onfly");
    Event = new cMessage("Event");
    Begin = new cMessage("Begin");

    old_inst=1;

    queue_length.setRange(0,168);
    dep_length.setRange(0,168);
    onfly_length.setRange(0,168);

    queue_time.setName("Queue_length_vector");
    onfly_time.setName("Onfly_length_vector");
    dep_time.setName("Dep_length_vector");

    empty=false;
}

void Control::handleMessage(cMessage *msg)
{
    /*
     * How to use a casting to a defined type of message. Best way is with dynamic cast and the null control
     * If you dont put the if it will provokes runtime errors
    Instruction *a = dynamic_cast<Instruction *>(msg);
    if(a!=NULL){
        a->setDep(12);
        EV << "ZZZZZZZ    "<< a->getId()<< "     ZZZZZZZZ     "<< a->getDep() << endl;
    }
     */
    //EV << "ZZZZZZZ  "<< nreg << endl;

    if (msg->arrivedOn("in") ){
        //Instructions arrived from the generator are processed in this if sentence (they came through gate "in")
        //All the msg are stored in the buffer of the controller, to be delivered in the future
        //First thing is to perform a casting from message to instruction type NOTE: remember to put the null control for the pointer to avoid runtime errors
        Instruction *arrived = dynamic_cast<Instruction *>(msg);
        if (arrived!=NULL){
           //EV << "Intruction type recevied is  "<< arrived->getType() << endl;
            //We do not need to check anything, just stores the instructions in the queue

            //in case the processor has no instructions, the first to arrive set the value for old_inst
            if(empty){
                empty=false;
                old_inst=arrived->getId();
            }

            queue.insert(arrived);
            //EV << "Instructions in queue of control: "<< queue.length() << endl;
            //Var x is used to just check the instructions queue each three packets
            //begin just to activate the functions of the module after the reception of the first three instructions
            x++;
            if(begin){
                scheduleAt(simTime(), Begin);
                begin=false;
            }else if(!Event->isScheduled()){
                scheduleAt(simTime()+1.0, Event);
            }

            //if we put this here the ipc grows, and behaves as expected ... more penalty less ipc, optimal with the icache desactivated
            //this can annulate the penalty of the branch by schedulling event always in the next cycle --> solution: when branch miss, remove event and reschedule with the penalty
            /*
             * else if(!Event->isScheduled()){
                scheduleAt(simTime()+1.0, Event);
            }
             */



            //To check the length. Since the ARM can send up to three instructions per cycle, we need to have this number of instructions in the instruction queue
/*            if(queue.length()<3 && x==3 ){
                //The buffer has less than three elements so it sends trigger to full its buffer
                cMessage *trigger = new cMessage("trigger");
                send( trigger, "out" );
                x=0;
            }*/
        }
    }else if(msg->isName("ack")){
        //This block handles the reception of acks from the sink to know when an instruction has finished.
        //Depending on the instruction type, different things need to be resolved.

        //First decrement all the number of instructions that are running on the processor. It is like the commitment stage. Even the instructions that cannot provoke a dependence
        //have to do that --> keep control of what is happening in the processor
        nInst--;

        //d is used to handle the possible instruction on the dependency list (used to store the temporary instruction read from dependency list)
        Instruction *d;
        //Perform the casting to the correspondent type -- ack has its own instruction type (by now it is really similar to the instruction)
        Ack *ak = dynamic_cast<Ack *>(msg);
        //if the instr is an integer then we need to modify onfly list and check the id of the dependent element which is attached to this ack
        //if ack is branch, we just need to decrement the nInst and thats all--> Remember that other instructions cannot depend on a branch
        if (ak!=NULL){
            //to compare strings we have to use the function strcmp. If the result is 0 then both strings are equal
            //if((strcmp(ak->getType(),"Integer")==0)||(strcmp(ak->getType(),"Load")==0)||(strcmp(ak->getType(),"Branch")==0)||(strcmp(ak->getType(),"Store")==0)){
            //since we are using EV, in the release version the messages will not appear.
            //            EV<< "Instructions on the fly: " << nInst << endl;
            EV<< "ACK RECIBIDO SOBRE INST CON ID   "<< ak->getRecId() << endl;


            //only the inst types that contained in the onfly list can access the for to delete id from that list.
            if((strcmp(ak->getType(),"Integer")==0)||(strcmp(ak->getType(),"Load")==0)){
                //remove the id received of the onfly list. This will avoid future dependencies on a finished instruction
                Id *rem;
                for(int r=0; r<onfly.length();r++){
                    //get returns a copy, but not extract the object from the queue
                    rem=(Id *)onfly.get(r);
                    if(rem->getValue()==ak->getRecId()){
                        //Id received matches with one in the onlfy list, so we proceed to remove it
                        //                    EV<< "Eliminado del vector onfly el id   "<< ak->getRecId() << endl;
                        onfly.remove(rem);
                        //since we found the desired element and we erased it from the onfly list, we can break the loop
                        break;
                    }
                }
                //Really important to delete the objects and messages we will not use to keep the memory usage linear.
                delete rem;
            }



            //All the insts have to check the oldest. In theory branch and store will not, but until now they are also involved
            EV<< "  old inst before change   "<< old_inst  << " onfly " <<  onfly.length() << " dep " << dependency.length() << endl;
            //once we removed the arrived id from onfly, lets check which is the oldest oldest instruction now
            if(ak->getRecId()==old_inst){
                //set the new possible old to the oldest element in onfly
                //each time I want to search for a new minimum I set the value to the max number for a long
                old_inst=2147483646;

                if(!onfly.isEmpty()){
                    Id *check;
                    //check=(Id *)onfly.get(0);
                    //value is the inst_id contained in the Id object
                    //old_inst=check->getValue();
                    //check oldest in onfly queue
                    for(int r=0; r<onfly.length();r++){
                        check=(Id *)onfly.get(r);
                        if(check->getValue()<old_inst){
                            old_inst=check->getValue();
                        }
                    }
                    EV<< "  111   "  << endl;
                    //delete check;
                }
                if(!dependency.isEmpty()){
                    Instruction *checkd;
                    //check new oldest in dep queue
                    for (int lk = 0; lk < dependency.length(); lk++) {
                        checkd= (Instruction *)dependency.get(lk);
                        //EV<<   checkd->getId()  << endl;
                        if(checkd->getId()<old_inst){
                            old_inst=checkd->getId();
                        }
                    }
                    EV<< "  222   "  << endl;
                    //delete checkd;
                }

                if(onfly.isEmpty()&&dependency.isEmpty()){
                    //in case both queues are empty, select the first instruction ready to be sent
                    Instruction *checkq;
                    if(!queue.isEmpty()){
                        checkq=(Instruction *)queue.get(0);
                        old_inst=checkq->getId();
                        EV<< "  333333   "  << endl;
                    }else{
                        empty=true;
                        EV<< "  4444444   "  << endl;
                    }
                    //EV<< "  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaAAAAAAAA   "  << endl;
                }
            }
            EV<< "  OLD inst after change   " << old_inst  << " onfly " <<  onfly.length() << " dep " << dependency.length() << endl;


            //Now we check the possible dependent elements in the correspondent list.
            //All intructions types has to access this bucle. Through dependencies is possible to depend on any inst type, while from onfly just from int and load.
            if(!dependency.isEmpty()){
                //take the dependent instruction to check if its dependency matches the received ack
                Instruction *g;
                //remember that is necessary to deal with a copy of the instruction to not modify the value contained inside the dependency list
                //if we only read the value is not necessary to dup, but if we modify is a MUST
                //we also need a duplicate because if we are going to store or sent the instruction, the original is owned by the queue dependency list
                x_for=0;
                //this for prints the entire dependency queue, from the oldest element to the young one.
                /*     for (i = 0; i < dependency.length(); i++) {
                        g= (Instruction *)dependency.get(i);
                        EV<< "  AAAAAAAAAAAAAAAAA   "<< g->getId()  <<  g->getDep()  << endl;
                    }  */
                int length=dependency.length();
                for (i = 0; i <= length; i++) {
                    if(!dependency.isEmpty()&&x_for<dependency.length()){
                        //read the first element of the queue, but keep it there, just read a copy of it
                        //g = (Instruction *) dependency.get(x_for);
                        g= (Instruction *)dependency.get(x_for)->dup();
                    }else{
                        break;
                    }
                    //g= (Instruction *)dependency.get(0)->dup();
                    if(ak->getRecId()== g->getDep()){
                        d=(Instruction *)dependency.remove((Instruction *)dependency.get(x_for));
                        //d=(Instruction *)dependency.pop();
                        EV<< "Unlock dependent element with id "<< d->getId()  << endl;
                        //                        EV<< "dep length   " << dependency.length() << endl;
                        //                         EV<< "x_for   " << x_for << endl;
                        //d=(Instruction *)dependency.pop();
                        if((strcmp(d->getType(),"Integer")==0)){
                            //Deal with int instruction
                            //insert the id of this new instruction to the onfly list and send it to the correspondent functional unit
                            Id *id=new Id();
                            id->setValue(d->getId());
                            onfly.insert(id);
                            //                             EV<< "Block inst enviada  " << d->getType()<< " id "<< d->getId() << endl;
                            specialSend(d, outInt,true);
                            capInt++;
                            //send(d,"out3");
                        }else if((strcmp(d->getType(),"Branch")==0)){
                            //Deal with branch instruction
                            //remember that nobody can depend on a branch instruction
                            //            EV<< "Instruccion blocked enviada  " << d->getType() << endl;
                            EV<< "Block inst branch enviada  " << d->getType()<< " id "<< d->getId() << endl;

                            double x = -log(1-prob_branch_miss/100);
                            double e = exponential(1);
                            EV<< " prob of misspredicted "<< x << endl;
                            EV<< " random exponential number "<< e << endl;
                            if(e>=x || !branch_miss){
                                //branch correctly predicted, or we do not want to take into account the miss-prediction rate(always enter here
                                EV<< "Correctly predicted branch  "  << endl;
                                specialSend(d, outInt,true);
                                capInt++;
                                //sched=true;

                            }else{
                                //branch misspredicted
                                EV<< "MISSpredicted branch  "  << endl;
                                //false to indicate we need a delayed send
                                specialSend(d,outInt,false);
                                capInt++;
                                //Schedule the event to four cycles in advance to simulate the effect of a misspredicted branch
                                if(!Event->isScheduled()){
                                    scheduleAt(simTime()+branch_miss_penalty+1.0, Event);
                                }else{
                                    cancelEvent(Event);
                                    scheduleAt(simTime()+branch_miss_penalty+1.0, Event);

                                }
                                //maybe use removeObject("Event") and instead of scheduleat use sendDelay trigger
                                //change the ask trigger at the end??? just in case of correct predicted branch. or do it with a bool

                                //Flush the queue of ready instructions due to the misspredicted branch
                                queue.clear();
                                //EV<< "check the flush  " << queue.length() << endl;

                            }
                        }else if((strcmp(d->getType(),"Load")==0)){
                            //Deal with load instruction
                            //insert the id of this new instruction to the onfly list and send it to the correspondent functional unit
                            Id *id=new Id();
                            id->setValue(d->getId());
                            onfly.insert(id);
                            //       EV<< "Instruccion blocked enviada  " << d->getType() << endl;
                            //       EV<< "Añadido al vector onfly el id  " << id->getValue() << endl;
                            nInst++;
                            //                            EV<< "Block inst enviada  " << d->getType()<< " id "<< d->getId() << endl;
                            send(d, "out4");
                            load_sent.collect(1);
                        }else if((strcmp(d->getType(),"Store")==0)){
                            //Deal with load instruction
                            //insert the id of this new instruction to the onfly list and send it to the correspondent functional unit
                            //        EV<< "Instruccion blocked enviada  " << d->getType() << endl;
                            nInst++;
                            //                            EV<< "Block inst enviada  " << d->getType()<< " id "<< d->getId() << endl;
                            send(d, "out5");
                        }
                        //important to activate again the clock
                        if(!Event->isScheduled()){
                            scheduleAt(simTime()+1.0, Event);
                        }
                    }else{
                        //if the element is not id=ack, we will check next element in the queue
                        x_for++;
                    }
                    //Always remember to delete the messages we are not using to free memory. The messages sent cannot be deleted because now they belong to another module
                    delete g;
                }
                //LAST MODIFICATION TO AVOID NO MORE EVENTS!!!!!!!!!!!!!!!
                //if we do not add this, the simulator runs out of ready instructions and it stops the execution
/*
                if (queue.length() <= (4)) {
                    //The buffer has less than three elements so it sends trigger to full its buffer
                    cMessage *trigger = new cMessage("trigger");
                    send(trigger, "out");
                    //                       EV<< "ASK FOR MORE INSTRUCTIONS --- " << endl;
                }
*/
            }
            //In case it is a branch, we do not need to do anything. Here we show a message to indicate that case
        }/*else if(strcmp(ak->getType(),"Branch")==0){
                //Do not do anything.
                //Branch only sends an ack to eleminate it from the nInst variable
               EV<< "Processed an ack of a branch instruction" << endl;
            }else if(strcmp(ak->getType(),"Store")==0){
                //Do not do anything.
                //Branch only sends an ack to eleminate it from the nInst variable
                EV<< "Processed an ack of a store instruction" << endl;
            }*/
        //}
        //Finally, delete the message that provokes the execution of this piece of code.
        delete ak;

        if(!Event->isScheduled()){
            scheduleAt(simTime()+1.0, Event);
        }

        queue_length.collect(queue.length());
        onfly_length.collect(onfly.length());
        dep_length.collect(dependency.length());

        queue_time.record(queue.length());
        onfly_time.record(onfly.length());
        dep_time.record(dependency.length());


    //These msgs are used to control when the capacity of a certain module is full and we need to stop or vice versa.
    //we have to use different packets for on and off due to if both get blocked at the same cycle one will block and the other will unblock, take care
    //dependencies are not necessary because the dependency is more reestrictive
    }else if(msg->isName("control_ON_Integer")){
        //received trigger from one of the integer functional units to block the sent
        blockInt=false;
        delete msg;
    }else if(msg->isName("control_OFF_Integer")){
        //received trigger from one of the integer functional units to unblock the sent
        blockInt=false;
        //once we unblock the sent of more instructions we need to reschedule the event because it happened before with no effect. Now, the system will start again
        if(!Event->isScheduled()){
            scheduleAt(simTime()+1.0, Event);
        }
        delete msg;
    }else if(msg->isName("control_ON_Branch")){
        //received trigger from the branch functional unit to block the sent
        blockBranch=false;
        delete msg;
    }else if(msg->isName("control_OFF_Branch")){
        //received trigger from the branch functional unit to unblock the sent
        blockBranch=false;
        //once we unblock the sent of more instructions we need to reschedule the event because it happened before with no effect. Now, the system will start again
        if(!Event->isScheduled()){
            scheduleAt(simTime()+1.0, Event);
        }
        delete msg;
    }else if ( msg->isSelfMessage() ){
        //This block of code is executed when the module receives a self-message. There are two possible self-messages, the first is begin when we start running the module
        //The second is Event, which is to simulate the clock events of a real processor.

        //This processor is in-order, so when an instruction is blocked due to dependencies we do not send more instructions until this dependency is solved


        //by now we leave the simple case, that none of the functional units is blocked. Later we can add more detail using this bools to block specific instructions
        //but allow another type. possibility-->put the bools at the if before send --> 18-05 -- since int=1 cycle never saturate!!!
        if(!blockInt && !blockBranch){

            //First we check if dependency is empty. This check is for security reasons. If we have scheduled by mistake an event and there is an element blocking, it will do nothing.
            if (!dependency.isEmpty()&&!ooo) {
                //Check the is there is any instr waiting for dependencies. If there is one, just do not do anything as a response to the triggers and the event
                //if the instruction is a sink, check the id to release the dependency

                //if it is an in-order processor you should not do anything until the dependency queue is empty

            } else {

                //We need to check if there is enough instructions in the queue
                //MODIFICATION: MAKE A CONTROL VARIABLE TO JUST SEND ONE TRIGGER FOR EACH GROUP OF INSTRUCTIONS. OR DO THE CHECK AND SEND THE TRIGGER WITH A REDUCED THRESHOLD
                //EV<< "Instructions in queue of control: "<< queue.length() << endl;

                branch=load=store=false;
                capInt=capLoad=0;
                x_for=0;
                block=false;
    //            EV<< " Inst queue at the control module  " << queue.length() << endl;
                //Since there is no pending instructions we can send new ones (up to four per cycle)
                Instruction *z;
                //To control the number of instructions per cycle
                //nunca puede  haber mas de cuatro instrucciones en queue, por fetch ... i<queue.length()
                for (i = 0; i < (ooo_window-dependency.length()); i++) {
                    //First, read an instruction from the ready queue.
                    if(!queue.isEmpty()&&x_for<queue.length()){
                        //read the first element of the queue, but keep it there, just read a copy of it
                        z = (Instruction *) queue.get(x_for);

                        //EV<< "aaaaaaaaaaaaaa" << x_for << endl;
                    }else{
                        EV<< "No insts to sent " << endl;
                        break;
                    }


                    //in case there is a pending inst at a distance of ooo_window, we block the processor until that inst is resolved
                    if((z->getId()-old_inst)>=distance){
                        EV<< "BBBBBBBBBBBBBBBBBB blocked due to old_inst: "<< z->getId()<<"-"<<old_inst  <<" mayor " << ooo_window << "  BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << endl;
                        //next cycle we need to continue with the activity of the processor
                        sched=true;
                        block=true;
                        //variable to control the number of blocked times of the processor
                        numblocks++;
                        //do not send any instruction that cycle
                        break;
                    }


                    //Now it is time to check dependencies before send and instruction
                    //Generate a random double number between 0.0 and 1.0 --> TAKE CARE - they are not really random and seems to be very similar if we generate it close in time

                    //random= genk_dblrand(0);
                    //By now this prob variable is the prob of dependency
                    //since branch instructions are counted with nInst maybe it is better to use onfly.lenght() --> not so bad because branch occupy a register
                    //prob = ((nInst / nreg) + (nInst / nreg))- ((nInst / nreg) * (nInst / nreg));
                    //prob = ((onfly.length() / nreg) + (onfly.length() / nreg))- ((onfly.length() / nreg) * (onfly.length() / nreg));
                    //if we want to try the model with no depedencies (each cycle we can send instructions) put prob=0.0
                    //prob=0.3;
                   /* if((strcmp(z->getType(),"Integer")==0)||(strcmp(z->getType(),"Store")==0)){
                        prob=2*onfly.length() / nreg;
                    }else if((strcmp(z->getType(),"Load")==0)||(strcmp(z->getType(),"Branch")==0)){
                        prob=onfly.length() / nreg;
                    }*/
                    //show the entire onfly list
                    /*for(int hj=0;hj<onfly.length();hj++){
                        Id *sd = (Id *)onfly.get(hj)->dup();
                        EV << "onfly-LIST   "<< sd->getValue()  << endl;
                    }*/
                    random = dblrand();
                    random_branch= dblrand();
                    //To determine the dependency probability we have to take into account the onfly, and also the dependency
                    int inst_fly=onfly.length()+dependency.length();
                    //int inst_fly=onfly.length();
                    if(inst_fly<=1){
                        prob= prob_dep_1;
                    }else if (inst_fly<=2){
                        prob= prob_dep_2;
                    }else if (inst_fly<=4){
                        prob= prob_dep_4;
                    }else if (inst_fly<=8){
                        prob= prob_dep_8;
                    }else if (inst_fly<=16){
                        prob= prob_dep_16;
                    }else if (inst_fly<=32){
                        prob= prob_dep_32;
                    }else if(inst_fly<=64){
                        prob= prob_dep_64;
                    }else{
                        //by now it is ok, but in the future more accuracy is required.
                        //VERY IMPORTANT ... IF WE PUT 1.0, THE PROCESSOR GETS BLOCKED DUE TO DEPENDENCY. IPC GROWS A LOT BECAUSE IT HAS A DEP QUEUE OF OOO_WINDOW TO SEND
                        prob=prob_else;
                        //EV<< " AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA " << endl;
                    }

                    EV<< " random number "<< random << endl;
                    EV<< " dep probability  "<< prob << endl;
                    if(random>=prob||(inst_fly)==0){
                        //if we enter here it means that the instruction does not have dependencies
                        //verify that we do not sent the 3int or 2int+1branch -- branch es true si se ha enviado el branch -- capInt counts ints
                        if((strcmp(z->getType(),"Integer")==0)&&(capInt<3)){
                            //Deal with int instruction
                            z=(Instruction *) queue.remove(z);
                            //insert the id of this new instruction to the onfly list and send it to the correspondent functional unit
                            Id *id=new Id();
                            id->setValue(z->getId());
                            onfly.insert(id);
                            EV<< "Instruccion enviada  " << z->getType()<< " id "<< z->getId() << endl;
                        //    EV<< "Añadido al vector onfly el id  " << id->getValue() << endl;
                            specialSend(z, outInt,true);
                            //capInt++;
                            sched=true;
                        }else if((strcmp(z->getType(),"Branch")==0)&&(capInt<3 && !branch)){
                            //Deal with branch instruction
                            //remember that nobody can depend on a branch instruction
                            //branch=true;
                            z=(Instruction *) queue.remove(z);
                            EV<< "Instruccion enviada  " << z->getType()<< " id "<< z->getId() << endl;

                            double x = -log(1-prob_branch_miss/100);
                            double e = exponential(1);
                            EV<< " prob of misspredicted "<< x << endl;
                            EV<< " random exponential number "<< e << endl;
                            if(e>=x || !branch_miss){
                                //branch correctly predicted, or we do not want to take into account the miss-prediction rate(always enter here
                                EV<< "Correctly predicted branch  "  << endl;
                                specialSend(z, outInt,true);
                                //capInt++;
                                sched=true;

                            }else{
                                //branch misspredicted
                                EV<< "MISSpredicted branch  "  << endl;
                                //false to indicate we need a delayed send
                                specialSend(z,outInt,false);
                                //capInt++;
                                //Schedule the event to four cycles in advance to simulate the effect of a misspredicted branch
                                if(!Event->isScheduled()){
                                    scheduleAt(simTime()+branch_miss_penalty+1.0, Event);
                                }else{
                                    cancelEvent(Event);
                                    scheduleAt(simTime()+branch_miss_penalty+1.0, Event);
                                }

                                //Flush the queue of ready instructions due to the misspredicted branch
                                queue.clear();
                                //EV<< "check the flush  " << queue.length() << endl;

                                ////////////////////break;
                            }

                        }else if((strcmp(z->getType(),"Load")==0)&&(capLoad<2)){
                            //Deal with load instruction
                            //load=true;
                            z=(Instruction *) queue.remove(z);
                            //insert the id of this new instruction to the onfly list and send it to the correspondent functional unit
                            Id *id=new Id();
                            id->setValue(z->getId());
                            onfly.insert(id);
                            EV<< "Instruccion enviada  " << z->getType()<< " id "<< z->getId() << endl;
                          //  EV<< "Añadido al vector onfly el id  " << id->getValue() << endl;
                            nInst++;
                            if(capLoad==0){
                                send(z, "out4");
                            }else{
                                send(z, "out5");
                            }
                            //capLoad++;

                            sched=true;
                        }else if((strcmp(z->getType(),"Store")==0)&&(capLoad<2 && !store)){
                            //Deal with load instruction
                            //store=true;
                            z=(Instruction *) queue.remove(z);
                            //insert the id of this new instruction to the onfly list and send it to the correspondent functional unit
                            EV<< "Instruccion enviada  " << z->getType() << " id "<< z->getId() << endl;
                            nInst++;
                            send(z, "out5");
                            sched=true;
                        }else{
                            //if functional units are full we break the for to proceed to next cycle
                            EV<< " we cannot sent more instructions this cycle due to funct units limitation  "  << endl;
                            //delete z;
                            //break;
                            x_for++;
                            sched=true;
                        }
                        //load_sent.collect(capLoad);
                    }else if(random<prob){
                        //If we enter here it means that the instruction depends on a previous one, so we store it in dependency queue
                        //Since we have dependency, we need to determine to which inst on_fly is related the dependent one





                        //we pop the inst because it is going to be stored in another queue. Remember ownership in omnet, it cannot be in two places
                        z=(Instruction *) queue.pop();

                        //here the branch&stores instr does not add more restrictions, it can depends on others but others cannot depend on them
                        //it can depend on others and as a consequence it will be blocked until it receives the correct ack

                        //The instruction on which we depend is chosen randomly.
                        //To achieve it we use a uniform distribution from 0 to the onfly.length()-1, both included (important the -1).
                        //This number will be the index to the onfly list-->gives id
                        //use an uniform distribuion to determine from which instruction we will depend
                        //[a,b] --> inclusive range
                        //uniform = intuniform(0,onfly.length()-1,0);

                        //inst_fly=onfly.length()+dependency.length();
                        //the term prob includes the check with inst_fly instructions, not necessary to do it here
                        int distance=0;
                        if(random<=prob_dep_1){
                            distance=1;
                        }else if (random<=prob_dep_2){
                            distance=2;
                        }else if (random<=prob_dep_4){
                            if(inst_fly>=4){
                                //if there we are on the limit of the interval, or bigger we have to chose a distance among this entire interval. REMEMBER, BOTH INCLUDED
                                distance=intuniform(3,4,0);
                            }else{
                                //in case we have less elements than the full interval, the limit is on inst_fly, and the begining is the first posible distance in this interval
                                //if we enter here, the check for dependencies random<prob guarantees that we can be in a specific interval
                                distance=intuniform(3,inst_fly,0);
                            }
                        }else if (random<=prob_dep_8){
                            if(inst_fly>=8){
                                distance=intuniform(5,8,0);
                            }else{
                                distance=intuniform(5,inst_fly,0);
                            }
                        }else if (random<=prob_dep_16){
                            if(inst_fly>=16){
                                distance=intuniform(9,16,0);
                            }else{
                                distance=intuniform(9,inst_fly,0);
                            }
                        }else if (random<=prob_dep_32){
                            if(inst_fly>=32){
                                distance=intuniform(17,32,0);
                            }else{
                                distance=intuniform(17,inst_fly,0);
                            }
                        }else if(random<=prob_dep_64){
                            if(inst_fly>=64){
                                distance=intuniform(33,64,0);
                            }else{
                                distance=intuniform(33,inst_fly,0);
                            }
                        }else{
                            //by now it is ok, but in the future more accuracy is required.
                            distance=inst_fly;
                        }
                       // EV << "Distancia a la que dependo   " << distance << endl;
                        //EV << "Probabilidad dependencia  " << prob << " random number  "<< random << endl;

                        //  It is more real to depend on the instructions that are already blocked than the ones that are onfly
                        //remember d=1 it means we depend on the back of the queue.
                        int position=0;
                        Id *x;
                        if(dependency.length()!=0){
                            if(distance<=dependency.length()){
                                //our dependency is with an element of the dep-list
                                position=(dependency.length()-1)-(distance-1);
                                //en dep-list hay instrucciones
                                z->setDep(((Instruction *)dependency.get(position))->getId());
                              //  EV << "aaaaaaaaaaaa   "  << endl;
                            }else{
                                //our dependency is with an element of onfly list
                                position=(onfly.length()-1)-(distance-1-dependency.length());
                                //en onfly list hay ids
                                x = (Id *)onfly.get(position)->dup();
                                z->setDep(x->getValue());
                             //   EV << "bbbbbbbbbbbb   " << position << "id value " << x->getValue() << endl;
                            }
                        }else{
                            //If dependency queue is empty, we can just depend with an onfly instruction
                            position=(onfly.length()-1)-(distance-1);
                            //en onfly list hay ids
                            x = (Id *)onfly.get(position)->dup();
                            z->setDep(x->getValue());
                         //   EV << "cccccccccc   "  << endl;
                        }
    //                    EV << "position in the correspondent queue   " << position << endl;
    //                    EV << "dependo del id   " << z->getDep() << endl;

                        //It is important to duplicate. It avoids to change any element in the queue onfly.
                        //Id *x = (Id *)onfly.get(uniform)->dup();
                        //EV << "Value obtained from the list onfly "<< x->getValue() << endl;
                        //We put the id from which this instruction depends on in the correspondent field of instruction and we proceed to store it in the dependent queue
                        //z->setDep(x->getValue());
                        dependency.insert(z);
                        EV << "Instruction   "<< z->getType()<< "  " <<z->getId() << "   blocked due to dependency to the id "<< z->getDep() << endl;
                       //if we uncomment this line it will not work, since we do not delete the value put bad dependencies and it provokes null pointers access!!!!!!!!!!!!!!!
                        // delete x;
                        //sched=true;
                        if(!ooo){
                            break;
                        }
                    }
                    //load_sent.collect(capLoad);
                }
                //here it means that we will not send more instructions this cycle so we reset variables for functional units. It means we have finished the for(5)
                //take care, they are critical, they can block the processor
                branch=load=store=false;
                capInt=capLoad=0;
                x_for=0;

                //If the branch prepared this event in advanced this if sentence has no effect because the event is already scheduled
                //Moreover, a normal instruction has to want this event in the future --> exAMPLE: not activate it with dependencies
                if(!Event->isScheduled()&&sched){
                    sched=false;
                    scheduleAt(simTime()+1.0, Event);
                }
                //if(msg->isName("Event")){
                    //if (queue.length() <= (4)) {
                        if ((queue.length()+onfly.length()+dependency.length()<=ooo_window-4)&&(!block)) {
                        //The buffer has less than three elements so it sends trigger to full its buffer
                        cMessage *trigger = new cMessage("trigger");
                        send(trigger, "out");

 //                       EV<< "ASK FOR MORE INSTRUCTIONS --- " << endl;
                    }
                //}
                    emit(numfunc, queue.length());
            }
        }

        queue_length.collect(queue.length());
        onfly_length.collect(onfly.length());
        dep_length.collect(dependency.length());

        queue_time.record(queue.length());
        onfly_time.record(onfly.length());
        dep_time.record(dependency.length());

        //Always important to remove the messages that we will not use it anymore.
        if(msg->isName("Begin")){
            delete msg;
        }

    }
}

//This function is specially defined to send the instructions in a fair way to the functional units. Just implement it with a boolean variable.
void Control::specialSend(Instruction *p, int b, bool h)
{
    if(b==0){
        if(h){
            send(p, "out1");
        }else{
            sendDelayed(p, branch_miss_penalty, "out1");
        }
        outInt++;
        nInst++;
    }else if (b==1){
        if(h){
            send(p, "out2");
        }else{
            sendDelayed(p, branch_miss_penalty, "out2");
        }
        outInt++;
        nInst++;
    }else if(b==2){
        if(h){
            send(p, "out3");
        }else{
            sendDelayed(p, branch_miss_penalty, "out3");
        }
        nInst++;
        outInt=0;
    }
}

void Control::finish(){
    load_sent.recordAs("Loads sent");
    queue_length.recordAs("Ready queue length");
    dep_length.recordAs("Dep queue length");
    onfly_length.recordAs("Onfly queue length");
    recordScalar("blocked",numblocks);
}

}; //namespace
