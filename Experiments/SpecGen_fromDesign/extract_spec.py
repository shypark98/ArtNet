import sys
import os
import re
import multiprocessing
import numpy as np
from multiprocessing import Process, Pool
import subprocess as sp
from concurrent.futures import ThreadPoolExecutor

run_dir = '.'
script_dir = f'{run_dir}/ref_scripts'
pdk = 'asap7'
exp = 'asap7'
ref_ArtNet_script = f'{script_dir}/extract_spec_{pdk}.tcl'
ref_RentCon_script = f'{script_dir}/ref_{pdk}.aux'
ref_OpenROAD_script = f'{script_dir}/ref_or_place_{pdk}.tcl'

def join_multiline_entries(text):
    lines = text.splitlines()
    merged_lines = []
    buffer = ""

    for line in lines:
        stripped = line.rstrip()
        if not stripped:
            continue

        if "END" in line:
            if buffer:
                merged_lines.append(buffer.rstrip())
                buffer = ""
            merged_lines.append(line.rstrip())
            continue

        buffer += "" + stripped

        if stripped.endswith(';'):
            merged_lines.append(buffer.rstrip())
            buffer = ""

    if buffer:
        merged_lines.append(buffer.rstrip())

    return '\n'.join(merged_lines)

def process_file(input_path, output_path):
    with open(input_path, 'r', encoding='utf-8') as infile:
        raw_text = infile.read()

    cleaned_text = join_multiline_entries(raw_text)

    with open(output_path, 'w', encoding='utf-8') as outfile:
        outfile.write(cleaned_text)

def gen_ArtNet_script(top_module: str,
                      work_dir: str, 
                      out_spec_dir: str,
                      out_veril_dir: str, 
                      out_script: str):

    inFile = open(ref_ArtNet_script, 'r')
    outFile = open(out_script, 'w')
     
    text = inFile.read()
    text = text.replace("__WORK_DIR__", work_dir)
    text = text.replace("__DESIGN__", top_module)
    text = text.replace("__SPEC_DIR__", out_spec_dir)
    text = text.replace("__VERI_DIR__", out_veril_dir)

    outFile.write(text)
    outFile.close()
    inFile.close()

def gen_RentCon_script(top_module: str,
                       def_file: str, 
                       out_script: str):

    inFile = open(ref_RentCon_script, 'r')
    outFile = open(out_script, 'w')

    text = inFile.read()
    text = text.replace("__DEF_FILE__", def_file)

    outFile.write(text)
    outFile.close()
    inFile.close()

def gen_OpenROAD_script(sub_module:str,
                        def_file: str,
                        netlist_file: str,
                        out_script: str):

    inFile = open(ref_OpenROAD_script, 'r')
    outFile = open(out_script, 'w')

    text = inFile.read()
    text = text.replace('__TOP_MODULE__', sub_module)
    text = text.replace('__DEF_FILE__', def_file)
    text = text.replace('__NETLIST_FILE__', netlist_file)

    outFile.write(text)
    outFile.close()
    inFile.close()

def run_ArtNet(top_module: str):
   
    target_dir = f'{run_dir}/{exp}/{top_module}'
    out_script_dir = f'{target_dir}/scripts'
    out_spec_dir = f'{target_dir}/spec'
    out_veril_dir = f'{target_dir}/netlists'
    out_script = f'{out_script_dir}/Extract_{top_module}.tcl'

    gen_ArtNet_script(top_module,
                      target_dir,
                      out_spec_dir,
                      out_veril_dir,
                      out_script)
    
    cur_cmd = f"{run_dir}/openroad {out_script_dir}/Extract_{top_module}.tcl"
    #process = sp.run(cur_cmd, shell=True, capture_output=True, text=True)
    print(cur_cmd)
    process = sp.run(cur_cmd, shell=True, stdout=sp.PIPE, stderr=sp.PIPE, universal_newlines=True)
    
    if process.returncode != 0:
        print("ArtNet Failed!")
        print("ERROR:", process.stderr)
    else:
        print("ArtNet Success!")
        print("--:", process.stdout)

