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
#include "PartLegality/1balanceChk.h"

using std::cout;
using std::endl;
using std::setw;

int main(int argc, const char *argv[])
{
//BoolParam helpRequest1("h",argc,argv);
//BoolParam helpRequest2("help",argc,argv);

  StringParam  baseFileName("f",argc,argv);
  Verbosity  verb(argc,argv);
  BoolParam    noconsolidate("nc",argc,argv);
//  UnsignedParam pushLimit("pl",argc,argv);

  PartitioningProblem::Parameters params(argc,argv);
//if (helpRequest1.found() || helpRequest2.found()) return 0;

  abkfatal(baseFileName.found()," -f baseFileName not found");
  PartitioningProblem problem(baseFileName,params);

  problem.propagateTerminals();
  BBPart::Parameters bbParams(argc,argv);
  bbParams.consolidateMultiEdges= !noconsolidate;
  BBPartBitBoardContainer bbBitBoards;
  BBPart bbPart(problem, bbBitBoards, bbParams);
  if (bbPart.timedOut())
    cout << "BBPart timed out " << endl;
  if (!bbPart.foundLegal())
    cout << "BBPart found no legal solution " << endl;

//UniversalPartEval eval(PartEvalType::NetCut2way);
//eval.printStatsForNon0CostNets(problem);

  NetCutWNetVec cutEval(problem, problem.getBestSoln());
  const HGraphFixed& hgraph = problem.getHGraph();
  unsigned totCut = 0;

      for(unsigned hgE = 0; hgE != hgraph.getNumEdges(); hgE++)
        if(cutEval.getNetCost(hgE) != 0) //was cut
        { totCut++; }

      cout<<"  Total Cut:              "<<setw(5)<<totCut<<endl;


      cout<<"  Evals Total Cut :"<<cutEval.getTotalCost()<<endl;

  Partitioning resultPart(problem.getBestSoln());

  if (verb.getForMajStats()>=10)
      {
      cout << "Final partitioning:" << endl;
      for (unsigned kk=0;kk<resultPart.size();kk++)
          cout << resultPart[kk] << " ";
      cout << endl;

      }


  problem.saveBestSol("best.sol");

  double area0=0.0,area1=0.0;
  for (unsigned kk=0;kk<hgraph.getNumNodes();kk++)
      {
      if (resultPart[kk].isInPart(0))
          {
          abkfatal2(!resultPart[kk].isInPart(1),"solution has "
              "nodes in multiple partitions, in case ",baseFileName);
          area0 += hgraph.getWeight(kk);
          }
      else
          area1 += hgraph.getWeight(kk);
      }
  double max0 = problem.getMaxCapacities()[0][0];
  double max1 = problem.getMaxCapacities()[1][0];
  cout << "\tarea0=" <<area0 << " area1=" << area1 << endl;
  cout << "\tmax0= " << max0<<" max1= "<<max1<< endl;
  if (area0>max0 || area1>max1)
      {
      
      if (bbPart.foundLegal())
          {
          cout << "In case " << baseFileName << ":" << endl
              << "\tBBPart thinks it found legal soln but "
              << "direct check disagrees" << endl;

          abkfatal2(0,"BBPart thinks it found "
              "a legal solution but direct check disagrees,"
              "in case ",baseFileName); 
          }
      }
  

//cout << SysInfo();

  return 0;
}
