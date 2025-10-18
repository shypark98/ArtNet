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

import sys
import os
import multiprocessing
import subprocess as sp
from multiprocessing import Process, Pool
from datetime import datetime
import itertools
import time
from functools import partial
import re
import pandas as pd

ref_rentcon_script = './ref_asap7.aux'
ref_artnet_script = './ref_artnet.tcl'
ref_or_script = './ref_or_place.tcl'

RUN_DIR = '.'

def gen_spec_script(ref_script,
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

def gen_artnet_script(ref_script,
                   result_script,
                   top_design,
                   result_path):

    with open(ref_script, 'r') as infile:
        with open(result_script, 'w') as outfile:
            text = infile.read()
            text = text.replace("__TOP_MODULE__", top_design)
            text = text.replace("__RESULT_PATH__", result_path)
            outfile.write(text)

def gen_or_script(top_module,
                  mode,
                  const,
                  iteration,
                  REF_SCRIPT, 
                  CUR_SCRIPT_DIR):

    inFile = open(REF_SCRIPT, 'r')
    outFile = open(CUR_SCRIPT_DIR+'/'+ top_module + '.tcl', 'w')
    
    def_dir = f"{RUN_DIR}/defs/{mode}" 
    netlist_dir = f"{RUN_DIR}/netlists/{mode}"

    text = inFile.read()
    text = text.replace('__TOP_MODULE__', top_module)
    text = text.replace('__DEF_DIR__', def_dir)
    text = text.replace('__NETLIST_DIR__', netlist_dir)

    outFile.write(text)
    outFile.close()
    inFile.close()

def gen_rentcon_script(def_dir, script_dir):
    
    inFile = open(ref_rentcon_script, 'r')
    outFile = open(script_dir, 'w')

    text = inFile.read()
    text = text.replace("__DEF_DIR__", def_dir)

    outFile.write(text)
    outFile.close()
    inFile.close()

def run_rentcon(mode=str, const=str, iteration=int):
    
    top_module = f"ArtNet_{mode}_{const}_{iteration}"
    def_dir = f'{RUN_DIR}/defs/{mode}/{top_module}_preRoute.def'
    script_dir = f'./scripts/RentCon/{mode}/{top_module}.aux'
    gen_rentcon_script(def_dir, script_dir)
    cur_cmd = f"./RentCon.exe -f ./scripts/RentCon/{mode}/{top_module}.aux -verb 3_3_3 -noGT -noRS -log ./log_dir/{mode}/{top_module}.log"
    process = sp.run(cur_cmd, shell=True, capture_output=True, text=True)
    
    if process.returncode != 0:
        print("RentCon Failed!")
        print("ERROR:", process.stderr)
    else:
        print("RentCon Success!")
        print("--:", process.stdout)


def extract_rent_parameters(mode=str, const=str, iteration=int):
    top_module = f"ArtNet_{mode}_{const}_{iteration}" 
    log_path = f"./log_dir/{mode}/{top_module}.log"

    with open(log_path, 'r') as file:
        content = file.read()
    
    matches = []

    start_arithmetic_fit = False
    in_type_ii_section = False
    
    for line in content.splitlines():
        if "Start to fit arithmetic Rent's parameter in Region I" in line:
            start_arithmetic_fit = True
            continue

        if start_arithmetic_fit and "-------- Type II Rent's parameter --------" in line:
            in_type_ii_section = True
            continue

        if "Type III Rent's parameter --------" in line:
            in_type_ii_section = False
        
        if "Start to fit geometric Rent's parameter in Region I" in line:
            start_arithmetic_fit = False
            continue

        if in_type_ii_section and start_arithmetic_fit:
            match = re.search(r"#points = (\d+), Rent's p = ([\d.]+), standard deviation of the residuals: ([\d.]+)", line)
            if match:
                matches.append(match.groups())
    

    if matches:
        last_match = matches[-1]
        points = int(last_match[0])
        rent_p = float(last_match[1])
        residual_std = float(last_match[2])
        
        if points == 11:
            region_one = 0.9
        elif points == 10:
            region_one = 0.5
        elif points == 9:
            region_one = 0.25
        elif points == 8:
            region_one = 0.125
        elif points == 7:
            region_one = 0.0625
        elif points == 6:
            region_one = 0.03125
        else:
            region_one = 0.01

        return region_one, rent_p, residual_std
    else:
        print("Cannot find Rent's parameter.")
        exit(1)


def count_numFF(file_path):
    try:
        result = sp.run(['grep', '-o', 'CLK', file_path], capture_output=True, text=True)

        count = len(result.stdout.splitlines())

        return count
    except Exception as e:
        print(f"ERROR: {e}")
        return None

def count_numInputs(file_path):
    try:
        result = sp.run(['grep', '-o', 'input', file_path], capture_output=True, text=True)

        count = len(result.stdout.splitlines())

        return count
    except Exception as e:
        print(f"ERROR: {e}")
        return None 

def count_numOutputs(file_path):
    try:
        result = sp.run(['grep', '-o', 'output', file_path], capture_output=True, text=True)

        count = len(result.stdout.splitlines())

        return count
    except Exception as e:
        print(f"ERROR: {e}")
        return None

def count_numInsts(file_path):
    try:
        result = sp.run(['grep', '-o', 'ASAP7_75t_', file_path], capture_output=True, text=True)

        count = len(result.stdout.splitlines())

        return count
    except Exception as e:
        print(f"ERROR: {e}")
        return None


def get_iter_results(mode=str, const=str, iteration=int):
    
    region_one, p, q = extract_rent_parameters(mode, const, iteration)
    top_module = f"ArtNet_{mode}_{const}_{iteration}"
    
    netlist_path = f"{RUN_DIR}/netlists/{mode}/{top_module}.v"
    
    num_pi = count_numInputs(netlist_path) - 2
    num_po = count_numOutputs(netlist_path)
    num_insts = count_numInsts(netlist_path)
    seq_ratio = float(count_numFF(netlist_path) / num_insts)
    
    return num_insts, num_pi, num_po, seq_ratio, region_one, p, q

def get_last_row_values(csv_path):
    df = pd.read_csv(csv_path)

    if df.empty:
        print("CSV file is empty.")
        return None

    last_row_dict = df.iloc[-1].to_dict()

    num_insts = str(int(last_row_dict.get("num_insts", "")))
    num_pi = str(int(last_row_dict.get("num_pi", "")))
    num_po = str(int(last_row_dict.get("num_po", "")))
    seq_ratio = str(last_row_dict.get("seq_ratio", ""))
    region_one = str(last_row_dict.get("region_one", ""))
    p = str(last_row_dict.get("p", ""))
    q = str(last_row_dict.get("q", ""))

    return num_insts, num_pi, num_po, seq_ratio, region_one, p, q


def create_or_append_csv(csv_path, num_insts, num_pi, num_po, seq_ratio, region_one, p, q):
    columns = ["num_insts", "num_pi", "num_po", "seq_ratio", "region_one", "p", "q"]
    
    new_row = {
        "num_insts": num_insts,
        "num_pi": num_pi,
        "num_po": num_po,
        "seq_ratio": seq_ratio,
        "region_one": region_one,
        "p": p,
        "q": q
    }

    if not os.path.exists(csv_path):
        print("Create CSV file.")
        df = pd.DataFrame([new_row], columns=columns)
        df.to_csv(csv_path, index=False)
        print(f"New CSV path: {csv_path}")
    else:
        df = pd.read_csv(csv_path)
        new_row_df = pd.DataFrame([new_row], columns=columns)
        df = pd.concat([df, new_row_df], ignore_index=True)
        df.to_csv(csv_path, index=False)
        print(f"Add row to CSV: {csv_path}")


def run_artnet(mode=str, const=str, iteration=int):

    top_module = f"ArtNet_{mode}_{const}_{iteration}"
    result_spec_script = f"{RUN_DIR}/scripts/ArtNet/{mode}/SPEC_{mode}_{const}_{iteration}.tcl"
    result_artnet_script = f"{RUN_DIR}/scripts/ArtNet/{mode}/{top_module}.tcl"

    csv_path = f"{RUN_DIR}/results/{mode}/results_{const}.csv"
    ref_spec_script = f"{RUN_DIR}/run_artnet.tcl"
    ref_artnet_script = f"{RUN_DIR}/ref_artnet.tcl"
    run_directory = f"{RUN_DIR}/scripts/ArtNet/{mode}"

    num_insts = ""
    num_pi = ""
    num_po = ""
    seq_ratio = ""
    avg_fanin = ""
    region_one = ""
    p = ""
    q = ""
    

    if mode == "Rent":
        if iteration == 0:
            p = ""
            num_insts = "9200"
            num_pi = "100"
            num_po = "100"
            seq_ratio = "0.1304"
            avg_fanin = "2.30"
            region_one = "0.45"
            q = "0.1"

            if const == "Min":
                p = "0.3"
            elif const == "Max":
                p = "0.8"
            else:
                print(f"Wrong input const {const}")
                exit(1)

            create_or_append_csv(csv_path, num_insts, num_pi, num_po, seq_ratio, region_one, p, q)
        else:
            num_insts, num_pi, num_po, seq_ratio, region_one, p, q = get_last_row_values(csv_path)
            avg_fanin = "2.30"


    elif mode == "PIPO":
        if iteration == 0:
            num_insts = "9200"
            num_pi = ""
            num_po = ""
            seq_ratio = "0.1304"
            avg_fanin = "2.30"
            region_one = "0.45"
            p = "0.55"
            q = "0.1"

            if const == "Min":
                num_pi = "10"
                num_po = "10"
            elif const == "Max":
                num_pi = "400"
                num_po = "400"
            else:
                print(f"Wrong input const {const}")
                exit(1)
            create_or_append_csv(csv_path, num_insts, num_pi, num_po, seq_ratio, region_one, p, q)
        else:
            num_insts, num_pi, num_po, seq_ratio, region_one, p, q = get_last_row_values(csv_path)
            avg_fanin = "2.30"


    elif mode == "Sratio":
        if iteration == 0:
            num_insts = "9200"
            num_pi = "100"
            num_po = "100" 
            seq_ratio = "" 
            avg_fanin = "2.30"
            region_one = "0.45"
            p = "0.55"
            q = "0.10"
            
            if const == "Min":
                seq_ratio = "0.0304"
            elif const == "Max":
                seq_ratio = "0.2701"
            else:
                print(f"Wrong input const {const}")
                exit(1)
            create_or_append_csv(csv_path, num_insts, num_pi, num_po, seq_ratio, region_one, p, q)
        else:
            num_insts, num_pi, num_po, seq_ratio, region_one, p, q = get_last_row_values(csv_path)
            avg_fanin = "2.30"
    
    else:
        print(f"Error: Wrong input {mode}")
        exit(1)


    gen_spec_script(ref_spec_script,
                   result_spec_script,
                   top_module,
                   num_insts,
                   num_pi,
                   num_po,
                   seq_ratio,
                   region_one,
                   p,
                   q,
                   avg_fanin,
                   run_directory)


    print(f"top_module: {top_module}, num_insts: {num_insts}, num_pi: {num_pi}, num_po: {num_po}, seq_ratio: {seq_ratio}")
    print(f"region_one: {region_one}, p: {p}, q: {q}, avg_fanin: {avg_fanin}")
            
    run_command = f"./openroad {result_spec_script}"
    print(run_command)
    process = sp.run(run_command, shell=True, capture_output=True, text=True)

    if process.returncode != 0:
        print("SpecGen Failed!")
        print("ERROR:", process.stderr)
    else:
        print("SPecGen Success!")
        print("--:", process.stdout) 

    gen_artnet_script(ref_artnet_script,
                   result_artnet_script,
                   top_module,
                   run_directory) 

    run_command = f"./openroad {result_artnet_script}"
    print(run_command)
    process = sp.run(run_command, shell=True, capture_output=True, text=True)
    
    if process.returncode != 0:
        print("ArtNet Failed!")
        print("ERROR:", process.stderr)
    else:
        print("ArtNet Success!")
        print("--:", process.stdout)

    run_command = f"mv {top_module}.v {RUN_DIR}/netlists/{mode}"
    print(run_command)
    process = sp.run(run_command, shell=True, capture_output=True, text=True) 
    if process.returncode != 0:
        print("move Netlist Failed!")
        print("ERROR:", process.stderr)
    else:
        print("move Netlist Success!")
        print("--:", process.stdout)

def run_or_place(mode=str, const=str, iteration=int):
    
    top_module = f"ArtNet_{mode}_{const}_{iteration}"
    
    result_script_dir = f"{RUN_DIR}/or_scripts/{mode}" 
    gen_or_script(top_module,
                  mode,
                  const,
                  iteration,
                  ref_or_script,
                  result_script_dir)

    run_command = f"./openroad {result_script_dir}/{top_module}.tcl"
    print(run_command)
    process = sp.run(run_command, shell=True, capture_output=True, text=True)
    
    if process.returncode != 0:
        print("OR place Failed!")
        print("ERROR:", process.stderr)
    else:
        print("OR place Success!")
        print("--:", process.stdout)

def break_point(mode=str, const=str, iteration=int):
    
    top_module = f"ArtNet_{mode}_{const}_{iteration}"
    cur_iter_netlist = f"{RUN_DIR}/netlists/{mode}/{top_module}.v"
    csv_path = f"{RUN_DIR}/results/{mode}/results_{const}.csv" 

    if not os.path.exists(csv_path):
        print(f"CSV file {csv_path} does not exist.")
        return
    
    df = pd.read_csv(csv_path)
    
    num_insts, num_pi, num_po, seq_ratio, region_one, p, q = get_iter_results(mode, const, iteration) 

    new_row = {
        "num_insts": num_insts,
        "num_pi": num_pi,
        "num_po": num_po,
        "seq_ratio": seq_ratio,
        "region_one": region_one,
        "p": p,
        "q": q
    }
    
    errors = {}
    for col in new_row:
        if col == "region_one":
            continue
        if col in df.columns:
            prev_value = df.iloc[-1][col]
            errors[col] = float(abs(prev_value - new_row[col]) / prev_value)
        else:
            errors[col] = None
    
    is_break = True
    
    threshold = 0.01

    for col, error in errors.items():
        if error is not None and error >= threshold:
            print(f"{col}'s error is above {threshold}: {error}")
            is_break = False
    
    if is_break:
        print("Converged")
    else:
        print("Not converged. Adding new row to CSV.")
        new_row_df = pd.DataFrame([new_row], columns=df.columns)
        df = pd.concat([df, new_row_df], ignore_index=True)
        df.to_csv(csv_path, index=False)
        print("Added to CSV.")

    return is_break;

def main():
    
    iteration = 0
    #mode = "Rent"
    #mode = "PIPO"
    mode = "Sratio"
    #const = "Max"
    const = "Min"

    while(1):
        run_artnet(mode, const, iteration);
        run_or_place(mode, const, iteration)
        run_rentcon(mode, const, iteration);   
        is_break = break_point(mode, const, iteration)

        if is_break:
            exit(1)
        else:
            iteration += 1
        
        if iteration > 50:
            exit(1)

if __name__ == "__main__":
    main()

