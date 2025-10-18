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
#include "cell.h"
#include "circuit.h"
#include "delay_level.h"

// #include "TritonPart.h"
// #include "Utilities.h"
#include "db_sta/dbNetwork.hh"
#include "db_sta/dbSta.hh"
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

using std::cout;
using std::endl;
using std::find;
using std::getline;
using std::ifstream;
using std::map;
using std::max;
using std::min;
using std::ofstream;
using std::pair;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::vector;

using odb::dbBlock;
using odb::dbBTerm;
using odb::dbChip;
using odb::dbDatabase;
using odb::dbInst;
using odb::dbIoType;
using odb::dbITerm;
using odb::dbModInst;
using odb::dbMTerm;
using odb::dbNet;
using odb::dbSet;
using odb::dbSigType;
using odb::dbLib;
using odb::dbMaster;

using sta::LibertyCell;

using utl::ANG;

namespace ang {

void getInSigCnt(int& minInSigCnt, int& maxInSigCnt)
{
  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();
  dbSet<dbLib> libs = db->getLibs();

  for (auto itl = libs.begin(); itl != libs.end(); itl++) {
    dbSet<dbMaster> masters = itl->getMasters();

    for (auto itm = masters.begin(); itm != masters.end(); itm++) {
      string masterName = itm->getName();

      dbMaster* master = db->findMaster(masterName.c_str());

      dbSet<dbMTerm> terms = master->getMTerms();

      int inSigCnt = 0;
      int inClkCnt = 0;
      int inRstCnt = 0;
      int inScanCnt = 0;
      int outSigCnt = 0;
      int outClkCnt = 0;
      int outRstCnt = 0;
      int outScanCnt = 0;

      for (auto it = terms.begin(); it != terms.end(); it++) {
        string termName = it->getName();

        dbSigType sigType = it->getSigType();
        dbIoType ioType = it->getIoType();

        if (sigType.getValue() == dbSigType::SIGNAL) {
          if (ioType.getValue() == dbIoType::INPUT) {
            inSigCnt++;
          } else if (ioType.getValue() == dbIoType::OUTPUT) {
            outSigCnt++;
          }

        } else if (sigType.getValue() == dbSigType::CLOCK) {
          if (ioType.getValue() == dbIoType::INPUT) {
            inClkCnt++;
          } else if (ioType.getValue() == dbIoType::OUTPUT) {
            outClkCnt++;
          }

        } else if (sigType.getValue() == dbSigType::RESET) {
          if (ioType.getValue() == dbIoType::INPUT) {
            inRstCnt++;
          } else if (ioType.getValue() == dbIoType::OUTPUT) {
            outRstCnt++;
          }

        } else if (sigType.getValue() == dbSigType::SCAN) {
          if (ioType.getValue() == dbIoType::INPUT) {
            inScanCnt++;
          } else if (ioType.getValue() == dbIoType::OUTPUT) {
            outScanCnt++;
          }

        } else {
        }
      }

      if (inSigCnt > 0) {
        maxInSigCnt = max(maxInSigCnt, inSigCnt);
        minInSigCnt = min(minInSigCnt, inSigCnt);
      }
    }
  }
}

unordered_map<int, int> makeFiDistribution(int minVal,
                                           int maxVal,
                                           float avgVal,
                                           int numInsts,
                                           int seed)
{
  unordered_map<int, int> samples;
  std::poisson_distribution<int> distribution(avgVal);
  std::default_random_engine generator;
  generator.seed(seed);

  for (int i = 0; i < numInsts; ++i) {
    int fanin = distribution(generator);

    if (fanin < minVal || fanin > maxVal) {
      fanin = std::uniform_int_distribution<int>(minVal, maxVal)(generator);
    }
    samples[fanin]++;
  }

  return samples;
}

unordered_map<int, int> makeFoDistribution(int minVal,
                                           int maxVal,
                                           float rent_constant,
                                           float rent_exponent,
                                           int numInsts)
{
  unordered_map<int, int> samples;
  for (int x = minVal; x <= maxVal; x++) {
    float denominator
        = rent_constant * numInsts
          * (pow(x, rent_exponent - 1) - pow(x + 1, rent_exponent - 1));

    float numerator = x + 1;
    int count = ceil(denominator / numerator);
    samples[x] = count;
  }

  return samples;
}

void ArtNetGen::createSpec(const char* topModule,
                           int numInsts,
                           int numMacros,
                           int numPIs,
                           int numPOs,
                           float region1,
                           float avgFanin,
                           float meanG,
                           float sigmaG,
                           float p,
                           float q,
                           float seqRatio,
                           int seed,
                           const char* onlyUseList,
                           const char* specFile)
{
  cout << "Creating spec file: " << specFile << endl;

  dbDatabase* db = ord::OpenRoad::openRoad()->getDb();
  int numStds = numInsts - numMacros;
  typedef pair<int, float> IntFloatPair;
  unordered_map<string, IntFloatPair> onlyUseMasters;
  unordered_map<string, IntFloatPair> onlyUseMacros;

  string topModuleName = topModule;
  int numInBTerms = numPIs;
  int numOutBTerms = numPOs;

  int minFanin = INT_MAX;
  int minFanouts = INT_MAX;
  int maxFanin = 0;
  getInSigCnt(minFanin, maxFanin);

  cout << onlyUseList << endl;
  if (string(onlyUseList) != "") {
    ifstream inFile(onlyUseList);

    if (!inFile.good()) {
      cout << "Error: cannot open file " << onlyUseList << endl;
      exit(0);
    } else {
      cout << "Only_use list is provided" << endl;
      cout << "Generate spec from given list" << endl;

      string line;
      string masterName;
      int count;
      float ratio;
      int totCount = 0;
      float totRatio = 0;
      while (getline(inFile, line)) {
        stringstream ss(line);
        ss >> masterName >> ratio;
        dbMaster* master = db->findMaster(masterName.c_str());
        if (!master) {
          cout << "Error: master " << masterName << " not found" << endl;
          exit(0);
        }

        int cellCnt = int(numStds * ratio);
        onlyUseMasters[masterName].first = cellCnt;
        onlyUseMasters[masterName].second = ratio;
        totCount += count;
        totRatio += ratio;
      }
    }

    float avgInPins = 0;

    for (auto it = onlyUseMasters.begin(); it != onlyUseMasters.end(); it++) {
      dbMaster* master = db->findMaster(it->first.c_str());
      int numInputs = 0;
      getNumInPins(master, numInputs);
      avgInPins += numInputs * it->second.second;
    }

    if (avgInPins != avgFanin) {
      cout << "Warning: average fanin does not match the average fanin of "
              "only_use masters"
           << endl;
      cout << "Warning: average fanin: " << avgFanin
           << " average fanin of only_use masters: " << avgInPins << endl;
      cout << "Warning: adjust average fanin to match the average fanin of "
              "only_use masters"
           << endl;
      avgFanin = avgInPins;
    }

  } else {  // when only_use list is not given --> generate cell list

    cout << "Warning: no only_use list is provided" << endl;
    cout << "Generate random masters" << endl;

    map<int, vector<string>> fi2CombiMaster;
    map<int, vector<string>> fi2SeqMaster;
    unordered_map<std::string, int> masterCountMap;

    dbSet<dbLib> libs = db->getLibs();

    for (auto itl = libs.begin(); itl != libs.end(); itl++) {
      dbSet<dbMaster> masters = itl->getMasters();

      for (auto itm = masters.begin(); itm != masters.end(); itm++) {
        string masterName = itm->getName();

        if (!itm->getType().isBlock()) {
          bool dont_use = false;
          cout << "masterName: " << masterName << " "
               << itm->getType().getString() << ";" << endl;

          for (auto name : dontUses_) {
            cout << name << endl;
            if (name == masterName) {
              dont_use = true;
              break;
            }
          }

          if (dont_use)
            continue;

          dbMaster* master = db->findMaster(masterName.c_str());
          dbSet<dbMTerm> terms = master->getMTerms();

          int inSigCnt = 0;

          for (auto it = terms.begin(); it != terms.end(); it++) {
            dbSigType sigType = it->getSigType();
            dbIoType ioType = it->getIoType();

            if (sigType.getValue() == dbSigType::SIGNAL) {
              if (ioType.getValue() == dbIoType::INPUT) {
                inSigCnt++;
              }
            }
          }

          if (itm->isSequential())
            fi2SeqMaster[inSigCnt].push_back(masterName);
          else
            fi2CombiMaster[inSigCnt].push_back(masterName);
        } else {  // store macro block name
          cout << masterName << "is block" << endl;
          onlyUseMacros[masterName].first = 0;
        }
      }
    }

    // assing random macro
    vector<string> keys;
    for (const auto& pair : onlyUseMacros) {
      keys.push_back(pair.first);
    }

    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<size_t> dist(0, keys.size() - 1);
    for (int i = 0; i < numMacros; ++i) {
      const std::string& random_key = keys[dist(rng)];
      onlyUseMacros[random_key].first++;
    }

    maxFanin = min(maxFanin, 6);

    int numCombi = numStds * (1 - seqRatio);
    int numSeq = numStds - numCombi;

    float combAvgFanin = float(avgFanin * numStds - numSeq) / numCombi;

    unordered_map<int, int> faninDistribution;
    faninDistribution
        = makeFiDistribution(minFanin, maxFanin, combAvgFanin, numCombi, seed);

    for (const auto& fis : faninDistribution) {
      int faninValue = fis.first;  // fanin value
      int requiredCount = fis.second;

      // cout << faninValue << " " << requiredCount << endl;

      auto it = fi2CombiMaster.find(faninValue);
      auto its = fi2SeqMaster.find(faninValue);

      if (it != fi2CombiMaster.end()) {
        const vector<string>& masters = it->second;
        if (masters.empty())
          continue;

        for (int i = 0; i < requiredCount; ++i) {
          int randomIndex = ang::randomIntInRange(0, masters.size() - 1);
          string selectedMaster = masters[randomIndex];
          onlyUseMasters[selectedMaster].first++;
        }
      }

      if (faninValue == 1) {
        if (its != fi2SeqMaster.end()) {
          const vector<string>& masters = its->second;
          if (masters.empty())
            continue;

          for (int i = 0; i < numSeq; ++i) {
            int randomIndex = ang::randomIntInRange(0, masters.size() - 1);
            string selectedMaster = masters[randomIndex];
            onlyUseMasters[selectedMaster].first++;
          }
        }
      }
    }
  }

  ofstream outFile(specFile);

  if (!outFile.good()) {
    cout << "Error: cannot open out file " << specFile << endl;
    exit(0);
  }

  outFile << "LIBRARY" << endl;
  outFile << "NAME " << topModuleName << "_lib" << endl;

  for (auto it = onlyUseMasters.begin(); it != onlyUseMasters.end(); it++) {
    outFile << "ONLY_USE " << it->first << endl;
  }

  for (auto it = onlyUseMacros.begin(); it != onlyUseMacros.end(); it++) {
    outFile << "ONLY_USE " << it->first << endl;
  }

  outFile << endl;
  outFile << "CIRCUIT" << endl;
  outFile << "NAME " << topModuleName << endl;
  outFile << "LIBRARIES " << topModuleName << "_lib" << endl;
  outFile << "DISTRIBUTION ";

  for (auto it = onlyUseMasters.begin(); it != onlyUseMasters.end(); it++) {
    outFile << it->second.first << " ";
  }

  for (auto it = onlyUseMacros.begin(); it != onlyUseMacros.end(); it++) {
    outFile << it->second.first << " ";
  }

  outFile << endl;

  outFile << "SIZE " << int(numInsts * region1) << endl;
  outFile << "p " << p << endl;
  outFile << "q " << q << endl;
  outFile << "meanG " << meanG << endl;
  outFile << "sigmaG " << sigmaG << endl;
  outFile << "END" << endl;

  outFile << "SIZE " << numInsts << endl;
  outFile << "I " << numInBTerms << endl;
  outFile << "O " << numOutBTerms << endl;
  outFile << "END" << endl;
}

void ArtNetGen::readSpec()
{
  // cout << "Reading spec file: " << specFile_ << endl;
  ifstream ifs(specFile_);
  if (!ifs.good()) {
    cout << "Error: cannot open file " << specFile_ << endl;
    exit(0);
  }
  string line;

  string cur_libraryName;
  string cur_circuitName;

  vector<DistUnit> fo_dist;
  vector<DistUnit> depth_dist;

  while (getline(ifs, line)) {
    stringstream ss(line);
    string modeStr;
    ss >> modeStr;

    if (modeStr == "LIBRARY") {
      while (getline(ifs, line)) {
        stringstream ss(line);
        string modeStr;
        ss >> modeStr;

        if (modeStr == "NAME") {
          string name;
          ss >> name;
          if (name.empty()) {
            cout << "Error: library name is empty" << endl;
            exit(0);
          }
          if (cellLists_.find(name) != cellLists_.end()) {
            cout << "Error: library " << name << " already exists" << endl;
            exit(0);
          }
          cur_libraryName = name;

        } else if (modeStr == "ONLY_USE") {
          string masterName;
          ss >> masterName;
          int numInputs = 0;
          int numOutputs = 0;
          bool isSequential;
          dbMaster* master = db_->findMaster(masterName.c_str());
          auto sta_cell = dbNetwork_->dbToSta(master);
          LibertyCell* lib_cell = dbNetwork_->libertyCell(sta_cell);
          if (!master) {
            cout << "Error: master " << masterName << " not found" << endl;
            exit(0);
          } else {
            getNumInOutPins(master, numInputs, numOutputs);
            isSequential = lib_cell->hasSequentials();
          }

          // added later
          if (master->isBlock()) {
            if (std::find(macros_.begin(), macros_.end(), masterName)
                == macros_.end()) {
              macros_.push_back(masterName);
            }
          }

          auto cell = new Librarycell(master->getName(),
                                      numInputs,
                                      numOutputs,
                                      isSequential,
                                      1,
                                      1.0,
                                      master,
                                      lib_cell);

          cellLists_[cur_libraryName].cells.push_back(cell);

          if (flop_.empty() && isSequential
              && (numInputs == 1 && numOutputs == 1)) {
            flopCell_ = new Librarycell(master->getName(),
                                        numInputs,
                                        numOutputs,
                                        isSequential,
                                        1,
                                        1.0,
                                        master,
                                        lib_cell);
          } else {
            if (!flop_.empty()) {
              dbMaster* masterFlop = db_->findMaster(flop_.c_str());
              auto sta_cell = dbNetwork_->dbToSta(masterFlop);
              LibertyCell* lib_cell = dbNetwork_->libertyCell(sta_cell);
              int numFlopInputs = 0;
              int numFlopOutputs = 0;
              getNumInOutPins(masterFlop, numFlopInputs, numFlopOutputs);
              flopCell_ = new Librarycell(masterFlop->getName(),
                                          numFlopInputs,
                                          numFlopOutputs,
                                          1,
                                          1,
                                          1.0,
                                          masterFlop,
                                          lib_cell);
            }
          }

        } else {
          break;
        }
      }  // while getline library
    } else if (modeStr == "MODULE" || modeStr == "CIRCUIT") {
      bool inCircuit = false;

      if (modeStr == "CIRCUIT") {
        inCircuit = true;
      }

      auto block = new Block;
      int numCells = 0;
      int libCells = 0;

      while (getline(ifs, line)) {
        stringstream ss(line);
        string modeStr;
        ss >> modeStr;

        if (modeStr == "NAME") {
          string moduleName;
          ss >> moduleName;
          if (moduleName.empty()) {
            if (modeStr == "MODULE") {
              cout << "Error: module name is empty" << endl;
              exit(0);
            } else if (modeStr == "CIRCUIT") {
              cout << "Circuit name is empty" << endl;
              cout << "Using default name : top" << endl;
              moduleName = "top";
            }
          }

          if (cellLists_.find(moduleName) != cellLists_.end()) {
            cout << "Error: module " << moduleName << " already exists" << endl;
            exit(0);
          }
          block->setName(moduleName);

          if (!inCircuit) {
            cellLists_[moduleName].cells.push_back(block);
          }

        } else if (modeStr == "LIBRARIES") {
          string libName;
          while (ss >> libName) {
            if (cellLists_.find(libName) == cellLists_.end()) {
              cout << "Error: library " << libName << " not found" << endl;
              exit(0);
            }
            block->addCellList(libName);
            libCells += cellLists_[libName].cells.size();
          }
          if (libName.empty()) {
            cout << "Error: no libraries found" << endl;
            exit(0);
          }
        } else if (modeStr == "DISTRIBUTION") {
          int value;
          while (ss >> value) {
            if (value < 0) {
              cout << "Error: Number of master cell is negative" << endl;
              exit(0);
            }
            numCells++;
            block->addDistribution(value);
          }
        } else if (modeStr == "SIZE") {
          Block::Region region;
          int regionBound = -1;
          int region_size;
          ss >> region_size;

          if (ss.fail() || region_size < 0) {
            cout << "Error: region bound is negative" << endl;
            exit(0);
          } else {
            regionBound = region_size;
          }

          while (getline(ifs, line)) {
            stringstream ss(line);
            string modeStr;
            ss >> modeStr;

            if (modeStr == "meanT") {
              double meanT;
              ss >> meanT;
              region.meanT = meanT;
              //        cout << "meanT: " << meanT << endl;
            } else if (modeStr == "sigmaT") {
              double sigmaT;
              ss >> sigmaT;
              region.sigmaT = sigmaT;
              //        cout << "sigmaT: " << sigmaT << endl;
            } else if (modeStr == "p") {
              double p;
              ss >> p;
              region.p = p;
              //        cout << "p: " << p << endl;
            } else if (modeStr == "q") {
              double q;
              ss >> q;
              region.q = q;
              //        cout << "q: " << q << endl;
            } else if (modeStr == "meanG") {
              double meanG;
              ss >> meanG;
              region.meanG = meanG;
              //        cout << "meanG: " << meanG << endl;
            } else if (modeStr == "sigmaG") {
              double sigmaG;
              ss >> sigmaG;
              region.sigmaG = sigmaG;
              //        cout << "sigmaG: " << sigmaG << endl;
            } else if (modeStr == "I") {
              int I;
              ss >> I;
              block->setNumInputs(I);  // for clock, rst
                                       //        cout << "I: " << I << endl;
            } else if (modeStr == "O") {
              int O;
              ss >> O;
              block->setNumOutputs(O);
              //        cout << "O: " << O << endl;
            } else if (modeStr == "END") {
              block->addRegion(regionBound, region);
              break;
            } else {
              cout << "Syntax error " << modeStr << endl;
              exit(0);
            }
          }  // while getline size
        } else if (modeStr == "FANOUT_DIST") {
          int target;
          int value = 0;
          while (ss >> target) {
            if (target < 0) {
              cout << "Error: Fanout distribution is negative" << endl;
              exit(0);
            }
            fo_dist.push_back(DistUnit(value, target, 0, 0.0));
            value++;
          }
        } else if (modeStr == "DEPTH") {
          int target;
          int value = 0;
          while (ss >> target) {
            if (target < 0) {
              cout << "Error: Depth distribution is negative" << endl;
              exit(0);
            }
            depth_dist.push_back(DistUnit(value, target, 0, 0.0));
            value++;
          }
        } else {
          break;
        }

        if (!fo_dist.empty())
          block->initFoDist(fo_dist);

        if (!depth_dist.empty())
          block->initDepthDist(depth_dist);

      }  // while getline module

      if (block->getRegions().empty()) {
        cout << "Error: no region found" << endl;
        exit(0);
      }
      if (numCells != libCells) {
        cout << "Error: number of cells does not match" << endl;
        cout << "Number of cells: " << numCells << endl;
        cout << "Number of library cells: " << libCells << endl;
        exit(0);
      }

      block->checkRegion();

      if (inCircuit) {
        circuit_ = block;
      }

    } else {  // else if modeStr
      cout << "Syntax error " << modeStr << endl;
      exit(0);
    }
  }  // while getline ifs
}  // readSpec

void ArtNetGen::writeSpec_v2(float mini_brain, const char* fileName)
{
  unordered_map<string, int> onlyUseMasters;

  string top_name = block_->getTopModule()->getName();

  dbSet<dbInst> insts = block_->getInsts();
  dbSet<dbBTerm> bterms = block_->getBTerms();  // get port terminals

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

  }  // for insts

