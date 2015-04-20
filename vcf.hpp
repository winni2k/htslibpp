/* @(#)bcf.hpp
 */

#ifndef _BCF_HPP
#define _BCF_HPP 1

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <htslib/vcf.h>
#include "hts.hpp"

typedef std::unique_ptr<int32_t, decltype(free) *> u_ptr_int32;
typedef std::unique_ptr<float, decltype(free) *> u_ptr_float;

class bcf_hdr {

protected:
  bcf_hdr_t *m_bcf_hdr = nullptr;

public:
  bcf_hdr(){};
  bcf_hdr(const std::string &mode) { m_bcf_hdr = bcf_hdr_init(mode.c_str()); }
  bcf_hdr(const bcf_hdr_t &toDup) { m_bcf_hdr = bcf_hdr_dup(&toDup); }
  bcf_hdr(bcf_hdr_t &&toWrap) { m_bcf_hdr = &toWrap; }
  ~bcf_hdr() {
    if (m_bcf_hdr)
      bcf_hdr_destroy(m_bcf_hdr);
  }

  bcf_hdr_t *data() { return m_bcf_hdr; }
  const bcf_hdr_t *cdata() const { return m_bcf_hdr; }
};

class bcf1 {
protected:
  bcf1_t *m_bcf1 = nullptr;

public:
  bcf1() { m_bcf1 = bcf_init(); }

  // duplication may modify the original record
  bcf1(bcf1_t &toDup) { m_bcf1 = bcf_dup(&toDup); }
  bcf1(bcf1_t &&toWrap) { m_bcf1 = &toWrap; }
  ~bcf1() {
    if (m_bcf1)
      bcf_destroy(m_bcf1);
  }

  bcf1_t *data() { return m_bcf1; }
};

/*
  The bcf1_extended class extends the bcf1 class with useful accessor functions
  for the bcf1_t struct.

*/

class bcf1_extended : public bcf1 {

public:
  // do not allow anyone to change our internal state!
  bcf1_t *data() = delete;

  // updates this object with data from the next VCF/BCF line
  int bcf_read(bcfFile_cpp &fd, const bcf_hdr &h);

  // returns name of this record's chromosome name
  std::string chromName(const bcf_hdr &hdr);

  // get genomic position in VCF 1-based format
  unsigned pos1();

  // returns a copy of alleles, alleles[0] is ref allele, alleles[1] is first
  // alt and so on
  std::vector<std::string> alleles();

  bcf_fmt_t *get_fmt(const bcf_hdr &hdr, const std::string &tag);

  // returns unique ptr to int32s stored in bcf1
  // unique_ptr frees automatically
  // second element is number of int32s stored in the array
  std::pair<u_ptr_int32, size_t> get_format_int32(bcf_hdr &hdr,
                                                  const std::string &tag);

  std::pair<u_ptr_float, size_t> get_format_float(bcf_hdr &hdr,
                                                  const std::string &tag);

  //  std::vector<float> bcf_get_fmt_float(const bcf_hdr &hdr,
  //                                     const std::string &tag);
};

class bcf_hrec {
private:
  bcf_hrec_t *m_bcf_hrec = nullptr;

public:
  bcf_hrec(){};

  // duplication may modify the original record
  bcf_hrec(bcf_hrec_t &toDup) { m_bcf_hrec = bcf_hrec_dup(&toDup); }
  bcf_hrec(bcf_hrec_t &&toWrap) { m_bcf_hrec = &toWrap; }
  ~bcf_hrec() {
    if (m_bcf_hrec)
      bcf_hrec_destroy(m_bcf_hrec);
  }

  bcf_hrec_t *data() { return m_bcf_hrec; }
};

// helper functions
namespace htspp {
std::vector<std::string> get_sample_names(const bcf_hdr_t &hdr);
};

#endif /* _BCF_HPP */
