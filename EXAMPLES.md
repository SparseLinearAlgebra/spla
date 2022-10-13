# Spla Examples

## C++ API

### Matrix-market data loading

Use `MtxLoader` class to load graph or matrix data in `.mtx` format from disc. Loader allows configuration of parsing
mode, removing self-loops, doubling edges, offsetting indices and evaluation statistics for loaded data. Example output
of loading `bcsstk33.mtx` data from [SuiteSparse Matrix Collection](https://sparse.tamu.edu/HB/bcsstk33):

```text
Loading matrix-market coordinate format data...
 Reading from "./bcsstk33.mtx"
 Matrix size 8738 rows, 8738 cols
 Removing self-loops
 Offsetting indices by -1
 Doubling edges
 Read data: 300334 lines, 300321 directed edges
 Parsed in 3.0644 sec
 Calc stats in 0.003426 sec
 Loaded in 3.07206 sec, 583166 edges total
 deg: min 19, max 140, avg 66.7391, sd 16.1078
 distribution:
  [  19 -   31]:    6.8% ******
  [  31 -   43]:     32% *******************************
  [  43 -   55]:      8% ********
  [  55 -   67]:     12% ***********
  [  67 -   79]:     39% ***************************************
  [  79 -   91]:    1.5% *
  [  91 -  103]:   0.79%
  [ 103 -  115]:      0%
  [ 115 -  127]:  0.046%
  [ 127 -  140]:  0.023%
```