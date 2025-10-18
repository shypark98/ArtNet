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
#include "subPartProb.h"

using std::cout;
using std::endl;
using std::ifstream;
using uofm::vector;

int main(int argc, char *argv[])
{

    StringParam areFile("are",argc,argv);
    StringParam netFile("net",argc,argv);
    StringParam placementFile("place", argc, argv);

    cout<<"Reading"<<endl;
    cout<<areFile<<endl<<netFile<<endl<<placementFile<<endl;



    HGraphFixed hgraph(netFile, areFile);

    ifstream placementStream(placementFile);
    Placement newPl(placementStream);


    BBox coreBBox;

    for(unsigned n = hgraph.getNumTerminals(); n != hgraph.getNumNodes(); n++)
	coreBBox += newPl[n];		

    cout<<"Core BBox "<<coreBBox<<endl<<endl;

     double xMin = coreBBox.xMin;
//   double xMax = coreBBox.xMax;
     double xMid = ((coreBBox.xMax - coreBBox.xMin)/2)+coreBBox.xMin;
//   double x1Qt = ((coreBBox.xMax - coreBBox.xMin)/4)+coreBBox.xMin;
//   double x3Qt = ((coreBBox.xMax - coreBBox.xMin)/4)+xMid;
//   double yMin = coreBBox.yMin;
//   double yMax = coreBBox.yMax;
     double yMid = ((coreBBox.yMax - coreBBox.yMin)/2)+coreBBox.yMin;
//   double y1Qt = ((coreBBox.yMax - coreBBox.yMin)/4)+coreBBox.yMin;
//   double y3Qt = ((coreBBox.yMax - coreBBox.yMin)/4)+yMid;

     double x3_8ths= -((coreBBox.xMax - coreBBox.xMin)/8)+xMin;
     double y3_8ths= -((coreBBox.yMax - coreBBox.yMin)/8)+yMid;
     double x5_8ths=  ((coreBBox.xMax - coreBBox.xMin)/8)+xMid;
     double y5_8ths=  ((coreBBox.yMax - coreBBox.yMin)/8)+yMid;

     vector<BBox> partitions(2);

     cout<<"Fifth-Level Cycling Problem"<<endl;

     cout<<" 'Center, VCut"<<endl;

     partitions[0] = BBox(x3_8ths,y3_8ths, xMid,y5_8ths);
     partitions[1] = BBox(xMid, y3_8ths, x5_8ths,y5_8ths);

     SubPartitioningProblem secondCV(partitions, hgraph, newPl);
     secondCV.propagateTerminals();
     secondCV.saveAsNetDAre("ibm10X_L5_C33-55V");

    return 0;
}


