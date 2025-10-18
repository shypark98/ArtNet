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


#include <iostream>
#include <algorithm>
#include "permut.h"

using std::cout;
using std::endl;
using uofm::vector;

int main()
{
    vector<double>  data1(10);
    vector<double>  data2(10,5);
    unsigned k=0;
    for(k=0; k!=data1.size(); k++) data1[k]=k;
    for(k=0; k!=data2.size(); k++) data2[k]=k;
	cout << data1 << data2;
    Permutation perm1(data1);
    Permutation perm2(data2);
//	Permutation perm5();
	cout << perm1 << perm2;
	cout << "----------------"<< endl;
cout<<"A"<<endl;

//	unsigned size = 3;
/*      Permutation perm6(size);
	cout << perm6 << endl;
	cout << "Perm 6 is above" << endl;
*/

    std::swap(data2[3],data2[4]);
    std::swap(data2[1],data2[8]);
    
	cout << data1 << data2;
	cout << data1[0] << "..." << data1[9] << endl;
	cout << data2[0] << "..." << data2[9] << endl;
    Permutation perm3(data1);
    Permutation perm4(data2);
    cout << perm3 << perm4;
    return 0;
}
