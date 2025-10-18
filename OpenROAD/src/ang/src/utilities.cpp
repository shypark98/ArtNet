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
// High-level description
// This file includes the basic utility functions for operations
///////////////////////////////////////////////////////////////////////////////
#include "utilities.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <random>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "ang/artNetGen.h"
#include "db_sta/dbSta.hh"
#include "odb/db.h"
#include "sta/MakeConcreteNetwork.hh"
#include "sta/ParseBus.hh"
#include "sta/PortDirection.hh"
#include "sta/VerilogWriter.hh"
#include "utl/Logger.h"

using utl::ANG;

namespace ang {

void ArtNetGen::extractSpec(const char* fileName,
                            const char* out_dir,
                            bool is_flat,
                            int level,
                            float mini_brain,
                            int threshold)
{
  block_ = getDbBlock();
  logger_->report("========================================");
  logger_->report("[STATUS] Starting Extracting Spec");
  logger_->report("========================================");

  logger_->report("[STATUS] Reading hierarchy**** ");
  bool is_hierarchy = BuildLogicalHierarchy();
  logger_->report("[STATUS] Finish reading hierarchy**** ");

  if (is_hierarchy && !is_flat) {
    logger_->report("[STATUS] Start coarsening hierarchy**** ");
    HTreeCoarsening(level);
    logger_->report("Tree vertex count = {}", htree_->GetNumVertices());

    logger_->report("[STATUS] Start writing spec****");
    writeSpec(fileName, level, threshold, mini_brain, out_dir);
    logger_->report("[STATUS] Finish writing spec****");
  } else if (!is_hierarchy || is_flat) {
    logger_->report("[STATUS] Start writing spec****");
    writeSpec_v2(mini_brain, fileName);
    logger_->report("[STATUS] Finish writing spec****");
  }
  /*
  if (is_hierarchy && !is_flat) {
      for (auto node : htree_->GetNodes()) {
          writeNodeVerilog(node, out_dir);
      }
  }*/
}

// read the logical hierarchy
// generate a logical hierarchy tree
// and generate clusters
bool ArtNetGen::BuildLogicalHierarchy()
{
  // Get top module of the block (this is the root node of the hierarchy)
  // block_ = getDbBlock();
  odb::dbModule* top_module = block_->getTopModule();

  // Get all the instances in the top module
  std::vector<odb::dbInst*> insts;

  // Check if submodules exist
  if (top_module->getChildren().empty()) {
    // no logical hierarchy is found
    logger_->report("No logical hierarchy is found");
    return false;
  } else {
    logger_->report("Logical hierarchy is found");
    // create hierarchy nodes for the top module
    std::string module_name = top_module->getName();
    // get submodules of the top module
    odb::dbSet<odb::dbModInst> children = top_module->getChildren();
    // convert dbSet to std::vector
    std::vector<odb::dbModInst*> children_modules;
    for (odb::dbModInst* child : children) {
      children_modules.push_back(child);
    }
    // check if some leaf instances exist here
    odb::dbSet<odb::dbInst> children_insts = top_module->getInsts();
    std::vector<odb::dbInst*> children_insts_vec;
    if (!children_insts.empty()) {
      for (odb::dbInst* inst : children_insts) {
        children_insts_vec.push_back(inst);
      }
    }
    // create a hierarchy node for the top module
    std::shared_ptr<HierarchyNode> top_node = std::make_shared<HierarchyNode>(
        module_name, children_modules, children_insts_vec);
    // First define the hierarchy tree
    htree_ = std::make_shared<HierarchyTree>();
    htree_->SetRoot(top_node);
    // Build the subtree from the root
    BuildSubTrees(top_node);
    // visit all nodes in the tree and assign vertex ids to them
    int tree_vtx_idx = 0;
    std::vector<std::shared_ptr<HierarchyNode>> nodes = htree_->GetNodes();
    for (std::shared_ptr<HierarchyNode> node : nodes) {
      node->SetVertexId(tree_vtx_idx);
      ++tree_vtx_idx;
    }
    return true;
  }
}

void ArtNetGen::BuildSubTrees(HierNodePtr node)
{
  // Assert that node is not nullptr
  assert(node != nullptr);
  // Assert that tree is not nullptr
  assert(htree_ != nullptr);
  // Add the node to the tree
  htree_->AddNode(node);
  // Get the children modules of the current node
  std::vector<odb::dbModInst*> children = node->GetChildrenMods();

  while (true) {
    // Check if the current node has children
    if (children.empty()) {
      break;
    }
    // Iterate over all the children
    for (odb::dbModInst* child : children) {
      // Get the module of the child
      odb::dbModule* child_module = child->getMaster();
      // Get the name of the child module
      std::string child_module_name = child_module->getName();
      // Get the children modules of the child module
      odb::dbSet<odb::dbModInst> child_children = child_module->getChildren();
      // convert dbSet to std::vector
      std::vector<odb::dbModInst*> child_children_modules;
      for (odb::dbModInst* child_child : child_children) {
        child_children_modules.push_back(child_child);
      }
      // check if some leaf instances exist here
      odb::dbSet<odb::dbInst> child_leaf_insts = child_module->getInsts();
      std::vector<odb::dbInst*> child_leaf_insts_vec;
      if (!child_leaf_insts.empty()) {
        for (odb::dbInst* inst : child_leaf_insts) {
          child_leaf_insts_vec.push_back(inst);
        }
      }
      // create a hierarchy node for the child module
      std::shared_ptr<HierarchyNode> child_node
          = std::make_shared<HierarchyNode>(
              child_module_name, child_children_modules, child_leaf_insts_vec);
      // Add the child node to the current node
      node->AddChild(child_node);
      child_node->AddParent(node);
      // Build the subtree from the child node
      BuildSubTrees(child_node);
    }
    break;
  }
}

void ArtNetGen::HTreeCoarsening(int level)
{
  // Assert that tree is not nullptr
  assert(htree_ != nullptr);

  // find how many levels are there in the hierarchy tree
  std::vector<int> levels_of_nodes(htree_->GetNumVertices(), 0);
  std::vector<HierNodePtr> nodes = htree_->GetNodes();
  // get root node
  HierNodePtr node = nodes[0];
  // set level of this node to 0
  levels_of_nodes[node->GetVertexId()] = 0;

  std::vector<HierNodePtr> targetNodes;

  // get hierarchical level of each node
  for (auto node : nodes) {
    int node_id = node->GetVertexId();
    if (node_id == 0) {
      node->SetLevel(0);
      std::vector<HierNodePtr> children = node->GetChildrenNodes();

      continue;
    }

    HierNodePtr parent = node->GetParent();
    // int level_parent = levels_of_nodes[parent->GetVertexId()];
    int level_parent = parent->GetLevel();
    levels_of_nodes[node->GetVertexId()] = level_parent + 1;
    node->SetLevel(level_parent + 1);
  }

  for (auto node : nodes)
    if (node->GetLevel() == level) {
      std::cout << "level: " << level << " node: " << node->GetName()
                << std::endl;
      targetNodes.push_back(node);
    }
  int max_level
      = *std::max_element(levels_of_nodes.begin(), levels_of_nodes.end());
  logger_->report("Max level = {}", max_level);
  htree_->SetMaxLevel(max_level);

  if (level > max_level) {
    logger_->report(
        "Input level is larger than max level ({}); set level to max level",
        max_level);
    return;
  }

  /*
  for (auto node : targetNodes) {
      std::vector<HierNodePtr> sub_tree = htree_->GetSubTree(node);
      std::vector<odb::dbInst*> sub_tree_insts =
  htree_->GetSubTreeInsts(sub_tree); htree_->DeleteTree(node);
      node->AddChildrenInsts(sub_tree_insts);
  }
  */
  /*
  sort(targetNodes.begin(), targetNodes.end(), [](const HierNodePtr& a, const
  HierNodePtr& b) { return a->GetLevel() > b->GetLevel();
  });
  */
}

void ArtNetGen::writeSpec(const char* fileName,
                          int level,
                          int threshold,
                          float mini_brain,
                          const char* out_dir)
{
  // Assert that tree is not nullptr
  assert(htree_ != nullptr);

  block_ = getDbBlock();
  std::unordered_map<std::string, int> onlyUseMasters;
  odb::dbSet<odb::dbInst> insts = block_->getInsts();
  int numInsts = insts.size();
  std::ofstream outFile(fileName);

  if (!outFile) {
    std::cerr << "Error opening file: " << fileName << std::endl;
    return;
  }

  // write entire module lib
  outFile << "LIBRARY" << std::endl;
  outFile << "NAME lib" << std::endl;

  for (auto inst_itr = insts.begin(); inst_itr != insts.end(); ++inst_itr) {
    odb::dbInst* inst = *inst_itr;
    odb::dbMaster* master = inst->getMaster();

    if (onlyUseMasters.find(master->getName()) == onlyUseMasters.end())
      onlyUseMasters[master->getName()] = 0;
    onlyUseMasters[master->getName()]++;
  }

  std::vector<int> topDist(onlyUseMasters.size(), 0);

  int i = 0;
  for (auto it : onlyUseMasters) {
    outFile << "ONLY_USE " << it.first << std::endl;
    topDist[i] = it.second;
    i++;
  }

  std::vector<HierNodePtr> targetNodes = htree_->GetLevelNodes(level);

  std::cout << "level: " << level << std::endl;
  for (auto node : targetNodes)
    std::cout << "node: " << node->GetName()
              << " node level: " << node->GetLevel() << std::endl;

  int total_insts = 0;

  auto writeNodeSpec = [&](HierNodePtr node) {
    int miniInsts = 0;
    std::vector<odb::dbInst*> LeafInsts;
    std::unordered_map<std::string, int> modCount;

    for (auto& it : onlyUseMasters) {
      it.second = 0;
    }

    std::vector<HierNodePtr> subtree = htree_->GetSubTree(node);

    int tot_tree_insts = htree_->GetSubTreeInsts(subtree).size();
    if (tot_tree_insts < threshold) {
      HierNodePtr parent_node = node->GetParent();
      std::vector<odb::dbInst*> sub_tree_insts
          = htree_->GetSubTreeInsts(subtree);
      parent_node->RemoveChild(node);
      parent_node->AddChildrenInsts(sub_tree_insts);

      htree_->DeleteTree(node);
      node->RemoveChildrenInsts();
      return 0;
    }

    if (node->GetLevel() < level) {
      for (auto& child_node : node->GetChildrenNodes()) {
        modCount[child_node->GetName()]++;
        miniInsts += child_node->GetMiniInstances();
      }
      LeafInsts = node->GetChildrenInsts();
    } else {
      LeafInsts = htree_->GetSubTreeInsts(subtree);
    }

    for (auto inst_itr = LeafInsts.begin(); inst_itr != LeafInsts.end();
         ++inst_itr) {
      odb::dbMaster* master = (*inst_itr)->getMaster();
      if (onlyUseMasters.find(master->getName()) == onlyUseMasters.end())
        logger_->report("!!!!!!!!!!!!!!!!!");
      onlyUseMasters[master->getName()]++;
    }

    for (const std::string& banned : dontUses_) {
      if (onlyUseMasters.find(banned) != onlyUseMasters.end()) {
        onlyUseMasters[banned] = 0;
      }
    }

    if (node->GetLevel() != 0) {
      outFile << "" << std::endl;
      outFile << "MODULE" << std::endl;
    } else {
      outFile << "" << std::endl;
      outFile << "CIRCUIT" << std::endl;
    }

    outFile << "NAME " << node->GetName() << std::endl;
    outFile << "LIBRARIES";
    for (const auto& pair : modCount)
      outFile << " " << pair.first;

    outFile << " lib" << std::endl;
    for (const auto& pair : modCount) {
      outFile << "DISTRIBUTION " << pair.second << std::endl;
    }

    outFile << "DISTRIBUTION ";
    for (auto it : onlyUseMasters) {
      if (mini_brain < 1.0) {
        outFile << std::round(it.second * mini_brain) << " ";
        miniInsts += std::round(it.second * mini_brain);
        total_insts += std::round(it.second * mini_brain);
      } else
        outFile << it.second << " ";
    }

    node->SetMiniInstances(miniInsts);
    outFile << std::endl;

    outFile << "SIZE " << node->GetName() << "_RegionI" << std::endl;
    outFile << "p " << node->GetName() << "_rent" << std::endl;
    outFile << "q " << node->GetName() << "_rentSig" << std::endl;
    outFile << "END" << std::endl;

    int numPIs = 0;
    int numPOs = 0;

    if (node->GetLevel() == 0) {  // isTopModule
      getModuleIONum(node, subtree, numPIs, numPOs);
      if (mini_brain < 1.0)
        outFile << "SIZE " << total_insts << std::endl;
      else
        outFile << "SIZE " << numInsts << std::endl;
      outFile << "I " << std::round(numPIs * mini_brain) << std::endl;
      outFile << "O " << std::round(numPOs * mini_brain) << std::endl;
      outFile << "END" << std::endl;
    } else {
      getModuleIONum(node, subtree, numPIs, numPOs);
      if (mini_brain < 1.0)
        outFile << "SIZE " << miniInsts << std::endl;
      else
        outFile << "SIZE " << tot_tree_insts << std::endl;
      outFile << "I " << std::round(numPIs * mini_brain) << std::endl;
      outFile << "O " << std::round(numPOs * mini_brain) << std::endl;
      outFile << "END" << std::endl;
    }
    return 1;
  };

  sort(targetNodes.begin(),
       targetNodes.end(),
       [](const HierNodePtr& a, const HierNodePtr& b) {
         return a->GetLevel() > b->GetLevel();
       });

  for (auto& node : targetNodes) {
    if (writeNodeSpec(node))
      writeNodeVerilog(node, out_dir);
  }
}

void ArtNetGen::getModuleIONum(HierNodePtr node,
                               std::vector<HierNodePtr> subtree,
                               int& numPIs,
                               int& numPOs)
{
  const std::string top_name = node->GetName();

  // Build submodule partitions
  std::map<sta::Net*, sta::Port*> sta_port_map;
  std::set<sta::Instance*> instance_set;
  for (odb::dbInst* inst : htree_->GetSubTreeInsts(subtree))
    instance_set.insert(dbNetwork_->dbToSta(inst));

  // Build submodule partitions
  const std::string subModule_name = node->GetName();

  // Create new network and library
  sta::NetworkReader* sub_network = sta::makeConcreteNetwork();
  sta::Library* sub_library = sub_network->makeLibrary("Partitions", nullptr);

  sta::Instance* sub_inst
      = buildPartitionedInstance(subModule_name.c_str(),
                                 "sub",
                                 sub_library,
                                 sub_network,
                                 nullptr,  // No top instance
                                 &instance_set,
                                 &sta_port_map);

  if (!sub_inst)
    logger_->report("Failed to create instance ", subModule_name);

  reinterpret_cast<sta::ConcreteNetwork*>(sub_network)
      ->setTopInstance(sub_inst);

  // sta::Instance* inst = sub_network->topInstance(); // Instance *inst
  // sta::Cell* cell = sub_network->cell(inst);
  // sta::CellPortIterator *port_iter = sub_network->portIterator(cell);

  for (const auto& [net, port] : sta_port_map) {
    sta::PortDirection* dir = sub_network->direction(port);
    if (!sub_network->direction(port)->isPowerGround()) {
      if (dir->isAnyInput()) {
        if (sub_network->isBus(port))
          std::cout << network_->fromIndex(port) << " "
                    << network_->toIndex(port) << " " << sub_network->size(port)
                    << std::endl;
        numPIs += sub_network->size(port);
      } else {
        if (sub_network->isBus(port))
          std::cout << network_->fromIndex(port) << " "
                    << network_->toIndex(port) << " " << sub_network->size(port)
                    << std::endl;
        numPOs += sub_network->size(port);
      }
    }
  }

  /*
  while (port_iter->hasNext()) {
      sta::Port* port = port_iter->next();
      sta::PortDirection* dir = sub_network->direction(port);
      if (!sub_network->direction(port)->isPowerGround()) {
          if (dir->isAnyInput()) {
              if (sub_network->isBus(port))
                  std::cout << network_->fromIndex(port) << " " <<
  network_->toIndex(port)
                      << " " << sub_network->size(port) << std::endl;
              numPIs += sub_network->size(port);
          } else {
              if (sub_network->isBus(port))
                  std::cout << network_->fromIndex(port) << " " <<
  network_->toIndex(port)
                      << " " << sub_network->size(port) << std::endl;
              numPOs += sub_network->size(port);
          }
      }
  }*/

  delete sub_network;  // Clean up memory
}

void ArtNetGen::writeNodeVerilog(HierNodePtr node, const char* out_dir)
{
  logger_->report("Writing submodule to verilog.");
  const std::string subModule_name = node->GetName();

  // Build submodule partitions
  std::set<sta::Instance*> instance_set;
  std::map<sta::Net*, sta::Port*> sta_port_map;

  std::vector<HierNodePtr> subtree = htree_->GetSubTree(node);
  for (odb::dbInst* inst : htree_->GetSubTreeInsts(subtree)) {
    instance_set.insert(dbNetwork_->dbToSta(inst));
  }

  // Create new network and library
  sta::NetworkReader* sub_network = sta::makeConcreteNetwork();
  sta::Library* sub_library = sub_network->makeLibrary("Partitions", nullptr);

  sta::Instance* sub_inst
      = buildPartitionedInstance(subModule_name.c_str(),
                                 "sub",
                                 sub_library,
                                 sub_network,
                                 nullptr,  // No top instance
                                 &instance_set,
                                 &sta_port_map);
  if (!sub_inst)
    logger_->report("Failed to create instance ", subModule_name);

  reinterpret_cast<sta::ConcreteNetwork*>(sub_network)
      ->setTopInstance(sub_inst);

  std::string verilog_file = std::string(out_dir) + "/" + subModule_name + ".v";
  sta::writeVerilog(verilog_file.c_str(), true, false, {}, sub_network);

  delete sub_network;  // Clean up memory
  return;
}

}  // namespace ang
