===========  ========================  ======================
 |license|    Linux + OS X: |travis|    Windows: |appveyor|
===========  ========================  ======================

c4stl
=====

C++ library of performance-minded contiguous containers, strings and streams.


Features
--------

* ranges

  * non-owning writeable ranges: ``c4::span<T>``, ``c4::spanrs<T>``, ``c4::etched_span<T>``

  * non-owning read-only ranges: ``c4::cspan<T>``, ``c4::cspanrs<T>``,
    ``c4::cetched_span<T>``

* Containers **WIP**: hold raw storage and are responsible for object lifetime.

  * vector models:

    * ``c4::array<T,N>``: fixed capacity and size

    * ``c4::vector<T>``:  dynamic capacity and size

    * ``c4::array_vector<T,N>``: fixed capacity, dynamic size

    * ``c4::small_vector<T,N>``: with inplace storage for up to N elements,
      switching to the heap when the capacity exceeds N.

    * ``c4::spanning_vector<T>``: non-owning, allows insertion and removal of
      elements without requiring size parameterization (thus providing
      size-erasure for client code)

    * customizeable behaviour:

      * size type (defaulting to `size_t`) is given as a template parameter

      * data alignment (defaulting to `alignof(T)`) is given as a template
        parameter

      * the capacity policy for dynamic types is given as a template
        parameter. Default policy is growth-by-two for small sizes and
        growth-by-golden-section for large sizes

  * sorted vector: ``c4::sorted_vector<T,Compare,Storage>``. Leveraging the
    fact that insertions and lookups are often done in separate phases, it
    provides an explicit API and a ``valid()`` order state predicate to
    efficiently manage this flow:

    * defaults to the safe (but inefficient) always-sorted behaviour

    * insert without sorting by calling ``insert_nosort()``. Inserted
      elements will be pushed to the back of the container; if the order
      was valid before inserting, a comparison to the back() element is made
      and the valid/invalid boundary adjusted when they do not `Compare`.

    * lookups in this invalid state will then consist of a binary search in
      the valid portion, and a linear search in the invalid portion. Of
      course, when the state is valid then the binary search will apply
      everywhere.

    * ``lower_bounds()``, ``upper_bound()`` and ``equal_range()`` will
      trigger an assertion if the container is not fully valid (ie if the
      valid boundary is not at the end).

    * explicitly calling ``fix()`` will make the container fully valid.

    * can also call ``insert()``, which will do insert and immediately
      sort. This is practical when used occasionally (inefficiently).

    * also provides an explicit API for building the container without sorting or
      checking the order of the given input: ``assign_nosort()`` (does not
      sort, but does a check and marks the invalid portion) and
      ``assign_nocheck()`` (assumes the input is fully sorted and will mark
      the container as fully valid). Again, the vanilla ``assign()`` method
      will safely (but inefficiently) behave as expected: its input is
      checked and sorted if needed, so that the container is valid on return.

  * contiguous maps with customizeable storage. As with lists, the vector
    storage is given as a template parameter.

    * ``c4::flat_map<K,V,Compare,Storage>``: based on a sorted vector with
      ``std::pair<K,V>`` as the value type; useful for frequent iterations
      over both keys and values)

    * ``c4::split_map<K,V,Compare,Storage>``: based on a sorted vector for
      the keys and a separate vector for the values; useful for when lookups
      are more important than iteration

    * need to investigate to what extent using an index map will solve
      the problem of data movement.

  * index-based contiguous memory lists (forward- and doubly-linked):

    * ``c4::flat_list<T,Storage>``: based on a vector of ``{ T value, I next }``
      pairs

    * ``c4::split_list<T,Storage>``: based on a vector for the indices and a
      different vector for the values

    * using the ``raw_paged`` storage policy will allow constant-time
      insertion/lookup with zero-copies-resizing, even without any prior
      calls to ``reserve()``, with all the mechanical sympathy for caches
      that arrays are known for

* container building blocks (facilitate building more containers):

  * raw storage: uninitialized memory returning elements via the ``[]`` operator.

    * ``c4::stg::raw_fixed<T,N>``: capacity fixed at compile time

    * ``c4::stg::raw<T>``: capacity set at run time

    * ``c4::stg::raw_small<T,N>``: capacity set at run time with in-place
      storage up to N elements; will only allocate if the required capacity
      is larger than N.

    * ``c4::stg::raw_paged<T>`` (quasi-contiguous). With the page size being a
      power of two the ``[]`` operator is constant time. This allows for
      constant time zero-copies-resizing insertion on vector-based lists and
      maps without the need for a prior call to ``reserve()``). Unlike array
      storage, it also saves the need to copy over the lower pages whenever
      the capacity is changed. The page size is also a 

  * storage growth models: powers of two, Fibonacci, composed, etc.

  * object (mass-) construction/destruction/copy/move facilities

