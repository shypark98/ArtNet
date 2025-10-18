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

#include <math.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "ang/artNetGen.h"
#include "circuit.h"
#include "db_sta/dbNetwork.hh"
#include "db_sta/dbSta.hh"
#include "odb/db.h"
#include "ord/OpenRoad.hh"
#include "sta/ArcDelayCalc.hh"
#include "sta/Bfs.hh"
#include "sta/Corner.hh"
#include "sta/DcalcAnalysisPt.hh"
#include "sta/ExceptionPath.hh"
#include "sta/FuncExpr.hh"
#include "sta/Graph.hh"
#include "sta/GraphDelayCalc.hh"
#include "sta/Liberty.hh"
#include "sta/Network.hh"
#include "sta/PathAnalysisPt.hh"
#include "sta/PathEnd.hh"
#include "sta/PathExpanded.hh"
#include "sta/PatternMatch.hh"
#include "sta/PortDirection.hh"
#include "sta/Sdc.hh"
#include "sta/Search.hh"
#include "sta/SearchPred.hh"
#include "sta/Sequential.hh"
#include "sta/Sta.hh"
#include "sta/Units.hh"
#include "utl/Logger.h"
// for easier coding with boost

// save point and dbInst* pointer

using std::cout;
using std::endl;
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::set;
using std::string;
using std::vector;
using namespace odb;
using std::ifstream;
using std::map;
using std::multimap;
using std::ofstream;
using std::queue;
using std::sqrt;
using std::stringstream;
using std::to_string;
using std::unordered_map;

namespace ang {

using utl::ANG;

/*
void ArtNetGen::BuildTimingPaths(int top_n_, bool verbose, bool sort, bool
minmax)
{
  dbBlock* block_ = db_->getChip()->getBlock();
  sta_->ensureGraph();     // Ensure that the timing graph has been built
  sta_->searchPreamble();  // Make graph and find delays
  sta_->ensureLevelized();
  // Step 1:  find the top_n critical timing paths
  sta::ExceptionFrom* e_from = nullptr;
  sta::ExceptionThruSeq* e_thrus = nullptr;
  sta::ExceptionTo* e_to = nullptr;
  bool include_unconstrained = false;
  bool get_max = minmax;  // max for setup check, min for hold check
  // Timing paths are grouped into path groups according to the clock
  // associated with the endpoint of the path, for example, path group for clk
  int group_count = top_n_;
  int endpoint_count = 1;  // The number of paths to report for each endpoint.
  sta::PathEndSeq path_ends = sta_->search()->findPathEnds(  // from, thrus,
      e_from,   // return paths from a list of clocks/instances/ports/register
                // clock pins or latch data pins
      e_thrus,  // return paths through a list of instances/ports/nets
      e_to,     // return paths to a list of clocks/instances/ports or pins
      include_unconstrained,  // return unconstrained paths
      // corner, min_max,
      sta_->cmdCorner(),  // return paths for a process corner
      get_max ? sta::MinMaxAll::max()
              : sta::MinMaxAll::min(),  // return max/min paths checks
      // group_count, endpoint_count, unique_pins
      group_count,     // number of paths in total
      endpoint_count,  // number of paths for each endpoint
      true,
      -sta::INF,
      sta::INF,  // slack_min, slack_max,
      true,      // sort_by_slack
      nullptr,   // group_names
      // setup, hold, recovery, removal,
      get_max,
      !get_max,
      false,
      false,
      // clk_gating_setup, clk_gating_hold
      false,
      false);
  if (sort) {
      if (get_max) {
        std::sort(path_ends.begin(), path_ends.end(),
              [&](const auto& path_end1, const auto& path_end2) {
                  sta::PathExpanded expand1(path_end1->path(), sta_);
                  sta::PathExpanded expand2(path_end2->path(), sta_);
                  return expand1.size() > expand2.size();
              });
      } else {
        std::sort(path_ends.begin(), path_ends.end(),
              [&](const auto& path_end1, const auto& path_end2) {
                  sta::PathExpanded expand1(path_end1->path(), sta_);
                  sta::PathExpanded expand2(path_end2->path(), sta_);
                  return expand1.size() < expand2.size();
              });
      }
  }
  cout << "total num paths : " << path_ends.size() << endl;
  // check all the timing paths
  for (auto& path_end : path_ends) {
    // Printing timing paths to logger
    // sta_->reportPathEnd(path_end);
    auto* path = path_end->path();
    const float slack = path_end->slack(sta_);  // slack information
    // TODO: to be deleted.  We should not
    // normalize the slack according to the clock period for multi-clock
    // design
    const float clock_period = path_end->targetClk(sta_)->period();
    //maximum_clock_period_ = std::max(maximum_clock_period_, clock_period);
    // logger_->report("clock_period = {}, slack = {}", maximum_clock_period_,
    // timing_path.slack);
    sta::PathExpanded expand(path, sta_);
    expand.path(expand.size() - 1);
    std::cout << "here path starts -> num vertices: " << double(expand.size()) /
2 << endl; for (size_t i = 0; i < expand.size(); i++) {
      // PathRef is reference to a path vertex
      sta::PathRef* ref = expand.path(i);
      sta::Pin* pin = ref->vertex(sta_)->pin();
      // Nets connect pins at a level of the hierarchy
      auto net = network_->net(pin);  // sta::Net*
      // Check if the pin is connected to a net
      if (net == nullptr) {
        continue;  // check if the net exists
      }
      if (network_->isTopLevelPort(pin) == true) {
        auto bterm = block_->findBTerm(network_->pathName(pin));
        if (verbose)
            std::cout << bterm->getName() << std::endl;

      } else {
        auto inst = network_->instance(pin);
        auto db_inst = block_->findInst(network_->pathName(inst));
        if (verbose)
            std::cout << db_inst->getName() << " " <<
db_inst->getMaster()->getName() << std::endl;
      }
    }
  }
}
*/

void ArtNetGen::printTree()
{
  int seed = getSeed();
  checkLibs();
  srand(seed);
  readSpec();
  delay_.setMaxPath(maxPathLen_);
  delay_.setMinPath(minPathLen_);
  circuit_->printTree();
}

void Block::printTree()
{
  InitName();
  InitQueue();
  /*
  if (ArtNetGen::printForrest_) {
      cout << "Before Hierarchical Clustering" << endl;

      for (auto it = pqueue_.begin(); it != pqueue_.end(); ++it) {
          InstNode *lc = dynamic_cast<InstNode *>(it->second);
          cout << lc->cell_->getName() << " " << endl;
      }

      cout << "After Hierarchical Clustering" << endl;
  }

  HClustering();

  std::function<void(TreeNode*)> dfs = [&](TreeNode* node) {
      if (node == nullptr) {
          cout << "Node is null" << endl;
          return;
      }

      InstNode* libNode = dynamic_cast<InstNode*>(node);
      CompoundNode* compNode = dynamic_cast<CompoundNode*>(node);

      if (libNode) {
          cout << "library node: " << libNode->cell_->getName() << endl;
          return;
      } else if (compNode) {
          cout << "compound node- sum: " << compNode->getNumBlocks()<< " left: "
  << compNode->getLeft()->getNumBlocks() << " right: " <<
  compNode->getRight()->getNumBlocks() << endl; dfs(compNode->getLeft()); cout
  << "----------------" << endl; dfs(compNode->getRight()); } else { cout <<
  "Wrong node type" << endl; return;
      }
  };

  if (pqueue_.empty()) {
      cout << "Forrest is empty" << endl;
      return;
  } else if (pqueue_.size() != 1) {
      cout << "Forrest has more than one tree" << endl;
      return;
  }

  for (auto it = pqueue_.begin(); it != pqueue_.end(); ++it) {
      TreeNode* node = it->second;
      dfs(node);
  }
  */
}

void ArtNetGen::reportMetrics()
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

