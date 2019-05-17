# 🌸 hana

**hana**, a small dynamically-typed scripting language. It is written in Rust/C
and is inspired by Pascal, Ruby and Javascript. It primarily supports prototype-based
object orientation and first-class functions (with closure support). The language
has a simple mark-and-sweep garbage collector, meaning you won't have to worry
about manual memory allocation.

**haru**, the Rust parser/runtime generates bytecode that runs on an optimised
virtual machine written in C (about as fast as Python and Ruby!)

## Building

(building was tested by using rust-nightly-2019-05-01 and gcc-8 on an x64 with Linux, mileage
may vary on other architectures)

Just do:

```
cargo build
```

## Running

Once built, you can write hana code into a source file, then invoke the interpreter like this:

```
./main program.hana
```

## Documentation

*see [DOCUMENTATION.md](/DOCUMENTATION.md)*

## Examples

*see [DOCUMENTATION.md#Examples](/DOCUMENTATION.md#examples) for more*

### Hello World

```
print("Hello World\n")
```

### Variables

```
name = "Alice"
age = 20
print(name, " is ", age, " years old.\n")
```

### Fibonacci numbers

```
// Regular recursive
fib(n) = n <= 1 ? 1 : fib(n-1) + fib(n-2)
print(fib(30), "\n")

// Faster recursive (with tail-call optimization!)
fibrec(n, prev, curr) = n <= 0 ? curr : fibrec(n-1, prev+curr, prev)
fib(n) = fibrec(n+1, 1, 0)
print(fib(50), "\n")
```

## License

GPLv3 License
