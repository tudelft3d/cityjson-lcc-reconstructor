# CityJSON LCC Reconstructor

A tool that reconstructs a CityJSON file to a Linear Cell Complex. The LCC can then be either saved a three-dimensional combinatorial map file (`.3map`) or as a CityJSON file with the LCC data stored according to the [LCC Extension](https://github.com/tudelft3d/cityjson-lcc-extension).

## Requirements

* [CGAL](https://www.cgal.org/): The LCC (and Combinatorial Map) package is used for the topological represntation.
* [JSON for Modern C++](https://github.com/nlohmann/json): Used for JSON parsing (the required files are included in source code).
* [CMake](https://cmake.org/): To compile this code.

## Installation

1. Clone this repository.
2. Install CGAL (instructions [here](https://doc.cgal.org/latest/Manual/installation.html)).
3. Install CMake.
4. Build the software:
```
mkdir build
cd build
cmake ..
make
```

## Usage

```
./cityjson2lcc /path/to/cityjson.json -n /path/to/new_files.json -p [number_of_decimal_digits]
```