  dbChip* chip = db->getChip();
  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }

  dbBlock* block = chip->getBlock();
  dbSet<dbInst> insts = block->getInsts();
  dbSet<dbNet> nets = block->getNets();

  int FFcount = 0;
  for (auto inst : insts)
    if (inst->getMaster()->isSequential())
      FFcount++;

  int Netcount = 0;
  for (auto net : nets) {
    int ipin_counts = net->getITerms().size();
    int bpin_counts = net->getBTerms().size();

    int tot_pins = ipin_counts + bpin_counts;
    if (tot_pins < 2)
      continue;
    else
      Netcount++;
  }

  cout << "Number of Insts: " << insts.size() << endl;
  cout << "Number of FFs: " << FFcount << endl;
  cout << "Number of Nets: " << Netcount << endl;
}

void ArtNetGen::writeGNLfile(const char* fileName)
{
  unordered_map<string, int> onlyUseMasters;

  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

  dbChip* chip = db->getChip();
  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }

  dbBlock* block_ = chip->getBlock();
  string top_name = block_->getTopModule()->getName();

  dbSet<dbInst> insts = block_->getInsts();
  dbSet<dbBTerm> bterms = block_->getBTerms();

  cout << "here" << endl;

  int numInsts = insts.size();
  int numPIs = 0;
  int numPOs = 0;

  int numSeq = 0;

  for (auto bterm_itr = bterms.begin(); bterm_itr != bterms.end();
       ++bterm_itr) {
    dbBTerm* bterm = *bterm_itr;

    if (bterm->getIoType() == dbIoType::INPUT)
      numPIs++;

    if (bterm->getIoType() == dbIoType::OUTPUT)
      numPOs++;
  }

  for (auto inst_itr = insts.begin(); inst_itr != insts.end(); ++inst_itr) {
    dbInst* inst = *inst_itr;
    dbMaster* master = inst->getMaster();

    if (master->isSequential())
      numSeq++;

    if (onlyUseMasters.find(master->getName()) == onlyUseMasters.end())
      onlyUseMasters[master->getName()] = 0;
    onlyUseMasters[master->getName()]++;
  }

  ofstream outFile(fileName);

  if (!outFile.good()) {
    cout << "Error: cannot open file " << fileName << endl;
    exit(0);
  }

  outFile << "[library]" << endl;
  outFile << "name=lib" << endl;

  for (auto it : onlyUseMasters) {
    dbMaster* master = db_->findMaster((it.first).c_str());

    int numInputs = 0;
    int numOutputs = 0;
    getNumInOutPins(master, numInputs, numOutputs);

    if (master->isSequential())
      outFile << "latch=" << it.first << " " << numInputs << " " << numOutputs
              << endl;
    else
      outFile << "gate=" << it.first << " " << numInputs << " " << numOutputs
              << endl;
  }

  outFile << "[circuit]" << endl;
  outFile << "name=" << top_name << endl;
  outFile << "libraries=lib" << endl;
  outFile << "distribution=";
  for (auto it : onlyUseMasters) {
    outFile << it.second << " ";
  }
  outFile << endl;
  outFile << "size=" << endl;
  outFile << "  p=" << endl;
  outFile << "size=" << numInsts << endl;
  outFile << " I=" << numPIs << endl;
  outFile << " O=" << numPOs << endl;
  outFile.close();
}

