/**************************************************************************
***    
*** Copyright (c) 2008 Regents of the University of California,
***               Andrew B. Kahng, Kwangok Jeong and Hailong Yao
***
***  Contact author(s): abk@cs.ucsd.edu, kjeong@vlsicad.ucsd.edu, hailong@cs.ucsd.edu
***  Original Affiliation:   UCSD, Computer Science and Engineering Department,
***                          La Jolla, CA 92093-0404 USA
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

/***************************
*** Created by Hailong Yao
*** hailong@cs.ucsd.edu
*** Date: Jul. 30, 2008
***************************/

#include <iostream>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <unistd.h>
#include <cmath>
#include <queue>

#include "Design.h"
#include "Cluster.h"

#include "ClusteredHGraph/baseClustHGraph.h"
#include "HGraph/hgFixed.h"
#include "Geoms/bbox.h"
#include "Partitioning/partProb.h"
#include "ABKCommon/abkversion.h"
#include "ABKCommon/abkseed.h"
#include "MLPart/mlPart.h"
#include "PartEvals/partEvals.h"
#include "Stats/trivStats.h"
#include "Stats/expMins.h"
#include "PartEvals/partEvals.h"

#include "ABKCommon/abkassert.h"
#include "ABKCommon/abktimer.h"
#include "ABKCommon/abkrand.h"

#include <new>

using namespace std;

namespace DESIGN {
  void Design::print()
  {
    cout<<"Total "<<chip.getGateNum()<<" gates"<<endl;
    cout<<"Chip Area: ["<<chip.getLX()<<","<<chip.getBY()<<"]->["<<chip.getRX()<<","<<chip.getTY()<<"]"<<endl;
  }
	
	void Design::chipPartition() {}

	void Design::addModulesEachGateNum(BoxVector &bv, Method *method)
	{
		BoolVector inside;
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
		int boxNum = bv.size();
		IntIVMap gateNumT1, gateNumT2, gateNumT3, gateNumT4;
		
		for (int i = 0; i < boxNum; ++ i)
		{
			Box b = bv[i];
			inside.assign(gateNum, false);
			int insideGateNum = 0;
			for (int j = 0; j < gateNum; ++ j)
			{
				Gate *g = chipGates[j];
				abkassert(j == g->getId(), "Error in addModulesEachGateNum()");
				if (g->insideBox(b))
				{
					inside[j] = true;
					++ insideGateNum;
				}
			}
			if (!insideGateNum)
				continue;
			
			double sumG = 0;
			double sumT1 = 0;//Type I: (graph edge model with differentiated pin directions) counts each source-sink connection crossing the boundary as one pin.
			double sumT2 = 0;//Type II: (hyper edge model with differentiated pin directions) counts all source-sink connections of the same net crossing the boundary as one pin.
			double sumT3 = 0;//Type III: (graph edge model without pin directions) counts each connection crossing the boundary as one pin.
			countTerms(inside, sumG, sumT1, sumT2, sumT3);
			
			if (sumT1 <= 0)
				sumT1 = 1;
			if (sumT2 <= 0)
				sumT2 = 1;
			if (sumT3 <= 0)
				sumT3 = 1;
			
			gateNumT1[insideGateNum].push_back(sumT1);
			gateNumT2[insideGateNum].push_back(sumT2);
			gateNumT3[insideGateNum].push_back(sumT3);
		}
		
		for (IntIVMapItr itr = gateNumT1.begin(); itr != gateNumT1.end(); ++ itr)
		{
			int gateNum = (*itr).first;
			
	    double avgG = (double)gateNum;
	    double geomAvgG = (double)gateNum;
	    double avgT1 = 0, avgT2 = 0, avgT3 = 0;
	    double geomAvgT1 = 1.0, geomAvgT2 = 1.0, geomAvgT3 = 1.0;
			{
				IntVector & terms = gateNumT1[gateNum];
				double termNum = (double)terms.size();
				abkassert(termNum > 0, "Error in terminal number in addModulesEachGateNum()");
				double expo = 1.0/termNum;
	
				for (int i = 0; i < termNum; ++ i)
				{
					double num = (double)terms[i];
					if (num >= 1)
					{
						avgT1 += num*expo;
						geomAvgT1 *= pow(num, expo);
					}
				}
		  }
			{
				IntVector & terms = gateNumT2[gateNum];
				double termNum = (double)terms.size();
				abkassert(termNum > 0, "Error in addModulesEachGateNum()");
				double expo = 1.0/termNum;
	
				for (int i = 0; i < termNum; ++ i)
				{
					double num = (double)terms[i];
					if (num >= 1)
					{
						avgT2 += num*expo;
						geomAvgT2 *= pow(num, expo);
					}
				}
		  }
			{
				IntVector & terms = gateNumT3[gateNum];
				double termNum = (double)terms.size();
				abkassert(termNum > 0, "Error in addModulesEachGateNum()");
				double expo = 1.0/termNum;
	
				for (int i = 0; i < termNum; ++ i)
				{
					double num = (double)terms[i];
					if (num >= 1)
					{
						avgT3 += num*expo;
						geomAvgT3 *= pow(num, expo);
					}
				}
		  }
		  
			if (avgG >= 1 && geomAvgG >= 1 && avgT1 >= 1 && avgT2 >= 1 && avgT3 >= 1 && geomAvgT1 >= 1 && geomAvgT2 >= 1 && geomAvgT3 >= 1)
			{
		    Module *m = new Module(method->getModuleNum());
	    	if (m == NULL)
		  	{
		  		fprintf(stdout, "Error: out of memory in function addModulesEachGateNum(). \nExisting ...\n");
		  		exit(-1);
		  	}
		    m->setAvgG(avgG);
		    m->setGeomAvgG(geomAvgG);
		    m->setAvgT1(avgT1);
		    m->setAvgT2(avgT2);
		    m->setAvgT3(avgT3);
		    m->setGeomAvgT1(geomAvgT1);
		    m->setGeomAvgT2(geomAvgT2);
		    m->setGeomAvgT3(geomAvgT3);
		    
		    method->addModule(m);
		  }
	  }
	}

	void Design::addModulesEachRectSize(BoxVector &bv, Method *method)
	{
		BoolVector inside;
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
		int boxNum = bv.size();
		abkassert(boxNum > 0, "Error in addModulesEachRectSize()");
		double geomAvgG = 1;
		double geomAvgT1 = 1;
		double geomAvgT2 = 1;
		double geomAvgT3 = 1;
		double expo = 1.0/boxNum;
		double avgG = 0;
		double avgT1 = 0;//Type I: (graph edge model with differentiated pin directions) counts each source-sink connection crossing the boundary as one pin;
		double avgT2 = 0;//Type II: (hyper edge model with differentiated pin directions) counts all source-sink connections of the same net crossing the boundary as one pin;
		double avgT3 = 0;//Type III: (graph edge model without pin directions) counts each connection crossing the boundary as one pin;
		
		for (int i = 0; i < boxNum; ++ i)
		{
			Box b = bv[i];
			inside.assign(gateNum, false);
			for (int j = 0; j < gateNum; ++ j)
			{
				Gate *g = chipGates[j];
				abkassert(j == g->getId(), "Error in addModulesEachRectSize()");
				if (g->insideBox(b))
				{
					inside[j] = true;
				}
			}
			
			double sumG = 0, sumT1 = 0, sumT2 = 0, sumT3 = 0;
			countTerms(inside, sumG, sumT1, sumT2, sumT3);
			
			if (sumG >= 1.0 && sumT1 >= 1.0 && sumT2 >= 1.0 && sumT3 >= 1.0)
			{
				geomAvgG *= pow(sumG, expo);
				geomAvgT1 *= pow(sumT1, expo);
				geomAvgT2 *= pow(sumT2, expo);
				geomAvgT3 *= pow(sumT3, expo);
				avgG += sumG * expo;
				avgT1 += sumT1 * expo;
				avgT2 += sumT2 * expo;
				avgT3 += sumT3 * expo;
			}
		}
		
		if (avgG >= 1 && geomAvgG >= 1 && avgT1 >= 1 && avgT2 >= 1 && avgT3 >= 1 && geomAvgT1 >= 1 && geomAvgT2 >= 1 && geomAvgT3 >= 1)
		{
	    Module *m = new Module(method->getModuleNum());
    	if (m == NULL)
	  	{
	  		fprintf(stdout, "Error: out of memory in function addModulesEachRectSize(). \nExisting ...\n");
	  		exit(-1);
	  	}
	    m->setAvgG(avgG);
	    m->setGeomAvgG(geomAvgG);
	    m->setAvgT1(avgT1);
	    m->setAvgT2(avgT2);
	    m->setAvgT3(avgT3);
	    m->setGeomAvgT1(geomAvgT1);
	    m->setGeomAvgT2(geomAvgT2);
	    m->setGeomAvgT3(geomAvgT3);
	    
	    method->addModule(m);
	  }
	}
	
	void Design::compAvgK()
	{
    GateVector &chipGates = chip.getGates();
		double gateNum = (double)chipGates.size();
		double pinNum = 0;
		m_geomAvgK = 1;
		abkassert(gateNum > 0, "Error in compAvgK()");
		double expo = 1.0/gateNum;
  	for (int i = 0; i < chipGates.size(); ++ i)
  	{
  		Gate *g = chipGates[i];
//  		fprintf(stdout, "Gate %d\n", g->getId());
  		if (g->getEffPinNum() >= 1)
  		{
	  		pinNum += g->getEffPinNum();
				m_geomAvgK *= pow((double)g->getEffPinNum(), expo);
			}
  	}
  	
		m_avgK = pinNum/gateNum;
		if (m_verb.getForMajStats() > 0)
		{
			fprintf(stdout, "Arithmetic average K: %f, geometric average K: %f\n", m_avgK, m_geomAvgK);
		}
	}
	
