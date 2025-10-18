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


#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "bitBoard.h"
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char** argv)
{
   BitBoard bb(10);
   const uofm_bit_vector& bv=bb.getBoardSpace();
   cout << bb;
   unsigned k;
   for(k=0; k!=bv.size(); k++) cout << bv[k]; cout << endl;
   cout << " Bit 5 is set to " << bb.isBitSet(5) << endl;
   cout << " Set bits 3 and 7 " << endl;
   bb.setBit(3); bb.setBit(7);
   cout << bb;
   for(k=0; k!=bv.size(); k++) cout << bv[k]; cout << endl;
   cout << " Set bit 3 once again " << endl;
   bb.setBit(3);
   cout << bb;
   cout << " Clear the BitBoard " << endl;
   bb.clear();
   cout << bb;
   for(k=0; k!=bv.size(); k++) cout << bv[k]; cout << endl;
}
