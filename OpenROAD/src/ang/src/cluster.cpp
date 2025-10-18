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

#include <ctime>
#include <deque>
#include <limits>
#include <stack>
#include <unordered_set>

#include "circuit.h"
#include "sta/Liberty.hh"

using std::abs;
using std::cout;
using std::deque;
using std::endl;
using std::list;
using std::log;
using std::make_move_iterator;
using std::map;
using std::max;
using std::min;
using std::move;
using std::multimap;
using std::ofstream;
using std::pair;
using std::stack;
using std::string;
using std::unordered_set;
using std::vector;

using utl::ANG;

using odb::dbMaster;
using sta::LibertyCell;

namespace ang {

int CounterMap::operator[](void* ptr)
{
  pair<map<void*, int>::iterator, bool> mi
      = counterMap_.insert(pair<void* const, int>(ptr, next_));

  if (mi.second)
    ++next_;

  return mi.first->second;
}

Cluster::~Cluster()
{
  for (auto it = instances_.begin(); it != instances_.end(); ++it)
    delete *it;

  for (auto it = inNets_.begin(); it != inNets_.end(); ++it)
    delete *it;

  for (auto it = outNets_.begin(); it != outNets_.end(); ++it)
    delete *it;

  for (auto it = internalNets_.begin(); it != internalNets_.end(); ++it)
    delete *it;
}

Cluster::Cluster(Librarycell* cell) : numBlocks_(1)
{
  area_ = cell->getArea();
  numInputs_ = cell->getNumInputs();
  numOutputs_ = cell->getNumOutputs();

  Instance* inst = new Instance(cell);
  inst->number_ = number_;
  instances_.push_back(inst);

  // set the terminals
  for (int n = 0; n < cell->getNumInputs(); ++n) {
    // sample the path distribution
    // if path distribution is not set, it returns the max path

    // if cell is combinational
    double maxPath;
    double minPath;
    if (!cell->isSequential()) {
      maxPath = ArtNetGen::delay_.SamplePath();
      maxPath -= cell->getDelay();
      minPath = 0;
    } else {
      maxPath = ArtNetGen::delay_.getMacroMaxPath();
      minPath = min(double(ArtNetGen::delay_.getMacroMinPath()), maxPath);
    }

    // create the input net
    // minPath is the required min path length and maxPath is the allowed max
    // path length
    InputNet* inNet = new InputNet(minPath, maxPath);
    inNets_.push_back(inNet);
    // add the terminal to the list of sinks, n is the index of the input pin
    // for the cell
    inNet->sinks_.push_back(Terminal(inst, n));
    inst->inNets_[n] = inNet;
  }  // for n

  for (int n = 0; n < cell->getNumOutputs(); ++n) {
    // if the cell is sequential, the delay is 0
    // if the cell is combinational, the delay is the delay of the cell
    OutputNet* outNet
        = new OutputNet(cell->isSequential() ? 0 : cell->getDelay());
    outNets_.push_back(outNet);
    outNet->source_ = Terminal(inst, n);
    inst->outNets_[n] = outNet;
  }

  // store which output is connected
  if (!ArtNetGen::allowLoops_ && !cell->isSequential()) {
    for (auto iit = inNets_.begin(); iit != inNets_.end(); ++iit) {
      for (auto oit = outNets_.begin(); oit != outNets_.end(); ++oit) {
        (*iit)->connectedOutputs_[*oit] = cell->getDelay();
      }  // for oit
    }  // for iit
  }  // if cell is combinational

  // store the partitioning information
  number_ = ++ArtNetGen::moduleCounter_;
  ArtNetGen::treeData_.push_back(ArtNetGen::PtreeNode(
      number_, -1, -1, area_, numBlocks_, numInputs_, numOutputs_));
}

// Net Generation flow
Cluster::Cluster(Cluster* clustA, Cluster* clustB, Block* block)
{
  int ia = clustA->getNumInputs();
  int ib = clustB->getNumInputs();
  int oa = clustA->getNumOutputs();
  int ob = clustB->getNumOutputs();
  int parIc, parOc;

  area_ = clustA->getArea() + clustB->getArea();
  numBlocks_ = clustA->getNumBlocks() + clustB->getNumBlocks();

  int curFF = clustA->getNumFFs() + clustB->getNumFFs();
  double tarRatio = block->getSeqRatio();

  double factor = log(double(area_))
                  / (double(ArtNetGen::pathLenCutOff_)
                     * log(double(ArtNetGen::circuit_->getArea())) / 100);
  double delayScaleFactor = min(1.0, factor);

  int remain = block->seqForrest_.size();

  int tarFF;

  if (delayScaleFactor == 1)
    tarFF = round(tarRatio * numBlocks_);
  else
    tarFF = floor(tarRatio * numBlocks_);

  reqFFNum_ = max(tarFF - curFF, 0);

  block->getIO(area_ + reqFFNum_, parIc, parOc);

  int ic = parIc;
  int oc = parOc;

  int minInputs = min(ia + ib, ArtNetGen::minInputs_);
  int minOutputs = min(oa + ob, ArtNetGen::minOutputs_);

  // to make sure local rent is not negative
  if (oc < minOutputs) {
    oc = minOutputs;
  }
  // to make sure local rent is not negative
  if (ic < minInputs) {
    ic = minInputs;
  }

  // to prevent local rent > 1
  if (oc > oa + ob) {
    oc = oa + ob;
  }

  // to prevent local rent > 1
  if (ic > ia + ib) {
    ic = ia + ib;
  }

  // calculate the number of internal connections and the number of external
  // connections
  int si;  // internal connections
  int se;  // external connections

  // internal : connections inside the clusters
  // external : connections outside of the block

  // if the number of internal connections is greater than the number of
  // external connections
  if (oa + ob - oc > ia + ib - ic) {
    si = int(double(oa + ob - oc + ia + ib - ic) / 2.0 + 0.5);
    if (ia + ib - si < minInputs) {
      si = ia + ib - minInputs;
    }
  } else {
    si = oa + ob - oc;
  }

  if (si < 0) {
    cout << "Error: internal connections is negative" << endl;
    exit(0);
  }

  se = (ia + ib - ic) - si;

  if (se < 0) {
    // cout << "External connections is negative" << endl;
    se = 0;
  }

  // make sure at least one external connection is made
  if (se + si == 0)
    se = 1;

  // make connections by combining nets
  //   original gnl's schemes:
  //     * make external connections from A to B and from B to A in a random
  //     order
  //     * convert some of these connections to internal connections
  //     * make other external connections by combining inputs

  if (ArtNetGen::verbose_) {
    cout << "Combining clusters " << endl;
    cout << "Cluster A: " << clustA->getNumBlocks()
         << "Cluster B: " << clustB->getNumBlocks() << endl;
    for (auto it = clustA->instances_.begin(); it != clustA->instances_.end();
         ++it) {
      cout << "clustA instance name : " << (*it)->cell_->getName() << " "
           << (*it)->number_ << endl;
    }
    for (auto it = clustB->instances_.begin(); it != clustB->instances_.end();
         ++it) {
      cout << "clustB instance name : " << (*it)->cell_->getName() << " "
           << (*it)->number_ << endl;
    }
    cout << "T: " << (ia + oa) << " " << (ib + ob) << " " << (ic + oc) << endl;
    cout << "I: " << ia << " " << ib << " " << ic << " (" << parIc << ") "
         << endl;
    cout << "O: " << oa << " " << ob << " " << oc << " (" << parOc << ") "
         << endl;
    cout << "se: " << se << " si: " << si << endl;
  }

  int external = 0;
  int externalAtoB = 0;

  // make external connections from A to B and from B to A
  if (ArtNetGen::verbose_) {
    cout << "Phase 1 : Make external connections" << endl;
  }

  auto iait = clustA->inNets_.begin();
  auto oait = clustA->outNets_.begin();
  auto ibit = clustB->inNets_.begin();
  auto obit = clustB->outNets_.begin();

  bool AtoB, BtoA, firstConnection = true;

  // while the number of external connections is less than
  // the sum of the number of internal connections and the number of external
  // connections

  // matchNets(clustA->outNets_, clustB->inNets_);
  // matchNets(clustB->outNets_, clustA->inNets_);

  while (external < se + si) {
    list<OutputNet*>::iterator toSplice;

    // check if the connection is possible
    // AtoB means the connection from A to B, and vice versa
    AtoB = (oait != clustA->outNets_.end() && ibit != clustB->inNets_.end());
    BtoA = (obit != clustB->outNets_.end() && iait != clustA->inNets_.end());

    bool isA;

    if (AtoB && BtoA) {
      bool condA = ((*oait)->maxLen_ <= (*ibit)->maxPath_);
      bool condB = ((*obit)->maxLen_ <= (*iait)->maxPath_);

      if (condA && condB) {
        if ((*oait)->maxLen_ >= (*obit)->maxLen_) {
          isA = true;
        } else {
          isA = false;
        }
      } else if (!condA && condB) {
        isA = false;
      } else if (condA && !condB) {
        isA = true;
      }
    } else {
      isA = randomInt(2);
    }

    // impossible to make the connection
    if (!AtoB && !BtoA) {
      if (ArtNetGen::verbose_)
        cout << "Impossible to make the connection" << endl;
      break;
    }

    // make the connection from A to B --> combine the output of A to the input
    // of B
    // if (!BtoA || (AtoB && (ang::randomInt(2) || firstConnection))) {
    if (!BtoA || (AtoB && (isA || firstConnection))) {
      if ((*oait)->connect(
              *ibit, clustA, clustB, this, delayScaleFactor, block)) {
        if (ArtNetGen::verbose_)
          cout << "here A to B" << endl;
        toSplice = oait++;
        outNets_.splice(outNets_.begin(), clustA->outNets_, toSplice);

        clustB->inNets_.erase(ibit);
        ibit = clustB->inNets_.begin();
        externalAtoB++;
        external++;
      } else {
        if (++oait == clustA->outNets_.end()) {
          oait = clustA->outNets_.begin();
          ++ibit;
        }
        /*
        if(++ibit == clustB->inNets_.end()) {
            ibit = clustB->inNets_.begin();
            ++oait;
        }*/

      }  // if *oait->connect
    } else {
      // make the connection from B to A --> combine the output of B to the
      // input of A
      if ((*obit)->connect(
              *iait, clustB, clustA, this, delayScaleFactor, block)) {
        if (ArtNetGen::verbose_)
          cout << "here B to A" << endl;
        toSplice = obit++;

        outNets_.splice(outNets_.begin(), clustB->outNets_, toSplice);
        clustA->inNets_.erase(iait);
        iait = clustA->inNets_.begin();
        external++;
      } else {
        // if (++iait == clustA->inNets_.end()) {
        //     iait = clustA->inNets_.begin();
        //     ++obit;
        // }
        if (++obit == clustB->outNets_.end()) {
          obit = clustB->outNets_.begin();
          ++iait;
        }
      }  // if *obit->connect
    }
    firstConnection = 0;
  }  // while external < se + si

  if (ArtNetGen::verbose_) {
    // cout << "External connections from A to B: " << externalAtoB << endl;
    // cout << "External connections from B to A: " << external - externalAtoB
    // << endl; cout << "Total external connections: " << external << endl;
    cout << "Phase 2 : Converting external nets to internal nets" << endl;
  }

  // convert some external connections to internal connections
  // randomly select some external connections and convert them to internal
  // connections
  // ang::randomizeList(outNets_);
  sortOutNet();

  int internal = 0;
  list<OutputNet*>::iterator li = outNets_.begin();
  // while there are no more external connections (external == 0)
  // or until the required number of internal connections is reached (si == 0)
  while (si > 0 && external > 0) {
    if (delayScaleFactor == 1)
      (*li)->MakeInternal(clustA, clustB, this);
    li++;
    si--;
    external--;
    internal++;
  }

  se -= external;

  internalNets_.splice(internalNets_.end(), outNets_, outNets_.begin(), li);

  if (ArtNetGen::verbose_) {
    // cout << "Internal connections: " << internal << endl;
    // cout << "se: " << se << " si: " << si << endl;
    cout << "Phase 3 : Making external connections by combining inputs" << endl;
  }

  // loop over the inputs of A and B and combine them
  int inputCombinations = 0;

  while (se > 0 && !clustA->inNets_.empty() && !clustB->inNets_.empty()) {
    clustA->inNets_.front()->connect(clustB->inNets_.front());

    inNets_.splice(inNets_.begin(), clustA->inNets_, clustA->inNets_.begin());
    clustB->inNets_.pop_front();

    se--;
    external++;
    inputCombinations++;
  }

  if (ArtNetGen::verbose_) {
    // cout << "Input combinations: " << inputCombinations << endl;
    // cout << "se = " << se << endl;
  }

  // merge remaining instances, input nets, output nets, and internal nets
  Merge(clustA);
  Merge(clustB);

  numInputs_
      += clustA->getNumInputs() + clustB->getNumInputs() - external - internal;
  numOutputs_ = clustA->getNumOutputs() + clustB->getNumOutputs() - internal;

  // make connections from 'this' cluster to itself
  if (ArtNetGen::localConnect_
      && (log(double(area_)) / log(double(block->getArea())) * 100
          > ArtNetGen::localCutOff_)) {
    if (ArtNetGen::verbose_) {
      cout << "Local connections" << endl;
    }
    // ang::randomizeList(inNets_);
    // ang::randomizeList(outNets_);
    sortInNet();
    sortOutNet();

    external = 0;

    list<OutputNet*> newOutputs;
    list<InputNet*>::iterator iit = inNets_.begin();
    list<OutputNet*>::iterator oit = outNets_.begin();

    if (ArtNetGen::verbose_) {
      // cout << "Local connections" << endl;
      cout << "Phase 4 : Output connections" << endl;
    }

    while ((external < se + si) && (iit != inNets_.end())
           && (oit != outNets_.end())) {
      list<OutputNet*>::iterator toSplice;
      if ((*oit)->connect(*iit, this, nullptr, this, delayScaleFactor, block)) {
        toSplice = oit++;
        newOutputs.splice(newOutputs.begin(), outNets_, toSplice);
        inNets_.erase(iit);
        iit = inNets_.begin();
        external++;
      } else {
        if (++oit == outNets_.end()) {
          oit = outNets_.begin();
          ++iit;
        }
      }  // if *oit->connect
    }  // while external < se + si

    if (ArtNetGen::verbose_) {
      // cout << "External connections: " << external << endl;
      cout << "Phase 5 : Converting external nets to internal nets" << endl;
    }

    // convert some external connections to internal connections
    // ang::randomizeList(newOutputs);
    internal = 0;

    // sortOutList(newOutputs);
    list<OutputNet*>::iterator noit = newOutputs.begin();

    while ((si > 0) && (external > 0)) {
      if (delayScaleFactor == 1)
        (*noit)->MakeInternal(this, nullptr, nullptr);
      noit++;
      si--;
      external--;
      internal++;
    }

    se -= external;
    internalNets_.splice(
        internalNets_.end(), newOutputs, newOutputs.begin(), noit);

    if (ArtNetGen::verbose_) {
      // cout << "Internal connections: " << internal << endl;
      // cout << "se = " << se << " si = " << si <<  endl;
      cout << "Phase 6 : Internal connections" << endl;
    }

    // combine the remaining inputs
    inputCombinations = 0;

    while ((se > 0) && (inNets_.size() >= 2)) {
      // ang::randomizeList(inNets_);
      revsortInNet();
      InputNet* inNet = inNets_.front();
      inNets_.pop_front();
      inNets_.front()->connect(inNet);
      se--;
      external++;
      inputCombinations++;
    }

    if (ArtNetGen::verbose_) {
      // cout << "Input combinations: " << inputCombinations << endl;
      // cout << "se = " << se << endl;
    }

    outNets_.splice(outNets_.end(), newOutputs);
    numInputs_ -= external + internal;
    numOutputs_ -= internal;

  }  // if !ArtNetGen::localConnect_

  // store final number of inputs and outputs
  block->putIO(area_, numInputs_, numOutputs_);

  if (numInputs_ != inNets_.size()) {
    cout << "Error: number of inputs is not equal to the number of input nets"
         << endl;
    cout << "numInputs_: " << numInputs_
         << " inNets_.size(): " << inNets_.size() << endl;
    exit(0);
  }
  if (numOutputs_ != outNets_.size()) {
    cout << "Error: number of outputs is not equal to the number of output nets"
         << endl;
    cout << "numOutputs_: " << numOutputs_
         << " outNets_.size(): " << outNets_.size() << endl;
    exit(0);
  }

  if (delayScaleFactor < 1)
    swapOutput();

  sortInNet();
  sortOutNet();

  // store partitioning information
  number_ = ++ArtNetGen::moduleCounter_;
  ArtNetGen::treeData_.push_back(ArtNetGen::PtreeNode(number_,
                                                      clustA->getNumber(),
                                                      clustB->getNumber(),
                                                      area_,
                                                      numBlocks_,
                                                      numInputs_,
                                                      numOutputs_));
  delete clustA;
  delete clustB;
}

void Cluster::swapOutput()
{
  sortInternalNet();
  sortOutNet();

  auto oit = outNets_.begin();
  auto init = internalNets_.begin();

  while (oit != outNets_.end() && init != internalNets_.end()) {
    OutputNet* outNet = *oit;
    OutputNet* internalNet = *init;

    if (outNet == nullptr || internalNet == nullptr) {
      std::cerr << "Null pointer encountered!" << std::endl;
      return;
    }

    if (internalNet->isDone_) {
      ++init;
    } else {
      if (outNet->maxLen_ > internalNet->maxLen_ && outNet->sinks_.size() > 1
          && internalNet->sinks_.size() > 1) {
        oit = outNets_.erase(oit);
        internalNets_.push_back(outNet);

        init = internalNets_.erase(init);
        outNets_.push_back(internalNet);
      } else {
        ++oit;
      }
    }
  }
}

// (1) before insertFlop: inNet_ --> (inst) --> outNets_
// (2) after insertFlop:  inNet_ --> (inst) --> (flop) --> outNets_
void Cluster::insertFlop(Instance* inst, Block* block, int outTerm)
{
  auto it = (block->getSeqForrest().begin());

  Instance* flop = new Instance((*it).second);
  block->seqForrest_.erase(it);

  flop->number_ = number_;
  instances_.push_back(flop);
  numBlocks_++;
  area_ += flop->getArea();

  // output of flop
  OutputNet* ff_outNet = new OutputNet(0);
  flop->outNets_[0] = ff_outNet;
  ff_outNet->source_ = Terminal(flop, 0);

  for (auto outNet : inst->outNets_) {
    for (auto it = outNet->sinks_.begin(); it != outNet->sinks_.end(); ++it) {
      if (it->first->minLevel_ > 2 && it->first->maxLevel_ - inst->maxLevel_ < 3
          && !it->first->isSequential()) {
        it->first->inNets_[it->second] = ff_outNet;
        ff_outNet->sinks_.push_back(*it);
        outNet->sinks_.erase(it);
      }
    }
  }

  flop->inNets_[0] = inst->outNets_[outTerm];
  inst->outNets_[outTerm]->sinks_.push_back(Terminal(flop, 0));

  if (!ArtNetGen::allowLoops_) {
    for (auto sink : flop->outNets_[outTerm]->sinks_) {
      Instance* sink_inst = sink.first;
      for (auto inNet : sink_inst->inNets_) {
        for (auto outNet : sink_inst->outNets_) {
          InputNet* iNet = dynamic_cast<InputNet*>(inNet);
          iNet->connectedOutputs_[outNet] = sink_inst->cell_->getDelay();
        }  // for oit
      }  // for iit
    }
  }
}

void Cluster::sortInternalNet()
{
  internalNets_.sort([](const OutputNet* a, const OutputNet* b) {
    return a->maxLen_ < b->maxLen_;
  });
}

void Cluster::sortOutNet()
{
  outNets_.sort([](const OutputNet* a, const OutputNet* b) {
    return a->maxLen_ > b->maxLen_;
  });
}

void Cluster::sortOutList(list<OutputNet*>& newOutputs)
{
  newOutputs.sort([](const OutputNet* a, const OutputNet* b) {
    if (a->maxLen_ != b->maxLen_)
      return a->maxLen_ > b->maxLen_;
    return a->sinks_.size() > b->sinks_.size();
  });
}

void Cluster::sortInNet()
{
  inNets_.sort([](const InputNet* a, const InputNet* b) {
    if (a->maxPath_ != b->maxPath_)
      return a->maxPath_ > b->maxPath_;
    return a->sinks_.size() < b->sinks_.size();
  });
}

void Cluster::revsortInNet()
{
  inNets_.sort([](const InputNet* a, const InputNet* b) {
    return a->maxPath_ < b->maxPath_;
  });
}

void Cluster::moveNtoFront(int n)
{
  if (n <= 0)
    return;
  else if (n >= inNets_.size())
    inNets_.reverse();

  auto it = inNets_.end();
  std::advance(it, -n);

  list<InputNet*> temp(it, inNets_.end());

  temp.reverse();

  inNets_.erase(it, inNets_.end());

  inNets_.splice(inNets_.begin(), temp);
}

void Cluster::moveNtoBack(int n)
{
  if (n <= 0)
    return;
  else if (n >= inNets_.size())
    inNets_.reverse();

  auto it = inNets_.begin();
  std::advance(it, n);

  list<InputNet*> temp(inNets_.begin(), it);

  temp.reverse();

  inNets_.erase(inNets_.begin(), it);

  inNets_.splice(inNets_.end(), temp);
}

void Cluster::matchNets(list<OutputNet*>& outNets, list<InputNet*>& inNets)
{
  for (auto outNet : outNets)
    cout << "maxLen_: " << outNet->maxLen_ << endl;

  for (auto inNet : inNets)
    cout << "before maxPath_: " << inNet->maxPath_ << endl;

  inNets.sort([&outNets](const auto& a, const auto& b) {
    double minDiffA = std::numeric_limits<double>::max();
    double minDiffB = std::numeric_limits<double>::max();
    bool aSatisfies = false;
    bool bSatisfies = false;

    for (const auto& outNet : outNets) {
      if (outNet->maxLen_ < a->maxPath_) {
        aSatisfies = true;
        minDiffA = std::max(minDiffA, std::abs(a->maxPath_ - outNet->maxLen_));
      }
    }

    for (const auto& outNet : outNets) {
      if (outNet->maxLen_ < b->maxPath_) {
        bSatisfies = true;
        minDiffB = std::max(minDiffB, std::abs(b->maxPath_ - outNet->maxLen_));
      }
    }

    if (aSatisfies && bSatisfies) {
      return minDiffA < minDiffB;
    }

    if (aSatisfies)
      return true;
    if (bSatisfies)
      return false;
    return false;
  });

  for (auto inNet : inNets)
    cout << "after maxPath_: " << inNet->maxPath_ << endl;
  cout << endl;
}

void Cluster::Net::connect(InputNet* inNet)
{
  // vector<Terminal> sinks_;
  // typedef std::pair<Instance*, int> Terminal;

  for (auto it = inNet->sinks_.begin(); it != inNet->sinks_.end(); ++it) {
    it->first->inNets_[it->second] = this;
    // it->first is the instance and it->second is the index of the input pin
    // add the terminal to the list of sinks_
  }
  sinks_.splice(sinks_.end(), inNet->sinks_);
}

bool Cluster::InputNet::connect(InputNet* inNet)
{
  // connect the input nets
  Net::connect(inNet);

  // update connectedOutputs_
  if (!ArtNetGen::allowLoops_) {
    for (auto cit = inNet->connectedOutputs_.begin();
         cit != inNet->connectedOutputs_.end();
         ++cit) {
      connectedOutputs_[cit->first]
          = max(connectedOutputs_[cit->first], cit->second);
    }
  }  // if !ArtNetGen::allowLoops

  // minPath_ is the required min path length
  // maxPath_ is the allowed max path length
  minPath_ = max(minPath_, inNet->minPath_);
  maxPath_ = min(maxPath_, inNet->maxPath_);

  delete inNet;
  return 1;
}

bool Cluster::OutputNet::connect(InputNet* inNet,
                                 Cluster* clustA,
                                 Cluster* clustB,
                                 Cluster* clustC,
                                 double delayScaleFactor,
                                 Block* block)
{
  if (source_.first == nullptr) {
    std::cerr << "Error: source_.first is null." << std::endl;
    exit(1);
  }

  if (block == nullptr) {
    std::cerr << "Error: block is null." << std::endl;
    exit(1);
  }

  for (Cluster* clust = clustA; clust; clust = (clust == clustA)   ? clustB
                                               : (clust == clustB) ? clustC
                                                                   : nullptr) {
    if (clust == nullptr) {
      std::cerr << "Error: clust is null." << std::endl;
      exit(1);
    }

    for (auto iit = clust->inNets_.begin(); iit != clust->inNets_.end();
         ++iit) {
      if (*iit == nullptr) {
        std::cerr << "Error: *iit is null." << std::endl;
        exit(1);
      }
    }
  }

  // connect the output nets
  bool allowed = 1;
  // check if connection is allowed -- i.e. no loops are begin generated
  // if the output net is connected to the input net, then the connection is not
  // allowed (comb loop)
  if (!ArtNetGen::allowLoops_
      && inNet->connectedOutputs_.find(this)
             != inNet->connectedOutputs_.end()) {
    allowed = 0;
  }

  if (!ArtNetGen::allowLoops_) {
    // too long path
    if (maxLen_ > inNet->maxPath_) {
      // if (maxLen_ > inNet->maxPath_ || maxLen_ > maxCrit) {
      allowed = 0;
    }

    // output net's maxLen_ should be larger than the minPath_ of the input net
    // if (maxLen_ < delayScaleFactor * inNet->minPath_) {
    if (maxLen_ < inNet->minPath_) {
      cout << "too short " << maxLen_ << " "
           << delayScaleFactor * inNet->minPath_ << " " << inNet->minPath_
           << endl;
      return 0;
    }
  }

  bool isAdded = false;

  if (!allowed && !isAdded) {
    // addFlop to break the loop and long path
    if (ArtNetGen::insertFlop_
        && log(double(clustC->numBlocks_)) / log(double(block->getNumBlocks()))
                   * 100
               > ArtNetGen::flopCutOff_
        && ArtNetGen::flopInsertProb_ >= ang::uniformDist()) {
      addFlop(clustA, clustB, clustC, block);
      clustC->reqFFNum_--;
      isAdded = true;
    } else {
      return 0;
    }
  }

  if ((!isAdded) && (clustC->reqFFNum_ > 0)
      && (maxLen_ >= ArtNetGen::delay_.getMinPath())
      && (inNet->maxPath_ <= ArtNetGen::delay_.SamplePath())) {
    addFlop(clustA, clustB, clustC, block);
    clustC->reqFFNum_--;
    isAdded = true;
  }

  // inNet's sink --> outNet's sink
  Net::connect(inNet);

  // compare the delay_ of the output net in connectedOutputs_
  // and the sum of the delay_ of the output net and the delay_ of the 'this'
  // output net
  for (auto cit = inNet->connectedOutputs_.begin();
       cit != inNet->connectedOutputs_.end();
       ++cit) {
    cit->first->maxLen_ = max(cit->first->maxLen_, cit->second + maxLen_);
  }

  // update the connected outputs
  if (!ArtNetGen::allowLoops_) {
    // loop over all the clusters' inputs
    for (Cluster* clust = clustA; clust; clust = (clust == clustA)   ? clustB
                                                 : (clust == clustB) ? clustC
                                                                     : 0) {
      for (auto iit = clust->inNets_.begin(); iit != clust->inNets_.end();
           ++iit) {
        // find "this" output belongs to their connectedOutputs
        map<OutputNet*, double>::iterator cit
            = (*iit)->connectedOutputs_.find(this);

        // if "this" output belongs to thier connectedOutput --> cit.first ==
        // this
        if (cit != (*iit)->connectedOutputs_.end()) {
          for (auto it = inNet->connectedOutputs_.begin();
               it != inNet->connectedOutputs_.end();
               ++it) {
            (*iit)->connectedOutputs_[it->first] = max(
                (*iit)->connectedOutputs_[it->first], cit->second + it->second);
          }  // for it

          // allowedMaxPath - connectedOutput's delay
          // double len = inNet->maxPath_ - cit->first->maxLen_;
          double len = inNet->maxPath_ - cit->second;

          if (len < 0) {
            cout << "Error: allowed max length is negative" << endl;
            exit(0);
          }  // if len < 0

          if (len < (*iit)->maxPath_) {
            (*iit)->maxPath_ = len;
          }  // if len < (*iit)->maxPath_

          len = max(0.0, inNet->minPath_ - cit->second);
          if (len > (*iit)->minPath_) {
            (*iit)->minPath_ = len;
          }  // if len > (*iit)->minPath_
        }  // if cit
      }  // for iit
    }  // for mod
  }  // if !ArtNetGen::allowLoops

  delete inNet;
  return 1;
}

// (1) before addFlop: source_ --> (this) --> sinks_
// (2) after addFlop: source_ --> (outNet) --> | flop | --> (this)
void Cluster::OutputNet::addFlop(Cluster* clustA,
                                 Cluster* clustB,
                                 Cluster* clustC,
                                 Block* block)
{
  if (ArtNetGen::flopCell_ == nullptr) {
    cout << "Error: flop cell is not set" << endl;
    exit(0);
  }
  Instance* inst;

  if (block->seqForrest_.size() > 0) {
    inst = new Instance(block->seqForrest_.begin()->second);
    block->seqForrest_.erase(block->seqForrest_.begin());
  } else {
    inst = new Instance(ArtNetGen::flopCell_);
  }

  int numInpins = inst->cell_->getNumInputs();

  if (numInpins > 1) {
    for (int n = 1; n < numInpins; ++n) {
      double maxPath = ArtNetGen::delay_.SamplePath();
      double minPath = 0;

      // create the input net
      // minPath is the required min path length and maxPath is the allowed max
      // path length
      InputNet* inNet = new InputNet(minPath, maxPath);
      clustC->inNets_.push_back(inNet);
      clustC->addNumInputs(1);
      // add the terminal to the list of sinks, n is the index of the input pin
      // for the cell
      inNet->sinks_.push_back(Terminal(inst, n));
      inst->inNets_[n] = inNet;
    }
  }
   
  clustC->FFs_.push_back(inst);
  clustC->instances_.push_back(inst);
  clustC->numBlocks_++;
  clustC->area_ += ArtNetGen::flopCell_->getArea();

  // connect the flop to the output net
  OutputNet* outNet = new OutputNet(maxLen_);
  source_.first->outNets_[source_.second] = outNet;
  outNet->source_ = source_;

  for (auto sit = sinks_.begin(); sit != sinks_.end(); ++sit) {
    sit->first->inNets_[sit->second] = outNet;
  }
  outNet->sinks_.splice(outNet->sinks_.end(), sinks_);

  inst->inNets_[0] = outNet;
  outNet->sinks_.push_back(Terminal(inst, 0));

  double thisMax = maxLen_;

  inst->outNets_[0] = this;
  source_.first = inst;
  source_.second = 0;
  maxLen_ = 0;

  double maxPath = 0.0;

  // loop over all the inputs and erase this output net from their connected
  // outputs mod --> (1) clustA, (2) clustB, (3) clustC
  for (Cluster* mod = clustA; mod; mod = (mod == clustA)   ? clustB
                                         : (mod == clustB) ? clustC
                                                           : nullptr) {
    for (auto iit = mod->inNets_.begin(); iit != mod->inNets_.end(); ++iit) {
      (*iit)->connectedOutputs_.erase(this);
    }  // for iit
  }  // for mod
}

void Cluster::OutputNet::MakeInternal(Cluster* clustA,
                                      Cluster* clustB,
                                      Cluster* clustC)
{
  if (!ArtNetGen::allowLoops_) {
    for (Cluster* mod = clustA; mod; mod = (mod == clustA)   ? clustB
                                           : (mod == clustB) ? clustC
                                                             : 0) {
      for (auto iit = mod->inNets_.begin(); iit != mod->inNets_.end(); ++iit) {
        (*iit)->connectedOutputs_.erase(this);
      }  // for iit
    }
  }
}

void Cluster::Merge(Cluster* cluster)
{
  // merge the instances
  instances_.splice(instances_.end(), cluster->instances_);
  // merge the internal nets
  internalNets_.splice(internalNets_.end(), cluster->internalNets_);
  // merge the input nets
  inNets_.splice(inNets_.end(), cluster->inNets_);
  // merge the output nets
  outNets_.splice(outNets_.end(), cluster->outNets_);
  // merge the flip-flops
  FFs_.splice(FFs_.end(), cluster->FFs_);
  // merge the combinational cells
  Combis_.splice(Combis_.end(), cluster->Combis_);
}

void Cluster::levelize()
{
  if (ArtNetGen::verbose_)
    cout << "here in levelize" << endl;

  unlevelizeCombis();
  checkSequential();
  checkStartPoints();
  levelizeCombis();
  checkLevelConsistency();
}

void Cluster::unlevelizeCombis()
{
  if (ArtNetGen::verbose_)
    cout << "here unlevelizeCombis" << endl;

  for (auto inst : instances_) {
    if (!inst->isSequential()) {
      inst->isVisited_ = false;
      inst->minLevel_ = INT_MAX;
      inst->maxLevel_ = -1;
    }
  }
}

void Cluster::checkSequential()
{
  if (ArtNetGen::verbose_)
    cout << "here checkSequential" << endl;

  for (auto inst : instances_) {
    if (inst->isSequential()) {
      inst->minLevel_ = 0;
      inst->maxLevel_ = 0;
      inst->isVisited_ = true;
    }
  }
}

void Cluster::checkStartPoints()
{
  if (ArtNetGen::verbose_)
    cout << "here in checkStartPoints" << endl;

  for (auto inNet : inNets_) {
    for (auto sink : inNet->sinks_) {
      auto it = find(startPoints_.begin(), startPoints_.end(), sink.first);
      if (it == startPoints_.end()) {
        startPoints_.push_back(sink.first);
      }
    }
  }

  for (auto ff : FFs_) {
    for (auto outNet : ff->outNets_) {
      for (auto sink : outNet->sinks_) {
        auto it = find(startPoints_.begin(), startPoints_.end(), sink.first);
        if (it == startPoints_.end()) {
          startPoints_.push_back(sink.first);
        }
      }
    }
  }
}

void Cluster::levelizeCombis()
{
  deque<Instance*> candiInsts;

  for (auto inst : startPoints_) {
    if (inst->isSequential()) {
      if (!(inst->isVisited_ && inst->minLevel_ == 0 && inst->maxLevel_ == 0)) {
        cout << "Error: " << inst->cell_->getName()
             << " is sequential but not labeled with level 0" << endl;
        exit(0);
      }
    } else {
      candiInsts.push_back(inst);

      while (!candiInsts.empty()) {
        Instance* candiInst = candiInsts.front();
        candiInsts.pop_front();

        if (!candiInst->isVisited_) {
          calcCombiLevel(candiInst, candiInsts, false);
        }
      }
    }
  }
}

void Cluster::calcCombiLevel(Instance* inst,
                             deque<Instance*>& candiInsts,
                             bool isIncremental)
{
  int max_level = 0;
  int min_level = 0;

  getUnvisitedSinks(inst, candiInsts, isIncremental);

  // find max delay of fanin nodes
  if (inst->inNets_.size() == 0) {
    cout << "Error: " << inst->cell_->getName() << " has no fanin nodes"
         << endl;
    exit(0);
  }

  for (auto inNet : inst->inNets_) {
    OutputNet* iNet = dynamic_cast<OutputNet*>(inNet);

    if (iNet) {
      auto faninCell = iNet->source_.first;
      if (faninCell != inst && faninCell != nullptr) {
        // if fanin cell is combi logic
        if (!faninCell->isSequential()) {
          if (!faninCell->isVisited_)
            calcCombiLevel(faninCell, candiInsts, isIncremental);

          if (!(faninCell->isVisited_ && faninCell->maxLevel_ >= 0
                && faninCell->minLevel_ < INT_MAX)) {
            cout << "Error: fanin cell should not be marked" << endl;
            exit(0);
          }
          max_level = max(max_level, faninCell->maxLevel_);
          min_level = min(min_level, faninCell->minLevel_);
        }
      }
    }
  }

  inst->maxLevel_ = max_level + 1;
  inst->minLevel_ = min_level + 1;
  inst->isVisited_ = true;
  lvedInsts[max_level + 1].push_back(inst);
  max_level_ = max(max_level_, max_level + 1);
  min_level_ = min(min_level_, min_level + 1);
}

void Cluster::getUnvisitedSinks(Instance* inst,
                                deque<Instance*>& candiInsts,
                                bool isIncremental)
{
  for (auto outNet : inst->outNets_) {
    for (auto sink : outNet->sinks_) {
      if (sink.first->isVisited_ == false && !sink.first->isSequential()) {
        if (sink.first == inst) {
          continue;
        }
        if (sink.first == nullptr) {
          continue;
        }
        candiInsts.push_back(sink.first);
      } else {
        if (!isIncremental) {
          if (!(!sink.first->isVisited_ || sink.first->isSequential())) {
            cout << "Error: " << sink.first->cell_->getName() << " is marked ("
                 << sink.first->isVisited_ << ")" << " or sequential ("
                 << sink.first->cell_->isSequential() << ")" << endl;
            exit(0);
          }
        }
      }
    }
  }
}

// unlevelize all the outcone nodes from the given instance
void Cluster::unlevelizeOutcones(Instance* inst)
{
  unordered_set<Instance*> unlevelized;
  stack<Instance*> stack;  // stack for unlevelizing

  while (!stack.empty()) {
    Instance* inst = stack.top();
    stack.pop();

    if (unlevelized.find(inst) == unlevelized.end()) {
      if (!inst->isSequential()) {
        inst->maxLevel_ = INT_MIN;
        inst->minLevel_ = INT_MAX;
        inst->isVisited_ = false;
        unlevelized.insert(inst);

        for (auto outNet : inst->outNets_) {
          for (auto sink : outNet->sinks_) {
            if (!(sink.first->isSequential())) {
              stack.push(sink.first);
            }
          }
        }
      }
    }
  }
}

// re-levelize all the outcone nodes from the given instance
void Cluster::incrementalLevelize(Instance* inst)
{
  unlevelizeOutcones(inst);

  deque<Instance*> candiInsts;

  for (auto outNet : inst->outNets_) {
    for (auto sink : outNet->sinks_) {
      auto it = find(startPoints_.begin(), startPoints_.end(), sink.first);
      if (it == startPoints_.end()) {
        startPoints_.push_back(sink.first);
      }
    }
  }

  for (auto inst : startPoints_) {
    if (inst->isSequential()) {
      if (!(inst->isVisited_ && inst->maxLevel_ == 0)) {
        cout << "Error: " << inst->cell_->getName()
             << " is sequential but not labeled with level 0" << endl;
        exit(0);
      }
    } else {
      candiInsts.push_back(inst);
      while (!candiInsts.empty()) {
        Instance* candiInst = candiInsts.front();
        candiInsts.pop_front();
        calcCombiLevel(candiInst, candiInsts, true);
      }
    }
  }
  // checkLevelConsistency();
}

void Cluster::checkLevelConsistency()
{
  int undone = 0;
  for (auto inst : instances_) {
    if (inst->isSequential()) {
      if (inst->maxLevel_ != 0) {
        cout << "Error: " << inst->cell_->getName()
             << " is sequential but not labeled with level 0" << endl;
        exit(0);
      }
    } else {
      if (!inst->isVisited_) {
        int count = 0;
        for (auto inNet : inNets_) {
          OutputNet* oNet = dynamic_cast<OutputNet*>(inNet);
          if (oNet) {
            // cout << oNet->source_.first->cell_->getName() << endl;
            // cout << oNet->source_.first->maxLevel_ << endl;
            count++;
          }
        }

        if (count == 0) {
          // instance connected to the input
          inst->maxLevel_ = 1;
          inst->isVisited_ = true;
        } else {
          cout << "Error: " << inst->cell_->getName()
               << " is not labeled with level (" << inst->maxLevel_ << ")"
               << endl;
          undone++;
        }
      }
    }

    // cout << "inst name : " << inst->cell_->getName() << " level : " <<
    // inst->maxLevel_ << endl;
  }

  if (undone > 0) {
    cout << "Error: " << undone << " instances are not labeled with level"
         << endl;
    exit(0);
  }
}

void Cluster::printDepth()
{
  int sumDepth = 0;
  cout << "FF level: ";

  map<int, int> depthDist;
  int maxDepth = 0;

  for (auto FF : FFs_) {
    OutputNet* outNet = dynamic_cast<OutputNet*>(FF->inNets_[0]);

    int depth = outNet->source_.first->maxLevel_;

    depthDist[depth]++;

    maxDepth = max(maxDepth, depth);
  }

  for (int i = 0; i <= maxDepth; ++i) {
    if (depthDist.find(i) == depthDist.end()) {
      depthDist[i] = 0;
    }
  }

  for (int i = 0; i <= maxDepth; ++i) {
    cout << i << " " << depthDist[i] << endl;
  }
}

void Cluster::checkConsistency()
{
  // check the consistency of the cluster
  if (numBlocks_ != instances_.size()) {
    cout << "Error: number of blocks is not equal to the number of instances"
         << endl;
    exit(0);
  }
  if (numInputs_ != inNets_.size()) {
    cout << "Error: number of inputs is not equal to the number of input nets"
         << endl;
    cout << "numInputs_: " << numInputs_
         << " inNets_.size(): " << inNets_.size() << endl;
    exit(0);
  }
  if (numOutputs_ != outNets_.size()) {
    cout << "Error: number of outputs is not equal to the number of output nets"
         << endl;
    exit(0);
  }
  int realArea = 0;
  // loop over the instances
  for (auto inst = instances_.begin(); inst != instances_.end(); ++inst) {
    (*inst)->checkConsistency();
    realArea += (*inst)->cell_->getArea();
  }
  if (realArea != area_) {
    cout << "Error: real area is not equal to the area" << endl;
    exit(0);
  }
  // loop over the input nets
  for (auto iit = inNets_.begin(); iit != inNets_.end(); ++iit) {
    (*iit)->checkConsistency();
  }
  // loop over the output nets
  for (auto oit = outNets_.begin(); oit != outNets_.end(); ++oit) {
    (*oit)->checkConsistency();
  }
  // loop over the internal netsOcout  << "inNet done" << endl;
  for (auto nit = internalNets_.begin(); nit != internalNets_.end(); ++nit) {
    (*nit)->checkConsistency();
  }
}

void Cluster::Instance::checkConsistency()
{ 
  // check the consistency of the instance
  if (inNets_.size() != cell_->getNumInputs()) {
    cout << "Error: number of input nets is not equal to the number of inputs"
         << endl;
    exit(0);
  }
  if (outNets_.size() != cell_->getNumOutputs()) {
    cout << "Error: number of output nets is not equal to the number of outputs"
         << endl;
    exit(0);
  }
  // loop over the input nets of the instance
  for (int i = 0; i < inNets_.size(); ++i) {
    int count = 0;
    // loop over the sinks of the input net
    for (auto iit = inNets_[i]->sinks_.begin(); iit != inNets_[i]->sinks_.end();
         ++iit) {
      // check if the terminal in the input net is the same as the current
      // instance
      if (iit->first == this && iit->second == i) {
        count++;
      }
      // return the error message if the terminal in the input net is different
      // from the current instance
    }  // for iit
    if (count != 1) {
      cout << "Error: input net is not connected" << endl;
      exit(0);
    }
  }  // for i
  // loop over the output nets of the instance
  for (int o = 0; o < outNets_.size(); ++o) {
    // check if output net is connected
    if (!outNets_[o]->source_.first) {
      cout << "Error: output terminal is not connected" << endl;
      exit(0);
    }
    if (this->cell_->getLibCell()->hasSequentials()) {
      break;
    }
    // check if the output net is connected to the current instance
    // and if the output net is connected to the current output terminal
    if (outNets_[o]->source_.first != this
        || outNets_[o]->source_.second != o) {
      cout << "Error: net does not point back to instance output terminal"
           << endl;
      exit(0);
    }
  }
}

void Cluster::Net::checkConsistency()
{
  // check the consistency of the net
  for (auto sit = sinks_.begin(); sit != sinks_.end(); ++sit) {
    if (sit->first->inNets_[sit->second] != this) {
      cout << "Error: block input terminal does not point back to net" << endl;
      exit(0);
    }
  }
}

void Cluster::OutputNet::checkConsistency()
{
  Net::checkConsistency();
  if (!source_.first) {
    cout << "Error: output net is not connected" << endl;
    exit(0);
  }
  if (source_.first && source_.first->outNets_[source_.second] != this) {
    cout << "Error: block output terminal does not point back to net" << endl;
    exit(0);
  }
}

void Cluster::writeNetlistInfo(ofstream& outFile, Block* block, string prefix)
{
  auto now = std::chrono::system_clock::now();
  std::time_t cur_time = std::chrono::system_clock::to_time_t(now);

  int I, O;
  block->getIO(block->getArea(), I, O);

  int PI = numInputs_;
  int PO = numOutputs_;

  outFile << prefix << " Generated by ArtNetGen on " << std::ctime(&cur_time)
          << endl;
  outFile << prefix << " Netlist Information" << endl;
  outFile << prefix << " \tBlock Name: " << block->getBlockName() << endl;
  outFile << prefix << " \tNumber of Insts: " << block->getNumBlocks() << endl;
  outFile << prefix << " \tNumber of Inputs: " << PI << endl;
  outFile << prefix << " \tNumber of Outputs: " << PO << endl;
  outFile << prefix << " \tg_frac: " << double(PO) / (PI + PO) << endl;
  outFile << prefix << endl;
  outFile << prefix << endl;

  block->writeRegionInfo(outFile, prefix);
}

void Cluster::printPI()
{
  cout << "Now print PIs - total number of PI is " << inNets_.size() << endl;
  int i = 0;
  for (auto inNet : inNets_) {
    for (auto sink : inNet->sinks_) {
      cout << "    sink of PI " << sink.first->cell_->getName() << endl;
      i++;
    }
  }
  if (i != inNets_.size()) {
    cout << "need check PI " << i << endl;
  }
}

void Cluster::printPO()
{
  cout << "Now print POs - total number of PO is " << outNets_.size() << endl;
  int i = 0;
  for (auto outNet : outNets_) {
    cout << "    source of PO " << outNet->source_.first->cell_->getName()
         << endl;
    i++;
  }
  if (i != outNets_.size()) {
    cout << "need check PO " << i << endl;
  }
}

void Cluster::postProcessing(Block* block)
{
  /* PI, PO post processing */
  int tarNumPI;
  int tarNumPO;

  block->getIO(INT_MAX, tarNumPI, tarNumPO);

  int curNumPI = inNets_.size();
  int curNumPO = outNets_.size();

  int PIdiff = tarNumPI - curNumPI;

  if (PIdiff > 0) {
    cout << "add PI " << PIdiff << endl;
    addPI(PIdiff, block);
  } else if (PIdiff < 0) {
    PIdiff = abs(PIdiff);
    cout << "del PI " << PIdiff << endl;
    delPI(PIdiff, block);
  }

  if (PIdiff != 0) {
    cout << "Error : target number of PI and "
         << "current number of PI do not match " << PIdiff << endl;
  }

  int POdiff = tarNumPO - curNumPO;

  if (POdiff > 0) {
    cout << "add PO " << POdiff << endl;
    addPO(POdiff, block);

  } else if (POdiff < 0) {
    POdiff = abs(POdiff);
    cout << "del PO " << POdiff << endl;

    delPO(POdiff, block);
  }

  if (POdiff != 0) {
    cout << "Error : target number of PO and "
         << "current number of PO do not match " << POdiff << endl;
  }
}

// add PI / upper bound is instances_.size()
void Cluster::addPI(int& tarNum, Block* block)
{
  // disconnect existing connections
  // priority : PI lev distribution
  // if not given,
  // (1) multi-sink PI, (2) low-level insts, (3) number of sinks

  for (auto inNet : inNets_) {
    if (inNet->sinks_.size() > 1) {
      for (auto sink_iter = inNet->sinks_.begin();
           sink_iter != inNet->sinks_.end();) {
        if (sink_iter == inNet->sinks_.begin()) {
          ++sink_iter;
          continue;
        }
        double maxPath = ArtNetGen::delay_.SamplePath();
        double minPath
            = min(double(ArtNetGen::delay_.getMinPath()), maxPath / 2);
        InputNet* newIn = new InputNet(minPath, maxPath);

        Instance* inst = (*sink_iter).first;
        int n = (*sink_iter).second;

        newIn->sinks_.push_back(Terminal(inst, n));
        inst->inNets_[n] = newIn;
        inNets_.push_back(newIn);
        tarNum--;
        sink_iter = inNet->sinks_.erase(sink_iter);

        if (tarNum == 0)
          return;
      }
    }
    if (tarNum == 0)
      return;
  }

  multimap<IntPair, Instance*> instMap;

  int indexA;
  int indexB;

  if (tarNum > instances_.size()) {
    cout << "Error: target number of PI is larger than the number of instances "
         << tarNum << endl;
    tarNum = instances_.size();
  }

  for (auto inst : instances_) {
    if (inst->isSequential() || inst->inNets_.size() < 2) {
      continue;
    }

    indexA = inst->maxLevel_;
    indexB = INT_MAX;

    for (auto inNet : inst->inNets_) {
      int numSink = inNet->sinks_.size();
      if (numSink > 2) {
        indexB = min(indexB, numSink);
        instMap.insert(pair<IntPair, Instance*>(IntPair(indexA, indexB), inst));
      }
    }
  }

  for (auto it = instMap.begin(); it != instMap.end(); ++it) {
    int minSink = it->first.second;

    int n = 0;
    for (auto inNet : it->second->inNets_) {
      if (inNet->sinks_.size() == minSink) {
        if (find(inNets_.begin(), inNets_.end(), inNet) == inNets_.end()) {
          double maxPath = ArtNetGen::delay_.SamplePath();
          double minPath
              = min(double(ArtNetGen::delay_.getMinPath()), maxPath / 2);
          InputNet* newIn = new InputNet(minPath, maxPath);

          for (auto sink_iter = inNet->sinks_.begin();
               sink_iter != inNet->sinks_.end();
               ++sink_iter) {
            if ((*sink_iter).first == it->second) {
              inNet->sinks_.erase(sink_iter);
              break;
            }
          }

          newIn->sinks_.push_back(Terminal(it->second, n));
          it->second->inNets_[n] = newIn;
          inNets_.push_back(newIn);
          tarNum--;
        }
      }
      n++;

      if (tarNum == 0)
        return;
    }
  }
}

// delte PI / lower bound is 1
void Cluster::delPI(int& tarNum, Block* block)
{
  // combine existing Inputnets
  // priority (1): number of sinks
  // priority (2): number of connected outputs

  multimap<IntPair, InputNet*> inNetMap;

  int indexA = 0;
  int indexB = 0;

  for (auto inNet : inNets_) {
    indexA = inNet->sinks_.size();
    indexB = inNet->connectedOutputs_.size();

    inNetMap.insert(pair<IntPair, InputNet*>(IntPair(indexA, indexB), inNet));
  }

  while (tarNum > 0) {
    InputNet* inNetA = inNetMap.begin()->second;
    inNetMap.erase(inNetMap.begin());
    InputNet* inNetB = inNetMap.begin()->second;
    inNetMap.erase(inNetMap.begin());

    if (inNetA->connect(inNetB)) {
      tarNum--;
      auto it = find(inNets_.begin(), inNets_.end(), inNetB);
      if (it != inNets_.end()) {
        inNets_.erase(it);
      }
      inNetMap.insert(pair<IntPair, InputNet*>(
          IntPair(inNetA->sinks_.size(), inNetA->connectedOutputs_.size()),
          inNetA));
    }

    if (inNets_.size() == 1) {
      return;
    }
  }
}

// add PO / uppder bound is instances_.size()
void Cluster::addPO(int& tarNum, Block* block)
{
  // priority (1): low-level insts
  // priority (2): number of sinks
  ang::randomizeList(outNets_);

  multimap<IntPair, Instance*> instMap;

  int indexA = 0;
  int indexB = 0;

  if (tarNum > instances_.size()) {
    cout << "Error: target number of PO is larger than the number of instances"
         << endl;
    tarNum = instances_.size();
  }

  for (auto inst : instances_) {
    if (inst->isSequential())
      continue;

    indexA = inst->maxLevel_;
    indexB = inst->outNets_[0]->sinks_.size();

    instMap.insert(pair<IntPair, Instance*>(IntPair(indexA, indexB), inst));
  }

  while (tarNum > 0) {
    for (auto it = instMap.begin(); it != instMap.end(); ++it) {
      if (find(outNets_.begin(), outNets_.end(), it->second->outNets_[0])
          == outNets_.end()) {
        outNets_.push_back(it->second->outNets_[0]);
        tarNum--;
      }

      if (tarNum == 0)
        return;
    }
  }
}

// delete PO
void Cluster::delPO(int& tarNum, Block* block)
{
  // priority (1): remove multi-fanout nets from outNet
  // priority (2): connect w PI? --> not implemented yet

  ang::randomizeList(outNets_);

  while (tarNum > 0) {
    int noSink = 0;

    for (auto it = outNets_.begin(); it != outNets_.end();) {
      if ((*it)->sinks_.size() > 0) {
        it = outNets_.erase(it);
        tarNum--;

        if (tarNum == 0)
          return;

      } else {
        ++it;
        ++noSink;
      }
    }
    if (noSink == outNets_.size()) {
      cout << "no sink: " << noSink << endl;
      return;
    }
  }
}

void Cluster::writeVerilog(Block* block)
{
  string fileName;
  string clockName;
  string resetName;

  if (!ArtNetGen::netlistFile_.empty())
    fileName = ArtNetGen::netlistFile_;
  else
    fileName = block->getBlockName() + ".v";

  if (!ArtNetGen::clockName_.empty())
    clockName = ArtNetGen::clockName_;
  else
    clockName = "clk";

  resetName = "rst";

  ofstream verilog(fileName);

  if (ArtNetGen::printNetlistInfo_) {
    writeNetlistInfo(verilog, block, "#");
  }

  if (!verilog.is_open()) {
    cout << "Error: cannot open file" << endl;
    exit(0);
  }

  CounterMap countNet;

  CounterMap countInst;

  verilog << "module " << block->getBlockName() << " (";

  // PI
  if (!inNets_.empty()) {
    for (auto it = inNets_.begin(); it != inNets_.end(); ++it) {
      verilog << "net_" << countNet[*it] << ", " << endl;
    }
  }
  // CLK
  verilog << clockName << ", " << endl;
  verilog << resetName << ", " << endl;

  // PO
  if (!outNets_.empty()) {
    int i = 0;
    for (auto it = outNets_.begin(); it != outNets_.end(); ++it) {
      if (i == outNets_.size() - 1) {
        verilog << "net_" << countNet[*it] << ");" << endl;
      } else {
        verilog << "net_" << countNet[*it] << ", " << endl;
      }
      i++;
    }
  }
  verilog << endl;
  // PI
  if (!inNets_.empty()) {
    for (auto iit = inNets_.begin(); iit != inNets_.end(); ++iit) {
      verilog << "input net_" << countNet[*iit] << ";" << endl;
    }
  }

  // CLK
  verilog << "input " << clockName << ";" << endl;
  verilog << "input " << resetName << ";" << endl;

  verilog << endl;

  // PO
  if (!outNets_.empty()) {
    for (auto it = outNets_.begin(); it != outNets_.end(); ++it) {
      verilog << "output net_" << countNet[*it] << ";" << endl;
    }
  }

  verilog << endl;

  // Wires
  if (!internalNets_.empty()) {
    for (auto it = internalNets_.begin(); it != internalNets_.end(); ++it) {
      verilog << "wire net_" << countNet[*it] << ";" << endl;
    }
  }

  verilog << endl;

  // Instances
  for (auto iit = instances_.begin(); iit != instances_.end(); ++iit) {
    dbMaster* master = (*iit)->cell_->getDbMaster();
    LibertyCell* lib_cell = (*iit)->cell_->getLibCell();

    if (master == nullptr) {
      cout << "Error: master is null" << endl;
      exit(0);
    }

    vector<string> pinName;
    verilog << (*iit)->cell_->getName() << " " << "inst_" << countInst[*iit]
            << " (";

    if ((*iit)->cell_->isMacro) {
      // clock pin
      ang::getMTermNames(master, "in", "clock", pinName);

      if (pinName.size() != 1) {
        cout << master->getName() << endl;
        cout << "Error: number of clock pins is not equal to 1" << endl;
        cout << "This version does not support Multi-Bit FF" << endl;
        exit(0);
      }

      verilog << "." << pinName[0] << "(" << clockName << ")," << endl;
      pinName.clear();

      // signal in pin
      ang::getMTermNames(master, "in", "signal", pinName);

      if (pinName.size() != (*iit)->inNets_.size()) {
        cout << "Error: number of input nets is not equal to the number of "
                "input pins"
             << endl;
        cout << master->getName() << " " << pinName.size() << " "
             << (*iit)->inNets_.size() << endl;

        for (int i = 0; i < pinName.size(); ++i)
          cout << pinName[i] << endl;
        exit(0);
      }

      map<string, vector<string>> pinGroups;

      for (int i = 0; i < (*iit)->inNets_.size(); ++i) {
        string pin = pinName[i];
        string baseName;

        size_t bracketPos = pin.find('[');
        if (bracketPos != std::string::npos) {
          baseName = pin.substr(0, bracketPos);
        } else {
          baseName = pin;
        }

        string net = "net_" + std::to_string(countNet[(*iit)->inNets_[i]]);
        pinGroups[baseName].push_back(net);
      }

      for (const auto& [baseName, nets] : pinGroups) {
        verilog << "." << baseName << "({";

        for (size_t i = 0; i < nets.size(); ++i) {
          verilog << nets[i];
          if (i < nets.size() - 1) {
            verilog << ", ";
          }
        }

        verilog << "})," << endl;
      }

      pinName.clear();
      pinGroups.clear();

      // signal out pin
      ang::getMTermNames(master, "out", "signal", pinName);

      if (pinName.size() != (*iit)->outNets_.size()) {
        cout << "Error: number of output nets is not equal to the number of "
                "output pins"
             << endl;
        cout << master->getName() << " " << pinName.size() << " "
             << (*iit)->outNets_.size() << endl;
        for (int i = 0; i < pinName.size(); ++i)
          cout << pinName[i] << endl;
        exit(0);
      }

      for (int i = 0; i < (*iit)->outNets_.size(); ++i) {
        string pin = pinName[i];
        string baseName;

        size_t bracketPos = pin.find('[');
        if (bracketPos != std::string::npos) {
          baseName = pin.substr(0, bracketPos);
        } else {
          baseName = pin;
        }

        string net = "net_" + std::to_string(countNet[(*iit)->outNets_[i]]);
        pinGroups[baseName].push_back(net);
      }

      int groupCount = 0;
      int totalGroups = pinGroups.size();

      for (const auto& [baseName, nets] : pinGroups) {
        groupCount++;
        bool isLastGroup = (groupCount == totalGroups);

        if (nets.size() == 1) {
          if (isLastGroup) {
            verilog << "." << baseName << "(" << nets[0] << "));" << std::endl;
          } else {
            verilog << "." << baseName << "(" << nets[0] << ")," << std::endl;
          }

        } else {
          verilog << "." << baseName << "({";

          for (size_t i = 0; i < nets.size(); ++i) {
            verilog << nets[i];
            if (i < nets.size() - 1) {
              verilog << ", ";
            }
          }

          if (isLastGroup) {
            verilog << "}));" << std::endl;
          } else {
            verilog << "})," << std::endl;
          }
        }
      }

      pinName.clear();
      pinGroups.clear();

    } else {
      if (lib_cell->hasSequentials()) {
        ang::getMTermNames(master, "in", "clock", pinName);

        if (pinName.size() != 1) {
          cout << master->getName() << endl;
          cout << "Error: number of clock pins is not equal to 1" << endl;
          cout << "This version does not support Multi-Bit FF" << endl;
          exit(0);
        }

        verilog << "." << pinName[0] << "(" << clockName << ")," << endl;
        pinName.clear();
      }

      if (lib_cell->hasSequentials()) {
        ang::getMTermNames(master, "in", "reset", pinName);

        for (int i = 0; i < pinName.size(); ++i)
          if (pinName[i] == "RESET")  // for asap7
            verilog << "." << pinName[i] << "(" << resetName << ")," << endl;
          else
            verilog << "." << pinName[i] << "(1'b0)," << endl;
        pinName.clear();
      }

      ang::getMTermNames(master, "in", "signal", pinName);

      if (pinName.size() != (*iit)->inNets_.size()) {
        cout << "Error: number of input nets is not equal to the number of "
                "input pins"
             << endl;
        cout << master->getName() << " " << pinName.size() << " "
             << (*iit)->inNets_.size() << endl;

        for (int i = 0; i < pinName.size(); ++i)
          cout << pinName[i] << endl;
        exit(0);
      }

      for (int i = 0; i < (*iit)->inNets_.size(); ++i) {
        verilog << "." << pinName[i] << "(net_" << countNet[(*iit)->inNets_[i]]
                << ")," << endl;
      }
      pinName.clear();

      ang::getMTermNames(master, "out", "signal", pinName);

      if (pinName.size() != (*iit)->outNets_.size()) {
        cout << "Error: number of output nets is not equal to the number of "
                "output pins"
             << endl;
        exit(0);
      }

      for (int o = 0; o < (*iit)->outNets_.size(); ++o) {
        if (o == (*iit)->outNets_.size() - 1) {
          verilog << "." << pinName[o] << "(net_"
                  << countNet[(*iit)->outNets_[o]] << "));" << endl;
        } else {
          verilog << "." << pinName[o] << "(net_"
                  << countNet[(*iit)->outNets_[o]] << ")," << endl;
        }
      }
      pinName.clear();
    }
  }

  verilog << "endmodule" << endl;
}

}  // namespace ang
