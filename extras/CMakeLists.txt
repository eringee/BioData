cmake_minimum_required(VERSION 3.24...3.30)

project(extras VERSION 1.0 LANGUAGES CXX)

# Define the library sources
add_library(extras
  PeakDetector.cpp
  PeakDetector.h
  utils.cpp
  utils.h
)