void ArtNetGen::convHNL2Netlist(const char* inFile, const char* outNetlist)
{
  ifstream fin(inFile);
  if (!fin) {
    std::cerr << "Cannot open file: " << inFile << std::endl;
    return;
  }

  string top_module;
  vector<string> inputs;
  vector<string> outputs;
  set<string> allNets;
  map<string, vector<string>> inst2nets;
  map<string, dbMaster*> inst2master;

  std::string line;
  int gate_id = 0;

  while (std::getline(fin, line)) {
    std::istringstream iss(line);
    string token;
    iss >> token;

    if (token == "circuit") {
      iss >> top_module;
    } else if (token == "input") {
      string net;
      while (iss >> net) {
        inputs.push_back(net);
        allNets.insert(net);
      }
    } else if (token == "output") {
      string net;
      while (iss >> net) {
        outputs.push_back(net);
        allNets.insert(net);
      }
    } else if (token == "end") {
      break;
    } else {
      string master_name = token;
      vector<string> nets;
      while (iss >> token) {
        nets.push_back(token);
        allNets.insert(token);
      }

      string inst_name = "inst_" + std::to_string(gate_id++);
      inst2nets[inst_name] = nets;
      dbMaster* master = db_->findMaster(master_name.c_str());
      inst2master[inst_name] = master;
      cout << master_name << endl;
    }
  }

  // === wire = all nets - inputs - outputs
  vector<string> wires;
  for (const auto& net : allNets) {
    if (std::find(inputs.begin(), inputs.end(), net) == inputs.end()
        && std::find(outputs.begin(), outputs.end(), net) == outputs.end()) {
      wires.push_back(net);
    }
  }

  ofstream outFile(outNetlist);
  // === Write Verilog ===
  outFile << "module " << top_module << "(";
  for (const auto& in : inputs)
    outFile << in << "," << endl;
  outFile << "clk," << endl;
  for (size_t i = 0; i < outputs.size(); ++i) {
    outFile << outputs[i];
    if (i != outputs.size() - 1)
      outFile << "," << endl;
  }
  outFile << ");\n" << endl;

  for (const auto& in : inputs)
    outFile << "  input " << in << ";" << endl;

  outFile << "  input clk;" << endl;
  outFile << endl;

  for (const auto& out : outputs)
    outFile << "  output " << out << ";" << endl;
  outFile << endl;

  for (const auto& wire : wires)
    outFile << "  wire " << wire << ";" << endl;

  outFile << endl;

  for (const auto& [inst_name, nets] : inst2nets) {
    dbMaster* master = inst2master[inst_name];

    vector<string> pinName;
    outFile << "  " << master->getName() << " " << inst_name << "(";
    if (master->isSequential()) {
      getMTermNames(master, "in", "clock", pinName);
      outFile << "." << pinName[0] << "(clk)," << endl;
      pinName.clear();
    }
    int i = 0;
    getMTermNames(master, "in", "signal", pinName);
    for (int k = 0; k < pinName.size(); k++) {
      outFile << "." << pinName[k] << "(" << nets[i] << ")," << endl;
      i++;
    }
    pinName.clear();

    getMTermNames(master, "out", "signal", pinName);
    for (int k = 0; k < pinName.size(); k++) {
      if (k == pinName.size() - 1)
        outFile << "." << pinName[k] << "(" << nets[i] << "));" << endl;
      else
        outFile << "." << pinName[k] << "(" << nets[i] << ")," << endl;
      i++;
    }
  }

  outFile << "endmodule" << endl;
}

