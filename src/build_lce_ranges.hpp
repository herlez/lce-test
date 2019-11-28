#include <sais-lite-lcp/sais.h>
#include <fstream>
#include <iostream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

void build_lce_range(std::string file, std::string output_folder, long prefix_length=0) {
    
    long text_length;
    
    FILE *fp;
    unsigned char *T;
    int *SA;
    int *LCP;
    
    std::string output_folder_name;
    if (prefix_length == 0) {
        output_folder_name = output_folder;
    } else {
        output_folder_name = output_folder + std::to_string(prefix_length);
    }
    std::cout << output_folder_name << std::endl;   
    if(mkdir(output_folder_name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
        std::cout << "Folder for sorted lce queries could not get created" << std::endl;}

    
    
    /* Open a file for reading. */
    if((fp = fopen(file.c_str(), "rb")) == NULL) {
        fprintf(stderr, "Cannot open file");
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    
    /* Get the file size. */
    if(fseek(fp, 0, SEEK_END) == 0) {
        text_length = ftell(fp);
        rewind(fp);
        if(text_length < 0) {
            fprintf(stderr, "Cannot ftell: ");
            perror(NULL);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Cannot fseek: ");
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    if(prefix_length != 0) {
        text_length = prefix_length;
    }
    
    
    /* Allocate 9n bytes of memory. */
    T = (unsigned char *)malloc((size_t)text_length * sizeof(unsigned char));
    SA = (int *)malloc((size_t)(text_length+1) * sizeof(int)); // +1 for computing LCP
    LCP = (int *)malloc((size_t)text_length * sizeof(int));
    if((T == NULL) || (SA == NULL) || (LCP == NULL)) {
        fprintf(stderr, "Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Read n bytes of data. */
    if(fread(T, sizeof(unsigned char), (size_t)text_length, fp) != (size_t)text_length) {
        fprintf(stderr, "Cannot read text");
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    
    /* Construct the suffix array. */
    fprintf(stderr, "%ld bytes ... \n", text_length);
    if(sais(T, SA, LCP, (int)text_length) != 0) {
        fprintf(stderr, "Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }
    
    
    /* Extract LCE */
  FILE *lce0 = fopen("./english/i0", "w");
  FILE *lce1 = fopen("./english/i1", "w");
  FILE *lce2 = fopen("./english/i2", "w");
  FILE *lce3 = fopen("./english/i3", "w");
  FILE *lce4 = fopen("./english/i4", "w");
  FILE *lce5 = fopen("./english/i5", "w");
  FILE *lce6 = fopen("./english/i6", "w");
  FILE *lce7 = fopen("./english/i7", "w");
  FILE *lce8 = fopen("./english/i8", "w");
  FILE *lce9 = fopen("./english/i9", "w");
  FILE *lce10 = fopen("./english/i10", "w");
  FILE *lce11 = fopen("./english/i11", "w");
  FILE *lce12 = fopen("./english/i12", "w");
  FILE *lce13 = fopen("./english/i13", "w");
  FILE *lce14 = fopen("./english/i14", "w");
  FILE *lce15 = fopen("./english/i15", "w");
  FILE *lce16 = fopen("./english/i16", "w");
  FILE *lce17 = fopen("./english/i17", "w");
  FILE *lce18 = fopen("./english/i18", "w");
  FILE *lce19 = fopen("./english/i19", "w");
  FILE *lceX = fopen("./english/iH", "w");
    
    const int limita = 100001;
  int limit[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  unsigned int i;
  
  for (i = 1; i < text_length; i++) {
	  int lstart = 0;
	if((LCP[i] + SA[i] != (text_length-1)) & (LCP[i] + SA[i-1] != (text_length-1))) {
		if(LCP[i] < (1L << lstart++)) {
			limit[0]++;
			if(limit[0] < limita) {
				fprintf(lce0, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[1]++;
			if(limit[1] < limita) {
				fprintf(lce1, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[2]++;
			if(limit[2] < limita) {
				fprintf(lce2, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[3]++;
			if(limit[3] < limita) {
				fprintf(lce3, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[4]++;
			if(limit[4] < limita) {
				fprintf(lce4, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[5]++;
			if(limit[5] < limita) {
				fprintf(lce5, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[6]++;
			if(limit[6] < limita) {
				fprintf(lce6, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[7]++;
			if(limit[7] < limita) {
				fprintf(lce7, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[8]++;
			if(limit[8] < limita) {
				fprintf(lce8, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[9]++;
			if(limit[9] < limita) {
				fprintf(lce9, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[10]++;
			if(limit[10] < limita) {
				fprintf(lce10, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[11]++;
			if(limit[11] < limita) {
				fprintf(lce11, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[12]++;
			if(limit[12] < limita) {
				fprintf(lce12, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[13]++;
			if(limit[13] < limita) {
				fprintf(lce13, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[14]++;
			if(limit[14] < limita) {
				fprintf(lce14, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[15]++;
			if(limit[15] < limita) {
				fprintf(lce15, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[16]++;
			if(limit[16] < limita) {
				fprintf(lce16, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[17]++;
			if(limit[17] < limita) {
				fprintf(lce17, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[18]++;
			if(limit[18] < limita) {
				fprintf(lce18, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else if (LCP[i] < (1L << lstart++)) {
			limit[19]++;
			if(limit[19] < limita) {
				fprintf(lce19, "%d\n%d\n", SA[i-1],SA[i]);
			}
		} else {
			limit[17]++;
			if(limit[17] < limita) {
				fprintf(lceX, "%d\n%d\n", SA[i-1],SA[i]);
			}
		}
	}
  }


   
  /* Deallocate memory. */
  free(SA);
  free(LCP);
  free(T);
    /*
    ofstream lce1(output_path + string("/i0"), ios::out|ios::trunc);
    ofstream lce2(output_path + string("/i0"), ios::out|ios::trunc);
    ofstream lce3(output_path + string("/i0"), ios::out|ios::trunc);
    ofstream lce4(output_path + string("/i0"), ios::out|ios::trunc);
    ofstream lce5(output_path + string("/i0"), ios::out|ios::trunc);
    FILE *lce1 = fopen("./english/i1", "w");
    FILE *lce2 = fopen("./english/i2", "w");
    FILE *lce3 = fopen("./english/i3", "w");
    FILE *lce4 = fopen("./english/i4", "w");
    FILE *lce5 = fopen("./english/i5", "w");
    FILE *lce6 = fopen("./english/i6", "w");
    FILE *lce7 = fopen("./english/i7", "w");
    FILE *lce8 = fopen("./english/i8", "w");
    FILE *lce9 = fopen("./english/i9", "w");
    FILE *lce10 = fopen("./english/i10", "w");
    FILE *lce11 = fopen("./english/i11", "w");
    FILE *lce12 = fopen("./english/i12", "w");
    FILE *lce13 = fopen("./english/i13", "w");
    FILE *lce14 = fopen("./english/i14", "w");
    FILE *lce15 = fopen("./english/i15", "w");
    FILE *lce16 = fopen("./english/i16", "w");
    FILE *lce17 = fopen("./english/i17", "w");
    FILE *lce18 = fopen("./english/i18", "w");
    FILE *lce19 = fopen("./english/i19", "w");
    FILE *lceX = fopen("./english/iH", "w");
    */
}
