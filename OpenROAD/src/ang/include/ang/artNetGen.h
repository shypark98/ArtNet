///////////////////////////////////////////////////////////////////////////
//
// BSD 3-Clause License
//
// Copyright (c) 2025, Seonghyeon Park and the Regents of the University of California
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __ARTNETGEN_HEADER__
#define __ARTNETGEN_HEADER__

#include <limits.h>
#include <math.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "db_sta/dbNetwork.hh"
#include "db_sta/dbSta.hh"
#include "odb/db.h"
#include "utl/Logger.h"

namespace ord {
class dbVerilogNetwork;
}

namespace odb {
class dbDatabase;
class dbChip;
class dbBlock;
class dbInst;
class dbMaster;
}  // namespace odb

namespace sta {
class dbSta;
class dbNetwork;
class Instance;
class NetworkReader;
class Library;
class Port;
class Net;
}  // namespace sta

namespace utl {
class Logger;
}

namespace ang {

class DelayDist
{
 public:
  void setMaxPath(int maxPath) { maxPath_ = maxPath; }
  void setMinPath(int minPath) { minPath_ = minPath; }
  void setMacroMaxPath(int maxPath) { macroMaxPath_ = maxPath; }
  void setMacroMinPath(int minPath) { macroMinPath_ = minPath; }

  void setPathDist(std::vector<int>& pathDist) { pathDist_ = pathDist; }

  int getMaxPath() { return maxPath_; }
  int getMinPath() { return minPath_; }
  int getMacroMaxPath() { return macroMaxPath_; }
  int getMacroMinPath() { return macroMinPath_; }
  std::vector<int>& getPathDist() { return pathDist_; }

  double SamplePath()
  {
    if (pathDist_.size() > 0) {
      return pathDist_[std::rand() % pathDist_.size()];
    } else {
      return maxPath_;
    }
  }

 private:
  std::vector<int> pathDist_;
  double maxPath_;
  double minPath_;
  double macroMaxPath_;
  double macroMinPath_;
};

class DistUnit
{
 public:
  DistUnit(int value, int target, int current, double ratio)
      : value_(value), target_(target), current_(current), ratio_(ratio)
  {
  }
  ~DistUnit();
  void clear();

 public:
  void setValue(int value) { value_ = value; }
  void setTarget(int target) { target_ = target; }
  void setCurrent(int current) { current_ = current; }
  void setRatio(double ratio) { ratio_ = ratio; }

 public:
  void tarIncr(int cnt) { target_ += cnt; }
  void tarDecr(int cnt) { target_ -= cnt; }
  void curIncr(int cnt) { current_ += cnt; }
  void curDecr(int cnt) { current_ -= cnt; }
  int getValue() const { return value_; }
  int getTarget() const { return target_; }
  int getCurrent() const { return current_; }
  int getRatio() const { return ratio_; }
  int error() const { return std::abs(target_ - current_); }

 private:
  int value_;
  int target_;
  int current_;
  double ratio_;
};

class Distrib
{
 public:
  Distrib();
  Distrib(Distrib* dist);
  ~Distrib();

  void init(std::vector<DistUnit> unit);
  void refine(int totCnt);
  void clear();

 public:
  int getMin() const { return min_; }
  int getMax() const { return max_; }
  int getTarget(int value) const { return units_[value].getTarget(); }
  int getCurrent(int value) const { return units_[value].getCurrent(); }
  int getRatio(int value) const { return units_[value].getRatio(); }
  int getTotCnt() const { return totCnt_; }
  double getTarAvg() const;
  double getCurAvg() const;
  double getCurRatio(int x) const;
  double getTarRatio(int x) const;

 public:
  void tarIncr(int x, int cnt = 1);

  void tarDecr(int x, int cnt = 1);

  void curIncr(int x, int cnt = 1);

  void curDecr(int x, int cnt = 1);

  int error(int x) { return units_[x].error(); }

  double ratioError(int x) const;
  double delRatio(int x, int dy) const;