  for (const std::string& banned : dontUses_) {
    if (onlyUseMasters.find(banned) != onlyUseMasters.end()) {
      numInsts -= onlyUseMasters[banned];
      onlyUseMasters[banned] = 0;
    }
  }

  ofstream outFile(fileName);

  if (!outFile.good()) {
    cout << "Error: cannot open file " << fileName << endl;
    exit(0);
  }

  outFile << "LIBRARY" << endl;
  outFile << "NAME lib" << endl;
  // unordered_map<string, int> --> cellName / ratio
  for (auto it : onlyUseMasters) {
    outFile << "ONLY_USE " << it.first << " " << 1.0 * it.second / numInsts
            << endl;
  }

  outFile << endl;

  int miniInsts = 0;

  outFile << "CIRCUIT" << endl;
  outFile << "NAME " << top_name << endl;
  outFile << "LIBRARIES lib" << endl;
  outFile << "DISTRIBUTION ";
  for (auto it : onlyUseMasters) {
    if (mini_brain < 1.0) {
      outFile << std::round(it.second * mini_brain) << " ";
      miniInsts += std::round(it.second * mini_brain);
    } else
      outFile << it.second << " ";
  }
  outFile << endl;
  outFile << "SIZE " << top_name << "_RegionI" << endl;
  outFile << "p " << top_name << "_rent" << endl;
  outFile << "q " << top_name << "_rentSig" << endl;
  outFile << "END" << endl;
  if (mini_brain < 1.0)
    outFile << "SIZE " << miniInsts << endl;
  else
    outFile << "SIZE " << numInsts << endl;
  outFile << "I " << std::round(numPIs * mini_brain) << endl;
  outFile << "O " << std::round(numPOs * mini_brain) << endl;
  outFile << "END" << endl;
  outFile.close();
  outFile << endl;