void ArtNetGen::reportCellDist(const char* fileName)
{
  unordered_map<string, int> onlyUseMasters;

  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();
  dbChip* chip = db->getChip();
  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }
  dbBlock* block_ = chip->getBlock();
  dbSet<dbInst> insts = block_->getInsts();

  int numInsts = insts.size();

  for (auto inst_itr = insts.begin(); inst_itr != insts.end(); ++inst_itr) {
    dbInst* inst = *inst_itr;
    dbMaster* master = inst->getMaster();

    if (onlyUseMasters.find(master->getName()) == onlyUseMasters.end())
      onlyUseMasters[master->getName()] = 0;
    onlyUseMasters[master->getName()]++;

  }  // for insts

  ofstream outFile(fileName);
  if (!outFile.good()) {
    cout << "Error: cannot open file " << fileName << endl;
    exit(0);
  }

  for (auto it : onlyUseMasters) {
    outFile << it.first << " " << 1.0 * it.second / numInsts << endl;
  }

  outFile << endl;
}

void ArtNetGen::CalculateRentParam()
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

  dbChip* chip = db->getChip();
  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }

  dbBlock* block = chip->getBlock();
  dbSet<dbInst> insts = block->getInsts();

  int numOuts = 0;
  int numInts = 0;
  int numInsts = insts.size();

  for (dbNet* net : block->getNets()) {
    bool outGoing = false;

    if (net->getBTerms().size() > 0) {
      outGoing = true;
    }
    if (outGoing) {
      numOuts++;
    } else {
      numInts++;
    }
  }

  float numerator = std::log(static_cast<float>(numOuts)
                             / static_cast<float>(numOuts + numInts));
  float denominator = std::log(static_cast<float>(numInsts));

  cout << "Total Outgoing Nets : " << numOuts << " Internal Nets : " << numInts
       << " Instances : " << numInsts << endl;
  cout << "Rent param is : " << 1.0 + (numerator / denominator) << endl;
}

void ArtNetGen::FloatingCells()
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

  dbChip* chip = db->getChip();

  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }

  dbBlock* block = chip->getBlock();
  dbSet<dbInst> insts = block->getInsts();

  int floats = 0;
  int float_pins = 0;
  for (dbInst* inst : block->getInsts()) {
    int numFanins = 0;
    int numFanouts = 0;

    for (dbITerm* inst_iterm : inst->getITerms()) {
      if (inst_iterm->getNet() == NULL) {
        cout << "No nets : " << inst->getName() << " "
             << inst->getMaster()->getName() << " " << inst_iterm->getName()
             << endl;
        float_pins++;
        continue;
      }
      if (inst_iterm->getIoType() == dbIoType::INPUT) {
        numFanins++;
      }

      if (inst_iterm->getIoType() == dbIoType::OUTPUT) {
        dbNet* net = inst_iterm->getNet();

        dbSet<dbITerm> net_iterms = net->getITerms();

        for (auto net_iterm_itr = net_iterms.begin();
             net_iterm_itr != net_iterms.end();
             ++net_iterm_itr) {
          dbITerm* net_iterm = *net_iterm_itr;
          dbInst* sink_inst = net_iterm->getInst();

          if (net_iterm->getIoType() == dbIoType::INPUT) {
            numFanouts++;
          }
          if (net_iterm->getMTerm() != NULL) {
          }
        }
      }
    }

    if (!numFanins)
      cout << "No Fanins : " << inst->getName() << " "
           << inst->getMaster()->getName() << endl;

    if (!numFanouts)
      cout << "No Fanouts : " << inst->getName() << " "
           << inst->getMaster()->getName() << endl;

    if (!numFanouts || !numFanins)
      floats++;
  }
  cout << "Total number of float cells : " << floats << endl;
  cout << "Total number of float pins : " << float_pins << endl;
}

