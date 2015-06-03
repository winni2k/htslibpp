/* @(#)vcf.hpp
 */

#ifndef _BCF_HPP
#define _BCF_HPP 1

#include <memory>
#include <string>
#include <type_traits>
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

/*
 UNIQUE == TRUE means that bcf1 will clean up after itself
 UNIQUE == FALSE means that bcf1 will simply work on a pointer
 but no destruction of the bcf1_t when the object goes out of scope
*/
template <bool UNIQUE> class bcf1 {
protected:
  bcf1_t *m_bcf1 = nullptr;
  void dealloc() {
    if (UNIQUE == true && m_bcf1)
      bcf_destroy(m_bcf1);
  }

public:
  bcf1() { m_bcf1 = bcf_init(); }

  // duplication may modify the original record
  bcf1(bcf1_t &toDup) { m_bcf1 = bcf_dup(&toDup); }
  bcf1(bcf1_t &&toWrap) { m_bcf1 = &toWrap; }
  ~bcf1() { dealloc(); }

  // move operator
  void acquire_wrap(bcf1_t &toWrap) {
    if (&toWrap == m_bcf1)
      return;
    dealloc();
    m_bcf1 = &toWrap;
    return;
  }
  // copy operator
  void acquire_copy(bcf1_t &toDup) {
    if (&toDup == m_bcf1)
      return;
    dealloc();
    m_bcf1 = bcf_dup(&toDup);
    return;
  }

  bcf1_t *data() { return m_bcf1; }
};

// template <> class bcf1<true>;
// template <> class bcf1<false>;

/*
  The bcf1_extended class extends the bcf1 class with useful accessor functions
  for the bcf1_t struct.

*/
template <bool UNIQUE> class bcf1_extended : public bcf1<UNIQUE> {

public:
  // do not allow anyone to change our internal state!
  bcf1_t *data() = delete;

  // updates this object with data from the next VCF/BCF line
  int bcf_read(bcfFile_cpp &fd, const bcf_hdr_t &h) {
    return ::bcf_read(fd.data(), &h, this->m_bcf1);
  }

  // returns name of this record's chromosome name
  std::string chromName(const bcf_hdr_t &hdr) {
    bcf_unpack(this->m_bcf1, BCF_UN_STR);
    return bcf_hdr_id2name(&hdr, this->m_bcf1->rid);
  }

  // get genomic position in VCF 1-based format
  unsigned pos1() {
    bcf_unpack(this->m_bcf1, BCF_UN_STR);
    return this->m_bcf1->pos + 1;
  }

  // returns a copy of alleles, alleles[0] is ref allele, alleles[1] is first
  // alt and so on
  std::vector<std::string> alleles() {
    bcf_unpack(this->m_bcf1, BCF_UN_STR);
    std::vector<std::string> alls;
    const unsigned numAlls = this->m_bcf1->n_allele;
    alls.reserve(numAlls);
    for (size_t i = 0; i < numAlls; ++i)
      alls.push_back(this->m_bcf1->d.allele[i]);
    return alls;
  }

  bcf_fmt_t *get_fmt(const bcf_hdr_t &hdr, const std::string &tag) {
    return ::bcf_get_fmt(&hdr, this->m_bcf1, tag.c_str());
  }

  // returns unique ptr to ints stored in bcf1
  // unique_ptr frees automatically
  // second element is number of ints stored in the array
  template <typename INT_T>
  std::pair<std::unique_ptr<INT_T, decltype(free) *>, size_t>
  get_format_int(bcf_hdr_t &hdr, const std::string &tag) {

    bcf_fmt_t *format = get_fmt(hdr, tag.c_str());
    int numVals = 0;
    void *dst = nullptr;
    if (!format)
      throw std::runtime_error(tag + " format field does not exist in BCF");

    // check to make sure types match
    switch (format->type) {
    case BCF_BT_INT32:
      if (!std::is_same<INT_T, int32_t>::value)
        throw std::runtime_error(tag + " format field does not contain int32s");
      break;
    case BCF_BT_INT16:
      if (!std::is_same<INT_T, int16_t>::value)
        throw std::runtime_error(tag + " format field does not contain int16s");
      break;
    case BCF_BT_INT8:
      if (!std::is_same<INT_T, int8_t>::value)
        throw std::runtime_error(tag + " format field does not contain int8s");
      break;
    default:
      throw std::runtime_error(tag + " format field does not contain integers");
    }

    int n =
        bcf_get_format_int32(&hdr, this->m_bcf1, tag.c_str(), &dst, &numVals);
    if (n <= 0) {
      throw std::runtime_error("Error parsing tag [" + tag + "]: " +
                               std::to_string(n));
    }

    // wrap returned data in smart pointer
    std::unique_ptr<INT_T, decltype(free) *> ret{static_cast<INT_T *>(dst),
                                                 free};
    return make_pair(move(ret), n);
  }

  std::pair<u_ptr_float, size_t> get_format_float(bcf_hdr_t &hdr,
                                                  const std::string &tag) {
    bcf_fmt_t *format = get_fmt(hdr, tag.c_str());
    if (!format)
      throw std::runtime_error(tag + " format field does not exist in BCF");
    if (format->type != BCF_BT_FLOAT)
      throw std::runtime_error(tag + " format field does not contain floats");
    int numVals = 0;
    void *dst = nullptr;
    int n =
        bcf_get_format_float(&hdr, this->m_bcf1, tag.c_str(), &dst, &numVals);
    if (n <= 0) {
      throw std::runtime_error("Error parsing tag [" + tag + "]: " +
                               std::to_string(n));
    }

    // wrap returned data in smart pointer
    u_ptr_float ret{static_cast<float *>(dst), free};
    return make_pair(move(ret), n);
  }

  //  std::vector<float> bcf_get_fmt_float(const bcf_hdr &hdr,
  //                                     const std::string &tag);
};

// template <> class bcf1_extended<true>;
// template <> class bcf1_extended<false>;

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
