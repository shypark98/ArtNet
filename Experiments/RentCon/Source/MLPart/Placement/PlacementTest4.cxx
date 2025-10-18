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


// Created by Igor Markov, Sept 1997, VLSICAD ABKGroup UCLA/CS
#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif

#include <fstream>
#include "gridInPl.h"

using std::ofstream;
using std::cout;
using std::endl;

int main()
{
   Placement pl1(Placement::_Grid2,50);
   ofstream ofs("orig1.pla");
   ofs << pl1; 
   GridInPlacement grid1(10,20,pl1);
   ofstream grid1out("grid1out.pla");
   unsigned i;
   for(i=0; i!=grid1.getSize(); i++)
      grid1out << pl1[grid1.getPointIdcs()[i]] << endl;

   Placement pl2(Placement::_Random,2500);
   ofstream ofs1("orig2.pla");
   ofs1 << pl2;
   GridInPlacement grid2(10,20,pl2);
   ofstream grid2out("grid2out.pla");
   for(i=0; i!=grid2.getSize(); i++) 
      grid2out << pl2[grid2.getPointIdcs()[i]] << endl;
 
   return 0;
}
