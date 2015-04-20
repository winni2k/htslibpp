#include <cassert>
#include <string>
#include <htslib/vcf.h>
#include "htslibpp/vcf.hpp"

using namespace std;

int main() {

  // new constructor
  bcf_hdr myHeader1("w");

  // use the data() method to add a line to the wrapped header
  // record using an htslib function
  bcf_hdr_set_version(myHeader1.data(), "VCFv4.1");

  // copy constructor
  // the bcf duplicate function is called by the copy constructor
  bcf_hdr myHeader2(*(myHeader1.data()));

  assert(myHeader2.data() != myHeader1.data());

  // wrapper constructor using the move operator
  // hdr and myHeader3 refer to the same struct
  // this is not a move in the way c++11 thinks of it.
  // However, using move allows the constructor to know
  // not to duplicate the bcf_hdr_t record.
  bcf_hdr_t *hdr = bcf_hdr_dup(myHeader1.data());
  bcf_hdr myHeader3(std::move(*(hdr)));

  assert(myHeader3.data() == hdr);
}
