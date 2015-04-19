# htslibpp

This is a collection of wrapper classes for many dynamically allocated
htslib structs.  Currently, the classes only come with an assortment
of constructors, a destructor and the data() accessor to get at 
the pointer that is being wrapped.

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
    bcf_hdr myHeader("w");
    
    // use the data() method to work on the wrapped header record
    bcf_hdr_append(myHeader.data(), "##fileformat=VCFv4.1");
    
    // copy constructor
    // the bcf duplicate function is called by the copy constructor
    bcf_hdr myHeader2(*(myHeader.data()));
    
    // wrapper constructor using the move operator
    // myHeader2 and myHeader3 refer to the same struct
    // this is not a move in the way c++11 thinks of it.
    // However, using move allows the constructor to know
    // not to duplicate the bcf_hrd_t record.
    bcf_hdr myHeader3(std::move(*(myHeader.data())));

    assert(myHeader3.data() == myHeader2.date());
    assert(myHeader2.data() != myHeader1.data());
    
