//
// Created by nbdy on 01.11.21.
//

#include "test_common.h"

#define TEST_DB_PATH "test.db"


TEST(Database, construct) {
  Database db(TEST_DB_PATH);
  EXPECT_TRUE(db.isOpen());
  EXPECT_TRUE(db.deleteDatabase());
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
    db.deleteDatabase();
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
  EXPECT_TRUE(db.deleteDatabase());
}

TEST(Database, insert) {
  Database db(TEST_DB_PATH);

  EXPECT_TRUE(db.isOpen());

  db.insert(EntryOne {1, 2.34, "abc"});
  insertEntryOne(&db, 20);
  insertEntryTwo(&db, 7);
  // 28

  EXPECT_TRUE(db.isOpen());

  EntryOne e0 {};
  auto idx = db.find<EntryOne>(e0, [](const EntryOne& e) {
    return strcmp(e.m_String, "abc") == 0;
  });

  EXPECT_NE(idx, -1);
  EXPECT_EQ(idx, 21);
  EXPECT_TRUE(isEqual(e0.m_Float, 2.34));

  insertEntryThree(&db, 29);
  db.insert(EntryTwo {'p', 3.14}); // 28 + 29 + 1 = 58
  insertEntryOne(&db, 69);
  // 58 + 69 = 127

  EntryTwo e1 {};
  idx = db.find<EntryTwo>(e1, [](const EntryTwo& e) {
    return e.m_Char == 'x';
  });

  EXPECT_NE(idx, -1);
  EXPECT_EQ(idx, 58);
  EXPECT_TRUE(isEqual(e0.m_Float, 3.14));

  EXPECT_TRUE(db.deleteDatabase());
}
