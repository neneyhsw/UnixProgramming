# UnixProgramming
Advanced Unix Programming course in NYCU (NCTU).

## hw1
Implementing a "lsof" like program.

### Program Arguments
Your program should work without any arguments. In the meantime, your program has to handle the following arguments properly:
* -c REGEX: a regular expression (REGEX) filter for filtering command line. For example -c sh would match bash, zsh, and share.
* -t TYPE: a TYPE filter. Valid TYPE includes REG, CHR, DIR, FIFO, SOCK, and unknown. TYPEs other than the listed should be considered invalid. For invalid types, your program has to print out an error message Invalid TYPE option. in a single line and terminate your program.
* -f REGEX: a regular expression (REGEX) filter for filtering filenames.

A sample output from this homework is demonstrated as follows:
