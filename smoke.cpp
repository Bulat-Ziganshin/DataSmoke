char version[] = "DataSmoker 0.3  2014-02-11";

char copyright[] = "Developed by Bulat Ziganshin\n"
                   "The code is placed in public domain\n";


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define kb 1024
#define mb (1024*kb)
#define gb (1024*mb)
typedef unsigned char byte;

class Entropy
{
public:
  virtual const char* name() = 0;
  virtual ~Entropy() {}
  virtual void smoke (void *buf, size_t bufsize, double *entropy) = 0;
};

uint32_t hash_function (uint32_t x)
{
  uint64_t hash  =  x * uint64_t(123456791u);
  return uint32_t(hash>>32) ^ uint32_t(hash);
}


// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC hashing **************************************************************************************************************************************
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#if GCC_VERSION >= 403

// Requires GCC4.3 and SSE4.2-enabled CPU; and of course compatible only with Crc32CastagnoliPolynom
#include <x86intrin.h>
#include <cpuid.h>
uint32_t a_mm_crc32_u32(uint32_t crc, uint32_t value) {
  asm("crc32l %[value], %[crc]\n" : [crc] "+r" (crc) : [value] "rm" (value));
  return crc;
}
#define hash_function(c)  (a_mm_crc32_u32(0xFFFFFFFF,(c)))

bool crc32c()  /* Check CPU for CRC32c asm instruction support (part of SSE4.2) */
{
  uint32_t eax, ebx, ecx, edx;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  return (ecx & bit_SSE4_2) != 0;
}

#endif

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


/*********************************************************************************************************/
/* DWord hash entropy: calculate compression ratio with the 16-bit hashes of 32-bit values order-0 model */
/*********************************************************************************************************/

