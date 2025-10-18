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
import numpy as np

def load_distribution(file_path):
    """
    Load cell type distribution from a CSV file.
    The CSV must have two columns: 'cell type' and 'count'.
    Returns a dictionary {cell_type: count}.
    """
    df = pd.read_csv(file_path)
    return dict(zip(df['cell type'], df['count']))

def cosine_similarity(A, B):
    """
    Compute cosine similarity between two numpy vectors A and B.
    """
    dot_product = np.dot(A, B)
    norm_A = np.linalg.norm(A)
    norm_B = np.linalg.norm(B)
    return dot_product / (norm_A * norm_B)

def compute_similarity(target_csv, generated_csv):
    # Load distributions
    target_dist = load_distribution(target_csv)
    generated_dist = load_distribution(generated_csv)
    
    # Ensure both have the same set of cell types
    all_cell_types = sorted(set(target_dist.keys()) | set(generated_dist.keys()))
    
    target_vector = np.array([target_dist.get(ct, 0) for ct in all_cell_types])
    generated_vector = np.array([generated_dist.get(ct, 0) for ct in all_cell_types])
    
    # Compute cosine similarity
    similarity = cosine_similarity(target_vector, generated_vector)
    return similarity, all_cell_types, target_vector, generated_vector

if __name__ == "__main__":
    target_csv = "target.csv"
    generated_csv = "generated.csv"
    
    similarity, cell_types, target_vec, generated_vec = compute_similarity(target_csv, generated_csv)
    
    print("Cell Types:", cell_types)
    print("Target Distribution:", target_vec)
    print("Generated Distribution:", generated_vec)
    print("Cosine Similarity:", similarity)

