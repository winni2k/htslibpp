#include "vcf.hpp"

using namespace std;

namespace htspp {
// fils a vector of strings with the sample names in this bcf_hdr
vector<string> get_sample_names(const bcf_hdr_t &hdr) {

  vector<string> names;
  names.reserve(bcf_hdr_nsamples(&hdr));
  for (int sampNum = 0; sampNum < bcf_hdr_nsamples(&hdr); ++sampNum)
    names.push_back(hdr.samples[sampNum]);
  return names;
}
}

int bcf1_extended::bcf_read(bcfFile_cpp &fd, const bcf_hdr &h) {
  return ::bcf_read(fd.data(), h.cdata(), m_bcf1);
  // need to add error once I know how to tell when the end of the file has been
  // reached
  //    throw std::htslib_error(
  //        "Encountered critical error in htslib while trying to read record");
}

bcf_fmt_t *bcf1_extended::get_fmt(const bcf_hdr &hdr, const string &tag) {
  return ::bcf_get_fmt(hdr.cdata(), m_bcf1, tag.c_str());
}

string bcf1_extended::chromName(const bcf_hdr &hdr) {
  bcf_unpack(m_bcf1, BCF_UN_STR);
  return bcf_hdr_id2name(hdr.cdata(), m_bcf1->rid);
}

// get genomic position in VCF 1-based format
unsigned bcf1_extended::pos1() {
  bcf_unpack(m_bcf1, BCF_UN_STR);
  return m_bcf1->pos + 1;
}

// returns a copy of alleles, alleles[0] is ref allele, alleles[1] is first
// alt and so on
std::vector<std::string> bcf1_extended::alleles() {
  bcf_unpack(m_bcf1, BCF_UN_STR);
  std::vector<std::string> alls;
  const unsigned numAlls = m_bcf1->n_allele;
  alls.reserve(numAlls);
  for (size_t i = 0; i < numAlls; ++i)
    alls.push_back(m_bcf1->d.allele[i]);
  return alls;
}

// returns a unique pointer to array of int32_t's plus the size of the array
// => poor man's vector, but avoids copying all the data into a vector...
pair<u_ptr_int32, size_t>
bcf1_extended::get_format_int32(bcf_hdr &hdr, const std::string &tag) {

  if (get_fmt(hdr, tag.c_str())->type != BCF_BT_INT32)
    throw std::runtime_error(tag + " format field does not contain int32s");
  int numVals = 0;
  void *dst = nullptr;
  int n = bcf_get_format_int32(hdr.data(), m_bcf1, tag.c_str(), &dst, &numVals);
  if (n <= 0) {
    throw std::runtime_error("Error parsing tag [" + tag + "]: " +
                             to_string(n));
  }

  // wrap returned data in smart pointer
  u_ptr_int32 ret{static_cast<int32_t *>(dst), free};
  return make_pair(move(ret), n);
}

// returns a unique pointer to array of floats plus the size of the array
// => poor man's vector, but avoids copying all the data into a vector...
pair<u_ptr_float, size_t>
bcf1_extended::get_format_float(bcf_hdr &hdr, const std::string &tag) {

  if (get_fmt(hdr, tag.c_str())->type != BCF_BT_FLOAT)
    throw std::runtime_error(tag + " format field does not contain floats");
  int numVals = 0;
  void *dst = nullptr;
  int n = bcf_get_format_float(hdr.data(), m_bcf1, tag.c_str(), &dst, &numVals);
  if (n <= 0) {
    throw std::runtime_error("Error parsing tag [" + tag + "]: " +
                             to_string(n));
  }

  // wrap returned data in smart pointer
  u_ptr_float ret{static_cast<float *>(dst), free};
  return make_pair(move(ret), n);
}

/*
vector<float> bcf1_extended::bcf_get_format_float(const bcf_hdr &hdr,
                                                  const std::string &tag) {
  auto fmt_ptr = bcf_get_fmt(hdr, tag.c_str());
  if (fmt_ptr->type != BCF_BT_FLOAT)
    throw std::runtime_error(tag + " format field does not contain floats");
  vector<float> retFloats;
  const unsigned len = fmt_ptr->p_len;
  retFloats.reserve(len);
  for (int valNum = 0; 0 < len; ++valNum)
    retFloats.push_back(*(fmt_ptr->p + valNum));
  return retFloats;
};
*/
