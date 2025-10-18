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


// Created by Igor Markov, July 1997, VLSICAD ABKGroup UCLA/CS

#include <fstream>
#include "combi.h"

using std::cout;
using std::endl;
using uofm::vector;

int main()
{
   cout << " Testing permutations " << endl;
   unsigned data[3]={0,2,1};
   vector<unsigned> d(3); d[0]=0; d[1]=2; d[2]=1;
   Permutation p1(3,data),p2(3,d);
   cout << p1 << p2;
// --------------
   Permutation prm(3);
   prm[0]=1,prm[1]=2,prm[2]=0;
   Permutation prm2(prm,prm); 
   cout << prm << "Square of the above is " << endl <<prm2;
   prm2=prm;
   cout << "After assignment " << endl;
   cout << prm2 ;
   cout << " Inverse " << endl << prm2.getInverse(prm);
   cout << " ---- Testing general mappings --- " << endl;
   Mapping  m1(2,4,data),m2(2,4,d);
   cout << m1 << m2;
   Mapping  id1(2,3,Mapping::Identity), id2(4,5,Mapping::_Reverse);
   Mapping  id3(id2,id1);
   cout << id1 << id2 << " The product of the two" << id3;

   vector<unsigned>   pinv;
   id3.getPartialInverse(pinv);
   cout << " Partial inverse : " << id3;
 
// copy(pinv.begin(), pinv.end(), ostream_iterator<unsigned>(cout, " "));
   cout << endl; 
   return 0;
}
