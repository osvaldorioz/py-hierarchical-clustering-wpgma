from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "hierarchical_clustering_wpgma",
        ["app/wpgma.cpp"],
        extra_compile_args=["-std=c++20"],
    ),
]

setup(
    name="hierarchical_clustering_wpgma",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
