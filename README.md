
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
