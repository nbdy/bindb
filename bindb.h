//
// Created by nbdy on 26.10.21.
//

#ifndef DBPP__DBPP_H_
#define DBPP__DBPP_H_

#ifndef NDEBUG
#define PFNC std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

#ifndef EXPECTED_MAGIC
#define EXPECTED_MAGIC 420
#endif

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <functional>
#include <fstream>
#include <filesystem>


class Database {
  using EntryHeader = size_t;
  static constexpr uint32_t m_u32EntryHeaderSize = sizeof(EntryHeader);

  struct Header {
    uint32_t m_u32Magic = EXPECTED_MAGIC;
    uint32_t m_u32EntryCount = 0;
    uint32_t m_u32EntryTypeCount = 0;
  };
  const static uint32_t m_u32HeaderSize = sizeof(Header);

  struct EntryTypeDescription {
    size_t m_Hash;
    uint32_t m_u32EntrySize;
  };
  const static uint32_t m_u32EntryTypeDescriptionSize = sizeof(EntryTypeDescription);

  const char* m_sDatabasePath = nullptr;
  uint32_t m_u32ExpectedMagic = EXPECTED_MAGIC;

  std::fstream m_FileStream;
  bool m_bError = false;

  Header m_Header;

  std::vector<EntryTypeDescription> m_EntryDescriptions;
  std::vector<EntryHeader> m_EntryPositions;

  uint32_t getEntryDescriptionsSize() const {
    return m_Header.m_u32EntryTypeCount * m_u32EntryTypeDescriptionSize;
  }

  std::map<EntryHeader, uint32_t> entryDescriptions2Map() {
    std::map<EntryHeader, uint32_t> r;
    for(const auto& d : m_EntryDescriptions) {
      r[d.m_Hash] = d.m_u32EntrySize;
    }
    return r;
  }

  void _writeHeader() {
    m_FileStream.seekp(0, std::ios::beg);
    m_FileStream.write(reinterpret_cast<const char*>(&m_Header), m_u32HeaderSize);
    sync();
  }

  void _readHeader() {
    m_FileStream.seekg(0, std::ios::beg);
    m_FileStream.read(reinterpret_cast<char*>(&m_Header), m_u32HeaderSize);
  }

  void _readEntryTypes() {
    m_EntryDescriptions.resize(m_Header.m_u32EntryTypeCount);
    m_FileStream.seekg(m_u32HeaderSize, std::ios::beg);
    m_FileStream.read(reinterpret_cast<char*>(&m_EntryDescriptions[0]), getEntryDescriptionsSize());
  }

  void _writeEntryTypes() {
    m_FileStream.seekp(m_u32HeaderSize, std::ios::beg);
    m_FileStream.write(reinterpret_cast<const char*>(&m_EntryDescriptions[0]), getEntryDescriptionsSize());
    sync();
  }

  void _index() {
    // Seek to first entry header
    auto firstHeaderOffset = m_u32HeaderSize + getEntryDescriptionsSize();

    if (firstHeaderOffset == m_u32HeaderSize) {
      return;
    }

    m_FileStream.seekg(firstHeaderOffset, std::ios::beg);
    // Iterate over all the entries
    EntryHeader eHdr;
    for (uint32_t idx = 0; idx < m_Header.m_u32EntryCount; idx++) {
      m_FileStream.read(reinterpret_cast<char*>(&eHdr), m_u32EntryHeaderSize);
      if (_skip2Next(eHdr)) {
        m_EntryPositions.push_back(eHdr);
      }
    }
    if (m_EntryPositions.size() < m_Header.m_u32EntryCount) {
      std::cout << "!! Read " << std::to_string(m_EntryPositions.size()) << " out of " << std::to_string(m_Header.m_u32EntryCount) << " entries" << std::endl;
      std::cout << "!! Database might be corrupted" << std::endl;
    }
  }

  void _init() {
    memset(&m_Header, 0, m_u32HeaderSize);

    // Check if the file is empty
    if (getFileSize() == 0) {
      // File is empty, since we just created it. So let's also create the header.
      m_Header.m_u32EntryCount = 0;
      m_Header.m_u32EntryTypeCount = 0;
      m_Header.m_u32Magic = m_u32ExpectedMagic;
      _writeHeader();
    } else {
      // File is not empty, read the header
      _readHeader();
      if (m_bError) {
        return;// There was an error, lets not proceed
      }
    }

    // Index entry descriptions
    if (!m_bError) {
      _readEntryTypes();
    }

    // Index entry headers and positions
    if (!m_bError) {
      _index();
    }
  }

  // TODO(nbdy): optimize this
  bool _skip2Next(EntryHeader i_EntryHeader) {
    bool found = false;
    for (const auto& e : m_EntryDescriptions) {
      if (e.m_Hash == i_EntryHeader) {
        found = true;
        m_FileStream.seekg(e.m_u32EntrySize, std::ios::cur);
        break;
      }
    }
    if (!found) {
      std::cout << "Could not find entry hash" << std::endl;
    }
    return found;
  }

  uint32_t getBodyOffset() const {
    return m_u32HeaderSize + (m_u32EntryTypeDescriptionSize * m_Header.m_u32EntryTypeCount);
  }

  uint32_t getBodySize() {
    auto s = getFileSize();
    auto o = getBodyOffset();
    if (s <= o) {
      return 0;
    }
    return s - o;
  }

