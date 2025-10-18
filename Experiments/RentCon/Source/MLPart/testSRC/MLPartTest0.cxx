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


// created by Igor Markov, 980609

// This example illustrates stand-alone ML

#include "ABKCommon/abkversion.h"
#include "Partitioning/partProb.h"
#include "MLPart/mlPart.h"
#include "PartEvals/partEvals.h"
#include "FilledHier/fillHier.h"
#include <new>

using std::cout;
using std::endl;

int main(int argc, const char *argv[]) 
{
    BoolParam    helpRequest1("h",   argc,argv),
                 helpRequest2("help",argc,argv);
    NoParams     noParams(argc,argv);

    StringParam  baseFileName("f",argc,argv);
    UnsignedParam numStarts("num", argc, argv);

    StringParam  hclFile("hcl", argc, argv);

    PRINT_VERSION_INFO
    cout << CmdLine(argc,argv);

    MLPartParams mlParams(argc,argv);
    cout << endl << mlParams;

    if (helpRequest1.found() || helpRequest2.found() || noParams.found())
    {
        cout << " -f   <aux filename>  \n"
             << " -num <multistarts>   \n"
             << " -help " << endl;
        return 0;
    }

    abkfatal(baseFileName.found()," -f baseFileName not found");
   
    PartitioningProblem::Parameters ppParams(argc,argv); 
    PartitioningProblem problem(baseFileName,ppParams);
    problem.propagateTerminals(mlParams.partFuzziness);

    if(numStarts.found()) problem.reserveBuffers(numStarts);
    else problem.reserveBuffers(1);

    FillableHierarchy *hier = NULL;

    if(hclFile.found())
    {
        FillableHierarchyParameters  hierParams(argc, argv); 
	hier = new FillableHierarchy(hclFile, problem.getHGraph(), hierParams);
    }	

    try
    {
      BBPartBitBoardContainer bbBitBoards;
      MaxMem maxMem;
      MLPart mlPartitioner(problem, mlParams, bbBitBoards, &maxMem, hier);
      if(hier) delete hier; 
      hier = NULL;

      Partitioning& curPart =(problem.getSolnBuffers()[problem.getBestSolnNum()]);

      UnivPartEval costCheck(PartEvalType::NetCutWNetVec);
      cout<<"Net Cut:    "<<costCheck.computeCost(problem, curPart)<<endl;

      cout<<"Part Areas: "<<mlPartitioner.getPartitionArea(0)
	        << "  ("<< mlPartitioner.getPartitionNumNodes(0)<<" nodes with terms) " 
		<<"  /  "<<mlPartitioner.getPartitionArea(1)
	        << "  ("<< mlPartitioner.getPartitionNumNodes(1)<<" nodes with terms) " <<endl;

      costCheck.printStatsForNon0CostNets(problem);

      problem.saveBestSol("best.sol");
      cout << SysInfo();
    }
    catch(const std::bad_alloc &e)
    {
      cout << endl << e.what() << endl;
      cout << endl << "MLPart has run out of memory. Exiting." << endl;
    }

    return 0;
}