	void Design::countTerms(BoolVector &inside, double &sumG, double &sumT1, double &sumT2, double &sumT3)
	{
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
    sumG = sumT1 = sumT2 = sumT3 = 0;
//				cout<<"cellNums[cellNumIndex]: "<<cellNums[cellNumIndex]<<endl;
		//compute the T1 and T2 terms
		NetSet type2Nets;
		for (int j = 0; j < gateNum; ++ j)
		{
			if (inside[j])
			{
				sumG += 1;
				Gate *g = chipGates[j];
				sumT1 += g->getType1Terms(inside);
				//for type 2
				for (int k = 0; k < g->getNetNum(); ++ k)
				{
					type2Nets.insert(g->getNet(k));
				}
				sumT3 += g->getType3Terms(inside);
			}
		}
		sumT2 = getType2Terms(inside, type2Nets);
//		//verify that no two nets are the same in the set
//		{
//			IntIntMap nIds;
//			for (NetSetItr itr = type2Nets.begin(); itr != type2Nets.end(); ++ itr)
//			{
//				Net *n = (*itr);
//				abkassert(nIds[n->getId()] <= 0, "Error in computed type 2 nets");
//				nIds[n->getId()] += 1;
//			}
//		}
//		cout<<"net verified"<<endl;
		
//				fprintf(stdout, "cellNum in the module: %d, sumT1: %f, sumT2: %f, sumT3: %f\n", num, sumT1, sumT2, sumT3);
		
		if (sumG <= 0 || sumT1 <= 0 || sumT2 <= 0 || sumT3 <= 0)
		{
			fprintf(stdout, "Warning: in countTerms(), sumT1: %f, sumT2: %f, sumT3: %f\n", sumT1, sumT2, sumT3);
		}
		
		if (sumG <= 0)
			sumG = 1.0;
		if (sumT1 <= 0)
			sumT1 = 1.0;
		if (sumT2 <= 0)
			sumT2 = 1.0;
		if (sumT3 <= 0)
			sumT3 = 1.0;
	}

	//priority_queue cannot dynamically update the ordering when its elements are changed
	//so have to include some redundancy
	void Design::compTermsClusteringNew2(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms)
	{
		abkassert(cellNums.size() > 0, "Error in number of designated modules");
		abkassert(cellNums[0] > 1, "Error in number of designated instances in the first module");
		GateVector &chipGates = chip.getGates();		
		int gateNum = chipGates.size();
    //initialize adjacent gate number out of the cluster
    for (int i = 0; i < gateNum; ++ i)
    {
    	chipGates[i]->initAdjNumOutCluster();
    }
    
		BoolVector inside(gateNum, false);
		Gate *sourceG = chipGates[gId];
		inside[gId] = true;
		for (int i = 0; i < sourceG->getAdjGateNum(); ++ i)
		{
			Gate *adjGate = sourceG->getAdjGate(i);
			int adjId = adjGate->getId();
			abkassert(!inside[adjId], "Error in compTermsClusteringNew2()");
			adjGate->decAdjNumOutCluster();
		}
		int num = 1;
		int cellNumIndex = 0;
    
		//add all remaining gates into the priority queue
		GatePQ pq, pq1;
		for (int i = 0; i < gateNum; ++ i)
		{
			if (i != gId)
				pq.push(chipGates[i]);
		}
		
		//extract one gate each time
		while (!pq.empty())
		{
    	Gate *g = pq.top();
    	pq.pop();
			int gIndex = g->getId();
    	if (!inside[gIndex])
    	{
				inside[gIndex] = true;
//				fprintf(stdout, "Gate %s is added into the cluster, number of connections increased when added into the cluster: %d\n", g->getName().c_str(), g->getIncConNum());
				
//				pq1 = pq;
//				while (!pq1.empty())
//				{
//		    	Gate *tmpG = pq1.top();
//		    	fprintf(stdout, "Gate %s, number of connections increased when added into the cluster: %d\n", tmpG->getName().c_str(), tmpG->getIncConNum());
//		    	pq1.pop();
//		    }
//				getchar();
				
				if (++ num == cellNums[cellNumIndex])
				{
					if (m_verb.getForMajStats() > 0)
					{
						fprintf(stdout, "One module generated with %d instances, queue size: %d\n", num, pq.size());
					}
					if(m_verb.getForSysRes() > 2)
					{
						reportMem();
			    }
			    
					double sumG = 0, sumT1 = 0, sumT2 = 0, sumT3 = 0;
					countTerms(inside, sumG, sumT1, sumT2, sumT3);
					T1Terms.push_back(sumT1);
					T2Terms.push_back(sumT2);
					T3Terms.push_back(sumT3);
					if (++cellNumIndex >= cellNums.size())
					{
						pq = pq1;
						break;
					}
				}
				
				for (int i = 0; i < g->getAdjGateNum(); ++ i)
				{
					Gate *adjGate = g->getAdjGate(i);
					int adjId = adjGate->getId();
					if (!inside[adjId])
					{
						adjGate->decAdjNumOutCluster();
						pq.push(adjGate);
					}
				}
			}
		}
	}

	//priority_queue cannot dynamically update the ordering when its elements are changed
	//so have to include some redundancy
	void Design::compTermsClusteringNew(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms)
	{
		abkassert(cellNums.size() > 0, "Error in number of designated modules");
		abkassert(cellNums[0] > 1, "Error in number of designated instances in the first module");
		GateVector &chipGates = chip.getGates();
//		IntPQ ipq;
//    for (int i = 0; i < 20; ++ i)
//    {
//    	ipq.push(i);
//    }
//		while (!ipq.empty())
//		{
//    	int r = ipq.top();
//    	fprintf(stdout, "number: %d\n", r);
//    	ipq.pop();
//    }
//    exit(0);
		
		int gateNum = chipGates.size();
    //initialize adjacent gate number out of the cluster
    for (int i = 0; i < gateNum; ++ i)
    {
    	chipGates[i]->initIncConNum();
    }
    
		BoolVector inside(gateNum, false);
		Gate *sourceG = chipGates[gId];
		inside[gId] = true;
		for (int i = 0; i < sourceG->getNetNum(); ++ i)
		{
			int gatesIn = 0;
			int gatesOut = 0;
			Net *adjNet = sourceG->getNet(i);
			for (int j = 0; j < adjNet->getPinNum(); ++ j)
			{
				Pin *p = adjNet->getPin(j);
				if (inside[p->getGate()->getId()])
					++ gatesIn;
				else
					++ gatesOut;
			}
			if (gatesIn == 1)
			{
				for (int j = 0; j < adjNet->getPinNum(); ++ j)
				{
					Pin *p = adjNet->getPin(j);
					Gate *tmpGate = p->getGate();
					if (!inside[tmpGate->getId()])
						tmpGate->decIncConNum();
				}
			}
			if (gatesOut == 1)
			{
				for (int j = 0; j < adjNet->getPinNum(); ++ j)
				{
					Pin *p = adjNet->getPin(j);
					Gate *tmpGate = p->getGate();
					if (!inside[tmpGate->getId()])
						tmpGate->decIncConNum();
				}
			}
		}
		
		int num = 1;
		int cellNumIndex = 0;
    
		//add all remaining gates into the priority queue
		GateIncConPQ pq, pq1;
		for (int i = 0; i < gateNum; ++ i)
		{
			if (i != gId)
				pq.push(chipGates[i]);
		}
		
		//extract one gate each time
		while (!pq.empty())
		{
    	Gate *g = pq.top();
    	pq.pop();
			int gIndex = g->getId();
    	if (!inside[gIndex])
    	{
				inside[gIndex] = true;
//				fprintf(stdout, "Gate %s is added into the cluster, number of connections increased when added into the cluster: %d\n", g->getName().c_str(), g->getIncConNum());
				
//				pq1 = pq;
//				while (!pq1.empty())
//				{
//		    	Gate *tmpG = pq1.top();
//		    	fprintf(stdout, "Gate %s, number of connections increased when added into the cluster: %d\n", tmpG->getName().c_str(), tmpG->getIncConNum());
//		    	pq1.pop();
//		    }
//				getchar();
				
				if (++ num == cellNums[cellNumIndex])
				{
					if (m_verb.getForMajStats() > 0)
					{
						fprintf(stdout, "One module generated with %d instances, queue size: %d\n", num, pq.size());
					}
					if(m_verb.getForSysRes() > 2)
					{
						reportMem();
			    }
			    
					double sumG = 0, sumT1 = 0, sumT2 = 0, sumT3 = 0;
					countTerms(inside, sumG, sumT1, sumT2, sumT3);
					T1Terms.push_back(sumT1);
					T2Terms.push_back(sumT2);
					T3Terms.push_back(sumT3);
					if (++cellNumIndex >= cellNums.size())
					{
						pq = pq1;
						break;
					}
				}
				
				GateSet gateSet;
				for (int i = 0; i < g->getNetNum(); ++ i)
				{
					int gatesIn = 0;
					int gatesOut = 0;
					Net *adjNet = g->getNet(i);
					for (int j = 0; j < adjNet->getPinNum(); ++ j)
					{
						Pin *p = adjNet->getPin(j);
						if (inside[p->getGate()->getId()])
							++ gatesIn;
						else
							++ gatesOut;
					}
					if (gatesIn == 1)
					{
						for (int j = 0; j < adjNet->getPinNum(); ++ j)
						{
							Pin *p = adjNet->getPin(j);
							Gate *tmpGate = p->getGate();
							if (!inside[tmpGate->getId()])
							{
								tmpGate->decIncConNum();
								gateSet.insert(tmpGate);
							}
						}
					}
					if (gatesOut == 1)
					{
						for (int j = 0; j < adjNet->getPinNum(); ++ j)
						{
							Pin *p = adjNet->getPin(j);
							Gate *tmpGate = p->getGate();
							if (!inside[tmpGate->getId()])
							{
								tmpGate->decIncConNum();
								gateSet.insert(tmpGate);
							}
						}
					}
				}
				
				for (GateSetItr itr = gateSet.begin(); itr != gateSet.end(); ++ itr)
				{
					Gate *tmpG = (*itr);
					pq.push(tmpG);
				}
			}
		}
	}
	
