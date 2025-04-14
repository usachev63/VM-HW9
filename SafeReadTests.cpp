#include "SafeRead.h"
#include "gtest/gtest.h"

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
