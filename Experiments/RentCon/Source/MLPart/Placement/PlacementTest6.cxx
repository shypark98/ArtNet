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
#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif

#include "subPlacement.h"

using std::cout;
using std::endl;

int main()
{
    Placement pl(5);
    pl[0]=Point(0,0);
    pl[1]=Point(1,1);
    pl[2]=Point(2,2);
    pl[3]=Point(3,3);
    pl[4]=Point(4,4);
    pl[3]=Point(3,3);

    Mapping mp(3,5);
    mp[0]=1;
    mp[1]=3;
    mp[2]=4;
 
    SubPlacement sbp(pl,mp);

    Permutation  pm(5);
    pm[0]=2;
    pm[1]=0;
    pm[2]=1;
    pm[3]=4;
    pm[4]=3;

    cout << sbp;

    pl.reorder(pm);
    sbp.reorder(pm);

    cout << "Now reorder " << endl << sbp;
    cout << pl;
    
    return 0;
}
