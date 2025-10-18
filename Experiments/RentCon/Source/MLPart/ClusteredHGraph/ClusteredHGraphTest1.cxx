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


#include "Partitioning/partitioning.h"
#include "clustHGraph.h"
#include "Combi/partitionIds.h"
#include "ABKCommon/abkcommon.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using uofm::vector;

unsigned calculateCut(const HGraphFixed& graph, const Partitioning& part);

int main(int argc, const char *argv[]) 
{
    StringParam  baseFileName("f",argc,argv);
    StringParam  hclFileName("hcl",argc,argv);

    ClustHGraphParameters clustParams(argc, argv);
    BoolParam   help1("h", argc, argv);
    BoolParam   help2("help", argc, argv); 
    if(help1.found() || help2.found())
    {
	cout<<" -f <aux file> "<<endl;
	cout<<" -hcl <hcl file> "<<endl;
	return 0;
    }

    cout<<clustParams<<endl;

    abkfatal(baseFileName.found()," -f baseFileName not found");
    PartitioningProblem problem(baseFileName);
    const HGraphFixed&  graph=problem.getHGraph();

    Timer clTime;

    problem.propagateTerminals();

    abkfatal(hclFileName.found()," -hcl hclFileName not found");
    FillableHierarchyParameters fillHParams(argc, argv);
    FillableHierarchy fillH(hclFileName, graph, fillHParams);

    ClusteredHGraph tree(fillH, graph, clustParams);

    clTime.stop();
    const HGraphFixed& topLvl = tree.getHGraph(tree.getNumLevels()-1);

    cout<<clTime<<endl;
    cout<<MemUsage()<<endl;
    cout<<"Degree 2 duplicate edges removed: "<<tree.deg2Removed<<endl;
    cout<<"Degree 3 duplicate edges removed: "<<tree.deg3Removed<<endl;

    cout<<"There are "<<tree.getNumLevels()<<" saved HGraphs"<<endl;

    cout<<"TopLevel HGraph (#"<<tree.getNumLevels()-1<<")"<<endl;
    cout<<"  NumNodes: "<<topLvl.getNumNodes()<<endl;
    cout<<"  NumTerms: "<<topLvl.getNumTerminals()<<endl;
    cout<<"  NumEdges: "<<topLvl.getNumEdges()<<endl;


    vector<Partitioning> parts;
    parts.push_back( Partitioning(tree.getNumLeafNodes()) );
    parts.push_back( Partitioning(tree.getNumLeafNodes()) );
    

    RandomRawUnsigned rng;
    for(unsigned n = 0; n < topLvl.getNumNodes(); n++)
	parts[0][n].setToPart(rng %2);

    cout<<"Top level cut: "<<calculateCut(topLvl, parts[0])<<endl;

    unsigned curPart = 0;

    char hgraphName[100];
    sprintf(hgraphName,"HGTop");
    tree.getHGraph(tree.getNumLevels()-1).saveAsNodesNetsWts(hgraphName);

    for(int l = tree.getNumLevels()-2; l >= 0; l--)
    {
	tree.mapPartitionings(parts[curPart], parts[1-curPart], l);
	curPart = 1-curPart;

	const HGraphFixed& lvl = tree.getHGraph(l);

	cout<<"HGraph #"<<l<<endl;
     	cout<<"  NumNodes: "<<lvl.getNumNodes()<<endl;
     	cout<<"  NumTerms: "<<lvl.getNumTerminals()<<endl;
	cout<<"  NumEdges: "<<lvl.getNumEdges()<<endl;
        cout<<"  Cut: "<<calculateCut(lvl, parts[curPart])<<endl;
    }
    return 0;
}

unsigned calculateCut(const HGraphFixed& graph, const Partitioning& part)
{
    unsigned totalCut = 0;
    itHGFEdgeGlobal e;
    for(e = graph.edgesBegin(); e != graph.edgesEnd(); e++)
    {
	const HGFEdge& edge = **e;
	itHGFNodeLocal n;
	bool p0 = false;
	bool p1 = false;
	for(n = edge.nodesBegin(); n != edge.nodesEnd(); n++)
	{
	    if(part[(*n)->getIndex()].isInPart(0))
		p0 = true;
	    else
		p1 = true;
	}
	if(p0 && p1)
	    totalCut += static_cast<unsigned>(ceil(edge.getWeight()));
    }
    return totalCut;
}
