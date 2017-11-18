# chessproblem

chess and chess problem C++11 libraries with a demo program for
finding solutions of chess problems (mate, selfmate, and helpmate) with cooks

(C) Martin Väth (martin at mvath.de).
This project is distributed under the terms of the
GNU General Public License v2.

This project was written within about one day (plus about the same time for
testing/debugging) to train myself for a job interview in programming in C++.

I attempted to conform to the Google C++ style guide (1-2 exceptions are
explicitly justified in the code). Parts of the code (like the data structure
for handling castling and en passant rules in a quick manner) is inspired by a
program which I once wrote for the Commodore C64 in 6510 assembler and
addapted in 1991 for C; in 1992, I had slightly revised the latter, but since
then I had never touched it. The documentation of this original program
is in German, and it is available for download here:
https://www.mathematik.uni-wuerzburg.de/~vaeth/download/index.html

However, the latter was only used for inspiration: The current code is an
independent implementation, using concepts which I did not know explicitly
(only somewhat vague) 30 years ago...

I did not want to spend much time with a sophisticated build system, so I
just quickly adapted a variant of the build system for eix from
https://github.com/vaeth/eix/

The mentioned chess libarary is written with speed in mind.
Note that if you compile normally, there is an enormous slowdown in the chess
library due to many asserts: Disable them to get the full speed, using e.g.
`-DNDEBUG -DNO_DEBUG -DG_DISABLE_ASSERT`

The library uses some techniques like managing lists of nonempty fields
which the inspiring project had not implemented. Also it is simpler to use
(e.g. it generates only valid moves; the old library had generated moves
for which one would need to check separately whether the king is in check
after the move).

The __chess library__ consists of the files:

- `chess.h`:
	the main header which also documents how the chess library is used
- `chess.cc`:
	the implementation of the chess library
- `m_attribute.h`:
	auxiliary macros for compiler specific attributes to
	suppress/emit warnings or speed up the library
- `m_likely.h`:
	auxiliary compiler-dependent macros `LIKELY` and `UNLIKELY` for tests
	which might speed up the library

The library is written in such a way that it can easily be adapted for
different field sizes than 8x8.

Also the __chessproblem library__ is rather general:

- `chessproblem.h`:
	header and documentation for the chessproblem library
- `chessproblem.cc`:
	implementation of the chessproblem library

It is a general recursive multithreaded solver for chess problems.
There is no I/O: the output happens only over a `virtual Output()` function
which has to be overridden if output is desired.

The demo program is kept relatively simple:

- `main.cc`:
	the main function for initializing and calling the library

The usage of the demo `chessproblem` is rather elementary and unix style:
Just the data can be entered over options and arguments (or stdin).
Then the output is printed, and the program exits.
Since __chessproblem-2.5__, the demo program depends for its output on the
library
https://github.com/vaeth/osformat

See `chessproblem -h` for further help.

## Known Bugs

When threads are killed by the kernel due to lack of resources, segfaults
can happen: You might need to increase system limits correspondingly
if you observe in the system logs that threads are killed by the kernel.
Originally this bug was conjectured to be a race, because it was never
observed with `-j4` or less.
To mitigate this problem, `chessproblem.cc` automatically decreases the maximal
number of running tasks to the result of `std::thread::hardware_concurrency()`.
More threads than this number cannot improve performance, anyway.
