# Optional sanitizer instrumentation, off by default. Enable with e.g.:
#   cmake -S . -B build-tsan -DPLF_SKIPLIST_SANITIZE=thread
#   cmake -S . -B build-asan -DPLF_SKIPLIST_SANITIZE=address,undefined
#
# Applied only to plf_skiplist_sanitizers, an INTERFACE library that test/example binaries
# link against — a plain (PLF_SKIPLIST_SANITIZE unset) build is completely unaffected.
set(PLF_SKIPLIST_SANITIZE "" CACHE STRING
    "Comma-separated sanitizers to build with (address, thread, undefined, leak). \
ThreadSanitizer cannot be combined with AddressSanitizer.")

add_library(plf_skiplist_sanitizers INTERFACE)

if(PLF_SKIPLIST_SANITIZE)
  string(REPLACE "," ";" _plf_skiplist_sanitize_list "${PLF_SKIPLIST_SANITIZE}")
  foreach(_san IN LISTS _plf_skiplist_sanitize_list)
    if(NOT _san MATCHES "^(address|thread|undefined|leak)$")
      message(FATAL_ERROR "plf-skiplist: unknown sanitizer '${_san}' in PLF_SKIPLIST_SANITIZE (expected address, thread, undefined, or leak)")
    endif()
  endforeach()
  if(PLF_SKIPLIST_SANITIZE MATCHES "thread" AND PLF_SKIPLIST_SANITIZE MATCHES "address")
    message(FATAL_ERROR "plf-skiplist: ThreadSanitizer and AddressSanitizer cannot be combined in one build")
  endif()

  target_compile_options(plf_skiplist_sanitizers INTERFACE -fsanitize=${PLF_SKIPLIST_SANITIZE} -fno-omit-frame-pointer -g)
  target_link_options(plf_skiplist_sanitizers INTERFACE -fsanitize=${PLF_SKIPLIST_SANITIZE})
endif()
