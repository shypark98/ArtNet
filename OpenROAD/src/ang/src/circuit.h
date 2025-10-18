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

#include "ang/artNetGen.h"
#include "cell.h"

namespace ang {

//  - If ptr exists as a key, return the value for that key.
class CounterMap
{
 public:
  CounterMap() : next_(0) {}
  // '[]' operator overloading
  // If ptr exists as a key, return the value for that key.
  // else if ptr does not exist as a key,
  // add a new key and return the next_ value, then increase the next_ value
  // by 1.
  int operator[](void* ptr);

 private:
  std::map<void*, int> counterMap_;
  int next_;
};

// Block is a class that represents a submodule in the design
// Cluster is a set of cells

// in Cluster, cells have a connection, but in Block, not

//                                 (instance, block, module)
//                                 +---------------------+
//                                 |        cell_        |
//                                 |                     | +---+ sinks_ | ..|..
//                                 outNets_[0] -- (OutputNet1) ---+
//     (InputNet1) -- inNets_[0] ..|..                   |
//                                 |                     |
//                                 |                   ..|.. outNets_[1] --
//                                 (OutputNet2) ---+ sinks_ | |
//     (InputNet2) -- inNets_[1] ..|..                   |
//                                 |                   ..|.. outNets_[2] --
//                                 (OutputNet3) ---+ |                     |
//                                 +---+ sinks_ |                     |
//                                 +---------------------+
//    ex)  InputNet1.connectOutputs_ = {(OutputNet1, 0.5), (OutputNet2, 0.3),
//    (OutputNet3, 0.2)}
//
//

// area_ in Cluster is the sum of the area of the cells in the module
// numBlocks_ in Cluster is the number of cells in the module
// --> need to fix the naming convention (numBlocks_ should be numInsts_)

// in class CELL --> add term2pin_ (std::map<int, string> term2pin_)
// int --> index of the terminal, string --> name of the pin (from )
// dbSet<dbMTerm> terms = master->getMTerms();
//

class Block : public CELL
{
 public:
  Block() : CELL(-1, -1, -1), number_(0) {}

  typedef std::pair<int, int> IntPair;

 private:
  struct TreeNode;
  struct CompoundNode;
  struct InstNode;
  struct SubModuleNode;

 public:
  struct Region
  {
    Region() : meanT(-1), sigmaT(-1), p(10), q(10), meanG(-1), sigmaG(-1) {}

    double meanT;
    double sigmaT;
    double p;
    double q;
    double meanG;
    double sigmaG;
    double g_factor;
  };

  void writeRegionInfo(std::ofstream& outFile, std::string prefix);

  // class that stores the distribution of T and G in this class
  struct DistribBucket
  {
    DistribBucket() : sumT(0), sumG(0), number(0) {}

    void addData(int T, double g)
    {
      sumT += T;
      sumG += g;
      number++;
    }
    double getMeanT() { return double(sumT) / number; }
    double getMeanG() { return double(sumG) / number; }
    unsigned long sumT;
    double sumG;
    unsigned int number;
    double newMeanT;
    double newMeanG;
  };

 public:
  void checkRegion();  // check if the region is valid
  void addCellList(std::string libName);
  void addDistribution(int value);
  void addFaninRatio(int count, double value);
  void addFanoutRatio(int count, double value);
  void addNodeShape(int count, double value);
  void addPIShape(int count, double value);
  void addPOShape(int count, double value);
  void addFFShape(int count, double value);
  void addRegion(int regionBound, Region region);
  void writeDat();
  void fillBuckets();
  void WriteDatWord(std::ofstream& out, double value, bool g = 0);
  void getMeanIO(double size,
                 double& meanT,
                 double& meanI,
                 double& meanO,
                 double& meanG,
                 double& sigmaT,
                 double& sigmaG);

  void WriteDatLine(std::ofstream& out,
                    double B,
                    double tarT,
                    double tarI,
                    double tarO,
                    double tarG,
                    double tarStdevT,
                    double tarStdevG,
                    double T,
                    double I,
                    double O,
                    double g,
                    double stdevT,
                    double stdevG);

 public:
  std::map<int, Region> getRegions() { return regions_; }
  int getMaxT(int numBlocks);
  void getIO(int area, int& parIc, int& parOc);
  void getRent(int area, int& endRegion, double& p, double& k);
  virtual int getNumBlocks() { return numBlocks_; }
  std::string getBlockName() { return blockName_; }
  virtual bool isSequential() { return true; }
  std::list<int> getDistribution() { return distribution_; }
  std::multimap<IntPair, TreeNode*> getPQueue() { return pqueue_; }

  int getFFNum() { return numFF_; }
  int getCombiNum() { return numCombi_; }
  double getSeqRatio() { return double(numFF_) / (numFF_ + numCombi_); }

 public:
  class Cluster* Generate();  // GetInstance() in gnl
  void InitName();
  void InitQueue();
  void InsertNode(TreeNode* node);
  void InsertSeqNode(Librarycell* cell);
  void HClustering();
  void putIO(int area, int inputs, int outputs);

