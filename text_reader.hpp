#pragma once

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static inline
void readfile(const char* const filename, const char ** const out, uint64_t* const size) {
 FILE *fp = fopen(filename,"r");
 if(!fp) {
   fflush(fp); 
   fclose(fp); 
   return;
 }
 struct stat f_s;
 stat(filename,&f_s);
 *size = f_s.st_size;

 *out = (char*)malloc((f_s.st_size)*sizeof(char));

 fread((void*)*out,sizeof(char),*size,fp);
 fflush(fp);
 fclose(fp);
}

static inline
void map_file(const char* const file, uint8_t** const out, uint64_t* size){
    int file_descriptor;
    struct stat file_stat;

    file_descriptor = open(file, O_RDONLY);
    if (file_descriptor == -1) {
        perror("Error opening file");
        return;
    }
    // Get file information
    if (fstat(file_descriptor, &file_stat) == -1) {
        perror("Error getting file information");
        close(file_descriptor);
        return;
    }

    // Map the file to memory
    *out = (uint8_t*)mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
    if (*out == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(file_descriptor);
        return;
    }

    *size = file_stat.st_size;
    close(file_descriptor);
}

static inline
void umap_file(void* map, uint64_t size){
  if (munmap(map, size) == -1) {
      perror("Error unmapping file");
  }
}

