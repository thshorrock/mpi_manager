

##########################################################################
#  Build the TEST app
##########################################################################

IF(Boost_FOUND)
  include_directories ("../include")
  include_directories(${Boost_INCLUDE_DIRS})

  project(test_mpi_manager)

  add_executable(test_mpi_manager mpi_test.cpp)
  

  target_link_libraries (test_mpi_manager mpi_manager)
  target_link_libraries (test_mpi_manager ${Boost_LIBRARIES})
  target_link_libraries (test_mpi_manager ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES})
  add_test(test-mpi_manager-lib ${CMAKE_BINARY_DIR}/bin/test_mpi_manager)

ENDIF()