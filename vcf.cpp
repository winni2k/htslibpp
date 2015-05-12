#include "htslibpp/vcf.hpp"

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
