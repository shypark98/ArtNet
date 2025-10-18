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

#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <sstream>
#include <unordered_map>

#include "ang/artNetGen.h"
#include "ord/OpenRoad.hh"

using odb::dbBlock;
using odb::dbBTerm;
using odb::dbChip;
using odb::dbDatabase;
using odb::dbInst;
using odb::dbSet;
using utl::ANG;

namespace ang {

void ArtNetGen::Extract()
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();

  dbChip* chip = db->getChip();

  if (chip == nullptr) {
    cout << "dbChip does not exist!" << endl;
    return;
  }

  dbBlock* block = chip->getBlock();

  unordered_map<int, int> PI_shape;
  unordered_map<int, int> PO_shape;
  unordered_map<int, int> Node_shape;
  unordered_map<int, int> FF_shape;
  unordered_map<int, int> Macro_shape;

  dbSet<dbInst> insts = block->getInsts();  // get instances

  int numInsts = insts.size();
  int numPIs = 0;
  int numPOs = 0;
  int numCombinational = 0;
  int numSequential = 0;
  int numMacros = 0;

  dbSet<dbBTerm> bterms = block->getBTerms();  // get port terminals

  // need to check if the terminal is a signal or not
  for (dbSet<dbBTerm>::iterator bterm_itr = bterms.begin();
       bterm_itr != bterms.end();
       ++bterm_itr) {
    dbBTerm* bterm = *bterm_itr;

    if (bterm->getSigType() == dbSigType::SIGNAL) {
      if (bterm->getIoType() == dbIoType::INPUT)
        numInBTerms++;
      if (bterm->getIoType() == dbIoType::OUTPUT)
        numOutBTerms++;
    }
  }

  for (dbSet<dbInst>::iterator insts.begin(); inst_itr != insts.end();
       ++inst_itr) {
    dbInst* inst = *inst_itr;
    dbSet<dbITerm> inst_iterms = inst->getITerms();
  }
}

}  // namespace ang