  std::vector<int> rouletteVector();
  std::vector<DistUnit>::iterator end() { return units_.end(); }
  std::vector<DistUnit>::iterator begin() { return units_.begin(); }

 private:
  int min_;
  int max_;
  int totCnt_;
  std::vector<DistUnit> units_;
};

class MasterInfo;
class CellList;
class Block;
class Librarycell;

// Define data structure for logical hierarchy
class HierarchyNode;
using HierNodePtr = std::shared_ptr<HierarchyNode>;
class HierarchyTree;
using HierTreePtr = std::shared_ptr<HierarchyTree>;

class ArtNetGen
{
 public:
  void init(odb::dbDatabase* db, sta::dbSta* sta, utl::Logger* logger);
  void clear();

  void checkLibs();
  void printParams();
  void labelMacros();

  void run();
  void getNumInOutPins(odb::dbMaster* master, int& numInpins, int& numOutpins);
  void getNumInPins(odb::dbMaster* master, int& numInpins);

  struct PtreeNode
  {
    PtreeNode(int p, int c1, int c2, int a, int b, int i, int o)
        : parent(p),
          child1(c1),
          child2(c2),
          area(a),
          numBlocks(b),
          inputs(i),
          outputs(0)
    {
    }

    int parent, child1, child2;
    int area, numBlocks;
    int inputs, outputs;
  };

  static std::list<PtreeNode> treeData_;

  // setter for param
  void setDontUse(const char* masterName) { dontUses_.push_back(masterName); }
  void setMacro(const char* masterName) { macros_.push_back(masterName); }
  void setDefaultFlop(const char* flop);
  void setNetlistFile(const char* netlistFile) { netlistFile_ = netlistFile; }
  void setClockName(const char* clockName) { clockName_ = clockName; }
  void setIsClock(bool isClock) { isClock_ = isClock; }
  void setIsReset(bool isReset) { isReset_ = isReset; }
  void setNetlistFlag(bool netlistFlag) { netlistFlag_ = netlistFlag; }
  void setPrintDepth(bool printDepth) { printDepth_ = printDepth; }
  void setPrintForrest(bool printForrest) { printForrest_ = printForrest; }
  void setPrintNetlistInfo(bool printNetlistInfo)
  {
    printNetlistInfo_ = printNetlistInfo;
  }
  void setSpecFile(const char* specFile) { specFile_ = specFile; }
  void setMaxPath(int maxPath) { delay_.setMaxPath(maxPath); }
  void setMinPath(int minPath) { delay_.setMinPath(minPath); }
  void setMacroMaxPath(int maxPath) { delay_.setMacroMaxPath(maxPath); }
  void setMacroMinPath(int minPath) { delay_.setMacroMinPath(minPath); }

  void setSeed(int seed) { seed_ = seed; };
  void setVerbose(bool verbose) { verbose_ = verbose; };
  void setAllowLoops(bool allowLoops) { allowLoops_ = allowLoops; }
  void setIsAscend(bool isAscend) { isAscend_ = isAscend; }
  void setLocalConnect(bool localConnect) { localConnect_ = localConnect; }
  void setInsertFlop(bool insertFlop) { insertFlop_ = insertFlop; }
  void setSigmaTFactor(int sigmaTFactor) { sigmaTFactor_ = sigmaTFactor; }
  void setMinSeqBlocks(int minSeqBlocks) { minSeqBlocks_ = minSeqBlocks; }
  void setLocalCutOff(int localCutOff) { localCutOff_ = localCutOff; }
  void setMinInputs(int minInputs) { minInputs_ = minInputs; }
  void setMinOutputs(int minOutputs) { minOutputs_ = minOutputs; }
  void setMaxPathLen(int maxPathLen) { maxPathLen_ = maxPathLen; }
  void setMinPathLen(int minPathLen) { minPathLen_ = minPathLen; }
  void setMacroMaxPathLen(int maxPathLen) { macroMaxPathLen_ = maxPathLen; }
  void setMacroMinPathLen(int minPathLen) { macroMinPathLen_ = minPathLen; }
  void setPathLenCutOff(int pathLenCutOff) { pathLenCutOff_ = pathLenCutOff; }
  void setFlopCutOff(int flopCutOff) { flopCutOff_ = flopCutOff; }
  void setFlopDam(int flopDam) { flopDam_ = flopDam; }
  void setDistThreshold(int distThreshold) { distThreshold_ = distThreshold; }
  void setDistMeanT(float distMeanT) { distMeanT_ = distMeanT; }
  void setDistMeanG(float distMeanG) { distMeanG_ = distMeanG; }
  void setDistFactor(float distFactor) { distFactor_ = distFactor; }
  void setFlopInsertProb(float flopInsertProb)
  {
    flopInsertProb_ = flopInsertProb;
  }

