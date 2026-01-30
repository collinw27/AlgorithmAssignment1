# Algorithm Assignment 1

Jordan Price (UFID 77967971) \
Collin Williams (UFID 73358805)

## Compiling

There is only one source file, `main.cpp`, which can be compiled
with your C++ compiler of choice. Its only dependencies are
within the standard library. I used C++17 to compile it, but earlier
versions might work as well.

## Executing

After compiling the program, you can run the executable with either the
`match` or `verify` command line arguments.

**Match mode:** \
`AlgorithmAssignment1.exe match [input_file] [output_file]` \
ex. `AlgorithmAssignment1.exe match .\example.in .\example.out`

**Verify mode:** \
`AlgorithmAssignment1.exe verify [input_file] [output_file]` \
ex. `AlgorithmAssignment1.exe verify .\example.in .\example.out`

Running without arguments defaults to `match * *` \
File arguments can be replaced with `*` to use terminal for input/output. \
`TIMED` can be added as a final argument to time the code in ns.

## Assumptions

The input & output format matches the format provided in the assignment instructions.

## Task 3: Scalability

The following graphs were constructed by running the match/verify engines 3 times and averaging the execution time in nanoseconds.

![](/scalability/match.png) \
![](/scalability/verify.png)

I notice that both the match and verify times grow as a function of n^2, where n is the input size. This is consistent with the fact that the worst-case time complexity
of Gale-Shapley is O(n^2).
