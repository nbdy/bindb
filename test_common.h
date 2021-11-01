//
// Created by nbdy on 01.11.21.
//

#ifndef DBPP__TEST_COMMON_H_
#define DBPP__TEST_COMMON_H_

#include "bindb.h"

#include <cmath>
#include <gtest/gtest.h>
#include <iostream>

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
  std::string m_String;
};

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

void insertEntryOne(Database *db, uint32_t count = 1) {
  for(uint32_t i = 0; i < count; i++) {
    db->insert(EntryOne {generateRandomInteger(), generateRandomFloat(), generateRandomChar()});
  }
}

void insertEntryTwo(Database *db, uint32_t count = 1) {
  for(uint32_t i = 0; i < count; i++) {
    db->insert(EntryTwo {generateRandomChar(), generateRandomFloat()});
  }
}

void insertEntryThree(Database *db, uint32_t count = 1) {
  for(uint32_t i = 0; i < count; i++) {
    db->insert(EntryThree {generateRandomFloat(), generateRandomString()});
  }
}

#endif//DBPP__TEST_COMMON_H_
