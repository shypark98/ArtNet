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

#include "delay_level.h"

#include "ang/artNetGen.h"
#include "ord/OpenRoad.hh"

using odb::dbBlock;
using odb::dbBTerm;
using odb::dbChip;
using odb::dbDatabase;
using odb::dbInst;
using odb::dbIoType;
using odb::dbITerm;
using odb::dbLib;
using odb::dbMaster;
using odb::dbNet;
using odb::dbSet;
using odb::dbSigType;

using std::cout;
using std::deque;
using std::endl;
using std::find;
using std::make_unique;
using std::map;
using std::max;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

using utl::ANG;

namespace ang {

bool Node::isSequential() const
{
  if (inst == nullptr)
    return false;
  else
    return inst->getMaster()->isSequential();
}

bool Node::isPrimaryIn() const
{
  if (bterm == nullptr)
    return false;
  else
    return (bterm->getIoType() == dbIoType::INPUT);
}

bool Node::isPrimaryOut() const
{
  if (bterm == nullptr)
    return false;
  else
    return (bterm->getIoType() == dbIoType::OUTPUT);
}

string Node::getName() const
{
  if (bterm == nullptr)
    return inst->getName();
  else
    return bterm->getName();
}

void DelayLevel::initNodes()
{
  cout << "init nodes begin" << endl;

  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();
  dbChip* chip = db->getChip();
  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }

  dbBlock* block = chip->getBlock();

  dbSet<dbInst> insts = block->getInsts();
  dbSet<dbBTerm> bterms = block->getBTerms();

  nodes_.reserve(insts.size() + bterms.size());

  for (auto inst_itr = insts.begin(); inst_itr != insts.end(); ++inst_itr) {
    dbInst* inst = *inst_itr;
    // dbMaster* master = inst->getMaster();

    Node* node = new Node();
    nodes_.push_back(node);
    node->inst = inst;
    inst2node_[inst] = node;
    if (node->isSequential())
      FFs_.push_back(node);
    else
      Combis_.push_back(node);
  }

  for (auto bterm_itr = bterms.begin(); bterm_itr != bterms.end();
       ++bterm_itr) {
    dbBTerm* bterm = *bterm_itr;

    if (bterm->getSigType() != dbSigType::SIGNAL)
      continue;

    Node* node = new Node();
    nodes_.push_back(node);
    node->bterm = bterm;
    bterm2node_[bterm] = node;

    if (bterm->getIoType() == dbIoType::INPUT) {
      PIs_.push_back(node);
    } else if (bterm->getIoType() == dbIoType::OUTPUT) {
      POs_.push_back(node);
    }
  }

  for (auto inst_itr = insts.begin(); inst_itr != insts.end(); ++inst_itr) {
    dbInst* source_inst = *inst_itr;
    dbSet<dbITerm> inst_iterms = source_inst->getITerms();

    for (auto inst_iterm_itr = inst_iterms.begin();
         inst_iterm_itr != inst_iterms.end();
         ++inst_iterm_itr) {
      dbITerm* iterm = *inst_iterm_itr;

      if (iterm->getNet() == NULL) {
        cout << "Net is not connected to " << source_inst->getName() << " "
             << iterm->getName() << endl;
        continue;
      }
      if (iterm->getSigType() != dbSigType::SIGNAL)
        continue;

      dbNet* net = iterm->getNet();
      if (iterm->getIoType() == dbIoType::OUTPUT) {
        dbSet<dbITerm> net_iterms = net->getITerms();
        dbSet<dbBTerm> net_bterms = net->getBTerms();

        for (auto net_iterm_itr = net_iterms.begin();
             net_iterm_itr != net_iterms.end();
             ++net_iterm_itr) {
          dbITerm* net_iterm = *net_iterm_itr;
          if (net_iterm->getIoType() == dbIoType::OUTPUT) {
            dbInst* sourceInst = net_iterm->getInst();
            if (sourceInst != source_inst) {
              cout << "Multi-driven Net!" << endl;
              // exit(0);
            }
          }

          if (net_iterm->getIoType() == dbIoType::INPUT) {
            dbInst* sinkInst = net_iterm->getInst();

            Node* sink = inst2node_[sinkInst];
            Node* source = inst2node_[source_inst];
            sink->sources.push_back(source);
            source->sinks.push_back(sink);
          }
        }

        for (auto net_bterm_itr = net_bterms.begin();
             net_bterm_itr != net_bterms.end();
             ++net_bterm_itr) {
          dbBTerm* net_bterm = *net_bterm_itr;
          // if net_bterm is primary output
          if (net_bterm->getIoType() == dbIoType::OUTPUT) {
            Node* sink = bterm2node_[net_bterm];
            Node* source = inst2node_[source_inst];
            sink->sources.push_back(source);
            source->sinks.push_back(sink);
          }
        }
      }  // if output
    }  // for iterms
  }  // for insts

