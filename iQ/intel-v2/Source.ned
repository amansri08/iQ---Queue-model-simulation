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
//long cannot be a parameter

simple Source
{
    parameters:
        @display("i=block/source");
        double prob_branch=default(0.0);
        double prob_int=default(0.0);
        double prob_load=default(0.0);
        double prob_store=default(0.0);
        double fetch=default(4.0);
        double icache_penalty=default(0.0);
        double icache_miss_prob=default(0.0);
        bool icache_active;
        
        
    gates:
    	input in1;
        output out;      
}


