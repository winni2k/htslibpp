htslibpp
=======
C++ wrapper for the [htslib](https://github.com/samtools/htslib) C library

This code is still very much in alpha.  Help greatly appreciated.

The goal of this library is to provide wrappers for dynamically
allocated htslib structs, and an object oriented interface for
interacting with htslib.

## Wrapping htslib structs

htslibpp contains a collection of wrapper classes for many dynamically
allocated [htslib](https://github.com/samtools/htslib) structs.
Currently, the classes only come with an assortment of constructors, a
destructor and the data() accessor to get at the pointer that is being
wrapped. 

The three constructors (new, copy, wrapper) allow the creation of a wrapper
object that:
- new - properly initializes a new record
- copy - duplicates an existing record
- move - wraps an already existing record (similar to unique_ptr)

The destructor makes sure the record destructor is called when the wrapper
object goes out of scope, thereby eliminating memory leaks (at least
at this level of the implementation).

data() can be used to get a hold of the wrapped pointer in order to call a
function on it.  For example:

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
    






