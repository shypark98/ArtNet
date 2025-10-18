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

#include <iostream>
#include "genHier.h"

using std::cin;
using std::cout;
using std::endl;
using uofm::vector;

int main()
{
   vector<const char*> names;
   cout << "Enter a string of one-character delimiters (e.g., '/|_'):  ";
   char delim[100]="";
   cin >> delim;
   cout << "The delimiter set is "<< delim << endl;
   cout << "Now enter a list of words containing those delimiters "
        << "\n (first empty line ends input) " << endl;
   char buf[1000]="x";
   while(cin)
   {
      cin >> buf;
      if (strlen(buf)==0) break;
      names.push_back(strdup(buf));
      buf[0]='\0';
   } 
   if (names.size()>1)
      cout << "Read " << names.size()
           << " words, will now ctruct a hierarhy...";
   else
      { cout << "Need two or more words" << endl; return 1; }
   GenericHierarchy gHie(names,delim);
   cout << " done \n saving test2.hcl ...";
   gHie.saveHCL("test2.hcl");
   cout << " done " << endl;
}
