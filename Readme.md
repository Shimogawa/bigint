# Bigint

The C implementation of big integer.

## Usage

Include `bigint.h`.

To convert a string to a `bigint`:

```c
const char* s = "9304294823748978957892374892374983274239871";
bigint* bi = BINT_atoi(s);
```

To convert a `bigint` back to a string:

```c
char* s = BINT_itoa(bi);
printf("%s\n", s);
free(s);
```

Remember to free the string because it is `malloc`ed.

To create a `bigint` from bytes (uint8 array),

```c
size_t sz = 100;
uint8_t* arr = malloc(sizeof(uint8_t) * 100);
bigint* bi = BINT_makep(arr, sz);
```

Remember to free the `bigint` after using.

```c
BINT_free(bi);
```

Arithmetic functions:

- `BINT_add`: only supports non-negative now.
- `BINT_mul`

```c
bigint* res = BINT_make(); // makes an empty bigint
BINT_mul(a, b, res); // res = a * b
```
