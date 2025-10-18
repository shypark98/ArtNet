/**************************************************************************
***    
*** Copyright (c) 1995-2000 Regents of the University of California,
***               Andrew E. Caldwell, Andrew B. Kahng and Igor L. Markov
*** Copyright (c) 2000-2007 Regents of the University of Michigan,
***               Saurabh N. Adya, Jarrod A. Roy, David A. Papa and
***               Igor L. Markov
***
***  Contact author(s): abk@cs.ucsd.edu, imarkov@umich.edu
***  Original Affiliation:   UCLA, Computer Science Department,
***                          Los Angeles, CA 90095-1596 USA
***
***  Permission is hereby granted, free of charge, to any person obtaining 
***  a copy of this software and associated documentation files (the
***  "Software"), to deal in the Software without restriction, including
***  without limitation 
***  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
***  and/or sell copies of the Software, and to permit persons to whom the 
***  Software is furnished to do so, subject to the following conditions:
***
***  The above copyright notice and this permission notice shall be included
***  in all copies or substantial portions of the Software.
***
*** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
*** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
*** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
*** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*** THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***
***
***************************************************************************/


#include <iostream>
#include "HGraph/hgFixed.h"
#include "Placement/placement.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

int main(int argc, char *argv[])
{
    StringParam areFile("are",argc,argv);
    StringParam netFile("net",argc,argv);
    StringParam placementFile("place", argc, argv);


    HGraphFixed hgraph(netFile, areFile);

    ifstream placementStream(placementFile);
    Placement origPl(placementStream);


    cout<<"Re-arranging placement - "<<endl;
    cout<<" Moving the last "<<hgraph.getNumTerminals()<<" to the front"<<endl;

    Placement newPl(origPl.getSize());
    unsigned destLoc = 0;
    unsigned srcLoc  = hgraph.getNumNodes() - hgraph.getNumTerminals();

	//copy the pads to the front
    for( ; srcLoc != hgraph.getNumNodes(); srcLoc++, destLoc++)
	newPl[destLoc] = origPl[srcLoc];

	//copy the core cells behind them
    for(srcLoc = 0; destLoc != hgraph.getNumNodes(); srcLoc++, destLoc++)
	newPl[destLoc] = origPl[srcLoc];

    ofstream newPlStream("newPl");
    newPlStream<<newPl;
    cout<<"Done saving new placement"<<endl;


    cout<<"newPl  size "<<newPl.getSize()<<endl;
    cout<<"hgraph size "<<hgraph.getNumNodes()<<endl;

    abkfatal(newPl.getSize() == hgraph.getNumNodes(),
		"mismatch between placement and hgraph"); 

    BBox coreBBox;

    for(unsigned n = hgraph.getNumTerminals(); n != hgraph.getNumNodes(); n++)
	coreBBox += newPl[n];		

    cout<<"Core BBox "<<coreBBox<<endl<<endl;

    return 0;
}


