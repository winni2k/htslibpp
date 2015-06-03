/* @(#)synced_bcf_reader.hpp
 */

#ifndef _SYNCED_BCF_READER_HPP
#define _SYNCED_BCF_READER_HPP 1

#include <htslib/synced_bcf_reader.h>

#include <stdexcept>
#include <string>

namespace bcf_srs_helper {

struct init {
  std::string region;
  bool useIndex = false;
};
}

class bcf_srs {
private:
    void set_target(const std::string &region) {
    if (bcf_sr_set_targets(m_sr, region.c_str(), false, 0) != 0)
      throw std::runtime_error("Could not set target: [" + region + "]");
    assert(m_sr->require_index == 0);
  }
  void set_region(const std::string &region) {

    if (bcf_sr_set_regions(m_sr, region.c_str(), false) != 0)
      throw std::runtime_error("Could not set region: [" + region + "]");
    assert(m_sr->require_index == 1);
  }

protected:
  bcf_srs_t *m_sr = nullptr;
  size_t m_nFiles = 0;

public:
  ~bcf_srs() {
    if (m_sr)
      bcf_sr_destroy(m_sr);
  }
  bcf_srs() { m_sr = bcf_sr_init(); }
  bcf_srs(bcf_srs_helper::init &init) : bcf_srs() {
    if (!init.region.empty()) {
      if (init.useIndex)
        set_region(init.region);
      else
        set_target(init.region);
    }
  }

  bcf_srs_t *data() { return m_sr; }

  void add_reader(const std::string &filename) {
    if (bcf_sr_add_reader(m_sr, filename.c_str()) != 1)
      throw std::runtime_error("Error while adding file: [" + filename + "]");
    ++m_nFiles;
  }

  size_t size() const { return m_nFiles; }
  int next_line() { return bcf_sr_next_line(m_sr); }
  bool all_indexed() {
    bool allIndexed = true;
    for (size_t rNum = 0; rNum < m_nFiles; ++rNum)
      if (!indexed(rNum))
        allIndexed = false;
    return allIndexed;
  }
  bool indexed(size_t fileNum) {
    if (m_nFiles == 0)
      throw std::runtime_error("Synced reader is empty.  It does not make "
                               "sense to ask if any readers are indexed.");
    assert(fileNum < m_nFiles);
    auto &reader = m_sr->readers[fileNum];
    return (reader.tbx_idx != nullptr || reader.bcf_idx != nullptr);
  }
  bcf1_t *get_line(size_t fileNum) {
    if (fileNum >= m_nFiles)
      throw std::range_error("fileNum is too large");
    return bcf_sr_get_line(m_sr, fileNum);
  }

  bcf_hdr_t *get_header(size_t fileNum) {
    if (fileNum >= m_nFiles)
      throw std::range_error("fileNum is too large");
    return bcf_sr_get_header(m_sr, fileNum);
  }
};

#endif /* _SYNCED_BCF_READER_HPP */
