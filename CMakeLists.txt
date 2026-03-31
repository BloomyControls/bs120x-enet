cmake_minimum_required(VERSION 3.14)

project(bs120xenet
  VERSION 0.0.4
  DESCRIPTION "BS120x Ethernet Library"
  HOMEPAGE_URL "https://github.com/BloomyControls/bs120x-enet"
)

option(BS120XENET_INSTALL "Enable install" ON)

if(BS120XENET_STATIC)
  set(BS120XENET_LIB_TYPE STATIC)
elseif(BUILD_SHARED_LIBS)
  set(BS120XENET_LIB_TYPE SHARED)
else()
  set(BS120XENET_LIB_TYPE STATIC)
endif()

include(FetchContent)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

set(FMT_SYSTEM_HEADERS TRUE)
set(FMT_INSTALL OFF)
find_package(fmt 12.1.0 QUIET)
if(NOT fmt_FOUND)
  FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 12.1.0
    GIT_SHALLOW TRUE
  )
  FetchContent_GetProperties(fmt)
  message(STATUS "Fetch fmt")
  FetchContent_MakeAvailable(fmt)
endif()

set(Boost_USE_STATIC_LIBS ON)
set(BOOST_INCLUDE_LIBRARIES system asio)
set(BOOST_ENABLE_CMAKE ON)
# use CONFIG to avoid CMP0167 warnings
find_package(Boost 1.88.0 CONFIG COMPONENTS system QUIET)
if (NOT Boost_FOUND)
  set(SAVED_BUILD_SHARED ${BUILD_SHARED_LIBS})
  set(BUILD_SHARED_LIBS OFF)
  FetchContent_Declare(Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz
    URL_HASH "SHA1=7667d5edc4e753965db02595c912f78ad64725fe"
  )
  FetchContent_GetProperties(Boost)
  if(NOT Boost_POPULATED)
    message(STATUS "Fetch Boost (may take a while)")
    FetchContent_MakeAvailable(Boost)
  endif()
  # workaround an issue with fetched boost asio
  set(BOOST_LIBS ${Boost_LIBRARIES} Boost::asio)
  set(BUILD_SHARED_LIBS ${SAVED_BUILD_SHARED})
else()
  set(BOOST_LIBS ${Boost_LIBRARIES})
endif()

find_package(Doxygen 1.9.8 OPTIONAL_COMPONENTS dot)

add_library(bs120xenet ${BS120XENET_LIB_TYPE}
  src/CInterface.cpp
  src/ConfigFile.cpp
  src/Errors.cpp
  src/EthernetClient.cpp
  src/Frame.cpp
  src/Messages.cpp
  src/TcpClient.cpp
  src/UdpListener.cpp
  src/Util.cpp
)
add_library(bci::bs120xenet ALIAS bs120xenet)

set_target_properties(bs120xenet PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  POSITION_INDEPENDENT_CODE ON
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN TRUE
)

include(GenerateExportHeader)
generate_export_header(bs120xenet
  EXPORT_MACRO_NAME BS120X_API
)
if(${BS120XENET_LIB_TYPE} STREQUAL STATIC)
  target_compile_definitions(bs120xenet PRIVATE BS120XENET_STATIC_DEFINE)
endif()

target_compile_features(bs120xenet PUBLIC cxx_std_20)

target_compile_definitions(bs120xenet PRIVATE
  "BS120XENET_VERSION=((${PROJECT_VERSION_MAJOR}*10000)+(${PROJECT_VERSION_MINOR}*100)+(${PROJECT_VERSION_PATCH}))"
)

if(MSVC)
  target_compile_options(bs120xenet PRIVATE /W3)
else()
  target_compile_options(bs120xenet PRIVATE -Wall -Wextra)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(bs120xenet PRIVATE -Wno-psabi)
  endif()
endif()

if(BS120XENET_INSTALL AND BUILD_SHARED_LIBS)
  include(GNUInstallDirs)
endif()

target_include_directories(bs120xenet PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(bs120xenet PRIVATE
  fmt::fmt-header-only
  ${BOOST_LIBS}
)

if(DOXYGEN_FOUND)
  include(cmake/add_doxygen_target.cmake)
endif()

if(BS120XENET_INSTALL AND BUILD_SHARED_LIBS)
  include(CMakePackageConfigHelpers)

  set(targets_export_name "bs120xenet-targets")
  set(version_config "${PROJECT_BINARY_DIR}/bs120xenet-config-version.cmake")
  set(project_config "${PROJECT_BINARY_DIR}/bs120xenet-config.cmake")
  set(bs120xenet_cmake_dir "${CMAKE_INSTALL_LIBDIR}/cmake/bs120xenet")

  install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/bci"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT devel)

  install(FILES "${PROJECT_BINARY_DIR}/bs120xenet_export.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/bci/bs120x
    COMPONENT devel)

  install(TARGETS bs120xenet
    EXPORT ${targets_export_name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT libs)

  write_basic_package_version_file(
    ${version_config}
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

  configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/bs120xenet-config.cmake.in
    ${project_config}
    INSTALL_DESTINATION ${bs120xenet_cmake_dir})

  install(FILES ${project_config} ${version_config}
    DESTINATION ${bs120xenet_cmake_dir}
    COMPONENT devel)
  install(EXPORT ${targets_export_name} DESTINATION ${bs120xenet_cmake_dir}
    NAMESPACE bci::
    COMPONENT devel)

  # TODO: generate DEB packages
  set(CPACK_PACKAGE_NAME "bs120xenet")
  set(CPACK_PACKAGE_VENDOR "Bloomy Controls, Inc.")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
  set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
  install(FILES
    ${CPACK_RESOURCE_FILE_LICENSE}
    ${CPACK_RESOURCE_FILE_README}
    DESTINATION .
    COMPONENT Metadata
    EXCLUDE_FROM_ALL)
  set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
  set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)

  set(CPACK_PACKAGE_INSTALL_DIRECTORY "Bloomy Controls/bs120xenet")
  set(CPACK_WIX_VERSION 4)
  set(CPACK_WIX_UPGRADE_GUID "E53FFA5E-3F9D-4592-8573-CF975C2CA019")
  set(CPACK_WIX_ROOT_FEATURE_TITLE "BS120x Ethernet Driver")
  set(CPACK_WIX_ROOT_FEATURE_DESCRIPTION
    "BS120x Ethernet driver C/C++ library and development files")
  set(CPACK_WIX_PROPERTY_ARPCOMMENTS "BS120x Ethernet Driver C/C++ Library")
  set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "${PROJECT_HOMEPAGE_URL}")
  set(CPACK_WIX_CMAKE_PACKAGE_REGISTRY "bs120xenet")
  set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/img/installer_banner.jpg")
  set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/img/installer_dialog.jpg")

  if(WIN32)
    # this will create a "package" target on Windows so that we don't have to
    # invoke CPack directly
    set(CPACK_GENERATOR "WIX")
  endif()

  include(CPack)

  cpack_add_component(Metadata HIDDEN)
  cpack_add_component(libs REQUIRED
    DISPLAY_NAME "BS120x driver"
    DESCRIPTION "C/C++ BS120x driver library")
  cpack_add_component(devel
    DISPLAY_NAME "Development files"
    DESCRIPTION "Headers and CMake support files for developing software using the libraries")
endif()
