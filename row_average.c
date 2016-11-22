/* row_average.c

   Average each row of the channel matrix, and print it.

   This code is experimental, and error-handling is primitive.
*/

/* Copyright 2013, NICTA.  See COPYRIGHT for license details. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "sparse.h"

int
main(int argc, char *argv[]) {
    csc_mat_t *Q;
    FILE *in;
    int quiet;
    csc_errno_t e;
    double *row_avg;
    double *row_std;
    double *row_ci; 
    int c, r;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <channel_matrix> [-q]\n", argv[0]);
        return 1;
    }

    if(argc > 2 && !strcmp(argv[2], "-q")) quiet= 1;

    if(!quiet) printf("Loading channel matrix...");
    fflush(stdout);
    in= fopen(argv[1], "rb");
    if(!in) { perror("fopen"); return 1; }

    Q= csc_load_binary(in, &e);
    if(!Q) { csc_perror(e, "csc_load_binary"); return 1; }

    fclose(in);
    if(!quiet) printf(" done.\n");

    row_avg= calloc(Q->nrow, sizeof(double));
    if(!row_avg) {
        perror("calloc");
        return 1;
    }

    row_std= calloc(Q->nrow, sizeof(double));
    if(!row_std) {
        free(row_avg); 
        perror("calloc");
        return 1;
    }
    row_ci= calloc(Q->nrow, sizeof(double));
    if(!row_ci) {
        free(row_avg); 
        free(row_std); 
        perror("calloc");
        return 1;
    }
    
    /*CSC format, store the sparse matrix, easier for accessing columns*/
    for(c= 0; c < Q->ncol; c++) {
        int64_t i;
        /*for number of entries in a column*/
        for(i= Q->ci[c]; i < Q->ci[c+1]; i++) {
            /*get the row number, and the entry*/
            r= Q->rows[i];
            double p= Q->entries[i];

            assert(0 <= r);
            assert(r < Q->nrow);

            /*the entry value is sacled by the total number of entries in this row
             , the percentage of the colum value in this row 
             then p * c is the contribution to the avg.*/
            row_avg[r]+= p * c;
        }
    }

    /*calcualting the std dev */
    for(c= 0; c < Q->ncol; c++) {
        int64_t i;
        /*for number of entries in a column*/
        for(i= Q->ci[c]; i < Q->ci[c+1]; i++) {
            /*get the row number, and the entry*/
            r= Q->rows[i];
            double p= Q->entries[i];

            assert(0 <= r);
            assert(r < Q->nrow);

            /*the entry value is sacled by the total number of entries in this row
             , the percentage of the colum value in this row 
             then p * c is the contribution to the avg.*/
            double sq = c - row_avg[r]; 
            sq *= sq; 
            row_std[r] += p * sq;
        }
    }
    for(r= 0; r < Q->nrow; r++)
        row_std[r] = sqrt(row_std[r]);


    /*calculating the 0.95 confidence interval
     z * std / root(n)*/ 
    for (r = 0; r < Q->nrow; r++) {
        double ci_z = 1.96f;
        row_ci[r] = ci_z * row_std[r] / sqrt(Q->row_size[r]);

    }
    
    /*using confidence interval to make the difference clear*/
    for(r= 0; r < Q->nrow; r++)
        printf("%d %.12e %.12e\n", r, row_avg[r], row_ci[r]);

    free(row_avg);
    free(row_std);
    free(row_ci);
    csc_mat_destroy(Q);

    return 0;
}
