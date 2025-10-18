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

def execute_command(command, timeout=10800) :
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

def run_multiprocess(commands, num_threads=60, timeout=10800):
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
            text = text.replace("__NUM_INSTS__", num_insts)
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

    run_directory = ""
    ref_script = ""

    top_design = "ArtNet"
    num_insts = [str(num) for num in range(10000, 200001, 10000)] #100
    num_pi = ["100"]
    num_po = ["100"]
    seq_ratio = ["0.05", "0.10", "0.15", "0.20", "0.25", "0.30"] #8
    avg_fanin = ["2.30"] #3
    region_one = ["0.45"] #2
    p = ["0.35", "0.45", "0.50", "0.55", "0.60", "0.65", "0.70"] #7
    q = ["0.10"] #7
    run_commands = []

    # top_modules = ["test1"]
    # for top_module in top_modules :

    for num_insts_ in num_insts:
        for num_pi_ in num_pi:
            for num_po_ in num_po:
                for seq_ratio_ in seq_ratio:
                    for region_one_ in region_one:
                        for p_ in p:
                            for q_ in q:
                                for avg_fanin_ in avg_fanin:
                                    if int(num_insts_) < 100000: 
                                        top_module = f"{num_insts_[:2]}_{avg_fanin_[-2:]}_{seq_ratio_[-2:]}_{region_one_[-2:]}_{p_[-2:]}_{q_[-2:]}"
                                    else:
                                        top_module = f"{num_insts_[:3]}_{avg_fanin_[-2:]}_{seq_ratio_[-2:]}_{region_one_[-2:]}_{p_[-2:]}_{q_[-2:]}"
                                        
                                    top_directory = run_directory + f"/{top_module}"
                                    create_directory(top_directory)
                                    
                                    result_script_directory = top_directory + f"/ang_script"
                                    create_directory(result_script_directory)
                                    
                                    result_script = result_script_directory + f"/run_artnet.tcl"
                                    
                                    top_design = f"ArtNet_{top_module}"
                                     
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

                                    run_command = f"../openroad {result_script}"
                                    run_commands.append(run_command)
                                    top_module = str()
    run_multiprocess(run_commands)

#-------------------------------------------------------------------------------#

if __name__ == "__main__":
   main()
