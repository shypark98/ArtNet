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

#include <chrono>
#include <cmath>

#include "circuit.h"

using utl::ANG;

using odb::dbMaster;

using std::cout;
using std::endl;
using std::exp;
using std::find;
using std::list;
using std::log;
using std::make_pair;
using std::map;
using std::multimap;
using std::ofstream;
using std::pair;
using std::pow;
using std::sqrt;
using std::string;
using std::vector;

namespace ang {

Cluster* Block::Generate()
{
  bool isTop = (this == ArtNetGen::circuit_);

  std::chrono::system_clock::time_point start;
  std::chrono::duration<double> runtime;

  InitName();

  // Priority Queue Initialization
  start = std::chrono::system_clock::now();
  InitQueue();
  if (isTop) {
    runtime = std::chrono::system_clock::now() - start;
    cout << "\t-- Priority Queue Initialization Finished (" << runtime.count()
         << " sec)" << endl;
  }

  if (ArtNetGen::verbose_) {
    cout << "Priority Queue: " << endl;

    for (multimap<IntPair, TreeNode*>::iterator it = pqueue_.begin();
         it != pqueue_.end();
         ++it) {
      auto inode = dynamic_cast<InstNode*>(it->second);
      cout << inode->getCell()->getName() << endl;
    }
  }

  // Hierarchical Clustering
  start = std::chrono::system_clock::now();
  HClustering();

  if (isTop) {
    runtime = std::chrono::system_clock::now() - start;
    cout << "\t-- Hierarchical Clustering Finished (" << runtime.count()
         << " sec)" << endl;
  }

  buckets_.clear();
  distribBuckets_.clear();

  // Net Generation
  // module --> Root Node of HTree
  start = std::chrono::system_clock::now();
  Cluster* module = pqueue_.begin()->second->BuildCluster(this);

  if (isTop) {
    runtime = std::chrono::system_clock::now() - start;
    cout << "\t-- Circuit generation Finished (" << runtime.count() << " sec)"
         << endl;
  }

  module->checkConsistency();

  /*
  if (isTop) {
      module->postProcessing(this);
  }*/

  if (isTop && ArtNetGen::printDepth_) {
    module->levelize();

    cout << "levelizing done " << endl;
    module->printDepth();
  }

  // Write verilog netlist
  start = std::chrono::system_clock::now();

  if (isTop && ArtNetGen::netlistFlag_) {
    module->writeVerilog(this);
    runtime = std::chrono::system_clock::now() - start;
    cout << "\t-- Writing verilog netlist Finished (" << runtime.count()
         << " sec)" << endl;
  }

  DeleteTree();

  if (isTop)
    cout << "delete Tree done" << endl;

  return module;
}

void Block::InitName()
{
  blockName_ = name_;
  if (ArtNetGen::circuit_ != this)
    blockName_ += "_" + std::to_string(++number_);
}

void Block::InitQueue()
{
  if (!pqueue_.empty()) {
    cout << "Error: forrest is not empty" << endl;
    exit(0);
  }

  list<int>::iterator dit = distribution_.begin();
  for (auto lit = cellList_.begin(); lit != cellList_.end(); ++lit) {
    auto it = ArtNetGen::cellLists_.find(*lit);

    if (it == ArtNetGen::cellLists_.end()) {
      cout << "Error: library " << (*lit) << " not found" << endl;
      exit(0);
    }

    for (auto cit = it->second.cells.begin(); cit != it->second.cells.end();
         ++cit) {
      for (int i = 0; i < *dit; ++i) {
        auto cell = dynamic_cast<Librarycell*>(*cit);
        auto block = dynamic_cast<Block*>(*cit);

        if (cell) {
          if (auto property
              = odb::dbIntProperty::find(cell->getDbMaster(), "macro_id")) {
            cell->isMacro = true;
            numFF_++;
            InsertNode(new InstNode(cell));

          } else {
            if (cell->isSequential()) {
              numFF_++;
              InsertSeqNode(cell);
            } else {
              numCombi_++;
              InsertNode(new InstNode(cell));
            }
          }
        } else if (block) {
          // UPDATE DISTRIBUTION INFO TO THIS BLOCK
          numFF_ += block->getFFNum();
          numCombi_ += block->getCombiNum();
          InsertNode(new SubModuleNode(block));
        } else {
          cout << "Error: unknown cell type" << endl;
          exit(0);
        }
      }
      ++dit;
    }
  }
}

// insert node to priority queue
void Block::InsertNode(TreeNode* node)
{
  int indexA = 0;
  int indexB = 0;

  // if ascend is true, combine nodes according to the area
  if (ArtNetGen::isAscend_) {
    indexA = node->getArea();
  }
  // Insert node into priority queue (pqueue_) in the order of indexA and indexB
  // If there is a node with the same indexA, insert the node after the node
  // with the same indexA If there is a node with the same indexB, insert the
  // node before the node with the same indexB
  auto fit = pqueue_.upper_bound(IntPair(indexA, -1));

  if (fit != pqueue_.end()) {
    indexB = fit->first.second;
  }

  pqueue_.insert(pair<IntPair, TreeNode*>(
      IntPair(indexA, randomIntInRange(indexB, INT_MAX)), node));
}

void Block::InsertSeqNode(Librarycell* seqCell)
{
  // int indexA = seqCell->getNumInputs();
  int indexA = 0;
  int indexB = randomIntInRange(0, INT_MAX);

  seqForrest_.insert(
      pair<IntPair, Librarycell*>(IntPair(indexA, indexB), seqCell));
}

void Block::HClustering()
{
  // cout << pqueue_.size() << endl;
  //  Hierarchical clustering from priority queue (pqueue_)
  //  Iterate through pqueue_ and combine nodes until there is only one node
  //  left
  while (pqueue_.size() > 1) {
    auto nodeA = pqueue_.begin()->second;
    pqueue_.erase(pqueue_.begin());

    InstNode* inodeA = dynamic_cast<InstNode*>(nodeA);
    // if nodeA is a library node and it is sequential, insert it back to
    // pqueue_
    if (inodeA && inodeA->getCell()->isSequential() && !ArtNetGen::isAscend_) {
      InsertNode(nodeA);

    } else {
      TreeNode* nodeB = pqueue_.begin()->second;
      pqueue_.erase(pqueue_.begin());

      // if nodeB is a single block and nodeA is not, swap nodeA and nodeB
      if ((nodeB->getNumBlocks() == 1) && (nodeA->getNumBlocks() > 1)) {
        auto temp = nodeA;
        nodeA = nodeB;
        nodeB = temp;
      }

      if (nodeA->getNumBlocks() == 1 && nodeB->getNumBlocks() >= 1
          && (nodeA->getTerminals() > getMaxT(nodeB->getNumBlocks())
              || (nodeB->getNumBlocks() == 1
                  && nodeB->getTerminals() > getMaxT(1)))) {
        InsertNode(nodeA);
        InsertNode(nodeB);
      } else {
        InstNode* inodeB = dynamic_cast<InstNode*>(nodeB);
        if (inodeB && inodeB->getCell()->isSequential()
            && (nodeA->getNumBlocks() <= ArtNetGen::minSeqBlocks_)) {
          InsertNode(nodeA);
          InsertNode(nodeB);
        } else {
          InsertNode(new CompoundNode(nodeA, nodeB));
        }  // if lnodeB
      }  // if nodeA
    }  // if lnodeA
  }  // while
}

// convert instance to Cluster
Cluster* Block::InstNode::BuildCluster(Block* block)
{
  return new Cluster(cell_);
}

// convert SubModule to Cluster
Cluster* Block::SubModuleNode::BuildCluster(Block* block)
{
  // Build cluster from macro node
  Cluster* cluster = block_->Generate();

  // Set the number of inputs and outputs of the module
  numInputs_ = cluster->getNumInputs();
  numOutputs_ = cluster->getNumOutputs();

  return cluster;
}

// convert cluster to Cluster
Cluster* Block::CompoundNode::BuildCluster(Block* block)
{
  // Build module from left and right nodes
  Cluster* cluster = new Cluster(
      left_->BuildCluster(block), right_->BuildCluster(block), block);

  // Set the number of inputs and outputs of the module
  area_ = cluster->getArea();
  numBlocks_ = cluster->getNumBlocks();
  numInputs_ = left_->getNumInputs();
  numOutputs_ = right_->getNumOutputs();

  return cluster;
}

// check if the region is valid
// if the region is not valid, print an error message and return
// if the region is valid, initialize the region when B == 1
void Block::checkRegion()
{
  int totalT = 0;
  double totalG = 0;
  numBlocks_ = 0;
  area_ = 0;

  list<int>::iterator dit = distribution_.begin();
  for (auto li = cellList_.begin(); li != cellList_.end(); ++li) {
    string libName = *li;
    list<CELL*>& cells = ArtNetGen::cellLists_[libName].cells;
    for (auto it = cells.begin(); it != cells.end(); ++it) {
      if (dit == distribution_.end()) {
        cout << "Error: distribution size is smaller than the number of cells"
             << endl;
        exit(0);
      }
      auto cell = *it;
      totalT += cell->getTerminals() * (*dit);
      totalG += cell->getG() * (*dit);
      numBlocks_ += cell->getNumBlocks() * (*dit);
      area_ += cell->getArea() * (*dit);
      ++dit;
    }
  }
  if (dit != distribution_.end()) {
    cout << "Error: distribution size is larger than the number of cells"
         << endl;
    exit(0);
  }

  double meanT = double(totalT) / numBlocks_;
  double meanG = totalG / numBlocks_;
  double totalSqDivT = 0;
  double totalSqDivG = 0;
  dit = distribution_.begin();

  // In Block, cellList_ has the key value of ArtNetGen::cellLists_ in
  // std::string distribution_ has the number of cells in each cellList_ The
  // order of cellList_ and distribution_ is the same

  for (auto li = cellList_.begin(); li != cellList_.end(); ++li) {
    string libName = *li;
    list<CELL*>& cells = ArtNetGen::cellLists_[libName].cells;
    for (auto it = cells.begin(); it != cells.end(); ++it) {
      double divT = (*it)->getTerminals() - meanT;
      double divG = (*it)->getG() - meanG;
      totalSqDivT += divT * divT * (*dit);
      totalSqDivG += divG * divG * (*dit);
      ++dit;
    }
  }

  double sigmaT = sqrt(totalSqDivT / numBlocks_);
  double sigmaG = sqrt(totalSqDivG / numBlocks_);

  // initialize region when B == 1
  if (regions_.begin()->first != 1)
    regions_[1] = Region();
  if (regions_[1].meanT < 0)
    regions_[1].meanT = meanT;
  if (regions_[1].sigmaT < 0)
    regions_[1].sigmaT = sigmaT;
  if (regions_[1].meanG < 0)
    regions_[1].meanG = meanG;
  if (regions_[1].sigmaG < 0)
    regions_[1].sigmaG = sigmaG;

  // check if the end region is valid
  if (regions_.rbegin()->first != area_) {
    cout << numBlocks_ << " " << regions_.rbegin()->first << endl;
    cout << "Error: region bound is not equal to the number of blocks" << endl;
    exit(0);
  }

  if (numInputs_ >= 0 && numOutputs_ >= 0) {
    regions_.rbegin()->second.meanT = numInputs_ + numOutputs_;
    regions_.rbegin()->second.meanG
        = double(numOutputs_) / (numInputs_ + numOutputs_);
  }

  // check if other regions are valid
  for (auto rit = ++regions_.begin(); rit != regions_.end(); ++rit) {
    int regionBound = rit->first;
    Region& region = rit->second;

    if (rit->first < 1 || rit->first > numBlocks_) {
      cout << "Error: region " << regionBound << " is out of range" << endl;
      exit(0);
    }

    // get previous region
    auto prev = rit;
    --prev;

    // check meanT and p
    if (region.meanT >= 0 && region.p < 5) {
      cout << "Error: cannot specify both meanT and p" << endl;
      exit(0);
    }
    if (region.meanT >= 0)
      region.p = log(region.meanT / prev->second.meanT)
                 / log(double(regionBound) / prev->first);
    else {
      if (region.p >= 5)
        region.p = 0.55;
      region.meanT = prev->second.meanT
                     * pow(double(regionBound) / prev->first, region.p);
    }

    // check sigmaT and q

    if (region.sigmaT >= 0 && region.q < 5) {
      cout << "Error: cannot specify both sigmaT and q" << endl;
      exit(0);
    }
    if (region.sigmaT >= 0) {
      if (region.sigmaT < 0.01)
        region.q = -100;
      else
        region.q = log(region.sigmaT / prev->second.sigmaT)
                   / log(double(regionBound) / prev->first);
    } else {
      if (region.q >= 5)
        region.q = region.p - 0.1;
      region.sigmaT = prev->second.sigmaT
                      * pow(double(regionBound) / prev->first, region.q);
    }

    // check meanG and sigmaG

    if (region.meanG < 0)
      region.meanG = 0.3;
    if (region.sigmaG < 0)
      region.sigmaG = 0.1;

    // check g_factor (growth factor in meanG between regions)
    region.g_factor = (region.meanG - prev->second.meanG)
                      / log(double(regionBound) / prev->first);
  }

  // check IO
  if (numInputs_ < 0)
    numInputs_ = int(regions_.rbegin()->second.meanT
                         * (1 - regions_.rbegin()->second.meanG)
                     + 0.5);
  if (numOutputs_ < 0)
    numOutputs_
        = int(regions_.rbegin()->second.meanT * regions_.rbegin()->second.meanG
              + 0.5);
}

void Block::getMeanIO(double size,
                      double& meanT,
                      double& meanI,
                      double& meanO,
                      double& meanG,
                      double& sigmaT,
                      double& sigmaG)
{
  map<int, Region>::iterator rit = regions_.lower_bound(int(size)), prev;

  if (rit == regions_.begin())
    ++rit;

  if (rit == regions_.end()) {
    rit = regions_.lower_bound(area_);
    size = area_;
  }

  prev = rit;
  --prev;

  meanT = prev->second.meanT * pow(size / prev->first, rit->second.p);
  meanG = prev->second.meanG + rit->second.g_factor * log(size / prev->first);
  meanI = meanT * (1 - meanG);
  meanO = meanT * meanG;
  sigmaT = prev->second.sigmaT * pow(size / prev->first, rit->second.q);
  sigmaG = prev->second.sigmaG;
}

// get the maximum T for a given number of blocks
int Block::getMaxT(int numBlocks)
{
  // find the first region that contains equal or more blocks than numBlocks
  map<int, Region>::iterator rit = regions_.lower_bound(numBlocks);

  // if there is no such region, find the first region that contains less blocks
  // than numBlocks
  if (rit == regions_.begin())
    ++rit;

  if (rit == regions_.end()) {
    rit = regions_.lower_bound(area_);
    numBlocks = area_;
  }

  // get the previous region
  map<int, Region>::iterator prev = rit;
  --prev;

  // calculate the maximum T
  // meanT = meanT_prev * (size/size_prev)^p
  // sigmaT = sigmaT_prev * (size/size_prev)^q
  // maxT = meanT + sigmaTFactor * sigmaT
  double meanT
      = prev->second.meanT * pow(numBlocks / prev->first, rit->second.p);
  double sigmaT
      = prev->second.sigmaT * pow(numBlocks / prev->first, rit->second.q);

  return int(meanT + ArtNetGen::sigmaTFactor_ * sigmaT);
}

void Block::getRent(int area, int& endRegion, double& p, double& k)
{
  map<int, Region>::iterator rit = regions_.lower_bound(area);
  if (rit == regions_.begin()) {
    rit++;
  }

  endRegion = rit->first;
  p = rit->second.p;
  k = rit->second.meanT;
}

void Block::getIO(int area, int& parIc, int& parOc)
{
  if (area >= area_) {
    parIc = numInputs_;
    parOc = numOutputs_;
    return;
  }

  map<int, Region>::iterator rit = regions_.lower_bound(area);
  map<int, Region>::iterator prev;

  // first region is B == 1, so skip it
  if (rit == regions_.begin()) {
    rit++;
  }
  if (rit == regions_.end()) {
    cout << "Error: size out of bound" << endl;
    exit(0);
  }

  prev = rit;
  --prev;

  double meanT
      = prev->second.meanT * pow(double(area) / prev->first, rit->second.p);
  double sigmaT
      = prev->second.sigmaT * pow(double(area) / prev->first, rit->second.q);
  double meanG = prev->second.meanG
                 + rit->second.g_factor * log(double(area) / prev->first);
  double sigmaG = rit->second.sigmaG;

  // sample from difference of target and actual distribution
  DistribBucket& bucket = DistributionBucket(area);
  double T, G;

  if (int(bucket.number) < ArtNetGen::distThreshold_) {
    T = ang::gaussianDist(meanT, sigmaT);
    G = ang::gaussianDist(meanG, sigmaG);
    bucket.newMeanT = meanT;
    bucket.newMeanG = meanG;
  } else {
    bucket.newMeanT += (meanT - bucket.getMeanT()) * ArtNetGen::distMeanT_;
    double newMeanG = (meanG - bucket.getMeanG()) * ArtNetGen::distMeanG_;

    if ((newMeanG > meanG - 2 * sigmaG) && (newMeanG < meanG + 2 * sigmaG)) {
      bucket.newMeanG += newMeanG;
    }
    T = ang::gaussianDist(bucket.newMeanT, sigmaT);
    G = ang::gaussianDist(bucket.newMeanG, sigmaG);
  }

  // parIc = int(T * (1 - 0.5*G) + 0.5);
  // parOc = int(T * 0.5*G + 0.5);
  parIc = int(T * (1 - G) + 0.5);
  parOc = int(T * G + 0.5);

  // cout << "prevG: " << prev->second.meanG << endl;
  // cout << "MeanG: " << meanG << endl;
  // cout << "parIc: " << parIc << endl;
  // cout << "parOc: " << parOc << endl;

  if (ArtNetGen::verbose_) {
    cout << "meanT: " << meanT << " sigmaT: " << sigmaT << " meanG: " << meanG
         << " sigmaG: " << sigmaG << endl;
    cout << "T: " << T << " G: " << G << endl;
    cout << "parIc: " << parIc << " parOc: " << parOc << endl;
  }
}

void Block::DeleteTree()
{
  if (pqueue_.size() != 1) {
    cout << "Error: forrest size is not 1" << endl;
    exit(0);
  }
  DeleteNode(pqueue_.begin()->second);
  pqueue_.clear();
}

void Block::DeleteNode(TreeNode* node)
{
  CompoundNode* compound = dynamic_cast<CompoundNode*>(node);
  if (compound) {
    DeleteNode(compound->left_);
    DeleteNode(compound->right_);
  }
  delete node;
}

void Block::addCellList(std::string libName)
{
  if (find(cellList_.begin(), cellList_.end(), libName) != cellList_.end()) {
    cout << "Warning: library " << libName << " already exists in module "
         << name_ << endl;
  } else {
    // cout << "Adding library " << libName << " to module " << name_ << endl;
    cellList_.push_back(libName);
  }
}

void Block::addDistribution(int value)
{
  distribution_.push_back(value);
}

void Block::refineDist()
{
  /*
  if (!foDist_.empty())
      foDist_.refine(numBlocks_);

  if (!depthDist_.empty())
      depthDist_.refine(numBlocks_);

  if (!nodeLev_.empty())
      nodeLev_.refine(numBlocks_);

  if (!piLev_.empty())
      piLev_.refine(numBlocks_);

  if (!piLev_.empty())
      poLev_.refine(numBlocks_);
  */
}

void Block::initFoDist(vector<DistUnit> fo_dist)
{
  foDist_.init(fo_dist);
}

void Block::initDepthDist(vector<DistUnit> depth_dist)
{
  depthDist_.init(depth_dist);
}

void Block::initNodeLev(vector<DistUnit> node_level)
{
  nodeLev_.init(node_level);
}

void Block::initPILev(vector<DistUnit> pi_level)
{
  piLev_.init(pi_level);
}

void Block::initPOLev(vector<DistUnit> po_level)
{
  poLev_.init(po_level);
}

void Block::addRegion(int regionBound, Region region)
{
  if (regions_.find(regionBound) != regions_.end()) {
    cout << "Error: region bound " << regionBound << " already exists" << endl;
    return;
  }
  regions_[regionBound] = region;
}

void Block::putIO(int area, int inputs, int outputs)
{
  int T = inputs + outputs;
  double g = double(outputs) / T;
  DistributionBucket(area).addData(T, g);
}

Block::DistribBucket& Block::DistributionBucket(int area)
{
  return distribBuckets_[int(log(double(area))
                             / log(double(ArtNetGen::distFactor_)))];
}

void Block::fillBuckets()
{
  if (buckets_.size() != 0)
    return;

  if (pqueue_.size() != 1) {
    cout << "Error: forrest size is not a tree" << endl;
    exit(0);
  }

  pqueue_.begin()->second->FillBucketsWithTree(buckets_);
  buckets_[buckets_.rbegin()->first + 1].push_back(pqueue_.begin()->second);
}

void Block::TreeNode::FillBucketsWithTree(map<int, list<TreeNode*>>& buckets)
{
  buckets[int(log(double(getArea())) / log(1.9))].push_back(this);
}

void Block::CompoundNode::FillBucketsWithTree(
    map<int, list<TreeNode*>>& buckets)
{
  TreeNode::FillBucketsWithTree(buckets);
  left_->FillBucketsWithTree(buckets);
  right_->FillBucketsWithTree(buckets);
}

void Block::writeDat()
{
  auto now = std::chrono::system_clock::now();
  std::time_t cur_time = std::chrono::system_clock::to_time_t(now);

  fillBuckets();

  string fileName = blockName_ + ".dat";
  ofstream outFile(fileName);
  outFile << "Netlist data for circuit " << blockName_
          << "generated by ArtNetGen on " << std::ctime(&cur_time) << endl;
  outFile << "               ----------------------- target value "
             "---------------------        ---------------------- actual value "
             "----------------------\n"
          << endl;
  outFile << "      B        meanT      meanI      meanO     meanG    stddevT  "
             " stddevg        meanT      meanI      meanO     meanG    stddevT "
             "  stddevg\n"
          << endl;

  map<int, Region>::iterator rit = regions_.begin();

  // bucket_ is a map with key as the area and value as a list of nodes
  for (map<int, list<TreeNode*>>::iterator bit = buckets_.begin();
       bit != buckets_.end();
       ++bit) {
    int num = 0;
    double bSum = 0, tSum = 0, iSum = 0, oSum = 0, gSum = 0;

    // iterate through the list of nodes in the bucket
    for (list<TreeNode*>::iterator nit = bit->second.begin();
         nit != bit->second.end();
         ++nit) {
      num++;
      bSum += log(double((*nit)->getArea()));  // log(B)
      tSum += (*nit)->getTerminals();          // T
      oSum += (*nit)->getNumOutputs();         // O
      iSum += (*nit)->getNumInputs();          // I
      gSum += (*nit)->getG();                  // G
    }  // for nit

    double B = exp(bSum / num);
    double T = double(tSum) / num;
    double I = double(iSum) / num;
    double O = double(oSum) / num;
    double g = double(gSum) / num;

    double stdevT = -1, stdevG = -1;

    if (num > 1) {
      double sqDivTSum = 0, sqDivGSum = 0;
      for (list<TreeNode*>::iterator nit = bit->second.begin();
           nit != bit->second.end();
           ++nit) {
        double div = (*nit)->getTerminals() - T;
        sqDivTSum += div * div;
        div = (*nit)->getG() - g;
        sqDivGSum += div * div;
      }  // for nit
      stdevT = sqrt(sqDivTSum / (num - 1));
      stdevG = sqrt(sqDivGSum / (num - 1));
    }

    // calculate target values and print intermediate lines (target only)
    double tarT, tarI, tarO, tarG, tarStdevT, tarStdevG;
    int tarB;

    do {
      getMeanIO(B, tarT, tarI, tarO, tarG, tarStdevT, tarStdevG);

      if (rit == regions_.end())
        break;

      tarB = rit->first;
      if (tarB < B * 1.0001) {
        tarT = rit->second.meanT;
        tarG = rit->second.meanG;
        tarI = tarT * (1 - tarG);
        tarO = tarT * tarG;
        tarStdevT = rit->second.sigmaT;
        tarStdevG = rit->second.sigmaG;
        ++rit;
      }

      if (tarB < B * 0.9999)
        WriteDatLine(outFile,
                     tarB,
                     tarT,
                     tarI,
                     tarO,
                     tarG,
                     tarStdevT,
                     tarStdevG,
                     -1,
                     -1,
                     -1,
                     -1,
                     -1,
                     -1);

    } while (tarB < B * 0.9999);  // to prevent floating point error

    WriteDatLine(outFile,
                 B,
                 tarT,
                 tarI,
                 tarO,
                 tarG,
                 tarStdevT,
                 tarStdevG,
                 T,
                 I,
                 O,
                 g,
                 stdevT,
                 stdevG);

  }  // for bit
}

void Block::WriteDatLine(ofstream& outFile,
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
                         double stdevG)
{
  char buf[1024];
  sprintf(buf, "%10.3f", B);
  outFile << buf;
  WriteDatWord(outFile, tarT);
  WriteDatWord(outFile, tarI);
  WriteDatWord(outFile, tarO);
  WriteDatWord(outFile, tarG, 1);
  WriteDatWord(outFile, tarStdevT);
  WriteDatWord(outFile, tarStdevG, 1);

  outFile << "  ";

  WriteDatWord(outFile, T);
  WriteDatWord(outFile, I);
  WriteDatWord(outFile, O);
  WriteDatWord(outFile, g, 1);
  WriteDatWord(outFile, stdevT);
  WriteDatWord(outFile, stdevG, 1);
  outFile << endl;
}

void Block::WriteDatWord(ofstream& outFile, double value, bool g)
{
  char buf[1024];

  if (g) {
    if (value < 0)
      sprintf(buf, " %9s", "-    ");
    else
      sprintf(buf, " %9.4f", value);
  } else {
    if (value < 0)
      sprintf(buf, " %10s", "-   ");
    else
      sprintf(buf, " %10.3f", value);
  }
  outFile << endl;
}

void Block::writeRegionInfo(ofstream& outFile, string prefix)
{
  outFile << prefix << " Region Info" << endl;
  for (map<int, Region>::iterator rit = regions_.begin(); rit != regions_.end();
       ++rit) {
    outFile << prefix << " B >= " << rit->first << endl;
    outFile << prefix << " meanT: " << rit->second.meanT << endl;
    outFile << prefix << " sigmaT: " << rit->second.sigmaT << endl;
    outFile << prefix << " meanG: " << rit->second.meanG << endl;
    outFile << prefix << " sigmaG: " << rit->second.sigmaG << endl;
    if (rit->first > 1) {
      outFile << prefix << " p: " << rit->second.p << endl;
      outFile << prefix << " q: " << rit->second.q << endl;
      outFile << prefix << " g_factor: " << rit->second.g_factor << endl;
    }
  }
}

}  // namespace ang
