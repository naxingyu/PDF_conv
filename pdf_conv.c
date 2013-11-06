/**************************************************/
/* PDF_conv                                       */
/* Convert .pdf file from EMIME to hts_engine     */
/**************************************************/
/* Author : Xingyu Na                             */
/* Date   : November 2013                         */
/**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NSTATE 5
#define MSD 0
#define STREAM 3

/* Usage: output usage */
void Usage(void)
{
   fprintf(stderr, "\n");
   fprintf(stderr, "PDF_conv - Convert .pdf file from EMIME to hts_engine.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "  usage:\n");
   fprintf(stderr, "       PDF_conv [ options ] \n");
   fprintf(stderr, "  options:                                                                   [  def]\n");
   fprintf(stderr, "    -m pdf         : model file                                              [  N/A]\n");
   fprintf(stderr, "    -n int         : number of states                                        [   %d]\n", NSTATE);
   fprintf(stderr, "    -i int         : MSD model index                                         [   %d]\n", MSD);
   fprintf(stderr, "    -s int         : number of streams                                       [   %d]\n", STREAM);
   fprintf(stderr, "    -o pdf         : output model file                                       [  N/A]\n");
   fprintf(stderr, "\n");

   exit(0);
}

static int PDF_byte_swap(void *p, const int size, const int block)
{
   char *q, tmp;
   int i, j;

   q = (char *) p;

   for (i = 0; i < block; i++) {
      for (j = 0; j < (size / 2); j++) {
         tmp = *(q + j);
         *(q + j) = *(q + (size - 1 - j));
         *(q + (size - 1 - j)) = tmp;
      }
      q += size;
   }

   return i;
}

int PDF_write(void *p, const int size, const int num, FILE * fp)
{
   const int block = PDF_byte_swap(p, size, num);

   fwrite(p, size, num, fp);

   PDF_byte_swap(p, size, block);
   return block;
}

void main (int argc, char** argv)
{
	FILE *fpdf = NULL, *fout = NULL;
	int *npdfs, nstate = NSTATE, nvec, iMSD = MSD, nstream = STREAM;
	char *fnpdf = NULL, *fnout = NULL;
	float pdf;
	int i, j, k;

	/* parse command line */
   if (argc == 1)
      Usage();

	while (--argc) {
		if (**++argv == '-') {
			switch (*(*argv + 1)) {
			case 'm':
				fnpdf = *++argv;
				--argc;
				break;
			case 'n':
				nstate = atoi(*++argv);
				--argc;
				break;
			case 'i':
				iMSD = atoi(*++argv);
				--argc;
				break;
			case 's':
				nstream = atoi(*++argv);
				--argc;
				break;
			case 'o':
				fnout = *++argv;
				--argc;
				break;
			default:
				printf("PDF_conv: Invalid option '-%c'.\n", *(*argv + 1));
				exit(0);
			}
		}
	}
	if((fpdf = fopen(fnpdf, "rb")) == NULL) {
		printf("PDF_conv: Cannot open %s.\n", fnpdf);
		exit(-1);
	}

	if(fnout == NULL || fnout == fnpdf) {
		printf("PDF_conv: Please identify the output model file with a different name with %s.\n", fnpdf);
		exit(-1);
	}
	if((fout = fopen(fnout, "wb")) == NULL) {
		printf("PDF_out: Creating %s failed.\n", fnout);
		exit(-1);
	}

	/* load MSD flag */
	PDF_write(&iMSD, sizeof(int), 1, fout);

	/* load stream size */
	PDF_write(&nstream, sizeof(int), 1, fout);

	/* load vector size */
	fread(&nvec, sizeof(int), 1, fpdf);
	PDF_write(&nvec, sizeof(int), 1, fout);

	/* load number of pdfs */
	npdfs = (int *)calloc(nstate, sizeof(int));
	for(i = 0; i < nstate; i++) {
		fread(&npdfs[i], sizeof(int), 1, fpdf);
		PDF_write(&npdfs[i], sizeof(int), 1, fout);
	}

	/* load pdfs */
	for(i = 0; i < nstate; i++) {
		for(j = 0; j < npdfs[i]; j++) {
			for(k = 0; k < nvec; k++) {
				fread(&pdf, sizeof(float), 1, fpdf);
				PDF_write(&pdf, sizeof(float), 1, fout);
				fread(&pdf, sizeof(float), 1, fpdf);
				PDF_write(&pdf, sizeof(float), 1, fout);
				if(iMSD == 1) {
					fread(&pdf, sizeof(float), 1, fpdf);
					PDF_write(&pdf, sizeof(float), 1, fout);
					fread(&pdf, sizeof(float), 1, fpdf);
					PDF_write(&pdf, sizeof(float), 1, fout);
				}
			}
		}
	}
	fclose(fpdf);
	fclose(fout);
	free(npdfs);
}