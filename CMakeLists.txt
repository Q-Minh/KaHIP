cmake_policy(SET CMP0048 NEW)
project(KaHIP VERSION 3.14 LANGUAGES C CXX)

cmake_minimum_required(VERSION 3.21)
include(CheckCXXCompilerFlag)

# if no build mode is specified build in release mode
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# --march=nativeflag
option(NONATIVEOPTIMIZATIONS "Disable --march=native optimizations" OFF)

# check dependencies
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  message(STATUS "OpenMP support detected")
  add_definitions(${OpenMP_CXX_FLAGS})
else()
  message(WARNING "OpenMP not available, activating workaround")
  add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
  set_property(TARGET OpenMP::OpenMP_CXX PROPERTY INTERFACE_COMPILE_OPTIONS "")
  target_include_directories(OpenMP::OpenMP_CXX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/misc)
endif()

# 64 Bit option
option(64BITMODE "64 bit mode" OFF)

# optimized output
option(OPTIMIZED_OUTPUT "optimized output" OFF)
  
# tcmalloc
option(USE_TCMALLOC "if available, link against tcmalloc" OFF)

function(add_definitions_to_target target)
  if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP support detected")
    target_compile_definitions(${target} PRIVATE ${OpenMP_CXX_FLAGS})
  endif()

  if(64BITMODE)
    target_compile_definitions(${target} PRIVATE "-DMODE64BITEDGES")
    target_compile_definitions(${target} PRIVATE "-DPOINTER64=1")
  endif()

  if(OPTIMIZED_OUTPUT)
    target_compile_definitions(${target} PRIVATE "-DKAFFPAOUTPUT")
  endif()

  # tweak compiler flags
  CHECK_CXX_COMPILER_FLAG(-funroll-loops COMPILER_SUPPORTS_FUNROLL_LOOPS)
  if(COMPILER_SUPPORTS_FUNROLL_LOOPS)
    target_compile_definitions(${target} PRIVATE -funroll-loops)
  endif()
  CHECK_CXX_COMPILER_FLAG(-fno-stack-limit COMPILER_SUPPORTS_FNOSTACKLIMITS)
  if(COMPILER_SUPPORTS_FNOSTACKLIMITS)
    target_compile_definitions(${target} PRIVATE -fno-stack-limit) 
  endif()
  CHECK_CXX_COMPILER_FLAG(-Wall COMPILER_SUPPORTS_WALL)
  if(COMPILER_SUPPORTS_WALL)
    target_compile_definitions(${target} PRIVATE -Wall)
  endif()
  CHECK_CXX_COMPILER_FLAG(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)
  if(COMPILER_SUPPORTS_MARCH_NATIVE)
    if( NOT NONATIVEOPTIMIZATIONS )
      target_compile_definitions(${target} PRIVATE -march=native)
    endif()
  endif()
  CHECK_CXX_COMPILER_FLAG(-fpermissive COMPILER_SUPPORTS_FPERMISSIVE)
  if(COMPILER_SUPPORTS_FPERMISSIVE)
    target_compile_definitions(${target} PRIVATE -fpermissive)
  endif()
  CHECK_CXX_COMPILER_FLAG(-Wno-unused-result COMPILER_SUPPORTS_UNUSED)
  if(COMPILER_SUPPORTS_UNUSED)
    target_compile_definitions(${target} PRIVATE -Wno-unused-result)
  endif()
  CHECK_CXX_COMPILER_FLAG(-Wno-sign-compare COMPILER_SUPPORTS_NOSIGNCOMP)
  if(COMPILER_SUPPORTS_NOSIGNCOMP)
    target_compile_definitions(${target} PRIVATE -Wno-sign-compare)
  endif()  
endfunction()

function(add_app_include_dirs target)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/tools)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/partition)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/io)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/argtable3-3.2.1)
endfunction()

function(add_library_include_dirs target)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/io)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/partition)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement)
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/tools)
  endfunction()
  
function(add_public_include_dirs target)
  target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
endfunction()

