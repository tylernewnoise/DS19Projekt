## DS19Projekt

#### General
Made this for fun and procrastination. It's not perfect and I'm pretty sure it has _some_ room for improvement.

Basic goal is to read a dictionary from a file and translate some text piped through stdin. Some examples are provided in [examples](https://github.com/tylernewnoise/DS19Projekt/examples), the fail test are provided by [belorenz](https://github.com/belorenz) as well as some code. More information can be found in [here](https://github.com/tylernewnoise/DS19Projekt/programmieraufgabe.pdf), but only in german.

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
- https://stackoverflow.com/questions/50399090/use-of-a-signed-integer-operand-with-a-binary-bitwise-operator-when-using-un
- http://www.cse.yorku.ca/~oz/hash.html
- https://groups.google.com/forum/#!msg/comp.lang.c/lSKWXiuNOAk/zstZ3SRhCjgJ
- https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
- https://stackoverflow.com/questions/7666509/hash-function-for-string?rq=1
- https://www.hackersdelight.org/
