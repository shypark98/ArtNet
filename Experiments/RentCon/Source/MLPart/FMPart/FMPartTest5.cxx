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


#include "ABKCommon/abkcommon.h"
#include "ABKCommon/abkversion.h"
#include "PartEvals/partEvals.h"
#include "fmPart.h"

int main(int argc, const char *argv[])
{
  BoolParam helpRequest1("h",argc,argv);
  BoolParam helpRequest2("help",argc,argv);

  StringParam  baseFileName("f",argc,argv);
//UnsignedParam numStarts  ("num",argc,argv);

  BoolParam saveSoln("save", argc,argv);

// PRINT_VERSION_INFO

  PartitioningProblem::Parameters params(argc,argv);
  PartitionerParams pParams(argc,argv);
  if (helpRequest1.found() || helpRequest2.found()) return 0;

  abkfatal(baseFileName.found()," -f baseFileName not found");
  cout << params << endl;
  PartitioningProblem problem(baseFileName,params);

  problem.propagateTerminals();
//if(numStarts.found()) problem.reserveBuffers(numStarts);

  pParams.saveMoveLog=true;
  cout<<pParams<<endl;

  MaxMem maxMem;

  FMPartitioner fm(problem, &maxMem, pParams);

  UniversalPartEval eval(PartEvalType::NetCut2way);
  eval.printStatsForNon0CostNets(problem);
 
  if(saveSoln.found()) 
  {
    cout << "***Saving the solution into best.sol"<< endl;
    problem.saveBestSol("best.sol");
  }

  SVMoveLog log=fm.getMoveLog();
  if (log.isEmpty()) cout << "The move log is empty" << endl;
  else
  {
    cout << "*** Writing the move-log from the last pass to moveLog.out"
         << endl;
    log.writeTo("moveLog.out");
  }

  problem.printLargestCellStats();
  
  cout << SysInfo();

  return 0;
}