set(LIBKAFFPA_SOURCE_FILES
lib/data_structure/graph_hierarchy.cpp
  lib/algorithms/strongly_connected_components.cpp
  lib/algorithms/topological_sort.cpp
  lib/algorithms/push_relabel.cpp
  lib/io/graph_io.cpp
  lib/tools/quality_metrics.cpp
  lib/tools/random_functions.cpp
  lib/tools/graph_extractor.cpp
  lib/tools/misc.cpp
  lib/tools/partition_snapshooter.cpp
  lib/partition/graph_partitioner.cpp
  lib/partition/w_cycles/wcycle_partitioner.cpp
  lib/partition/coarsening/coarsening.cpp
  lib/partition/coarsening/contraction.cpp
  lib/partition/coarsening/edge_rating/edge_ratings.cpp
  lib/partition/coarsening/matching/matching.cpp
  lib/partition/coarsening/matching/random_matching.cpp
  lib/partition/coarsening/matching/gpa/path.cpp
  lib/partition/coarsening/matching/gpa/gpa_matching.cpp
  lib/partition/coarsening/matching/gpa/path_set.cpp
  lib/partition/coarsening/clustering/node_ordering.cpp
  lib/partition/coarsening/clustering/size_constraint_label_propagation.cpp
  lib/partition/initial_partitioning/initial_partitioning.cpp
  lib/partition/initial_partitioning/initial_partitioner.cpp
  lib/partition/initial_partitioning/initial_partition_bipartition.cpp
  lib/partition/initial_partitioning/initial_refinement/initial_refinement.cpp
  lib/partition/initial_partitioning/bipartition.cpp
  lib/partition/initial_partitioning/initial_node_separator.cpp
  lib/partition/uncoarsening/uncoarsening.cpp
  lib/partition/uncoarsening/separator/area_bfs.cpp
  lib/partition/uncoarsening/separator/vertex_separator_algorithm.cpp
  lib/partition/uncoarsening/separator/vertex_separator_flow_solver.cpp
  lib/partition/uncoarsening/refinement/cycle_improvements/greedy_neg_cycle.cpp
  lib/partition/uncoarsening/refinement/cycle_improvements/problem_factory.cpp
  lib/partition/uncoarsening/refinement/cycle_improvements/augmented_Qgraph.cpp
  lib/partition/uncoarsening/refinement/mixed_refinement.cpp
  lib/partition/uncoarsening/refinement/label_propagation_refinement/label_propagation_refinement.cpp
  lib/partition/uncoarsening/refinement/refinement.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/2way_fm_refinement/two_way_fm.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/two_way_flow_refinement.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/boundary_bfs.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/flow_solving_kernel/cut_flow_problem_solver.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/flow_refinement/most_balanced_minimum_cuts/most_balanced_minimum_cuts.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_refinement.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/complete_boundary.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/partial_boundary.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_scheduling/quotient_graph_scheduling.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_scheduling/simple_quotient_graph_scheduler.cpp
  lib/partition/uncoarsening/refinement/quotient_graph_refinement/quotient_graph_scheduling/active_block_quotient_graph_scheduler.cpp
  lib/partition/uncoarsening/refinement/kway_graph_refinement/kway_graph_refinement.cpp
  lib/partition/uncoarsening/refinement/kway_graph_refinement/kway_graph_refinement_core.cpp
  lib/partition/uncoarsening/refinement/kway_graph_refinement/kway_graph_refinement_commons.cpp
  lib/partition/uncoarsening/refinement/cycle_improvements/augmented_Qgraph_fabric.cpp
  lib/partition/uncoarsening/refinement/cycle_improvements/advanced_models.cpp
  lib/partition/uncoarsening/refinement/kway_graph_refinement/multitry_kway_fm.cpp
  lib/partition/uncoarsening/refinement/node_separators/greedy_ns_local_search.cpp
  lib/partition/uncoarsening/refinement/node_separators/fm_ns_local_search.cpp
  lib/partition/uncoarsening/refinement/node_separators/localized_fm_ns_local_search.cpp
  lib/algorithms/cycle_search.cpp
  lib/partition/uncoarsening/refinement/cycle_improvements/cycle_refinement.cpp
  lib/partition/uncoarsening/refinement/tabu_search/tabu_search.cpp
  extern/argtable3-3.2.1/argtable3.c)
add_library(libkaffpa OBJECT ${LIBKAFFPA_SOURCE_FILES})

set(LIBMAPPING_SOURCE_FILES
  lib/mapping/local_search_mapping.cpp
  lib/mapping/full_search_space.cpp
  lib/mapping/full_search_space_pruned.cpp
  lib/mapping/communication_graph_search_space.cpp
  lib/mapping/fast_construct_mapping.cpp
  lib/mapping/construct_distance_matrix.cpp
  lib/mapping/mapping_algorithms.cpp
  lib/mapping/construct_mapping.cpp)
add_library(libmapping OBJECT ${LIBMAPPING_SOURCE_FILES})

set(LIBSPAC_SOURCE_FILES lib/spac/spac.cpp)
add_library(libspac OBJECT ${LIBSPAC_SOURCE_FILES})

set(NODE_ORDERING_SOURCE_FILES
  lib/node_ordering/min_degree_ordering.cpp
  lib/node_ordering/nested_dissection.cpp
  lib/node_ordering/ordering_tools.cpp
  lib/node_ordering/reductions.cpp)
add_library(libnodeordering OBJECT ${NODE_ORDERING_SOURCE_FILES})

foreach(target IN ITEMS 
  libkaffpa 
  libmapping 
  libspac 
  libnodeordering
)
  add_library_include_dirs(${target})
  add_definitions_to_target(${target})
endforeach()