void ArtNetGen::FanoutWL()
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

  dbChip* chip = db->getChip();

  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }

  dbBlock* block = chip->getBlock();

  vector<vector<int>> fanoutBbox(51);
  int maxFanout = 0;

  for (dbInst* inst : block->getInsts()) {
    cout << inst->getName() << endl;
    dbSet<dbITerm> inst_iterms = inst->getITerms();

    int numFanouts = 0;
    int bboxSize = 0;

    int sourceX, sourceY;
    inst->getLocation(sourceX, sourceY);

    int lx, ly, ux, uy;
    lx = sourceX, ly = sourceY;
    ux = sourceX, uy = sourceY;

    for (dbITerm* inst_iterm : inst->getITerms()) {
      if (inst_iterm->getNet() == NULL)
        continue;

      if (inst_iterm->getIoType() == dbIoType::OUTPUT) {
        dbNet* net = inst_iterm->getNet();

        for (dbITerm* net_iterm : net->getITerms()) {
          dbInst* sink_inst = net_iterm->getInst();

          int sinkX, sinkY;
          sink_inst->getLocation(sinkX, sinkY);
          lx = min(lx, sinkX), ly = min(ly, sinkY);
          ux = max(ux, sinkX), uy = max(uy, sinkY);

          if (net_iterm->getIoType() == dbIoType::OUTPUT) {
            if (inst_iterm != net_iterm) {
              cout << "invalid output term!" << endl;
            }
          }
          if (net_iterm->getIoType() == dbIoType::INPUT) {
            numFanouts++;
          }
        }
      }
    }
    bboxSize = (abs(ux - lx) + abs(uy - ly));
    if ((numFanouts == 1) && (bboxSize == 0)) {
      cout << "bbox is 0 " << inst->getName() << endl;
    }
    maxFanout = max(maxFanout, numFanouts);
    cout << "maxFanout : " << maxFanout << endl;
    if (numFanouts > 50)
      continue;
    cout << numFanouts << endl;
    fanoutBbox[numFanouts].push_back(bboxSize);
  }

  vector<int> means;

  for (const auto& bbox : fanoutBbox) {
    int sum = 0;
    for (int val : bbox) {
      sum += val;
    }
    if (!bbox.empty()) {
      int mean = sum / bbox.size();
      means.push_back(mean);
    } else {
      // If the vector is empty, consider mean as 0
      means.push_back(0);
    }
  }
  int i = 0;
  for (auto mean : means)
    cout << i++ << " " << mean << endl;

  i = 0;

  for (const auto& bbox : fanoutBbox) {
    int Min = INT_MAX;
    int Max = INT_MIN;
    int Var = 0;
    long int sum = 0;
    for (int val : bbox) {
      Min = min(val, Min);
      Max = max(val, Max);
      // cout << "val : " << val << endl;
      sum += val;
    }
    int mean = 0;
    float stdev = 0;

    if (!bbox.empty()) {
      cout << "sum : " << sum << endl;
      cout << "bbox size : " << bbox.size() << endl;

      mean = sum / bbox.size();
    } else {
      Min = 0;
      Max = 0;
    }

    for (int val : bbox)
      Var += pow(val - mean, 2);

    if (!bbox.empty())
      stdev = sqrt(Var / bbox.size());

    cout << i++ << " " << mean << " " << Min << " " << Max << " " << stdev
         << endl;
  }
}

