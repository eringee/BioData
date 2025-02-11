cmake_minimum_required(VERSION 3.24...3.30)

project(BioData VERSION 1.0 LANGUAGES CXX)

# Define the library sources
add_library(BioData
  src/Heart.h
  src/Heart.cpp
  src/SkinConductance.cpp
  src/SkinConductance.h
  src/Timing.h
  src/Timing.cpp
  src/utils.h
  src/utils.cpp
  src/Average.h
  src/MinMax.h
  src/Lop.h
  src/Hip.h
  src/Threshold.h
)
