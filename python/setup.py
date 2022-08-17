import setuptools
import pathlib
import os

ROOT = pathlib.Path(__file__).parent
README = ROOT / "README.md"
VERSION = ROOT / "VERSION.md"


def get_readme():
    with open(README, "r") as f:
        return f.read()


def get_version():
    with open(VERSION, "r") as f:
        return f.read().replace("\n", "")


def get_lib_names():
    return ["spla_x64.dll",
            "libspla_x64.so",
            "libspla_x64.dylib",
            "libspla_arm64.dylib"]


setuptools.setup(
    name="pyspla",
    version=get_version(),
    author="Egor Orachev",
    author_email="egororachyov@gmail.com",
    license="MIT",
    description="spla library python bindings",
    long_description=get_readme(),
    long_description_content_type="text/markdown",
    url="https://github.com/JetBrains-Research/spla",
    project_urls={
        "Spla project": "https://github.com/JetBrains-Research/spla",
        "Bug Tracker": "https://github.com/JetBrains-Research/spla/issues"
    },
    classifiers=[
        "Development Status :: 1 - Planning",
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "License :: OSI Approved :: MIT License",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
        "Operating System :: MacOS",
        "Environment :: GPU",
        "Environment :: Console",
        "Intended Audience :: Developers",
        "Intended Audience :: End Users/Desktop",
        "Intended Audience :: Information Technology",
        "Intended Audience :: Science/Research",
        "Natural Language :: English",
        "Topic :: Scientific/Engineering",
        "Topic :: Scientific/Engineering :: Mathematics",
        "Topic :: Scientific/Engineering :: Information Analysis",
        "Topic :: Scientific/Engineering :: Bio-Informatics"
    ],
    keywords=[
        "python",
        "cplusplus",
        "generalized",
        "sparse-matrix",
        "linear-algebra",
        "graph-analysis",
        "graph-algorithms",
        "graphblas",
        "nvidia-cuda",
        "opencl",
        "gpu"
    ],
    packages=["pyspla"],
    package_dir={'': '.'},
    package_data={'': get_lib_names()},
    python_requires=">=3.0",
    include_package_data=True
)
