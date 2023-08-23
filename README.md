# bfcomp
Simple brainfuck compiler.

## Requirements
- [CMake](https://cmake.org)
- [Make](https://en.wikipedia.org/wiki/Make_(software)) (such as [GNU Make](https://www.gnu.org/software/make/))
- [C compiler](https://en.wikipedia.org/wiki/List_of_compilers#C_compilers) (such as [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/))
- [C standard library](https://en.wikipedia.org/wiki/C_standard_library) (such as [glibc](https://www.gnu.org/software/libc/))
- [NASM](https://www.nasm.us/)
- [GNU Linker](https://www.gnu.org/software/binutils/)

## Compile
You can compile debug or release version by running CMake with `CMAKE_BUILD_TYPE` set to either "Debug" or "Release".
### Debug
```sh
cmake -DCMAKE_BUILD_TYPE=Debug .
make
```
### Release
```sh
cmake -DCMAKE_BUILD_TYPE=Release .
make
```

## Usage
After running `make` the executable `bfcomp` will be either in `debug/` or `release/` depending on `CMAKE_BUILD_TYPE`.

Run
```sh
bfcomp --help
```
to display this help:
```
Usage: bfcomp [options]

Options:
  --help                -h  -- Displays help.
  --input_file <file>   -i  -- Sets input file.
  --output_file <file>  -o  -- Sets output file.
  --stack_size <value>  -s  -- Sets length of the stack.
  --cell_size <value>   -c  -- Sets cell size. (Accepts 1, 2, 4 or 8 bytes)
  --assembly            -S  -- Outputs assembly instead of an executable.
```

## Examples

### Cat
#### Code
```brainfuck
+[,.]
```
#### Compile
```sh
bfcomp -i examples/cat.bf -o cat
```
#### Run
```sh
./cat
```
```
This program writes its input directly to its output.
This program writes its input directly to its output.
```

### Hello World!
#### Code
```brainfuck
+++++ +++               Set Cell #0 to 8
[
    >++++               Add 4 to Cell #1; this will always set Cell #1 to 4
    [                   as the cell will be cleared by the loop
        >++             Add 4*2 to Cell #2
        >+++            Add 4*3 to Cell #3
        >+++            Add 4*3 to Cell #4
        >+              Add 4 to Cell #5
        <<<<-           Decrement the loop counter in Cell #1
    ]                   Loop till Cell #1 is zero
    >+                  Add 1 to Cell #2
    >+                  Add 1 to Cell #3
    >-                  Subtract 1 from Cell #4
    >>+                 Add 1 to Cell #6
    [<]                 Move back to the first zero cell you find; this will
                        be Cell #1 which was cleared by the previous loop
    <-                  Decrement the loop Counter in Cell #0
]                       Loop till Cell #0 is zero

>>.                     Cell #2 has value 72 which is 'H'
>---.                   Subtract 3 from Cell #3 to get 101 which is 'e'
+++++ ++..+++.          Likewise for 'llo' from Cell #3
>>.                     Cell #5 is 32 for the space
<-.                     Subtract 1 from Cell #4 for 87 to give a 'W'
<.                      Cell #3 was set to 'o' from the end of 'Hello'
+++.----- -.----- ---.  Cell #3 for 'rl' and 'd'
>>+.                    Add 1 to Cell #5 gives us an exclamation point
>++.                    And finally a newline from Cell #6
```
#### Compile
```sh
bfcomp -i examples/hello.bf -o hello
```
#### Run
```sh
./hello
```
```
Hello World!
```

### Get cell size
#### Code
```brainfuck
Calculate the value 256 and test if it's zero
++++++++[>++++++++<-]>[<++++>-]
>+<<[
    Not zero so calculate the value 65536 and check if it's zero
    [>++++<-]>[<++++++++>-]<[>++++++++<-]
    >[
        Not zero so calculate the value 4294967296 and check if it's zero
        [<++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++              
         +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
         +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
         ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++>-]
        <[>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++   
         +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
         +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++   
        +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++<-]
        >[>
            Print "64"
            ++++++++[>++++++<-]>.--.[-]<
        ]>[>
            Print "32"
            ++++++++++[>+++++<-]>+.-.[-]<
        <-]<
    ]>[>
        Print "16"
        +++++++[>+++++++<-]>.+++++.[-]<
    <-<<]
]>>[>
    Print "8"
    ++++++++[>+++++++<-]>.[-]<
<-]

Print " bit cells\n"
+++++++++++[>+++>+++++++++>+++++++++>+<<<<-]>-.>-.+++++++.+++++++++++.<.
>>.++.+++++++..<-.>>->++++++++++.
```
#### Compile
```sh
bfcomp -i examples/cell_size.bf -c 8 -o cell_size
```
#### Run
```sh
./cell_size
```
```
64 bit cells
```
