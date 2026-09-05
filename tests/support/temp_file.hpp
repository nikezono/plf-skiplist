#pragma once

#include <unistd.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <plf_skiplist/plf_skiplist.hpp>
#include <string>

namespace plf_skiplist_test {

// A fresh, unique path under the system temp directory for a PSkipList backing file.
// Removes whatever ends up at that path (if anything) when it goes out of scope.
class TempFile {
 public:
  explicit TempFile(const std::string &label) {
    static std::atomic<uint64_t> counter{0};
    const auto id = counter.fetch_add(1, std::memory_order_relaxed);
    path_ = std::filesystem::temp_directory_path() /
            ("plf_skiplist_test_" + label + "_" + std::to_string(::getpid()) + "_" + std::to_string(id));
  }
  ~TempFile() {
    std::filesystem::remove(path_);
    std::filesystem::remove(plf_skiplist::manifest_path(path_));
  }

  TempFile(const TempFile &) = delete;
  auto operator=(const TempFile &) -> TempFile & = delete;

  [[nodiscard]] auto path() const -> const std::filesystem::path & { return path_; }

 private:
  std::filesystem::path path_;
};

// Bytes needed to hold `usable_nodes` insertable slots plus the head/tail sentinels,
// matching how capacity was expressed before the mmap-backed constructor (in node
// counts) — keeps small-capacity tests (e.g. "capacity exhausted") easy to state exactly.
template <typename Key, typename Value = uint64_t>
[[nodiscard]] auto capacity_bytes_for_nodes(size_t usable_nodes) -> size_t {
  return (usable_nodes + 2) * sizeof(plf_skiplist::DurableNode<Key, Value>);
}

}  // namespace plf_skiplist_test
