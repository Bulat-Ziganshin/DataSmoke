char version[] = "DataSmoker 0.3  2014-02-10";

char copyright[] = "Developed by Bulat Ziganshin\n"
                   "The code is placed in public domain\n";


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define kb 1024
#define mb (kb*kb)
typedef unsigned char byte;

class Entropy
{
public:
  virtual const char* name() = 0;
  virtual ~Entropy() {}
  virtual void smoke (void *buf, size_t bufsize, double *entropy) = 0;
};


/**************************************************************************/
/* Byte entropy: calculate compression ratio with the 8-bit order-0 model */
/**************************************************************************/

class ByteEntropy : public Entropy
{
public:
  virtual const char* name()  {return "Byte entropy";};
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

void ByteEntropy::smoke (void *buf, size_t bufsize, double *entropy)
{
  size_t count1[256] = {0};
  size_t count2[256] = {0};
  size_t count3[256] = {0};
  size_t count4[256] = {0};

  byte *p = (byte*) buf;  int i;
  for (i=0; i<bufsize-3; i+=4)
    count1[ p[i]   ]++,
    count2[ p[i+1] ]++,
    count3[ p[i+2] ]++,
    count4[ p[i+3] ]++;
  for (; i<bufsize; i++)
    count1[ p[i]   ]++;

  double order0 = 0;
  for (int i=0; i<256; i++)
  {
    size_t count  =  count1[i] + count2[i] + count3[i] + count4[i];
    if (count)
      order0 += count * log(double(bufsize)/count)/log(double(2)) / 8;
  }

  *entropy  =  order0 / bufsize;
}


/***************************************************************************/
/* Word entropy: calculate compression ratio with the 16-bit order-0 model */
/***************************************************************************/

class WordEntropy : public Entropy
{
  uint32_t *count;
public:
  WordEntropy()               {count = new uint32_t[256*256];}
  virtual const char* name()  {return "Word entropy";};
  virtual ~WordEntropy()      {delete[] count;}
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

void WordEntropy::smoke (void *buf, size_t bufsize, double *entropy)
{
  memset (count, 0, 256*256*sizeof(*count));

  byte *p = (byte*) buf;
  for (int i=0; i<bufsize-1; i++)
    count[ *(unsigned*)(p+i) & 0xFFFF ]++;

  double order0 = 0;
  for (int i=0; i<256*256; i++)
  {
    if (count[i])
      order0 += count[i] * log(double(bufsize)/count[i])/log(double(2)) / 16;
  }

  *entropy  =  order0 / bufsize;
}


/**********************************************************************************/
/* Order-1 byte entropy: calculate compression ratio with the 8-bit order-1 model */
/**********************************************************************************/

class Order1Entropy : public Entropy
{
  uint32_t *count;
public:
  Order1Entropy()             {count = new uint32_t[256*256];}
  virtual const char* name()  {return "Order-1 byte entropy";};
  virtual ~Order1Entropy()    {delete[] count;}
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

void Order1Entropy::smoke (void *buf, size_t bufsize, double *entropy)
{
  memset (count, 0, 256*256*sizeof(*count));

  byte *p = (byte*) buf;
  for (int i=0; i<bufsize-1; i++)
    count[ *(unsigned*)(p+i) & 0xFFFF ]++;

  double order1 = 0;
  for (int i=0; i<256; i++)
  {
    size_t total = 0;
    for (int j=0; j<256; j++)
      total += count[i*256+j];

    if (total)
      for (int j=0; j<256; j++)
        if (count[i*256+j])
          order1 += count[i*256+j] * log(double(total)/count[i*256+j])/log(double(2)) / 8;
  }

  *entropy  =  order1 / bufsize;
}


/***************************************************************************/
/* DWord coverage: calculate which part of 32-bit dwords are unique        */
/***************************************************************************/

class DWordCoverage : public Entropy
{
  static const size_t HASHSIZE = 2*mb;  // it should be small enough to fit in most CPU last-level caches
  byte *table;
  size_t bits[256];
public:
  DWordCoverage();
  virtual const char* name()  {return "DWord coverage";};
  virtual ~DWordCoverage()    {delete[] table;}
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

DWordCoverage::DWordCoverage()
{
  table = new byte[HASHSIZE];
  bits[0] = 0;
  for (int i=1; i<256; i++)
    bits[i]  =  bits[i/2] + (i%2);
}

uint32_t hash_function (uint32_t x)
{
  uint64_t hash  =  x * uint64_t(123456791u);
  return uint32_t(hash>>32) ^ uint32_t(hash);
}

void DWordCoverage::smoke (void *buf, size_t bufsize, double *entropy)
{
  const size_t   STEP = 1;        // Check only every n'th position
  const uint32_t FILTER = 16;     // Of those checked, count only every n'th hash
  const uint32_t FILTER_MAX_HASH  =  uint32_t(-1) / FILTER;   // Count only hashes smaller or equal to this value
  const uint32_t FILTER_HASH_DIVIDER  =  (FILTER_MAX_HASH / CHAR_BIT) + 1;  // Dividing filtered hashes by this value will leave only 3 highest bits required to address bit in the byte

  memset(table,0,HASHSIZE);
  byte *p = (byte*) buf;
  size_t total_hashes = 0;

  for (size_t i=0; i<bufsize-sizeof(uint32_t)+1; i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    if (hash <= FILTER_MAX_HASH)
      total_hashes++,
      table[hash % HASHSIZE]  |=  1 << (hash/FILTER_HASH_DIVIDER);
  }

  if (total_hashes==0)
    {*entropy=0; return;}

  size_t unique_hashes = 0;
  for (size_t i=0; i<HASHSIZE; i++)
    unique_hashes += bits[table[i]];

  // Coverage is ratio of unique hashes to the total amount of hashes checked
  *entropy  =  double(unique_hashes) / total_hashes;
}


/***************************************************************************/
/* DWord coverage: calculate which part of 32-bit dwords are unique        */
/***************************************************************************/

class TwoPassDWordCoverage : public Entropy
{
  static const size_t HASHSIZE = 256*256;
  uint32_t *table;
public:
  TwoPassDWordCoverage()            {table = new uint32_t[HASHSIZE];}
  virtual const char* name()        {return "2-pass DWord coverage";};
  virtual ~TwoPassDWordCoverage()   {delete[] table;}
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

void TwoPassDWordCoverage::smoke (void *buf, size_t bufsize, double *entropy)
{
  const size_t STEP = 1;        // Check only every n'th position
  byte *p = (byte*) buf;

  memset(table,0,HASHSIZE*sizeof(*table));
  for (size_t i=0;  i<bufsize-sizeof(uint32_t)+1;  i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    table[hash%HASHSIZE]++;
  }

  size_t max_i=0;  uint32_t max_count=0;
  for (size_t i=0; i<HASHSIZE; i++)
    if (table[i] > max_count)
      max_i=i,  max_count=table[i];

  memset(table,0,HASHSIZE*sizeof(*table));
  for (size_t i=0;  i<bufsize-sizeof(uint32_t)+1;  i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    if (hash%HASHSIZE == max_i)
      table[hash/HASHSIZE]=1;
  }

  size_t unique_hashes = 0;
  for (size_t i=0; i<(1u<<31)/(HASHSIZE/2); i++)
    unique_hashes += table[i];

  // Coverage is ratio of unique hashes to the total amount of hashes checked
  *entropy  =  double(unique_hashes) / max_count;
  //printf("\n%d / %d = %.2lf%%", int(unique_hashes), int(max_count), *entropy*100);
}


/**********************************************************************/
/* Supplementary code                                                 */
/**********************************************************************/

int main (int argc, char **argv)
{
  if (argc==1)
  {
    printf("%s", version);
    printf("\n\nUsage: smoke infiles...\n\n%s", copyright);
    return EXIT_FAILURE;
  }

  for (int file=1; file < argc; file++)
  {
    const int BUFSIZE = 4*mb;
    static char buf[BUFSIZE];

    FILE *infile  = fopen (argv[file], "rb");  if (infile==NULL)  {printf("Can't open input file %s!\n",    argv[file]); return EXIT_FAILURE;}
    int width = strlen(argv[file]);  width = width>21? width : 21;
    printf("%s%*s | min %% | avg %% | max %% | incompressible %dMB blocks", file>1?"\n":"", width, argv[file], BUFSIZE/mb);

    ByteEntropy   ByteS;
    WordEntropy   WordS;
    Order1Entropy Order1S;
    DWordCoverage DWordS;
    TwoPassDWordCoverage TwoPassDWordS;
    Entropy *smokers[] = {&ByteS, &WordS, &Order1S, &DWordS, &TwoPassDWordS};
    const int NumSmokers = sizeof(smokers)/sizeof(*smokers);
    double entropy, min_entropy[NumSmokers], avg_entropy[NumSmokers], max_entropy[NumSmokers];  int incompressible[NumSmokers];

    uint64_t origsize = 0;  int blocks = 0;
    for(;;)
    {
      int bytes_read = fread(buf, 1, BUFSIZE, infile);
      if (bytes_read==0) break;

      for (int i=0; i<NumSmokers; i++)
      {
        smokers[i]->smoke(buf, bytes_read, &entropy);
        if (origsize==0) {                 // first block
          min_entropy[i] = max_entropy[i] = entropy;
          avg_entropy[i] = incompressible[i] = 0;
        } else if (bytes_read==BUFSIZE) {  // update min/max entropy only on complete blocks
          if (entropy < min_entropy[i])
            min_entropy[i] = entropy;
          if (entropy > max_entropy[i])
            max_entropy[i] = entropy;
        }
        avg_entropy[i] += entropy*bytes_read;
        incompressible[i] += (entropy > 0.95);
      }

      origsize += bytes_read;  blocks++;
    }
    fclose(infile);

    printf("\n");  for(int i=0; i<width; i++) printf("-");  printf("-|------:|------:|------:|----------------------------\n");
    for (int i=0; i<NumSmokers; i++)
      printf("%*s |%6.2lf |%6.2lf |%6.2lf | %d of %d\n", width, smokers[i]->name(), min_entropy[i]*100, avg_entropy[i]/origsize*100, max_entropy[i]*100, incompressible[i], blocks);
  }

  return EXIT_SUCCESS;
}
