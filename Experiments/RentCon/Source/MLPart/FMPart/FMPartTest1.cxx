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


#include "ABKCommon/abkversion.h"
#include "Stats/trivStats.h"
#include "Partitioning/partProb.h"
#include "PartEvals/partEvals.h"
#include "fmPart.h"

using std::cout;
using std::endl;
using uofm::vector;

int main(int argc, const char *argv[]) 
{
    BoolParam    helpRequest1("h",   argc,argv),
                 helpRequest2("help",argc,argv);
    NoParams     noParams(argc,argv);

    StringParam  baseFileName("f",argc,argv);
    UnsignedParam numStarts("num", argc, argv);

    UnsignedParam numRuns("runs", argc, argv);

    PRINT_VERSION_INFO

    cout << CmdLine(argc,argv);

    PartitioningProblem::Parameters params(argc,argv);

    if (helpRequest1.found() || helpRequest2.found() || noParams.found())
    {
        cout << " -f    <aux filename>  \n"
             << " -num  <multistarts>   \n"
             << " -runs <# complete trials> \n"
             << " -help " << endl;
        return 0;
    }

    abkfatal(baseFileName.found()," -f baseFileName not found");

    PartitioningProblem problem(baseFileName,params);

    PartitionerParams pParams(argc,argv);
    cout << endl << pParams<<endl;

    unsigned startsPerRun = 1;
    if(numStarts.found()) startsPerRun = numStarts;
    
    problem.reserveBuffers(startsPerRun);

    unsigned totalRuns = 1;
    if(numRuns.found()) totalRuns = numRuns;

    vector<unsigned>	bests;
    vector<double>	userTimes;

    MaxMem maxMem;

    for(unsigned r = 0; r< totalRuns; r++)
    {
  	PartitioningBuffer& partBuff = problem.getSolnBuffers();
	unsigned s;
	for(s = partBuff.beginUsedSoln(); s != partBuff.endUsedSoln(); s++)
	{
	   for(unsigned cell = problem.getHGraph().getNumTerminals();
	       cell != problem.getHGraph().getNumNodes(); cell++)
	   {
		partBuff[s][cell].setToAll(problem.getNumPartitions());
	   }
	}

        cout<<"-----   TestRun Number "<<r<<" ("<<totalRuns<<" starts) \n";

	Timer fmTimer;

        FMPartitioner fmPartitioner(problem,&maxMem,pParams);
        Partitioning& curPart =
		(problem.getSolnBuffers()[problem.getBestSolnNum()]);

	fmTimer.stop();

        UnivPartEval costCheck(PartEvalType::NetCutWNetVec);
        cout<<"Net Cut: "<<costCheck.computeCost(problem, curPart)<<endl;
	bests.push_back(costCheck.computeCost(problem, curPart));
	userTimes.push_back(fmTimer.getUserTime());
	cout<<"Imbalance "<<fmPartitioner.getBestSolnPinImbalance()<<endl;

        problem.reserveBuffers(startsPerRun);

    }

    cout<<"________________ Overall Results ___________________\n";
    cout<<" Best Costs:   "<<TrivialStatsWithStdDev(bests);
    cout<<" RunTime:      "<<TrivialStatsWithStdDev(userTimes);

    cout << SysInfo();
    return 0;
}
