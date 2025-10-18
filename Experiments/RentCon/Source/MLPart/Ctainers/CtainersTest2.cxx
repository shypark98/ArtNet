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

#include "riskyStack.h"

#include <cfloat>
#include <iostream>

struct silly
{
    silly(unsigned uu,double dd):u(uu),d(dd){}
    silly():u(UINT_MAX),d(DBL_MAX){}
    unsigned u;
    double d;
};
int main(void)
{
    RiskyStack<silly> rs(17);

    rs.push_back(silly(5,12.0));
    rs.push_back(silly(4,2.0));

    const RiskyStack<silly> &rs_ref=rs;

    std::cout << rs_ref.back().u <<" "<< rs_ref.back().d << std::endl;

    RiskyStack<silly> rs_test_copy_ctor(rs);
    std::cout << rs_test_copy_ctor[0].u <<" "<< rs_test_copy_ctor[1].d << std::endl;

    RiskyStack<silly> rs_test_assignment;
    rs_test_assignment=rs_ref;
    std::cout << rs_test_assignment[0].u <<" "<< rs_test_assignment[1].d << std::endl;

    rs.push_back(silly(32,1.79));
    rs.pop_back();
    const silly &sillyConstRef = rs.backPlusOne();
    std::cout << sillyConstRef.u << " " << sillyConstRef.d << std::endl;

    rs.back() = silly(193,27.3);
    std::cout << rs[1].u << " " << rs[1].d << std::endl;

    rs.reserve(47); //NOTE! invalidates sillyConstRef
    std::cout << rs[1].u << " " << rs[1].d << std::endl;
    const silly &newConstRef = rs.backPlusOne();
    std::cout << newConstRef.u << " " << newConstRef.d << std::endl;



    return 0;
}

