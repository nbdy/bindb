//
// Created by nbdy on 01.11.21.
//

#ifndef DBPP__TEST_COMMON_H_
#define DBPP__TEST_COMMON_H_

#include "bindb.h"

#include <cmath>
#include <gtest/gtest.h>
#include <iostream>

#define TEST_DB_PATH "test.db"

#define TIMESTAMP std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now())

std::string TIMEIT_NAME = "TIMEIT";
time_t TIMEIT_START_TIMESTAMP = 0;
time_t TIMEIT_END_TIMESTAMP = 0;
time_t TIMEIT_DIFF = 0;

#define TIMEIT_START(name) \
TIMEIT_NAME = name;        \
TIMEIT_START_TIMESTAMP = TIMESTAMP;

#define TIMEIT_END \
TIMEIT_END_TIMESTAMP = TIMESTAMP;

#define TIMEIT_RESULT \
TIMEIT_DIFF = TIMEIT_END_TIMESTAMP - TIMEIT_START_TIMESTAMP; \
std::cout << TIMEIT_NAME << ": " << std::to_string(TIMEIT_DIFF) << " s" << std::endl;

struct EntryOne {
  int m_Int;
  float m_Float;
  char m_String[64];
};

struct EntryTwo {
  char m_Char;
  float m_Float;
};

struct EntryThree {
  double m_Double;
  char m_String[42];
};

bool deleteDatabase(Database *db) {
#ifndef NDEBUG
  PFNC
#endif
  db->sync();
  remove(db->getFilePath());
  return !std::filesystem::exists(db->getFilePath());
}

bool isEqual(float x, float y) {
  return std::fabs(x - y) < std::numeric_limits<float>::epsilon();
}

char generateRandomChar(){
  return 'A' + std::rand() % 24; // NOLINT(cert-msc50-cpp,cppcoreguidelines-narrowing-conversions)
}

std::string generateRandomString(uint32_t count = 10) {
  std::stringstream r;
  for(uint32_t i = 0; i < count; i++) {
    r << generateRandomChar();
  }
  return r.str();
}

int generateRandomInteger() {
  return std::rand() % 10000000; // NOLINT(cert-msc50-cpp)
}

float generateRandomFloat() {
  return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); // NOLINT(cert-msc50-cpp)
}

void insertEntryOneSingle(Database *db, uint32_t count = 1) {
  for(uint32_t i = 0; i < count; i++) {
    db->insert(EntryOne {generateRandomInteger(), generateRandomFloat(), generateRandomChar()});
  }
}

void insertEntryTwoSingle(Database *db, uint32_t count = 1) {
  for(uint32_t i = 0; i < count; i++) {
    db->insert(EntryTwo {generateRandomChar(), generateRandomFloat()});
  }
}

void insertEntryThreeSingle(Database *db, uint32_t count = 1) {
  for(uint32_t i = 0; i < count; i++) {
    db->insert(EntryThree {generateRandomFloat(), generateRandomChar()});
  }
}

void insertEntryOneMultiple(Database *db, uint32_t count = 10, uint32_t chunkSize = 10) {
  std::vector<EntryOne> v;
  uint32_t l = count / chunkSize;
  for(uint32_t i = 0; i < l; i++) {
    for(uint32_t j = 0; j < chunkSize; j++) {
      v.push_back(EntryOne {generateRandomInteger(), generateRandomFloat(), generateRandomChar()});
    }
    db->insertMultiple(v);
    v.clear();
  }
}

void insertEntryTwoMultiple(Database *db, uint32_t count = 10, uint32_t chunkSize = 10) {
  std::vector<EntryTwo> v;
  uint32_t l = count / chunkSize;
  for(uint32_t i = 0; i < l; i++) {
    for(uint32_t j = 0; j < chunkSize; j++) {
      v.push_back(EntryTwo {generateRandomChar(), generateRandomFloat()});
    }
    db->insertMultiple(v);
  }
}

void insertEntryThreeMultiple(Database *db, uint32_t count = 10, uint32_t chunkSize = 10) {
  std::vector<EntryThree> v;
  uint32_t l = count / chunkSize;
  for(uint32_t i = 0; i < l; i++) {
    for(uint32_t j = 0; j < chunkSize; j++) {
      v.push_back(EntryThree {generateRandomFloat(), generateRandomChar()});
    }
    db->insertMultiple(v);
  }
}


#endif//DBPP__TEST_COMMON_H_
