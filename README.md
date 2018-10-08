# NES-HLS

An NES emulator implemented on FPGA by using Intel HLS Compiler.

To increase performance, my 6502 processor is a little different from original one.


## Summary report
This is a part of summary report of quartus prime. Target board is DE10-Lite (MAX10 10M50DAF484C7G)

| summay |  |
----|----
| Total logic elements | 17,011 ( 34 % ) |
| Total registers | 14,169 ( 28 % ) |
| Total memory bits    | 845,334 ( 50 % ) |
| M9Ks | 179 ( 98 % )|
| Frequency (MHz) | 169.98 |

## Support ROMs
Now supports only mapper 0.
Super Mario Bros and Donkey Kong work correctly. 

