#include "SafeRead.h"
#include "gtest/gtest.h"
#include <sys/mman.h>

TEST(SafeRead, StackPointer) {
  uint8_t x = 10;
  ASSERT_EQ(std::optional<uint8_t>(10), safe_read_uint8(&x));
}

TEST(SafeRead, HeapPointer) {
  auto u = std::make_unique<uint8_t>(8);
  ASSERT_EQ(std::optional<uint8_t>(8), safe_read_uint8(u.get()));
}

TEST(SafeRead, StaticLocalPointer) {
  static uint8_t g = 11;
  ASSERT_EQ(std::optional<uint8_t>(11), safe_read_uint8(&g));
}

uint8_t glob3 = 3;

TEST(SafeRead, GlobalPointer) {
  ASSERT_EQ(std::optional<uint8_t>(3), safe_read_uint8(&glob3));
}

static uint8_t glob4 = 4;

TEST(SafeRead, StaticGlobalPointer) {
  ASSERT_EQ(std::optional<uint8_t>(4), safe_read_uint8(&glob4));
}

TEST(SafeRead, NullPointer) {
  ASSERT_EQ(std::nullopt, safe_read_uint8(nullptr));
}

TEST(SafeRead, VeryLowPointers) {
  for (long i = 1; i < 8192; ++i)
    ASSERT_EQ(std::nullopt, safe_read_uint8((uint8_t *)i));
}

TEST(SafeRead, VeryHighPointers) {
  unsigned long long top = (unsigned long long)(-1);
  for (int i = 0; i < 8192; ++i)
    ASSERT_EQ(std::nullopt, safe_read_uint8((uint8_t *)(top - i)));
}

TEST(SafeRead, UseAfterFree) {
  uint8_t *p;
  {
    auto u = std::make_unique<uint8_t>(8);
    p = u.get();
  }
  safe_read_uint8(p); // does not crash
}

TEST(SafeRead, UseStackShifted) {
  uint8_t x;
  safe_read_uint8(&x + 1024 * 1024); // does not crash
}

TEST(SafeRead, MisalignedAccess) {
  int x;
  safe_read_uint8((uint8_t *)&x + 1); // does not crash
}

TEST(SafeRead, ProtectedPage) {
  const size_t page_size = sysconf(_SC_PAGESIZE);
  void *ptr =
      mmap(nullptr, page_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  safe_read_uint8(static_cast<uint8_t *>(ptr)); // does not crash
  munmap(ptr, page_size);
}

TEST(SafeRead, X) {
  const size_t page_size = sysconf(_SC_PAGESIZE);
  int fd = memfd_create("test", 0);
  ftruncate(fd, 4096);
  uint8_t *mapping =
      (uint8_t *)mmap(nullptr, page_size, PROT_READ, MAP_SHARED, fd, 0);
  safe_read_uint8(mapping + 4096); // does not crash
  munmap(mapping, page_size);
}
