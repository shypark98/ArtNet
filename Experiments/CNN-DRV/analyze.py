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
import glob
import pandas as pd
import numpy as np
from sklearn.preprocessing import StandardScaler
from scipy.spatial.distance import pdist
from sklearn.cluster import KMeans

#CSV file can be extracted from DRV-Prediction model

artnet_files = glob.glob("./CSV_ART_only/*.csv")
ang_files = glob.glob("./CSV_ANG/*.csv")

scaler = StandardScaler()

def analyze_csv(file_paths):
    variances = []
    mean_distances = []
    eigenvalues_list = []

    for file_path in file_paths:
        try:
            data = pd.read_csv(file_path)
            features = data.iloc[:, 2:] 

            if features.empty:
                print(f"Empty features in file: {file_path}")
                continue

            features_scaled = scaler.fit_transform(features)

            variance = np.var(features_scaled, axis=0).mean()

            mean_distance = np.mean(pdist(features_scaled))

            covariance_matrix = np.cov(features_scaled, rowvar=False)
            eigenvalues = np.linalg.eigvalsh(covariance_matrix)
            eigenvalues_list.append(eigenvalues)
            
            print(f"Processed {file_path}: Variance={variance:.4f}, Mean Distance={mean_distance:.4f}, Eigenvalues={eigenvalues}")

            variances.append(variance)
            mean_distances.append(mean_distance)

        except Exception as e:
            print(f"Error processing file {file_path}: {e}")

    return variances, mean_distances, eigenvalues_list

artnet_variances, artnet_mean_distances, artnet_eigenvalues = analyze_csv(artnet_files)
ang_variances, ang_mean_distances, ang_eigenvalues = analyze_csv(ang_files)

summary_df = pd.DataFrame({
    "Metric": ["Variance", "Mean Pairwise Distance"],
    "ArtNet Mean": [
        np.mean(artnet_variances), np.mean(artnet_mean_distances)
    ],
    "ArtNet Std": [
        np.std(artnet_variances), np.std(artnet_mean_distances)
    ],
    "ANG Mean": [
        np.mean(ang_variances), np.mean(ang_mean_distances)
    ],
    "ANG Std": [
        np.std(ang_variances), np.std(ang_mean_distances)
    ]
})

print("Summary Comparison of ArtNet and ANG Datasets:")
print(summary_df.to_string(index=False))

print("\nEigenvalues Statistics:")

artnet_eigenvalues_mean = np.mean([np.mean(e) for e in artnet_eigenvalues])
artnet_eigenvalues_std = np.std([np.std(e) for e in artnet_eigenvalues])

print(f"ArtNet Eigenvalues Mean: {artnet_eigenvalues_mean:.4f}")
print(f"ArtNet Eigenvalues Std: {artnet_eigenvalues_std:.4f}")

ang_eigenvalues_mean = np.mean([np.mean(e) for e in ang_eigenvalues])
ang_eigenvalues_std = np.std([np.std(e) for e in ang_eigenvalues])

print(f"ANG Eigenvalues Mean: {ang_eigenvalues_mean:.4f}")
print(f"ANG Eigenvalues Std: {ang_eigenvalues_std:.4f}")

