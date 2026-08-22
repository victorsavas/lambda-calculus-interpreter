# lambda-calculus
An interpreter of the untyped lambda calculus written in the C Programming Language. The lambda calculus is a formal system for representing functions created by Alonzo Church during his studies into the foundations of mathematics. More on the lambda calculus and its theory in the [Stanford Encyclopedia of Philosophy](https://plato.stanford.edu/entries/lambda-calculus/).

## Quick start

### Cloning
```bash
git clone github.com/octo-victor/lambda-calculus
cd lambda-calculus
```

### Compile and run
To compile using make and [gcc](https://gcc.gnu.org/), simply type `make` into the console, and a `lambda-calculus` output file will be generated in the main directory.
```bash
make
./lambda-calculus
```

### Debugging
To debug the program with the `-g` flag, use `BUILD_MODE=DEBUG`. Furthermore, to compile with [Address Sanitizer](https://github.com/google/sanitizers/wiki/addresssanitizer), use `ASAN=1`.
```bash
make BUILD_MODE=DEBUG ASAN=1
./lambda-calculus
```

### Cleanup and recompile
To manually clean up object files and recompile the project, use `make clean`.
```bash
make clean && make
```
