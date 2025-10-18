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











#include "bbPart.h"
#include "Partitioning/partProb.h"
#include "PartEvals/partEvals.h"
#include "PartLegality/1balanceGen.h"

double partMax0,partMax1;
double bestMaxViol;
double bestSeen;

class CompareByDegree
{ 
    const HGraphFixed& _hg;
    public:
        CompareByDegree(const HGraphFixed& hg): _hg(hg) {}
        bool operator()(unsigned i, unsigned j) 
     { return _hg.getNodeByIdx(i).getDegree()>_hg.getNodeByIdx(j).getDegree();}
};

void checkForLegalSol(PartitioningProblem &problem,
                      NetCut2way          &eval)
    {
    HGraphFixed const &hgraph=problem.getHGraph();
    PartitioningBuffer &buffer = problem.getSolnBuffers();
    Partitioning       &parting= buffer[buffer.beginUsedSoln()];
    bool foundLegal=false;
    // deterministic engineer's method checks for legal solns
        {
        const Permutation& sortAscWts_1=hgraph.getNodesSortedByWeights();
        double areaLeft[2]={partMax0,partMax1};
        for(unsigned n=hgraph.getNumNodes()-1; n!=unsigned(-1); n-- )
            {
            unsigned part=(areaLeft[0]>areaLeft[1] ? 0 : 1);
            unsigned nodeIdx=sortAscWts_1[n];
            double wt=hgraph.getNodeByIdx(nodeIdx).getWeight();
            if (!parting[nodeIdx].isInPart(part)) 
                {
                abkfatal3(parting[nodeIdx].isInPart(1-part), 
                    " Node ", nodeIdx, " cannot be assigned to any partition");
                part=1-part;
                }
            areaLeft[part]-=wt;
            parting[nodeIdx].setToPart(part);
            }
        bestMaxViol=max(-min(areaLeft[0],areaLeft[1]),0.0);
        if (bestMaxViol==0.0)
            {
            foundLegal=true;
            cout<<" Deterministic engineer's method found a legal solution " << endl;
            }
        else
            cout
            <<" Deterministic engineer's method did not find a legal solution "<<endl;
        }
        
        if (!foundLegal)
            {  // randomized engineer's method
            bestMaxViol=DBL_MAX;
            Partitioning  copyPart(parting), tmpPart(parting);
            for(unsigned k=1;k!=5;k++)
                {
                SingleBalanceGenerator gen(problem);
                vector<double> balances=gen.generateSoln(tmpPart);
                double curViol=max(balances[0]-partMax0,balances[1]-partMax1);
                if (curViol<bestMaxViol)
                    {
                    bestMaxViol=max(curViol,0.0);
                    parting=tmpPart; 
                    tmpPart=copyPart;
                    if (bestMaxViol==0.0) { foundLegal=true; break;}
                    }
                }
            
            if ( foundLegal )  
                cout<< " Randomized engineers method found a legal solution in " 
                << k << " tries " << endl;
            else
                cout<<" Rand. engineer's method found no legal solution. Will do without."
                <<endl;
            }
        
        // cout << result;
        eval.resetTo(parting);
        bestSeen=2*eval.getTotalCost(); 
        cout << " Initial bound in main5.cxx"<<bestSeen/2.0 << endl;
    }



int main(int argc, const char *argv[])
    {
    //BoolParam helpRequest1("h",argc,argv);
    //BoolParam helpRequest2("help",argc,argv);
	cout << "SmallPart Partitioner compiled " << __DATE__ << endl;
    StringParam  baseFileName("f",argc,argv);
    Verbosity  verb(argc,argv);
    PartitioningProblem::Parameters params(argc,argv);
    abkfatal(baseFileName.found()," -f baseFileName not found");
    PartitioningProblem problem(baseFileName,params);
    
    problem.propagateTerminals();
    BBPart::Parameters bbParams(argc, argv); 
	bbParams.verb=verb;
    NetCut2way eval(problem);
    abkfatal(problem.getNumPartitions()==2," #parts != 2 not supported yet");
    
    PartitionIds part0, part1, part01;
    part0.setToPart(0); part1.setToPart(1); part01.setToAll(2);
    vector<unsigned> movables;
    HGraphFixed const &hgraph=problem.getHGraph();
    
    const Partitioning& fixedConstr=problem.getFixedConstr();
    unsigned nNodes=hgraph.getNumNodes();
    for(unsigned k=0; k!=nNodes; k++)
        {
        if ( fixedConstr[k].isEmpty() || fixedConstr[k]==part01 ) 
            {
            movables.push_back(k);
            }
        }
    CompareByDegree byDegree(hgraph);
    sort(movables.begin(),movables.end(),byDegree);
    
    PartitioningBuffer &buffer = problem.getSolnBuffers();
    Partitioning       &part   = buffer[buffer.beginUsedSoln()];
    
    for (k=0;k<movables.size();k++)
        {
        if (k%2==0)
            {
            part[movables[k]]=part0;
            }
        else
            {
            part[movables[k]]=part1;
            }
        }
    
    checkForLegalSol(problem,eval);
    
    partMax0=problem.getMaxCapacities()[0][0] + bestMaxViol;
    partMax1=problem.getMaxCapacities()[1][0] + bestMaxViol;
    
    BBPart bbPart(hgraph,part,movables,partMax0,partMax1,bbParams);
    NetCutWNetVec cutEval(problem, part);
    unsigned totCut = 0;
    
    for(unsigned hgE = 0; hgE != hgraph.getNumEdges(); hgE++)
        {
        if(cutEval.getNetCost(hgE) != 0) //was cut
            { totCut++; }
        }
    
    cout<<"  Total Cut:              "<<setw(5)<<totCut<<endl;
    
    cout<<"  Evals Total Cut :"<<cutEval.getTotalCost()<<endl;
 
	if (!bbPart.timedOut()) {
		problem.setBestSolnNum(buffer.beginUsedSoln());
		problem.saveBestSol("best.sol");
	}
    
    return 0;
    }
