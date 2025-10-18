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
#include "ABKCommon/abkcommon.h"
#include "stats.h"

using std::cout;
using std::endl;
using uofm::vector;

int main(int argc, char *argv[])
{
   (void)argc;(void)argv;
   double  ar[]={1.2,4.1,9.1,13.1,2.1,4.2,9.2,13.2};
   vector<double>    dataSet1(3,Pi);
   vector<double>    dataSet2(6);
   vector<double>    dataSet3(8);

   //iota(dataSet2.begin(),dataSet2.end(),0);
   {for(unsigned i=0;i<dataSet2.size(); i++) dataSet2[i]=i;}

   std::copy(ar,ar+8,dataSet3.begin());

   cout << "=== Distribution 1 ========= " << endl << FreqDistr(dataSet1)
        << "=== Distribution 2 ========= " << endl << FreqDistr(dataSet2)
        << "=== Distribution 3 ========= " << endl
        << FreqDistr(dataSet2,10,FreqDistr::Magnitudes);
   FreqDistr distr4(dataSet3);
   distr4.numSubranges=4;
   cout << "=== Distribution 4 ========= " << endl << distr4;

return 0;
}
