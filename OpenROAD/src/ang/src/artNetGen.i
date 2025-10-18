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

%module ArtNetGen 

%{
#include "ord/OpenRoad.hh"
#include "ang/artNetGen.h"

#include <array>
#include <iostream>
#include <memory>
#include <regex>
#include <vector>

namespace ord {

ang::ArtNetGen*
getArtNetGen(); 

odb::dbDatabase*
getDb();

sta::dbSta*
getSta();

utl::Logger*
getLogger();
}

using std::regex;
using ord::getArtNetGen;
using ord::getDb;
using ord::getSta;
using ord::getLogger;
using ang::ArtNetGen;

template <typename Type>
int SwigDouble_As(Tcl_Interp * interp, Tcl_Obj * o, Type * val)
{
  int return_val;
  double temp_val;
  return_val = Tcl_GetDoubleFromObj(interp, o, &temp_val);
  *val = (Type) temp_val;
  return return_val;
}


%}

%import<std_vector.i>
%template(wt_factors) std::vector<float>;

%inline %{

void 
artnetgen_init_cmd()
{
  ArtNetGen* artNetGen= getArtNetGen();
  artNetGen->init(getDb(), getSta(), getLogger());
}

void
artnetgen_clear_cmd() 
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->clear();
}

void
artnetgen_run_cmd() 
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->run();
}

void
artnetgen_set_dont_use_cmd(const char* master_name) {
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setDontUse(master_name);
}

void
artnetgen_set_macro_cmd(const char* master_name) {
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setMacro(master_name);
}

void
artnetgen_write_spec_cmd(const char* file_name, 
                         const char* out_dir, 
                         bool is_flat, 
                         int level, 
                         float mini_brain,
                         int min_insts) {

    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->extractSpec(file_name, 
                           out_dir, 
                           is_flat, 
                           level, 
                           mini_brain,
                           min_insts);
}

void
artnetgen_create_spec_cmd(const char* topModule,
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
                          const char* specFile) {

    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->createSpec(topModule, 
                          numInsts,
                          numMacros,
                          numPIs, 
                          numPOs,
                          region1,
                          avgFanin, 
                          meanG, 
                          sigmaG,
                          p,
                          q,
                          seqRatio,
                          seed,
                          onlyUseList, 
                          specFile);
}

/*
void
artnetgen_set_dont_use_cmd(const char* macro_name) 
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setDontUse(macro_name);
}
*/

void
artnetgen_print_parameters_cmd()
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->printParams();
}

void
artnetgen_print_tree_cmd(bool printForrest)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setPrintForrest(printForrest);
    artNetGen->printTree();
}

/*For writing verilog*/
void
artnetgen_set_netlist_cmd(const char* file_name)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setNetlistFile(file_name);
}

void
artnetgen_set_printDepth_cmd(bool printDepth)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setPrintDepth(printDepth);
}

void
artnetgen_set_clockName_cmd(const char* clock_name)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setClockName(clock_name);
}

void
artnetgen_set_isClock_cmd(bool is_clock)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setIsClock(is_clock);
}

void
artnetgen_set_isReset_cmd(bool is_reset)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setIsReset(is_reset);
}

void
artnetgen_set_netlist_flag_cmd(bool netlist_flag)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setNetlistFlag(netlist_flag);
}

void
artnetgen_set_printNetlistInfo_cmd(bool print_netlist_info)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setPrintNetlistInfo(print_netlist_info);
}


/*For setting parameters*/
void
artnetgen_set_seed_cmd(int seed) 
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setSeed(seed);
}

void
artnetgen_set_verbose_cmd(bool verbose)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setVerbose(verbose);
}

void
artnetgen_set_allowLoops_cmd(bool allowLoops)
{
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->setAllowLoops(allowLoops);
}

void
artnetgen_set_isAscend_cmd(bool isAscend)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setIsAscend(isAscend);
}

