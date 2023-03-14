/* extract_plot.c

   Generate a plot of the channel matrix, suitable for gnuplot 'with image'.

   This code is experimental, and error-handling is primitive.
*/

/* Copyright 2013, NICTA.  See COPYRIGHT for license details. */

#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sparse.h"

int
main(int argc, char *argv[]) {
    FILE *in;
    csc_mat_t *M;
    csc_errno_t e;
    uint64_t cmin= ULONG_MAX, cmax= 0, ccount;
    uint64_t rmin= ULONG_MAX, rmax= 0, rcount;
    uint64_t nrow, ncol, rowbin, colskip;
    uint64_t xmin= ULONG_MAX, xmax= 0, ymin= ULONG_MAX, ymax= 0;
    int c, x, y, r;
    float *plot;

    if(argc < 4) {
        fprintf(stderr, "Usage: %s <matrix_filename> <nrow> <ncol>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    in= fopen(argv[1], "rb");
    if(!in) { perror("fopen"); exit(EXIT_FAILURE); }

    nrow= atoi(argv[2]);
    if(0 >= nrow) {
        fprintf(stderr, "nrow <= 0\n");
        exit(EXIT_FAILURE);
    }

    ncol= atoi(argv[3]);
    if(0 >= ncol) {
        fprintf(stderr, "ncol <= 0\n");
        exit(EXIT_FAILURE);
    }

    M= csc_load_binary(in, &e);
    if(!M) { csc_perror(e, "csc_load_binary"); exit(EXIT_FAILURE); }
    fclose(in);

    if(!csc_check(M, 1)) abort();
    // printf("# ");
    // csc_stats(M);
    printf("X Y value\n");

    if(nrow > M->nrow) nrow= M->nrow;
    
    if(ncol > M->ncol) ncol= M->ncol;

    if(STRIDE_OF(M) != 1) {
        fprintf(stderr, "Too lazy to handle strided matrices\n");
        exit(EXIT_FAILURE);
    }

    for(c= 0; c < M->ncol; c++) {
        if(M->ci[c] < M->ci[c+1]) {
            if(c < cmin) cmin= c;
            if(c > cmax) cmax= c;
        }
    }
    
    for(c= 0; c < M->ncol; c++) {

        int64_t i;

        /*for number of entries in a column*/
        for(i= M->ci[c]; i < M->ci[c+1]; i++) {
            /*get the row number*/
             r= M->rows[i];
     
            if(r < rmin) rmin= r;
            if(r > rmax) rmax= r;
        }
    }   

    /*define the number of column and rows*/
    ccount= cmax - cmin + 1;
    if(ncol > ccount) ncol= ccount;

    rcount = rmax - rmin + 1; 
    if (nrow > rcount) nrow = rcount;

    plot= calloc(nrow * ncol, sizeof(float));

    for(c= 0; c < M->ncol; c++) {
        int64_t i;

        for(i= M->ci[c]; i < M->ci[c+1]; i++) {
            if(M->entries[i] > 0) {
                uint64_t r= M->rows[i];
                /* x cannot be bigger than nrow, y cannot be bigger than ncol*/
               /*as the x range is between rmin to rmax, so (r - rmin) < rcount
                 then (r - min) * nrow / rcount is less than nrow  */ 
                uint64_t x= ((r - rmin) * nrow) / rcount; 
                uint64_t y= ((c-cmin) * ncol) / ccount;

                /*adding all the propability within a range (rowbin) into 
                 one plot data point*/
                plot[y * nrow + x]+= M->entries[i];

                if(x < xmin) xmin= x;
                if(x > xmax) xmax= x;
                if(y < ymin) ymin= y;
                if(y > ymax) ymax= y;
            }
        }
    }
    
    rowbin= rcount / nrow;
    colskip= ccount / ncol;

    /*spanning multiple columns into one, as the result of scaling a large number
     of data columns into a few columns */
    for(x= xmin; x <= xmax; x++) {

        /*the middle row value of a range of coloums (rowbin)*/
        uint64_t r= ((x * rowbin) + ((x+1) * rowbin -1))/2 + rmin;

        /*each of the y step skips colskip number of columns*/
        for(y= ymin; y <= ymax; y++) {
            uint64_t c= y * colskip + cmin;

            /*averge the accumulated value for the range (rowbin)*/
            printf("%"PRIu64" %"PRIu64" %.12e\n", r, c,
                    plot[y * nrow + x] / rowbin);
        }
    }

    free(plot);
    csc_mat_destroy(M);

    return 0;
}
