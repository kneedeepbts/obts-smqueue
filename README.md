# obts-smqueue
Refactoring the RangeNetworks OpenBTS project.

Had to install the `libfmt-dev` package to get the library to link correctly.
While this wasn't necessary in CLion running on Ubuntu, it seemed to be needed
on Debian 11.5.

Installing `libenet-dev` for the UDP networking library.  Make move back to
something more bespoke later, but using a networking library will greatly
simplify the code right now.
