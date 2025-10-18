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

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "ang/artNetGen.h"

namespace odb {
class dbDatabase;
class dbChip;
class dbBlock;
class dbInst;
class dbIterm;
class dbBTerm;
class dbMTerm;
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

enum NodeType
{
  PI,
  PO,
  Combi,
  Seq,
  Macro
};

enum PinType
{
  RST,
  CLK,
  Sig,
};

enum DirType
{
  Input,
  Output,
  InOut
};

class Node
{
 public:
  Node() {}

  odb::dbInst* inst = nullptr;
  odb::dbBTerm* bterm = nullptr;
  std::vector<Node*> sources;
  std::vector<Node*> sinks;
  std::unordered_map<odb::dbIterm*, Node*> terminal;
  std::string getName() const;
  bool is_marked = false;
  int level = -1;

  bool isSequential() const;
  bool isPrimaryIn() const;
  bool isPrimaryOut() const;
};

class DelayLevel
{
 public:
  DelayLevel(odb::dbDatabase* db, sta::dbSta* sta, utl::Logger* logger)
      : db_(db), sta_(sta), logger_(logger)
  {
  }

  void initNodes();
  void calcCombiDelay(Node* node, std::deque<Node*>& candiNodes);

  void getUnmarkedSinks(Node* node, std::deque<Node*>& candiNodes);

  void labelPIDelay();
  void labelFFDelay();
  void labelCombiDelay();
  void setTopNodes();
  void checkConsistency() const;

  std::vector<Node*> getNodes() { return nodes_; }
  std::vector<Node*> getTopNodes() { return top_nodes_; }
  std::vector<Node*> getPIs() { return PIs_; }
  std::vector<Node*> getPOs() { return POs_; }
  std::vector<Node*> getFFs() { return FFs_; }
  std::vector<Node*> getCombis() { return Combis_; }

  std::unordered_map<int, int> getNodeShape();
  std::unordered_map<int, int> getPIShape();
  std::unordered_map<int, int> getPOShape();
  std::map<int, int> getFFShape();

  void clearNodes() { nodes_.clear(); }

 private:
  odb::dbDatabase* db_ = nullptr;
  sta::dbSta* sta_ = nullptr;
  utl::Logger* logger_ = nullptr;

  int max_level_ = 0;

  std::vector<Node*> nodes_;
  //  top nodes are nodes connected to PI or DFF.
  std::vector<Node*> top_nodes_;
  std::vector<Node*> PIs_;
  std::vector<Node*> POs_;
  std::vector<Node*> FFs_;
  std::vector<Node*> Combis_;

  std::unordered_map<odb::dbInst*, Node*> inst2node_;
  std::unordered_map<odb::dbBTerm*, Node*> bterm2node_;
  std::unordered_map<int, int> nodeShape_;
  std::unordered_map<int, int> PIShape_;
  std::unordered_map<int, int> POShape_;
  std::map<int, int> FFShape_;
};

}  // namespace ang
