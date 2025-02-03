from fastapi import FastAPI
from fastapi.responses import FileResponse
from pydantic import BaseModel
import numpy as np
from typing import List
import matplotlib
import matplotlib.pyplot as plt
import scipy.cluster.hierarchy as sch
import hierarchical_clustering_wpgma
import json

matplotlib.use('Agg')  # Usar backend no interactivo
app = FastAPI()

# Definir el modelo para el vector
class VectorF(BaseModel):
    vector: List[float]
    
def plot_dendrogram(data, output_file:str):
    merges, num_points = hierarchical_clustering_wpgma.hierarchical_clustering_wpgma(data)
    
    # Convertir la lista de merges en un array compatible con scipy
    linkage_matrix = np.zeros((num_points - 1, 4))
    cluster_ids = list(range(num_points))  # Control de índices de cluster
    next_cluster_id = num_points  # Índices nuevos para los clusters fusionados
    
    for i, (a, b, dist) in enumerate(merges):
        linkage_matrix[i, 0] = cluster_ids[a]
        linkage_matrix[i, 1] = cluster_ids[b]
        linkage_matrix[i, 2] = dist
        linkage_matrix[i, 3] = cluster_ids[a] + cluster_ids[b]  # Tamaño del nuevo cluster
        
        # Asignar un nuevo ID al cluster fusionado y marcar 'b' como eliminado
        cluster_ids[a] = next_cluster_id
        cluster_ids[b] = next_cluster_id  # Evitar acceso fuera de rango
        next_cluster_id += 1
    
    # Graficar el dendrograma
    plt.figure(figsize=(10, 5))
    sch.dendrogram(linkage_matrix, labels=np.arange(num_points))
    plt.title("Dendrograma - WPGMA")
    plt.xlabel("Índice del Punto")
    plt.ylabel("Distancia")
    #plt.show()
    plt.savefig(output_file)
    plt.close()

@app.post("/wpgma-linkage")
def calculo(n_points: int):
    output_file = 'hierarchical_clustering_wpgma_linkage.png'
    data = np.random.rand(n_points, 2)  # n puntos en 2D
    plot_dendrogram(data, output_file)
    
    j1 = {
        "Grafica": output_file
    }
    jj = json.dumps(str(j1))

    return jj

@app.get("/wpgma-linkage-graph")
def getGraph(output_file: str):
    return FileResponse(output_file, media_type="image/png", filename=output_file)