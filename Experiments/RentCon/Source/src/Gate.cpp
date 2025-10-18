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
#include <fstream>
#include <ctime>
#include <cmath>

#include "Gate.h"
#include "Pad.h"
#include "ABKCommon/abkassert.h"

using namespace std;

namespace DESIGN {
	
  Gate::~Gate()
  {
  	m_adjGates.clear();
//    faninGates.clear();
//    fanoutGates.clear();
//    faninPads.clear();
//    fanoutPads.clear();
    inputSubnets.clear();
    outputSubnets.clear();
    m_nets.clear();
		for (PinMapItr itr = m_pinMap.begin(); itr != m_pinMap.end(); ++ itr)
		{
			Pin *pin = (*itr).second;
			delete pin;
		}
		m_pinMap.clear();
  }
	
  void Gate::addAdjGate(Gate *g)
  {
  	for (int i = 0; i < m_adjGates.size(); ++ i)
  	{
  		if (m_adjGates[i]->getId() == g->getId())
  		{
  			//cout<<"faninGates gate "<<g->getId()<<" Name: "<<g->getName()<<" already added for gate "<<l_name<<endl;
		  	return;
		  }
  	}
  	m_adjGates.push_back(g);
  }

//  void Gate::addFaninPad(Pad *p)
//  {
//  	for (int i = 0; i < faninPads.size(); i ++)
//  	{
//  		if (faninPads[i]->getId() == p->getId())
//  		{
//  			//cout<<"faninGates pad "<<p->getId()<<" Name: "<<p->getName()<<" already added for gate "<<l_name<<endl;
//		  	return;
//		  }
//  	}
//  	faninPads.push_back(p);
//  }
//  
//  void Gate::addFanoutPad(Pad *p)
//  {
//  	for (int i = 0; i < fanoutPads.size(); i ++)
//  	{
//  		if (fanoutPads[i]->getId() == p->getId())
//  		{
//  			//cout<<"fanoutGates pad "<<p->getId()<<" Name: "<<p->getName()<<" already added for gate "<<l_name<<endl;
//		  	return;
//		  }
//  	}
//  	fanoutPads.push_back(p);
//  }
//
//  void Gate::addFaninGate(Gate *g)
//  {
//  	for (int i = 0; i < faninGates.size(); i ++)
//  	{
//  		if (faninGates[i]->getId() == g->getId())
//  		{
//  			//cout<<"faninGates gate "<<g->getId()<<" Name: "<<g->getName()<<" already added for gate "<<l_name<<endl;
//		  	return;
//		  }
//  	}
//  	faninGates.push_back(g);
//  }
//  
//  void Gate::addFanoutGate(Gate *g)
//  {
//  	for (int i = 0; i < fanoutGates.size(); i ++)
//  	{
//  		if (fanoutGates[i]->getId() == g->getId())
//  		{
//  			//cout<<"fanoutGates gate "<<g->getId()<<" Name: "<<g->getName()<<" already added for gate "<<l_name<<endl;
//		  	return;
//		  }
//  	}
//  	fanoutGates.push_back(g);
//  }
  
  void Gate::print()
  {
    cout<<"Gate "<<id<<" Name "<<l_name<<" TYPE "<<type<<endl;
  }

	void Gate::addPin(Pin *p)
	{
		m_pinMap[p->getName()] = p;
	}

	void Gate::addNet(Net *n)
	{
		for (int i = 0; i < m_nets.size(); i ++)
		{
			if (n->getId() == m_nets[i]->getId())
				return;
		}
		m_nets.push_back(n);
	}
	
  bool Gate::insideBox(Box &b)
  {
    return (centerX >= b.left() && centerX <= b.right() && centerY >= b.bottom() && centerY <= b.top());
  }
  
  //(graph edge model with differentiated pin directions) counts each source-sink connection crossing the boundary as one pin;
	double Gate::getType1Terms(BoolVector &inside)
	{
		double terms = 0;
		
		for (int i = 0; i < inputSubnets.size(); ++ i)
		{
			Subnet *s = inputSubnets[i];
			abkassert(s != NULL, "Error in getType1Terms()");
			if (s->inputIsPad())
			{
				terms += 1;
			}
			else
			{
				Pin *p = s->getInputPin();
				abkassert(p != NULL, "Error in getType1Terms()");
				if (!inside[p->getGate()->getId()])
				{
					terms += 1;
				}
			}
		}

		for (int i = 0; i < outputSubnets.size(); ++ i)
		{
			Subnet *s = outputSubnets[i];
			abkassert(s != NULL, "Error in getType1Terms()");
			if (s->outputIsPad())
			{
				terms += 1;
			}
			else
			{
				Pin *p = s->getOutputPin();
				abkassert(p != NULL, "Error in getType1Terms()");
				if (!inside[p->getGate()->getId()])
				{
					terms += 1;
				}
			}
		}

		return terms;
	}

  //(graph edge model without pin directions) counts each connection crossing the boundary as one pin;
	double Gate::getType3Terms(BoolVector &inside)
	{
		double terms = 0;
		for (int i = 0; i < m_nets.size(); ++ i)
		{
			Net *n = m_nets[i];
			abkassert(n != NULL, "Error in getType3Terms()");
			terms += n->getPadNum();
			for (int j = 0; j < n->getPinNum(); ++ j)
			{
				Pin *p = n->getPin(j);
				abkassert(p != NULL, "Error in getType3Terms()");
				if (!inside[p->getGate()->getId()])
				{
					terms += 1;
				}
			}
		}

		return terms;
	}
		
}
