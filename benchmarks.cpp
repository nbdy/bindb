//
// Created by nbdy on 01.11.21.
//

#include "test_common.h"

void benchmarkSingle(const char* name, uint32_t count) {
  Database db(TEST_DB_PATH);

  TIMEIT_START(name)
  insertEntryOneSingle(&db, count);
  TIMEIT_END
  TIMEIT_RESULT

  EXPECT_EQ(db.getEntryCount(), count);
  std::cout << db.getFileSize() << std::endl;

  EXPECT_TRUE(db.deleteDatabase());
}

void benchmarkMultiple(const char* name, uint32_t count, uint32_t chunkSize) {
  Database db(TEST_DB_PATH);

  TIMEIT_START(name)
  insertEntryOneMultiple(&db, count, chunkSize);
  TIMEIT_END
  TIMEIT_RESULT

  EXPECT_EQ(db.getEntryCount(), count);
  std::cout << db.getFileSize() << std::endl;

  EXPECT_TRUE(db.deleteDatabase());
}

TEST(Database, benchmark1k) {
  static uint32_t chunkSize = 10;
  static uint32_t count = 1000;
  benchmarkSingle("benchmark1kSingle", count);
  benchmarkMultiple("benchmark1kMultiple", count, chunkSize);
}

TEST(Database, benchmark10kChunkSize10) {
  static uint32_t chunkSize = 10;
  static uint32_t count = 10000;
  benchmarkSingle("benchmark10kSingle", count);
  benchmarkMultiple("benchmark10kMultiple", count, chunkSize);
}

TEST(Database, benchmark100kChunkSize10) {
  static uint32_t count = 100000;
  static uint32_t chunkSize = 10;
  benchmarkSingle("benchmark100kSingle", count);
  benchmarkMultiple("benchmark100kMultiple", count, chunkSize);
}

TEST(Database, benchmark1mChunkSize10) {
  static uint32_t count = 1000000;
  static uint32_t chunkSize = 10;
  benchmarkMultiple("benchmark1mMultiple", count, chunkSize);
}

TEST(Database, benchmark10kChunkSize100) {
  static uint32_t count = 10000;
  static uint32_t chunkSize = 100;
  benchmarkMultiple("benchmark10kMultiple", count, chunkSize);
}

TEST(Database, benchmark100kChunkSize100) {
  static uint32_t count = 100000;
  static uint32_t chunkSize = 100;
  benchmarkMultiple("benchmark100kMultiple", count, chunkSize);
}

TEST(Database, benchmark1mChunkSize100) {
  static uint32_t count = 1000000;
  static uint32_t chunkSize = 100;
  benchmarkMultiple("benchmark1mMultiple", count, chunkSize);
}

TEST(Database, benchmark10kChunkSize1k) {
  static uint32_t count = 10000;
  static uint32_t chunkSize = 1000;
  benchmarkMultiple("benchmark10kMultiple", count, chunkSize);
}

TEST(Database, benchmark100kChunkSize1k) {
  static uint32_t count = 100000;
  static uint32_t chunkSize = 1000;
  benchmarkMultiple("benchmark100kMultiple", count, chunkSize);
}

TEST(Database, benchmark1mChunkSize1k) {
  static uint32_t count = 1000000;
  static uint32_t chunkSize = 1000;
  benchmarkMultiple("benchmark1mMultiple", count, chunkSize);
}