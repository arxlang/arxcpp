# Roadmap

The roadmap document define the direction that the project is taking.

The initial and decisive part of the project is the implementation of the
Apache Arrow datatypes as the native datatypes.

## Implement Apache Arrow datatypes

ArxLang is based on [Kaleidoscope compiler](https://llvm.org/docs/tutorial/),
so it just implements float data type for now.

In order to accept more datatypes, the language should have a way to specify
the type for each variable and function returning.

* wave 1: float32
* wave 2: int8, int16, int32, int64
* wave 3: float16, float64
* wave 4: string
* wave 5: datetime
