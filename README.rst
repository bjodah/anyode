anyode
======

.. image:: http://hera.physchem.kth.se:9090/api/badges/bjodah/anyode/status.svg
   :target: http://hera.physchem.kth.se:9090/bjodah/anyode
   :alt: Build status

anyode provides C++ headers used by:

- `pyodesys <https://github.com/bjodah/pyodesys>`_
- `pycvodes <https://github.com/bjodah/pycvodes>`_
- `pyodeint <https://github.com/bjodah/pyodeint>`_
- `pygslodeiv2 <https://github.com/bjodah/pygslodeiv2>`_

Notes
-----
When compiled with ``-DNDEBUG`` then ``AnyODE::buffer_factory`` will return a ``std::unique_ptr`` to an (unitizialized)
array. When compiled without ``NDEBUG`` it will instead return a ``std::vector`` initizlied with signaling NaN.

By default, AnyODE uses BLAS/LAPACK for preconditioning operations on the Jacobian matrix. If you do not have
(or do not wish to compile with) BLAS/LAPACK, ``-DANYODE_NO_LAPACK=1`` should be set, in which case built-in
linear algebra routines will be substituted.

License
-------
The source code is Open Source and is released under the simplified 2-clause BSD license. See `LICENSE <LICENSE>`_ for further details.
Contributors are welcome to suggest improvements at https://github.com/bjodah/pyodesys

Author
------
Björn I. Dahlgren, contact:

- gmail address: bjodah
- kth.se address: bda
