## DS19Projekt

#### General
Made this for fun and procrastination. It's not perfect and I'm pretty sure it has room for improvement.

Basic goal is to read a dictionary from a file and translate some text piped through stdin. Only C standard library functions should be used (althoug it was allowed to use getline() or strdup()). Some examples are provided [here](https://github.com/tylernewnoise/DS19Projekt/data/testcases.tar.gz), the fail test are provided by [belorenz](https://github.com/belorenz) as well as some code. More information can be found in [here](https://github.com/tylernewnoise/DS19Projekt/data/programmieraufgabe.pdf), but only in german.

#### Build
```
$ gcc -o loesung -O3 -std=c11 -Wall -Werror -DNDEBUG loesung.c
```

#### Run
```
$ cat example.stdin | ./loesung example.wb
```

#### Resources
- https://github.com/jamesroutley/write-a-hash-table
- https://stackoverflow.com/questions/7666509/hash-function-for-string?rq=1
- https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
- http://www.cse.yorku.ca/~oz/hash.html
- https://groups.google.com/forum/#!msg/comp.lang.c/lSKWXiuNOAk/zstZ3SRhCjgJ
- https://stackoverflow.com/questions/50399090/use-of-a-signed-integer-operand-with-a-binary-bitwise-operator-when-using-un
##### Great stuff on bit twiddling
- http://graphics.stanford.edu/~seander/bithacks.html
- https://www.geeksforgeeks.org/case-conversion-lower-upper-vice-versa-string-using-bitwise-operators-cc/
- https://www.hackersdelight.org/
