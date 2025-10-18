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

from multiprocessing import Process, Pool
from datetime import datetime
import itertools
import time
from functools import partial
#-------------------------------------------------------------------------------#

def create_directory(directory):
    if not os.path.isdir( directory ) :
        os.makedirs(os.path.join(directory))

#-------------------------------------------------------------------------------#

def execute_command(command, timeout=108000) :
    print(f"Executing: {command}")
    try:
        process = sp.Popen(command, shell=True)
        start_time = time.time()

        while True:
            if process.poll() is not None:
                break
            elapsed_time = time.time() - start_time
            if elapsed_time > timeout:
                process.kill()
                print(f"Process killed after {timeout} seconds for command: {command}")
                break
            time.sleep(10)
    except Exception as e:
        print(f"An error occurred with command {command}: {e}")

#-------------------------------------------------------------------------------#

def run_multiprocess(commands, num_threads=12, timeout=108000):
    try:
        with Pool(processes=num_threads) as pool:
            execute_with_timeout = partial(execute_command, timeout=timeout)
            pool.map(execute_with_timeout, commands)
    except KeyboardInterrupt:
        pool.terminate()
        print("Process pool terminated.")
    except Exception as e:
        print(f"An error occurred: {e}")


#-------------------------------------------------------------------------------#

def generate_ang_script(ref_script,
                        result_script,
                        top_design,
                        num_insts,
                        num_pi,
                        num_po,
                        seq_ratio,
                        region_one,
                        p,
                        q,
                        avg_fanin,
                        result_path):

    with open(ref_script, 'r') as infile:
        with open(result_script, 'w') as outfile:
            text = infile.read()
            text = text.replace("__TOP_MODULE__", top_design)
            text = text.replace("__NUM_INSTS_P1+r6B32=1B4F51\P1+r6B33=1B4F52\P1+r6B34=1B4F53\P1+r6B35=1B5B31357E\P1+r6B36=1B5B31377E\P1+r6B37=1B5B31387E\P1+r6B38=1B5B31397E\P1+r6B39=1B5B32307E\P1+r6B3B=1B5B32317E\P1+r4631=1B5B32337E\_", num_insts)
            text = text.replace("__REGION_ONE__", region_one)
            text = text.replace("__NUM_PI__", num_pi)
            text = text.replace("__NUM_PO__", num_po)
            text = text.replace("__AVG_FANIN__", avg_fanin)
            text = text.replace("__P__", p)
            text = text.replace("__Q__", q)
            text = text.replace("__SEQ_RATIO__", seq_ratio)
            text = text.replace("__RESULT_PATH__", result_path)
            outfile.write(text)

#-------------------------------------------------------------------------------#

def main():
    run_directory = "./run_dir"
    ref_script = "./ref_artnet.tcl"
    log_directory = "./log_dir"

    #create_directory(run_directory)
    #create_directory(log_directory)

    top_design = "ArtNet"
    num_insts = [str(num) for num in range(500000, 100000001, 500000)] #40
    num_pi = ["100"]
    num_po = ["100"]
    seq_ratio = ["0.20"]
    avg_fanin = ["2.30"]
    region_one = ["0.45"]
    p = ["0.45", "0.50", "0.55"]
    q = ["0.10"]
    run_commands = []

    for num_insts_ in num_insts:
        for num_pi_ in num_pi:
            for num_po_ in num_po:
                for seq_ratio_ in seq_ratio:
                    for region_one_ in region_one:
                        for p_ in p:
                            for q_ in q:
                                for avg_fanin_ in avg_fanin:
                                    if int(num_insts_) < 1000000:
                                        top_module = f"{num_insts_[:2]}_{avg_fanin_[-2:]}_{seq_ratio_[-2:]}_{region_one_[-2:]}_{p_[-2:]}_{q_[-2:]}"
                                    elif int(num_insts_) < 10000000:
                                        top_module = f"{num_insts_[:3]}_{avg_fanin_[-2:]}_{seq_ratio_[-2:]}_{region_one_[-2:]}_{p_[-2:]}_{q_[-2:]}"
                                    elif int(num_insts_) < 100000000:
                                        top_module = f"{num_insts_[:4]}_{avg_fanin_[-2:]}_{seq_ratio_[-2:]}_{region_one_[-2:]}_{p_[-2:]}_{q_[-2:]}"
                                    else:
                                        top_module = f"{num_insts_[:5]}_{avg_fanin_[-2:]}_{seq_ratio_[-2:]}_{region_one_[-2:]}_{p_[-2:]}_{q_[-2:]}"

                                    top_directory = os.path.join(run_directory, top_module)
                                    create_directory(top_directory)

                                    result_script_directory = os.path.join(top_directory, "ang_script")
                                    create_directory(result_script_directory)

                                    result_script = os.path.join(result_script_directory, "run_artnet.tcl")

                                    top_design = f"ArtNet_{top_module}"
                                    log_file = os.path.join(log_directory, f"{top_design}.log")
                                    if os.path.exists(log_file):
                                        print(f"[Skip] Log already exists for {top_design}, skipping...")
                                        continue

                                    generate_ang_script(ref_script,
                                                        result_script,
                                                        top_design,
                                                        num_insts_,
                                                        num_pi_,
                                                        num_po_,
                                                        seq_ratio_,
                                                        region_one_,
                                                        p_,
                                                        q_,
                                                        avg_fanin_,
                                                        result_script_directory)

                                    run_command = f"./openroad {result_script} | tee ./log_dir/{top_design}.log"
                                    run_commands.append(run_command)
                                    top_module = str()

    run_multiprocess(run_commands)

#-------------------------------------------------------------------------------#

if __name__ == "__main__":
   main()