  void _createFile() {
    m_FileStream.open(m_sDatabasePath, std::fstream::out | std::fstream::binary);
    m_FileStream.close();
  }

 public:
  explicit Database(const char* i_sDatabasePath) : m_sDatabasePath(i_sDatabasePath) {
    if(!std::filesystem::exists(m_sDatabasePath)) {
      _createFile();
    }

    m_FileStream.open(m_sDatabasePath, std::fstream::out | std::fstream::in | std::fstream::binary);
    if(!m_FileStream.is_open()) {
      std::cout << "Could not open " << m_sDatabasePath << std::endl;
    }

    _init();
  }

  /*
  Database(const char* i_sDatabasePath, uint32_t i_u32Magic) : m_sDatabasePath(i_sDatabasePath), m_u32ExpectedMagic(i_u32Magic) {
    _init();
  }
  */

  ~Database() {
    sync();
    m_FileStream.close();
  }

  uint32_t getFileOffset() {
    return m_FileStream.tellg();
  }

  uint32_t getFileSize() {
    m_FileStream.seekg(0, std::ios::end);
    auto r = m_FileStream.tellg();

    if(r == -1) {
      return 0;
    }

    return r;
  }

  static uint32_t getHeaderSize() {
    return m_u32HeaderSize;
  }

  uint32_t getEntryCount() const {
    return m_Header.m_u32EntryCount;
  }

  template<typename EntryType>
  bool hasType() {
    auto h = typeid(EntryType).hash_code();
    return std::any_of(m_EntryDescriptions.begin(), m_EntryDescriptions.end(), [h](const EntryTypeDescription& e) {
      return h == e.m_Hash;
    });
  }

  bool hasError() const {
    return m_FileStream.fail();
  }

  template<typename EntryType>
  void addType() {
#ifndef NDEBUG
    PFNC
#endif

    m_EntryDescriptions.push_back(EntryTypeDescription {
        typeid(EntryType).hash_code(), sizeof(EntryType)
    });

    // read body into buffer, so we can move it further back
    auto bodySize = getBodySize();
    char body[bodySize + 1];

    auto bodyOffset = getBodyOffset();
    m_FileStream.seekg(bodyOffset, std::ios::beg);

    if (bodySize > 0) {
#ifndef NDEBUG
      std::cout << __PRETTY_FUNCTION__ << " Reading body (" << bodySize << " bytes)" << std::endl;
#endif
      m_FileStream.read(reinterpret_cast<char*>(body), bodySize);
    }

    // set the new entry type count after we read the body
    // since getBodySize internally depends on m_Header.m_u32EntryTypeCount
    m_Header.m_u32EntryTypeCount = m_EntryDescriptions.size();

    // write the header with the incremented entry type count
    _writeHeader();
    _writeEntryTypes();

    bodyOffset = getBodyOffset();

    if(bodySize > 0) {
      m_FileStream.seekp(bodySize, std::ios::beg);
      m_FileStream.write(reinterpret_cast<const char*>(body), bodySize);
    }

    sync();
  }

  template<typename EntryType>
  void insert(EntryType entry) {
    auto entrySize = sizeof(EntryType);

    if (!hasType<EntryType>()) {
      addType<EntryType>();
    }

    auto hdr = typeid(EntryType).hash_code();
    m_EntryPositions.push_back(hdr);

    m_FileStream.seekp(0, std::ios::beg);
    m_FileStream.write(reinterpret_cast<const char*>(&hdr), m_u32EntryHeaderSize);
    m_FileStream.write(reinterpret_cast<const char*>(&entry), (long) entrySize);
    sync();
  }

  template<typename EntryType>
  void insertMultiple(std::vector<EntryType> entries) {
    for(const auto& e : entries) {
      insert(e);
    }
  }

  template<typename EntryType>
  int64_t find(EntryType& entry, std::function<bool(const EntryType&)> i_FilterFunction) {
    auto hdrHash = typeid(EntryType).hash_code();
    int64_t idx = 0;
    size_t tmpSize = 0;
    EntryType tmp {};
    // skip to first entry
    auto bodyOffset = getBodyOffset();
    auto em = entryDescriptions2Map();
    // iterate over positions
    for (const auto& p : m_EntryPositions) {
      if(m_FileStream.eof()) {
        std::cout << "eof" << std::endl;
        break;
      }
      bodyOffset += m_u32EntryHeaderSize;
      m_FileStream.seekg(bodyOffset, std::fstream::beg);
      auto g = m_FileStream.tellg();
      tmpSize = em[p];
      if (p == hdrHash) {
        m_FileStream.read(reinterpret_cast<char*>(&tmp), (long) tmpSize);
        if(i_FilterFunction(tmp)) {
          entry = tmp;
          return idx;
        }
      }
      bodyOffset += tmpSize;
      idx++;
    }

    return -1;
  }

  bool isOpen() const {
    return m_FileStream.is_open();
  }

  const char* getFilePath() {
    return m_sDatabasePath;
  }

  bool deleteDatabase() {
#ifndef NDEBUG
    PFNC
#endif
    sync();
    m_FileStream.close();
    bool r = remove(m_sDatabasePath) == 0;
    return r;
  }

  void sync() {
    if(m_FileStream.sync() == -1){
      std::cout << __PRETTY_FUNCTION__ << " sync error: " << strerror(errno) << std::endl;
    }
  }
};

#endif//DBPP__DBPP_H_
