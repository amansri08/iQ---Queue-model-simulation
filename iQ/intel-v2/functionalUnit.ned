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

module FunctionalUnit
{
    @display("i=block/cogwheel");
    gates:
        input in;
        output out;
        output out1;
    submodules:
        fifo: Fifo {
            @display("p=87,92");
        }
        delay: Delay {
            @display("p=171,92");
        }
    connections:
        fifo.out --> delay.in;
        in --> fifo.in;
        delay.out --> out1;
        fifo.out1 --> out;
}
