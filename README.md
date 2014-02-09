DataSmoke
=========

Datatype detection in order to choose appropriate compression algorithm or not compress at all. Since text, multimedia and incompresible files are better processed with specific algorithms, we need a fast and reliable way to detect those data. I call it data smoking. This project will provide various experimental algorithms that can recognize some of special datatypes (not necessary all), as well as samples of data that are especially hard to smoke correctly.


The full list of smells (speeds measured on the single core of i7-4770) and their minimal sizes of processed blocks to provide meaningful results:

- ByteSmoker: computes entropy of independent bytes (2 GB/s). Minimum recommended block is 4 KBytes.
- WordSmoker: computes entropy of independent 16-bit words (0.7-1.5 GB/s). Minimum recommended block is 1 MBytes.
- Order1Smoker: computes order-1 entropy of bytes (0.7-1.5 GB/s). Minimum recommended block is 1 MBytes.
- DWordSmoker: computes coverage of independent 32-bit dwords (3 GB/s). Minimum recommended block is STEP*FILTER Kbytes (default STEP=4, FILTER=16).


And examples of their work:

Text file (enwik9):
- ByteSmoker entropy: minimum 62.68%, average 64.20%, maximum 66.97%
- WordSmoker entropy: minimum 53.14%, average 55.97%, maximum 57.93%
- Order1Smoker entropy: minimum 42.43%, average 47.75%, maximum 48.88%
- DWordSmoker entropy: minimum 4.14%, average 10.37%, maximum 16.01%

Binary file:
- ByteSmoker entropy: minimum 48.49%, average 77.67%, maximum 93.62%
- WordSmoker entropy: minimum 33.09%, average 68.74%, maximum 92.00%
- Order1Smoker entropy: minimum 17.69%, average 59.81%, maximum 90.39%
- DWordSmoker entropy: minimum 1.78%, average 31.92%, maximum 92.00%

Compressed file:
- ByteSmoker entropy: minimum 100.00%, average 100.00%, maximum 100.00%
- WordSmoker entropy: minimum 99.75%, average 99.93%, maximum 99.93%
- Order1Smoker entropy: minimum 99.49%, average 99.86%, maximum 99.86%
- DWordSmoker entropy: minimum 96.20%, average 96.95%, maximum 98.04%


The whole idea of the library is that we get data of unknown type and analyze them to determine whether it's text, incompressible or so. Overall, I think that ByteSmoker should suffice for 95% of cases, failing only on repetitions of random data (such as two copies of the same zip file) and DWordSmoker should suffice on another 95%, failing on things like base64 encoding (random data of limited charset). I tend to consider as incompressible the data that has >90% for both ByteSmoker and DWordSmoker. Combining them together should provide us with algorithm processing 2 GB/s (since ByteSmoker and DWordSmoker employs different CPU resources) and able to determine almost any data that can be compressed by some algorithm.

Please provide us with the samples of compressible data having unusually high "entropy" values, especially in ByteSmoker and DWordSmoker simultaneously.
