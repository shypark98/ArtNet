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
import time
import pandas as pd
import numpy as np
import networkx as nx
import igraph as ig
import leidenalg as la
from typing import Tuple, List, Dict
from sklearn.metrics import davies_bouldin_score, silhouette_score, calinski_harabasz_score
import community as community_louvain

SEED = 4

def gen_rgb_color(num_colors: int) -> List[Tuple[int, int, int]]:
    rgb_values = np.linspace(50, 255, num=int(np.ceil(num_colors ** (1/3))), dtype=int)
    colors = [(r, g, b) for r in rgb_values for g in rgb_values for b in rgb_values]
    return colors[:num_colors]

def gen_nx_graph(node_file: str, edge_file: str) -> Tuple[nx.Graph, pd.DataFrame]:
    edge_df = pd.read_csv(edge_file)
    node_df = pd.read_csv(node_file)

    # Clean names
    node_df['Name'] = node_df['Name'].replace(to_replace=r'[{}\\]', value='', regex=True)
    edge_df['Source'] = edge_df['Source'].replace(to_replace=r'[{}\\]', value='', regex=True)
    edge_df['Sink'] = edge_df['Sink'].replace(to_replace=r'[{}\\]', value='', regex=True)

    node_df['isSource'] = node_df['Name'].isin(edge_df['Source'])
    node_df['isSink'] = node_df['Name'].isin(edge_df['Sink'])
    node_df['isNode'] = node_df['isSource'] | node_df['isSink']

    G_nx = nx.from_pandas_edgelist(edge_df, 'Source', 'Sink', edge_attr='Weight', create_using=nx.Graph)
    return G_nx, node_df

def nx_graph_to_igraph(G_nx: nx.Graph) -> ig.Graph:
    mapping = {name: idx for idx, name in enumerate(G_nx.nodes())}
    edges = [(mapping[u], mapping[v]) for u, v in G_nx.edges()]
    g_ig = ig.Graph(edges=edges, directed=False)
    g_ig.vs["name"] = list(mapping.keys())
    return g_ig

def run_leiden_via_nx(node_file: str, edge_file: str, cluster_file: str,
                      n_iteration: int = 10, seed: int = SEED, isCsv: bool = True) -> Tuple[pd.DataFrame, nx.Graph]:
    G_nx, node_df = gen_nx_graph(node_file, edge_file)
    G_ig = nx_graph_to_igraph(G_nx)
    
    '''
    partition = la.find_partition(
        G_ig,
        la.ModularityVertexPartition,
        n_iterations=n_iteration,
        seed=seed,
    )
    '''
    partition = la.find_partition(
            G_ig, 
            la.RBConfigurationVertexPartition,
            n_iterations=n_iteration,
            seed=seed, 
            resolution_parameter=0.8)

    # Map node to cluster
    cluster_map = {}
    for cluster_id, nodes in enumerate(partition):
        for node in nodes:
            name = G_ig.vs[node]["name"]
            cluster_map[name] = cluster_id

    cluster_df = pd.DataFrame(cluster_map.items(), columns=['Name', 'Cluster_id'])
    node_df = node_df.merge(cluster_df, on='Name', how='left')
    node_df = node_df[node_df['Cluster_id'].notna()]
    node_df = update_cluster(node_df)
    write_cluster_file(node_df, cluster_file, isCsv)
    return node_df, G_nx

def update_cluster(node_df: pd.DataFrame) -> pd.DataFrame:
    max_cluster_id = int(node_df['Cluster_id'].max() if not node_df['Cluster_id'].isna().all() else -1)

    if node_df['Cluster_id'].isna().any():
        node_df['Cluster_id'].fillna(max_cluster_id + 1, inplace=True)
        node_df['Cluster_id'] = node_df['Cluster_id'].astype(int)

    num_clusters = node_df['Cluster_id'].nunique()
    colors = gen_rgb_color(num_clusters)
    color_df = pd.DataFrame({'Cluster_id': sorted(node_df['Cluster_id'].unique()), 'color': colors})
    node_df = node_df.merge(color_df, on='Cluster_id', how='left')
    return node_df

def write_cluster_file(cluster_df: pd.DataFrame, cluster_file: str, isCsv: bool = False) -> None:
    with open(cluster_file, 'w') as fp:
        for node, cluster_id, color in cluster_df[['Name', 'Cluster_id', 'color']].values.tolist():
            hex_color = '#%02x%02x%02x' % tuple(color)
            fp.write(f'{node} {cluster_id} {hex_color}\n')
    if isCsv and cluster_file.endswith('.rpt'):
        cluster_df.to_csv(cluster_file.replace('.rpt', '.csv'), index=False)

def report_metrics(df: pd.DataFrame, G: nx.Graph) -> None:
    df = df[df['Name'].notna() & df['Cluster_id'].notna()]
    df['Cluster_id'] = df['Cluster_id'].astype(int)

    node_cluster = df['Cluster_id'].tolist()
    node_metrics = df[['X', 'Y']].values.tolist()

    dbi_score = davies_bouldin_score(node_metrics, node_cluster)
    sc_score = silhouette_score(node_metrics, node_cluster)
    vrc_score = calinski_harabasz_score(node_metrics, node_cluster)

    partition = {row['Name']: row['Cluster_id'] for _, row in df.iterrows()}
    G = G.subgraph(partition.keys()).copy()
    modularity_score = community_louvain.modularity(partition, G, weight='Weight')

    def conductance(G, cluster_nodes):
        cut_edges = 0
        volume = 0
        for node in cluster_nodes:
            for neighbor in G.neighbors(node):
                if neighbor not in cluster_nodes:
                    cut_edges += 1
                volume += 1
        return cut_edges / volume if volume > 0 else 0

    cluster_dict: Dict[int, set] = {}
    for name, cid in partition.items():
        cluster_dict.setdefault(cid, set()).add(name)
    conductance_scores = [conductance(G, nodes) for nodes in cluster_dict.values()]
    avg_conductance = np.mean(conductance_scores)

    print('The Clustering metrics are:')
    print(f'Number of clusters: {len(cluster_dict)}')
    print(f'DBI: {dbi_score:.4f}')
    print(f'SC: {sc_score:.4f}')
    print(f'VRC: {vrc_score:.4f}')
    print(f'Modularity: {modularity_score:.4f}')
    print(f'Average Conductance: {avg_conductance:.4f}')

def run_clustering(run_dir: str, design: str) -> None:
    if not os.path.exists(run_dir):
        print(f"Error: {run_dir} does not exist")
        exit()

    node_file = f"{run_dir}/{design}_nodes.csv"
    edge_file = f"{run_dir}/{design}_edges.csv"
    placement_file = f"{run_dir}/{design}_placement.csv"

    print("\nRunning Leiden Clustering via iGraph")
    cluster_rpt = f"{run_dir}/{design}_cluster_leiden.rpt"
    start_time = time.time()
    clustered_df, G_nx = run_leiden_via_nx(node_file, edge_file, cluster_rpt)
    end_time = time.time()
    print(f"Time taken: {end_time - start_time:.2f} seconds")

    if os.path.exists(placement_file):
        placement_df = pd.read_csv(placement_file)
        placement_df = placement_df.replace(to_replace=r'[{}\\]', value='', regex=True)

        print("\nReporting Clustering Metrics")
        full_df = pd.merge(placement_df, clustered_df, on='Name')
        report_metrics(full_df, G_nx)
    else:
        print(f"\nError: {placement_file} does not exist")

if __name__ == '__main__':
    design = "nova"
    exp = "real"
    run_dir = f"./info/{exp}/{design}"
    run_clustering(run_dir, design)

