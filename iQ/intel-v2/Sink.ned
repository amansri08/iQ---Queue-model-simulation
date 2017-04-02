//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// Destroys the packets and collects statistics
//
simple Sink
{
    parameters:
        @display("i=block/sink");
        @signal[lifetime](type="simtime_t");
        @statistic[lifetime](title="lifetime of arrived jobs"; unit=s; record=vector,mean,max; interpolationmode=none);
        double real_ipc=default(1.0); 
        bool dep_f_active=default(false);
        double dep_correction=default(1.0);  
    gates:
        input in;
        input in1;
        input in2;
        input in3;
        input in4;
        input in5;
        input in6;
        output out;
}

