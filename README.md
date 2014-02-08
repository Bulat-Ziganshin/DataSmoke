DataSmoke
=========

Datatype detection in order to choose appropriate compression algorithm.

Since incompresible, text and multimedia files are better compressed with specific algorithms, we need a fast and reliable way to detect those data. I call it data smoking.

This project will provide various experimental algorithms that can recognize some of special datatypes (not necessary all), as well as samples of data that are especially hard to smoke correctly.


The full list of smells (speeds measured on the single core of i7-4770):

- ByteSmoker: computes entropy of individual bytes (2 GB/s).
- WordSmoker: computes entropy of 16-bit words (0.7-1.5 GB/s).
- DWordSmoker: computes entropy of 32-bit dwords (3 GB/s).
- Order1Smoker: computes order-1 entropy of 8-bit bytes (0.7-1.5 GB/s).


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
