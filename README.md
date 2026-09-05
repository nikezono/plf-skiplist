# plf-skiplist

A header-only, offset-based, lock-free, persistent skip list library for C++20.

- **Lock-free concurrent access** — `get()`/`put()`/`remove()`/`scan()` are all safe to
  call concurrently from multiple threads without external locking.
- **Crash-consistent** — backed by a single `mmap`'d file, `MAP_SHARED`. `checkpoint()`
  is synchronous: once it returns, every write that had already returned is guaranteed
  durable across an unclean process exit.
- **Real physical reclaim** — deleted keys' space is genuinely freed (not just
  tombstoned) and reused, without needing a stop-the-world compaction pass.

See [`design.md`](design.md) for the full design (data layout,
epoch-based reclamation, crash-consistency model, linearization points, and the prior
art each design element draws from).

## Usage

```cpp
#include <plf_skiplist/plf_skiplist.hpp>

plf_skiplist::PSkipList<int> list("/path/to/data.db", capacity_bytes);

list.put(42, 100);
list.get(42);              // -> std::optional<uint64_t>{100}
list.remove(42);
list.scan(0, 100, [](int key, uint64_t value) { /* ... */ });

list.checkpoint();         // durable up to this point, survives an unclean exit
list.reclaim();            // physically frees removed keys' space for reuse
```

`plf_skiplist.hpp` is a single, self-contained amalgamated header — copy it into your
project and `#include` it, no build step required. See
[`examples/basic_usage.cpp`](examples/basic_usage.cpp) for a complete walkthrough
including recovery after a simulated crash.

## Integration via CMake

### `find_package`

After installing `plf-skiplist`:

```cmake
find_package(plf-skiplist CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE plf-skiplist::plf-skiplist)
```

### FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
  plf-skiplist
  GIT_REPOSITORY https://github.com/nikezono/plf-skiplist.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(plf-skiplist)

target_link_libraries(your_target PRIVATE plf-skiplist::plf-skiplist)
```

## Building and testing

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

### Installation

```sh
cmake --build build --target install
```

Editing anything under `include/plf_skiplist/detail/` regenerates the single-header
`include/plf_skiplist/plf_skiplist.hpp` via `scripts/amalgamate.sh` (requires
[`quom`](https://github.com/Viatorus/quom): `pip install quom`) as part of the build.
If `quom` isn't installed, the amalgamation step is skipped with a warning and the
previously-generated header is used as-is — regenerate it manually before committing
a change to `detail/`.

To build with a sanitizer (see `cmake/Sanitizers.cmake`):

```sh
cmake -S . -B build-tsan -DPLF_SKIPLIST_SANITIZE=thread
cmake --build build-tsan
TSAN_OPTIONS="suppressions=$(pwd)/tsan_suppressions.txt" ctest --test-dir build-tsan --output-on-failure
```

`tsan_suppressions.txt` covers one intentional, verified-safe seqlock race (see that file's own
comment) — omit it and ThreadSanitizer will report that one finding.

## Requirements

- C++20, POSIX (`mmap`/`msync`).
- CMake >= 3.16.

## License

[MIT](LICENSE)
