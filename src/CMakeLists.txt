## Make the ICA example project
project (mpi_manager)

IF(Boost_FOUND)

  #include the includes of this project
  include_directories ("../include")
  include_directories(${Boost_INCLUDE_DIRS})

  find_package ( MPI )
  if ( MPI_FOUND )
    include_directories( ${MPI_INCLUDE_PATH} )
  endif( MPI_FOUND )

  #build
  add_library(mpi_manager  mpi_manager.cpp)
  target_link_libraries (mpi_manager ${Boost_LIBRARIES} ${boost_serialization_LIBRARY})
  target_link_libraries (mpi_manager ${math_rng_LIBRARIES} ${GSLCBLAS_LIBRARIES})
  target_link_libraries (mpi_manager ${GSL_LIBRARIES} ${GSLCBLAS_LIBRARIES})
  target_link_libraries (mpi_manager ${MPI_LIBRARIES})

  install(DIRECTORY ../include/ DESTINATION include
    FILES_MATCHING PATTERN "*.hpp")
  install (TARGETS mpi_manager DESTINATION lib)

ENDIF()