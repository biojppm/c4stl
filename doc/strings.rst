Strings
=======

.. doxygengroup:: string_classes

Non-owning strings
------------------

Types
^^^^^

.. doxygentypedef:: substring
.. doxygentypedef:: substringrs
.. doxygentypedef:: csubstring
.. doxygentypedef:: csubstringrs
.. doxygentypedef:: wsubstring                    
.. doxygentypedef:: wsubstringrs
.. doxygentypedef:: cwsubstring                    
.. doxygentypedef:: cwsubstringrs

Classes
^^^^^^^

.. doxygenclass:: basic_substring

.. doxygenclass:: basic_substringrs

Owning strings
--------------

Types
^^^^^

.. doxygentypedef:: string
.. doxygentypedef:: wstring                    

.. doxygentypedef:: text
.. doxygentypedef:: wtext                    

Classes
^^^^^^^

.. doxygenclass:: basic_string
.. doxygenclass:: basic_text


The string CRTP base
--------------------

This is a reference to :cpp:func:`_str_crtp::find_first_of()`.
This is a reference to :cpp:func:`_str_crtp::find_first_not_of()`.
This is a reference to :cpp:func:`_str_crtp::find_last_of()`.
This is a reference to :cpp:func:`_str_crtp::find_last_not_of()`.

.. doxygenclass:: _str_crtp

