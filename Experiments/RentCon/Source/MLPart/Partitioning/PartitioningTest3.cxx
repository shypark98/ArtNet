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


#include "Geoms/plGeom.h"
#include "partitioning.h"

using std::cout;
using std::endl;
using uofm::vector;

int main()
{
   BBox term(Point(0,0)), term1(Point(1,0)), term2(3,-1,4,1);
   vector<BBox> partitions(4);
   partitions[0]=BBox(-1,0,0,1);
   partitions[1]=BBox(0,0,1,1 );
   partitions[2]=BBox(0,-1,1,0);
   partitions[3]=BBox(-1,-1,0,0);

   TerminalPropagator propag(partitions);
   PartitionIds tIds, tIds1, tIds2, fixedConstr;
   propag.doOneTerminal(term,fixedConstr,tIds);
   cout << tIds;
   propag.doOneTerminal(term1,fixedConstr,tIds1);
   cout << tIds1;
   propag.doOneTerminal(term2,fixedConstr,tIds2);
   cout << tIds2;
}
