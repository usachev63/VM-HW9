POSIX only

After git clone, please initialize all submodules:
```sh
git submodule update --init --recursive
```

Compile & run unit-tests:

```sh
mkdir -p build
cd build
cmake ..
make
./tests
```
