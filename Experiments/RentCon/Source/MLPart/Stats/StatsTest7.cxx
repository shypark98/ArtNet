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


#include <algorithm>
#include "ABKCommon/abkcommon.h"
#include "trivStats.h"
#include "rancor.h"

using std::copy;
using std::cout;
using std::endl;
using uofm::vector;

int main(void)
{
   double case1wl[12]
     ={6.35,6.44,6.29,6.30,6.32,6.26,6.28,6.25,6.26,6.33,6.37,6.40};
   double case1rp[12]
     ={0.663,0.665,0.673,0.670,0.673,0.672,0.672,0.673,0.671,0.673,0.672,0.670};

    double case2wl[8]={2.93,2.99,2.89,2.87,2.86,2.87,2.88,2.90};
    double case2rp[8]={0.489,0.492,0.495,0.496,0.496,0.498,0.496,0.497};

    double case3wl[8]={6.41,6.41,6.31,6.28,6.31,6.27,6.28,6.30};
    double case3rp[8]={0.422,0.42,0.439,0.441,0.438,0.434,0.435,0.449};

    vector<double> v1_1(12), v1_2(12),
                   v2_1(6),  v2_2(6),
                   v3_1(6),  v3_2(6);

    copy(case1wl,case1wl+12,v1_1.begin());
    copy(case1rp,case1rp+12,v1_2.begin());
    copy(case2wl,case2wl+ 6,v2_1.begin());
    copy(case2rp,case2rp+ 6,v2_2.begin());
    copy(case3wl,case3wl+ 6,v3_1.begin());
    copy(case3rp,case3rp+ 6,v3_2.begin());

    cout << "Case 1" << endl; 
    cout <<"correl= " << Correlation(v1_1,v1_2) << endl;
    cout <<"Rank Correlation= " << RankCorrelation(v1_1,v1_2) << endl;

    cout << "Case 2" << endl; 
    cout <<"correl= " << Correlation(v2_1,v2_2) << endl;
    cout <<"Rank Correlation= " << RankCorrelation(v2_1,v2_2) << endl;

    cout << "Case 3" << endl; 
    cout <<"correl= " << Correlation(v3_1,v3_2) << endl;
    cout <<"Rank Correlation= " << RankCorrelation(v3_1,v3_2) << endl;

    return 0;
}
