
# COMMON

<img alt="The Original Common" title="The Original Common" src="https://i.imgur.com/QOy4c1H.jpg" width="127" height="180" style="float: right;" />

COMMON is a command-line utility for finding the most common string between two
files. COMMON builds a hash table as it parses the input files, dynamically
adding new strings to the table or incrementing reference counts for previously
seen strings as they are encountered.

The most common string is determined by keeping separate counts for each input
file per string, and taking the geometric mean of the counts. The hash table
entry with the highest geometric mean between its two counts is determined to
be the most common string between the two input files.

## Usage

```
$ common --help
Usage: common [OPTIONS...] FILE1 FILE2
Find the most common string shared between two files.

    -j, --threads                Use this many threads (default: 2)
    -h, --help                   Display this help menu and exit
        --version                Display program version info and exit
    -v, --verbose                Display detailed info during program execution

```
