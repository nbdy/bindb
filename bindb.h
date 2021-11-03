//
// Created by nbdy on 26.10.21.
//

#ifndef BINDB__BINDB_H_
#define BINDB__BINDB_H_

#ifndef NDEBUG
#define PFNC std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif

#ifndef EXPECTED_MAGIC
#define EXPECTED_MAGIC 420U
#endif

#define USE_MALLOC

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
  static constexpr auto m_u32EntryHeaderSize = sizeof(EntryHeader);

  class Header {
    uint32_t m_u32Magic = EXPECTED_MAGIC;
    uint32_t m_u32EntryCount = 0U;
    size_t m_EntryTypeCount = 0U;

   public:
    [[nodiscard]] uint32_t getMagic() const {
      return m_u32Magic;
    }

    [[nodiscard]] uint32_t getEntryCount() const {
      return m_u32EntryCount;
    }

    [[nodiscard]] size_t getEntryTypeCount() const {
      return m_EntryTypeCount;
    }

    void setMagic(uint32_t i_u32Value) {
      m_u32Magic = i_u32Value;
    }

    void setEntryCount(uint32_t i_u32Value) {
      m_u32EntryCount = i_u32Value;
    }

    void setEntryTypeCount(size_t i_Value) {
      m_EntryTypeCount = i_Value;
    }

    void incrementEntryCount(uint32_t i_u32Count = 1U) {
      m_u32EntryCount += i_u32Count;
    }
  };
  const static auto m_u32HeaderSize = sizeof(Header);

  struct EntryTypeDescription {
    size_t m_Hash;
    size_t m_EntrySize;
  };
  const static auto m_u32EntryTypeDescriptionSize = sizeof(EntryTypeDescription);

  const char* m_sDatabasePath = nullptr;
  uint32_t m_u32ExpectedMagic = EXPECTED_MAGIC;

  int m_iFileDescriptor = -1;
  bool m_bError = false;

  Header m_Header;

  std::vector<EntryTypeDescription> m_EntryDescriptions;
  std::vector<EntryHeader> m_EntryPositions;

  [[nodiscard]] uint64_t getEntryDescriptionsSize() const {
    return m_Header.getEntryTypeCount() * m_u32EntryTypeDescriptionSize;
  }

  std::map<EntryHeader, size_t> entryDescriptions2Map() {
    std::map<EntryHeader, size_t> r;
    for(const auto& d : m_EntryDescriptions) {
      r[d.m_Hash] = d.m_EntrySize;
    }
    return r;
  }

  void writeHeader() {
    (void)(lseek(m_iFileDescriptor, 0, SEEK_SET));
    (void)(write(m_iFileDescriptor, &m_Header, m_u32HeaderSize));
    sync();
  }

  void readHeader() {
    (void)(lseek(m_iFileDescriptor, 0, SEEK_SET));
    (void)(read(m_iFileDescriptor, &m_Header, m_u32HeaderSize));
  }

  void readEntryTypes() {
    m_EntryDescriptions.resize(m_Header.getEntryTypeCount());
    (void)(lseek(m_iFileDescriptor, m_u32HeaderSize, SEEK_SET));
    (void)(read(m_iFileDescriptor, &m_EntryDescriptions[0U], m_u32EntryTypeDescriptionSize * m_Header.getEntryTypeCount()));
  }

  void writeEntryTypes() {
    (void)(lseek(m_iFileDescriptor, m_u32HeaderSize, SEEK_SET));
    (void)(write(m_iFileDescriptor, &m_EntryDescriptions[0U], m_u32EntryTypeDescriptionSize * m_Header.getEntryTypeCount()));
    sync();
  }

  void index() {
    // Seek to first entry header
    auto firstHeaderOffset = m_u32HeaderSize + getEntryDescriptionsSize();

    if (firstHeaderOffset == m_u32HeaderSize) {
      return;
    }

    (void)(lseek(m_iFileDescriptor, firstHeaderOffset, SEEK_SET));
    // Iterate over all the entries
    EntryHeader hdr;
    for (uint32_t i = 0U; i < m_Header.getEntryCount(); i++) {
      (void) read(m_iFileDescriptor, &hdr, m_u32EntryHeaderSize);
      if (skip2Next(hdr)) {
        m_EntryPositions.push_back(hdr);
      }
    }
    if (m_EntryPositions.size() < m_Header.getEntryCount()) {
      std::cout << "!! Read " << std::to_string(m_EntryPositions.size()) << " out of " << std::to_string(m_Header.getEntryCount()) << " entries" << std::endl;
      std::cout << "!! Database might be corrupted" << std::endl;
    }
  }

  void init() {
    // Check if the file is empty
    if (getFileSize() == 0) {
      // File is empty, since we just created it. So let's also create the header.
      m_Header.setMagic(m_u32ExpectedMagic);
      writeHeader();
    } else {
      // File is not empty, read the header
      readHeader();

      m_bError = m_Header.getMagic() != m_u32ExpectedMagic;

      if (m_bError) {
        return; // There was an error, lets not proceed
      }
    }

    // Index entry descriptions
    if (!m_bError) {
      readEntryTypes();
    }

    // Index entry headers and positions
    if (!m_bError) {
      index();
    }

#ifndef NDEBUG
    std::cout << "Entry types: " << m_Header.getEntryTypeCount() << " | Entry count: " << m_Header.getEntryCount() << std::endl;
    std::cout << "Entry description:" << std::endl;
    for(const auto& e : m_EntryDescriptions) {
      std::cout << "\tHash: " << e.m_Hash << " | Size: " << e.m_EntrySize << std::endl;
    }
#endif
  }

  // TODO(nbdy): optimize this
  bool skip2Next(EntryHeader i_EntryHeader) {
    bool found = false;
    for (const auto& e : m_EntryDescriptions) {
      if (e.m_Hash == i_EntryHeader) {
        found = true;
        (void)(lseek(m_iFileDescriptor, e.m_EntrySize, SEEK_CUR));
        break;
      }
    }
    if (!found) {
      std::cout << "Could not find entry hash" << std::endl;
    }
    return found;
  }

  [[nodiscard]] uint64_t getBodyOffset() const {
    return m_u32HeaderSize + (m_u32EntryTypeDescriptionSize * m_Header.getEntryTypeCount());
  }

  uint64_t getBodySize() {
    auto s = getFileSize();
    auto o = getBodyOffset();
    if (s <= o) {
      return 0U;
    }
    return s - o;
  }

 public:
  explicit Database(const char* i_sDatabasePath) : m_sDatabasePath(i_sDatabasePath) {
    m_iFileDescriptor = open(m_sDatabasePath, O_CREAT | O_RDWR, 420);
    if(m_iFileDescriptor == -1) {
      std::cout << "Could not open " << m_sDatabasePath << std::endl;
    }
    init();
  }

  /*
  Database(const char* i_sDatabasePath, uint32_t i_u32Magic) : m_sDatabasePath(i_sDatabasePath), m_u32ExpectedMagic(i_u32Magic) {
    _init();
  }
  */

  ~Database() {
    if(m_iFileDescriptor != -1) {
      sync();
      (void)(close(m_iFileDescriptor));
    }
  }

  [[nodiscard]] off_t getFileSize() const {
    auto r = lseek(m_iFileDescriptor, 0, SEEK_END);

    if(r == -1) {
      return 0;
    }

    return r;
  }

  [[nodiscard]] uint32_t getEntryCount() const {
    return m_Header.getEntryCount();
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
#ifdef USE_MALLOC
    void *body = malloc(bodySize + 1U);
#else
    char body[bodySize + 1U];
#endif

    auto bodyOffset = getBodyOffset();
    (void)(lseek(m_iFileDescriptor, bodyOffset, SEEK_SET));

    if (bodySize > 0U) {
#ifndef NDEBUG
      std::cout << __PRETTY_FUNCTION__ << " Reading body (" << bodySize << " bytes)" << std::endl;
#endif
#ifdef USE_MALLOC
      (void)(read(m_iFileDescriptor, body, bodySize));
#else
      (void)(read(m_iFileDescriptor, &body, bodySize));
#endif
    }

    // set the new entry type count after we read the body
    // since getBodySize internally depends on m_Header.m_u32EntryTypeCount
    m_Header.setEntryTypeCount(m_EntryDescriptions.size());

    // write the header with the incremented entry type count
    writeHeader();
    writeEntryTypes();

    bodyOffset = getBodyOffset();

    if(bodySize > 0U) {
#ifndef NDEBUG
      std::cout << __PRETTY_FUNCTION__ << " Writing body (" << bodySize << " bytes)" << std::endl;
#endif
      (void)(lseek(m_iFileDescriptor, bodyOffset, SEEK_SET));
#ifdef USE_MALLOC
      (void)(write(m_iFileDescriptor, body, bodySize));
#else
      (void)(write(m_iFileDescriptor, &body, bodySize));
#endif
    }

    sync();
#ifdef USE_MALLOC
    free(body);
#endif
  }

  template<typename EntryType>
  void insert(EntryType i_Entry) {
    auto hdr = typeid(EntryType).hash_code();

    if (hasType<EntryType>() == false) {
      addType<EntryType>();
    }

    m_EntryPositions.push_back(hdr);

    m_Header.incrementEntryCount();
    writeHeader();

    std::vector<std::pair<EntryHeader, EntryType>> v;
    v.push_back(std::make_pair(hdr, i_Entry));

    (void)(lseek(m_iFileDescriptor, 0, SEEK_END));
    (void)(write(m_iFileDescriptor, &v[0], sizeof(std::pair<EntryHeader, EntryType>)));
    sync();
  }

  template<typename EntryType>
  void insertMultiple(std::vector<EntryType> i_Entries) {
    auto hdr = typeid(EntryType).hash_code();

    if (hasType<EntryType>() == false) {
      addType<EntryType>();
    }

    std::vector<EntryHeader> hdrs(i_Entries.size(), hdr);
    (void) m_EntryPositions.insert(m_EntryPositions.end(), hdrs.begin(), hdrs.end());
    m_Header.incrementEntryCount(i_Entries.size());

    std::vector<std::pair<EntryHeader, EntryType>> writeVector;
    std::for_each(i_Entries.begin(), i_Entries.end(), [&writeVector, hdr](const auto& e) {
      writeVector.push_back(std::make_pair(hdr, e));
    });

    (void) write(m_iFileDescriptor, &writeVector[0], sizeof(std::pair<EntryHeader, EntryType>) * writeVector.size());
    sync();
  }

  template<typename EntryType>
  int64_t find(EntryType& i_Entry, std::function<bool(const EntryType&)> i_FilterFunction) {
    auto hdrHash = typeid(EntryType).hash_code();
    int64_t idx = 0;
    EntryType tmp {};
    // skip to first entry
    auto bodyOffset = getBodyOffset();
    auto em = entryDescriptions2Map();
    (void) lseek(m_iFileDescriptor, bodyOffset, SEEK_SET);
    // iterate over positions
    for (const auto& p : m_EntryPositions) {
      bodyOffset += m_u32EntryHeaderSize;
      if (p == hdrHash) {
        (void) lseek(m_iFileDescriptor, bodyOffset, SEEK_SET);
        (void) read(m_iFileDescriptor, &tmp, em[p]);
        if(i_FilterFunction(tmp)) {
          i_Entry = tmp;
          return idx;
        }
      }
      bodyOffset += em[p];
      idx++;
    }

    return -1;
  }

  template<typename EntryType>
  std::vector<EntryType> findMultiple(std::function<bool(const EntryType&)> i_FilterFunction) {
    std::vector<EntryType> r;
    auto hdrHash = typeid(EntryType).hash_code();
    int64_t idx = 0;
    EntryType tmp {};
    // skip to first entry
    auto bodyOffset = getBodyOffset();
    auto em = entryDescriptions2Map();
    (void) lseek(m_iFileDescriptor, bodyOffset, SEEK_SET);
    // iterate over positions
    for (const auto& p : m_EntryPositions) {
      bodyOffset += m_u32EntryHeaderSize;
      if (p == hdrHash) {
        (void) lseek(m_iFileDescriptor, bodyOffset, SEEK_SET);
        (void) read(m_iFileDescriptor, &tmp, em[p]);
        if(i_FilterFunction(tmp)) {
          r.push_back(tmp);
        }
      }
      bodyOffset += em[p];
      idx++;
    }

    return r;
  }

  [[nodiscard]] bool isOpen() const {
    return m_iFileDescriptor != -1;
  }

  const char* getFilePath() {
    return m_sDatabasePath;
  }

  void sync() const {
    if(fsync(m_iFileDescriptor) == -1) {
      std::cout << "sync error: " << strerror(errno) << std::endl;
    }
  }
};


class AsyncDatabase : public Database {
  explicit AsyncDatabase(const char* i_sDatabasePath) : Database(i_sDatabasePath) {};
};

#endif//BINDB__BINDB_H_
