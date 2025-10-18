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

#ifndef __VERTEX_H__
#define __VERTEX_H__

#include <cassert>
#include "ABKCommon/abkassert.h"

namespace DESIGN {
  
  class Vertex {
  public:
    //constructors
    Vertex(int i) : m_gateId(i), m_adjNumOutCluster(0)
    {
    }
    ~Vertex()
    {
    }
    //modifiers
    void setAdjNumOutCluster(int i) { m_adjNumOutCluster = i; }
    void incAdjNumOutCluster() { ++ m_adjNumOutCluster; }
    void decAdjNumOutCluster() { -- m_adjNumOutCluster; }
    //accossers
    int getAdjNumOutCluster() { return m_adjNumOutCluster; }
  private:
    int m_gateId;
  };

  class VertexS2L {
		public:
			inline bool operator () (Vertex *v1, Vertex *v2) { return v1->getAdjNumOutCluster() < v2->getAdjNumOutCluster(); }
  };
  
}

#endif //__VERTEX_H__
