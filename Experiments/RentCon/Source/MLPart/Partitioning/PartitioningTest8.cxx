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


#include "Stats/trivStats.h"
#include "partitioning.h"
#include "subPartProb.h"

using std::ifstream;
using std::flush;
using std::cout;
using std::endl;
using std::max;
using uofm::vector;

int main(int argc, const char *argv[]) 
{
    StringParam  netDFileName("netD",argc,argv);  
    StringParam  areFileName("are",argc,argv);  
    StringParam  plFileName("pl",argc,argv);
    abkfatal(netDFileName.found() && areFileName.found() && plFileName.found(),
             " usage: prog -netD <netD file> -are <are file> -pl <pl file>");


    ifstream plFile(plFileName);
    Placement pl(plFile);
    plFile.close();

    HGraphFixed hg(netDFileName, areFileName);

    //hg.printEdgeSizeStats();
    //hg.printNodeDegreeStats();

    LayoutBBoxes boxes(2); 
    boxes[0] = BBox(0,0,100000,100000);
    boxes[1] = BBox(0,100000,100000,150000);

    cout<<"Generating partProb....."<<endl;
    SubPartitioningProblem problem(boxes, hg, pl);

    problem.saveAsNetDAre("t8");

    cout<<"Stats for Problem"<<endl;
    const HGraphFixed &hgraph = problem.getHGraph();

    vector<unsigned> nodeDegs;
    double           totalArea = 0;
    double   	     maxArea = 0;
    vector<double>   nodeAreas;

    for(itHGFNodeGlobal n = hgraph.nodesBegin(); n != hgraph.nodesEnd(); n++)
    {
	nodeDegs.push_back((*n)->getDegree());
	totalArea += hgraph.getWeight((*n)->getIndex());
	nodeAreas.push_back(hgraph.getWeight((*n)->getIndex()));	
	maxArea = max(maxArea, hgraph.getWeight((*n)->getIndex()));
    }

    vector<unsigned> netDegs;
    for(itHGFEdgeGlobal e = hgraph.edgesBegin(); e != hgraph.edgesEnd(); e++)
    {
	netDegs.push_back((*e)->getDegree());
    }

    cout<<"___Node Stats______"<<endl<<endl << flush;
    cout<<"Total Nodes: "<<hgraph.getNumNodes()<<endl;
    cout<<"  Terminals: "<<hgraph.getNumTerminals()<<endl;
    cout<<"  Non-Terms: "<<hgraph.getNumNodes()-hgraph.getNumTerminals()<<endl;
    cout<<"TotalNodeArea: "<<totalArea<<endl;
    cout<<" Node Areas: "<<TrivialStats(nodeAreas)<<endl;
    cout<<" Largest Node is "<<(maxArea / totalArea)*100<<
			"% of total area"<<endl;

    cout<<"Node Degrees: "<<TrivialStats(nodeDegs)<<endl;

    cout<<endl<<"___Net Stats______"<<endl;

    cout<<"Total Nets:  "<<hgraph.getNumEdges()<<endl;
    cout<<" Net Degrees: "<<TrivialStats(netDegs)<<endl;

    return 0;
}
	