	void Design::compTermsClustering(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms)
	{
		abkassert(cellNums.size() > 0, "Error in number of designated modules");
		abkassert(cellNums[0] > 1, "Error in number of designated instances in the first module");
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
    //initialize adjacent gate number out of the cluster
//    //test PQ
//		GatePQ testPQ;
    for (int i = 0; i < gateNum; ++ i)
    {
    	chipGates[i]->initAdjNumOutCluster();
//    	testPQ.push(chipGates[i]);
    }
//		while (!testPQ.empty())
//		{
//    	Gate *g = testPQ.top();
//    	fprintf(stdout, "adjnum: %d, adjNum out of cluster: %d\n", g->getAdjGateNum(), g->getAdjNumOutCluster());
//    	testPQ.pop();
//    }
//    exit(0);
    
		BoolVector inside(gateNum, false), inQueue(gateNum, false);
		GatePQ pq, pq1;
		pq.push(chipGates[gId]);
		int cellNumIndex = 0;
		int num = 0;
    
		//start from gate gId, perform BFS search
		while (!pq.empty())
		{
    	Gate *g = pq.top();
			int gIndex = g->getId();
    	pq.pop();
			if (!inside[gIndex])
			{
				inside[gIndex] = true;
				
//				fprintf(stdout, "Gate %s is added into the cluster, number of gates out of the cluster: %d\n", g->getName().c_str(), g->getAdjNumOutCluster());
//				pq1 = pq;
//				while (!pq1.empty())
//				{
//		    	Gate *tmpG = pq1.top();
//		    	fprintf(stdout, "Gate %s, number of gates out of the cluster: %d\n", tmpG->getName().c_str(), tmpG->getAdjNumOutCluster());
//		    	pq1.pop();
//		    }
//				getchar();
				
				if (++ num == cellNums[cellNumIndex])
				{
					if (m_verb.getForMajStats() > 0)
					{
						fprintf(stdout, "One module generated with %d instances, queue size: %d\n", num, pq.size());
					}
					if(m_verb.getForSysRes() > 2)
					{
						reportMem();
			    }
			    
					double sumG = 0, sumT1 = 0, sumT2 = 0, sumT3 = 0;
					countTerms(inside, sumG, sumT1, sumT2, sumT3);
					T1Terms.push_back(sumT1);
					T2Terms.push_back(sumT2);
					T3Terms.push_back(sumT3);
					
					if (++cellNumIndex >= cellNums.size())
					{
						pq = pq1;
						break;
					}
				}
				
				for (int i = 0; i < g->getAdjGateNum(); ++ i)
				{
					Gate *adjGate = g->getAdjGate(i);
					int adjId = adjGate->getId();
					if (!inside[adjId])
					{
						adjGate->decAdjNumOutCluster();
						pq.push(adjGate);
					}
				}
			}
			
			if (pq.empty())
			{
				if (m_verb.getForMajStats() > 0)
				{
					fprintf(stdout, "Disjoint clusters found!\n");
				}
		    for (int i = 0; i < gateNum; ++ i)
		    {
		    	if (!inside[i])
		    	{
		    		pq.push(chipGates[i]);
		    		break;
		    	}
		    }
			}
		}
	}
	
	void Design::compTermsBFS(int gId, IntVector &cellNums, DVector &T1Terms, DVector &T2Terms, DVector &T3Terms)
	{
		abkassert(cellNums.size() > 0, "Error in number of designated modules");
		abkassert(cellNums[0] > 1, "Error in number of designated instances in the first module");
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
		BoolVector inside(gateNum, false);
		IntList live;
		live.push_back(gId);
		inside[gId] = true;
		int cellNumIndex = 0;
		int num = 1;
		//start from gate gId, perform BFS search
		while (live.size())
		{
			int gIndex = live.front();
			live.pop_front();
			Gate *g = chipGates[gIndex];
//			fprintf(stdout, "Gate %s is visited\n", g->getName().c_str());
			abkassert(inside[gIndex], "Error in BFS search");
			
			for (int i = 0; i < g->getAdjGateNum(); ++ i)
			{
				int gateId = g->getAdjGate(i)->getId();
				if (!inside[gateId])
				{
					live.push_back(gateId);
					inside[gateId] = true;
					if (++ num == cellNums[cellNumIndex])
					{
						if (m_verb.getForMajStats() > 0)
						{
							fprintf(stdout, "One module generated with %d instances, queue size: %d\n", num, live.size());
						}
						if(m_verb.getForSysRes() > 2)
						{
							reportMem();
				    }
						double sumG = 0, sumT1 = 0, sumT2 = 0, sumT3 = 0;
						countTerms(inside, sumG, sumT1, sumT2, sumT3);
						T1Terms.push_back(sumT1);
						T2Terms.push_back(sumT2);
						T3Terms.push_back(sumT3);
						if (++cellNumIndex >= cellNums.size())
							goto OUT;
					}
				}
			}
			
			if (live.size() == 0)
			{
				if (m_verb.getForMajStats() > 0)
				{
					fprintf(stdout, "Disjoint clusters found!\n");
				}
		    for (int i = 0; i < gateNum; ++ i)
		    {
		    	if (!inside[i])
		    	{
		    		live.push_back(i);
		    		inside[i] = true;
						if (++ num == cellNums[cellNumIndex])
						{
							if (m_verb.getForMajStats() > 0)
							{
								fprintf(stdout, "One module generated with %d instances, queue size: %d\n", num, live.size());
							}
							if(m_verb.getForSysRes() > 2)
							{
								reportMem();
					    }
							double sumG = 0, sumT1 = 0, sumT2 = 0, sumT3 = 0;
							countTerms(inside, sumG, sumT1, sumT2, sumT3);
							T1Terms.push_back(sumT1);
							T2Terms.push_back(sumT2);
							T3Terms.push_back(sumT3);
							if (++cellNumIndex >= cellNums.size())
								goto OUT;
						}
		    		break;
		    	}
		    }
			}
		}

OUT: 	;
	}