def run_RentCon(top_module: str, sub_module: str):
    
    target_dir = f'{run_dir}/{exp}/{top_module}'
    def_file = f'{target_dir}/defs/{sub_module}_preRoute.def'
    out_script = f'{target_dir}/scripts/{sub_module}.aux'

    gen_RentCon_script(top_module,
                       def_file, 
                       out_script)
    
    cur_cmd = f"{run_dir}/RentCon.exe -f {out_script} -verb 3_3_3 -noGT -noRS -log {target_dir}/log_dir/{sub_module}_RentCon.log"
    print(cur_cmd)
    #process = sp.run(cur_cmd, shell=True, capture_output=True, text=True)
    process = sp.run(cur_cmd, shell=True, stdout=sp.PIPE, stderr=sp.PIPE, universal_newlines=True)

    if process.returncode != 0:
        print("RentCon Failed!")
        print("ERROR:", process.stderr)
    else:
        print("RentCon Success!")
        print("--:", process.stdout)

def run_OpenROAD_place(top_module: str, 
                       sub_module: str):
    
    target_dir = f'{run_dir}/{exp}/{top_module}'
    def_file = f'{target_dir}/defs/{sub_module}_preRoute.def'
    netlist_file = f'{target_dir}/netlists/{sub_module}.v'
    out_script = f"{target_dir}/scripts/{sub_module}_or_place.tcl"
    gen_OpenROAD_script(sub_module,
                        def_file,
                        netlist_file,
                        out_script)

    run_command = f"{run_dir}/openroad {out_script}"
    print(run_command)
    #process = sp.run(run_command, shell=True, capture_output=True, text=True)
    process = sp.run(run_command, shell=True, stdout=sp.PIPE, stderr=sp.PIPE, universal_newlines=True)

    if process.returncode != 0:
        print("OR place Failed!")
        print("ERROR:", process.stderr)
    else:
        print("OR place Success!")
        print("--:", process.stdout)
    
    process_file(def_file, def_file)
     

def make_directories(top_module: str):
    target_dir = f'{run_dir}/{exp}/{top_module}'
    out_script_dir = f'{target_dir}/scripts'
    out_spec_dir = f'{target_dir}/spec'
    out_veril_dir = f'{target_dir}/netlists'
    out_def_dir = f'{target_dir}/defs'
    log_dir = f'{target_dir}/log_dir'

    os.makedirs(out_script_dir, exist_ok=True)
    os.makedirs(out_spec_dir, exist_ok=True)
    os.makedirs(out_veril_dir, exist_ok=True)
    os.makedirs(out_def_dir, exist_ok=True)
    os.makedirs(log_dir, exist_ok=True)

    src_netlist = f'{run_dir}/ref_netlists/{top_module}.v'
    dst_netlist = os.path.join(target_dir, os.path.basename(src_netlist))
    with open(src_netlist, 'rb') as fsrc, open(dst_netlist, 'wb') as fdst:
        fdst.write(fsrc.read())

def process_sub_module(top_module: str, 
                       sub_module: str):
    run_OpenROAD_place(top_module, sub_module)
    run_RentCon(top_module, sub_module)

def compute_RegionI(final_points: int,
                    max_points: int,
                    total_insts: int):
    
    diff = final_points - max_points
    
    print(f"total instances: {total_insts} and diff: {diff}")
    print(type(total_insts))
    if diff == 0:
        return int(0.9 * total_insts)
    else:
        return int((2 ** diff) * total_insts) 

def fit_values(value_list: list):
    
    value_list = np.array(value_list)
    n = len(value_list)
    if n > 1:
        oldDev = np.std(value_list)
    else:
        print(value_list, len(value_list))
    bestN = n
    bestAvg = np.mean(value_list)

    while n > 1:
        n -= 1
        current_values = value_list[:n]
        
         # Compute new standard deviation
        newDev = np.std(current_values)
        
        if newDev > oldDev:
            break
        else:
            oldDev = newDev
            bestN = n
            bestAvg = np.mean(current_values)

    return round(bestAvg, 3)

