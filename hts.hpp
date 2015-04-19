/* @(#)hts.hpp
 */

#ifndef _HTS_HPP
#define _HTS_HPP 1

#include <string>
#include <stdexcept>

class htslib_error : public std::runtime_error {};

class htsFile_cpp {
protected:
  htsFile *m_file = nullptr;

public:
  ~htsFile_cpp() {
    if (m_file)
      hts_close(m_file);
  }
  htsFile_cpp() = default;
  htsFile_cpp(const std::string &fileName, const std::string &mode) {
    open(fileName, mode);
  }
  // "move" constructor
  htsFile_cpp(htsFile &&file) : m_file(&file){};

  htsFile *data() { return m_file; }

  void open(const std::string &file, const std::string &mode) {
    m_file = hts_open(file.c_str(), mode.c_str());
  }
};

class bcfFile_cpp : public htsFile_cpp {

public:
  bcfFile_cpp() : htsFile_cpp(){};
  bcfFile_cpp(const std::string &fileName, const std::string &mode)
      : htsFile_cpp(fileName, mode) {}

  void open(const std::string &file, const std::string &mode) {
    htsFile_cpp::open(file, mode);
    if (hts_get_format(m_file)->category != htsFormatCategory::variant_data)
      throw std::runtime_error(
          "Input file is not a variant file (bcf, vcf, etc.): " + file);
  }
};

#endif /* _HTS_HPP */