	void Design::graphTraversalRent()
  {
  	int sampleNum = m_sampleNumGT;
		int clusterSizeNum = m_sampleSizeNum;
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
    abkassert(gateNum > 2, "Error in gate number in graphTraversalRent()");
    abkassert(clusterSizeNum > 0, "Error in cluster size in graphTraversalRent()");
  	double step = (gateNum - 2)/(double)clusterSizeNum;
  	if (step < 1)
  		step = 1;
		
  	Method *method1 = new Method(methods.size());
  	if (method1 == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in function graphTraversalRent(). \nExisting ...\n");
  		exit(-1);
  	}
  	methods.push_back(method1);
  	char *msg1 = "\n========= Graph traversal based method I =========";
  	method1->setMSG(msg1);
  	
  	Method *method2 = new Method(methods.size());
  	if (method2 == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in function graphTraversalRent(). \nExisting ...\n");
  		exit(-1);
  	}
  	methods.push_back(method2);
  	char *msg2 = "\n========= Graph traversal based method II =========";
  	method2->setMSG(msg2);
  	
    abkassert(sampleNum > 0, "Error in sampleNum in graphTraversalRent()");
  	double expo = 1.0/sampleNum;
  	DVector avgT1V1, avgT2V1, avgT3V1, geomAvgT1V1, geomAvgT2V1, geomAvgT3V1;
  	DVector avgT1V2, avgT2V2, avgT3V2, geomAvgT1V2, geomAvgT2V2, geomAvgT3V2;
  	IntVector cellNums;
  	
  	for (int i = 0; i < clusterSizeNum; ++ i)
  	{
  		int cellNum = (int)(2+step*i);
  		if (cellNum >= gateNum)
  			break;
  		int curSize = cellNums.size();
  		if (curSize == 0 || cellNum > cellNums[curSize-1])
  			cellNums.push_back(cellNum);
  	}
  	clusterSizeNum = cellNums.size();
  	avgT1V1.assign(clusterSizeNum, 0);
  	avgT2V1.assign(clusterSizeNum, 0);
  	avgT3V1.assign(clusterSizeNum, 0);
  	geomAvgT1V1.assign(clusterSizeNum, 1.0);
  	geomAvgT2V1.assign(clusterSizeNum, 1.0);
  	geomAvgT3V1.assign(clusterSizeNum, 1.0);
  	avgT1V2.assign(clusterSizeNum, 0);
  	avgT2V2.assign(clusterSizeNum, 0);
  	avgT3V2.assign(clusterSizeNum, 0);
  	geomAvgT1V2.assign(clusterSizeNum, 1.0);
  	geomAvgT2V2.assign(clusterSizeNum, 1.0);
  	geomAvgT3V2.assign(clusterSizeNum, 1.0);
		
		if (!SeedHandler::isInitialized())
		{
			SeedHandler::turnOffLogging();
			SeedHandler::overrideExternalSeed(m_seedGT);
		}
		RandomUnsigned r(0, gateNum, m_seedGT);
  	
  	for (int j = 0; j < sampleNum; ++ j)
  	{
			if (m_verb.getForActions() > 0)
			{
				fprintf(stdout, "Compute sample %d ...\n", j);
			}
			if(m_verb.getForSysRes() > 1)
			{
				reportMem();
	    }
  		int gateId = r;
  		
			DVector T1Terms1, T2Terms1, T3Terms1, T1Terms2, T2Terms2, T3Terms2;
//	  	compTermsBFS(gateId, cellNums, T1Terms, T2Terms, T3Terms);
			if (m_verb.getForActions() > 0)
			{
				fprintf(stdout, "Graph traversal method I ...\n", j);
			}
	  	compTermsClustering(gateId, cellNums, T1Terms1, T2Terms1, T3Terms1);
			if (m_verb.getForActions() > 0)
			{
				fprintf(stdout, "Graph traversal method II ...\n", j);
			}
	  	compTermsClusteringNew(gateId, cellNums, T1Terms2, T2Terms2, T3Terms2);
//	  	compTermsClusteringNew2(gateId, cellNums, T1Terms, T2Terms, T3Terms);
	  	
	  	abkassert(T1Terms1.size() == clusterSizeNum && T2Terms1.size() == clusterSizeNum && T3Terms1.size() == clusterSizeNum, "Error in number of terms in graphTraversalRent()");
	  	abkassert(T1Terms2.size() == clusterSizeNum && T2Terms2.size() == clusterSizeNum && T3Terms2.size() == clusterSizeNum, "Error in number of terms in graphTraversalRent()");
	  	for (int i = 0; i < clusterSizeNum; ++ i)
	  	{
	  		if (T1Terms1[i] >= 1 && T2Terms1[i] >= 1 && T3Terms1[i] >= 1)
	  		{
	  			geomAvgT1V1[i] *= pow(T1Terms1[i], expo);
	  			geomAvgT2V1[i] *= pow(T2Terms1[i], expo);
	  			geomAvgT3V1[i] *= pow(T3Terms1[i], expo);
		  		avgT1V1[i] += T1Terms1[i]*expo;
		  		avgT2V1[i] += T2Terms1[i]*expo;
		  		avgT3V1[i] += T3Terms1[i]*expo;
		  	}
	  		if (T1Terms2[i] >= 1 && T2Terms2[i] >= 1 && T3Terms2[i] >= 1)
	  		{
	  			geomAvgT1V2[i] *= pow(T1Terms2[i], expo);
	  			geomAvgT2V2[i] *= pow(T2Terms2[i], expo);
	  			geomAvgT3V2[i] *= pow(T3Terms2[i], expo);
		  		avgT1V2[i] += T1Terms2[i]*expo;
		  		avgT2V2[i] += T2Terms2[i]*expo;
		  		avgT3V2[i] += T3Terms2[i]*expo;
		  	}
	  	}
  	}
  	
  	for (int i = 0; i < clusterSizeNum; ++ i)
  	{
  		double avgG = (double)cellNums[i];
  		double geomAvgG = (double)cellNums[i];
  		double avgT11 = avgT1V1[i], avgT21 = avgT2V1[i], avgT31 = avgT3V1[i];
  		double geomAvgT11 = geomAvgT1V1[i], geomAvgT21 = geomAvgT2V1[i], geomAvgT31 = geomAvgT3V1[i];

  		double avgT12 = avgT1V2[i], avgT22 = avgT2V2[i], avgT32 = avgT3V2[i];
  		double geomAvgT12 = geomAvgT1V2[i], geomAvgT22 = geomAvgT2V2[i], geomAvgT32 = geomAvgT3V2[i];
  		
			if (avgG >= 1 && geomAvgG >= 1 && avgT11 >= 1 && avgT21 >= 1 && avgT31 >= 1 && geomAvgT11 >= 1 && geomAvgT21 >= 1 && geomAvgT31 >= 1)
			{
		    Module *m = new Module(method1->getModuleNum());
		  	if (m == NULL)
		  	{
		  		fprintf(stdout, "Error: out of memory in function graphTraversalRent(). \nExisting ...\n");
		  		exit(-1);
		  	}
		    m->setAvgG(avgG);
		    m->setGeomAvgG(geomAvgG);
		    m->setAvgT1(avgT11);
		    m->setAvgT2(avgT21);
		    m->setAvgT3(avgT31);
		    m->setGeomAvgT1(geomAvgT11);
		    m->setGeomAvgT2(geomAvgT21);
		    m->setGeomAvgT3(geomAvgT31);
		    
//		    m->print();
		    method1->addModule(m);
		  }
		  
			if (avgG >= 1 && geomAvgG >= 1 && avgT12 >= 1 && avgT22 >= 1 && avgT32 >= 1 && geomAvgT12 >= 1 && geomAvgT22 >= 1 && geomAvgT32 >= 1)
			{
		    Module *m = new Module(method2->getModuleNum());
		  	if (m == NULL)
		  	{
		  		fprintf(stdout, "Error: out of memory in function graphTraversalRent(). \nExisting ...\n");
		  		exit(-1);
		  	}
		    m->setAvgG(avgG);
		    m->setGeomAvgG(geomAvgG);
		    m->setAvgT1(avgT12);
		    m->setAvgT2(avgT22);
		    m->setAvgT3(avgT32);
		    m->setGeomAvgT1(geomAvgT12);
		    m->setGeomAvgT2(geomAvgT22);
		    m->setGeomAvgT3(geomAvgT32);
		    
//		    m->print();
		    method2->addModule(m);
		  }
  	}
  	//make a module containing all the gates

    int padNum = pads.size();

		if (gateNum >= 1 && padNum >= 1)
		{
	    Module *m = new Module(method1->getModuleNum());
	  	if (m == NULL)
	  	{
	  		fprintf(stdout, "Error: out of memory in function graphTraversalRent(). \nExisting ...\n");
	  		exit(-1);
	  	}
	    m->setAvgG((double)gateNum);
	    m->setGeomAvgG((double)gateNum);
	    m->setAvgT1((double)padNum);
	    m->setAvgT2((double)padNum);
	    m->setAvgT3((double)padNum);
	    m->setGeomAvgT1((double)padNum);
	    m->setGeomAvgT2((double)padNum);
	    m->setGeomAvgT3((double)padNum);
//	    m->print();
			method1->addModule(m);
	    method1->setAvgK(m_avgK);
	    method1->setGeomAvgK(m_geomAvgK);
	  	method1->linCurvFit();
	  	
	    m = new Module(method2->getModuleNum());
	  	if (m == NULL)
	  	{
	  		fprintf(stdout, "Error: out of memory in function graphTraversalRent(). \nExisting ...\n");
	  		exit(-1);
	  	}
	    m->setAvgG((double)gateNum);
	    m->setGeomAvgG((double)gateNum);
	    m->setAvgT1((double)padNum);
	    m->setAvgT2((double)padNum);
	    m->setAvgT3((double)padNum);
	    m->setGeomAvgT1((double)padNum);
	    m->setGeomAvgT2((double)padNum);
	    m->setGeomAvgT3((double)padNum);
//	    m->print();
			method2->addModule(m);
	    method2->setAvgK(m_avgK);
	    method2->setGeomAvgK(m_geomAvgK);
	  	method2->linCurvFit();
	  }
//  	method->print(stdout);
  }
	
