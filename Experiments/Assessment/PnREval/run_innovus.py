############################################################################
##
## BSD 3-Clause License
##
## Copyright (c) 2025, Seonghyeon Park and the Regents of the University of California
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
##
## * Neither the name of the copyright holder nor the names of its
##   contributors may be used to endorse or promote products derived from
##   this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##
############################################################################

import os
import sys
import subprocess as sp
import time
import multiprocessing
from multiprocessing import Process, Pool
from datetime import datetime
import itertools
import argparse

# TODO
# User configuration
WORK_DIR = ""
REF_SCRIPT_PATH = "../../innovus_scripts/run_invs.tcl"
BENCH_DIR = ""

#-----------------------------------------------------------------#

def create_sdc( \
        work_dir, \
        top_module, \
        pnr_clk_period, \
        sdc_path ):
    inFile = open(f'{sdc_path}/{top_module}.sdc', 'r')
    outFile = open(f'{sdc_path}/{top_module}_{pnr_clk_period}.sdc', 'w')

    text = inFile.read()
    text = text.replace("__CLK_CYCLE__", "%d" % pnr_clk_period)
    outFile.write(text)
    outFile.close()
    inFile.close()

#-----------------------------------------------------------------#

def create_inv_script( \
        work_dir, \
        top_module, \
        layout_util, \
        aspect_ratio, \
        max_route_layer, \
        pdn_type,\
        pnr_clk_period, \
        script_path ):
    inFile = open(REF_SCRIPT_PATH, 'r')
    outFile = open(script_path, 'w')

    text = inFile.read()
    text = text.replace("__TOP_MODULE__", top_module)   # aes_cipher_top
    text = text.replace("__WORK_HOME__", work_dir)      # ../design
    text = text.replace("__ASPECT_RATIO__", "%.1f" % aspect_ratio)
    text = text.replace("__UTILIZATION__", "%.2f" % layout_util)
    text = text.replace("__RMAX_LYR__", "%d" % max_route_layer)
    text = text.replace("__CLK_PERIOD__", "%d" % pnr_clk_period)
    text = text.replace("__PDN_TYPE__", "%s" % pdn_type)
    text = text.replace("__BENCH_DIR__", work_dir)
    outFile.write(text)
    outFile.close()
    inFile.close()


#-----------------------------------------------------------------#

def create_directory( dirName ):
    try:
        if not(os.path.isdir(dirName)):
            os.makedirs(os.path.join(dirName))
            print( dirName )
    except OSError as e:
        if e.errno != errno.EEXIST:
            print("Failed to create output directory.")
            raise

#-----------------------------------------------------------------#

def execute_cmd( curCmd ):
    try:
        status = curCmd.split(" ")[2]
        cur_time = datetime.now() #.strftime("%m/%d/%H:%M")
        print( "[%s] start P&R : %s" %  (cur_time.strftime("%m/%d/%H:%M"), status))
        print("[INFO] execute command `%s`" % curCmd )
        sp.call( curCmd , shell=True)
        run_time = datetime.now() - cur_time #.strftime("%m/%d/%H:%M")
        print( "[%s] finished P&R : %s (%s sec)" %  (datetime.now().strftime("%m/%d/%H:%M"), status, run_time.seconds))
    
    except KeyboardInterrupt:
        raise KeyboardInterruptError()

#-----------------------------------------------------------------#


def extract( DESIGN_DIR, TOP_MODULE, lu, ar, rm, pdn, cm, SCRIPT_FILE, LOG_FILE ):
    create_inv_script( DESIGN_DIR, TOP_MODULE, lu, ar, rm, pdn, cm, SCRIPT_FILE )
    run_innovus( SCRIPT_FILE, LOG_FILE )

#-----------------------------------------------------------------#

def run_multiprocess( cmd, num_threads=5 ):
    try:
        pool = Pool(processes = num_threads)
        pool.map(execute_cmd, cmd)
        pool.close()
        pool.join()
    except KeyboardInterrupt:
        pool.terminate()
        pool.join()
    
#-----------------------------------------------------------------#
    
def main():
    UTIL = [ 0.30] 
    AR = [ 1.0 ]
    RMAX = [ 7 ]
    CLK = [ 0 ]
    PDN = [ 'sparse' ]
    TOP_MODULES = ["nova"] 
    print(TOP_MODULES)
    print("# of top modules     : %d" % len(TOP_MODULES))
    run_list = []

    for TOP_MODULE in TOP_MODULES:
        DESIGN_DIR = os.path.join(WORK_DIR, TOP_MODULE)
        
        if not os.path.isfile("%s/netlist/%s.v" % (DESIGN_DIR, TOP_MODULE)):
            print(TOP_MODULE)
            continue

        if TOP_MODULE == 'nova':
            CLK = [450, 500, 550, 600, 650, 700, 750, 800] 
            
        for (lu, ar, rm, pdn, cp) in itertools.product(UTIL, AR, RMAX, PDN, CLK):

            TARGET_FLOORPLAN = "util_%.2f_ar_%.1f_rmax_%d_pdn_%s_clk_%d" % (lu, ar, rm, pdn, cp)
            
            SCRIPT_FILE = os.path.join(DESIGN_DIR, "run_inv_%s.tcl" % (TARGET_FLOORPLAN))
            LOG_FILE = os.path.join(DESIGN_DIR, "%s.log" % (TARGET_FLOORPLAN))
            create_inv_script( DESIGN_DIR, TOP_MODULE, lu, ar, rm, pdn, cp, SCRIPT_FILE )
            create_sdc( DESIGN_DIR, TOP_MODULE, cp, f"{DESIGN_DIR}/netlist" )
            cur_cmd = "innovus -init %s -overwrite -no_logv -abort_on_error -log %s &>/dev/null" % (SCRIPT_FILE, LOG_FILE)
            run_list.append(cur_cmd)

    run_multiprocess(run_list)


if __name__ == '__main__':
    main()
