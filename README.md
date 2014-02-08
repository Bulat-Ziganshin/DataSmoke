DataSmoke
=========

Datatype detection in order to choose appropriate compression algorithm.

Since already compressed, text and multimedia files are better compressed with specific algorithms, we need a fast and reliable way to detect those data. I call it data smoking.

This project will provide various experimental algorithms that can recognize some of special datatypes (not necessary all), as well as samples of data that are especially hard to smoke correctly.


The full list of smells:

- ByteSmoker: computes entropy of individual bytes (2 GB/s).
- WordSmoker: computes entropy of 16-bit words (1 GB/s).
- DWordSmoker: computes entropy of 32-bit dwords (3 GB/s).
