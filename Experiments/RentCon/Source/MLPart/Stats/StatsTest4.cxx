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


#define NUM_CASES      10000
#define DESIRED_CORR    0.75
#define MAX_INDIVIDUAL 1000000000
#define RAND_SEED1      75412
#define RAND_SEED2      214
#define RAND_SEED3      487

#include <math.h>
#include "ABKCommon/abkcommon.h"
#include "trivStats.h"
#include "rancor.h"

using std::cout;
using std::endl;
using uofm::vector;

int main(void)
    {
    int i;
    vector<unsigned> v1,v2;
    unsigned comp0,comp1,comp2;
    double   alpha = 1/(1+sqrt(1/DESIRED_CORR - 1));

    RandomDouble ru0(0,MAX_INDIVIDUAL,RAND_SEED1), 
                 ru1(0,MAX_INDIVIDUAL,RAND_SEED2),
                 ru2(0,MAX_INDIVIDUAL,RAND_SEED3);

    for (i=0;i<NUM_CASES;i++)
        {
        comp0 = ru0;
        comp1 = ru1;
        comp2 = ru2;
        v1.push_back((unsigned)((1.0-alpha)*comp1 + alpha*comp0));
        v2.push_back((unsigned)((1.0-alpha)*comp2 + alpha*comp0));
        }

    cout <<"v1: " << TrivialStatsWithStdDev(v1) << endl;
    cout <<"v2: " << TrivialStatsWithStdDev(v2) << endl;
    cout <<"correl= " << Correlation(v1,v2) << endl;
    cout <<"Rank Correlation= " << RankCorrelation(v1,v2) << endl;


    return 0;
    }