* strings

  * non-owning writeable strings: ``c4::substring``, ``c4::substringrs`` with
    ``wchar_t`` counterparts

  * non-owning read-only strings: ``c4::csubstring``, ``c4::csubstringrs``
    with ``wchar_t`` counterparts

  * owning strings: ``c4::string`` (with small string optimization),
    ``c4::text`` (without SSO) with ``wchar_t`` counterparts

  * no virtuals anywhere.

  * where the semantics make sense, all string methods are common to every type

  * also, there's methods for path-like pop/push from right/left, with custom
    separator and the / operator which joins operands as directories.

  * expression templates are used for string concatenation operations
    (allowing for efficient lazy evaluation of the length before and proper
    ``reserve()`` before any actual data copy begins). Expression templates can be
    switched off (reverting to less-efficient allocation-happy behaviour).

  * all string selection operations keep allocations to a minimum by returning
    substrings

  * clear and transparent ownership semantics:

    * assigning an owning string to a non-owning substring ``subs=s;`` means
      "point subs to the buffer of s"

    * assigning a non-owning substring to an owning string ``s=subs;`` means
      "copy the content of subs to the buffer of s"

    * assigning a sum of strings (of any type, including raw C strings) to a
      substring or string means "copy the result of this operation to the
      string's internal buffer". Owning strings are free to enlarge their
      buffer as needed, but nonowning strings will assert if the space needed
      to store the result is bigger than the size of the buffer they're
      pointing to.

* string stream: ``c4::sstream< StringType >``

  * the string can be moved in and out! (WIP)

  * works with ``std::string`` / ``std::wstring`` and all the c4 strings

  * no virtuals anywhere.

  * many methods for writing/reading:

    * iostream-like chevron ``<<`` ``>>`` operators

    * type safe concatenation: ``ss.cat(var1, var2)`` and ``ss.uncat(var1, var2)``
      serializes/deserializes the object into the string (via ``<<`` ``>>``
      overloads)

    * Python-like, type safe: eg, ``ss.printp("hi I am {}", name)``, ``ss.scanp()``

    * C-like, type unsafe: ``ss.printf()``, ``ss.vprintf()`` (sorry, no scanf
      due to it being difficult to find the number of characters read)

  * essentially a decorator for writing into / reading from a string,
    without having to copy to get the result (a major sink of efficiency in
    the design of ``std::stringstream``)

* size types are given as template parameters for all containers.

  * This is meant more for situations in which it is important to have an
    overall narrow type as the default for the container sizes (as in
    embedded platforms), than to have dozens of different container types
    parameterized by the size type.

  * But you can also do it! It also helps to be able to go narrow for just
    that particular hotspot! For example, using a 16-bit integer for the list
    index type will make its node type 96 bits wide ``(64 + 2 * 16)`` instead
    of the 192 bits that a std three-pointer node would take in a 64-bit
    platform. This is a ``50%`` saving in memory, and can really matter in
    some cases.

* alignment (defaulting to ``alignof(T)``) is also a template parameter for
  all containers to facilitate SIMD operations on containers (strings are an
  exception, but this is easy to bypass if the string buffer is kept on an
  aligned container and a substring is used to access it).

* C++17-like polymorphic memory resource semantics. Allocations are slow
  anyway, so this is a place where virtual behaviour has advantages. If
  this is too slow for you, you can still plug in your ultra-lean
  ultra-fast no-virtuals-anywhere allocator into the containers.

* customizeable behaviour on error, including callbacks and macros for
  turning on/off assertions irrespective of ``NDEBUG`` status

* Basic unit tests are already in place. Although extensive unit tests for
  size type interoperation are yet to be implemented, things should mostly
  work here (assertions for overflow are generously spliced throughout the
  code where this might occur). Of course, there still may be some places
  where this was overlooked -- so your contributions or bug reports are
  welcome.

* Tested in Windows and Linux.

* Compilers: ``MSVC>=2015``, ``g++>=5``, ``clang++>=3.8``,
  ``icc>=2016``. c4stl uses ``std::is_trivially_move_constructible`` and
  ``std::is_trivially_move_assignable``, which is not available in older
  libstdc.

* Tested with valgrind and the clang sanitizers.


Caveats
-------

This is a pre-alpha. Although there are already hundreds of unit tests, and they are
executed with the clang sanitizers, and valgrind, bugs are bound to
happen.

Also, design flaws will be present in some corner cases, and it may very well
be possible to successfully compile method calls which should not be
possible to do. Again, I welcome your input regarding this and any other methods.


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

Running the tests::

    $ cmake --build --target unit_tests   # builds and runs the tests
    $ cmake --build --target test         # only runs the tests


Contribute
----------

Your contributions are welcome! Send pull requests to `<https://github.com/biojppm/c4stl/pulls>`.


Support
-------

Your bug reports are also welcome! Send them to `<https://github.com/biojppm/c4stl/issues>`.

.. |license| image:: https://img.shields.io/badge/License-MIT-green.svg
   :alt: License: MIT
   :target: https://github.com/biojppm/c4stl/blob/master/LICENSE.txt
.. |travis| image:: https://travis-ci.org/biojppm/c4stl.svg?branch=master
    :target: https://travis-ci.org/biojppm/c4stl
.. |appveyor| image:: https://ci.appveyor.com/api/projects/status/github/biojppm/c4stl?branch=master&svg=true
    :target: https://ci.appveyor.com/project/biojppm/c4stl