	void Design::rectSamplingRent()
  {
  	int rectSizeNum = m_sampleSizeNum;
  	int sampleNum = m_sampleNumRS;
  	
    GateVector &chipGates = chip.getGates();
		int chipWidth = chip.getChipX();
		int chipHeight = chip.getChipY();
		abkassert(chipHeight > 0, "chipHeight <= 0 in rectSamplingRent");
		double ratio = chipWidth/(double)chipHeight;
    int chipLX = chip.getLX();
    int chipRX = chip.getRX();
    int chipBY = chip.getBY();
    int chipTY = chip.getTY();
    int cellWidth = chipGates[0]->getWidth();
    int cellHeight = chipGates[0]->getHeight();
//  	cout<<"cellWidth: "<<cellWidth<<" cellHeight: "<<cellHeight<<endl;
		abkassert(ratio > 0, "ratio <= 0 in rectSamplingRent");
    int minRectH = (int)sqrt((double)cellWidth*cellHeight*2/ratio);
    int minRectW = (int)(minRectH * ratio);
//  	cout<<"minRectW: "<<minRectW<<" minRectH: "<<minRectH<<endl;
		abkassert(rectSizeNum > 0, "Error in rectSizeNum in rectSamplingRent()");
    double deltaH = (chipHeight-minRectH)/(double)rectSizeNum;
    double deltaW = (chipWidth-minRectW)/(double)rectSizeNum;
//  	cout<<"deltaW: "<<deltaW<<" deltaH "<<deltaH<<endl;
    
  	Method *method1 = new Method(methods.size());
  	if (method1 == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in function rectSamplingRent(). \nExisting ...\n");
  		exit(-1);
  	}
  	methods.push_back(method1);
  	Method *method2 = new Method(methods.size());
  	if (method2 == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in function rectSamplingRent(). \nExisting ...\n");
  		exit(-1);
  	}
  	methods.push_back(method2);
//  	char *msg1 = "\n========= Placement-based method -- Rectangle sampling method =========";
  	char *msg1 = "\n========= Placement-based method -- Rectangle sampling based method I =========";
  	char *msg2 = "\n========= Placement-based method -- Rectangle sampling based method II =========";
  	method1->setMSG(msg1);
  	method2->setMSG(msg2);
//  	fprintf(stdout, "CHIP [%d,%d] -> [%d,%d]\n", chipLX, chipBY, chipRX, chipTY);
		BoxVector allBoxes;

		if (!SeedHandler::isInitialized())
		{
			SeedHandler::turnOffLogging();
			SeedHandler::overrideExternalSeed(m_seedRS);
		}

  	for (int i = 0; i < rectSizeNum; ++ i)
  	{
  		int rectW = (int)(minRectW + deltaW*i);
  		int rectH = (int)(minRectH + deltaH*i);
			if(m_verb.getForActions() > 0)
			{
				fprintf(stdout, "Compute samples of rectangle size %d: W=%d, H=%d ...\n", i, rectW, rectH);
	    }
  		//randomly decide the bottom left corner of the rectangle
  		
  		BoxVector bv;
  		//inclusive for lower bound, strict for upper bound
			RandomUnsigned r1(0, chipRX-rectW-chipLX+1, m_seedRS);
			RandomUnsigned r2(0, chipTY-rectH-chipBY+1, m_seedRS);
			
  		for (int j = 0; j < sampleNum; j ++)
  		{
	  		int x = chipLX+r1;
	  		int y = chipBY+r2;
//    	cout<<"x: "<<x<<" y: "<<y<<endl;
//    	cout<<"rectW: "<<rectW<<" rectH: "<<rectH<<endl;
	  		abkassert(x >= chipLX && x + rectW <= chipRX, "Error in sample position in rectSamplingRent()");
	  		abkassert(y >= chipBY && y + rectH <= chipTY, "Error in sample position in rectSamplingRent()");
			  Box b(x, y, x+rectW, y+rectH);
			  bv.push_back(b);
			  allBoxes.push_back(b);
	  	}
	  	addModulesEachRectSize(bv, method1);
			if(m_verb.getForSysRes() > 1)
			{
				reportMem();
	    }
	  	
	  	bv.clear();
  	}
  	addModulesEachGateNum(allBoxes, method2);
  	allBoxes.clear();
  	//make a module containing all the gates
    int gateNum = chipGates.size();
    int padNum = pads.size();
		if (gateNum >= 1 && padNum >= 1)
		{
	    Module *m = new Module(method1->getModuleNum());
	  	if (m == NULL)
	  	{
	  		fprintf(stdout, "Error: out of memory in function rectSamplingRent(). \nExisting ...\n");
	  		exit(-1);
	  	}
	    m->setAvgG((double)gateNum);
	    m->setGeomAvgG((double)gateNum);
	    m->setAvgT1((double)padNum);
	    m->setAvgT2((double)padNum);
	    m->setAvgT3((double)padNum);
	    m->setGeomAvgT1((double)padNum);
	    m->setGeomAvgT2((double)padNum);
	    m->setGeomAvgT3((double)padNum);
			method1->addModule(m);
	    method1->setAvgK(m_avgK);
	    method1->setGeomAvgK(m_geomAvgK);
	  	method1->linCurvFit();
	  }
//  	method1->print(stdout);
    
		if (gateNum >= 1 && padNum >= 1)
		{
	    Module *m = new Module(method2->getModuleNum());
	  	if (m == NULL)
	  	{
	  		fprintf(stdout, "Error: out of memory in function rectSamplingRent(). \nExisting ...\n");
	  		exit(-1);
	  	}
	    m->setAvgG((double)gateNum);
	    m->setGeomAvgG((double)gateNum);
	    m->setAvgT1((double)padNum);
	    m->setAvgT2((double)padNum);
	    m->setAvgT3((double)padNum);
	    m->setGeomAvgT1((double)padNum);
	    m->setGeomAvgT2((double)padNum);
	    m->setGeomAvgT3((double)padNum);
			method2->addModule(m);
	    method2->setAvgK(m_avgK);
	    method2->setGeomAvgK(m_geomAvgK);
	  	method2->linCurvFit();
	  }
//  	method2->print(stdout);
  }

  //(hyper edge model with differentiated pin directions) counts all source-sink connections of the same net crossing the boundary as one pin;
	int Design::getType2Terms(BoolVector &inside, NetSet &netSet)
	{
		int terms = 0;
		
		for (NetSetItr itr = netSet.begin(); itr != netSet.end(); ++ itr)
		{
			Net *n = (*itr);
			
			if (n->getPadNum() > 0)
			{
				++ terms;
			}
			else
			{
				for (int j = 0; j < n->getPinNum(); ++ j)
				{
					Pin *p = n->getPin(j);
					if (!inside[p->getGate()->getId()])
					{
						++ terms;
						break;
					}
				}
			}
		}
		
		return terms;
	}

  void Design::saveRent()
  {
		FILE *f;
		char fileName[50];
		string defName = m_defNames[0].substr(0, m_defNames[0].length()-4);
		//XX
		sprintf(fileName, "RentParam.txt");
		//YY
		f = fopen(fileName, "w");
		
		if (f == NULL)
		{
			fprintf(stdout, "Error: cannot open file %s. Exiting ...", fileName);
			exit(-1);
		}
		
    for (int i = 0; i < methods.size(); ++ i)
    {
    	methods[i]->printRents(f);
    	methods[i]->printRents(stdout);
    }
    fprintf(f, "============= Detailed information for each module =============\n");
    for (int i = 0; i < methods.size(); ++ i)
    {
    	methods[i]->printModules(f);
    	methods[i]->printModules(stdout);
    }
    
		fclose(f);
  }
  
  void Design::evaluateRent()
  {
		if(m_verb.getForSysRes() > 0)
		{
			reportMem();
    }
    compAvgK();

		if (!m_noCP)
		{
			//------------------MLPart------------------------
			if (m_verb.getForActions() > 0)
			{
				fprintf(stdout, "\n\nStart to compute circuit partitioning based Rent parameter using MLPart ...\n");
			}
			if(m_verb.getForSysRes() > 0)
			{
				reportMem();
	    }
		  Timer totalTime1;
	    MLPartRent();
		  totalTime1.stop();
			if(m_verb.getForSysRes() > 0)
			{
				reportMem();
	    }
			if (m_verb.getForActions() > 0)
			{
			  cout<<"Circuit partitioning based Rent parameter computed in "<<totalTime1.getUserTime()<<" seconds"<<endl;
			}
		}
	  
		//------------------Graph traversal------------------------
		if (!m_noGT)
		{
			if (m_verb.getForActions() > 0)
			{
				fprintf(stdout, "\n\nStart to compute graph traversal based Rent parameter ...\n");
			}
			if(m_verb.getForSysRes() > 0)
			{
				reportMem();
	    }
		  Timer totalTime2;
	  	graphTraversalRent();
		  totalTime2.stop();
			if(m_verb.getForSysRes() > 0)
			{
				reportMem();
	    }
			if (m_verb.getForActions() > 0)
			{
			  cout<<"Graph traversal based Rent parameter computed in "<<totalTime2.getUserTime()<<" seconds"<<endl;
			}
	  }
	  
		//------------------Rectangle sampling------------------------
		if (!m_noRS)
		{
			if (m_verb.getForActions() > 0)
			{
				fprintf(stdout, "\n\nStart to compute rectangle sampling based Rent parameter ...\n");
			}
			if(m_verb.getForSysRes() > 0)
			{
				reportMem();
	    }
		  Timer totalTime3;
	    rectSamplingRent();
		  totalTime3.stop();
			if(m_verb.getForSysRes() > 0)
			{
				reportMem();
	    }
			if (m_verb.getForActions() > 0)
			{
			  cout<<"Rectangle sampling based Rent parameter computed in "<<totalTime3.getUserTime()<<" seconds"<<endl;
			}
		}
    saveRent();
    for (int i = 0; i < methods.size(); ++ i)
    {
    	methods[i]->printRents(stdout);
    }
  }
	
	CellMaster *Design::getCellMaster(string cellName)
	{
		if (cellMasterMap.find(cellName) == cellMasterMap.end())
		{
			fprintf(stderr, "Error: cell master %s does not exist in the LEF file\n", cellName.c_str());
			exit(-1);
		}
		return cellMasterMap[cellName];
	}
	