# generate targets for each binary
# add_executable(kaffpa app/kaffpa.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping>)
# target_compile_definitions(kaffpa PRIVATE "-DMODE_KAFFPA")
# target_link_libraries(kaffpa PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS kaffpa DESTINATION bin)
# if (USE_TCMALLOC)
#   find_library(TCMALLOC_LIB tcmalloc)
#   if (TCMALLOC_LIB)
#     target_link_libraries(kaffpa ${TCMALLOC_LIB})
#     message(STATUS "Using tcmalloc: ${TCMALLOC_LIB}")
#   else ()
#     message(STATUS "tcmalloc enabled but unavailable on this system")
#   endif ()
# endif ()

# add_executable(global_multisection app/global_multisection.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping>)
# target_compile_definitions(global_multisection PRIVATE "-DMODE_KAFFPA -DMODE_GLOBALMS")
# target_link_libraries(global_multisection PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS global_multisection DESTINATION bin)
# if (USE_TCMALLOC)
#   find_library(TCMALLOC_LIB tcmalloc)
#   if (TCMALLOC_LIB)
#     target_link_libraries(global_multisection ${TCMALLOC_LIB})
#     message(STATUS "Using tcmalloc: ${TCMALLOC_LIB}")
#   else ()
#     message(STATUS "tcmalloc enabled but unavailable on this system")
#   endif ()
# endif ()

# add_executable(evaluator app/evaluator.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping>)
# target_compile_definitions(evaluator PRIVATE "-DMODE_EVALUATOR")
# target_link_libraries(evaluator PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS evaluator DESTINATION bin)

# add_executable(node_separator app/node_separator_ml.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping>)
# target_compile_definitions(node_separator PRIVATE "-DMODE_NODESEP")
# target_link_libraries(node_separator PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS node_separator DESTINATION bin)

# add_executable(label_propagation app/label_propagation.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping>)
# target_compile_definitions(label_propagation PRIVATE "-DMODE_LABELPROPAGATION")
# target_link_libraries(label_propagation PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS label_propagation DESTINATION bin)

# add_executable(partition_to_vertex_separator app/partition_to_vertex_separator.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping>)
# target_compile_definitions(partition_to_vertex_separator PRIVATE "-DMODE_PARTITIONTOVERTEXSEPARATOR")
# target_link_libraries(partition_to_vertex_separator PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS partition_to_vertex_separator DESTINATION bin)

add_executable(interface_test misc/example_library_call/interface_test.cpp interface/kaHIP_interface.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping> $<TARGET_OBJECTS:libnodeordering>)
target_include_directories(interface_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/interface)
target_compile_definitions(interface_test PRIVATE "-DMODE_KAFFPA")
target_link_libraries(interface_test PRIVATE OpenMP::OpenMP_CXX)
install(TARGETS interface_test DESTINATION bin)

add_executable(graphchecker app/graphchecker.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping>)
target_compile_definitions(graphchecker PRIVATE "-DMODE_GRAPHCHECKER")
target_link_libraries(graphchecker PRIVATE OpenMP::OpenMP_CXX)
install(TARGETS graphchecker DESTINATION bin)

# add_executable(edge_partitioning app/spac.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libmapping> $<TARGET_OBJECTS:libspac>)
# target_compile_definitions(edge_partitioning PRIVATE "-DMODE_KAFFPA")
# target_link_libraries(edge_partitioning PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS edge_partitioning DESTINATION bin)

# add_executable(node_ordering app/node_ordering.cpp $<TARGET_OBJECTS:libkaffpa> $<TARGET_OBJECTS:libnodeordering>)
# target_compile_definitions(node_ordering PRIVATE "-DMODE_NODESEP -DMODE_NODEORDERING")
# target_link_libraries(node_ordering PRIVATE OpenMP::OpenMP_CXX)
# install(TARGETS node_ordering DESTINATION bin)

foreach (target IN ITEMS 
  # kaffpa 
  # global_multisection 
  # evaluator 
  # node_separator 
  # label_propagation 
  # partition_to_vertex_separator 
  interface_test 
  graphchecker 
  # edge_partitioning 
  # node_ordering
)
  add_app_include_dirs(${target})
endforeach()

# TODO: Expose kahip internals for local refinement operators for example, in the future.
# interface library
add_library(kahip interface/kaHIP_interface.cpp)
target_include_directories(kahip 
PUBLIC 
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/interface>
)
add_library_include_dirs(kahip)
target_compile_definitions(kahip PRIVATE "-DMODE_KAFFPA")
target_link_libraries(kahip PRIVATE libkaffpa libmapping libnodeordering OpenMP::OpenMP_CXX)

# Install
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
  TARGETS kahip libkaffpa libmapping libnodeordering
  EXPORT kahip_targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} 
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} 
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR} 
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY interface/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

install(
  EXPORT kahip_targets
  FILE kahipTargets.cmake
  NAMESPACE kahip::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kahip
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/kahipConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/kahipConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/kahipConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kahip
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kahipConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/kahipConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kahip
)