void
artnetgen_set_localConnect_cmd(bool localConnect)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setLocalConnect(localConnect);
}

void
artnetgen_set_localCutOff_cmd(int localCutOff)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setLocalCutOff(localCutOff);
}

void
artnetgen_set_sigmaTFactor_cmd(float sigmaTFactor)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setSigmaTFactor(sigmaTFactor);
}

void
artnetgen_set_minSeqBlocks_cmd(int minSeqBlocks)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setMinSeqBlocks(minSeqBlocks);
}

void
artnetgen_set_minInputs_cmd(int minInputs)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setMinInputs(minInputs);
}

void
artnetgen_set_minOutputs_cmd(int minOutputs)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setMinOutputs(minOutputs);
}

void
artnetgen_set_minPathLen_cmd(int minPathLen)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setMinPathLen(minPathLen);
}

void
artnetgen_set_maxPathLen_cmd(int maxPathLen)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setMaxPathLen(maxPathLen);
}


void
artnetgen_set_macroMinPathLen_cmd(int minPathLen)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setMacroMinPathLen(minPathLen);
}

void
artnetgen_set_macroMaxPathLen_cmd(int maxPathLen)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setMacroMaxPathLen(maxPathLen);
}


void
artnetgen_set_pathLenCutOff_cmd(int pathLenCutOff)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setPathLenCutOff(pathLenCutOff);
}

void
artnetgen_set_insertFlop_cmd(bool insertFlop)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setInsertFlop(insertFlop);
}

void
artnetgen_set_flopCutOff_cmd(int flopCutOff)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setFlopCutOff(flopCutOff);
}

void
artnetgen_set_flopDam_cmd(int flopDam)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setFlopDam(flopDam);
}

void
artnetgen_set_distThreshold_cmd(int distThreshold)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setDistThreshold(distThreshold);
}

void
artnetgen_set_distMeanT_cmd(float distMeanT)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setDistMeanT(distMeanT);
}


void
artnetgen_set_distMeanG_cmd(float distMeanG)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setDistMeanG(distMeanG);
}

void
artnetgen_set_distFactor_cmd(float distFactor)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setDistFactor(distFactor);
}

void
artnetgen_set_flopInsertProb_cmd(float flopInsertProb)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setFlopInsertProb(flopInsertProb);
}

void
artnetgen_set_defaultFlop_cmd(const char* flop)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setDefaultFlop(flop);
}

void
artnetgen_set_spec_file_cmd(const char* file) 
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->setSpecFile(file);
}

void artnetgen_write_verilog_cmd(
    const char* port_prefix, const char* module_suffix, const char* file_name)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->writePartitionVerilog(file_name, 
                                   port_prefix, 
                                   module_suffix);
}


/*in test.cpp*/
void
artnetgen_write_gnl_cmd(const char* file_name) {
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->writeGNLfile(file_name);
}

void
artnetgen_hnl2netlist_cmd(const char* inFile,
                          const char* outFile) {
    ArtNetGen* artNetGen = getArtNetGen();
    artNetGen->convHNL2Netlist(inFile, outFile);
}

void
artnetgen_build_timing_path_cmd(int top_n,
                                bool minmax)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->BuildTimingPaths(top_n, minmax);
}

void
artnetgen_get_edge_bbox_cmd(const char* file)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->EdgeBbox(file);
}

void
artnetgen_real_ideal_cmd(float rent_constant, 
                         float rent_exponent, 
                         int numInsts, 
                         const char* file)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->RealIdeal(rent_constant, rent_exponent, numInsts, file);
}

void
artnetgen_report_metrics_cmd()
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->reportMetrics();
}

void
artnetgen_report_cell_dist_cmd(const char* file)
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->reportCellDist(file);
}

void
artnetgen_check_floating_cmd()
{
  ArtNetGen* artNetGen = getArtNetGen();
  artNetGen->FloatingCells();
}


%}