 public:
  void refineDist();
  void initFoDist(std::vector<DistUnit> fo_dist);
  void initDepthDist(std::vector<DistUnit> depth_dist);
  void initNodeLev(std::vector<DistUnit> node_level);
  void initPILev(std::vector<DistUnit> pi_level);
  void initPOLev(std::vector<DistUnit> po_level);
  double foGain(int v1, int v2);
  double nodeLevGain(int v1, int v2);
  double piLevGain(int v1, int v2);
  double poLevGain(int v1, int v2);
  double ffLevGain(int v1, int v2);

 public:
  // queue for flip-flops
  std::multimap<IntPair, Librarycell*> getSeqForrest() { return seqForrest_; }
  std::multimap<IntPair, Librarycell*> seqForrest_;

 private:
  void DeleteNode(TreeNode* node);
  void DeleteTree();

 private:
  std::string blockName_;
  int number_;
  int numBlocks_;

  int numFF_ = 0;
  int numCombi_ = 0;

  std::list<std::string> cellList_;
  std::list<int> distribution_;
  std::map<int, Region> regions_;

  Distrib foDist_;
  Distrib depthDist_;
  Distrib nodeLev_;
  Distrib piLev_;
  Distrib poLev_;

  DistribBucket& DistributionBucket(int area);
  // priority queue consists of combi logics and submodules
  std::multimap<IntPair, TreeNode*> pqueue_;

  std::map<int, DistribBucket> distribBuckets_;
  std::map<int, std::list<TreeNode*>> buckets_;

 public:
  // for debugging
  void printTree();
};

inline bool operator<(Block::IntPair& a, Block::IntPair& b)
{
  return a.first < b.first || (a.first == b.first && a.second < b.second);
}

class Cluster
{
 public:
  Cluster(Librarycell* cell);
  Cluster(Cluster* clustA, Cluster* clustB, Block* block);
  ~Cluster();
  typedef std::pair<int, int> IntPair;

  void postProcessing(Block* block);
  void addPI(int& tarNum, Block* block);
  void delPI(int& tarNum, Block* block);
  void addPO(int& tarNum, Block* block);
  void delPO(int& tarNum, Block* block);

  void checkConsistency();
  int getNumBlocks() const { return numBlocks_; }
  int getNumInputs() const { return numInputs_; }
  int getNumOutputs() const { return numOutputs_; }
  int getArea() const { return area_; }
  int getNumber() const { return number_; }
  void writeNetlistInfo(std::ofstream& outFile,
                        Block* block,
                        std::string prefix);
  void writeVerilog(Block* block);

  struct Instance;
  struct Net;
  struct InputNet;
  struct OutputNet;
  // Terminal is a pair of Instance and int, int means the index of the
  // input/output pins
  typedef std::pair<Instance*, int> Terminal;

 private:
  void Merge(Cluster* cluster);

  // struct Instance;
  // struct Net;
  // struct InputNet;
  // struct OutputNet;
  //  Terminal is a pair of Instance and int, int means the index of the
  //  input/output pins
  // typedef std::pair<Instance *, int> Terminal;
 private:
  std::list<Instance*> instances_;
  std::list<Instance*> FFs_;
  std::list<Instance*> Combis_;
  std::list<Instance*> startPoints_;

  std::list<InputNet*> inNets_;
  std::list<OutputNet*> outNets_;
  std::list<OutputNet*> internalNets_;
  std::unordered_map<int, std::list<Instance*>> lvedInsts;
  int area_;
  int max_level_;
  int min_level_;
  int reqFFNum_ = 0;
  int numBlocks_;
  int numInputs_ = 0;
  int numOutputs_;
  int number_;
  friend struct Net;
  friend struct InputNet;
  friend struct OutputNet;

 public:
  // for logic dist matching
  int getNumFFs() { return FFs_.size(); }
  double getSeqRatio() { return double(FFs_.size()) / numBlocks_; }
  void swapOutput();
  void sortInternalNet();
  void sortOutNet();
  void sortOutList(std::list<OutputNet*>& newOutputs);
  void sortInNet();
  void revsortInNet();
  void moveNtoFront(int n);
  void moveNtoBack(int n);
  void matchNets(std::list<OutputNet*>& outNets, std::list<InputNet*>& inNets);

  void levelize();
  void unlevelizeCombis();
  void checkSequential();
  void checkStartPoints();
  void levelizeCombis();
  void printDepth();
  void calcCombiLevel(Instance* inst,
                      std::deque<Instance*>& candiInsts,
                      bool isIncremental);
  void getUnvisitedSinks(Instance* inst,
                         std::deque<Instance*>& candiInsts,
                         bool isIncremental);
  void unlevelizeOutcones(Instance* inst);
  void incrementalLevelize(Instance* inst);
  void checkLevelConsistency();
  void insertFlop(Instance* inst, Block* block, int outTerm);
  void addNumInputs(int incr) { numInputs_ += incr; }
  void addNumOutputs(int incr) { numOutputs_ += incr; }

