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

using std::ifstream;
using std::cout;
using std::endl;
using uofm::vector;

int main(int argc, char *argv[])
{
    StringParam areFile("are",argc,argv);
    StringParam netFile("net",argc,argv);
    StringParam placementFile("place", argc, argv);
    StringParam base("base", argc, argv);

    cout<<"Reading"<<endl;
    cout<<areFile<<endl<<netFile<<endl<<placementFile<<endl;

    unsigned baseNameLen = strlen(base);


    HGraphFixed hgraph(netFile, areFile);

    ifstream placementStream(placementFile);
    Placement newPl(placementStream);


    BBox coreBBox;

    for(unsigned n = hgraph.getNumTerminals(); n != hgraph.getNumNodes(); n++)
	coreBBox += newPl[n];		

    cout<<"Core BBox "<<coreBBox<<endl<<endl;

     double xMin = coreBBox.xMin;
     double xMax = coreBBox.xMax;
     double xMid = ((coreBBox.xMax - coreBBox.xMin)/2)+coreBBox.xMin;
     double x1Qt = ((coreBBox.xMax - coreBBox.xMin)/4)+coreBBox.xMin;
     double x3Qt = ((coreBBox.xMax - coreBBox.xMin)/4)+xMid;
     double yMin = coreBBox.yMin;
     double yMax = coreBBox.yMax;
     double yMid = ((coreBBox.yMax - coreBBox.yMin)/2)+coreBBox.yMin;
     double y1Qt = ((coreBBox.yMax - coreBBox.yMin)/4)+coreBBox.yMin;
     double y3Qt = ((coreBBox.yMax - coreBBox.yMin)/4)+yMid;

     vector<BBox> partitions(2);

     char* partProbName = new char[128];
     char* strOffset = partProbName;
     strcpy(strOffset, base);	//all will start w/ the same string
     strOffset += baseNameLen;

    cout<<"Top-Level Problem:"<<endl;
    cout<<"  Vertical Bisection "<<endl;

     partitions[0] = BBox(xMin,yMin, xMid,yMax);
     partitions[1] = BBox(xMid,yMin, xMax,yMax);

     SubPartitioningProblem topV(partitions, hgraph, newPl);
     topV.propagateTerminals();
     topV.makeVanilla();
     strcpy(strOffset, "A_L0_V");
     *(strOffset+strlen("A_L0_V")) = 0;
     topV.saveAsNetDAre(partProbName);

    cout<<"  Horizontal Bisection "<<endl;

     partitions[0] = BBox(xMin,yMin, xMax,yMid);
     partitions[1] = BBox(xMin,yMid, xMax,yMax);

     SubPartitioningProblem topH(partitions, hgraph, newPl);
     topH.propagateTerminals();
     topH.makeVanilla();
     strcpy(strOffset, "B_L0_H");
     *(strOffset+strlen("B_L0_H")) = 0;
     topH.saveAsNetDAre(partProbName);


    cout<<"SecondLevel Problems:"<<endl;

    cout<<" 'Left-Half, VCut"<<endl;

     partitions[0] = BBox(xMin,yMin, x1Qt,yMax);
     partitions[1] = BBox(x1Qt,yMin, xMid,yMax);

     SubPartitioningProblem secondLV(partitions, hgraph, newPl);
     secondLV.propagateTerminals();
     secondLV.makeVanilla();
     strcpy(strOffset, "C_L1_V0V");
     *(strOffset+strlen("C_L1_V0V")) = 0;
     secondLV.saveAsNetDAre(partProbName);

    cout<<" 'Left-Half, HCut"<<endl;

     partitions[0] = BBox(xMin,yMid, xMid,yMax);
     partitions[1] = BBox(xMin,yMin, xMid,yMid);

     SubPartitioningProblem secondLH(partitions, hgraph, newPl);
     secondLH.propagateTerminals();
     secondLH.makeVanilla();
     strcpy(strOffset, "D_L1_V0H");
     *(strOffset+strlen("D_L1_V0H")) = 0;
     secondLH.saveAsNetDAre(partProbName);

    cout<<" 'Right-Half, VCut"<<endl;

     partitions[0] = BBox(xMid,yMin, x3Qt,yMax);
     partitions[1] = BBox(x3Qt,yMin, xMax,yMax);

     SubPartitioningProblem secondRV(partitions, hgraph, newPl);
     secondRV.propagateTerminals();
     secondRV.makeVanilla();
     strcpy(strOffset, "E_L1_V1V");
     *(strOffset+strlen("E_L1_V1V")) = 0;
     secondRV.saveAsNetDAre(partProbName);


    cout<<" 'Right-Half, HCut"<<endl;

     partitions[0] = BBox(xMid,yMid, xMax,yMax);
     partitions[1] = BBox(xMid,yMin, xMax,yMid);

     SubPartitioningProblem secondRH(partitions, hgraph, newPl);
     secondRH.propagateTerminals();
     secondRH.makeVanilla();
     strcpy(strOffset, "F_L1_V1H");
     *(strOffset+strlen("F_L1_V1H")) = 0;
     secondRH.saveAsNetDAre(partProbName);

    cout<<"Fourth-Level Cycling Problems"<<endl;

    cout<<" Center, VCut"<<endl;

     partitions[0] = BBox(x1Qt,y1Qt, xMid,y3Qt);
     partitions[1] = BBox(xMid,y1Qt, x3Qt,y3Qt);

     SubPartitioningProblem secondCV(partitions, hgraph, newPl);
     secondCV.propagateTerminals();
     secondCV.makeVanilla();
     strcpy(strOffset, "G_L3_C11-33V");
     *(strOffset+strlen("G_L3_C11-33V")) = 0;
     secondCV.saveAsNetDAre(partProbName);

    cout<<" Center, HCut"<<endl;

     partitions[0] = BBox(x1Qt,y1Qt, x3Qt,yMid);
     partitions[1] = BBox(x1Qt,yMid, x3Qt,y3Qt);

     SubPartitioningProblem secondCH(partitions, hgraph, newPl);
     secondCH.propagateTerminals();
     secondCH.makeVanilla();
     strcpy(strOffset, "H_L3_C11-33H");
     *(strOffset+strlen("H_L3_C11-33H")) = 0;
     secondCH.saveAsNetDAre(partProbName);

    return 0;
}


