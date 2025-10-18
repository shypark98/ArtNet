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
import re
import csv
import math

log_dir = "./log_dir"
output_csv = "summary_times.csv"

patterns = {
    "Forrest Time": re.compile(r"-- Priority Queue Initialization Finished \(([\d\.]+) sec\)"),
    "Building Time": re.compile(r"-- Hierarchical Clustering Finished \(([\d\.]+) sec\)"),
    "Circuit Time": re.compile(r"-- Circuit generation Finished \(([\d\.]+) sec\)")
}

def extract_number_from_filename(filename):
    match = re.search(r'ArtNet_(\d+)_', filename)
    if match:
        return int(match.group(1)) * 10000
    else:
        return None

def ceil_2_decimals(x):
    return math.ceil(x * 100) / 100

rows = []

for fname in os.listdir(log_dir):
    if fname.startswith("ArtNet_") and fname.endswith(".log"):
        path = os.path.join(log_dir, fname)
        with open(path, "r") as f:
            content = f.read()

        number = extract_number_from_filename(fname)
        if number is None:
            continue

        times = {}
        for key, pattern in patterns.items():
            m = pattern.search(content)
            if m:
                times[key] = float(m.group(1))
            else:
                times[key] = None

        if all(t is not None for t in times.values()):
            for key in times:
                times[key] = ceil_2_decimals(times[key])
            total_time = sum(times.values())
            total_time = ceil_2_decimals(total_time)

with open(output_csv, "w", newline='') as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Number", "Forrest Time", "Building Time", "Circuit Time", "Total Time"])
    writer.writerows(rows)

print(f"Summary saved to {output_csv}")
