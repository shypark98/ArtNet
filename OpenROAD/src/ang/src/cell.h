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

#ifndef __ARTNETGEN_NODE_HEADER__
#define __ARTNETGEN_NODE_HEADER__

#include <algorithm>
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "odb/db.h"
#include "sta/Liberty.hh"

namespace odb {
class dbMaster;
class dbMTerm;
};  // namespace odb

namespace ang {

// library cell or sub block
class CELL
{
 public:
  CELL(int in, int out, int area)
      : numInputs_(in), numOutputs_(out), area_(area)
  {
  }
  CELL(std::string name, int in, int out, int area)
      : name_(name), numInputs_(in), numOutputs_(out), area_(area)
  {
  }
  virtual ~CELL() {}

  // setter
  void setName(std::string name) { name_ = name; }
  void setArea(int area) { area_ = area; }
  void setNumInputs(int numInputs) { numInputs_ = numInputs; }
  void setNumOutputs(int numOutputs) { numOutputs_ = numOutputs; }

  // getter
  std::string getName() { return name_; }
  int getNumInputs() { return numInputs_; }
  int getNumOutputs() { return numOutputs_; }
  int getTerminals() { return numInputs_ + numOutputs_; }
  double getG() { return double(numOutputs_) / (numInputs_ + numOutputs_); }
  int getArea() { return area_; }
  virtual int getNumBlocks() = 0;
  virtual bool isSequential() = 0;
  bool isMacro = false;

 protected:
  std::string name_;
  int numInputs_;
  int numOutputs_;
  int area_;
};

class Librarycell : public CELL
{
 public:
  Librarycell(std::string name,
              int in,
              int out,
              bool seq,
              int area,
              double delay,
              odb::dbMaster* master,
              sta::LibertyCell* lib_cell)
      : CELL(name, in, out, area),
        is_sequential_(seq),
        delay_(delay),
        master_(master),
        lib_cell_(lib_cell)
  {
  }

  virtual bool isSequential() { return is_sequential_; }
  virtual int getNumBlocks() { return 1; }
  double getDelay() const { return delay_; }

  odb::dbMaster* getDbMaster() const { return master_; }
  sta::LibertyCell* getLibCell() const { return lib_cell_; }

 private:
  bool is_sequential_;
  double delay_;
  odb::dbMaster* master_;
  sta::LibertyCell* lib_cell_;
};

class CellList
{
 public:
  std::list<CELL*> cells;
  ~CellList();
};

};  // namespace ang

#endif
