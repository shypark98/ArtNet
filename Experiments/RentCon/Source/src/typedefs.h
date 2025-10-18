/**************************************************************************
***    
*** Copyright (c) 2008 Regents of the University of California,
***               Andrew B. Kahng, Kwangok Jeong and Hailong Yao
***
***  Contact author(s): abk@cs.ucsd.edu, kjeong@vlsicad.ucsd.edu, hailong@cs.ucsd.edu
***  Original Affiliation:   UCSD, Computer Science and Engineering Department,
***                          La Jolla, CA 92093-0404 USA
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

/***************************
*** Created by Hailong Yao
*** hailong@cs.ucsd.edu
*** Date: Jul. 30, 2008
***************************/

#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#include <vector>
#include <string>
#include <list>
// Mateus@180515
//#include <slist>
//--------------
#include <map>
#include <queue>
#include <iostream> // cout
#include <utility> // pair
#include <set>

using namespace std;

namespace DESIGN {

  class Gate;
  class Pin;
  class Pad;
  class Net;
  class Subnet;
  class Module;
  class Partition;
  class CellMaster;
  class PinMaster;
  class Box;
  class Method;
  class Cluster;
  class GateL2S;
  class GateIncConL2S;
  
  typedef vector<int> IntVector;
  typedef IntVector::iterator IntVectorItr;

  typedef vector<IntVector> DIntVector;
  typedef DIntVector::iterator DIntVectorItr;

  typedef vector<float> FVector;
  typedef FVector::iterator FVectorItr;

  typedef vector<FVector> DFVector;
  typedef DFVector::iterator DFVectorItr;

  typedef vector<double> DVector;
  typedef DVector::iterator DVectorItr;

  typedef vector<DVector> DDVector;
  typedef DDVector::iterator DDVectorItr;

  typedef vector<DDVector> DDDVector;
  typedef DDDVector::iterator DDDVectorItr;

  typedef map<string, DDDVector> StrDDDVMap;
  typedef StrDDDVMap::iterator StrDDDVMapItr;

  typedef vector<DDDVector> DDDDVector;
  typedef DDDDVector::iterator DDDDVectorItr;

  typedef map<string, Gate *> GateMap;
  typedef GateMap::iterator GateMapItr;

  typedef vector<Gate *> GateVector;
  typedef GateVector::iterator GateVectorItr;
  typedef GateVector::reverse_iterator GateVectorRItr;
  
  typedef vector<Pin *> PinVector;
  typedef PinVector::iterator PinVectorItr;

  typedef map<string, Pin *> PinMap;
  typedef PinMap::iterator PinMapItr;

  typedef map<string, Pad *> PadMap;
  typedef PadMap::iterator PadMapItr;

  typedef vector<Pad *> PadVector;
  typedef PadVector::iterator PadVectorItr;
  
  typedef vector<Net *> NetVector;
  typedef NetVector::iterator NetVectorItr;
  
  typedef vector<Subnet *> SubnetVector;
  typedef SubnetVector::iterator SubnetVectorItr;
  
  typedef vector<string> StrVector;
  typedef StrVector::iterator StrVectorItr;
  
  typedef map<string, CellMaster *> CellMasterMap;
  typedef CellMasterMap::iterator CellMasterMapItr;

  typedef map<string, PinMaster *> PinMasterMap;
  typedef PinMasterMap::iterator PinMasterMapItr;
	
  typedef vector<Pin *> PinVector;
  typedef PinVector::iterator PinVectorItr;

  typedef map<int, int> IntIntMap;
  typedef IntIntMap::iterator IntIntMapItr;

  typedef map<int, IntVector> IntIVMap;
  typedef IntIVMap::iterator IntIVMapItr;

  typedef map<int, bool> IntBoolMap;
  typedef IntBoolMap::iterator IntBoolMapItr;

  typedef map<string, int> StrIntMap;
  typedef StrIntMap::iterator StrIntMapItr;
  
  typedef map<int, string> IntStrMap;
  typedef IntStrMap::iterator IntStrMapItr;

  typedef map<string, double> StrDMap;
  typedef StrDMap::iterator StrDMapItr;

  typedef map<string, DVector> StrDVMap;
  typedef StrDVMap::iterator StrDVMapItr;
  
  typedef pair<int, double> IntDPair;
  
  typedef vector<IntDPair> SparseRow;
  typedef SparseRow::iterator SparseRowItr;

  typedef vector<SparseRow> SparseM;
  typedef SparseM::iterator SparseMItr;
  
  typedef map<int, double> IntDMap;
  typedef IntDMap::iterator IntDMapItr;

  typedef vector<bool> BoolVector;
  typedef BoolVector::iterator BoolVectorItr;
  
  typedef list<int> IntList;
  typedef IntList::iterator IntListItr;

  typedef list<Gate *> GateList;
  typedef GateList::iterator GateListItr;
  
  typedef vector<Module *> ModuleVector;
  typedef ModuleVector::iterator ModuleVectorItr;

  typedef vector<Partition *> PartitionVector;
  typedef PartitionVector::iterator PartitionVectorItr;

  typedef map<int, Partition *> IntPartMap;
  typedef IntPartMap::iterator IntPartMapItr;

  typedef vector<Box> BoxVector;
  typedef BoxVector::iterator BoxVectorItr;

  typedef vector<Method *> MethodVector;
  typedef MethodVector::iterator MethodVectorItr;

  typedef set<Net *> NetSet;
  typedef NetSet::iterator NetSetItr;

  typedef vector<Cluster *> ClusterVector;
  typedef ClusterVector::iterator ClusterVectorItr;

  typedef vector< ClusterVector > ClusterVV;
  typedef ClusterVV::iterator ClusterVVItr;
	
	typedef priority_queue <Gate *, vector <Gate *>, GateL2S> GatePQ;
	typedef priority_queue <Gate *, vector <Gate *>, GateIncConL2S> GateIncConPQ;

  typedef set<Gate *> GateSet;
  typedef GateSet::iterator GateSetItr;
}
#endif //__TYPEDEFS_H__