  for (auto bterm_itr = bterms.begin(); bterm_itr != bterms.end();
       ++bterm_itr) {
    dbBTerm* bterm = *bterm_itr;

    if (bterm->getSigType() != dbSigType::SIGNAL)
      continue;

    if (bterm->getNet() == NULL)
      continue;

    dbNet* net = bterm->getNet();

    // if bterm is primary input
    if (bterm->getIoType() == dbIoType::INPUT) {
      dbSet<dbITerm> net_iterms = net->getITerms();
      dbSet<dbBTerm> net_bterms = net->getBTerms();

      // pin connected to primary input
      for (auto net_iterm_itr = net_iterms.begin();
           net_iterm_itr != net_iterms.end();
           ++net_iterm_itr) {
        dbITerm* net_iterm = *net_iterm_itr;
        if (net_iterm->getIoType() == dbIoType::OUTPUT) {
        }

        if (net_iterm->getIoType() == dbIoType::INPUT) {
          dbInst* sink_inst = net_iterm->getInst();
          Node* sink = inst2node_[sink_inst];
          Node* source = bterm2node_[bterm];
          sink->sources.push_back(source);
          source->sinks.push_back(sink);
        }
      }

      for (auto net_bterm_itr = net_bterms.begin();
           net_bterm_itr != net_bterms.end();
           ++net_bterm_itr) {
        dbBTerm* net_bterm = *net_bterm_itr;

        // if net_bterm is primary output
        if (net_bterm->getIoType() == dbIoType::OUTPUT) {
          Node* sink = bterm2node_[net_bterm];
          Node* source = bterm2node_[bterm];
          sink->sources.push_back(source);
          source->sinks.push_back(sink);
        }
      }
    }
  }
  cout << "init nodes done" << endl;
}

void DelayLevel::labelCombiDelay()
{
  deque<Node*> candiNodes;
  cout << "label CombiDelay start " << top_nodes_.size() << endl;

  for (auto node : top_nodes_) {
    if (node->isSequential()) {
      if (!(node->is_marked && node->level == 0)) {
        cout << "Error: " << node->inst->getName() << " is not a combi logic"
             << endl;
        exit(0);
      }
    } else {  // if node is combi
      candiNodes.push_back(node);
      while (!candiNodes.empty()) {
        Node* candiNode = candiNodes.front();
        candiNodes.pop_front();
        if (!candiNode->is_marked)
          calcCombiDelay(candiNode, candiNodes);
      }
    }
  }
}

void DelayLevel::calcCombiDelay(Node* node, deque<Node*>& candiNodes)
{
  int max_level = 0;

  getUnmarkedSinks(node, candiNodes);

  // find max delay of fanin nodes
  if (node->sources.size() != 0) {
    for (auto source : node->sources) {
      // if source is combi logic
      if (!source->isSequential()) {
        if (!source->is_marked)
          calcCombiDelay(source, candiNodes);
        if (!(source->is_marked && source->level >= 0)) {
          cout << "Error: source should not be marked" << endl;
          exit(0);
        }
        max_level = max(max_level, source->level);
      }
    }
  }

  node->level = max_level + 1;
  node->is_marked = true;
  max_level_ = max(max_level_, max_level + 1);
}

