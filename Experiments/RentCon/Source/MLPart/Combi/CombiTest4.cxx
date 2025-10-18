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
#include <cstdlib>
#include "ABKCommon/abkcommon.h"
#include "grayPermut.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

int main(int argc, char** argv)
{
    if (argc<2)
    {
        cerr<<"Insufficient arguments";
        abort();
    }

    unsigned size=atoi(argv[1]);
    unsigned arr[15];
    int i;
    for (i=0; i!=15; ++i) arr[i]=i;


    GrayTranspositionForPermutations perm(size);

    for (unsigned j=0; j!=26; ++j)
    {
        // for size>4, just the first 26 perm's
        unsigned ii;
        for (ii=0; ii!=size; ii++) cout<<arr[ii]<<" ";
        cout<<"\n";
        cout<<perm.getPermutation()<<"\n";

        if (perm.finished()) break;

        i=perm.next();
        std::swap(arr[i],arr[i+1]);
    }
}


