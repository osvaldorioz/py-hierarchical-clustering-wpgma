#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <vector>
#include <tuple>
#include <limits>
#include <cmath>

//g++ -O2 -Wall -shared -std=c++20 -fPIC `python3.12 -m pybind11 --includes` wpgma.cpp -o hierarchical_clustering_wpgma`python3.12-config --extension-suffix`
//g++ -O3 -Wall -shared -std=c++20 -fPIC $(python3.12 -m pybind11 --includes) wpgma.cpp -o hierarchical_clustering_wpgma$(python3.12-config --extension-suffix)

namespace py = pybind11;
using Matrix = std::vector<std::vector<double>>;

// Calcula la distancia euclidiana entre dos puntos
double euclidean_distance(const std::vector<double>& a, const std::vector<double>& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return std::sqrt(sum);
}

// Calcula la distancia media ponderada por pares entre dos grupos
double wpgma_linkage(int size_a, int size_b, double dist_ab) {
    return 0.5 * dist_ab; // WPGMA utiliza pesos fijos de 0,5 para ambos grupos
}

// Hierarchical clustering usando WPGMA
py::tuple hierarchical_clustering_wpgma(py::array_t<double> input_data) {
    py::buffer_info buf = input_data.request();
    if (buf.ndim != 2) {
        throw std::runtime_error("Input_data debe ser un array 2D");
    }

    size_t num_points = buf.shape[0];
    size_t dimensions = buf.shape[1];
    
    // Convertir el input data a un vector C++
    std::vector<std::vector<double>> data(num_points, std::vector<double>(dimensions));
    double* ptr = static_cast<double*>(buf.ptr);
    for (size_t i = 0; i < num_points; ++i) {
        for (size_t j = 0; j < dimensions; ++j) {
            data[i][j] = ptr[i * dimensions + j];
        }
    }
    
    // Calcular la matriz de distancia inicial
    Matrix distances(num_points, std::vector<double>(num_points, 0.0));
    for (size_t i = 0; i < num_points; ++i) {
        for (size_t j = i + 1; j < num_points; ++j) {
            distances[i][j] = distances[j][i] = euclidean_distance(data[i], data[j]);
        }
    }

    // Inicializa los clusters
    std::vector<int> cluster_sizes(num_points, 1); // All clusters start with size 1
    py::list merges;

    // Agglomerative clustering usando WPGMA
    for (size_t step = 0; step < num_points - 1; ++step) {
        double min_dist = std::numeric_limits<double>::max();
        int cluster_a = -1, cluster_b = -1;

        for (size_t i = 0; i < num_points; ++i) {
            for (size_t j = i + 1; j < num_points; ++j) {
                if (cluster_sizes[i] > 0 && cluster_sizes[j] > 0 && distances[i][j] < min_dist) {
                    min_dist = distances[i][j];
                    cluster_a = i;
                    cluster_b = j;
                }
            }
        }

        // Se juntan los clusters
        merges.append(py::make_tuple(cluster_a, cluster_b, min_dist));
        cluster_sizes[cluster_a] += cluster_sizes[cluster_b];
        cluster_sizes[cluster_b] = 0;

        // Se actualizan las distancias usando la formula WPGMA
        for (size_t k = 0; k < num_points; ++k) {
            if (k != static_cast<size_t>(cluster_a) && cluster_sizes[k] > 0) {
                distances[cluster_a][k] = distances[k][cluster_a] = wpgma_linkage(cluster_sizes[cluster_a], cluster_sizes[k], distances[cluster_a][k]);
            }
        }
    }

    return py::make_tuple(merges, num_points);
}

PYBIND11_MODULE(hierarchical_clustering_wpgma, m) {
    m.def("hierarchical_clustering_wpgma", &hierarchical_clustering_wpgma, "Hierarchical clustering usando WPGMA (Weighted Pairwise Mean Linkage)");
}