def extract_parameters(top_module: str, 
                       sub_module: str):
    
    target_dir = f'{run_dir}/{exp}/{top_module}'
    log_file = f"{target_dir}/log_dir/{sub_module}_RentCon.log"

    with open(log_file, 'r') as file:
        log_data = file.read()
    
    inst_match = re.search(r"Number of actual cell instances: (\d+)", log_data)
    if inst_match:
        total_insts = int(inst_match.group(1))
    else:
        raise ValueError(f"Cannot find total number of cell instances. {sub_module}")

    rent_p_match = re.search(r"Final #points = (\d+), Rent's p = ([\d\.]+), standard deviation of the residuals: ([\d\.]+)", log_data)
    if rent_p_match:
        final_points = int(rent_p_match.group(1))
        rent_p = float(rent_p_match.group(2))
        p_sig = float(rent_p_match.group(3)) 
    else:
        final_points = 1
        rent_p = 0.65
        p_sig = 0.1
        #raise ValueError(f"Cannot find Rent's parameter and its std. dev {sub_module}")
    
    max_points_match = re.search(r"Total # of points : (\d+)", log_data)
    max_points = int(max_points_match.group(1)) - 1 if max_points_match else 2
    
    if max_points == 2:
        final_points = 1

    g_values = []
    gsig_values = []
    points_pattern = re.compile(r"#points =\s*(\d+).*?G:\s*([\d\.]+)\s*Gsig:\s*([\d\.e-]+)")
    for match in points_pattern.finditer(log_data):
        point = int(match.group(1))
        if point <= final_points:
            g_values.append(float(match.group(2)))
            gsig_values.append(float(match.group(3)))
    
    regionI = compute_RegionI(final_points, max_points, total_insts)
    
    g_value = fit_values(g_values)
    gsig_value = fit_values(gsig_values)
    
    return {"rent_p": rent_p,
            "rent_sig": p_sig, 
            "regionI": regionI, 
            "g_value": g_value, 
            "gsig": gsig_value}

def write_parameters(top_module: str, 
                     sub_modules: str):
    
    target_dir = f'{run_dir}/{exp}/{top_module}'
    spec_file = f"{target_dir}/spec/{top_module}.spec"

    module_data = {}
    for sub_module in sub_modules:
        module_data[sub_module] = extract_parameters(top_module, sub_module)
    
    with open(spec_file, "r") as f:
        content = f.read()
    
    def replace_values(match):
        module_name = match.group(2)
        print(module_name)
        if module_name in module_data:
            rent_p = module_data[module_name]["rent_p"]
            rent_sig = module_data[module_name]["rent_sig"]
            regionI = module_data[module_name]["regionI"]
            meanG = module_data[module_name]["g_value"]
            sigmaG = module_data[module_name]["gsig"]
            print(module_name, rent_sig)
            modified_block = match.group(0)
            modified_block = re.sub(r"SIZE\s+" + module_name + r"_RegionI", f"SIZE {regionI}", modified_block)
            modified_block = re.sub(r"meanG\s+" + module_name + r"_meanG", f"meanG {meanG}", modified_block)
            modified_block = re.sub(r"sigmaG\s+" + module_name + r"_sigmaG", f"sigmaG {sigmaG}", modified_block)
            modified_block = re.sub(r"p\s+" + module_name + r"_rent", f"p {rent_p}", modified_block)
            modified_block = re.sub(r"q\s+" + module_name + r"_rentSig", f"q {rent_sig}", modified_block)
            
            return modified_block
        return match.group(0)
    
    pattern = re.compile(r"(MODULE|CIRCUIT)\s+NAME\s+(\S+)(.*?)(?=\n(MODULE|CIRCUIT)|\Z)", re.S)
    updated_content = pattern.sub(replace_values, content)
    
    with open(spec_file, "w") as f:
        f.write(updated_content)

def main():
    
    top_modules = ["nova"]
    
    for top_module in top_modules:
        make_directories(top_module)
        run_ArtNet(top_module)
        out_veril_dir = f'{run_dir}/{exp}/{top_module}/netlists'
        sub_modules = [f[:-2] for f in os.listdir(out_veril_dir) if f.endswith('.v')] 
        
        with ThreadPoolExecutor() as executor:
            executor.map(lambda sub_module: process_sub_module(top_module, sub_module), sub_modules)
        
        write_parameters(top_module, sub_modules)

if __name__ == "__main__":
    main()
