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

import matplotlib.pyplot as plt
import pandas as pd
import matplotlib.patches as patches
import matplotlib.colors as mcolors
import numpy as np
import os

# === Identify Layer Keys ===
def get_layer_keys(df):
    return [(col[:-2], col[-1]) for col in df.columns if col.endswith('_u')]

# === Discrete Congestion Plot Function ===
def plot_layer_congestion(df, layer, output_dir):
    fig, ax = plt.subplots(figsize=(10, 10), dpi=200)

    # Use discrete colormap with 10 bins
    levels = np.linspace(0, 1, 11)
    cmap = plt.get_cmap('jet', len(levels) - 1)
    norm = mcolors.BoundaryNorm(boundaries=levels, ncolors=cmap.N)

    for _, row in df.iterrows():
        llx, lly, urx, ury = row['llx'], row['lly'], row['urx'], row['ury']
        used = row.get(f"{layer}_u", 0)
        total = row.get(f"{layer}_t", 1)
        congestion = used / total if total else 0
        color = cmap(norm(congestion))
        rect = patches.Rectangle((llx, lly), urx - llx, ury - lly,
                                 facecolor=color, edgecolor='black', linewidth=0.05)
        ax.add_patch(rect)

    ax.set_title(f"{layer} Congestion Heatmap")
    ax.set_xlim(df['llx'].min(), df['urx'].max())
    ax.set_ylim(df['lly'].min(), df['ury'].max())
    ax.set_aspect('equal')
    ax.axis('off')

    sm = plt.cm.ScalarMappable(cmap=cmap, norm=norm)
    sm.set_array([])
    cbar = plt.colorbar(sm, ax=ax, fraction=0.046, pad=0.04, ticks=levels)
    cbar.set_label('Congestion (used/total)')

    os.makedirs(output_dir, exist_ok=True)
    output_path = os.path.join(output_dir, f"{layer}_congestion.png")
    plt.savefig(output_path, bbox_inches='tight', pad_inches=0.1)
    plt.close(fig)

# === Main Function ===
def plot_congestion_from_file(file_path: str, output_dir: str = "congestion_maps"):
    df = pd.read_csv(file_path)
    layer_pairs = get_layer_keys(df)
    all_layers = sorted(set(layer for layer, _ in layer_pairs))
    for layer in all_layers:
        plot_layer_congestion(df, layer, output_dir)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Plot and save congestion heatmaps from a CSV file.")
    parser.add_argument("file", type=str, help="Path to the layerwise_features.rpt CSV file")
    parser.add_argument("--out", type=str, default="congestion_maps", help="Output directory for PNG files")
    args = parser.parse_args()
    plot_congestion_from_file(args.file, args.out)

