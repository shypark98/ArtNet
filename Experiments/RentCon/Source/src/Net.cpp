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
#include "Net.h"
#include "ABKCommon/abkassert.h"

using namespace std;

namespace DESIGN {
  
  Net::~Net()
  {
    pins.clear();
    pads.clear();
  }
  
  Pin * Net::getSourcePin()
  {
    if(pins.size() == 0)
    {
    	fprintf(stdout, "\nWarning: Net %s has no pins.\n", name.c_str());
      return NULL;
    }
    
    abkassert(pins.size() > 0, "pins.size() <= 0 in getSourcePin()");
    
    if (pins[0]->getType() == OUTPUT || pins[0]->getType() == INOUT)
      return pins[0];
    else
      return NULL;
  }
  
  Pad * Net::getSourcePad()
  {
    if(pads.size() == 0)
    {
    	fprintf(stdout, "\nWarning: Net %s has no pads.\n", name.c_str());
      return NULL;
    }
    
    abkassert(pads.size() > 0, "pads.size() <= 0 in getSourcePad()");
    if (pads[0]->getPadType() == PrimiaryOutput || pads[0]->getPadType() == InputOutput)
      return pads[0];
    else
      return NULL;
  }
  
  void Net::checkPins()
  {
    if(pins.size() == 0)
    {
    	fprintf(stdout, "\nWarning: Net %s has no pins.\n", name.c_str());
      return;
    }
  	int sourceNum = 0, sinkNum = 0;
  	
  	for (int i = 0; i < pins.size(); i ++)
  	{
  		Pin *pin = pins[i];
  		
  		if (pin->getType() == INOUT || pin->getType() == OUTPUT)
  		{
  			sourceNum ++;
  		}
  		
  		if (pin->getType() == INOUT || pin->getType() == INPUT)
  		{
  			sinkNum ++;
  		}
  	}

  	for (int i = 0; i < pads.size(); i ++)
  	{
  		Pad *pad = pads[i];
  		if (pad->getPadType() == InputOutput || pad->getPadType() == PrimiaryOutput)
  		{
  			sinkNum ++;
  		}

  		if (pad->getPadType() == InputOutput || pad->getPadType() == PrimiaryInput)
  		{
  			 sourceNum ++;
  		}
  	}
    
		if (sourceNum == 0)
    {
    	fprintf(stdout, "\nWarning: no source pin/pad in Net %s.\n", name.c_str());
    }
		if (sourceNum > 1)
    {
    	fprintf(stdout, "\nWarning: Net %s has %d candidate source pins/pads.\n", name.c_str(), sourceNum);
    }
		if (sinkNum == 0)
    {
    	fprintf(stdout, "\nWarning: no sink pin/pad in Net %s.\n", name.c_str());
    }
  }
  
  void Net::makeSourcePin()
  {
    if(pins.size() == 0)
    {
    	fprintf(stdout, "\nWarning: Net %s has no pins.\n", name.c_str());
      return;
    }
  	
		if (pins[0]->getType() == OUTPUT || pins[0]->getType() == INOUT)
			return;
  	
  	int i = 1;
  	for (; i < pins.size(); i ++)
  	{
  		Pin *pin = pins[i];
  		
  		if (pin->getType() == INOUT || pin->getType() == OUTPUT)
  		{
  			pins.erase(pins.begin()+i);
  			pins.insert(pins.begin(), pin);
  			break;
  		}
  	}
  	
		if (i >= pins.size())
    {
    	fprintf(stdout, "\nWarning: no OUTPUT/INOUT pin in Net %s. Pick one INPUT pin as source pin.\n\n", name.c_str());
    	pins[0]->setType(INOUT);
    }
  }
  
  void Net::makeSinkPins()
  {
    if(pins.size() <= 1)
    {
//    	fprintf(stdout, "\nWarning: Net %s has %d pin.\n", name.c_str(), pins.size());
      return;
    }
  	
  	for (int i = 1; i < pins.size(); i ++)
  	{
  		Pin *pin = pins[i];
  		if (pin->getType() == OUTPUT)
	    {
	    	pin->setType(INOUT);
	    }
  	}
  }

  void Net::print()
  {
    cout<<"=====NET "<<id<<"====="<<endl;
    cout<<"Name "<<name<<endl;
    cout<<"=================="<<endl;
  }

}
