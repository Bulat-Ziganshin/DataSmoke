DataSmoke
=========

Datatype detection in order to choose appropriate compression algorithm or not compress at all.

Since text, multimedia and incompresible files are better processed with specific algorithms, we need a fast and reliable way to detect those data. I call it data smoking. This project will provide various experimental algorithms that can recognize some of special datatypes (not necessary all), as well as samples of data that are especially hard to smoke correctly.


The full list of smokers (with speeds measured on the single core of i7-4770 and minimum recommended block sizes to provide meaningful results):

- Byte entropy: computes entropy of independent bytes (2 GB/s, 16 KBytes)
- Word entropy: computes entropy of independent 16-bit words (0.7-1.5 GB/s, 4 MBytes)
- Order-1 byte entropy: computes order-1 entropy of bytes (0.7-1.5 GB/s, 4 MBytes)
- DWord coverage: computes coverage of independent 32-bit dwords (0.5 GB/s, STEP*FILTER*64 bytes, default STEP=1, FILTER=16).

-
And examples of their work:

   Text file (enwik9) | min % | avg % | max % | incompressible 4MB blocks
----------------------|------:|------:|------:|----------------------------
         Byte entropy | 62.68 | 64.20 | 66.97 | 0 of 239
         Word entropy | 53.14 | 55.97 | 57.93 | 0 of 239
 Order-1 byte entropy | 42.43 | 47.75 | 48.88 | 0 of 239
   DWord hash entropy | 71.04 | 83.69 | 85.85 | 0 of 239
       DWord coverage |  1.93 |  5.36 |  6.75 | 0 of 239
2-pass DWord coverage |  1.42 |  4.29 |  5.25 | 0 of 239
2-pass QWord coverage |  8.68 | 35.04 | 41.65 | 0 of 239

          Binary file | min % | avg % | max % | incompressible 4MB blocks
----------------------|------:|------:|------:|----------------------------
         Byte entropy | 48.49 | 77.67 | 93.62 | 0 of 239
         Word entropy | 33.09 | 68.74 | 92.00 | 0 of 239
 Order-1 byte entropy | 17.69 | 59.81 | 90.39 | 0 of 239
   DWord hash entropy | 39.29 | 85.75 | 99.82 | 2 of 239:  92, 94
       DWord coverage |  2.07 | 25.04 | 90.34 | 0 of 239
2-pass DWord coverage |  0.27 | 13.82 | 87.31 | 0 of 239
2-pass QWord coverage |  0.76 | 29.29 | 97.91 | 2 of 239:  92, 94

16-bit stereo WAV file | min % | avg % | max % | incompressible 4MB blocks
-----------------------|------:|------:|------:|----------------------------
          Byte entropy | 96.70 | 97.40 | 98.39 | 9 of 10:  0, 1, 2, 3, 4, 5, 6, 7, 8
          Word entropy | 95.84 | 96.87 | 98.11 | 9 of 10:  0, 1, 2, 3, 4, 5, 6, 7, 8
  Order-1 byte entropy | 94.98 | 96.35 | 97.84 | 8 of 10:  1, 2, 3, 4, 5, 6, 7, 8
    DWord hash entropy | 99.71 | 99.43 | 99.93 | 9 of 10:  0, 1, 2, 3, 4, 5, 6, 7, 8
        DWord coverage | 97.83 | 97.69 | 99.05 | 9 of 10:  0, 1, 2, 3, 4, 5, 6, 7, 8
 2-pass DWord coverage | 92.58 | 96.95 | 99.17 | 8 of 10:  1, 2, 3, 4, 5, 6, 7, 8
 2-pass QWord coverage | 93.63 | 97.36 | 99.25 | 8 of 10:  1, 2, 3, 4, 5, 6, 7, 8

      Compressed file | min % | avg % | max % | incompressible 4MB blocks
----------------------|------:|------:|------:|----------------------------
         Byte entropy |100.00 |100.00 |100.00 | 67 of 67
         Word entropy | 99.93 | 99.93 | 99.93 | 67 of 67
 Order-1 byte entropy | 99.85 | 99.86 | 99.86 | 67 of 67
   DWord hash entropy | 99.93 | 99.93 | 99.93 | 67 of 67
       DWord coverage | 99.16 | 99.23 | 99.26 | 67 of 67
2-pass DWord coverage | 99.15 | 99.22 | 99.25 | 67 of 67
2-pass QWord coverage | 99.03 | 99.21 | 99.26 | 67 of 67

