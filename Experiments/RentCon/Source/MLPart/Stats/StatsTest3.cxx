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
#include <algorithm>
#include <vector>
#include "ABKCommon/abkcommon.h"
#include "stats.h"

using std::cout;
using std::endl;
using std::copy;
using uofm::vector;

int main(int argc, char *argv[])
{
   (void)argc;(void)argv;
   double  ar1[]={1.2,4.1,9.1,13.1,2.1,4.2,9.2,13.2,14.1,14.7};
   double  ar2[]={1.2,-4.1,9.1,-13.1,2.1,-4.2,9.2,-13.2,14.1,-14.7};
   double  ar3[]={1.2,1.7,1.7,1.7,1.7,1.7,1.7,1.7,1.7,2.9};
   
   vector<double> vec1(10);
   vector<double> vec2(10);
   vector<double> vec3(10);
   vector<double> vec4(10), vec5(10);
   copy(ar1,ar1+10,vec4.begin());
   copy(ar2,ar2+10,vec5.begin());

   vector<double> vec6(vec5);
   std::sort(vec6.begin(),vec6.end());

   vector<double> vec7(10);
   copy(ar3,ar3+10,vec7.begin());
  
   for(unsigned k=0;k<10;k++)
   {
      vec1[k]=k;
      vec2[k]=2*k;
      vec3[k]=-3.0*k;
   }

   cout << " Vec 1 : " << endl <<  vec1
        << " Vec 2 : " << endl <<  vec2
        << " Vec 3 : " << endl <<  vec3
        << " Vec 4 : " << endl <<  vec4
        << " Vec 5 : " << endl <<  vec5
        << " Vec 6 : " << endl <<  vec6
        << " Vec 7 : " << endl <<  vec7;

   cout << " Correlation(vec1,vec2) = " << Correlation(vec1,vec2) << endl
        << " Correlation(vec1,vec3) = " << Correlation(vec1,vec3) << endl
        << " Correlation(vec1,vec4) = " << Correlation(vec1,vec4) << endl
        << " Correlation(vec4,vec5) = " << Correlation(vec4,vec5) << endl;

   cout << " Rank Correlation(vec1,vec2) = " << RankCorr(vec1,vec2) << endl
        << " Rank Correlation(vec1,vec3) = " << RankCorr(vec1,vec3) << endl
        << " Rank Correlation(vec1,vec4) = " << RankCorr(vec1,vec4) << endl
        << " Rank Correlation(vec4,vec5) = " << RankCorr(vec4,vec5) << endl 
        << " Rank Correlation(vec1,vec6) = " << RankCorr(vec1,vec6) << endl 
        << " Rank Correlation(vec1,vec7) = " << RankCorr(vec1,vec7) << endl 
        << " Rank Correlation(vec5,vec7) = " << RankCorr(vec5,vec7) << endl 
        << " Rank Correlation(vec7,vec7) = " << RankCorr(vec7,vec7) << endl;

  LinRegression reg0(vec1,vec2), reg1(vec2,vec1), reg2(vec1,vec3),
                reg3(vec1,vec4), reg4(vec4,vec5), reg5(vec1,vec6);

  cout<<" LinRegression(vec1,vec2): k= "<<reg0.getK()<<" c= "<<reg0.getC()<<endl
      <<" LinRegression(vec2,vec1): k= "<<reg1.getK()<<" c= "<<reg1.getC()<<endl
      <<" LinRegression(vec1,vec3): k= "<<reg2.getK()<<" c= "<<reg2.getC()<<endl
      <<" LinRegression(vec1,vec4): k= "<<reg3.getK()<<" c= "<<reg3.getC()<<endl
      <<" LinRegression(vec4,vec5): k= "<<reg4.getK()<<" c= "<<reg4.getC()<<endl
      <<" LinRegression(vec1,vec6): k= "<<reg5.getK()<<" c= "<<reg5.getC()<<endl
      ;
  
return 0;
}
