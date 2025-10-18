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


// from main0.cxx, by Igor Markov
// modified by Andrew Caldwell 08/04/98

//this version introduces random weight permutations into the
//edge weights

#include "ABKCommon/abkversion.h"
#include "Partitioning/partProb.h"
#include "mlPart.h"
#include "PartEvals/partEvals.h"
#include "Stats/trivStats.h"
#include "Stats/expMins.h"

using std::cout;
using std::endl;
using uofm::vector;

int main(int argc, const char *argv[]) 
{
    BoolParam    help1("h",   argc,argv),
                 help2("help",argc,argv);
    NoParams     noParams(argc,argv);

    StringParam   baseFileName("f",argc,argv);
    UnsignedParam numStarts   ("num", argc, argv);
    UnsignedParam numRuns     ("runs", argc, argv);
    BoolParam     skipVCycle  ("skipVCycle", argc, argv);


    cout<<endl<<"UCLA MLPart 5.2 (August 2004)"<<endl<<endl;
    cout<<"Compiled "<<__DATE__ << " at " << __TIME__<<endl;

    if(help1.found() || help2.found() || noParams.found())
    {
        cout<<" -f    <aux filename>  \n"
	    <<" -num  <n>   (will compute the best-of-n)\n"
	    <<" -runs <r>   (will average the best-of-n over r indep. runs)\n"
	    <<" -skipVCycle\n"
	    <<" -help "<<endl;
        return 0;
    }

    MLPartParams mlParams(0,0);
    mlParams.eval = PartEvalType::NetCut2wayWWeights;
	//mlParams.verb = Verbosity("0_0_0");
    if(skipVCycle.found())
	mlParams.Vcycling = MLPartParams::NoVcycles;
	

    abkfatal(baseFileName.found()," -f baseFileName not found");

    PartitioningProblem::Parameters   pParams(argc,argv);

    PartitioningProblem problem(baseFileName, pParams);
    problem.propagateTerminals(mlParams.partFuzziness);

    unsigned startsPerRun = 1;
    if(numStarts.found()) startsPerRun = numStarts;
    
    problem.reserveBuffers(startsPerRun);

    unsigned totalRuns = 1;
    if(numRuns.found()) totalRuns = numRuns;

    vector<unsigned>	bests;
    vector<double>	userTimes;
    double		totalRuntime = 0;

    UnivPartEval cutCheck(PartEvalType::NetCut2wayWWeights);
    UnivPartEval wCutCheck(PartEvalType::NetCut2way);

    HGraphFixed& hgraph = const_cast<HGraphFixed&>(problem.getHGraph());
    RandomRawUnsigned rng;

    for(unsigned e = 0; e < hgraph.getNumEdges(); e++)
    {
        HGFEdge& edge = hgraph.getEdgeByIdx(e);
        //this gives us range of +/- 5
        double adjPct = static_cast<int>(rng%10)-5;

        double newWt  = (100.0+adjPct);
        edge.setWeight(rint(newWt));
    }

    cout<<endl;

    BBPartBitBoardContainer bbBitBoards;
    MaxMem maxMem;

    for(unsigned r = 0; r< totalRuns; r++)
    {

        cout<<"-----   Run Number "<<r+1<<" of "<<totalRuns
	    <<" runs (each a best of "<<startsPerRun<<")\n";

        MLPart mlPartitioner(problem,mlParams,bbBitBoards,&maxMem);
        Partitioning& curPart =
		(problem.getSolnBuffers()[problem.getBestSolnNum()]);

	unsigned actualCost = wCutCheck.computeCost(problem, curPart);

        cout<<"UnWeighted Net Cut: "
			<<cutCheck.computeCost(problem, curPart)<<endl;
        cout<<"Weighted   Net Cut: "<<actualCost<<endl;

        cout<<"Part Areas: "<<mlPartitioner.getPartitionArea(0)
                <<" / "<<mlPartitioner.getPartitionArea(1)<<endl;

	bests.push_back(actualCost);
	userTimes.push_back(mlPartitioner.getUserTime());
	totalRuntime += userTimes.back();

	//clear the buffer for the next run
	PartitioningBuffer &buff = problem.getSolnBuffers();
  	for(unsigned s = buff.beginUsedSoln(); s != buff.endUsedSoln(); s++)
        {
	    Partitioning &curBuff = buff[s];
	    for(unsigned i = 0; i < curBuff.size(); i++)
		curBuff[i].setToAll(31);
	}
    }

    cout<<"________________ Overall Results ___________________\n";
    cout<<"  Best Costs:   "<<TrivialStatsWithStdDev(bests)<<endl;
    cout<<"  RunTime:      "<<TrivialStatsWithStdDev(userTimes)<<endl;

    double runtimePer = totalRuntime/totalRuns;

    if(bests.size() >= 20)
    {
      ExpectedMins sampler(bests);

      cout<<"_______ Average Best Of N/ Runtime _______\n";
      cout<<" Ave Best Of 1: "<<sampler[1]<<"("<<runtimePer<<")"<<endl;
      cout<<" Ave Best Of 2: "<<sampler[2]<<"("<<runtimePer*2<<")"<<endl;

      if(bests.size() >= 40)
        cout<<" Ave Best Of 4: "<<sampler[4]<<"("<<runtimePer*4<<")"<<endl;

      if(bests.size() >= 50)
        cout<<" Ave Best Of 8: "<<sampler[8]<<"("<<runtimePer*8<<")"<<endl;
    }

    cout << SysInfo();

    return 0;
}