2 copies of compressed file | min % | avg % | max % | incompressible 4MB blocks
----------------------------|------:|------:|------:|----------------------------
               Byte entropy | 99.98 | 99.98 | 99.98 | 1 of 1
               Word entropy | 99.80 | 99.80 | 99.80 | 1 of 1
       Order-1 byte entropy | 99.62 | 99.62 | 99.62 | 1 of 1
         DWord hash entropy | 99.85 | 99.85 | 99.85 | 1 of 1
             DWord coverage | 49.33 | 49.33 | 49.33 | 0 of 1
      2-pass DWord coverage | 49.12 | 49.12 | 49.12 | 0 of 1
      2-pass QWord coverage | 49.22 | 49.22 | 49.22 | 0 of 1

-
And here with 1GB blocks:

   Text file (enwik9) | min % | avg % | max % | incompressible 1GB blocks
----------------------|------:|------:|------:|----------------------------
         Byte entropy | 64.46 | 64.46 | 64.46 | 0 of 1
         Word entropy | 56.57 | 56.57 | 56.57 | 0 of 1
 Order-1 byte entropy | 48.69 | 48.69 | 48.69 | 0 of 1
   DWord hash entropy | 85.56 | 85.56 | 85.56 | 0 of 1
       DWord coverage |  0.34 |  0.34 |  0.34 | 0 of 1
2-pass DWord coverage |  0.02 |  0.02 |  0.02 | 0 of 1
2-pass QWord coverage |  1.62 |  1.62 |  1.62 | 0 of 1

          Binary file | min % | avg % | max % | incompressible 1GB blocks
----------------------|------:|------:|------:|----------------------------
         Byte entropy | 81.33 | 81.33 | 81.33 | 0 of 1
         Word entropy | 75.01 | 75.01 | 75.01 | 0 of 1
 Order-1 byte entropy | 68.70 | 68.70 | 68.70 | 0 of 1
   DWord hash entropy | 90.54 | 90.54 | 90.54 | 0 of 1
       DWord coverage |  7.25 |  7.25 |  7.25 | 0 of 1
2-pass DWord coverage |  0.06 |  0.06 |  0.06 | 0 of 1
2-pass QWord coverage |  0.22 |  0.22 |  0.22 | 0 of 1

16-bit stereo WAV file | min % | avg % | max % | incompressible 1GB blocks
-----------------------|------:|------:|------:|----------------------------
          Byte entropy | 97.80 | 97.80 | 97.80 | 1 of 1
          Word entropy | 97.30 | 97.30 | 97.30 | 1 of 1
  Order-1 byte entropy | 96.80 | 96.80 | 96.80 | 1 of 1
    DWord hash entropy | 99.75 | 99.75 | 99.75 | 1 of 1
        DWord coverage | 89.80 | 89.80 | 89.80 | 0 of 1
 2-pass DWord coverage | 66.10 | 66.10 | 66.10 | 0 of 1
 2-pass QWord coverage | 74.38 | 74.38 | 74.38 | 0 of 1

      Compressed file | min % | avg % | max % | incompressible 1GB blocks
----------------------|------:|------:|------:|----------------------------
         Byte entropy |100.00 |100.00 |100.00 | 1 of 1
         Word entropy |100.00 |100.00 |100.00 | 1 of 1
 Order-1 byte entropy |100.00 |100.00 |100.00 | 1 of 1
   DWord hash entropy |100.00 |100.00 |100.00 | 1 of 1
       DWord coverage | 62.27 | 62.27 | 62.27 | 0 of 1
2-pass DWord coverage | 96.77 | 96.77 | 96.77 | 1 of 1
2-pass QWord coverage | 96.85 | 96.85 | 96.85 | 1 of 1

2 copies of compressed file  | min % | avg % | max % | incompressible 1GB blocks
-----------------------------|------:|------:|------:|----------------------------
                Byte entropy |100.00 |100.00 |100.00 | 1 of 1
                Word entropy |100.00 |100.00 |100.00 | 1 of 1
        Order-1 byte entropy |100.00 |100.00 |100.00 | 1 of 1
          DWord hash entropy |100.00 |100.00 |100.00 | 1 of 1
              DWord coverage | 29.63 | 29.63 | 29.63 | 0 of 1
       2-pass DWord coverage | 48.18 | 48.18 | 48.18 | 0 of 1
       2-pass QWord coverage | 48.19 | 48.19 | 48.19 | 0 of 1

-
The whole idea of the library is that we get data of unknown type and analyze them to determine whether it's text, incompressible or so. Overall, I think that ByteEntropy should suffice for 95% of cases, failing only on repetitions of random data (such as two copies of the same zip file) and DWordCoverage should suffice on another 95%, failing on things like base64 encoding (random data of limited charset). I tend to consider as incompressible the data that has >95% for both ByteEntropy and DWordCoverage. Combining them together should provide us with algorithm processing 2 GB/s (since ByteSmoker and DWordSmoker employs different CPU resources) and able to determine almost any data that can be compressed by some algorithm.

Please provide us with the samples of compressible data having unusually high "entropy" values, especially in ByteEntropy and DWordCoverage simultaneously.
