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


// Created by Igor Markov, Aug 1997, VLSICAD ABKGroup UCLA/CS

#include "combi.h"

using std::cout;
using std::endl;

int main()
{

    Subset sb (2,5);
    sb[0]=3;
    sb[1]=2;

    Subset sbs(3,5);
    sbs[0]=1;
    sbs[1]=3;
    sbs[2]=4;
 
    Permutation  pm(5);
    pm[0]=2;
    pm[1]=0;
    pm[2]=1;
    pm[3]=4;
    pm[4]=3;

    Subset  sbs1(Subset::COMPLEMENT,sbs);
    Subset  sbs2(pm,Subset::MINUS,sbs1);
    Subset  sbs3(pm,Subset::XSECT,sbs1);
    Subset  sbs4(sbs1,Subset::MINUS,pm);
    Subset  sbs5(sb,Subset::UNION,sbs);

    cout << sbs1 << endl;
    cout << sbs2 << endl;
    cout << sbs3 << endl;
    cout << sbs4 << endl;
    cout << sbs5 << endl;

    return 0;
}
