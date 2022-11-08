# Spla Examples

## C++ API

### Matrix-market data loading

Use `MtxLoader` class to load graph or matrix data in `.mtx` format from disc. Loader allows configuration of parsing
mode, removing self-loops, doubling edges, offsetting indices and evaluation statistics for loaded data. Loader builds
adaptive vertices degree distribution histogram, so it is easier to investigate properties of a graph.

Example output of loading `bcsstk33.mtx` data from [SuiteSparse Matrix Collection](https://sparse.tamu.edu):

```text
Loading matrix-market coordinate format data...
 Reading from "./bcsstk33.mtx"
 Matrix size 8738 rows, 8738 cols
 Removing self-loops
 Offsetting indices by -1
 Doubling edges
 Read data: 300334 lines, 300321 directed edges
 Parsed in 0.276823 sec
 Calc stats in 0.000554 sec
 Loaded in 0.279935 sec, 583166 edges total
 deg: min 19, max 140, avg 66.7391, sd 16.1078
 distribution:
  [  19 -   45):    9.6% *********
  [  45 -   54):     29% ****************************
  [  54 -   55):      0%
  [  55 -   56):  0.046%
  [  56 -   63):    7.7% *******
  [  63 -   79):    6.1% ******
  [  79 -   81):     45% *********************************************
  [  81 -   82):  0.011%
  [  82 -   83):  0.011%
  [  83 -   84):   0.08%
  [  84 -   85):      0%
  [  85 -  141):    2.4% **
```

Example output of loading `hollywood-2009.mtx` data from [SuiteSparse Matrix Collection](https://sparse.tamu.edu):

```text
Loading matrix-market coordinate format data...
 Reading from "./hollywood-2009.mtx"
 Matrix size 1139905 rows, 1139905 cols
 Removing self-loops
 Offsetting indices by -1
 Doubling edges
 Read data: 57515664 lines, 57515616 directed edges
 Parsed in 58.11 sec
 Calc stats in 0.084409 sec
 Loaded in 58.1974 sec, 112751422 edges total
 deg: min 0, max 11467, avg 98.913, sd 271.865
 distribution:
  [      0 -       3):    7.2% *******
  [      3 -       5):    4.6% ****
  [      5 -       7):    4.2% ****
  [      7 -      10):    6.5% ******
  [     10 -      13):    5.6% *****
  [     13 -      15):    3.6% ***
  [     15 -      19):    6.3% ******
  [     19 -      22):    4.3% ****
  [     22 -      26):      5% *****
  [     26 -      31):    5.4% *****
  [     31 -      37):    5.3% *****
  [     37 -      45):    5.7% *****
  [     45 -      54):      5% ****
  [     54 -      67):    5.2% *****
  [     67 -      86):      5% *****
  [     86 -     119):    5.2% *****
  [    119 -     187):    5.3% *****
  [    187 -     389):    5.2% *****
  [    389 -   11468):    5.3% *****
```