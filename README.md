Matrix Vector Multiplication in Parallel,
Dustin Franklin,
df59@uakron.edu

Given a rigidly formatted text file as input, this program will multiply an
m x n matrix by a n x 1 vector using strictly two processes. Processing of
the input file is done serially by both processes independently.
Multiplication and addition of even rows of the m x n matrix are done by
process 0 and added to the final output. The odd rows are processed by 
process 1 and sent back to process 0 to create the final vector output.

To compile, MPI must be installed on your machine.
Run mpic++ main.cpp for a 2 process parallel executable.
Run mpic++ serail.cpp for a single process executable.

To run, simply execute the executable in the terminal.

An input file must be in the working directory of the executable named mv_data.txt
and its format must follow that of the provided example where the first line is the
matrix m number of rows and the second line is the matrix n number of columns, and
where the lines following the matrix values are the values of the vector.
