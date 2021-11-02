//
// Created by nbdy on 01.11.21.
//

#include "test_common.h"

void benchmarkSingleInsert(const char* name, uint32_t count) {
  Database db(TEST_DB_PATH);

  TIMEIT_START(name)
  insertEntryOneSingle(&db, count);
  TIMEIT_END
  TIMEIT_RESULT

  EXPECT_EQ(db.getEntryCount(), count);
  std::cout << db.getFileSize() << std::endl;

  EXPECT_TRUE(deleteDatabase(&db));
}

void benchmarkMultipleInsert(const char* name, uint32_t count, uint32_t chunkSize) {
  Database db(TEST_DB_PATH);

  TIMEIT_START(name)
  insertEntryOneMultiple(&db, count, chunkSize);
  TIMEIT_END
  TIMEIT_RESULT

  EXPECT_EQ(db.getEntryCount(), count);
  std::cout << db.getFileSize() << std::endl;

  EXPECT_TRUE(deleteDatabase(&db));
}

TEST(Database, benchmark1kInsert) {
  static uint32_t chunkSize = 10;
  static uint32_t count = 1000;
  benchmarkSingleInsert("benchmark1kSingleInsert", count);
  benchmarkMultipleInsert("benchmark1kMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark10kChunkSize10Insert) {
  static uint32_t chunkSize = 10;
  static uint32_t count = 10000;
  benchmarkSingleInsert("benchmark10kSingleInsert", count);
  benchmarkMultipleInsert("benchmark10kMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark100kChunkSize10Insert) {
  static uint32_t count = 100000;
  static uint32_t chunkSize = 10;
  benchmarkSingleInsert("benchmark100kSingleInsert", count);
  benchmarkMultipleInsert("benchmark100kMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark1mChunkSize10Insert) {
  static uint32_t count = 1000000;
  static uint32_t chunkSize = 10;
  benchmarkMultipleInsert("benchmark1mMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark10kChunkSize100Insert) {
  static uint32_t count = 10000;
  static uint32_t chunkSize = 100;
  benchmarkMultipleInsert("benchmark10kMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark100kChunkSize100Insert) {
  static uint32_t count = 100000;
  static uint32_t chunkSize = 100;
  benchmarkMultipleInsert("benchmark100kMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark1mChunkSize100Insert) {
  static uint32_t count = 1000000;
  static uint32_t chunkSize = 100;
  benchmarkMultipleInsert("benchmark1mMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark10kChunkSize1kInsert) {
  static uint32_t count = 10000;
  static uint32_t chunkSize = 1000;
  benchmarkMultipleInsert("benchmark10kMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark100kChunkSize1kInsert) {
  static uint32_t count = 100000;
  static uint32_t chunkSize = 1000;
  benchmarkMultipleInsert("benchmark100kMultipleInsert", count, chunkSize);
}

TEST(Database, benchmark1mChunkSize1kInsert) {
  static uint32_t count = 1000000;
  static uint32_t chunkSize = 1000;
  benchmarkMultipleInsert("benchmark1mMultipleInsert", count, chunkSize);
}

void benchmarkSearch(const char* name, uint32_t count) {
  Database db(TEST_DB_PATH);

  uint32_t ic = count / 4;

  insertEntryTwoMultiple(&db, ic - 1, 1000);
  insertEntryOneMultiple(&db, ic, 1000);
  insertEntryThreeMultiple(&db, ic, 1000);
  db.insert(EntryThree { 4.44, "AYOOO this is the right entry"});
  insertEntryTwoMultiple(&db, ic, 1000);

  EntryThree e {};
  TIMEIT_START(name)
  auto idx = db.find<EntryThree>(e, [](const EntryThree& entry) {
    return entry.m_Double == 4.44;
  });
  TIMEIT_END

  TIMEIT_RESULT

  EXPECT_NE(idx, -1);
  EXPECT_TRUE(strcmp(e.m_String, "AYOOO this is the right entry") == 0);
  EXPECT_TRUE(deleteDatabase(&db));
}

TEST(Database, benchmark10kSearch) {
  benchmarkSearch("benchmark10kSearch", 10000);
}

TEST(Database, benchmark100kSearch) {
  benchmarkSearch("benchmark100kSearch", 100000);
}

TEST(Database, benchmark1mSearch) {
  benchmarkSearch("benchmark1mSearch", 1000000);
}
