DataSmoke
=========

Datatype detection in order to choose appropriate compression algorithm or not compress at all.

Since text, multimedia and incompresible files are better processed with specific algorithms, we need a fast and reliable way to detect those data. I call it data smoking. This project will provide various experimental algorithms that can recognize some of special datatypes (not necessary all), as well as samples of data that are especially hard to smoke correctly.


The full list of smells (speeds measured on the single core of i7-4770) and minimum recommended block sizes to provide meaningful results:

- Byte entropy: computes entropy of independent bytes (2 GB/s, 16 KBytes)
- Word entropy: computes entropy of independent 16-bit words (0.7-1.5 GB/s, 4 MBytes)
- Order-1 byte entropy: computes order-1 entropy of bytes (0.7-1.5 GB/s, 4 MBytes)
- DWord coverage: computes coverage of independent 32-bit dwords (0.5 GB/s, STEP*FILTER*64 bytes, default STEP=1, FILTER=16).


And examples of their work:

  Text file (enwik9) | min % | avg % | max % | incompressible 4MB blocks
---------------------|------:|------:|------:|----------------------------
        Byte entropy | 62.68 | 64.20 | 66.97 | 0 of 239
        Word entropy | 53.14 | 55.97 | 57.93 | 0 of 239
Order-1 byte entropy | 42.43 | 47.75 | 48.88 | 0 of 239
      DWord coverage |  2.34 |  5.15 |  6.39 | 0 of 239

         Binary file | min % | avg % | max % | incompressible 4MB blocks
---------------------|------:|------:|------:|----------------------------
        Byte entropy | 48.49 | 77.67 | 93.62 | 0 of 239
        Word entropy | 33.09 | 68.74 | 92.00 | 0 of 239
Order-1 byte entropy | 17.69 | 59.81 | 90.39 | 0 of 239
      DWord coverage |  0.38 | 13.74 | 87.16 | 0 of 239

16-bit stereo WAV file | min % | avg % | max % | incompressible 4MB blocks
-----------------------|------:|------:|------:|----------------------------
          Byte entropy | 96.70 | 97.40 | 98.39 | 9 of 10
          Word entropy | 95.84 | 96.87 | 98.11 | 9 of 10
  Order-1 byte entropy | 94.98 | 96.35 | 97.84 | 8 of 10
        DWord coverage | 92.17 | 95.82 | 99.13 | 8 of 10

     Compressed file | min % | avg % | max % | incompressible 4MB blocks
---------------------|------:|------:|------:|----------------------------
        Byte entropy |100.00 |100.00 |100.00 | 67 of 67
        Word entropy | 99.93 | 99.93 | 99.93 | 67 of 67
Order-1 byte entropy | 99.85 | 99.86 | 99.86 | 67 of 67
      DWord coverage | 99.13 | 99.18 | 99.23 | 67 of 67


The whole idea of the library is that we get data of unknown type and analyze them to determine whether it's text, incompressible or so. Overall, I think that ByteSmoker should suffice for 95% of cases, failing only on repetitions of random data (such as two copies of the same zip file) and DWordSmoker should suffice on another 95%, failing on things like base64 encoding (random data of limited charset). I tend to consider as incompressible the data that has >90% for both ByteSmoker and DWordSmoker. Combining them together should provide us with algorithm processing 2 GB/s (since ByteSmoker and DWordSmoker employs different CPU resources) and able to determine almost any data that can be compressed by some algorithm.

Please provide us with the samples of compressible data having unusually high "entropy" values, especially in ByteSmoker and DWordSmoker simultaneously.
