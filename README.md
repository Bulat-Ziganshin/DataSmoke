DataSmoke
=========

Datatype detection in order to choose appropriate compression algorithm.

Since already compressed, text and multimedia files are better compressed with specific algorithms, we need a fast and reliable way to detect those data. I call it data smoking.

This project will provide various experimental algorithms that can recognize some of special datatypes (not necessary all), as well as samples of data that are especially hard to smoke correctly.


The full list of algorithms:

- ByteDistribution: computes entropy of individual bytes.
- WordDistribution: computes entropy of 16-bit words.
- DWordDistribution: computes entropy of 32-bit dwords.
