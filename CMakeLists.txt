cmake_minimum_required(VERSION 3.16)
project(He3BeamSim)

find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

# ROOT is required for histogram output
find_package(ROOT REQUIRED COMPONENTS Core Hist Tree Graf)
include(${ROOT_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(He3BeamSim He3BeamSim.cc ${sources} ${headers})
target_link_libraries(He3BeamSim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})
