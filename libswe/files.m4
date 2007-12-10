dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4. You should
dnl use the provided autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`boundary_types',      `hh',)
add(`implicit_solver',     `hh', `test')
add(`limiter',             `hh', `test')
add(`scenario',            `hh',)
add(`solver',              `hh', `test', `benchmark')
