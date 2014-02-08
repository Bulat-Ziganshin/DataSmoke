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

class Distribution
{
public:
//  Distribution();
//  ~Distribution();
};

/**********************************************************************/
/* Byte distribution                                                  */
/**********************************************************************/

class ByteDistribution : public Distribution
{
public:
//  ByteDistribution();
  void smoke (void *buf, size_t bufsize, double *entropy);
//  ~ByteDistribution();
};

void ByteDistribution::smoke (void *buf, size_t bufsize, double *entropy)
{
  size_t count[256] = {0};

  byte *p = (byte*) buf;
  for (int i=0; i<bufsize; i++)
    count[p[i]]++;

  // Calculate compression ratio with the order-0 model
  double order0 = 0;
  for (int i=0; i<256; i++)
    if (count[i])
      order0 += count[i] * log(double(bufsize/count[i]))/log(double(2)) / 8;

  *entropy = order0 / bufsize;
}

/**********************************************************************/
/* DWord distribution                                                 */
/**********************************************************************/

uint32_t hash_function (uint32_t x)
{
  uint64_t hash  =  x * uint64_t(123456791u);
  return uint32_t(hash>>32) ^ uint32_t(hash);
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

  ByteDistribution ByteD;
  double entropy;
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
    //printf("%.2lf%%\n", entropy*100);

    origsize += buf_bytes;
  }

  char temp1[100], temp2[100];
  fprintf(stderr, "Processed %s bytes\n", show3(origsize,temp1));
  return EXIT_SUCCESS;
}
