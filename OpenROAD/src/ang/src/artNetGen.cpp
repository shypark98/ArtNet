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

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>

#include "cell.h"
#include "circuit.h"
#include "db_sta/dbReadVerilog.hh"
#include "db_sta/dbSta.hh"
#include "sta/Bfs.hh"
#include "sta/Graph.hh"
#include "sta/Liberty.hh"
#include "sta/LibertyClass.hh"
#include "sta/MakeConcreteNetwork.hh"
#include "sta/ParseBus.hh"
#include "sta/PortDirection.hh"
#include "sta/Sta.hh"
#include "sta/VerilogWriter.hh"
#include "utl/Logger.h"

using odb::dbBlock;
using odb::dbChip;
using odb::dbInst;
using odb::dbIntProperty;
using odb::dbIoType;
using odb::dbLib;
using odb::dbMaster;
using odb::dbMTerm;
using odb::dbSet;
using odb::dbSigType;

using sta::Cell;
using sta::CellPortBitIterator;
using sta::ConcreteNetwork;
using sta::dbNetwork;
using sta::Instance;
using sta::InstancePinIterator;
using sta::isBusName;
using sta::Library;
using sta::Net;
using sta::NetPinIterator;
using sta::NetTermIterator;
using sta::NetworkReader;
using sta::parseBusName;
using sta::Pin;
using sta::Port;
using sta::PortDirection;
using sta::Term;
using sta::writeVerilog;

using std::cout;
using std::endl;
using std::ifstream;
using std::list;
using std::ofstream;
using std::string;
using std::stringstream;
using std::to_string;
using std::unordered_map;
using std::vector;

using utl::ANG;

