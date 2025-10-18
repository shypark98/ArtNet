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


#include <iostream>
#include "bbPart.h"
#include "Partitioning/partProb.h"
#include "PartEvals/partEvals.h"
#include "PartLegality/1balanceChk.h"

using std::cout;
using std::endl;
using std::setw;
using std::ifstream;
using std::ofstream;

int main(int argc, const char *argv[])
{
//BoolParam helpRequest1("h",argc,argv);
//BoolParam helpRequest2("help",argc,argv);

  StringParam  baseFileName("f",argc,argv);
  StringParam  runCompFileName("runCompar",argc,argv);
  Verbosity  verb(argc,argv);
  BoolParam    noconsolidate("nc",argc,argv);
//  UnsignedParam pushLimit("pl",argc,argv);
  double totalReTime=0.0,totalNoReTime=0.0;
  if (runCompFileName.found())
      {
      ifstream runCompRead(runCompFileName); //,ios::in);
      if (runCompRead)
          {
          char dummy1[1023],dummy2[1023];
          runCompRead >> dummy1 >> totalReTime >> dummy2 >> totalNoReTime;
          abkfatal(strcmp(dummy1,"Total_reordering")==0 &&
                   strcmp(dummy2,"Total_not_reordering")==0,
                   "Incorrect format in running comparison file");

          }
      }

  PartitioningProblem::Parameters params(argc,argv);
//if (helpRequest1.found() || helpRequest2.found()) return 0;

  abkfatal(baseFileName.found()," -f baseFileName not found");
  PartitioningProblem problem(baseFileName,params);

  PartitioningProblem p1(problem);
  PartitioningProblem p2(problem);


  p1.propagateTerminals();
  p2.propagateTerminals();

  BBPart::Parameters bbParams(argc,argv);
  bbParams.consolidateMultiEdges= !noconsolidate;
  bbParams.reorderVertices=true;

  BBPartBitBoardContainer bbBitBoards;

  Timer reorderTm;
  BBPart bbPartRe(p1, bbBitBoards, bbParams);
  reorderTm.stop();

  bbParams.reorderVertices=false;

  Timer noReTm;
  BBPart bbPartNoRe(p2, bbBitBoards, bbParams);
  noReTm.stop();

  double timeRe=reorderTm.getUserTime();
  double timeNoRe=noReTm.getUserTime();

  totalReTime += timeRe;
  totalNoReTime += timeNoRe;
    if (runCompFileName.found())
      {
      ofstream runCompWrite(runCompFileName); //,ios::out);
      if (runCompWrite)
          {
          runCompWrite<<"Total_reordering "<<totalReTime
              <<" Total_not_reordering " <<totalNoReTime<< endl;
          }
      else
          abkfatal(0,"Unable to write to running-comparison file");
      }


  cout << "NUMCOMP: Reordering, " << timeRe <<" sec.  Not reordering, " <<
      timeNoRe << "sec, case " << baseFileName  << endl;

  if (timeRe<timeNoRe)
      cout << "ORDCOMP: faster when reordering, case " << baseFileName << endl;
  else if (timeRe>timeNoRe)
      cout << "ORDCOMP: slower when reordering, case " << baseFileName   << endl;
  else
      cout << "ORDCOMP: equal when reordering, case " << baseFileName  << endl;


  if (bbPartRe.timedOut())
    cout << "BBPart timed out (vertices reordered), case " << baseFileName   << endl;
  if (!bbPartRe.foundLegal())
    cout << "BBPart found no legal solution (vertices reordered), case " << baseFileName  << endl;

  if (bbPartNoRe.timedOut())
    cout << "BBPart timed out (vertices not reordered) , case " << baseFileName  << endl;
  if (!bbPartNoRe.foundLegal())
    cout << "BBPart found no legal solution (vertices not reordered), case " << baseFileName  << endl;


//UniversalPartEval eval(PartEvalType::NetCut2way);
//eval.printStatsForNon0CostNets(problem);

  NetCutWNetVec cutEvalRe(p1, p1.getBestSoln());
  const HGraphFixed& hgraphRe = p1.getHGraph();
  unsigned totCutRe = 0;

      for(unsigned hgERe = 0; hgERe != hgraphRe.getNumEdges(); hgERe++)
        if(cutEvalRe.getNetCost(hgERe) != 0) //was cut
        { totCutRe++; }

      cout<<"  Total Cut, vertices reordered:              "<<setw(5)<<totCutRe<<endl;
      abkfatal(cutEvalRe.getTotalCost()==totCutRe,
          "mismatch in calculating cost");

  NetCutWNetVec cutEvalNoRe(p2, p2.getBestSoln());
  const HGraphFixed& hgraphNoRe = p2.getHGraph();
  unsigned totCutNoRe = 0;

      for(unsigned hgENoRe = 0; hgENoRe != hgraphNoRe.getNumEdges(); hgENoRe++)
        if(cutEvalNoRe.getNetCost(hgENoRe) != 0) //was cut
        { totCutNoRe++; }

      cout<<"  Total Cut, vertices not reorderd:              "<<setw(5)<<totCutNoRe<<endl;

      abkfatal(cutEvalNoRe.getTotalCost()==totCutNoRe,
          "mismatch in calculating cost");
  

  if (bbPartRe.foundLegal() && !bbPartRe.timedOut() &&
      bbPartNoRe.foundLegal() && !bbPartNoRe.timedOut())
      {
      abkfatal(totCutRe==totCutNoRe,"reordering produced different cut "
          "from not reordering when both solutions must be optimal");
      }


  Partitioning resultPart(p2.getBestSoln());

  if (verb.getForMajStats()>=10)
      {
      cout << "Final partitioning (reordering):" << endl;
      for (unsigned kk=0;kk<resultPart.size();kk++)
          cout << resultPart[kk] << " ";
      cout << endl;

      }


  p2.saveBestSol("best.reord.sol");

  double area0=0.0,area1=0.0;
  for (unsigned kk=0;kk<hgraphRe.getNumNodes();kk++)
      {
      if (resultPart[kk].isInPart(0))
          {
          abkfatal2(!resultPart[kk].isInPart(1),"solution has "
              "nodes in multiple partitions, in case ",baseFileName);
          area0 += hgraphRe.getWeight(kk);
          }
      else
          area1 += hgraphRe.getWeight(kk);
      }
  double max0 = p2.getMaxCapacities()[0][0];
  double max1 = p2.getMaxCapacities()[1][0];
  cout << "\tarea0=" <<area0 << " area1=" << area1 << endl;
  cout << "\tmax0= " << max0<<" max1= "<<max1<< endl;
  if (area0>max0 || area1>max1)
      {
      
      if (bbPartRe.foundLegal())
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
