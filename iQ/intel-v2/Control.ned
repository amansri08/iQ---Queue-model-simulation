//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// Generates jobs (messages) with the given interarrival time,
// which can be a random variate.
//
simple Control
{
    parameters:
    
    	bool branch_miss=default(true);
    	double prob_branch_miss=default(0.0);
    	double branch_miss_penalty=default(3.0);
    	
        double n_registers=default(12.0); 
        double ooo_window=default(5); 
        bool ooo = default(true);
        
        double distance=default(168.0);
        
        double prob_dep_1=default(0.0);
        double prob_dep_2=default(0.0);
        double prob_dep_4=default(0.0);
        double prob_dep_8=default(0.0);
        double prob_dep_16=default(0.0);
        double prob_dep_32=default(0.0);
        double prob_dep_64=default(0.0);
        double prob_else=default(1.0);
        
        
         
        @signal[numfunc](type="simtime_t");
        @statistic[numfunc](title="number of ints in the queue of control"; unit=s; record=vector,stats; interpolationmode=none);
    gates:
    	input in;
    	input in1;
    	input in2;
    	input in4;
    	input in5;
    	input in6;
    	input in7;
        output out;
        output out1;
        output out2;
        output out3;
        output out4;
        output out5;
}