void ArtNetGen::reportLogicDepth()
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();
  dbChip* chip = db->getChip();
  if (chip == nullptr) {
    cout << "dbChip does not exist! (program terminated)" << endl;
    exit(0);
  }

  dbBlock* block = chip->getBlock();

  dbSet<dbInst> insts = block->getInsts();
  dbSet<dbBTerm> bterms = block->getBTerms();

  struct Node
  {
    odb::dbInst* inst;
    odb::dbBTerm* bterm;
    vector<Node*> sources;
    vector<Node*> sinks;

    Node* root;
    int order;

    bool isVisited;

    Node()
    {
      inst = nullptr;
      bterm = nullptr;
      order = -1;
    }

    bool isSequential()
    {
      if (inst == nullptr)
        return false;
      else
        return inst->getMaster()->isSequential();
    }

    bool isPrimaryIn()
    {
      if (bterm == nullptr)
        return false;
      else
        return (bterm->getIoType() == dbIoType::INPUT);
    }

    bool isPrimaryOut()
    {
      if (bterm == nullptr)
        return false;
      else
        return (bterm->getIoType() == dbIoType::OUTPUT);
    }
  };

  vector<Node> nodes;
  nodes.reserve(insts.size() + bterms.size());
  unordered_map<dbInst*, Node*> inst2node;
  unordered_map<dbBTerm*, Node*> bterm2node;

  // nodes에 instance 추가
  for (auto inst : insts) {
    Node node;
    node.inst = inst;
    nodes.push_back(node);
    inst2node[inst] = &nodes.back();
  }

  for (auto source_inst : insts) {
    // dbInst* source_inst = *inst_itr;
    string cur_inst = source_inst->getName();
    dbSet<dbITerm> inst_iterms = source_inst->getITerms();
    for (auto inst_iterm_itr = inst_iterms.begin();
         inst_iterm_itr != inst_iterms.end();
         ++inst_iterm_itr) {
      dbITerm* inst_iterm = *inst_iterm_itr;

      if (inst_iterm->getNet() == NULL)
        continue;

      if (inst_iterm->getSigType() != dbSigType::SIGNAL)
        continue;

      dbNet* net = inst_iterm->getNet();  // inst의 pin 중 signal net
      if (inst_iterm->getIoType() == dbIoType::OUTPUT) {
        // dbSet<dbITerm> net_iterms = net->getITerms();
        // dbSet<dbBTerm> net_bterms = net->getBTerms();

        for (dbITerm* net_iterm : net->getITerms()) {
          if (net_iterm->getIoType() == dbIoType::OUTPUT) {
            dbInst* source_inst = net_iterm->getInst();
            string new_inst = source_inst->getName();
            if (cur_inst != new_inst) {
              cout << "cur_inst : " << cur_inst << "  "
                   << "new_inst : " << new_inst << endl;
            }
          }

          if (net_iterm->getIoType() == dbIoType::INPUT) {
            dbInst* sink_inst = net_iterm->getInst();

            // cout << sink_inst->getName() << endl;
            Node* sink = inst2node[sink_inst];
            Node* source = inst2node[source_inst];
            sink->sources.push_back(source);
            source->sinks.push_back(sink);
          }
        }
        for (dbBTerm* net_bterm : net->getBTerms()) {
          // dbBTerm* net_bterm = *net_bterm_itr;
          if (net_bterm->getIoType() == dbIoType::INPUT) {
            Node* sink = bterm2node[net_bterm];
            Node* source = inst2node[source_inst];
            cout << "here BTERM INPUT" << endl;
            cout << "source_inst : " << source_inst->getName() << endl;
            cout << "sink_term : " << net_bterm->getName() << endl;
            sink->sources.push_back(source);
            source->sinks.push_back(sink);
          }
        }
      }
    }
  }

  queue<Node> Q;
  unordered_map<Node*, int> inDegree;
  unordered_map<int, int> depthDistribution;
  // unordered_map<unordered_map<Node*, Node*>, int> StartEndPair;

  for (int i = 0; i < nodes.size(); i++) {
    Node* node = &nodes[i];

    if (node->isSequential() || node->isPrimaryIn() || node->isPrimaryOut()) {
      inDegree[node] = 0;  // path의 시작 or 끝 의미:
      node->order = 0;
    } else {
      inDegree[node] = node->sources.size();
    }
    // path의 시작점만 넣음, FF/PI가 아니면 topoOrder == 1

    if (node->isSequential() || node->isPrimaryIn()) {
      Q.push(nodes[i]);
    }
  }

  int l = 0;

  std::map<pair<string, string>, int> pairDepths;
  std::map<pair<string, string>, vector<string>> pairPaths;
  vector<vector<string>> path_info;
  while (!Q.empty()) {
    Node n1 = Q.front();
    Q.pop();

    if (n1.isPrimaryOut())
      continue;

    for (Node* n : n1.sinks) {
      Node n2 = *n;

      if (n2.isSequential() || n2.isPrimaryOut() || n2.isPrimaryIn()) {
        if (n2.isPrimaryOut()) {
          continue;
        }
        if (!(n1.isSequential() || n1.isPrimaryIn() || n1.isPrimaryOut())) {
          string endName = n2.inst->getName();  // Name of endpoint

          Node cur_root = n1;

          string startName = "";  // Name of StartPoint

          while (true) {
            if (cur_root.isSequential()) {
              startName = cur_root.inst->getName();  // Name of StartPoint
              break;
            }
            Node* next_root = cur_root.root;
            cur_root = *next_root;
          }
          if (startName != "") {
            if (startName != endName) {
              pair<string, string> pair1 = {startName, endName};
              if (pairDepths.find(pair1) == pairDepths.end()
                  || pairDepths[pair1] < n1.order) {
                pairDepths[pair1] = n1.order;

                cout << startName << " " << endName << " " << n1.order << endl;
                cout << n2.inst->getName() << endl;
                cout << n1.inst->getName() << endl;
                Node cur_root = n1;
                while (true) {
                  if (cur_root.isSequential()) {
                    cout << cur_root.inst->getName() << endl;
                    break;
                  }
                  Node* next_root = cur_root.root;
                  cur_root = *next_root;
                  cout << cur_root.inst->getName() << endl;
                }
              }
            }
          }
        }
      } else {
        n2.order = n1.order + 1;
        Node* n2_root = new Node(n1);
        n2.root = n2_root;
        Q.push(n2);
      }
    }
  }

  for (const auto& pair : pairDepths) {
    cout << pair.first.first << " " << pair.first.second << " " << pair.second
         << endl;
    depthDistribution[pair.second]++;
    auto it = pairPaths.find(pair.first);
    if (it != pairPaths.end()) {
      for (const auto& path : it->second)
        cout << path << endl;
    }
  }

  // return depthDistribution;
  return;
}

/*
unordered_map<Node*, int>
Netlist::checkLoop() {

    queue<Node*> Q;

    unordered_map<Node*, int> inDegree;
    unordered_map<Node*, int> topoOrder;
    unordered_map<Node*, int> empty;

    for(int i=0; i < nodes_.size(); i++) {
        Node* node = nodes_[i];

        topoOrder[node] = -1; //.insert(make_pair(node, -1));

        inDegree[node] = node->numFanins(); break;
                //inDegree.insert(make_pair(node, node->numFanins())); break;
        }

        if(inDegree[node] == 0) {
            topoOrder[node] = 0;
            Q.push(node);
        }
    }

    while(!Q.empty()) {
        Node* n1 = Q.front();
        Q.pop();

        for(Node* n2 : n1->getSinks()) {

            if( inDegree[n2] == 0 ) continue;

            if( --inDegree[n2] == 0 ) {
                topoOrder[n2] = topoOrder[n1] + 1;
                Q.push(n2);
            }
        }
    }

    if (topoOrder.size() == nodes_.size())
        return topoOrder;
    else
        return empty;
}
*/

