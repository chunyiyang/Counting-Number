# Counting-Number
Counting number of integers from data stream
The program is implemented in C++ and runs two threads: one
thread takes a window size N and an input stream of 16-bit unsigned
integers from host:port pair from stdin first, and then another thread
accepts queries from stdin and anwers to stdout. The queries are
something like “how many <int> for last <k> data?”. The output show
the following:
a) input integers immediately,
b) the queries, and
c) the counts is “exact” or “estimated” as the answers of the queries.
