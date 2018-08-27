/*find the outlier range of a given file for every row 
 implemented the tukey's range test, the range is based as the 
 interquartile range*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAXLINE 1024

static void find_median(uint32_t s, uint32_t *l, uint32_t *h) {

    /*assume the data stored from index 1, return the index of median*/
    /*odd size, median is the middle value, even size, median is the 
     two middle value / 2*/
    if (s % 2) 
      *l = *h = s / 2 + 1;
    else {
        *l = s / 2; 
        *h = s / 2 + 1; 
    }
}

/*the data file has to be sorted*/
int
main(int argc, char *argv[]) {
    int r, c;
    int min, max; 
    int cmin, cmax;
    uint32_t *q1_l, *q1_h, *q3_l, *q3_h;
    double *low, *high, *q1, *q3; 
    uint32_t *counts;
    FILE *outlier_log, *data_file, *output_file;
    char buf[MAXLINE];
    int i;
    if(argc < 6) {
        fprintf(stderr, "Usage: %s <input min> <input max> "
                "<outlier logfile> <data file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    min= atoi(argv[1]);
    max= atoi(argv[2]);

    outlier_log= fopen(argv[3], "w");
    data_file= fopen(argv[4], "r");
    output_file = fopen(argv[5], "w");
    
    cmin= INT_MAX;
    cmax= INT_MIN;

    fprintf(stderr, " %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
 
    if(!outlier_log) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if(!data_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    if(!output_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    
    counts= calloc(max - min + 1, sizeof(uint32_t));
    if(!counts) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    q1 = calloc(max - min + 1, sizeof(double));
    if(!q1) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    q3= calloc(max - min + 1, sizeof(double));
    if(!q3) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }   

    low = calloc(max - min + 1, sizeof(double));
    if(!low) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    high = calloc(max - min + 1, sizeof(double));
    if(!high) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    q1_l= calloc(max - min + 1, sizeof(size_t));
    if(!q1_l) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    q1_h= calloc(max - min + 1, sizeof(size_t));
    if(!q1_h) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    q3_l= calloc(max - min + 1, sizeof(size_t));
    if(!q3_l) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }   

    q3_h= calloc(max - min + 1, sizeof(size_t));
    if(!q3_h) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    
    while(fgets(buf, MAXLINE, data_file)) {
        int n;

        n= sscanf(buf, "%d %d\n", &c, &r);
        if(n != 2) {
            continue;
        }

        if(c < min || max < c) {
            continue;
        }
        if(c < cmin) cmin= c;
        if(cmax < c) cmax= c;


        counts[c - min]++;
    }

    /*for each c, get the Q1 and Q3 index*/
    for (i = cmin ; i <= cmax; i++) {
        if (!counts[i - min])
            continue; 


        if (counts[i - min] % 2) {
            /*not include the median in either half*/   
            find_median( (counts[i - min] - 1) / 2,  &q1_l[i - min], &q1_h[i - min]); 

        } else {
            /*split the median in both half*/
            find_median( counts[i - min] / 2,  &q1_l[i - min], &q1_h[i - min]); 
        }
        /*the q1 and q 3 index, starting from 1*/
        /*q3 is the q1 + median (upper median)*/
        q3_l[i - min] = q1_l[i - min] + counts[i - min] / 2 + 1; 
        q3_h[i - min] = q1_h[i - min] + counts[i - min] / 2 + 1; 
    }

    fseek(data_file, 0, SEEK_SET);
    memset (counts, 0, sizeof (uint32_t ) * (max - min + 1));

    while(fgets(buf, MAXLINE, data_file)) {
        int n;

        n= sscanf(buf, "%d %d\n", &c, &r);
        if(n != 2) {
            continue;
        }

        if(c < min || max < c) {
            continue;
        }

        counts[c - min]++;
        
        /*get the q1 and q3 value*/
        if ((counts[c - min] == q1_l[c - min]) 
                || (counts[c - min] == q1_h[c - min])) 
            q1[c - min] += r; 
        if ((counts[c - min] == q3_l[c - min]) 
                || (counts[c - min] == q3_h[c - min])) 
            q3[c - min] += r; 
           
    }

    /*cacluate the range */
    for (i = cmin ; i <= cmax; i++) {
        if (!counts[i - min])
            continue;

        if (q1_l[i - min] != q1_h[i - min])
            q1[i - min] /= 2; 
        if (q3_l[i - min] != q3_h[i - min])
            q3[i - min] /= 2; 

        low[i - min] = q1[i - min] - 1.5f * (q3[i - min] - q1[i - min]); 
        high[i - min] = q3[i - min] + 1.5f * (q3[i - min] - q1[i - min]); 
        
        fprintf(outlier_log, "row %d total %u \n"
                "q1 low index %u high index %u\n"
                "q3 low index %u high index %u\n", 
                i, counts[i - min], 
                q1_l[i - min], q1_h[i - min], 
                q3_l[i - min], q3_h[i - min]); 
 
  
        fprintf(outlier_log, "row %d \n"
                "q1 %.2f q3 %.2f \n"
                "range low %.2f high %.2f\n", 
                i, q1[i - min], q3[i - min], low[i - min], high[i - min]);

    }
    
    fseek(data_file, 0, SEEK_SET);
    while(fgets(buf, MAXLINE, data_file)) {
        int n;

        n= sscanf(buf, "%d %d\n", &c, &r);
        if(n != 2) {
            continue;
        }

        if(c < min || max < c) {
            continue;
        }
        /*smaller than the low, bigger than the high*/    
        if ((r < low[c - min]) || (r > high[c - min])) {
            fprintf(outlier_log, "%s", buf);
            continue;
        }
        
        /*record*/
        fprintf(output_file, "%s", buf); 
    }
    free(counts);
    free(q1_l); free(q1_h); free(q3_l); free(q3_h); free(low); free(high);
    fclose(outlier_log); 
    fclose(data_file);
    fclose(output_file);
    return 0; 
}