 public:
  int getSeed() const { return seed_; }
  bool getVerbose() const { return verbose_; }
  bool getAllowLoops() const { return allowLoops_; }
  bool getIsAscend() const { return isAscend_; }
  bool getLocalConnect() const { return localConnect_; }
  bool getInsertFlop() const { return insertFlop_; }
  int getLocalCutOff() const { return localCutOff_; }
  int getSigmaTFactor() const { return sigmaTFactor_; }
  int getMinSeqBlocks() const { return minSeqBlocks_; }
  int getMinInputs() const { return minInputs_; }
  int getMinOutputs() const { return minOutputs_; }
  int getMaxPathLen() const { return maxPathLen_; }
  int getMinPathLen() const { return minPathLen_; }
  int getMacroMaxPathLen() const { return macroMaxPathLen_; }
  int getMacroMinPathLen() const { return macroMinPathLen_; }
  int getPathLenCutOff() const { return pathLenCutOff_; }
  int getFlopCutOff() const { return flopCutOff_; }
  int getFlopDam() const { return flopDam_; }
  int getDistThreshold() const { return distThreshold_; }
  float getDistMeanT() const { return distMeanT_; }
  float getDistMeanG() const { return distMeanG_; }
  float getDistFactor() const { return distFactor_; }
  float getFlopInsertProb() const { return flopInsertProb_; }
  // std::string getDefaultFlop() const { return flopCell_->getName(); }
  int getModuleCounter() const { return moduleCounter_; }

 public:
  static std::map<std::string, CellList> cellLists_;
  static bool allowLoops_;  // allow Combinational loops
  static bool isAscend_;  // combine according to ascending order of module size
  static bool verbose_;   // print debug info
  static bool localConnect_;  // connect local nets, which means nets within the
                              // same module
  static bool insertFlop_;    // allow flop insertion
  static int sigmaTFactor_;   // sigmaT factor
  static int minSeqBlocks_;   // minimum number of sequential blocks
  static int localCutOff_;    // minimum module size (on logarithmic scale) to
                              // allow local connection
  static int minInputs_;      // minimum number of inputs for a module
  static int minOutputs_;     // minimum number of outputs for a module
  static int pathLenCutOff_;  // path length cut off
  static int flopCutOff_;     // flop cut off
  static int flopDam_;        // starting point of reqFFNum_
  static int
      distThreshold_;       // terminal count distribution correction is applied
  static float distMeanT_;  // The target mean terminal count is corrected by
                            // this fraction of the error between the target and
                            // actual mean value
  static float distMeanG_;  // The target mean output fraction is corrected by
                            // this fraction of the error between the target and
                            // actual mean value
  static float distFactor_;  // The terminal distribution is corrected using a
                             // feedback mechanism.
  static float flopInsertProb_;  // flop insertion probability

  static Block* circuit_;
  static Librarycell* flopCell_;
  static DelayDist delay_;
  static int moduleCounter_;
  static std::string netlistFile_;
  static std::string clockName_;
  static bool isClock_;
  static bool isReset_;
  static bool netlistFlag_;
  static bool printForrest_;
  static bool printNetlistInfo_;
  static bool printDepth_;

 public:
  // read/write
  void readSpec();
  void createSpec(const char* topModule,
                  int numInsts,
                  int numMacros,
                  int numPIs,
                  int numPOs,
                  float region1,
                  float avgFanin,
                  float meanG,
                  float sigmaG,
                  float p,
                  float q,
                  float seqRatio,
                  int seed,
                  const char* onlyUseList,
                  const char* specFile);

