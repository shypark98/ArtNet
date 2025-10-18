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


// Created by Paul Tucker, Oct 1997, VLSICAD ABKGroup UCLA/CS

#include "interval.h"

using std::cout;
using std::endl;

int main()
{

  IntervalSeq IS(0);
  IS.push_back(Interval(1.7,2.4));
  IS.push_back(Interval(1.3,1.9));
  IS.push_back(Interval(0.1,0.9));
  IS.push_back(Interval(0.7,1.2));
  IS.push_back(Interval(1.5,1.6));

  cout << "Start sequence: " << IS << endl;
  
  IS.canonicalize();
  cout << "After canonicalization: " << IS << endl;

  RandomIntervalSeqGenerator RISG(0,10,20,50,200,1.5,0);
  //RISG.seed(0);

  for (unsigned i=0; i<10; i++)
  {
    IntervalSeq IS1(0), IS2(0);

    RISG.setInt(IS1);
    RISG.setInt(IS2);

    cout << "First random seq: " << IS1 << endl;
    cout << "2nd   random seq: " << IS2 << endl;
    IS1.merge(IS2);
    cout << "result of merge : " << IS1 << endl;
  }

  return 0;
}
