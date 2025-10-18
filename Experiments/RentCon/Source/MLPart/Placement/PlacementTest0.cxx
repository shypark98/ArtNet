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

#include "placement.h"
using std::cout;
using std::endl;

int main()
{
   Point a(2,3), b(4,5), c(3,-1);
   cout  << "a : "<< a << endl << "b : " << b << endl 
         << "a - b = " << a-b  << endl;
   BBox box(0,0,1,1);
   cout << "box  : "<< box << endl;
   box += a;
   cout << "box  : " << box << endl;
   BBox box1(a);
   box1+=b;
   cout << "box1 : "<< box1 << endl
        << "Half perimeter : " << box1.getHalfPerim() << endl;

   Placement pl(3);   
   pl[0]=a;
   pl[1]=b;
   pl[2]=c;
   cout << pl; 
   cout << pl.getBBox() << endl;

// cout << " Now input a placement : " << endl;
// Placement pl2(cin);   
   Placement pl2=pl;
   cout << pl2; 
   cout << pl.getBBox() << endl;
   cout << " ------------------------------------- "<< endl;
   BBox  bb(0,0,1,1);
   Point pt(1.3,1.2),pt1(-1.3,-1.2);
   cout << " Original :               " <<  pt << endl ;
   cout << " Coerced to the unit BBox :" << bb.coerce(pt)  << endl;
   cout << " Original                 :" << pt1 << endl ;
   cout << " Coerced to the unit BBox :" << bb.coerce(pt1) << endl;
   cout << " ------------------------------------- "<< endl;
   Point center(0.9,0.1);
   SmartBBox sbb(bb,center);
   cout << " Same with unitSmartBBox centered at "<<center<<endl;
   Point ptt(1.2,1.3),ptt1(-1.1,-2.9);
   cout << " Original :                " <<  ptt << endl;
   cout << " Coerced to the SmartBBox :" << sbb.coerce(ptt)  << endl;
   cout << " Original                 :" << ptt1 << endl;
   cout << " Coerced to the SmartBBox :" << sbb.coerce(ptt1) << endl;
   cout << " ------------------------------------- "<< endl;
   Placement pla(4);
   pla[0]=Point(1,1);
   pla[1]=Point(2,1);
   pla[2]=Point(1,2);
   pla[3]=Point(2,2);
   cout << pla;
   cout << " Center of Gravity : " << pla.getCenterOfGravity() << endl;
   return 0;
}
