/*
	//gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all
	//UTILISER UNIQUEMENT DES BMP 24bits
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bitmap.h"
#include <stdint.h>
#include <dirent.h>
#include "string.h"

#define DIM 3
#define LENGHT DIM
#define OFFSET DIM /2

const float KERNEL[DIM][DIM] = {{-1, -1, -1},
                                {-1, 8,  -1},
                                {-1, -1, -1}};

typedef struct Color_t {
    float Red;
    float Green;
    float Blue;
} Color_e;


void apply_effect(Image *original, Image *new_i);

void clean_directory(char *outPutDirectory);

char *get_file_input(char *const *argv, const struct dirent *deInput);

char *get_file_output(char *const *argv);

void do_treatment(char *const *argv, struct dirent *deInput, const DIR *dr);

const char *get_filename_ext(const char *filename);

int main(int argc, char **argv) {
    if (NULL == argv[6] || NULL == argv[7]) {
        printf("The value of the Inputdirectory or Outputdirectory is missing");
        return 0;
    }
    clean_directory(argv[7]);

    struct dirent *deInput = NULL;  // Pointer for directory entry
    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(argv[6]);
    if (NULL == dr)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory");
        return 0;
    }

    do_treatment(argv, deInput, dr);
    return 0;
}

void do_treatment(char *const *argv, struct dirent *deInput, const DIR *dr) {
    while ((deInput = readdir(dr)) != NULL)
    {
        if ((0 == strcmp(get_filename_ext(deInput->d_name), "bmp"))
            || (0 == strcmp(get_filename_ext(deInput->d_name), "BMP")))
        {
            char *fileInput = get_file_input(argv, deInput);
            printf("File open : %s", fileInput);
            Image img = open_bitmap(fileInput);
            Image new_i;
            apply_effect(&img, &new_i);
            char *fileOutPut = get_file_output(argv);
            save_bitmap(new_i, fileOutPut);
            fileInput = NULL;
            fileOutPut = NULL;
        }
    }
    closedir(dr);
}

char *get_file_input(char *const *argv, const struct dirent *deInput) {
    const unsigned long inputDirectoryLength = strlen(argv[6]) + strlen(deInput->d_name);
    char *fileInput = malloc(sizeof(char) * (inputDirectoryLength));

    if (NULL == fileInput)
    {
        fprintf(stderr, "Unable to allocate");
    }

    fileInput = strcat(fileInput, argv[6]);
    fileInput = strcat(fileInput, deInput->d_name);
    return fileInput;
}

char *get_file_output(char *const *argv) {
    const unsigned long outPutDirectoryLength = strlen(argv[7]) + strlen("test_out.bmp");
    char *fileOutPut = malloc(sizeof(char) * (outPutDirectoryLength));

    if (NULL == fileOutPut)
    {
        fprintf(stderr, "Unable to allocate");
    }

    fileOutPut = strcat(fileOutPut, argv[7]);
    fileOutPut = strcat(fileOutPut, "test_out.bmp");
    return fileOutPut;
}

void clean_directory(char *outPutDirectory) {
    DIR *dr = opendir(outPutDirectory);

    struct dirent *deOutPut;

    while (NULL != (deOutPut = readdir(dr)))
    {
        unsigned long outPutDirectoryLength = strlen(outPutDirectory) + strlen(deOutPut->d_name);
        char *fileOutPut = malloc(sizeof(char) * (outPutDirectoryLength));

        if (NULL == fileOutPut)
        {
            fprintf(stderr, "Unable to allocate");
        }

        fileOutPut = strcat(fileOutPut, outPutDirectory);
        fileOutPut = strcat(fileOutPut, deOutPut->d_name);
        // Delete file
        remove(fileOutPut);
    }
    closedir(dr);
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}


void apply_effect(Image *original, Image *new_i) {

    int w = original->bmp_header.width;
    int h = original->bmp_header.height;

    *new_i = new_image(w, h, original->bmp_header.bit_per_pixel, original->bmp_header.color_planes);

    for (int y = OFFSET; y < h - OFFSET; y++) {
        for (int x = OFFSET; x < w - OFFSET; x++) {
            Color_e c = {.Red = 0, .Green = 0, .Blue = 0};

            for (int a = 0; a < LENGHT; a++) {
                for (int b = 0; b < LENGHT; b++) {
                    int xn = x + a - OFFSET;
                    int yn = y + b - OFFSET;

                    Pixel *p = &original->pixel_data[yn][xn];

                    c.Red += ((float) p->r) * KERNEL[a][b];
                    c.Green += ((float) p->g) * KERNEL[a][b];
                    c.Blue += ((float) p->b) * KERNEL[a][b];
                }
            }

            Pixel *dest = &new_i->pixel_data[y][x];
            dest->r = (uint8_t)(c.Red <= 0 ? 0 : c.Red >= 255 ? 255 : c.Red);
            dest->g = (uint8_t)(c.Green <= 0 ? 0 : c.Green >= 255 ? 255 : c.Green);
            dest->b = (uint8_t)(c.Blue <= 0 ? 0 : c.Blue >= 255 ? 255 : c.Blue);
        }
    }
}