  void extractSpec(const char* fileName,
                   const char* out_dir,
                   bool is_flat,
                   int level,
                   float mini_brain,
                   int threshold);
  bool BuildLogicalHierarchy();
  void BuildSubTrees(HierNodePtr node);
  void HTreeCoarsening(int level);
  void writeSpec(const char* fileName,
                 int level,
                 int threshold,
                 float mini_brain,
                 const char* out_dir);
  void writeNodeSpec(std::ofstream& outFile, HierNodePtr node);
  void getModuleIONum(HierNodePtr node,
                      std::vector<HierNodePtr> subtree,
                      int& numPI,
                      int& numPO);
  void writeSpec_v2(float mini_brain, const char* fileName);
  void BuildTimingPaths(int top_n_, bool minmax);
  void writeNodeVerilog(HierNodePtr node, const char* out_dir);
  void writeSubmoduleVerilog(const char* port_prefix = "partition_",
                             const char* module_suffix = "_partition");
  void writePartitionVerilog(const char* file_name,
                             const char* port_prefix = "partition_",
                             const char* module_suffix = "_partition");

  // Not Used in ANG flow --> just for experiments (test.cpp)
  /*****************************************************/
  typedef std::pair<int, int> IntPair;
  void printTree();
  void writeGNLfile(const char* fileName);
  void convHNL2Netlist(const char* inFile, const char* outFile);
  void reportCellDist(const char* fileName);
  void CalculateRentParam();
  void FloatingCells();
  void FanoutWL();
  void reportMetrics();
  void reportLogicDepth();
  void EdgeBbox(const char* fileName);
  void RealIdeal(float rent_constant,
                 float rent_exponent,
                 int numInsts,
                 const char* fileName);
  /****************************************************/
  // void analyzeTopoOrder();

  ArtNetGen();
  ~ArtNetGen();

 private:
  odb::dbDatabase* db_;
  sta::dbSta* sta_;
  sta::dbNetwork* dbNetwork_;
  sta::Graph* graph_;
  sta::Network* network_;
  utl::Logger* logger_;
  odb::dbBlock* block_ = nullptr;

  HierTreePtr htree_ = nullptr;

  int seed_;
  std::string specFile_;
  std::string flop_;
  std::vector<std::string> dontUses_;
  std::vector<std::string> macros_;

  int maxPathLen_;       // maximum path length
  int minPathLen_;       // minimum path length
  int macroMaxPathLen_;  // maximum path length connected to macro
  int macroMinPathLen_;  // minimum path length connected to macro

  // partitioner
  odb::dbBlock* getDbBlock() const;
  sta::Instance* buildPartitionedInstance(
      const char* name,
      const char* port_prefix,
      sta::Library* library,
      sta::NetworkReader* network,
      sta::Instance* parent,
      const std::set<sta::Instance*>* insts,
      std::map<sta::Net*, sta::Port*>* port_map);

  sta::Instance* buildPartitionedTopInstance(const char* name,
                                             sta::Library* library,
                                             sta::NetworkReader* network);
};

// helper function
void getMTermNames(odb::dbMaster* master,
                   std::string dir,
                   std::string mode,
                   std::vector<std::string>& names);
std::unordered_map<std::string, int> getPortInfo(odb::dbMaster* master);
int randomIntInRange(int min, int max);
int randomInt(int mod);
double uniformDist();

template <typename T>
void randomVector(std::vector<T>& c)
{
  std::random_shuffle(c.begin(), c.end());
}

template <class List>
void randomizeList(List& l)
{
  std::vector<typename List::iterator> position;
  position.reserve(l.size());
  List n;
  position.push_back(n.end());
  while (!l.empty()) {
    position.push_back(
        n.insert(position[randomInt(position.size())], l.front()));
    l.pop_front();
  }
  l = n;
}

double normGaussianDist();
double gaussianDist(double mean, double sigma);

}  // namespace ang
#endif
