cmake_minimum_required(VERSION 3.16)

add_executable(test_plf_skiplist
  test_main.cpp
  normal/test_functional.cpp
  normal/test_concurrent.cpp
  normal/test_mmap_storage.cpp
  normal/test_manifest.cpp
  normal/test_checkpoint.cpp
  normal/test_recovery.cpp
  normal/test_level_generator.cpp
  normal/test_upper_chunk.cpp
  error/test_functional.cpp
  error/test_manifest.cpp
  error/test_recovery.cpp
)
target_include_directories(test_plf_skiplist PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_plf_skiplist PRIVATE plf-skiplist plf_skiplist_warnings plf_skiplist_sanitizers doctest::doctest)

add_test(NAME test_plf_skiplist COMMAND test_plf_skiplist --duration=true)
set_tests_properties(test_plf_skiplist PROPERTIES TIMEOUT 120)