  /*
  auto delay_level = std::make_unique<DelayLevel>(db_, sta_, logger_);
  delay_level->initNodes();
  delay_level->labelPIDelay();
  delay_level->labelFFDelay();

  delay_level->setTopNodes();
  delay_level->labelCombiDelay();
  delay_level->checkConsistency();

  map<int, int> FFShape = delay_level->getFFShape();

  int pathCnt = 0;
  for (auto it : FFShape) {
      pathCnt += it.second;

      cout << "depth : " << it.first << " cnt : " << it.second << endl;
  }
  */
}

void ArtNetGen::BuildTimingPaths(int top_n_, bool minmax)
{
  dbBlock* block_ = db_->getChip()->getBlock();
  sta_->ensureGraph();
  sta_->searchPreamble();  // Make graph and find delays
  sta_->ensureLevelized();
  sta::ExceptionFrom* e_from = nullptr;
  sta::ExceptionThruSeq* e_thrus = nullptr;
  sta::ExceptionTo* e_to = nullptr;
  bool include_unconstrained = false;
  bool get_max = minmax;  // max for setup check, min for hold check
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

  if (get_max) {
    std::sort(path_ends.begin(),
              path_ends.end(),
              [&](const auto& path_end1, const auto& path_end2) {
                sta::PathExpanded expand1(path_end1->path(), sta_);
                sta::PathExpanded expand2(path_end2->path(), sta_);
                return expand1.size() > expand2.size();
              });
  } else {
    std::sort(path_ends.begin(),
              path_ends.end(),
              [&](const auto& path_end1, const auto& path_end2) {
                sta::PathExpanded expand1(path_end1->path(), sta_);
                sta::PathExpanded expand2(path_end2->path(), sta_);
                return expand1.size() < expand2.size();
              });
  }
  // check all the timing paths
  for (auto& path_end : path_ends) {
    auto* path = path_end->path();
    const float slack = path_end->slack(sta_);  // slack information
    const float clock_period = path_end->targetClk(sta_)->period();
    sta::PathExpanded expand(path, sta_);
    expand.path(expand.size() - 1);
    if (get_max) {
      cout << "max depth: " << double(expand.size()) / 2 << endl;
      break;
    } else {
      cout << "min depth: " << double(expand.size()) / 2 << endl;
      break;
    }
  }
}

}  // namespace ang