 public:
  // for debugging
  void printPI();
  void printPO();
};

struct Cluster::Instance
{
  Instance(Librarycell* cell)
      : inNets_(cell->getNumInputs()),
        outNets_(cell->getNumOutputs()),
        cell_(cell),
        isSequential_(cell->isSequential())
  {
  }

  void checkConsistency();
  bool isSequential() { return isSequential_; }
  int getArea() { return cell_->getArea(); }
  std::list<Instance*> getFanin();

  std::vector<class Net*> inNets_;
  std::vector<class OutputNet*> outNets_;
  Librarycell* cell_;
  int number_;
  int minLevel_ = INT_MAX;
  int maxLevel_ = INT_MIN;
  bool isVisited_ = false;
  bool isSequential_ = false;
};

struct Cluster::Net
{
 public:
  virtual ~Net() {}
  void connect(InputNet* inNet);
  void checkConsistency();
  std::list<Terminal> sinks_;
};

struct Cluster::InputNet : public Cluster::Net
{
 public:
  InputNet(double minPath, double maxPath)
      : minPath_(minPath), maxPath_(maxPath)
  {
  }

  bool connect(InputNet* inNet);

 public:
  // minPath_ is required min path length
  // maxPath_ is allowed max path length
  double minPath_;
  double maxPath_;
  std::map<OutputNet*, double> connectedOutputs_;
  // public:
  //     int getSinkLevel(sinks_[0]->first->level_);
};

struct Cluster::OutputNet : public Cluster::Net
{
 public:
  OutputNet(double maxLen) : source_(Terminal(0, 0)), maxLen_(maxLen) {}

  bool connect(InputNet* inNet,
               Cluster* clustA,
               Cluster* clustB,
               Cluster* clustC,
               double delayScaleFactor,
               Block* block);
  void checkConsistency();
  void addFlop(Cluster* clustA, Cluster* clustB, Cluster* clustC, Block* block);
  void MakeInternal(Cluster* clustA, Cluster* clustB, Cluster* clustC);

 public:
  Terminal source_;
  double maxLen_;
  bool isDone_ = false;
  // public:
  //     int getSourceLevel(source_->first->level_);
};

class Block::TreeNode
{
 public:
  virtual int getArea() = 0;
  virtual int getNumBlocks() = 0;
  virtual int getNumInputs() = 0;
  virtual int getNumOutputs() = 0;
  int getTerminals() { return getNumInputs() + getNumOutputs(); }
  double getG()
  {
    return double(getNumOutputs()) / (getNumInputs() + getNumOutputs());
  }
  virtual Cluster* BuildCluster(Block* block) = 0;
  virtual void FillBucketsWithTree(
      std::map<int, std::list<TreeNode*>>& buckets_);
};

class Block::CompoundNode : public Block::TreeNode
{
 public:
  CompoundNode(TreeNode* left, TreeNode* right)
      : left_(left),
        right_(right),
        area_(left->getArea() + right->getArea()),
        numBlocks_(left->getNumBlocks() + right->getNumBlocks()),
        numInputs_(-1),
        numOutputs_(-1)
  {
  }

  virtual int getArea() { return area_; }
  virtual int getNumBlocks() { return numBlocks_; }
  virtual int getNumInputs() { return numInputs_; }
  virtual int getNumOutputs() { return numOutputs_; }
  virtual Cluster* BuildCluster(Block* block);
  virtual void FillBucketsWithTree(
      std::map<int, std::list<TreeNode*>>& buckets_);

  TreeNode* getLeft() { return left_; }
  TreeNode* getRight() { return right_; }

 private:
  TreeNode* left_;
  TreeNode* right_;
  int area_;
  int numBlocks_;
  int numInputs_;
  int numOutputs_;
  friend void Block::DeleteNode(TreeNode*);
};

class Block::InstNode : public Block::TreeNode
{
 public:
  InstNode(Librarycell* cell) : cell_(cell) {}
  virtual int getArea() { return cell_->getArea(); }
  virtual int getNumBlocks() { return 1; }
  virtual int getNumInputs() { return cell_->getNumInputs(); }
  virtual int getNumOutputs() { return cell_->getNumOutputs(); }
  virtual Cluster* BuildCluster(Block* block);
  Librarycell* getCell() { return cell_; }
  Librarycell* cell_;
};

class Block::SubModuleNode : public Block::TreeNode
{
 public:
  SubModuleNode(Block* block) : block_(block) {}
  virtual int getArea() { return block_->getArea(); }
  virtual int getNumBlocks() { return block_->getNumBlocks(); }
  virtual int getNumInputs() { return numInputs_; }
  virtual int getNumOutputs() { return numOutputs_; }
  virtual Cluster* BuildCluster(Block* block);

 private:
  int numInputs_;
  int numOutputs_;
  Block* block_;
};

}  // namespace ang