void DelayLevel::getUnmarkedSinks(Node* node, deque<Node*>& candiNodes)
{
  for (auto sink : node->sinks) {
    if (!(sink->is_marked && sink->isSequential())) {
      candiNodes.push_back(sink);
    } else {
      if (!(sink->is_marked || sink->isSequential())) {
        cout << "Error: " << sink->getName() << " is marked ("
             << sink->is_marked << ")" << " or sequential ("
             << sink->isSequential() << ")" << endl;
        exit(0);
      }
    }
  }
}

void DelayLevel::labelPIDelay()
{
  for (auto pi : PIs_) {
    pi->level = 0;
    pi->is_marked = true;
    // cout << "PI: " << pi->getName() << endl;
  }
  cout << "labelPI delay done" << endl;
}

void DelayLevel::labelFFDelay()
{
  for (auto ff : FFs_) {
    if (!ff->isSequential()) {
      cout << "Error: " << ff->inst->getName() << " is not a flip-flop" << endl;
      exit(0);
    }
    ff->level = 0;
    ff->is_marked = true;
    // cout << "FF: " << ff->getName() << " " << ff->inst->getName() << endl;
  }
  cout << "labelFF delay done" << endl;
}

void DelayLevel::setTopNodes()
{
  // set top nodes connected to primary inputs
  for (auto pi : PIs_) {
    for (auto sink : pi->sinks) {
      auto it = find(top_nodes_.begin(), top_nodes_.end(), sink);
      if (it == top_nodes_.end()) {
        top_nodes_.push_back(sink);
      }
    }
  }

  // set top nodes connected to flip-flops
  for (auto ff : FFs_) {
    for (auto sink : ff->sinks) {
      auto it = find(top_nodes_.begin(), top_nodes_.end(), sink);
      if (it == top_nodes_.end()) {
        top_nodes_.push_back(sink);
      }
    }
  }
  cout << "set top nodes done" << endl;
}

// check every node has its delay
void DelayLevel::checkConsistency() const
{
  for (auto node : nodes_) {
    if (node->isSequential()) {
      if (node->level != 0) {
        cout << "Error: " << node->getName()
             << " is sequential but not labeled with level 0" << endl;
        // exit(0);
      }
    } else {
      if (!node->is_marked) {
        cout << "Error: " << node->getName() << " is not labeled with level"
             << endl;
        // exit(0);
      }
    }
  }
  cout << "check consistency done" << endl;
}

unordered_map<int, int> DelayLevel::getNodeShape()
{
  for (int i = 0; i < max_level_; i++) {
    nodeShape_[i] = 0;
  }

  for (auto node : nodes_) {
    nodeShape_[node->level]++;
  }

  return nodeShape_;
}

unordered_map<int, int> DelayLevel::getPIShape()
{
  for (auto pi : PIs_) {
    int max_sink_level = 0;

    for (auto sink : pi->sinks) {
      max_sink_level = max(max_sink_level, sink->level);
    }

    if (PIShape_.find(max_sink_level) == PIShape_.end()) {
      PIShape_[max_sink_level] = 0;
    }

    PIShape_[max_sink_level]++;
  }

  return PIShape_;
}

unordered_map<int, int> DelayLevel::getPOShape()
{
  for (auto po : POs_) {
    int max_source_level = 0;

    for (auto source : po->sources) {
      max_source_level = max(max_source_level, source->level);
    }

    if (POShape_.find(max_source_level) == POShape_.end()) {
      POShape_[max_source_level] = 0;
    }

    POShape_[max_source_level]++;
  }

  return POShape_;
}

map<int, int> DelayLevel::getFFShape()
{
  cout << max_level_ << endl;

  for (int i = 0; i < max_level_; i++) {
    FFShape_[i] = 0;
  }

  for (auto ff : FFs_) {
    int max_source_level = 0;

    if (ff->sources.size() != 1) {
      cout << "Error: " << ff->inst->getName() << " has multiple sinks "
           << ff->inst->getMaster()->getName() << endl;
      for (auto source : ff->sources)
        cout << source->getName() << " ";
      cout << endl;
    }

    for (auto source : ff->sources) {
      max_source_level = max(max_source_level, source->level);
    }

    if (FFShape_.find(max_source_level) == FFShape_.end()) {
      FFShape_[max_source_level] = 0;
    }

    FFShape_[max_source_level]++;
  }

  return FFShape_;
}

}  // namespace ang
