#include "sais.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>

#include <filesystem>

#pragma once

namespace fs = std::filesystem;

void build_lce_range(std::string const& file, std::string const& output_folder,
                     long const prefix_length=0) {
    
  long text_length;
    
  FILE *fp;
  unsigned char *T;
  int *SA;
  int *LCP;
    
  std::string output_folder_name;
  if (prefix_length == 0) {
    output_folder_name = output_folder;
  } else {
    output_folder_name = output_folder + "_" + std::to_string(prefix_length);
  }
  //std::cout << "output_folder_name.c_str() " << output_folder_name.c_str()
  //          << std::endl;
  if(!fs::create_directory(output_folder_name.c_str())) {
    //std::cout << "Folder for sorted lce queries could not get created. "
    //         << "If it was already created and you want to calculate new "
    //         << "values, delete the folder first." << std::endl;
    return;
  }

    
    
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

  std::cout << "text_length " << text_length << std::endl;
    
  /* Allocate 9n bytes of memory. */
  T = (unsigned char *)malloc((size_t)text_length * sizeof(unsigned char));
  SA = (int *)malloc((size_t)(text_length) * sizeof(int)); // +1 for computing LCP
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
  if(saislcp(T, SA, LCP, (int)text_length) != 0) {
    fprintf(stderr, "Cannot allocate memory.\n");
    exit(EXIT_FAILURE);
  }
    
    
  /* Extract LCE */
  std::ofstream of0 {output_folder_name + "/lce_0", std::ofstream::out};
  std::ofstream of1 {output_folder_name + "/lce_1", std::ofstream::out};
  std::ofstream of2 {output_folder_name + "/lce_2", std::ofstream::out};
  std::ofstream of3 {output_folder_name + "/lce_3", std::ofstream::out};
  std::ofstream of4 {output_folder_name + "/lce_4", std::ofstream::out};
  std::ofstream of5 {output_folder_name + "/lce_5", std::ofstream::out};
  std::ofstream of6 {output_folder_name + "/lce_6", std::ofstream::out};
  std::ofstream of7 {output_folder_name + "/lce_7", std::ofstream::out};
  std::ofstream of8 {output_folder_name + "/lce_8", std::ofstream::out};
  std::ofstream of9 {output_folder_name + "/lce_9", std::ofstream::out};
  std::ofstream of10 {output_folder_name + "/lce_10", std::ofstream::out};
  std::ofstream of11 {output_folder_name + "/lce_11", std::ofstream::out};
  std::ofstream of12 {output_folder_name + "/lce_12", std::ofstream::out};
  std::ofstream of13 {output_folder_name + "/lce_13", std::ofstream::out};
  std::ofstream of14 {output_folder_name + "/lce_14", std::ofstream::out};
  std::ofstream of15 {output_folder_name + "/lce_15", std::ofstream::out};
  std::ofstream of16 {output_folder_name + "/lce_16", std::ofstream::out};
  std::ofstream of17 {output_folder_name + "/lce_17", std::ofstream::out};
  std::ofstream of18 {output_folder_name + "/lce_18", std::ofstream::out};
  std::ofstream of19 {output_folder_name + "/lce_19", std::ofstream::out};
  std::ofstream ofX {output_folder_name + "/lce_X", std::ofstream::out};
	
  //FILE *lce0 = fopen( strcat(output_folder_name.c_str(), "/lce_0"), "w");
    
  const int limita = 100001;
  int limit[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  unsigned int i;
    
  for (i = 1; i < text_length; i++) {
    int lstart = 0;
    //if((LCP[i] + SA[i] != (text_length-1)) & (LCP[i] + SA[i-1] != (text_length-1))) { //??????
      if(LCP[i] < (1L << lstart++)) {
        limit[0]++;
        if(limit[0] < limita) {
          of0 << SA[i-1] << '\n'; of0 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[1]++;
        if(limit[1] < limita) {
          of1 << SA[i-1] << '\n'; of1 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[2]++;
        if(limit[2] < limita) {
          of2 << SA[i-1] << '\n'; of2 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[3]++;
        if(limit[3] < limita) {
          of3 << SA[i-1] << '\n'; of3 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[4]++;
        if(limit[4] < limita) {
          of4 << SA[i-1] << '\n'; of4 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[5]++;
        if(limit[5] < limita) {
          of5 << SA[i-1] << '\n'; of5 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[6]++;
        if(limit[6] < limita) {
          of6 << SA[i-1] << '\n'; of6 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[7]++;
        if(limit[7] < limita) {
          of7 << SA[i-1] << '\n'; of7 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[8]++;
        if(limit[8] < limita) {
          of8 << SA[i-1] << '\n'; of8 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[9]++;
        if(limit[9] < limita) {
          of9 << SA[i-1] << '\n'; of9 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[10]++;
        if(limit[10] < limita) {
          of10 << SA[i-1] << '\n'; of10 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[11]++;
        if(limit[11] < limita) {
          of11 << SA[i-1] << '\n'; of11 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[12]++;
        if(limit[12] < limita) {
          of12 << SA[i-1] << '\n'; of12 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[13]++;
        if(limit[13] < limita) {
          of13 << SA[i-1] << '\n'; of13 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[14]++;
        if(limit[14] < limita) {
          of14 << SA[i-1] << '\n'; of14 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[15]++;
        if(limit[15] < limita) {
          of15 << SA[i-1] << '\n'; of15 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[16]++;
        if(limit[16] < limita) {
          of16 << SA[i-1] << '\n'; of16 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[17]++;
        if(limit[17] < limita) {
          of17 << SA[i-1] << '\n'; of17 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[18]++;
        if(limit[18] < limita) {
          of18 << SA[i-1] << '\n'; of18 << SA[i] << '\n';
        }
      } else if (LCP[i] < (1L << lstart++)) {
        limit[19]++;
        if(limit[19] < limita) {
          of19 << SA[i-1] << '\n'; of19 << SA[i] << '\n';
        }
      } else {
        limit[20]++;
        if(limit[20] < limita) {
          ofX << SA[i-1] << '\n'; ofX << SA[i] << '\n';
        }
      }
    //}
  }
   
  /* Deallocate memory. */
  free(SA);
  free(LCP);
  free(T);
}
