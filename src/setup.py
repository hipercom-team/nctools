#---------------------------------------------------------------------------
# Setup file for compiling SWIG module for libnc
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

import distutils

from distutils.core import setup, Extension


unusedExtension = Extension("wraplibnc",
                            ["wraplibnc.i"],
                            include_dirs=["."],
                            library_dirs=["."],
                            libraries=[],
                            swig_opts=[],
                            language="c"
                      )


extension = Extension("libncmodule", ["libncmodule.c"])

#print dir(extension)
#extension.undef_macros.append("NDEBUG")

setup(name="libncmodule", version="0.1",
      ext_modules = [ extension ])

#---------------------------------------------------------------------------
