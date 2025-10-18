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


//this is the test used to produce the bookshelf Flat Partitioner

#include "Partitioning/partProb.h"
#include "fmPart.h"
#include "PartEvals/partEvals.h"
#include "Stats/trivStats.h"
#include "Stats/expMins.h"
#include "ABKCommon/abkversion.h"

using std::cout;
using std::endl;
using uofm::vector;

int main(int argc, const char *argv[]) 
{
    BoolParam    helpRequest1("h",   argc,argv);
    BoolParam    helpRequest2("help",argc,argv);
    NoParams     noParams(argc,argv);

    StringParam  baseFileName("f",argc,argv);
    UnsignedParam numStarts("num", argc, argv);
    UnsignedParam numRuns("runs", argc, argv);
    BoolParam     clip("clip", argc, argv);

  cout<<"\n(c) Copyright, 1998-2000 Regents of the University of California\n";
  cout<<" Authors: Andrew E. Caldwell, Andrew B. Kahng, Igor L. Markov\n";
  cout<<" Email:   {caldwell,abk,imarkov}@cs.ucla.edu"<<endl<<endl;

  PRINT_VERSION_INFO

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
    
    if (helpRequest1.found() || helpRequest2.found() || noParams.found())
    {
        cout<<" -f    <aux filename>  \n"
            <<" -num  <n>   (will compute the best-of-n)\n"
            <<" -runs <r>   (will average the best-of-n over r indep. runs)\n"
	    <<" -clip \n";
        return 0;
    }
    
    abkfatal(baseFileName.found()," -f baseFileName not found");

    PartitioningProblem::Parameters params(argc,argv);
    PartitioningProblem problem(baseFileName,params);

    unsigned startsPerRun = 1;
    if(numStarts.found()) startsPerRun = numStarts;

    problem.reserveBuffers(startsPerRun);
    
    PartitionerParams pParams(0,0);

    if(clip.found()) pParams.useClip = 1;
    
    unsigned totalRuns = 1;
    if(numRuns.found()) totalRuns = numRuns;
    
    vector<double>	bests;
    vector<double>	userTimes;
    
    double 		totalRuntime = 0;

    MaxMem maxMem;

    cout<<endl;
    
    UnivPartEval costCheck(PartEvalType::NetCutWNetVec);
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
    
    	cout<<"-----   TestRun Number "<<r+1<<" ( of "<<totalRuns<<
				", each a best of "<<startsPerRun<<") \n";
    
    	Timer fmTimer;
    
    	FMPartitioner fmPartitioner(problem,&maxMem,pParams);
    	Partitioning& curPart =
    	(problem.getSolnBuffers()[problem.getBestSolnNum()]);
    
    	fmTimer.stop();
    
	cout<<" Best of "<<startsPerRun<<" Net Cut: "
	    <<costCheck.computeCost(problem, curPart)<<endl;

    	bests.push_back(costCheck.computeCost(problem, curPart));

    	userTimes.push_back(fmTimer.getUserTime());
	totalRuntime += fmTimer.getUserTime();
    
       	problem.reserveBuffers(startsPerRun);
    }

    cout<<"________________ Overall Results ___________________\n";
    cout<<" Best Costs:   "<<TrivialStatsWithStdDev(bests);
    cout<<" RunTime:      "<<TrivialStatsWithStdDev(userTimes);


 if(totalRuns >= 10)
 {
    double runtimePer = totalRuntime/bests.size();
    ExpectedMins sampler(bests);

    cout<<"_____ (Sampled)Average Best Of N (Average Runtime for N) ___\n";
    cout<<" Ave Best Of 1: "<<sampler[1]<<" ("<<runtimePer<<")"<<endl; 
    cout<<" Ave Best Of 2: "<<sampler[2]<<" ("<<runtimePer*2<<")"<<endl; 
  if(totalRuns >= 20)
    cout<<" Ave Best Of 4: "<<sampler[4]<<" ("<<runtimePer*4<<")"<<endl; 
  if(totalRuns >= 40)
    cout<<" Ave Best Of 8: "<<sampler[8]<<" ("<<runtimePer*8<<")"<<endl; 
    cout << SysInfo();
    return 0;
 }
}

