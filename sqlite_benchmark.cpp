//
// Created by nbdy on 03.11.21.
//

#include "test_common.h"

#include <sqlite3.h>

#define ENTRY_COUNT 100000U

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  for(auto i = 0; i < argc; i++) {
    std::cout << argv[i] << std::endl;
  }
  return 0;
}

sqlite3_stmt *stmt;
const char* insertSql = "INSERT INTO test_table (m_Int, m_Float, m_String) VALUES (?, ?, ?)";

void insertEntry(sqlite3* db, int32_t i, float f, const std::string& s) {
  auto rc = sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);
  rc = sqlite3_bind_int(stmt, 1, i); // generateRandomInteger()
  rc = sqlite3_bind_double(stmt, 2, f); // generateRandomFloat()
  rc = sqlite3_bind_text(stmt, 3, s.c_str(), s.size(), SQLITE_TRANSIENT); // generateRandomString().c_str()
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::cout << "Could not insert" << std::endl;
  }
  rc = sqlite3_finalize(stmt);
}

void insertRandomEntry(sqlite3* db) {
  insertEntry(db, generateRandomInteger(), generateRandomFloat(), generateRandomString());
}

void insertRandomEntries(sqlite3* db, uint32_t count) {
  auto p = std::rand() % count;
  for(uint32_t i = 0U; i < count; i++) {
    if(p == i) {
      insertEntry(db, 3, 4.44F, "EYYY THATS MY ENTRY1");
    } else {
      insertRandomEntry(db);
    }
  }
}

int main() {
  sqlite3 *db;
  char* error;
  int rc = sqlite3_open("sqlite_test.db", &db);
  std::cout << rc << std::endl;
  if(rc != SQLITE_OK) {
    std::cout << "Could not open db" << std::endl;
    (void)sqlite3_close(db);
    return 0;
  }
  rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS test_table(m_Int INTEGER, m_Float REAL, m_String TEXT)", callback, nullptr, &error);
  if(rc != SQLITE_OK) {
    std::cout << "Could not create test_table";
    (void)sqlite3_close(db);
    return 0;
  }

  std::cout << "inserting " << ENTRY_COUNT << " entries" << std::endl;
  TIMEIT_START("insert")
  insertRandomEntries(db, ENTRY_COUNT);
  TIMEIT_END
  TIMEIT_RESULT

  std::cout << "selecting" << std::endl;
  TIMEIT_START("select")
  rc = sqlite3_exec(db, "SELECT * FROM test_table WHERE m_String = 'EYYY THATS MY ENTRY1'", callback, nullptr, &error);
  TIMEIT_END
  TIMEIT_RESULT

  std::cout << "Done" << std::endl;
  return 0;
}