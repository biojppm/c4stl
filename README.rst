
========================  ======================
 Linux + OS X: |travis|    Windows: |appveyor|
========================  ======================

c4stl
=====

C++ library of performance-minded contiguous containers, strings and streams.


Features
--------

  * ranges
    * non-owning writeable ranges: `c4::span`, `c4::spanrs`, `c4::etched_span`
    * non-owning read-only ranges: `c4::cspan`, `c4::cspanrs`,
      `c4::cetched_span`
  * container building blocks:
    * raw storage: `c4::raw_fixed<T,N>`, `c4::raw<T>`, `c4::raw_small<T,N>`
      (via the small allocator trick) and importantly `c4::raw_paged<T>` (which
      allows for constant time insertion on vector-based lists and maps
      without the need for a prior call to `reserve()`).
    * storage growth models: powers of two, Fibonacci, composed, etc.
    * object (mass-) construction/destruction/copy/move facilities
  * **WIP**:
    * vector models:
      * `c4::fixed_vector<T,N>`: compile-time fixed capacity, variable size
      * `c4::small_vector<T,N>`: with inplace storage for up to N elements,
        switching to the heap when the size exceeds N.
      * `c4::array<T,N>`
      * `c4::vector<T>`
      * storage growth policy is given as a template parameter for the
        dynamic memory vectors.
    * sorted vector: `c4::sorted_vector<T,VectorImpl>`
    * index-based contiguous memory lists (forward- and doubly-linked):
      * `c4::flat_list<T,Storage>`: based on a vector of `{ T value, I next }`
        pairs
      * `c4::split_list<T,Storage>`: based on a vector for the indices and a
        different vector for the values
      * the `raw_paged` storage policy can be used, thus getting constant-time
        insertion/lookup even without any prior calls to `reserve()`, with
        all the mechanical sympathy for caches that arrays are known for
    * contiguous maps with customizeable storage. As with lists, the vector
      storage is given as a template parameter.
      * `c4::flat_map<K,V>`: based on a sorted vector with `std::pair<K,V>`
        as the value type; useful for frequent iterations over both keys and
        values)
      * `c4::split_map<K,V>`: based on a sorted vector for the keys and a
        separate vector for the values; useful for when lookups are more
        important than iteration
      * need to investigate to what extent using a full array-based rb-tree
        can be accomplished (separating the indices to save object copies?)
  * strings
    * non-owning writeable strings: `c4::substring`, `c4::substringrs` with `wchar_t` counterparts
    * non-owning read-only strings: `c4::csubstring`, `c4::csubstringrs` with `wchar_t` counterparts
    * owning strings: `c4::string` (with small string optimization), `c4::text`
    (without SSO) with `wchar_t` counterparts
    * no virtuals anywhere
    * where the semantics make sense, all string methods are common to every type
    * methods for path-like pop/push from right/left, with custom separator and
      the / operator which joins operands as directories.
    * expression templates are used for string concatenation operations, and
      can be switched off (reverting to less-efficient allocation-happy
      behaviour).
    * all string selection operations keep allocations to a minimum by returning
      substrings
    * clear and transparent ownership semantics:
      * assigning a string to a substring `subs=s;` means "point subs to
        the buffer of s"
      * assigning a substring to a string `s=subs;` means "copy the content
        of subs to the buffer of s"
      * assigning a string/substring/char sum to a substring or string
        means "copy the result of this operation to the string's internal
        buffer", wherever it is.
  * string stream: `c4::sstream< StringType >`
    * essentially a decorator for writing into / reading from a string,
      without having to copy to get the result (a major sink of efficiency in
      the design of `std::stringstream`)
    * the string can be moved in and out (WIP)
    * works with `std::string` / `std::wstring` and all the c4 strings
    * no virtuals anywhere.
    * many methods for writing/reading:
      * iostream-like chevron `<<` `>>` operators
      * type safe concatenation: `ss.cat(var)` and `ss.uncat(var)`
        serializes/deserializes the object into the string (via `<<` `>>`
        overloads)
      * Python-like, type safe: eg, `ss.printp("hi I am {}", name)`, `ss.scanp()`
      * C-like, type unsafe: `ss.printf()`, `ss.vprintf()` (sorry, no scanf
        due to it being difficult to find the number of characters read)
  * size types are given as template parameters for all containers. This is
    meant more for situations in which it is important to have an overall
    narrow type as the default for the containers (as in embedded platforms),
    than to have dozens of different container types parameterized by the
    size type. But it also helps to be able to go narrow for just that
    particular hotspot! Although extensive unit tests are yet to be written
    for size type interoperation, things should mostly work here (assertions
    for overflow are generously spliced throughout the code where this might
    occur). Of course, there might be some places where this was overlooked
    -- so your contributions or bug reports are welcome.
  * C++17-like polymorphic memory resource semantics. Allocations are slow
    anyway, so this is a place where virtual behaviour has advantages. If
    this is too slow for you, you can still plug in your ultra-lean
    ultra-fast no-virtuals-anywhere allocator.
  * customizeable behaviour on error, including callbacks
  * Tested in Windows and Linux.
  * Compilers: MSVC 2015+, g++4.9+, clang 3.8+, icc 2016+.
  * Tested with valgrind and the clang sanitizers.


Caveats
-------

This is an alpha. Although there are already hundreds of unit tests, and they are
executed with the clang sanitizers, and valgrind, bugs are bound to
happen.

Also, design flaws may be present (it may very well be possible to
successfully compile method calls which should not be possible). I welcome
your input on this too.


Documentation
-------------

For now, use Doxygen::

  $ cd doc
  $ doxygen Doxyfile


License
-------

This project is licensed under the MIT license.


Status
------

This project is a pre-alpha under development.


Building
--------

Build using cmake::

    $ git clone https://github.com/biojppm/c4stl
    $ cd c4stl
    $ mkdir build
    $ cd build
    $ cmake ..
    $ cmake --build .


Contribute
----------

Your contributions are welcome! Send pull requests to `<https://github.com/biojppm/c4stl/pulls>`.


Support
-------

Your bug reports are also welcome! Send them to `<https://github.com/biojppm/c4stl/issues>`.


.. |travis| image:: https://travis-ci.org/biojppm/c4stl.svg?branch=master
    :target: https://travis-ci.org/biojppm/c4stl
.. |appveyor| image:: https://ci.appveyor.com/api/projects/status/github/biojppm/c4stl?branch=master&svg=true
    :target: https://ci.appveyor.com/project/biojppm/c4stl
