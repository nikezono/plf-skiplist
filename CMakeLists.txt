cmake_minimum_required(VERSION 3.16)
project(plf-skiplist VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

file(GLOB PLF_SKIPLIST_DETAIL_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/plf_skiplist/detail/*.hpp)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/plf_skiplist/plf_skiplist.hpp
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/amalgamate.sh
  DEPENDS
    ${PLF_SKIPLIST_DETAIL_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/plf_skiplist/plf_skiplist_dev.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/amalgamate.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Amalgamating plf_skiplist.hpp from detail/ sources (requires quom; skips with a warning if not installed)"
  VERBATIM
)
add_custom_target(plf_skiplist_amalgamate DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/plf_skiplist/plf_skiplist.hpp)

add_library(plf-skiplist INTERFACE)
add_library(plf-skiplist::plf-skiplist ALIAS plf-skiplist)

target_include_directories(plf-skiplist INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
add_dependencies(plf-skiplist plf_skiplist_amalgamate)

add_library(plf_skiplist_warnings INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(plf_skiplist_warnings INTERFACE -Wall -Wextra -Wpedantic -Werror)
elseif(MSVC)
  target_compile_options(plf_skiplist_warnings INTERFACE /W4 /WX)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Sanitizers.cmake)

# ── Installation and Export Configuration ─────────────────────────────────────
install(TARGETS plf-skiplist
  EXPORT plf-skiplistTargets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/plf-skiplistConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/plf-skiplistConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/plf-skiplistConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/plf-skiplist"
)

install(EXPORT plf-skiplistTargets
  FILE plf-skiplistTargets.cmake
  NAMESPACE plf-skiplist::
  DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/plf-skiplist"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/plf-skiplistConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/plf-skiplistConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/plf-skiplist"
)

# ── doctest (single-header, header-only test framework) ─────────────────────
include(FetchContent)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.4.11
  GIT_SHALLOW    TRUE
)
set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(doctest)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)

# ── clang-tidy static analysis (CI-only) ─────────────────────────────────────
find_program(CLANG_TIDY_PROGRAM clang-tidy)
if(CLANG_TIDY_PROGRAM)
  add_custom_target(clang-tidy-check
    COMMAND ${CLANG_TIDY_PROGRAM} -quiet --extra-arg=-w
            --header-filter=${CMAKE_CURRENT_SOURCE_DIR}/include/plf_skiplist/detail/.*
            -p ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/examples/basic_usage.cpp
    COMMENT "Running clang-tidy static analysis..."
    VERBATIM
  )
endif()