void ArtNetGen::EdgeBbox(const char* fileName)
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

  dbChip* chip = db->getChip();
  if (chip == nullptr) {
    cout << "dbChip does not exist!" << endl;
    return;
  }

  dbBlock* block = chip->getBlock();

  dbSet<dbInst> insts = block->getInsts();

  vector<int> edge;
  vector<int> bbox;

  long totFanouts = 0;

  for (auto inst_itr = insts.begin(); inst_itr != insts.end(); ++inst_itr) {
    dbInst* inst = *inst_itr;
    dbSet<dbITerm> inst_iterms = inst->getITerms();

    dbMaster* master = inst->getMaster();

    int bboxSize = 0;
    int edgelen = 0;

    int sourceX, sourceY;
    inst->getLocation(sourceX, sourceY);

    int lx, ly, ux, uy;
    lx = sourceX, ly = sourceY;
    ux = sourceX, uy = sourceY;

    int numFanouts = 0;

    for (auto inst_iterm_itr = inst_iterms.begin();
         inst_iterm_itr != inst_iterms.end();
         ++inst_iterm_itr) {
      dbITerm* inst_iterm = *inst_iterm_itr;

      if (inst_iterm->getNet() == NULL)
        continue;

      if (inst_iterm->getSigType() != dbSigType::SIGNAL)
        continue;

      if (inst_iterm->getIoType() == dbIoType::OUTPUT) {
        dbNet* net = inst_iterm->getNet();

        dbSet<dbITerm> net_iterms = net->getITerms();

        int max_edge = 0;

        for (auto net_iterm_itr = net_iterms.begin();
             net_iterm_itr != net_iterms.end();
             ++net_iterm_itr) {
          dbITerm* net_iterm = *net_iterm_itr;
          dbInst* sink_inst = net_iterm->getInst();

          if (net_iterm->getIoType() == dbIoType::OUTPUT) {
            // for debug
            if (inst_iterm != net_iterm) {
              cout << "invalid output term!" << endl;
              // exit(0);
            }
          }

          if (net_iterm->getIoType() == dbIoType::INPUT) {
            numFanouts++;
            int sinkX, sinkY;
            sink_inst->getLocation(sinkX, sinkY);
            lx = min(lx, sinkX), ly = min(ly, sinkY);
            ux = max(ux, sinkX), uy = max(uy, sinkY);

            edgelen = (abs(sourceX - sinkX) + abs(sourceY - sinkY));
            edge.push_back(edgelen);
            max_edge = max(max_edge, edgelen);
          }
          if (net_iterm->getMTerm() != NULL) {
          }
        }

        bboxSize = (abs(ux - lx) + abs(uy - ly));
        bbox.push_back(bboxSize);
        totFanouts += numFanouts;

        if (max_edge > bboxSize)
          cout << "WRONG!!!!!!!!!!!!" << endl;
      }
    }
  }

  ofstream outFile(fileName);

  if (!outFile.good()) {
    cout << "Invalid output file" << endl;
    return;
  }

  long long sum_edge = 0;
  long long sum_bbox = 0;
  int num_edge = 0;
  int num_bbox = 0;

  for (auto EdgeLen : edge) {
    outFile << "EDGE " << EdgeLen << endl;
    sum_edge += EdgeLen;
    if (EdgeLen != 0)
      num_edge++;
  }
  for (auto Bbox : bbox) {
    outFile << "BBOX " << Bbox << endl;
    sum_bbox += Bbox;
    if (Bbox != 0)
      num_bbox++;
  }
  outFile << "Num. Inst " << insts.size() << endl;
  outFile << "Avg. Fo " << double(totFanouts) / double(insts.size()) << endl;
  outFile << "Avg. EDGE "
          << static_cast<long double>(sum_edge)
                 / static_cast<long double>(num_edge)
          << endl;
  auto max_edge = std::max_element(edge.begin(), edge.end());
  outFile << "Max. EDGE " << *max_edge << endl;
  outFile << "Num. EDGE " << edge.size() << endl;
  outFile << "Avg. BBOX "
          << static_cast<long double>(sum_bbox)
                 / static_cast<long double>(num_bbox)
          << endl;
  auto max_bbox = std::max_element(bbox.begin(), bbox.end());
  outFile << "Max. BBOX " << *max_bbox << endl;
  outFile << "Num. BBOX " << bbox.size() << endl;

  outFile.close();

  return;
}