	/* 
	* arguable makes more sense to have a Verbosity constructor which takes 
	* an 'int'
	*/
	Verbosity buildVerbosity(int level) {
		char verbString[12];
		sprintf(verbString, "%d_%d_%d", level, level, level);
		Verbosity v(verbString);
		return v;
	}
	
//UMpack_mlpart(nvtxs, nedges, vwgts, eptr, eind, hewgts, 2, balanceArray, 0.1, parts, 3, 3, 0);
	int UMpack_mlpart(int nvtxs, int nhedges, double *vwgts,
										int *eptr, 
										int *eind,
										double *edgeWeights, int nparts, double *balanceArray, 
										double tolerance, int *part,
										int startsPerRun, int totalRuns,
										int debugLevel, unsigned seed)
	{
#ifdef DEBUG
		FILE *f;
		f = fopen("Problem.txt", "w");
		fprintf(f, "Total number of vertices: %d\n", nvtxs);
		fprintf(f, "Total number of edges: %d\n", nhedges);
		
    for (int i = 0; i < nhedges; ++ i)
    {
    	fprintf(f, "Edge %d: ", i);
    	for (int j = eptr[i]; j < eptr[i+1]; ++ j)
    		fprintf(f, " %d", eind[j]);
    	fprintf(f, "\n");
    }
		fclose(f);
#endif
		
		abkassert(nparts == 2, "nparts != 2, bipartitioning only supported operation");
		if (!SeedHandler::isInitialized())
		{
			SeedHandler::turnOffLogging();
			SeedHandler::overrideExternalSeed(seed);
		}
		bool debugBool = (debugLevel != 0);
		if (debugBool)
		{
			cout << " Constructing HGraph" << endl;
		}
		HGraphFixed hg(eptr, eind, edgeWeights, vwgts, nvtxs, eptr[nhedges], nhedges, debugBool);
		abkassert(hg.getNumNodes() == (unsigned) nvtxs, "nvtxs and hg.getNumNodes() doesn't match");
		if (debugBool)
		{
			cout << "done constructing " << endl;	
		}
		uofm::vector<BBox> partitions = uofm::vector<BBox>(nparts, BBox(Point(0,0)));
		uofm::vector<uofm::vector<double> > capacities= uofm::vector<uofm::vector<double> >	(nparts,uofm::vector<double>(1,-1));
		uofm::vector<double> tolerances = uofm::vector<double>(1);
		uofm::vector<unsigned> terminalsToBlock = uofm::vector<unsigned>(0);
		uofm::vector<BBox> padBlocks = uofm::vector<BBox>(nparts, BBox(Point(0,0)));
		tolerances[0] = tolerance;
		PartitioningBuffer solnBuffers=PartitioningBuffer(hg.getNumNodes(),1);
		PartitionIds nowhere;
		for(unsigned k=0;k!=hg.getNumTerminals();k++)
			solnBuffers[0][k]=nowhere;
		for(unsigned k = 0;k!=solnBuffers[0].size();k++)
		{
			if(part[k] < 0)
				solnBuffers[0][k].setToAll(nparts);
			else
				solnBuffers[0][k].addToPart(part[k]);
		}
		double totalSize;
		if (!vwgts)
			totalSize = (double) nvtxs;
		else
		{
			totalSize = 0;
			for(int i = 0; i < nvtxs; i++)
				totalSize += vwgts[i];
		}
		for(int i = 0; i < nparts; i++)
			capacities[i][0] = totalSize * balanceArray[i];
		Partitioning fixedConstr = Partitioning(hg.getNumNodes());
		for (unsigned k = 0; k < hg.getNumNodes(); k++)
		{
			int predefPart = part[k];
			if (predefPart < 0)
				fixedConstr[k].setToAll(nparts);
			else
			{
				abkfatal(predefPart < nparts, "predefined to invalid partition");
				fixedConstr[k].setToPart(predefPart);
			}
		}
		
		const HGraphFixed& chg = hg;
		const PartitioningBuffer&	csolnBuffers = solnBuffers;
		const Partitioning&		cfixedConstr = fixedConstr;
		const uofm::vector<BBox> cpartitions = partitions;
		const uofm::vector<uofm::vector<double> >	ccapacities = capacities;
		const uofm::vector<double>		ctolerances = tolerances;
		const uofm::vector<unsigned> 	cterminalToBlock = terminalsToBlock;
		const uofm::vector<BBox> 	cpadBlocks = padBlocks;

		PartitioningProblem problem (chg, csolnBuffers, cfixedConstr, partitions, capacities, tolerances, terminalsToBlock, padBlocks);

//		PartitioningProblem problem (chg, csolnBuffers, cfixedConstr, cpartitions, ccapacities, ctolerances, cterminalsToBlock, cpadBlocks);
		problem.reserveBuffers(startsPerRun);
		UnivPartEval costCheck(PartEvalType::NetCutWNetVec);
		vector<unsigned> bests;
		Partitioning bestSeen(problem.getHGraph().getNumNodes());
		unsigned bestSeenCost = UINT_MAX;
		int argc = 0;
		const char *argv[10];
		BaseMLPart::Parameters mlParams(argc, argv);
		
		Verbosity v = buildVerbosity(debugLevel);
		// problem.propagateTerminals(mlParams.partFuzziness);
		mlParams.Vcycling = MLPartParams::NoVcycles;
		mlParams.clParams.verb = v;
		mlParams.verb = v;
		
		BBPartBitBoardContainer bbBitBoards;
		MaxMem maxMem;
		
		for(int r = 0; r< totalRuns; r++)
		{
			if (debugLevel > 2)
				cout<<"-----   Run Number "<<r+1<<" of "<<totalRuns;
			MLPart mlPartitioner(problem, mlParams, bbBitBoards, &maxMem, (FillableHierarchy *) NULL);
			Partitioning& curPart = (problem.getSolnBuffers()[problem.getBestSolnNum()]);
			unsigned actualCost = costCheck.computeCost(problem, curPart);
			
			if (actualCost < bestSeenCost) 
			{
				bestSeen=curPart; 
				bestSeenCost=actualCost; 
			}
			if (debugLevel > 3)
			{
				cout<<"Net Cut: "<<actualCost<<endl;
				cout<<"Part Areas: "<<mlPartitioner.getPartitionArea(0)
				<< "  ("<< mlPartitioner.getPartitionNumNodes(0)<<" nonterminals) "
				<<"  /  "<<mlPartitioner.getPartitionArea(1)
				<< "  ("<< mlPartitioner.getPartitionNumNodes(1)<<" nonterminals) " <<endl;
			}
			bests.push_back(actualCost);
			
			//clear the buffer for the next run
			PartitioningBuffer &buff = problem.getSolnBuffers();
			for(unsigned s = buff.beginUsedSoln(); s != buff.endUsedSoln(); s++)
			{
				Partitioning &curBuff = buff[s];
				for(unsigned i = 0; i < curBuff.size(); i++)
					curBuff[i].setToAll(31);
			}
		}
		/* now assign back to the array */
		for(int i = 0; i < nvtxs; i++)
			part[i] = bestSeen[i].lowestNumPart();
		
		partitions.clear();
		capacities.clear();
		tolerances.clear();
		terminalsToBlock.clear();
		padBlocks.clear();
		
		if (debugLevel > 4)
			cout << "Returning from mlpart DLL";
		return bestSeenCost;
	}
	
	void Design::reportMem()
	{
		MemUsage mu;
		
		double availMem = VMemUsage::getPhysTotal();
		availMem = min(availMem, static_cast<double>(1UL << (sizeof(void*)*8 - 20)));
		
		cout << "==================Mem info=================="<<endl
					<<"Current memory usage is " << mu.getEstimate() << "MB" << endl
					<< "Available physical memory is " << availMem << "MB" << endl
					<<"============================================"<<endl;
	}
	
	//we do not consider the terminals during the partitioning
	void Design::MLPart(Cluster *cluster, ClusterVector &resultCV)
	{
		static int clusterIndex = 0;
		int levelIndex = cluster->getLevel();
		//construct the hyper graph
		//get the nets connected in the cluster
		IntIntMap clusterIdMap;
		
		BoolVector inside;
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
		inside.assign(gateNum, false);

		int clusterGateNum = cluster->getGateNum();
		abkassert(clusterGateNum >= 2, "clusterGateNum < 2 in MLPart()");
		NetSet clusterNets;
		for (int k = 0; k < clusterGateNum; ++ k)
		{
			Gate *g = cluster->getGate(k);
			inside[g->getId()] = true;
			clusterIdMap[g->getId()] = k+1;
			for (int l = 0; l < g->getNetNum(); ++ l)
			{
				Net *n = g->getNet(l);
				clusterNets.insert(n);
			}
		}
		
		double balanceArray[2];
		int nvtxs = cluster->getGateNum();
		int nedges = clusterNets.size();
		int cutNets = 0;
		balanceArray[0] = 0.5;
		balanceArray[1] = 0.5;

		if (m_verb.getForMajStats() > 0)
		{
			fprintf(stdout, "Total %d gates in cluster %d at level %d\n", nvtxs, clusterIndex++, levelIndex);
		}
		//assume the node ids start from 1
//		fprintf(stdout, "Original net number: %d\n", nedges);

		int *eptr = (int *) malloc((sizeof(int) * (nedges + 1)));
		if (!eptr)
		{
			fprintf(stderr, "Out of Mem in MLPart in allocating mem for eptr. Exit ...\n");
			exit(-1);
		}
		
		long eIndMax = nvtxs*3;
		int *eind = (int *)malloc((sizeof(int)) * eIndMax);
		if (!eind)
		{
			fprintf(stderr, "Out of Mem in MLPart in allocating mem for eind. Exit ...\n");
			exit(-1);
		}

		int edgeIndex = 0;
		int connIdx = 0;
		int degree;
		for (NetSetItr itr = clusterNets.begin(); itr != clusterNets.end(); ++ itr)
		{
			Net *n = (*itr);
			eptr[edgeIndex] = connIdx;
//			fprintf(stdout, "connIdx = %d: ", connIdx);
			
			degree = 0;
			for (int j = 0; j < n->getPinNum(); ++ j)
			{
				Pin *p = n->getPin(j);
				int gateId = p->getGate()->getId();
				if (inside[gateId])
				{
					degree ++;
					abkassert(clusterIdMap[gateId] > 0, "clusterIdMap[gateId] <= 0 in MLPart");
					if (connIdx >= eIndMax)
					{
						eIndMax += nvtxs*2;
						eind = (int *)realloc(eind, (sizeof(int)) * eIndMax);
						if (!eind)
						{
							fprintf(stderr, "Out of Mem in MLPart in allocating mem for eind. Exit ...\n");
							exit(-1);
						}
					}
					eind[connIdx++] = clusterIdMap[gateId]-1;
//					fprintf(stdout, "%d ", clusterIdMap[gateId]-1);
				}
			}
//			fprintf(stdout, "\n");
//			fprintf(stdout, "degree of the net: %d\n", degree);
			abkassert(degree >= 1, "degree < 1 in MLPart");
			if (degree > 1)
			{
				edgeIndex ++;
			}
			else
			{
				connIdx -= degree;
			}
		}
		eptr[edgeIndex] = connIdx;
		nedges = edgeIndex;
//		fprintf(stdout, "Finally eptr[%d] = %d\n", edgeIndex, connIdx);
		
		if (m_verb.getForMajStats() > 0)
		{
			fprintf(stdout, "number of edges for MLPart: %d\n", edgeIndex);
		}

		double *vwgts = (double *) malloc(sizeof(double) * nvtxs);
		if (!vwgts)
		{
			fprintf(stderr, "Out of Mem in MLPart in allocating mem for vwgts. Exit ...\n");
			exit(-1);
		}
		double *hewgts = (double *) malloc(sizeof(double) * nedges);
		if (!hewgts)
		{
			fprintf(stderr, "Out of Mem in MLPart in allocating mem for hewgts. Exit ...\n");
			exit(-1);
		}
		
		for(int i = 0; i < nvtxs; i++)
			vwgts[i] = 1.0;
		for(int i = 0; i < nedges; i++)
			hewgts[i] = 1.0;

		int *parts = (int *) malloc((sizeof(int) * nvtxs));
		if (!parts)
		{
			fprintf(stderr, "Out of Mem in MLPart in allocating mem for parts. Exit ...\n");
			exit(-1);
		}

		for(int i = 0; i  < nvtxs; i++)
			parts[i] = -1;
		
		if (nedges > 0)
		{
			cutNets = UMpack_mlpart(nvtxs, nedges, vwgts, eptr, eind, hewgts, 2, balanceArray, 0.1, parts, m_startsPerRun, m_totalRuns, 0, m_seedCP);
//			cutNets = UMpack_mlpart(nvtxs, nedges, vwgts, eptr, eind, hewgts, 2, balanceArray, 0.1, parts, 3, 3, 0);
			if (m_verb.getForMajStats() > 0)
			{
				fprintf(stdout, "Total %d nets cut with MLPart\n", cutNets);
			}
		}
		else
		{
			if (m_verb.getForMajStats() > 0)
			{
				fprintf(stdout, "Total 0 nets cut with MLPart\n");
			}
			for(int i = 0; i  < nvtxs; i++)
				parts[i] = i%2;
		}

		resultCV.clear();
		Cluster *c = new Cluster(0);
  	if (c == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in function MLPart(). \nExisting ...\n");
  		exit(-1);
  	}
  	c->setLevel(levelIndex+1);
		resultCV.push_back(c);
		c = new Cluster(0);
  	if (c == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in function MLPart(). \nExisting ...\n");
  		exit(-1);
  	}
  	c->setLevel(levelIndex+1);
		resultCV.push_back(c);
		for(int i = 0; i  < nvtxs; i++)
		{
			resultCV[parts[i]]->addGate(cluster->getGate(i));
//			fprintf(stdout, "Part of node %d: %d\n", i, parts[i]);
		}

//		//verify the partitioning results
//		int cuttedNets = 0;
//		for (NetSetItr itr = clusterNets.begin(); itr != clusterNets.end(); ++ itr)
//		{
//			Net *n = (*itr);
//			bool flag0 = false, flag1 = false;
//			for (int j = 0; j < n->getPinNum(); ++ j)
//			{
//				Pin *p = n->getPin(j);
//				int gateId = p->getGate()->getId();
//				if (inside[gateId])
//				{
//					if (parts[clusterIdMap[gateId]-1])
//						flag1 = true;
//					else
//						flag0 = true;
//				}
//			}
//			if (flag0 && flag1)
//			{
//				cuttedNets ++;
//				fprintf(stdout, "Net %s is cut\n", n->getName().c_str());
//			}
//		}
//		if (cuttedNets != cutNets)
//		{
//			fprintf(stdout, "ERROR: computed cutted nets: %d vs. MLPart reported cutted nets %d\n", cuttedNets, cutNets);
//			exit(-1);
//		}
		
		free(eptr);
		free(eind);
		free(vwgts);
		free(hewgts);
		free(parts);
	}
	