class DWordHashEntropy : public Entropy
{
  static const size_t HASHSIZE = 256*256;
  uint32_t *count;
public:
  DWordHashEntropy()            {count = new uint32_t[HASHSIZE];}
  virtual const char* name()    {return "DWord hash entropy";}
  virtual ~DWordHashEntropy()   {delete[] count;}
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

void DWordHashEntropy::smoke (void *buf, size_t bufsize, double *entropy)
{
  const size_t STEP = 1;        // Check only one of every STEP positions
  byte *p = (byte*) buf;

  memset(count,0,HASHSIZE*sizeof(*count));
  for (size_t i=0;  i<=bufsize-sizeof(uint32_t);  i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    count[(hash+hash/HASHSIZE)%HASHSIZE]++;
  }

  double order0 = 0;
  for (int i=0; i<HASHSIZE; i++)
  {
    if (count[i])
      order0 += count[i] * log(double(bufsize)/count[i])/log(double(2)) / 16;
  }

  *entropy  =  order0 / bufsize;
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

void DWordCoverage::smoke (void *buf, size_t bufsize, double *entropy)
{
  const size_t   STEP = 1;        // Check only every n'th position
  const uint32_t FILTER = 16;     // Of those checked, count only every n'th hash
  const uint32_t FILTER_MAX_HASH  =  uint32_t(-1) / FILTER;   // Count only hashes smaller or equal to this value
  const uint32_t FILTER_HASH_DIVIDER  =  (FILTER_MAX_HASH / CHAR_BIT) + 1;  // Dividing filtered hashes by this value will leave only 3 highest bits required to address bit in the byte

  memset(table,0,HASHSIZE);
  byte *p = (byte*) buf;
  size_t total_hashes = 0;

  for (size_t i=0;  i<=bufsize-sizeof(uint32_t);  i+=STEP)
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
/* 2-pass DWord coverage: select the most populated sector and then        */
/*   calculate which part of 32-bit dwords in the sector are unique        */
/***************************************************************************/

class TwoPassDWordCoverage : public Entropy
{
  static const size_t HASHSIZE = 2*mb;  // it should be small enough to fit in most CPU last-level caches
  byte *table;
  size_t bits[256];
public:
  TwoPassDWordCoverage();
  virtual const char* name()        {return "2-pass DWord coverage";};
  virtual ~TwoPassDWordCoverage()   {delete[] table;}
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

TwoPassDWordCoverage::TwoPassDWordCoverage()
{
  table = new byte[HASHSIZE];
  bits[0] = 0;
  for (int i=1; i<256; i++)
    bits[i]  =  bits[i/2] + (i%2);
}

void TwoPassDWordCoverage::smoke (void *buf, size_t bufsize, double *entropy)
{
  const size_t STEP = 1;        // Check only one of every STEP positions
  const size_t MAX_SECTORS = 8192;
  uint32_t sector_cnt[MAX_SECTORS] = {0};
  byte *p = (byte*) buf;

  uint32_t sectors = bufsize/(256*kb),  sectors_log = 0;
  if (sectors > MAX_SECTORS)  sectors = MAX_SECTORS;
  while (sectors>1) sectors/=2, sectors_log++;
  sectors = 1<<sectors_log;
  uint32_t sectors_mask = sectors-1;

  // 1st pass: count hashes in each sector
  for (size_t i=0;  i<=bufsize-sizeof(uint32_t);  i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    sector_cnt[hash & sectors_mask]++;
  }

  // Find the most populated sector
  uint32_t sector=0, total_hashes=0;
  for (uint32_t i=0; i<sectors; i++)
    if (sector_cnt[i] > total_hashes)
      sector=i,  total_hashes=sector_cnt[i];

  // 2nd pass: compute the sector's coverage
  memset(table,0,HASHSIZE);
  for (size_t i=0;  i<=bufsize-sizeof(uint32_t);  i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    if ((hash&sectors_mask) == sector)
      table[(hash>>sectors_log) % HASHSIZE]  |=  1 << (hash>>29);
  }

  size_t unique_hashes = 0;
  for (size_t i=0; i<HASHSIZE; i++)
    unique_hashes += bits[table[i]];

  // Coverage is ratio of unique hashes to the total amount of hashes checked
  *entropy  =  double(unique_hashes) / total_hashes;
  //printf("\n%d sectors, %d / %d = %.2lf%%", int(sectors), int(unique_hashes), int(total_hashes), *entropy*100);
}


/**********************************************************************/
/* Supplementary code                                                 */
/**********************************************************************/

// Similar to parseInt, but the string param may have a suffix b/k/m/g/^, representing units of memory, or in the case of '^', the power of 2
size_t parseMem (char *param, int *error, char spec)
{
  size_t n=0;  *error=0;
  char c = *param=='='? *++param : *param;
  if (! (c>='0' && c<='9'))  {*error=1; return 0;}
  while (c>='0' && c<='9')   n=n*10+c-'0', c=*++param;
  switch (c? c : spec)
  {
    case 'b':  return n;
    case 'k':  return n*kb;
    case 'm':  return n*mb;
    case 'g':  return n*gb;
    case '^':  return size_t(1)<<n;
  }
  *error=1; return 0;
}

// Returns a string with the amount of memory
char *showMem (size_t mem, char *result)
{
       if (mem%gb==0) sprintf (result, "%.0lfgb", double(mem/gb));
  else if (mem%mb==0) sprintf (result, "%.0lfmb", double(mem/mb));
  else if (mem%kb==0) sprintf (result, "%.0lfkb", double(mem/kb));
  else                sprintf (result, "%.0lfb",  double(mem));
  return result;
}

int main (int argc, char **argv)
{
  int bufsize = 4*mb;  char temp1[100];
  if (argc==1)
  {
    printf("%s", version);
    printf("\n\nUsage: smoke [-bBUFSIZE] infiles...\nBUFSIZE examples: 64k, 16m/16, 1g; default=%s\n\n%s", showMem(bufsize,temp1), copyright);
    return EXIT_FAILURE;
  }

  if (memcmp(argv[1],"-b",2)==0)
  {
    int error;
    bufsize  =  parseMem (argv[1]+2, &error, 'm');
    if (error)  {printf("Bad option %s!\n", argv[1]); return EXIT_FAILURE;}
    argv++, argc--;
  }
  char *buf = new char[bufsize];

  for (int file=1; file < argc; file++)
  {

    FILE *infile  = fopen (argv[file], "rb");  if (infile==NULL)  {printf("Can't open input file %s!\n", argv[file]); return EXIT_FAILURE;}
    int width = strlen(argv[file]);  width = width>21? width : 21;
    printf("%s%*s | min %% | avg %% | max %% | incompressible %s blocks", file>1?"\n":"", width, argv[file], showMem(bufsize,temp1));

    ByteEntropy ByteS;
    WordEntropy WordS;
    Order1Entropy Order1S;
    DWordHashEntropy DWordHashS;
    DWordCoverage DWordS;
    TwoPassDWordCoverage TwoPassDWordS;
    Entropy *smokers[] = {&ByteS, &WordS, &Order1S, &DWordHashS, &DWordS, &TwoPassDWordS};
    const int NumSmokers = sizeof(smokers)/sizeof(*smokers);
    double entropy, min_entropy[NumSmokers], avg_entropy[NumSmokers], max_entropy[NumSmokers];
    int incompressible[NumSmokers];  char incompressible_list[NumSmokers][1000];

    uint64_t origsize = 0;  int blocks = 0;
    for(;;)
    {
      int bytes_read = fread(buf, 1, bufsize, infile);
      if (bytes_read==0) break;

      for (int i=0; i<NumSmokers; i++)
      {
        smokers[i]->smoke(buf, bytes_read, &entropy);
        if (origsize==0) {                 // first block
          min_entropy[i] = max_entropy[i] = entropy;
          avg_entropy[i] = incompressible[i] = 0;
          *incompressible_list[i] = '\0';
        } else if (bytes_read==bufsize) {  // update min/max entropy only on complete blocks
          if (entropy < min_entropy[i])
            min_entropy[i] = entropy;
          if (entropy > max_entropy[i])
            max_entropy[i] = entropy;
        }
        avg_entropy[i] += entropy*bytes_read;

        if (entropy > 0.95)
        {
          sprintf (strchr(incompressible_list[i],'\0'),
                   incompressible[i]== 0? ":  %d" :
                   incompressible[i] <10? ", %d"  :
                   incompressible[i]==10? "..."   :
                                          ""      , blocks);
          ++incompressible[i];
        }
      }

      origsize += bytes_read;  blocks++;
    }
    fclose(infile);

    printf("\n");  for(int i=0; i<width; i++) printf("-");  printf("-|------:|------:|------:|----------------------------\n");
    for (int i=0; i<NumSmokers; i++)
      printf("%*s |%6.2lf |%6.2lf |%6.2lf | %d of %d%s\n", width, smokers[i]->name(), min_entropy[i]*100, avg_entropy[i]/origsize*100, max_entropy[i]*100, incompressible[i], blocks, (incompressible[i]<blocks? incompressible_list[i] : ""));
  }

  return EXIT_SUCCESS;
}
