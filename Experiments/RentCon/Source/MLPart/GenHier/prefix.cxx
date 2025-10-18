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

#include "ABKCommon/sgi_hash_set.h"
#include "ABKCommon/sgi_hash_map.h"
#include "genHier.h"
#include <set>
#include <algorithm>

using std::set;
using std::min;
using std::sort;
using uofm::vector;

typedef  hash_set<char*, hash<const char*>,GenHier_Eqstr> HashSetChar;
//typedef  set<char*, GenHier_Ltstr> HashSetChar;

typedef hash_map<char*, char*, hash<char*>, GenHier_Eqstr> GenHier_ParentMap;
//typedef map<char*, char*, GenHier_Ltstr> GenHier_ParentMap;

struct grstr
{
  grstr() {};
  bool operator()(const char* s1, const char* s2) const
  {  return strcmp(s1,s2) < 0;}
};              

void gen_hier_common_prefix(const char* a, const char* b, 
                   const char* delim_set, char * commonPrefix)
{
  if (a[0]=='\0' || b[0]=='\0') 
  { 
       if (!commonPrefix) commonPrefix=strdup("");
       else strcpy(commonPrefix,"");
       return; 
  }
  unsigned lena=strlen(a), lenb=strlen(b), lenx=0;
  if (delim_set==NULL || delim_set[0]=='\0')
  {
     for( ; lenx!=min(lena,lenb); lenx++)
     { if (a[lenx]!=b[lenx]) break; }

     if (!commonPrefix) 
       commonPrefix=static_cast<char*>(calloc(lenx+1,sizeof(char)));
     strncpy(commonPrefix,a,lenx);
     commonPrefix[lenx]='\0';
     return;
  }
  const char *oa=strpbrk(a,delim_set), *ob=strpbrk(b,delim_set);
  lena=oa-a, lenb=ob-b; 
  if (!oa) lena=strlen(a);
  if (!ob) lenb=strlen(b);
  if (lena!=lenb || strncmp(a,b,lena))
  {
    if (!commonPrefix) commonPrefix=strdup("");
    else strcpy(commonPrefix,"");
    return;
  }
  else if (!oa || !ob) 
  {
    if (!commonPrefix) 
       commonPrefix=static_cast<char*>(calloc(lena+1,sizeof(char)));
    strncpy(commonPrefix,a,lena);
    commonPrefix[lena]='\0';
    return;
  }
 
  do
  {
    lenx=lena;
    oa=strpbrk(oa+1,delim_set);
    ob=strpbrk(ob+1,delim_set);
    lena=oa-a, lenb=ob-b;
    if (!oa || !ob) break;
  } while (lena==lenb && strncmp(a,b,lena)==0);

  if (oa==NULL) lena=strlen(a);
  if (ob==NULL) lenb=strlen(b);
  if (lena==lenb && strncmp(a,b,lena)==0) lenx=lena;

  if (!commonPrefix) 
       commonPrefix=static_cast<char*>(calloc(lenx+1,sizeof(char)));
  strncpy(commonPrefix,a,lenx);
  commonPrefix[lenx]='\0';

  return;
} 