namespace ang {

std::map<std::string, CellList> ArtNetGen::cellLists_;
bool ArtNetGen::allowLoops_;
bool ArtNetGen::isAscend_;
bool ArtNetGen::verbose_;
bool ArtNetGen::localConnect_;
bool ArtNetGen::insertFlop_;
int ArtNetGen::sigmaTFactor_;
int ArtNetGen::minSeqBlocks_;
int ArtNetGen::localCutOff_;
int ArtNetGen::minInputs_;
int ArtNetGen::minOutputs_;
int ArtNetGen::pathLenCutOff_;
int ArtNetGen::flopCutOff_;
int ArtNetGen::flopDam_;
int ArtNetGen::distThreshold_;
float ArtNetGen::distMeanT_;
float ArtNetGen::distMeanG_;
float ArtNetGen::distFactor_;
float ArtNetGen::flopInsertProb_;
Block* ArtNetGen::circuit_ = nullptr;
Librarycell* ArtNetGen::flopCell_ = nullptr;
DelayDist ArtNetGen::delay_;
string ArtNetGen::netlistFile_;
string ArtNetGen::clockName_;
bool ArtNetGen::isClock_;
bool ArtNetGen::isReset_;
bool ArtNetGen::netlistFlag_ = true;
bool ArtNetGen::printForrest_ = false;
bool ArtNetGen::printNetlistInfo_;
bool ArtNetGen::printDepth_;

int ArtNetGen::moduleCounter_ = 0;
list<ArtNetGen::PtreeNode> ArtNetGen::treeData_;

ArtNetGen::ArtNetGen()
    : db_(nullptr),
      sta_(nullptr),
      dbNetwork_(nullptr),
      graph_(nullptr),
      network_(nullptr),
      logger_(nullptr)
{
}

ArtNetGen::~ArtNetGen()
{
}

void ArtNetGen::init(odb::dbDatabase* db, sta::dbSta* sta, utl::Logger* logger)
{
  db_ = db;
  sta_ = sta;
  dbNetwork_ = sta_->getDbNetwork();
  graph_ = sta_->graph();
  network_ = sta_->network();
  logger_ = logger;
}

void ArtNetGen::clear()
{
  db_ = nullptr;
  sta_ = nullptr;
  dbNetwork_ = nullptr;
  graph_ = nullptr;
  network_ = nullptr;
  logger_ = nullptr;
}

void ArtNetGen::checkLibs()
{
  dbSet<dbLib> libs = db_->getLibs();
  if (libs.empty()) {
    cout << "No libraries found" << endl;
    exit(1);
  }
}

void ArtNetGen::printParams()
{
  cout << "ArtNetGen parameters" << endl;
  cout << endl;
  cout << "*-------------------*" << endl;
  cout << "Seed: " << seed_ << endl;
  cout << "Flop: " << flop_ << endl;
  cout << "Verbose: " << verbose_ << endl;
  cout << "Allow loops: " << allowLoops_ << endl;
  cout << "Local connect: " << localConnect_ << endl;
  cout << "Insert flop: " << insertFlop_ << endl;
  cout << "SigmaT factor: " << sigmaTFactor_ << endl;
  cout << "Min sequential blocks: " << minSeqBlocks_ << endl;
  cout << "Local cut off: " << localCutOff_ << endl;
  cout << "Min inputs: " << minInputs_ << endl;
  cout << "Min outputs: " << minOutputs_ << endl;
  cout << "Max path length: " << maxPathLen_ << endl;
  cout << "Min path length: " << minPathLen_ << endl;
  cout << "Path length cut off: " << pathLenCutOff_ << endl;
  cout << "Flop cut off: " << flopCutOff_ << endl;
  cout << "Dist threshold: " << distThreshold_ << endl;
  cout << "Dist mean T: " << distMeanT_ << endl;
  cout << "Dist mean G: " << distMeanG_ << endl;
  cout << "Dist factor: " << distFactor_ << endl;
  cout << "Flop insert prob: " << flopInsertProb_ << endl;
  cout << "*-------------------*" << endl;
  cout << endl;
  cout << endl;
}

void ArtNetGen::run()
{
  cout << endl;
  cout << "*** ArtNetGen ***" << endl;
  cout << "[Info] Running ArtNetGen" << endl;
  int seed = getSeed();
  checkLibs();
  cout << "[Info] Libraries checked" << endl;
  srand(seed);
  cout << "[Info] Reading spec file: " << specFile_ << endl;
  readSpec();
  labelMacros();
  delay_.setMaxPath(maxPathLen_);
  delay_.setMinPath(minPathLen_);
  delay_.setMacroMaxPath(macroMaxPathLen_);
  delay_.setMacroMinPath(macroMinPathLen_);

  cout << "[Info] Start generating " << circuit_->getName() << endl;
  circuit_->Generate();
  cout << "[Info] Program finished" << endl;
}

/*
void ArtNetGen::generate() {

    circuit_ = std::make_unique<Block>();
    circuit_->setParams();
    circuit_->Generate();

}
*/

void ArtNetGen::labelMacros()
{
  for (auto macroName : macros_) {
    dbMaster* macro = db_->findMaster(macroName.c_str());
    dbIntProperty::create(macro, "macro_id", 1);
  }
}

void ArtNetGen::getNumInPins(odb::dbMaster* master, int& numInpins)
{
  dbSet<dbMTerm> terms = master->getMTerms();
  for (auto it = terms.begin(); it != terms.end(); it++) {
    dbMTerm* mterm = *it;
    if (mterm->getSigType() == dbSigType::SIGNAL) {
      if (mterm->getIoType() == dbIoType::INPUT)
        numInpins++;
    }
  }
}

void ArtNetGen::getNumInOutPins(odb::dbMaster* master,
                                int& numInpins,
                                int& numOutpins)
{
  dbSet<dbMTerm> terms = master->getMTerms();

  for (auto it = terms.begin(); it != terms.end(); it++) {
    dbMTerm* mterm = *it;
    if (mterm->getSigType() == dbSigType::SIGNAL) {
      if (mterm->getIoType() == dbIoType::INPUT)
        numInpins++;
      else if (mterm->getIoType() == dbIoType::OUTPUT)
        numOutpins++;
    }
  }
}

void ArtNetGen::setDefaultFlop(const char* flop)
{
  checkLibs();
  // check if the flop exists in the library
  string flopName = flop;
  dbMaster* master = db_->findMaster(flopName.c_str());
  if (!master) {
    cout << "Error: master " << flopName << " not found" << endl;
    exit(1);
  } else {
    if (master->isSequential()) {
      cout << "Default flop set to " << flopName << endl;
      flop_ = flopName;
    } else {
      cout << "Error: master " << flopName << " is not a flop" << endl;
      exit(1);
    }
  }
}

// determine the required direction of a port.
static PortDirection* determinePortDirection(const Net* net,
                                             const std::set<Instance*>* insts,
                                             const dbNetwork* dbNetwork)
{
  bool local_only = true;
  bool locally_driven = false;
  bool externally_driven = false;

  NetTermIterator* term_iter = dbNetwork->termIterator(net);
  while (term_iter->hasNext()) {
    Term* term = term_iter->next();
    PortDirection* dir = dbNetwork->direction(dbNetwork->pin(term));
    if (dir->isAnyInput()) {
      externally_driven = true;
    }
    local_only = false;
  }
  delete term_iter;

  if (insts != nullptr) {
    NetPinIterator* pin_iter = dbNetwork->pinIterator(net);
    while (pin_iter->hasNext()) {
      const Pin* pin = pin_iter->next();
      const PortDirection* dir = dbNetwork->direction(pin);
      Instance* inst = dbNetwork->instance(pin);

      if (insts->find(inst) == insts->end()) {
        local_only = false;
        if (dir->isAnyOutput()) {
          externally_driven = true;
        }
      } else {
        if (dir->isAnyOutput()) {
          locally_driven = true;
        }
      }
    }
    delete pin_iter;
  }

  // no port is needed
  if (local_only) {
    return nullptr;
  }

  if (locally_driven && externally_driven) {
    return PortDirection::bidirect();
  }

  if (externally_driven) {
    return PortDirection::input();
  }

  return PortDirection::output();
}

// find the correct brackets used in the liberty libraries.
static void determineLibraryBrackets(const dbNetwork* dbNetwork,
                                     char* left,
                                     char* right)
{
  *left = '[';
  *right = ']';

  sta::LibertyLibraryIterator* lib_iter = dbNetwork->libertyLibraryIterator();
  while (lib_iter->hasNext()) {
    const sta::LibertyLibrary* lib = lib_iter->next();
    *left = lib->busBrktLeft();
    *right = lib->busBrktRight();
  }
  delete lib_iter;
}

// Builds an instance/cell of a partition
Instance* ArtNetGen::buildPartitionedInstance(const char* name,
                                              const char* port_prefix,
                                              sta::Library* library,
                                              sta::NetworkReader* network,
                                              sta::Instance* parent,
                                              const std::set<Instance*>* insts,
                                              std::map<Net*, Port*>* port_map)
{
  // build cell
  Cell* cell = network->makeCell(library, name, false, nullptr);

  // add global ports
  auto pin_iter = dbNetwork_->pinIterator(dbNetwork_->topInstance());
  while (pin_iter->hasNext()) {
    const Pin* pin = pin_iter->next();

    bool add_port = false;
    Net* net = dbNetwork_->net(dbNetwork_->term(pin));
    if (net) {
      NetPinIterator* net_pin_iter = dbNetwork_->pinIterator(net);
      while (net_pin_iter->hasNext()) {
        // check if port is connected to instance in this partition
        if (insts->find(dbNetwork_->instance(net_pin_iter->next()))
            != insts->end()) {
          add_port = true;
          break;
        }
      }
      delete net_pin_iter;
    }

    if (add_port) {
      const char* portname = dbNetwork_->name(pin);

      Port* port = network->makePort(cell, portname);
      // copy exactly the parent port direction
      network->setDirection(port, dbNetwork_->direction(pin));
      PortDirection* sub_module_dir
          = determinePortDirection(net, insts, dbNetwork_);
      if (sub_module_dir != nullptr) {
        network->setDirection(port, sub_module_dir);
      }

      port_map->insert({net, port});
    }
  }
  delete pin_iter;

  // make internal ports for partitions and if port is not needed.
  std::set<Net*> local_nets;
  for (Instance* inst : *insts) {
    InstancePinIterator* pin_iter = dbNetwork_->pinIterator(inst);
    while (pin_iter->hasNext()) {
      Net* net = dbNetwork_->net(pin_iter->next());
      if (net != nullptr &&                          // connected
          port_map->find(net) == port_map->end() &&  // port not present
          local_nets.find(net) == local_nets.end()) {
        // check if connected to anything in a different partition
        NetPinIterator* net_pin_iter = dbNetwork_->pinIterator(net);
        while (net_pin_iter->hasNext()) {
          Net* net = dbNetwork_->net(net_pin_iter->next());
          PortDirection* port_dir
              = determinePortDirection(net, insts, dbNetwork_);
          if (port_dir == nullptr) {
            local_nets.insert(net);
            continue;
          }
          string port_name = port_prefix;
          port_name += dbNetwork_->name(net);

          Port* port = network->makePort(cell, port_name.c_str());
          network->setDirection(port, port_dir);

          port_map->insert({net, port});
          break;
        }
        delete net_pin_iter;
      }
    }
    delete pin_iter;
  }

  // loop over buses and to ensure all bit ports are created, only needed for
  // partitioned modules
  char path_escape = dbNetwork_->pathEscape();
  char left_bracket;
  char right_bracket;
  determineLibraryBrackets(dbNetwork_, &left_bracket, &right_bracket);
  std::map<string, vector<Port*>> port_buses;
  for (auto& [net, port] : *port_map) {
    string portname = network->name(port);

    // check if bus and get name
    if (isBusName(portname.c_str(), left_bracket, right_bracket, path_escape)) {
      string bus_name;
      bool is_bus;
      int idx;
      parseBusName(portname.c_str(),
                   left_bracket,
                   right_bracket,
                   path_escape,
                   is_bus,
                   bus_name,
                   idx);

      portname = bus_name;
      port_buses[portname].push_back(port);
    }
  }
  for (auto& [bus, ports] : port_buses) {
    std::set<int> port_idx;
    std::set<PortDirection*> port_dirs;
    for (Port* port : ports) {
      string bus_name;
      bool is_bus;
      int idx;
      parseBusName(network->name(port),
                   left_bracket,
                   right_bracket,
                   path_escape,
                   is_bus,
                   bus_name,
                   idx);

      port_idx.insert(idx);
      port_dirs.insert(network->direction(port));
    }

    // determine real direction of port
    PortDirection* overall_direction = nullptr;
    if (port_dirs.size() == 1) {  // only one direction is used.
      overall_direction = *port_dirs.begin();
    } else {
      overall_direction = PortDirection::bidirect();
    }

    // set port direction to match
    for (Port* port : ports) {
      network->setDirection(port, overall_direction);
    }

    // fill in missing ports in bus
    const auto [min_idx, max_idx]
        = std::minmax_element(port_idx.begin(), port_idx.end());
    for (int idx = *min_idx; idx <= *max_idx; idx++) {
      if (port_idx.find(idx) == port_idx.end()) {
        // build missing port
        string portname = bus;
        portname += left_bracket + std::to_string(idx) + right_bracket;
        Port* port = network->makePort(cell, portname.c_str());
        network->setDirection(port, overall_direction);
      }
    }
  }

  network->groupBusPorts(cell, [](const char*) { return true; });

  // build instance
  string instname = name;
  instname += "_inst";
  Instance* inst = network->makeInstance(cell, instname.c_str(), parent);

  // create nets for ports in cell
  for (auto& [db_net, port] : *port_map) {
    Net* net = network->makeNet(network->name(port), inst);
    Pin* pin = network->makePin(inst, port, nullptr);
    network->makeTerm(pin, net);
  }

  // create and connect instances
  for (Instance* instance : *insts) {
    Instance* leaf_inst = network->makeInstance(
        dbNetwork_->cell(instance), dbNetwork_->name(instance), inst);

    InstancePinIterator* pin_iter = dbNetwork_->pinIterator(instance);
    while (pin_iter->hasNext()) {
      Pin* pin = pin_iter->next();
      Net* net = dbNetwork_->net(pin);
      if (net != nullptr) {  // connected
        Port* port = dbNetwork_->port(pin);

        // check if connected to a port
        auto port_find = port_map->find(net);
        if (port_find != port_map->end()) {
          Net* new_net
              = network->findNet(inst, network->name(port_find->second));
          network->connect(leaf_inst, port, new_net);
        } else {
          Net* new_net = network->findNet(inst, dbNetwork_->name(net));
          if (new_net == nullptr) {
            new_net = network->makeNet(dbNetwork_->name(net), inst);
          }
          network->connect(leaf_inst, port, new_net);
        }
      }
    }
    delete pin_iter;
  }

  return inst;
}

// Builds an instance/cell instantiating the partitions
Instance* ArtNetGen::buildPartitionedTopInstance(const char* name,
                                                 sta::Library* library,
                                                 sta::NetworkReader* network)
{
  // build cell
  Cell* cell = network->makeCell(library, name, false, nullptr);

  // add global ports
  auto pin_iter = dbNetwork_->pinIterator(dbNetwork_->topInstance());
  while (pin_iter->hasNext()) {
    const Pin* pin = pin_iter->next();

    const char* portname = dbNetwork_->name(pin);
    Port* port = network->makePort(cell, portname);
    network->setDirection(port, dbNetwork_->direction(pin));
  }
  delete pin_iter;

  network->groupBusPorts(cell, [](const char*) { return true; });

  // build instance
  string instname = name;
  instname += "_inst";
  Instance* inst = network->makeInstance(cell, instname.c_str(), nullptr);

  CellPortBitIterator* port_iter = network->portBitIterator(cell);
  while (port_iter->hasNext()) {
    Port* port = port_iter->next();
    const char* port_name = network->name(port);
    Net* net = network->makeNet(port_name, inst);
    Pin* pin = network->makePin(inst, port, nullptr);
    network->makeTerm(pin, net);
  }
  delete port_iter;

  return inst;
}

odb::dbBlock* ArtNetGen::getDbBlock() const
{
  odb::dbChip* chip = db_->getChip();
  odb::dbBlock* block = chip->getBlock();
  return block;
}

void ArtNetGen::writeSubmoduleVerilog(const char* port_prefix,
                                      const char* module_suffix)
{
  dbBlock* block = getDbBlock();
  if (block == nullptr) {
    return;
  }

  logger_->report("Writing partition to Verilog.");
  const std::string top_name = dbNetwork_->name(dbNetwork_->topInstance());

  // Partition instance map
  std::map<int, std::set<Instance*>> instance_map;
  for (dbInst* inst : block->getInsts()) {
    dbIntProperty* prop_id = dbIntProperty::find(inst, "partition_id");
    if (!prop_id) {
      logger_->report("Property 'partition_id' not found for inst {}.",
                      inst->getName());
    } else {
      const int partition = prop_id->getValue();
      instance_map[partition].insert(dbNetwork_->dbToSta(inst));
    }
  }

  // Create new network and library
  NetworkReader* network = sta::makeConcreteNetwork();
  Library* library = network->makeLibrary("Partitions", nullptr);

  // New top module
  // Instance* top_inst = buildPartitionedTopInstance(top_name.c_str(), library,
  // network);

  // Build submodule partitions
  std::map<int, Instance*> sta_instance_map;
  std::map<int, std::map<Net*, Port*>> sta_port_map;

  for (auto& [partition, instances] : instance_map) {
    const std::string cell_name
        = top_name + module_suffix + std::to_string(partition);

    NetworkReader* sub_network = sta::makeConcreteNetwork();
    Library* sub_library = sub_network->makeLibrary("Partitions", nullptr);

    Instance* sub_inst = buildPartitionedInstance(cell_name.c_str(),
                                                  port_prefix,
                                                  sub_library,
                                                  sub_network,
                                                  nullptr,  // No top instance
                                                  &instances,
                                                  &sta_port_map[partition]);

    if (!sub_inst)
      logger_->report("Failed to create instance ", cell_name);

    reinterpret_cast<ConcreteNetwork*>(sub_network)->setTopInstance(sub_inst);

    Instance* inst = sub_network->topInstance();  // Instance *inst
    Cell* cell = sub_network->cell(inst);
    sta::CellPortIterator* port_iter = sub_network->portIterator(cell);

    // Save each partition's Verilog file
    std::string submodule_file = cell_name + ".v";

    int subModulePI = 0;
    int subModulePO = 0;

    while (port_iter->hasNext()) {
      Port* port = port_iter->next();
      PortDirection* dir = sub_network->direction(port);
      if (!sub_network->direction(port)->isPowerGround()) {
        // string port_vname = sta::portVerilogName(sub_network->name(port),
        //                                          sub_network->pathEscape());
        // const char *vtype = sta::verilogPortDir(dir);
        if (dir->isAnyInput()) {
          subModulePI += sub_network->size(port);
        } else {
          subModulePO += sub_network->size(port);
        }
      }
    }

    writeVerilog(submodule_file.c_str(), true, false, {}, sub_network);

    delete sub_network;  // Clean up memory

    // Save instance in map
    sta_instance_map[partition] = sub_inst;
  }

  /*
  // connect submodule partitions in new top module
  for (auto& [partition, instance] : sta_instance_map) {
      for (auto& [portnet, port] : sta_port_map[partition]) {
          const char* net_name = network->name(port);
          Net* net = network->findNet(top_inst, net_name);
          if (net == nullptr) {
              net = network->makeNet(net_name, top_inst);
          }
          network->connect(instance, port, net);
      }
  }
  reinterpret_cast<ConcreteNetwork*>(network)->setTopInstance(top_inst);
  writeVerilog(file_name, true, false, {}, network);

  delete network;
  */

  return;
}

void ArtNetGen::writePartitionVerilog(const char* file_name,
                                      const char* port_prefix,
                                      const char* module_suffix)
{
  cout << "here in this function" << endl;
  dbBlock* block = getDbBlock();
  if (block == nullptr) {
    logger_->report("return");
    return;
  }

  logger_->report("Writing partition to verilog.");
  // get top module name
  const string top_name = dbNetwork_->name(dbNetwork_->topInstance());

  // build partition instance map
  std::map<int, std::set<Instance*>> instance_map;
  for (dbInst* inst : block->getInsts()) {
    dbIntProperty* prop_id = dbIntProperty::find(inst, "partition_id");
    if (!prop_id) {
      logger_->warn(ANG,
                    15,
                    "Property 'partition_id' not found for inst {}.",
                    inst->getName());
    } else {
      const int partition = prop_id->getValue();
      instance_map[partition].insert(dbNetwork_->dbToSta(inst));
    }
  }

  // create new network and library
  NetworkReader* network = sta::makeConcreteNetwork();
  Library* library = network->makeLibrary("Partitions", nullptr);

  // new top module
  Instance* top_inst
      = buildPartitionedTopInstance(top_name.c_str(), library, network);

  // build submodule partitions
  std::map<int, Instance*> sta_instance_map;
  std::map<int, std::map<Net*, Port*>> sta_port_map;
  for (auto& [partition, instances] : instance_map) {
    const string cell_name
        = top_name + module_suffix + std::to_string(partition);
    sta_instance_map[partition]
        = buildPartitionedInstance(cell_name.c_str(),
                                   port_prefix,
                                   library,
                                   network,
                                   top_inst,
                                   &instances,
                                   &sta_port_map[partition]);
  }
  // connect submodule partitions in new top module
  for (auto& [partition, instance] : sta_instance_map) {
    for (auto& [portnet, port] : sta_port_map[partition]) {
      const char* net_name = network->name(port);

      Net* net = network->findNet(top_inst, net_name);
      if (net == nullptr) {
        net = network->makeNet(net_name, top_inst);
      }

      network->connect(instance, port, net);
    }
  }

  reinterpret_cast<ConcreteNetwork*>(network)->setTopInstance(top_inst);

  writeVerilog(file_name, true, false, {}, network);

  delete network;
}

/*Partitioning End*/

}  // namespace ang
