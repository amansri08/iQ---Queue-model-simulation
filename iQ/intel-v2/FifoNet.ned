//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

//
// Simple queueing network: generator + FIFO + sink.
//
network FifoNet
{
    @display("bgb=813,470");
    submodules:
        gen: Source {
            parameters:
                @display("p=38,150");
        }
        sink: Sink {
            parameters:
                @display("p=769,150");
        }
        control: Control {

            @display("p=162,150;i=block/classifier");
        }

        functionalUnit: FunctionalUnit {
            @display("p=288,33");
        }
        functionalUnit1: FunctionalUnit {
            @display("p=288,102");
        }
        functionalUnit2: FunctionalUnit {
            @display("p=288,201");
        }
        functionalUnit3: FunctionalUnit {
            @display("p=288,270");
        }
        functionalUnit4: FunctionalUnit {
            @display("p=288,348");
        }
        cache1Control: Cache1Control {
            @display("p=392,309");
        }
        functionalUnit5: FunctionalUnit {
            @display("p=440,387");
        }
        functionalUnit6: FunctionalUnit {
            @display("p=564,387");
        }
        cache1Control1: Cache1Control {
            @display("p=505,309");
        }
        cache2Control: Cache2Control {
            @display("p=623,309");
        }
        functionalUnit7: FunctionalUnit {
            @display("p=686,387");
        }
    connections:
        control.out --> gen.in1;
        gen.out --> control.in;
        sink.out --> control.in4;
        control.out1 --> functionalUnit.in;
        functionalUnit.out --> control.in1;
        functionalUnit.out1 --> sink.in;
        control.out2 --> functionalUnit1.in;
        functionalUnit1.out --> control.in2;
        functionalUnit1.out1 --> sink.in1;
        functionalUnit2.out --> control.in5;
        control.out3 --> functionalUnit2.in;
        functionalUnit2.out1 --> sink.in2;
        control.out5 --> functionalUnit4.in;
        control.out4 --> functionalUnit3.in;
        functionalUnit3.out --> control.in6;
        functionalUnit4.out --> control.in7;
        functionalUnit3.out1 --> cache1Control.in;
        functionalUnit4.out1 --> cache1Control.in1;
        cache1Control.out1 --> functionalUnit5.in;
        cache1Control.out --> sink.in3;
        functionalUnit5.out --> cache1Control.in2;
        functionalUnit5.out1 --> cache1Control1.in;
        cache1Control1.out1 --> functionalUnit6.in;
        cache1Control1.out --> sink.in4;
        functionalUnit6.out --> cache1Control1.in2;
        functionalUnit6.out1 --> cache2Control.in;
        cache2Control.out1 --> functionalUnit7.in;
        cache2Control.out --> sink.in5;
        functionalUnit7.out1 --> sink.in6;
        functionalUnit7.out --> cache2Control.in2;
}


