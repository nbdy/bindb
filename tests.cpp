//
// Created by nbdy on 01.11.21.
//

#include "test_common.h"

TEST(Database, construct) {
  Database db(TEST_DB_PATH);
  EXPECT_TRUE(db.isOpen());
  EXPECT_TRUE(deleteDatabase(&db));
}

TEST(Database, reopen) {
  {
    Database db(TEST_DB_PATH);
    EXPECT_TRUE(db.isOpen());
  }
  {
    Database db(TEST_DB_PATH);
    EXPECT_TRUE(db.isOpen());
    EXPECT_EQ(db.getEntryCount(), 0);
    deleteDatabase(&db);
  }
}

TEST(Database, find) {
  Database db(TEST_DB_PATH);
  db.insert(EntryOne {3, 3.14, "pi"});
  db.insert(EntryOne {1, 2.34, "abc"});
  db.insert(EntryOne {5, 9.87, "oops"});

  EntryOne e0 {};
  auto idx = db.find<EntryOne>(e0, [](const EntryOne& e) {
    return strcmp(e.m_String, "abc") == 0;
  });

  EXPECT_NE(idx, -1);
  EXPECT_TRUE(isEqual(e0.m_Float, 2.34));
  EXPECT_TRUE(deleteDatabase(&db));
}

TEST(Database, insert) {
  Database db(TEST_DB_PATH);

  EXPECT_TRUE(db.isOpen());

  insertEntryOneSingle(&db, 20);
  db.insert(EntryOne {1, 2.34, "abc"});
  insertEntryTwoSingle(&db, 7);
  // 28

  EXPECT_TRUE(db.isOpen());

  EntryOne e0 {};
  auto idx = db.find<EntryOne>(e0, [](const EntryOne& e) {
    return strcmp(e.m_String, "abc") == 0;
  });

  EXPECT_NE(idx, -1);
  EXPECT_EQ(idx, 20);
  EXPECT_TRUE(isEqual(e0.m_Float, 2.34));

  insertEntryThreeSingle(&db, 28);
  db.insert(EntryTwo {'p', 3.14}); // 28 + 29 + 1 = 58
  insertEntryOneSingle(&db, 69);
  // 58 + 69 = 127

  EntryTwo e1 {};
  idx = db.find<EntryTwo>(e1, [](const EntryTwo& e) {
    return e.m_Char == 'p';
  });

  EXPECT_NE(idx, -1);
  EXPECT_EQ(idx, 56);
  EXPECT_TRUE(isEqual(e1.m_Float, 3.14));

  std::cout << db.getEntryCount() << std::endl;

  EXPECT_TRUE(deleteDatabase(&db));
}

TEST(Database, reopenAndWrite) {
  {
    Database db(TEST_DB_PATH);
    EXPECT_TRUE(db.isOpen());
    insertEntryOneSingle(&db, 20);
  }
  {
    Database db(TEST_DB_PATH);
    EXPECT_TRUE(db.isOpen());
    insertEntryTwoSingle(&db, 50);
    EXPECT_EQ(db.getEntryCount(), 70);
    EXPECT_TRUE(deleteDatabase(&db));
  }
}

TEST(Database, findMultiple) {
  Database db(TEST_DB_PATH);
  EXPECT_TRUE(db.isOpen());
  insertEntryOneMultiple(&db, 222);
  db.insert(EntryTwo {'a', 4.2});
  insertEntryTwoMultiple(&db, 523);
  db.insert(EntryTwo {'b', 4.2});
  insertEntryThreeMultiple(&db, 234);
  db.insert(EntryTwo {'c', 4.2});
  auto results = db.findMultiple<EntryTwo>([](const EntryTwo& e) {
    return isEqual(e.m_Float, 4.2);
  });
  EXPECT_EQ(results.size(), 3);
  EXPECT_TRUE(deleteDatabase(&db));
}