	bool Design::oneLevelPartition(ClusterVector &cv, Method *method)
	{
		int MIN_GATE_NUM_PER_CLUSTER = 5;
		bool flag = true;
		BoolVector inside;
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
		ClusterVector resultCV;
		int clusterNum = cv.size();
		double sampleNum = (double)clusterNum*2.0;
		abkassert(sampleNum > 0, "sampleNum <= 0 in oneLevelPartition()");
		double geomAvgG = 1;
		double geomAvgT1 = 1;
		double geomAvgT2 = 1;
		double geomAvgT3 = 1;
		double expo = 1.0/sampleNum;
		double avgG = 0;
		double avgT1 = 0;//Type I: (graph edge model with differentiated pin directions) counts each source-sink connection crossing the boundary as one pin;
		double avgT2 = 0;//Type II: (hyper edge model with differentiated pin directions) counts all source-sink connections of the same net crossing the boundary as one pin;
		double avgT3 = 0;//Type III: (graph edge model without pin directions) counts each connection crossing the boundary as one pin;
		
  	for (int i = 0; i < clusterNum; ++ i)
  	{
  		Cluster *c = cv[i];
  		resultCV.clear();
			MLPart(c, resultCV);
  		abkassert(resultCV.size() == 2, "resultCV.size() != 2 in oneLevelPartition");
  		cv.push_back(resultCV[0]);
  		cv.push_back(resultCV[1]);
	  	for (int j = 0; j < 2; ++ j)
	  	{
				Cluster *newC = resultCV[j];
				int newGateNum = newC->getGateNum();
				if (newGateNum < MIN_GATE_NUM_PER_CLUSTER)
					flag = false;
				
				inside.assign(gateNum, false);
		  	for (int k = 0; k < newGateNum; ++ k)
		  	{
					inside[newC->getGate(k)->getId()] = true;
		  	}
		  	
				double sumG = 0, sumT1 = 0, sumT2 = 0, sumT3 = 0;
				countTerms(inside, sumG, sumT1, sumT2, sumT3);
				if (sumG >= 1.0 && sumT1 >= 1.0 && sumT2 >= 1.0 && sumT3 >= 1.0)
				{
					geomAvgG *= pow(sumG, expo);
					geomAvgT1 *= pow(sumT1, expo);
					geomAvgT2 *= pow(sumT2, expo);
					geomAvgT3 *= pow(sumT3, expo);
					avgG += sumG * expo;
					avgT1 += sumT1 * expo;
					avgT2 += sumT2 * expo;
					avgT3 += sumT3 * expo;
				}
	  	}
  	}
		
		if (avgG >= 1 && geomAvgG >= 1 && avgT1 >= 1 && avgT2 >= 1 && avgT3 >= 1 && geomAvgT1 >= 1 && geomAvgT2 >= 1 && geomAvgT3 >= 1)
		{
	    Module *m = new Module(method->getModuleNum());
	  	if (m == NULL)
	  	{
	  		fprintf(stdout, "Error: out of memory in function oneLevelPartition(). \nExisting ...\n");
	  		exit(-1);
	  	}
	    m->setAvgG(avgG);
	    m->setGeomAvgG(geomAvgG);
	    m->setAvgT1(avgT1);
	    m->setAvgT2(avgT2);
	    m->setAvgT3(avgT3);
	    m->setGeomAvgT1(geomAvgT1);
	    m->setGeomAvgT2(geomAvgT2);
	    m->setGeomAvgT3(geomAvgT3);
	    
	    method->addModule(m);
	  }

  	for (int i = 0; i < clusterNum; ++ i)
  	{
			delete cv[i];
		}
		//erase the first clusterNum elements
		cv.erase(cv.begin(), cv.begin()+clusterNum);

		return flag;
	}
	
	void Design::MLPartRent()
  {
  	Method *method = new Method(methods.size());
  	if (method == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in function MLPartRent(). \nExisting ...\n");
  		exit(-1);
  	}
  	methods.push_back(method);
  	char *msg = "\n========= Circuit partitioning based method using MLPart =========";
  	method->setMSG(msg);
  	
  	ClusterVector cv;
  	Cluster *c = new Cluster(0);
  	if (c == NULL)
  	{
  		fprintf(stdout, "Error: out of memory in MLPartRent(). \nExisting ...\n");
  		exit(-1);
  	}
  	c->setLevel(0);
  	//make one cluster containing all the gates
    GateVector &chipGates = chip.getGates();
    int gateNum = chipGates.size();
  	for (int i = 0; i < gateNum; ++ i)
  	{
  		c->addGate(chipGates[i]);
  	}
  	cv.push_back(c);
  	
  	int levelIndex = 0;
  	bool flag = true;
  	while (flag)
  	{
			if (m_verb.getForMajStats() > 0)
			{
	  		fprintf(stdout, "MLPart at Level %d ...\n", levelIndex ++);
	  	}
			if(m_verb.getForSysRes() > 1)
			{
				reportMem();
	    }
  		flag = oneLevelPartition(cv, method);
  	}
  	//make a module containing all the gates
    int padNum = pads.size();
    
    int numPIOs = 0;
    for (PadMap::iterator it = pads.begin(); it != pads.end(); ++it) {
        if (it->second->getPadType() == PrimiaryInput)
            numPIOs++;
        else if (it->second->getPadType() == PrimiaryOutput)
            numPIOs++;
    }

	if (gateNum >= 1 && padNum >= 1) {
	    Module *m = new Module(method->getModuleNum());
    	if (m == NULL) {
	  		fprintf(stdout, "Error: out of memory in MLPartRent(). \nExisting ...\n");
	  		exit(-1);
	  	}

	    m->setAvgG((double)gateNum);
	    m->setGeomAvgG((double)gateNum);
        m->setAvgT1((double)numPIOs);
        m->setAvgT2((double)numPIOs);
        m->setAvgT3((double)numPIOs);
        m->setGeomAvgT1((double)numPIOs);
        m->setGeomAvgT2((double)numPIOs);
        m->setGeomAvgT3((double)numPIOs);
		method->addModule(m);
	    method->setAvgK(m_avgK);
	    method->setGeomAvgK(m_geomAvgK);
	  	method->linCurvFit();
	//  	method->print(stdout);
		}
  }
	
