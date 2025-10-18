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

#include "discretePrioritizer.h"
#include <ABKCommon/uofm_alloc.h>

using std::cout;
using std::endl;
using uofm::vector;

int main(void)
    {
    unsigned priorities1[] ={3,4,3,4,3,5,6,7,7,3,5};
    vector<unsigned> priorities;
    priorities.insert(priorities.end(),priorities1,priorities1+11);
    DiscretePrioritizer prior(priorities);
    cout << prior;
    prior.changePriorTail(8,5);
    prior.changePriorHead(7,4);
    prior.dequeue(6);
    cout << prior;
    cout << endl << "Enumeration of top bucket, head to tail:" << endl;
    unsigned val=prior.headTop();
    while (val!=UINT_MAX)
        {
        cout << val << " ";
        val = prior.next();
        }
    cout << endl << "Enumeration of top bucket, tail to head:" << endl;
    val=prior.tailTop();
    while (val!=UINT_MAX)
        {
        cout << val << " ";
        val = prior.prev();
        }
    cout << endl << "Enumeration of bucket 3, head to tail:" << endl;
    val=prior.head(3);
    while (val!=UINT_MAX)
        {
        cout << val << " ";
        val = prior.next();
        }
    cout << endl << "Enumeration of bucket 3, tail to head:" << endl;
    val=prior.tail(3);
    while (val!=UINT_MAX)
        {
        cout << val << " ";
        val = prior.prev();
        }
    cout << endl << "Enumeration of bottom bucket, head to tail:" << endl;
    val=prior.headBottom();
    while (val!=UINT_MAX)
        {
        cout << val << " ";
        val = prior.next();
        }
    cout << endl << "Enumeration of bottom bucket, tail to head:" << endl;
    val=prior.tailBottom();
    while (val!=UINT_MAX)
        {
        cout << val << " ";
        val = prior.prev();
        }
    cout << endl<< "Top = " << prior.top() << " Bottom = " << prior.bottom() << endl ;
    prior.enqueueTail(6,4);
    prior.changePriorTail(4,27);
    cout << prior;
    prior.changePriorHead(9,5);
    prior.dequeue(2);
    prior.changePriorHead(0,5);
    prior.dequeue(4);
    cout << prior;
    prior.dequeue(7);
    prior.dequeue(6);
    prior.dequeue(3);
    prior.dequeue(1);
    cout << prior;
    prior.dequeue(0);
    prior.dequeue(8);
    prior.dequeue(10);
    cout << prior;
    prior.dequeue(5);
    prior.dequeue(9);
    cout << prior;
    return 0;
    }

