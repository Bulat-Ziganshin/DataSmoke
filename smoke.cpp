char version[] = "DataSmoker 0.2  2014-02-09";

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
/* DWord coverage: calculate that part of 32-bit dwords are unique         */
/***************************************************************************/

const size_t HASHSIZE = 128*kb;

class DWordCoverage : public Entropy
{
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
  const size_t   STEP = 4;        // Check only every n'th position
  const uint32_t FILTER = 16;     // Of those checked, count only every n'th hash
  const uint32_t FILTER_MAX_HASH  =  uint32_t(-1) / FILTER;   // Count only hashes smaller or equal to this value
  const uint32_t FILTER_HASH_DIVIDER  =  (FILTER_MAX_HASH / CHAR_BIT) + 1;  // Dividing filtered hashes by this value will leave only 3 highest bits required to address bit in the byte

  memset(table,0,HASHSIZE);
  byte *p = (byte*) buf;
  for (size_t i=0; i<bufsize-STEP+1; i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    if (hash <= FILTER_MAX_HASH)
      table[hash % HASHSIZE]  |=  1 << (hash/FILTER_HASH_DIVIDER);
  }

  size_t count = 0;
  for (size_t i=0; i<HASHSIZE; i++)
    count += bits[table[i]];

  // We have checked bufsize/(STEP*FILTER) hashes and found `count` original values among them
  *entropy  =  count / (double(bufsize)/(STEP*FILTER));
}


/**********************************************************************/
/* Supplementary code                                                 */
/**********************************************************************/

static char* show3 (uint64_t n, char *buf, const char *prepend="")
{
    char *p = buf + 27+strlen(prepend);
    int i = 4;

    *p = '\0';
    do {
        if (!--i) *--p = ',', i = 3;
        *--p = '0' + (n % 10);
    } while (n /= 10);

    memcpy (p-strlen(prepend), prepend, strlen(prepend));
    return p-strlen(prepend);
}


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
    printf("%sProcessing %s in %dMB blocks: ", file>1?"\n":"", argv[file], BUFSIZE/mb);

    ByteEntropy   ByteS;
    WordEntropy   WordS;
    DWordCoverage DWordS;
    Order1Entropy Order1S;
    Entropy *smokers[] = {&ByteS, &WordS, &Order1S, &DWordS};
    const int NumSmokers = sizeof(smokers)/sizeof(*smokers);
    double entropy, min_entropy[NumSmokers], avg_entropy[NumSmokers], max_entropy[NumSmokers];

    uint64_t origsize = 0;
    for(;;)
    {
      int bytes_read = fread(buf, 1, BUFSIZE, infile);
      if (bytes_read==0) break;

      for (int i=0; i<NumSmokers; i++)
      {
        smokers[i]->smoke(buf, bytes_read, &entropy);
        if (origsize==0) {                 // first block
          min_entropy[i] = max_entropy[i] = entropy;
          avg_entropy[i] = 0;
        } else if (bytes_read==BUFSIZE) {  // update min/max entropy only on complete blocks
          if (entropy < min_entropy[i])
            min_entropy[i] = entropy;
          if (entropy > max_entropy[i])
            max_entropy[i] = entropy;
        }
        avg_entropy[i] += entropy*bytes_read;
      }

      origsize += bytes_read;
    }
    fclose(infile);

    char temp1[100];  printf("%s bytes\n", show3(origsize,temp1));
    for (int i=0; i<NumSmokers; i++)
      printf("  %s: minimum %.2lf%%, average %.2lf%%, maximum %.2lf%%\n", smokers[i]->name(), min_entropy[i]*100, avg_entropy[i]/origsize*100, max_entropy[i]*100);
  }

  return EXIT_SUCCESS;
}