	void Design::readAUX(string fileName)
	{
		char *str;
		string parts;
		ifstream filein(fileName.c_str());
		abkfatal3(filein, "Cannot open ", fileName.c_str(), "\n");
		unsigned linesRead=0;
		bool lefRead = false, defRead = false;
		bool lefReading = false, defReading = false, cellKeyReading = false, cellNameReading = false;
		bool headSpaceFlag, blankLineFlag;
    char buf[5001];
		while (filein.getline(buf, 5001))
		{
			headSpaceFlag = false;
			blankLineFlag = false;
			linesRead++;
			
			if (m_verb.getForMajStats() > 2)
			{
//				cout<<"=========================================="<<endl;
				cout<<"line "<<linesRead<<" : "<<buf<<endl;
//				cout<<"parts: "<<parts<<endl;
//				cout<<"lefReading: "<<lefReading<<endl;
//				cout<<"defReading: "<<defReading<<endl;
//				cout<<"lefRead: "<<lefRead<<endl;
//				cout<<"defRead: "<<defRead<<endl;
//				fprintf(stdout, "strlen(buf): %d\n", strlen(buf));
//				fprintf(stdout, "buf[strlen(buf)-1]: %c\n", buf[strlen(buf)-1]);
//				cout<<"==========================================="<<endl;
			}
			
			//remove tailing spaces
			int endIndex = strlen(buf)-1;
			while (endIndex >= 0 && (buf[endIndex] == ' ' || buf[endIndex] == '\t'))
			{
				-- endIndex;
			}
			buf[endIndex+1] = '\0';
			
//			fprintf(stdout, "========== After trailing spaces ==========\n");
//			fprintf(stdout, "strlen(buf): %d\n", strlen(buf));
//			fprintf(stdout, "buf[strlen(buf)-1]: %c\n", buf[strlen(buf)-1]);

			//blank line
			if (strlen(buf) == 0)
			{
				blankLineFlag = true;
			}
			else if (buf[0] == ' ' || buf[0] == '\t')
			{
				headSpaceFlag = true;
			}
			
			str = strtok(buf, ": \t");
			//blank line
			if (!str)
			{
				blankLineFlag = true;
			}
			
			if (blankLineFlag || headSpaceFlag)
			{
				if (!parts.empty() > 0)
				{
					abkassert((lefReading || defReading), "Auxiliary file format error");
					if (lefReading)
					{
						string lefName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"LEF file: "<<lefName<<endl;
						}
						m_lefNames.push_back(lefName);
						lefRead = true;
					}
					else if (defReading)
					{
						string defName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"DEF file: "<<defName<<endl;
						}
						m_defNames.push_back(defName);
						defRead = true;
					}
					parts.clear();
				}
				continue;
			}
			if (str[0]=='#') continue;
			
	    if (!strncmp(str, "LEF", 3) || !strncmp(str, "lef", 3))
	    {
				abkfatal3(!cellKeyReading, "Error in format of file ", fileName.c_str(), ": key cell names should appear after LEF files\n");
				abkfatal3(!cellNameReading, "Error in format of file ", fileName.c_str(), ": full cell names should appear after LEF files\n");
				str = strtok(NULL, ": \t");
				lefReading = true;
				if (defReading)
				{
					if (!parts.empty())
					{
						string defName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"DEF file: "<<defName<<endl;
						}
						m_defNames.push_back(defName);
						defRead = true;
						parts.clear();
					}
					defReading = false;
				}
	    }
	    else if (!strncmp(str, "DEF", 3) || !strncmp(str, "def", 3))
	    {
				abkfatal3(!cellKeyReading, "Error in format of file ", fileName.c_str(), ": key cell names should appear after DEF files\n");
				abkfatal3(!cellNameReading, "Error in format of file ", fileName.c_str(), ": full cell names should appear after DEF files\n");
				str = strtok(NULL, ": \t");
				defReading = true;
				if (lefReading)
				{
					if (!parts.empty())
					{
						string lefName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"LEF file: "<<lefName<<endl;
						}
						m_lefNames.push_back(lefName);
						lefRead = true;
						parts.clear();
					}
					lefReading = false;
				}
	    }
	    else if (!strncmp(str, "REMOVE_CELL_BY_KEY", 18))
	    {
				str = strtok(NULL, ": \t");
				cellKeyReading = true;
				abkfatal3(!cellNameReading, "Error in format of file ", fileName.c_str(), ": full cell names should appear after key cell names\n");
				abkfatal3(!lefReading || !defReading, "Error in reading ", fileName.c_str(), "\n");
				if (lefReading)
				{
					if (!parts.empty())
					{
						string lefName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"LEF file: "<<lefName<<endl;
						}
						m_lefNames.push_back(lefName);
						lefRead = true;
						parts.clear();
					}
					lefReading = false;
				}
				else if (defReading)
				{
					if (!parts.empty())
					{
						string defName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"DEF file: "<<defName<<endl;
						}
						m_defNames.push_back(defName);
						defRead = true;
						parts.clear();
					}
					defReading = false;
				}
	    }
	    else if (!strncmp(str, "REMOVE_CELL_BY_FULL_NAME", 24))
	    {
				str = strtok(NULL, ": \t");
				cellNameReading = true;
				abkfatal3(lefReading || defReading || cellKeyReading, "Error in reading ", fileName.c_str(), "\n");
				if (cellKeyReading)
				{
					if (!parts.empty())
					{
						string keyName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"Cell key: "<<keyName<<endl;
						}
						m_cellKeyNames.push_back(keyName);
						parts.clear();
					}
					cellKeyReading = false;
				}
				else if (lefReading)
				{
					if (!parts.empty())
					{
						string lefName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"LEF file: "<<lefName<<endl;
						}
						m_lefNames.push_back(lefName);
						lefRead = true;
						parts.clear();
					}
					lefReading = false;
				}
				else if (defReading)
				{
					if (!parts.empty())
					{
						string defName = parts;
						if (m_verb.getForMajStats() > 1)
						{
							cout<<"DEF file: "<<defName<<endl;
						}
						m_defNames.push_back(defName);
						defRead = true;
						parts.clear();
					}
					defReading = false;
				}
	    }
	    
			while (str)
			{
//				fprintf(stdout, "str: %s\n", str);			
				if (str[strlen(str)-1] == '\\')
				{
					if (strlen(str) == 1)
					{
						fprintf(stdout, "Warning: a single character '\\' separated by spaces only terminates the line without continuation\n");
					}
					else
					{
						str[strlen(str)-1] = '\0';
						parts = parts + str;
					}
					
					break;
				}
				if (lefReading)
				{
					string lefName = str;
					if (!parts.empty())
					{
						lefName = parts + lefName;
						parts.clear();
					}
					if (m_verb.getForMajStats() > 1)
					{
						cout<<"LEF file: "<<lefName<<endl;
					}
					m_lefNames.push_back(lefName);
					str = strtok(NULL, ": \t");
					lefRead = true;
				}
				else if (defReading)
				{
					string defName = str;
					if (!parts.empty())
					{
						defName = parts + defName;
						parts.clear();
					}
					
					if (m_verb.getForMajStats() > 0)
					{
						cout<<"DEF file: "<<defName<<endl;
					}
					m_defNames.push_back(defName);
					str = strtok(NULL, ": \t");
					defRead = true;
				}
				else if (cellKeyReading)
				{
					string keyName = str;
					if (!parts.empty())
					{
						keyName = parts + keyName;
						parts.clear();
					}
					
					if (m_verb.getForMajStats() > 0)
					{
						cout<<"Cell key name: "<<keyName<<endl;
					}
					m_cellKeyNames.push_back(keyName);
					str = strtok(NULL, ": \t");
				}
				else if (cellNameReading)
				{
					string cellName = str;
					if (!parts.empty())
					{
						cellName = parts + cellName;
						parts.clear();
					}
					
					if (m_verb.getForMajStats() > 0)
					{
						cout<<"Cell full name: "<<cellName<<endl;
					}
					m_cellFullNames.push_back(cellName);
					str = strtok(NULL, ": \t");
				}
				else
				{
					fprintf(stdout, "Auxiliary file format error: unrecognized string %s\n", str);
					exit(-1);
				}
			}
		}
		if (!parts.empty())
		{
			abkassert((lefReading || defReading || cellKeyReading || cellNameReading), "Auxiliary file format error");
			if (lefReading)
			{
				string lefName = parts;
				if (m_verb.getForMajStats() > 0)
				{
					cout<<"LEF file: "<<lefName<<endl;
				}
				m_lefNames.push_back(lefName);
				lefRead = true;
			}
			else if (defReading)
			{
				string defName = parts;
				if (m_verb.getForMajStats() > 0)
				{
					cout<<"DEF file: "<<defName<<endl;
				}
				m_defNames.push_back(defName);
				defRead = true;
			}
			else if (cellKeyReading)
			{
				string keyName = parts;
				if (m_verb.getForMajStats() > 0)
				{
					cout<<"Cell key name: "<<keyName<<endl;
				}
				m_cellKeyNames.push_back(keyName);
			}
			else if (cellNameReading)
			{
				string cellName = parts;
				if (m_verb.getForMajStats() > 0)
				{
					cout<<"Cell full name: "<<cellName<<endl;
				}
				m_cellFullNames.push_back(cellName);
			}
			parts.clear();
		}
		filein.close();
		
		if (!(lefRead && defRead))
		{
			cout<<"Please check the Auxiliary file and provide the necessary LEF and DEF file names"<<endl;
			exit(-1);
		}
		
		if (m_verb.getForMajStats() > 0)
		{
			fprintf(stdout, "Total %d LFE files\n", m_lefNames.size());
			for (int i = 0; i < m_lefNames.size(); ++ i)
			{
				fprintf(stdout, "LEF %d: %s\n", i, m_lefNames[i].c_str());
			}
			fprintf(stdout, "Total %d DFE files\n", m_defNames.size());
			for (int i = 0; i < m_defNames.size(); ++ i)
			{
				fprintf(stdout, "DEF %d: %s\n", i, m_defNames[i].c_str());
			}
			fprintf(stdout, "Total %d key cell names\n", m_cellKeyNames.size());
			for (int i = 0; i < m_cellKeyNames.size(); ++ i)
			{
				fprintf(stdout, "Key cell name %d: %s\n", i, m_cellKeyNames[i].c_str());
			}
			fprintf(stdout, "Total %d full cell names\n", m_cellFullNames.size());
			for (int i = 0; i < m_cellFullNames.size(); ++ i)
			{
				fprintf(stdout, "Full cell name %d: %s\n", i, m_cellFullNames[i].c_str());
			}
		}
	}
}

