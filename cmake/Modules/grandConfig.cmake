INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_GRAND grand)

FIND_PATH(
    GRAND_INCLUDE_DIRS
    NAMES grand/api.h
    HINTS $ENV{GRAND_DIR}/include
        ${PC_GRAND_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GRAND_LIBRARIES
    NAMES gnuradio-grand
    HINTS $ENV{GRAND_DIR}/lib
        ${PC_GRAND_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GRAND DEFAULT_MSG GRAND_LIBRARIES GRAND_INCLUDE_DIRS)
MARK_AS_ADVANCED(GRAND_LIBRARIES GRAND_INCLUDE_DIRS)

