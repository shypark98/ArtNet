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

#include "node.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

#include "odb/db.h"

namespace ang {

using std::ceil;
using std::cout;
using std::endl;
using std::find;
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

using namespace odb;

Edge::Edge()
{
}

Edge::~Edge()
{
}
string Edge::getName()
{
  return name_;
}

vector<pair<Node*, string>> Edge::getTerms()
{
  return terms_;
}

void Edge::setName(string name)
{
  name_ = name;
}

void Edge::addTerm(Node* node, string termName)
{
  if (termName == "PIN") {
    terms_.push_back(make_pair(node, node->getName()));
  } else {
    terms_.push_back(make_pair(node, termName));
  }
}

void Edge::print()
{
  cout << "- " << name_ << endl;

  int endlCnt = 0;
  for (int i = 0; i < terms_.size(); i++) {
    Node* node = terms_[i].first;
    string termName = terms_[i].second;

    if (node->getType() == NodeType::Combinational
        || node->getType() == NodeType::Sequential) {
      cout << "  ( " << node->getName() << " " << termName << " )";  // << endl;
    } else {
      cout << "  ( PIN " << termName << " )";  // << endl;
    }

    endlCnt++;

    if (endlCnt == 3) {
      endlCnt = 0;
      cout << endl;
    }
  }

  if (endlCnt != 0)
    cout << endl;
  cout << endl;
}

int Node::numFanins() const
{
  return sources_.size();
}

int Node::numFanouts() const
{
  return sinks_.size();
}

void Node::setName(string name)
{
  name_ = name;
}

string Node::getName()
{
  return name_;
}

void Node::setType(NodeType type)
{
  type_ = type;
}

void Node::setBbox(int lx, int ly, int ux, int uy)
{
  lx_ = lx, ly_ = ly, ux_ = ux, uy_ = uy;
}

void Node::addSource(Node* srcNode)
{
#ifdef DEBUG
  vector<Node*>::iterator it = find(sources_.begin(), sources_.end(), srcNode);
  if (it == sources_.end()) {
    cout << "Source node already exists!" << endl;
    exit(0);
  }
#endif

  sources_.push_back(srcNode);
}

void Node::addSink(Node* sinkNode)
{
  sinks_.push_back(sinkNode);
  int sx = sinkNode->x();
  int sy = sinkNode->y();

  lx_ = min(sx, lx_);
  ly_ = min(sy, ly_);
  ux_ = max(sx, ux_);
  uy_ = max(sy, uy_);
}

void Node::addSources(vector<Node*> sources)
{
  for (auto source : sources) {
    sources_.push_back(source);
    source->addSink(this);
  }

  // sources_.insert(sources_.end(), sources.begin(), sources.end());
}

void Node::addSinks(vector<Node*> sinks)
{
  for (auto sink : sinks) {
    sinks_.push_back(sink);
    sink->addSource(this);
  }
  // sinks_.insert(sinks_.end(), sinks.begin(), sinks.end());
}

void Node::removeAllSinks()
{
  for (auto sink : sinks_)
    sink->removeSource(this);
  sinks_.clear();  // = vector<Node*>();
}

void Node::removeAllSources()
{
  for (auto source : sources_)
    source->removeSink(this);
  sources_.clear();  // = vector<Node*>();
}

vector<Node*> Node::getSources()
{
  return sources_;
}

vector<Node*> Node::getSinks()
{
  return sinks_;
}

NodeType Node::getType()
{
  return type_;
}

Node::Node()
{
}
Node::~Node()
{
  // std::cout << "node is free" << std::endl;
}
void Node::setDbMaster(dbMaster* master)
{
  master_ = master;
}

dbMaster* Node::getDbMaster()
{
  return master_;
}

bool Node::hasConnection(Node* node)
{
  vector<Node*>::iterator it = find(sinks_.begin(), sinks_.end(), node);
  if (it == sinks_.end())
    return false;
  else
    return true;
}

string Node::getTerm(Node* node)
{
  return node2term_[node];
}

void Node::removeSink(Node* target)
{
  vector<Node*>::iterator it = find(sinks_.begin(), sinks_.end(), target);
  if (it != sinks_.end())
    sinks_.erase(it);
}

void Node::removeSource(Node* target)
{
  vector<Node*>::iterator it = find(sources_.begin(), sources_.end(), target);
  if (it != sources_.end())
    sources_.erase(it);
}

bool Node::isPIO() const
{
  switch (type_) {
    case NodeType::PrimaryIn:
      return true;
    case NodeType::PrimaryOut:
      return true;
    case NodeType::ClockIn:
      return true;
    case NodeType::ResetIn:
      return true;
    default:
      return false;
  }
}

bool Node::hasMultiPortConn()
{
  unordered_map<Node*, int> weight;

  for (Node* node : sources_) {
    if (weight.find(node) == weight.end())
      weight[node] = 0;
    weight[node]++;
    if (weight[node] > 1)
      return true;
  }

  return false;
}

};  // namespace ang
