char copyright[] = "Developed by Bulat Ziganshin\n"
                   "The code is placed in public domain\n";


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define kb 1024
#define mb (kb*kb)
typedef unsigned char byte;

class Smoker
{
public:
  virtual void smoke (void *buf, size_t bufsize, double *entropy) = 0;
  virtual ~Smoker() {}
};

/**********************************************************************/
/* Byte smoker: calculate compression ratio with the order-0 model    */
/**********************************************************************/

class ByteSmoker : public Smoker
{
public:
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
};

void ByteSmoker::smoke (void *buf, size_t bufsize, double *entropy)
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
      order0 += count * log(double(bufsize/count))/log(double(2)) / 8;
  }

  *entropy  =  order0 / bufsize;
}


/**********************************************************************/
/* DWord smoker                                                       */
/**********************************************************************/

uint32_t hash_function (uint32_t x)
{
  uint64_t hash  =  x * uint64_t(123456791u);
  return uint32_t(hash>>32) ^ uint32_t(hash);
}

const size_t HASHSIZE = 128*kb;

class DWordSmoker : public Smoker
{
  size_t bits[256];
  byte table[HASHSIZE];
public:
  DWordSmoker();
  virtual void smoke (void *buf, size_t bufsize, double *entropy);
  virtual ~DWordSmoker() {}
};

DWordSmoker::DWordSmoker()
{
  bits[0] = 0;
  for (int i=0; i<256; i++)
    bits[i]  =  bits[i/2] + (i%2);
}

void DWordSmoker::smoke (void *buf, size_t bufsize, double *entropy)
{
  const size_t   STEP = 4;         // Check every n'th position
  const uint32_t FILTER = 16;      // Of checked, count every n'th hash
  const uint32_t MAX_HASH = (1u<<31) / (FILTER/2);

  memset(table,0,HASHSIZE);
  byte *p = (byte*) buf;
  for (size_t i=0; i<bufsize-STEP+1; i+=STEP)
  {
    uint32_t hash = hash_function(*(uint32_t*)(p+i));
    if (hash < MAX_HASH)
      table[hash % HASHSIZE]  |=  1 << (hash>>25);
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
  fprintf(stderr, "DataSmoker 0.1. ");

  int repetitions = argc>2? atoi(argv[1]) : 1;  // Repeat detection many times in order to get precise times
  if (argc==1)
  {
    fprintf(stderr, "\n\nUsage: smoke [N] infile\n\nN is a number of repetitions for precise timing\n\n%s", copyright);
    return EXIT_FAILURE;
  }

  FILE *infile  = fopen (argv[argc>2?2:1], "rb");  if (infile==NULL)  {fprintf (stderr, "Can't open input file %s!\n",    argv[argc>2?2:1]); return EXIT_FAILURE;}

  ByteSmoker ByteD;
  double entropy,  min_entropy = 1,  avg_entropy = 0;
  uint64_t origsize = 0;

  for(;;)
  {
    int buf_bytes;
    const int BUFSIZE = 4*mb;
    static char buf[BUFSIZE];

    buf_bytes = fread(buf, 1, BUFSIZE, infile);
    if (buf_bytes==0) break;

    for (int i=0; i<repetitions; i++)
      ByteD.smoke(buf, buf_bytes, &entropy);

    if (entropy < min_entropy)
      min_entropy = entropy;
    avg_entropy += entropy*buf_bytes;

    origsize += buf_bytes;
  }

  char temp1[100], temp2[100];
  fprintf(stderr, "Processed %s bytes\n", show3(origsize,temp1));
  fprintf(stderr, "ByteSmoker entropy: minimum %.2lf%%, average %.2lf%%\n", min_entropy*100, avg_entropy/origsize*100);
  return EXIT_SUCCESS;
}
