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

using odb::dbIoType;
using odb::dbMaster;
using odb::dbMTerm;
using odb::dbSet;
using odb::dbSigType;

using std::cout;
using std::endl;
using std::iterator;
using std::map;
using std::normal_distribution;
using std::pair;
using std::rand;
using std::shuffle;
using std::string;
using std::unordered_map;
using std::vector;

namespace ang {

// helper functions

void getMTermNames(dbMaster* master,
                   string dir,
                   string mode,
                   vector<string>& names)
{
  dbSet<dbMTerm> terms = master->getMTerms();
  for (auto it = terms.begin(); it != terms.end(); it++) {
    dbMTerm* mterm = *it;
    if (mode == "clock") {
      if (mterm->getSigType() == dbSigType::CLOCK) {
        if (dir == "in") {
          if (mterm->getIoType() == dbIoType::INPUT)
            names.push_back(mterm->getName());
        } else if (dir == "out") {
          if (mterm->getIoType() == dbIoType::OUTPUT)
            names.push_back(mterm->getName());
        } else if (dir == "inout") {
          if (mterm->getIoType() == dbIoType::INOUT)
            names.push_back(mterm->getName());
        } else
          cout << "Error: invalid direction " << dir << endl;
      }
    } else if (mode == "signal") {
      if (mterm->getSigType() == dbSigType::SIGNAL) {
        if (dir == "in") {
          if (mterm->getIoType() == dbIoType::INPUT)
            names.push_back(mterm->getName());
        } else if (dir == "out") {
          if (mterm->getIoType() == dbIoType::OUTPUT)
            names.push_back(mterm->getName());
        } else if (dir == "inout") {
          if (mterm->getIoType() == dbIoType::INOUT)
            names.push_back(mterm->getName());
        } else
          cout << "Error: invalid direction " << dir << endl;
      }

    } else if (mode == "reset") {
      if (mterm->getSigType() == dbSigType::RESET) {
        if (dir == "in") {
          if (mterm->getIoType() == dbIoType::INPUT)
            names.push_back(mterm->getName());
        } else if (dir == "out") {
          if (mterm->getIoType() == dbIoType::OUTPUT)
            names.push_back(mterm->getName());
        } else if (dir == "inout") {
          if (mterm->getIoType() == dbIoType::INOUT)
            names.push_back(mterm->getName());
        } else
          cout << "Error: invalid direction " << dir << endl;
      }

    } else if (mode == "scan") {
      if (mterm->getSigType() == dbSigType::SCAN) {
        if (dir == "in") {
          if (mterm->getIoType() == dbIoType::INPUT)
            names.push_back(mterm->getName());
        } else if (dir == "out") {
          if (mterm->getIoType() == dbIoType::OUTPUT)
            names.push_back(mterm->getName());
        } else if (dir == "inout") {
          if (mterm->getIoType() == dbIoType::INOUT)
            names.push_back(mterm->getName());
        } else
          cout << "Error: invalid direction " << dir << endl;
      }
    } else {
      cout << "Error: invalid mode " << mode << endl;
    }
  }
}

// get port information
// input_signal, output_signal, input_clock, output_clock, input_scan,
// output_scan, input_reset, output_reset return map of port type and count
unordered_map<string, int> getPortInfo(dbMaster* master)
{
  dbSet<dbMTerm> terms = master->getMTerms();

  unordered_map<string, int> ioCount;
  ioCount["input_signal"] = 0;
  ioCount["input_clock"] = 0;
  ioCount["input_scan"] = 0;
  ioCount["input_reset"] = 0;
  ioCount["output_signal"] = 0;
  ioCount["output_clock"] = 0;
  ioCount["output_scan"] = 0;
  ioCount["output_reset"] = 0;
  for (auto it = terms.begin(); it != terms.end(); it++) {
    dbSigType sigType = it->getSigType();
    dbIoType ioType = it->getIoType();
    if (sigType.getValue() == dbSigType::SIGNAL) {
      if (ioType.getValue() == dbIoType::INPUT)
        ioCount["input_signal"]++;
      else if (ioType.getValue() == dbIoType::OUTPUT)
        ioCount["output_signal"]++;
    } else if (sigType.getValue() == dbSigType::CLOCK) {
      if (ioType.getValue() == dbIoType::INPUT)
        ioCount["input_clock"]++;
      else if (ioType.getValue() == dbIoType::OUTPUT)
        ioCount["output_clock"]++;
    } else if (sigType.getValue() == dbSigType::RESET) {
      if (ioType.getValue() == dbIoType::INPUT)
        ioCount["input_reset"]++;
      else if (ioType.getValue() == dbIoType::OUTPUT)
        ioCount["output_reset"]++;
    } else if (sigType.getValue() == dbSigType::SCAN) {
      if (ioType.getValue() == dbIoType::INPUT)
        ioCount["input_scan"]++;
      else if (ioType.getValue() == dbIoType::OUTPUT)
        ioCount["output_scan"]++;
    } else {
    }
  }
  return ioCount;
}

int randomIntInRange(int min, int max)
{
  return int(1.0 * (max - min) * rand() / (RAND_MAX + 1.0)) + min;
}

int randomInt(int mod)
{
  return int(1.0 * rand() * mod / (RAND_MAX + 1.0));
}

double uniformDist()
{
  return double(rand()) / (RAND_MAX + 1.0);
}

double normGaussianDist()
{
  // Polar form of the Box-Muller transformation
  double x1, x2, w, y1;
  static double y2;
  static bool use_last = false;

  if (use_last) {
    use_last = false;
    return y2;
  }

  do {
    x1 = 2.0 * uniformDist() - 1.0;
    x2 = 2.0 * uniformDist() - 1.0;
    w = x1 * x1 + x2 * x2;
  } while (w >= 1.0);

  w = sqrt((-2.0 * log(w)) / w);
  y1 = x1 * w;
  y2 = x2 * w;
  use_last = true;

  return y1;
}

double gaussianDist(double mean, double sigma)
{
  // std::cout << "norm Output " << normGaussianDist() << " " << mean << " " <<
  // sigma << std::endl; std::cout << "gaussian : " << mean + sigma *
  // normGaussianDist() << std::endl;
  return mean + sigma * normGaussianDist();
}

}  // namespace ang
