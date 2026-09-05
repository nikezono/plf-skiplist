#include <doctest/doctest.h>

#include <cstdint>
#include <fstream>
#include <plf_skiplist/plf_skiplist.hpp>

#include "support/temp_file.hpp"

using plf_skiplist::kManifestFormatVersion;
using plf_skiplist::ManifestHeader;
using plf_skiplist::read_manifest;
using plf_skiplist::write_manifest;
using plf_skiplist_test::TempFile;

TEST_CASE("reading a manifest that was never written returns nullopt") {
  TempFile tmp("manifest_missing");
  CHECK_FALSE(read_manifest(tmp.path()).has_value());
}

TEST_CASE("a corrupted checksum is rejected") {
  TempFile tmp("manifest_bad_checksum");
  write_manifest(tmp.path(), 42, 1000);

  const auto path = plf_skiplist::manifest_path(tmp.path());
  ManifestHeader header{};
  {
    std::fstream file(path, std::ios::in | std::ios::binary);
    REQUIRE(file.read(reinterpret_cast<char *>(&header), sizeof(header)));
  }
  header.epoch += 1;  // corrupt the payload without recomputing the checksum
  {
    std::fstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char *>(&header), sizeof(header));
  }

  CHECK_FALSE(read_manifest(tmp.path()).has_value());
}

TEST_CASE("a mismatched format_version is rejected") {
  TempFile tmp("manifest_bad_version");
  write_manifest(tmp.path(), 42, 1000);

  const auto path = plf_skiplist::manifest_path(tmp.path());
  ManifestHeader header{};
  {
    std::fstream file(path, std::ios::in | std::ios::binary);
    REQUIRE(file.read(reinterpret_cast<char *>(&header), sizeof(header)));
  }
  header.format_version = static_cast<uint8_t>(kManifestFormatVersion + 1);
  header.checksum = plf_skiplist::manifest_checksum(header);
  {
    std::fstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char *>(&header), sizeof(header));
  }

  CHECK_FALSE(read_manifest(tmp.path()).has_value());
}

TEST_CASE("a truncated manifest is rejected") {
  TempFile tmp("manifest_truncated");
  write_manifest(tmp.path(), 42, 1000);

  const auto path = plf_skiplist::manifest_path(tmp.path());
  std::filesystem::resize_file(path, sizeof(ManifestHeader) - 1);

  CHECK_FALSE(read_manifest(tmp.path()).has_value());
}
