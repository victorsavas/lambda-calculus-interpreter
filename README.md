# lambda-calculus
An interpreter of the untyped lambda calculus written in the C Programming Language.

### Cloning
```bash
git clone github.com/octo-victor/lambda-calculus
cd lambda-calculus
```

### Compile and run
To compile using make and gcc, simply type `make` into the console, and a `lambda-calculus` output file will be generated in the main directory.
```bash
make
./lambda-calculus
```

### Debugging
To debug the program with the `-g` flag, use `BUILD_MODE=DEBUG`. Furthermore, to compile with AddressSanitizer, use `ASAN=1`.
```bash
make BUILD_MODE=DEBUG ASAN=1
./lambda-calculus
```

### Cleanup and recompile
To manually clean up object files and recompile the project, use `make clean`.
```bash
make clean && make
```
