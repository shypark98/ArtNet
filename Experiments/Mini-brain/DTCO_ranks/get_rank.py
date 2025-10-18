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

import pandas as pd
from scipy.stats import kendalltau, spearmanr

def read_pdn_order(file_path):
    """
    Read PDN_Config from a CSV file and return it as a list in order.
    Assumes the file has columns: PDN_Config, Total_Power_mW
    """
    df = pd.read_csv(file_path)
    return df["PDN_Config"].tolist()

def compute_rank_correlations(file1, file2):
    """
    Compute Kendall tau and Spearman correlation between PDN_Config orders.
    Only compares the common PDN_Config entries between both files.
    """
    list1 = read_pdn_order(file1)
    list2 = read_pdn_order(file2)

    # Build ranking map from list1
    rank1 = {pdn: i for i, pdn in enumerate(list1)}
    
    # Keep only common configs, preserving list2 order
    common = [pdn for pdn in list2 if pdn in rank1]
    x = [rank1[pdn] for pdn in common]
    y = [list2.index(pdn) for pdn in common]

    # Compute correlations
    kendall_corr, kendall_pval = kendalltau(x, y)
    spearman_corr, spearman_pval = spearmanr(x, y)

    print(f"Kendall tau: {kendall_corr:.3f}, p-value: {kendall_pval:.3g}")
    print(f"Spearman rho: {spearman_corr:.3f}, p-value: {spearman_pval:.3g}")

# Example
# compute_rank_correlations("mini_brain.csv", "big_brain.csv")

if __name__ == '__main__':
    design = "ariane"
    list1 = f"./{design}_summary.csv" 
    list2 = f"./{design}_summary.csv"
    compute_rank_correlations(list1, list2)
