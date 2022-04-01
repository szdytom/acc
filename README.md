# acc
A zero-dependence (sub) C compiler working in progress written in pure ISO C.

## Build from source

Make sure you have `gcc` and `cmake` on your PATH.
If you don't, here's command for ubuntu:
```
sudo apt install build-essential cmake
```

To build, run:
```
cd build
cmake ..
make
```

## Usage

```
acc target inputfile (outputfile)
```

Output targets now includes:
- `x86_64`: Intel's x84-64 ASM
- `llvm`: LLVM's IR
- `ast`: (used for debugging) Abstruct Syntax Tree 

Example:

```
acc x86_64 test.c
```