GenericHierarchy::GenericHierarchy(const vector<const char*>& nodeNames,
                                   const char* delimiter_set)
    : _names(nodeNames.size()), _root(UINT_MAX)
{
    vector<char*> _tmpNames(nodeNames.size());
    unsigned numLeaves=nodeNames.size();
    //HashSetChar leaves(numLeaves);
    HashSetChar leaves;
    unsigned i,maxSz=0;
    for(i = 0; i < numLeaves; i++)
    {
        unsigned sz=strlen(nodeNames[i]);
        if (sz>maxSz) maxSz=sz;
	_tmpNames[i] = strdup(nodeNames[i]);
        leaves.insert(_tmpNames[i]);
    }
    grstr g;
    sort(_tmpNames.begin(),_tmpNames.end(),g);
    char *prefix=new char[maxSz+10];

    HashSetChar addedClusters;
    for(i=1; i!=numLeaves; i++)
    {
        gen_hier_common_prefix(_tmpNames[i-1],_tmpNames[i],delimiter_set,prefix);
        if (leaves.find(prefix)!=leaves.end()) continue;
        if (addedClusters.find(prefix)==addedClusters.end())
        {
            if (strcmp(prefix,"")==0)
                 addedClusters.insert(strdup("ROOT"));
            else addedClusters.insert(strdup(prefix));
            _tmpNames.push_back(strdup(prefix));
        }
    }
    sort(_tmpNames.begin(),_tmpNames.end(),g);

    //GenHier_ParentMap    parentNames(_tmpNames.size()*1.5);
    GenHier_ParentMap    parentNames;
    HashSetChar missingChildren;

    char *tmp;

    if (strcmp(_tmpNames[0],"")==0) 
    { 
//     free(_tmpNames[0]);
       char* rootName = new char[strlen("ROOT")+2];
       strcpy(rootName, "ROOT");

       parentNames[rootName]=strdup(rootName);
    }
    else parentNames[_tmpNames[0]]=strdup(_tmpNames[0]);

    unsigned nameSize=_tmpNames.size();
    for(i=1; i!=nameSize; i++)
    {
        gen_hier_common_prefix(_tmpNames[i-1],_tmpNames[i],delimiter_set,prefix);
        if (leaves.find(prefix)==leaves.end())
        { 
          if (strcmp(prefix,"")==0) parentNames[_tmpNames[i]]=strdup("ROOT");
          else parentNames[_tmpNames[i]]=strdup(prefix);
        }
        else
        {
            tmp=static_cast<char*>(calloc(strlen(prefix)+10,sizeof(char)));
            sprintf(tmp,"%s++cluster",prefix);
            parentNames[_tmpNames[i]]=tmp;
            if (missingChildren.find(prefix)==missingChildren.end())
                missingChildren.insert(strdup(prefix));
        }
    }

    delete[] prefix;

    for(i = 0; i < numLeaves; i++)
    {
	_names[i] = new char[strlen(nodeNames[i])+1];
        strcpy(_names[i],nodeNames[i]);
        _namesToId[strdup(_names[i])]=i;
    }

    HashSetChar::iterator it;

    for(it =addedClusters.begin(); it!=addedClusters.end(); it++)
    {
        char *temp = new char[strlen(*it)+1];
        strcpy(temp,*it);
	_names.push_back(temp);
        _namesToId[_names.back()]=i++;
    }

    for(it =missingChildren.begin(); it!=missingChildren.end(); it++)
    {
       tmp=new char[strlen(*it)+10];
       char *tmp2=new char[strlen(*it)+10];
       sprintf(tmp,"%s++cluster",*it);
       sprintf(tmp2,"%s++cluster",*it);
       parentNames[tmp]=parentNames[*it]; 
       parentNames[*it]=tmp;
       _names.push_back(tmp2);
       _namesToId[strdup(_names.back())]=i++;
    }

    //from the parentNames, generate the parent and then children Id's
    _parents  = vector<unsigned>(_names.size(), GENH_DELETED_NODE);
    _children = vector<vector<unsigned> >(_names.size());


    for(i = 0; i < _names.size(); i++)
    {
      abkfatal3(_namesToId.count(parentNames[_names[i]]) > 0,
     _names[i], "'s parent was not defined: ", parentNames[_names[i]]);

     tmp=parentNames[_names[i]];
     _parents[i] = _namesToId[tmp];


//   free(tmp);
        if(_parents[i] == i)
            _root = i;
        else
            _children[_parents[i]].push_back(i);
    }             

/*  for(i=0; i!=_names.size(); i++)
    {
       cout <<"\"" << _names[i] << "\"   \"" 
                   << parentNames[_names[i]]<< "\"" << endl;
    }
*/
    for(i = 0; i < _tmpNames.size(); i++) free(_tmpNames[i]); 
}

