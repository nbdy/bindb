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
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <unistd.h>
#include <vector>


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

  int m_iFileDescriptor = -1;
  bool m_bError = false;

  Header m_Header;

  std::vector<EntryTypeDescription> m_EntryDescriptions;
  std::vector<EntryHeader> m_EntryPositions;

  [[nodiscard]] uint32_t getEntryDescriptionsSize() const {
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
    lseek(m_iFileDescriptor, 0, SEEK_SET);
    write(m_iFileDescriptor, &m_Header, m_u32HeaderSize);
    sync();
  }

  void _readHeader() {
    lseek(m_iFileDescriptor, 0, SEEK_SET);
    read(m_iFileDescriptor, &m_Header, m_u32HeaderSize);
  }

  void _readEntryTypes() {
    m_EntryDescriptions.resize(m_Header.m_u32EntryTypeCount);
    lseek(m_iFileDescriptor, m_u32HeaderSize, SEEK_SET);
    read(m_iFileDescriptor, &m_EntryDescriptions[0], m_u32EntryTypeDescriptionSize * m_Header.m_u32EntryTypeCount);
  }

  void _writeEntryTypes() {
    lseek(m_iFileDescriptor, m_u32HeaderSize, SEEK_SET);
    write(m_iFileDescriptor, &m_EntryDescriptions[0], m_u32EntryTypeDescriptionSize * m_Header.m_u32EntryTypeCount);
    sync();
  }

  void _index() {
    // Seek to first entry header
    auto firstHeaderOffset = m_u32HeaderSize + getEntryDescriptionsSize();

    if (firstHeaderOffset == m_u32HeaderSize) {
      return;
    }

    lseek(m_iFileDescriptor, firstHeaderOffset, SEEK_SET);
    // Iterate over all the entries
    EntryHeader eHdr;
    for (uint32_t idx = 0; idx < m_Header.m_u32EntryCount; idx++) {
      read(m_iFileDescriptor, &eHdr, m_u32EntryHeaderSize);
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

#ifndef NDEBUG
    std::cout << "Entry types: " << m_Header.m_u32EntryTypeCount << " | Entry count: " << m_Header.m_u32EntryCount << std::endl;
    std::cout << "Entry description:" << std::endl;
    for(const auto& e : m_EntryDescriptions) {
      std::cout << "\tHash: " << e.m_Hash << " | Size: " << e.m_u32EntrySize << std::endl;
    }
#endif
  }

  // TODO(nbdy): optimize this
  bool _skip2Next(EntryHeader i_EntryHeader) {
    bool found = false;
    for (const auto& e : m_EntryDescriptions) {
      if (e.m_Hash == i_EntryHeader) {
        found = true;
        lseek(m_iFileDescriptor, e.m_u32EntrySize, SEEK_CUR);
        break;
      }
    }
    if (!found) {
      std::cout << "Could not find entry hash" << std::endl;
    }
    return found;
  }

  [[nodiscard]] uint32_t getBodyOffset() const {
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

 public:
  explicit Database(const char* i_sDatabasePath) : m_sDatabasePath(i_sDatabasePath) {
    m_iFileDescriptor = open(m_sDatabasePath, O_CREAT | O_RDWR, 0644);
    if(m_iFileDescriptor == -1) {
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
    if(m_iFileDescriptor != -1) {
      sync();
      close(m_iFileDescriptor);
    }
  }

  [[nodiscard]] uint32_t getFileSize() const {
    auto r = lseek(m_iFileDescriptor, 0, SEEK_END);

    if(r == -1) {
      return 0;
    }

    return r;
  }

  static uint32_t getHeaderSize() {
    return m_u32HeaderSize;
  }

  [[nodiscard]] uint32_t getEntryCount() const {
    return m_Header.m_u32EntryCount;
  }

  template<typename EntryType>
  bool hasType() {
    auto h = typeid(EntryType).hash_code();
    return std::any_of(m_EntryDescriptions.begin(), m_EntryDescriptions.end(), [h](const EntryTypeDescription& e) {
      return h == e.m_Hash;
    });
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
    lseek(m_iFileDescriptor, bodyOffset, SEEK_SET);

    if (bodySize > 0) {
#ifndef NDEBUG
      std::cout << __PRETTY_FUNCTION__ << " Reading body (" << bodySize << " bytes)" << std::endl;
#endif
      read(m_iFileDescriptor, &body, bodySize);
    }

    // set the new entry type count after we read the body
    // since getBodySize internally depends on m_Header.m_u32EntryTypeCount
    m_Header.m_u32EntryTypeCount = m_EntryDescriptions.size();

    // write the header with the incremented entry type count
    _writeHeader();
    _writeEntryTypes();

    bodyOffset = getBodyOffset();

    if(bodySize > 0) {
#ifndef NDEBUG
      std::cout << __PRETTY_FUNCTION__ << " Writing body (" << bodySize << " bytes)" << std::endl;
#endif
      lseek(m_iFileDescriptor, bodyOffset, SEEK_SET);
      write(m_iFileDescriptor, &body, bodySize);
    }

    sync();
  }

  template<typename EntryType>
  void insert(EntryType entry) {
    auto entrySize = sizeof(EntryType);
    auto hdr = typeid(EntryType).hash_code();

    if (!hasType<EntryType>()) {
      addType<EntryType>();
    }

    m_EntryPositions.push_back(hdr);

    m_Header.m_u32EntryCount++;
    _writeHeader();

    lseek(m_iFileDescriptor, 0, SEEK_END);
    write(m_iFileDescriptor, &hdr, m_u32EntryHeaderSize);
    write(m_iFileDescriptor, &entry, entrySize);
    sync();
  }

  template<typename EntryType>
  void insertMultiple(std::vector<EntryType> entries, uint32_t chunkSize = 1000) {
    auto entrySize = sizeof(EntryType);
    auto hdr = typeid(EntryType).hash_code();

    if (!hasType<EntryType>()) {
      addType<EntryType>();
    }

    std::vector<EntryHeader> hdrs(entries.size(), hdr);
    m_EntryPositions.insert(m_EntryPositions.end(), hdrs.begin(), hdrs.end());
    m_Header.m_u32EntryCount += entries.size();

    std::vector<std::pair<EntryHeader, EntryType>> writeVector;
    for(const auto& e: entries) {
      writeVector.template emplace_back(hdr, e);
    }

    write(m_iFileDescriptor, &writeVector[0], (m_u32EntryHeaderSize + entrySize) * writeVector.size());
    sync();
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
    lseek(m_iFileDescriptor, bodyOffset, SEEK_SET);
    // iterate over positions
    for (const auto& p : m_EntryPositions) {
      bodyOffset += m_u32EntryHeaderSize;
      lseek(m_iFileDescriptor, m_u32EntryHeaderSize, SEEK_CUR);
      tmpSize = em[p];
      if (p == hdrHash) {
        read(m_iFileDescriptor, &tmp, tmpSize);
        if(i_FilterFunction(tmp)) {
          entry = tmp;
          return idx;
        }
      } else {
        lseek(m_iFileDescriptor, (long) tmpSize, SEEK_CUR);
      }
      bodyOffset += tmpSize;
      idx++;
    }

    return -1;
  }

  [[nodiscard]] bool isOpen() const {
    return m_iFileDescriptor != -1;
  }

  const char* getFilePath() {
    return m_sDatabasePath;
  }

  bool deleteDatabase() {
#ifndef NDEBUG
    PFNC
#endif
    if(m_iFileDescriptor != -1) {
      close(m_iFileDescriptor);
      m_iFileDescriptor = -1;
      return remove(m_sDatabasePath) == 0;
    }
    return false;
  }

  void sync() const {
    if(fsync(m_iFileDescriptor) == -1) {
      std::cout << __PRETTY_FUNCTION__ << " sync error: " << strerror(errno) << std::endl;
    }
  }
};

#endif//DBPP__DBPP_H_