void ArtNetGen::RealIdeal(float rent_constant,
                          float rent_exponent,
                          int numInsts,
                          const char* fileName)
{
  /*
      int minFanouts = 1;
      int maxFanouts = pow(rent_constant * numInsts* (1 - rent_exponent), (1 /
     (3 - rent_exponent))); float Fo_denominator = 1 - pow(maxFanouts + 1,
     rent_exponent - 1); float Fo_numerator1 = 1 - pow(maxFanouts + 1,
     rent_exponent - 2); float Fo_numerator2 = 0; for (int i = 1; i <=
     maxFanouts; i++) { Fo_numerator1 += pow(i, rent_exponent) / (pow(i, 2) * (i
     + 1));
      }
      float avgFanout = Fo_denominator / (Fo_numerator1 + Fo_numerator2);

      float numNets = 0;
      for (int i = 1; i <= maxFanouts; i++) {
          float Net_denominator = rent_constant * numInsts * (pow(i,
     rent_exponent - 1) - pow(i + 1, rent_exponent - 1)); float Net_numerator =
     i + 1; numNets += (Net_denominator / Net_numerator);
      }

      unordered_map<int, int> fanoutDist_Ideal;
      for(int x = minVal; x <= maxVal; x++) {
          float denominator = rent_constant * numInsts * (pow(x, rent_exponent -
     1) - pow(x+1, rent_exponent - 1)); float numerator = x + 1; int count =
     ceil(denominator / numerator); fanoutDist_Ideal[x] = count;
      }

      numNets = ceil(numNets);
      int numEdges = ceil((avgFanout / (avgFanout + 1)) * rent_constant *
     numInsts * (1 - pow(numInsts, rent_exponent-1)));

      dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

      dbChip* chip = db->getChip();
      if( chip == nullptr ) {
          cout << "dbChip does not exist!" << endl;
          return;
      }

      dbBlock* block = chip->getBlock();

      dbSet<dbInst> insts = block->getInsts();

      vector<int> edge;
      vector<int> bbox;

      long totFanouts = 0;

      for(auto inst_itr = insts.begin(); inst_itr != insts.end(); ++inst_itr) {

          dbInst* inst = *inst_itr;
          dbSet<dbITerm> inst_iterms = inst->getITerms();

          dbMaster* master = inst->getMaster();

          int bboxSize = 0;
          int edgelen = 0;

          int sourceX, sourceY;
          inst->getLocation(sourceX, sourceY);

          int lx, ly, ux, uy;
          lx = sourceX, ly = sourceY;
          ux = sourceX, uy = sourceY;

          int numFanouts = 0;

          for(auto inst_iterm_itr = inst_iterms.begin(); inst_iterm_itr !=
     inst_iterms.end();
                  ++inst_iterm_itr) {
              dbITerm* inst_iterm = *inst_iterm_itr;

              if(inst_iterm->getNet() == NULL)
                  continue;

              if(inst_iterm->getSigType() != dbSigType::SIGNAL)
                  continue;

              if(inst_iterm->getIoType() == dbIoType::OUTPUT) {
                  dbNet* net = inst_iterm->getNet();

                  dbSet<dbITerm>  net_iterms = net->getITerms();

                  int max_edge = 0;

                  for (auto net_iterm_itr = net_iterms.begin(); net_iterm_itr !=
     net_iterms.end(); ++net_iterm_itr) { dbITerm* net_iterm = *net_iterm_itr;
                      dbInst*  sink_inst = net_iterm->getInst();

                      if(net_iterm->getIoType() == dbIoType::OUTPUT) {
                          // for debug
                          if(inst_iterm != net_iterm) {
                              cout << "invalid output term!" << endl;
                              //exit(0);
                          }
                      }

                      if(net_iterm->getIoType() == dbIoType::INPUT) {

                          numFanouts++;
                          int sinkX, sinkY;
                          sink_inst->getLocation(sinkX, sinkY);
                          lx = min(lx, sinkX), ly = min(ly, sinkY);
                          ux = max(ux, sinkX), uy = max(uy, sinkY);

                          edgelen = (abs(sourceX - sinkX) + abs(sourceY -
     sinkY)); edge.push_back(edgelen); max_edge = max(max_edge, edgelen);

                      }
                      if(net_iterm->getMTerm() != NULL) {
                      }
                  }

                  bboxSize = (abs(ux - lx) + abs(uy - ly));
                  bbox.push_back(bboxSize);
                  totFanouts += numFanouts;

                  if (max_edge > bboxSize)
                      cout << "WRONG!!!!!!!!!!!!" << endl;

              }
          }
      }

      ofstream outFile(fileName);

      if(!outFile.good()) {
          cout << "Invalid output file" << endl;
          return;
      }

      long long sum_edge = 0;
      long long sum_bbox = 0;
      int num_edge = 0;
      int num_bbox = 0;

      for (auto EdgeLen : edge) {
          outFile << "EDGE " << EdgeLen << endl;
          sum_edge += EdgeLen;
          if (EdgeLen != 0)
              num_edge++;
      }
      for (auto Bbox : bbox) {
          outFile << "BBOX " << Bbox << endl;
          sum_bbox += Bbox;
          if (Bbox != 0)
              num_bbox++;
      }

      outFile << "Num. Inst " << insts.size() << endl;
      outFile << "Avg. Fo " << double(totFanouts)/double(insts.size()) << endl;
      outFile << "Avg. EDGE " << static_cast<long double>(sum_edge) /
     static_cast<long double>(num_edge) << endl; auto max_edge =
     std::max_element(edge.begin(), edge.end()); outFile << "Max. EDGE " <<
     *max_edge << endl; outFile << "Num. EDGE " << edge.size() << endl; outFile
     << "Avg. BBOX " << static_cast<long double>(sum_bbox) / static_cast<long
     double>(num_bbox) << endl; auto max_bbox = std::max_element(bbox.begin(),
     bbox.end()); outFile << "Max. BBOX " << *max_bbox << endl; outFile << "Num.
     BBOX " << bbox.size() << endl;

      outFile.close();
  */
  return;
}
}  // namespace ang
