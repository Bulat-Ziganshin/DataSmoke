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
