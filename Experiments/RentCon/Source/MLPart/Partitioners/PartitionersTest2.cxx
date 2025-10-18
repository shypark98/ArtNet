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


#include "HGraph/hgFixed.h"
#include "Partitioning/partitioning.h"
#include "greedyHER.h"
#include "FMPart/fmPart.h"
#include "ABKCommon/abkcommon.h"
#include "Stats/trivStats.h"
#include "PartLegality/1balanceGen.h"
#include "PartEvals/netCut2way.h"

using std::cout;
using std::endl;

int main(int argc, const char *argv[])
{

    StringParam  baseFileName("f",argc,argv);  
    StringParam  outFileName ("out", argc, argv);
    UnsignedParam num("num", argc, argv);
    BoolParam initFM("FM", argc, argv);

    //MultiStartPartitioner::Parameters pParam(argc, argv);
    GreedyHERPartitioner::Parameters herParam(argc, argv);
    cout<<herParam<<endl;
    //cout<<pParam<<endl;

    Timer setupTime;
    abkfatal(baseFileName.found()," -f baseFileName not found");
    PartitioningProblem problem(baseFileName);
    if(num.found())
      problem.reserveBuffers(num); 
    problem.propagateTerminals(10);
    setupTime.stop();

    cout<<"----------------------- Generating initial solution-----------------------"<<endl;

    Timer fmTime;
    if(initFM.found())
    {
      FMPartitioner fmpart(problem, herParam.getMultiStartPartParams());
    }
    else
    {
        if(herParam.getMultiStartPartParams().verb.getForActions() > 1)
            cout<<"Generating an initial solution with Randomized Engineers"<<endl;
        SingleBalanceGenerator sGen(problem);
        PartitioningBuffer& buf = problem.getSolnBuffers();
        for(unsigned i = 0; i < buf.size(); ++i)
        {
            sGen.generateSoln(buf[i]);
            NetCut2way nc2wa(problem, buf[i]);
            problem.setCost(i, nc2wa.getTotalCostDouble());
        }
        vector<double> old_costs = problem.getCosts();
        cout<<"Randomized Engineer costs: ";
        for(unsigned i = 0; i < old_costs.size(); ++i)
            cout<<"  "<<old_costs[i];
        cout<<endl;
    }
    fmTime.stop();

    vector<double> old_costs = problem.getCosts();
   
    cout<<"----------------------- Starting greedy HER -----------------------"<<endl;

    Timer greedyHERTime;
    GreedyHERPartitioner gherPart(problem, herParam, true);
    greedyHERTime.stop();
    
    vector<double> new_costs = problem.getCosts();
    vector<double> improvement(new_costs.size(), 0);
    for(unsigned i = 0; i < new_costs.size(); ++i)
       improvement[i] = (old_costs[i] - new_costs[i])/old_costs[i] * 100;
    TrivialStatsWithStdDev stats(improvement); 

    if(herParam.getMultiStartPartParams().verb.getForMajStats() > 0)
    {
        cout<<"Runtime breakdown: "<<endl;
        cout<<"Parsing and problem setup: "<<(setupTime.getCombTime())<<endl;
        if(initFM.found())
        {
        cout<<"FMPart time: "<<(fmTime.getCombTime())<<endl;
        }
        else
        {
          cout<<"Initial partitioning time: "<<(fmTime.getCombTime())<<endl;
        }
        cout<<"Greedy HER time: "<<(greedyHERTime.getCombTime())<<endl;
        cout<<"Solution Quality Improvement: "<<endl;
        cout<<"Max: "<<stats.getMax()<<"% Avg: "<<stats.getAvg()<<"% Min: "<<stats.getMin()<<"% StdDev:"<<stats.getStdDev()<<"%"<<endl;
    }

    problem.saveBestSol("best.sol");

    return 0;
}



