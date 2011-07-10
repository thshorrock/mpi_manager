
cmake_minimum_required (VERSION 2.6)
project (mpi_manager-project)

# The version number.
set (mpi_manager-project_VERSION_MAJOR 0)
set (mpi_manager-project_VERSION_MINOR 1)
set (mpi_manager-project_PATCH_LEVEL 0 )

# project options
OPTION( BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON )
OPTION( INSTALL_DOC "Set to OFF to skip build/install Documentation" ON )
OPTION( USING_MPI "Set to OFF to turn off mpi" ON )
OPTION( BUILD_MISSING_DEPENDANCIES "Set to OFF to skip building missing dependencies (you might want to build the latest versions yourself)" ON )
OPTION( BUILD_COVERAGE "Set to ON to generate coverage information (make lcov)" OFF )

#if(USING_MPI)
#IF (CMAKE_COMPILER_IS_GNUCC)
#ADD_DEFINITIONS(-DUSING_MPI)
#else()
#ADD_DEFINITIONS(/DUSING_MPI)
#endif()
#endif()
# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf
# directories.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/xlib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#include find modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

##########################################################################
# MAKE SOME EXTRA BUILD TYPES:
#  MAINTAINER: with some extra warnings (only gcc)
#  PROFILE:  Profiling mode (only gcc)
###########################################################################
##########################################################################
#  MAINTAINTER
##########################################################################

IF (CMAKE_COMPILER_IS_GNUCC)
SET( CMAKE_CXX_FLAGS_MAINTAINER "-Wall -Wabi" CACHE STRING
    "Flags used by the C++ compiler during maintainer builds."
    FORCE )
SET( CMAKE_C_FLAGS_MAINTAINER "-Wall -pedantic" CACHE STRING
    "Flags used by the C compiler during maintainer builds."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    "-Wl,--warn-unresolved-symbols,--warn-once" CACHE STRING
    "Flags used for linking binaries during maintainer builds."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_MAINTAINER
    "-Wl,--warn-unresolved-symbols,--warn-once" CACHE STRING
    "Flags used by the shared libraries linker during maintainer builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_MAINTAINER
    CMAKE_C_FLAGS_MAINTAINER
    CMAKE_EXE_LINKER_FLAGS_MAINTAINER
    CMAKE_SHARED_LINKER_FLAGS_MAINTAINER )
# Update the documentation string of CMAKE_BUILD_TYPE for GUIs
SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Maintainer."
    FORCE )
ENDIF()
##########################################################################
#  PROFILE mode for linux
##########################################################################

IF (CMAKE_COMPILER_IS_GNUCC)
SET( CMAKE_CXX_FLAGS_PROFILE "-O2 -g -pg" CACHE STRING
    "Flags used by the C++ compiler during profile builds."
    FORCE )
SET( CMAKE_C_FLAGS_PROFILE "-O2 -g -pg" CACHE STRING
    "Flags used by the C compiler during profile builds."
    FORCE )
SET( CMAKE_EXE_LINKER_FLAGS_PROFILE
    "-pg " CACHE STRING 
    "Flags used for linking binaries during profile builds."
    FORCE )
SET( CMAKE_SHARED_LINKER_FLAGS_PROFILE
    "-pg  " CACHE STRING 
    "Flags used by the shared libraries linker during profile builds."
    FORCE )
MARK_AS_ADVANCED(
    CMAKE_CXX_FLAGS_PROFILE
    CMAKE_C_FLAGS_PROFILE
    CMAKE_EXE_LINKER_FLAGS_PROFILE
    CMAKE_SHARED_LINKER_FLAGS_PROFILE )
# Update the documentation string of CMAKE_BUILD_TYPE for GUIs
SET( CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel  Maintainer Profile."
    FORCE )
ENDIF()



##########################################################################
# Find the Boost Libraries
##########################################################################

##find the boost test suite

FIND_PACKAGE(Boost COMPONENTS test_exec_monitor serialization  mpi REQUIRED) # do we have any boost at all?

##########################################################################
#  Now gsl subset
##########################################################################
set(GSL_DIR cmake)
find_package(GSL REQUIRED)
IF(GSL_FOUND)
  include_directories(${GSL_INCLUDE_DIRS} ${GSLCBLAS_INCLUDE_DIRS})
ENDIF()


##########################################################################
#  Build the app
##########################################################################

add_subdirectory(src)

##########################################################################
#  Build the TEST app
##########################################################################

add_subdirectory(test)

##########################################################################
#  Build Documentation
##########################################################################
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${mpi_manager-project_SOURCE_DIR}/Doxyfile.in ${mpi_manager-project_SOURCE_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${mpi_manager-project_SOURCE_DIR}/Doxyfile
WORKING_DIRECTORY ${mpi_manager-project_SOURCE_DIR_SOURCE_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
##########################################################################
#  Packaging
##########################################################################


INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MPI manager")
SET(CPACK_PACKAGE_VENDOR "T.H. Shorrock")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
#  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\ICA.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} mpi_manager-project")
  #SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\git@github.com:thshorrock/Ensemble-Learning.git")
  #SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
  SET(CPACK_NSIS_CONTACT "t.h.shorrock@gmail.com")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
#  SET(CPACK_STRIP_FILES "bin/ICA")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
#SET(CPACK_PACKAGE_EXECUTABLES "ICA" "mpi_manager-project")
INCLUDE(CPack)