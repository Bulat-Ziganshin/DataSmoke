DataSmoke
=========

Datatype detection in order to choose appropriate compression algorithm or not compress at all. Since text, multimedia and incompresible files are better processed with specific algorithms, we need a fast and reliable way to detect those data. I call it data smoking. This project will provide various experimental algorithms that can recognize some of special datatypes (not necessary all), as well as samples of data that are especially hard to smoke correctly.


The full list of smells (speeds measured on the single core of i7-4770) and minimum recommended block sizes to provide meaningful results:

- Byte entropy: computes entropy of independent bytes (2 GB/s, 16 KBytes)
- Word entropy: computes entropy of independent 16-bit words (0.7-1.5 GB/s, 4 MBytes)
- Order-1 byte entropy: computes order-1 entropy of bytes (0.7-1.5 GB/s, 4 MBytes)
- DWord coverage: computes coverage of independent 32-bit dwords (3 GB/s, STEP*FILTER*64 bytes, default STEP=4, FILTER=16).


And examples of their work:

Text file (enwik9):
- Byte entropy: minimum 62.68%, average 64.20%, maximum 66.97%
- Word entropy: minimum 53.14%, average 55.97%, maximum 57.93%
- Order-1 byte entropy: minimum 42.43%, average 47.75%, maximum 48.88%
- DWord coverage: minimum 4.14%, average 10.37%, maximum 12.05%

Binary file:
- Byte entropy: minimum 48.49%, average 77.67%, maximum 93.62%
- Word entropy: minimum 33.09%, average 68.74%, maximum 92.00%
- Order-1 byte entropy: minimum 17.69%, average 59.81%, maximum 90.39%
- DWord coverage: minimum 1.78%, average 31.92%, maximum 92.00%

Compressed file:
- Byte entropy: minimum 100.00%, average 100.00%, maximum 100.00%
- Word entropy: minimum 99.93%, average 99.93%, maximum 99.93%
- Order-1 byte entropy: minimum 99.85%, average 99.86%, maximum 99.86%
- DWord coverage: minimum 96.20%, average 96.95%, maximum 98.01%


The whole idea of the library is that we get data of unknown type and analyze them to determine whether it's text, incompressible or so. Overall, I think that ByteSmoker should suffice for 95% of cases, failing only on repetitions of random data (such as two copies of the same zip file) and DWordSmoker should suffice on another 95%, failing on things like base64 encoding (random data of limited charset). I tend to consider as incompressible the data that has >90% for both ByteSmoker and DWordSmoker. Combining them together should provide us with algorithm processing 2 GB/s (since ByteSmoker and DWordSmoker employs different CPU resources) and able to determine almost any data that can be compressed by some algorithm.

Please provide us with the samples of compressible data having unusually high "entropy" values, especially in ByteSmoker and DWordSmoker simultaneously.
