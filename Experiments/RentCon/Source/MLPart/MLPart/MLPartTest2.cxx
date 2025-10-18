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


// Created by Igor Markov
// modified by Andrew Caldwell 08/04/98

//this is the test used to produce the Bookshelf MLPart executable

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
    BoolParam     saveBest    ("saveBest", argc, argv);


  cout<<endl<<"UCLA/UMichigan MLPart 5.2 (August 2004)"<<endl;
  cout<<"Compiled "<<__DATE__ << " at " << __TIME__<<endl<<endl;

  cout<<"Copyright (c) 1998-2000 Regents of the University of California\n";
  cout<<"Copyright (c) 2001-2004 Regents of the University of Michigan  \n";
  cout<<"           All rights reserved.\n";
  cout<<"Authors: Andrew E. Caldwell, Andrew B. Kahng, Igor L. Markov\n";
  cout<<"Email:   imarkov@eecs.umic.edu\n";
  cout<<endl;
  cout<<"Permission is hereby granted, without written agreement and \n"
      <<"without license or royalty fee, to use, copy, modify, and \n"
      <<"distribute and sell this software and its documentation for \n"
      <<"any purpose, provided that the above copyright notice, this \n"
      <<"permission notice, and the following two paragraphs appear \n"
      <<"in all copies of this software as well as in all copies of \n"
      <<"supporting documentation.\n\n";
  cout<<"THIS SOFTWARE AND SUPPORTING DOCUMENTATION IS PROVIDED \"AS IS\".\n"
      <<"The Microelectronics Advanced Research Corporation (MARCO), the\n"
      <<"Gigascale Silicon Research Center (GSRC),  and \n"
      <<"(\"PROVIDERS\") MAKE NO WARRANTIES, whether express \n"
      <<"or implied, including warranties of merchantability or fitness\n"
      <<"for a particular purpose or noninfringement, with respect to \n"
      <<"this software and supporting documentation.\n"
      <<"Providers have NO obligation to provide ANY support, assistance,\n"
      <<"installation, training or other services, updates, enhancements\n"
      <<"or modifications related to this software and supporting\n"
      <<"documentation.\n\n";
  cout<<"Providers shall NOT be liable for ANY costs of procurement of \n"
      <<"substitutes, loss of profits, interruption of business, or any \n"
      <<"other direct, indirect, special, consequential or incidental \n"
      <<"damages arising from the use of this software and its \n"
      <<"documentation, whether or not Providers have been advised of \n"
      <<"the possibility of such damages."<<endl<<endl;


    if(help1.found() || help2.found() || !baseFileName.found())
    {
        cout<<" -f    <aux filename>  (required) \n"
	    <<" -num  <n>   (will compute the best-of-n)\n"
	    <<" -runs <r>   (will average the best-of-n over r indep. runs)\n"
	    <<" -skipVCycle\n"
            <<" -saveBest\n"
	    <<" -help "<<endl;
        return 0;
    }

    MLPartParams mlParams(argc,argv);
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

    UnivPartEval costCheck(PartEvalType::NetCutWNetVec);

    cout<<endl;
 
    Partitioning bestSeen(problem.getHGraph().getNumNodes());
    unsigned     bestSeenCost=UINT_MAX;

    BBPartBitBoardContainer bbBitBoards;
    MaxMem maxMem;

    for(unsigned r = 0; r< totalRuns; r++)
    {

        cout<<"-----   Run Number "<<r+1<<" of "<<totalRuns
	    <<" runs (each a best of "<<startsPerRun<<")\n";

        MLPart mlPartitioner(problem,mlParams,bbBitBoards,&maxMem);
        Partitioning& curPart =
		(problem.getSolnBuffers()[problem.getBestSolnNum()]);

	unsigned actualCost = costCheck.computeCost(problem, curPart);

        if (saveBest.found() && actualCost < bestSeenCost) 
        { bestSeen=curPart; bestSeenCost=actualCost; }

        cout<<"Net Cut: "<<actualCost<<endl;
        cout<<"Part Areas: "<<mlPartitioner.getPartitionArea(0)
                   << "  ("<< mlPartitioner.getPartitionNumNodes(0)<<" nodes with terms) "
                   <<"  /  "<<mlPartitioner.getPartitionArea(1)
                   << "  ("<< mlPartitioner.getPartitionNumNodes(1)<<" nodes with terms) " <<endl;

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
      {
        cout<<" Ave Best Of 8: "<<sampler[8]<<"("<<runtimePer*8<<")"<<endl;
        cout<<" Ave Best Of 10: "<<sampler[10]<<"("<<runtimePer*10<<")"<<endl;
      }
      if(bests.size() >= 100)
      {
        cout<<" Ave Best Of 20: "<<sampler[20]<<"("<<runtimePer*20<<")"<<endl;
        cout<<" Ave Best Of 30: "<<sampler[30]<<"("<<runtimePer*30<<")"<<endl;
      }
      if(bests.size() >= 200)
      {
        cout<<" Ave Best Of 40: "<<sampler[40]<<"("<<runtimePer*40<<")"<<endl;
        cout<<" Ave Best Of 50: "<<sampler[50]<<"("<<runtimePer*50<<")"<<endl;
      }
    }

    if (saveBest.found())
    {
       Partitioning& curPart =
 		(problem.getSolnBuffers()[problem.getBestSolnNum()]);
       curPart =  bestSeen;
       problem.saveBestSol("best.sol");
    }

    cout << SysInfo();

    return 0;
}
