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
#include <pthread.h>


#define DIM 3
#define LENGHT DIM
#define OFFSET DIM /2


const float KERNEL[DIM][DIM] = {{-1, -1, -1},
                                {-1, 8,  -1},
                                {-1, -1, -1}};

int numberEntriesInDir = 0;

typedef struct Color_t {
    float Red;
    float Green;
    float Blue;
} Color_e;

#define STACK_MAX 20

typedef struct stack_t {
    Image image[STACK_MAX];
    int count;
    char *fileName[STACK_MAX];
    char *name[STACK_MAX];
    int max;
    int count_progress;
    pthread_mutex_t lock;
    pthread_cond_t can_consume;
    pthread_cond_t can_produce;
} Stack;

static Stack stack;

void apply_effect(Image *original, Image *new_i);

void *clean_directory(void *argument);

void *producer(void *argument);

void *consumer(void *argument);

const char *get_filename_ext(const char *filename);

int isValidParameters(char *const *argv);

int isValidExpectedThread(char *const *argv);

int countEntriesInDir(const char *dirname);

void *readDirectory(void *argument);

char *getInputNameFileAbsolute(char *const *argv, const struct dirent *deInput);

char *getoutputNameFileAbsolute(char *const *argv);

void stack_init() {
    pthread_cond_init(&stack.can_produce, NULL);
    pthread_cond_init(&stack.can_consume, NULL);
    pthread_mutex_init(&stack.lock, NULL);
    stack.max = STACK_MAX;
    stack.count = 0;
    srand(time(NULL));
}


int main(int argc, char **argv) {
    pthread_t threadCleanDirectory;
    pthread_t threadTreatment;

    if (NULL == argv[6] || NULL == argv[7]) {
        printf("The value of the Inputdirectory or Outputdirectory is missing");
        return 0;
    }

    if (!isValidParameters(argv)) {
        printf("./apply-effect \"./in/\" \"./out/\" 3 -boxblur -edgedetect -sharpen");
        return 0;
    }

    clean_directory((void *) argv[7]);

    readDirectory((void *) argv);

    int numberOfThreads = atoi(argv[8]);

    pthread_t threads_id[numberOfThreads];
    stack_init();
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


    for (int i = 0; i < numberOfThreads; i++) {
        pthread_create(&threads_id[i], &attr, producer, (void *) argv);
    }

    pthread_create(&threads_id[numberOfThreads], NULL, consumer, (void *) argv);
    //on attends le consommateur
    pthread_join(threads_id[numberOfThreads], NULL);
    return 0;

}


void *consumer(void *argument) {

    char *const *argv = (char *const *) argument;
    int total = 0;

    while (1) {
        pthread_mutex_lock(&stack.lock);

        while (stack.count == 0) {
            printf("[CONSUMER] Rien a consommer :( \n");
            pthread_cond_wait(&stack.can_consume, &stack.lock);
        }
        stack.count--;

        char *output = getoutputNameFileAbsolute(argv);

        printf("[CONSUMER] Je consomme !\n");
        printf("[CONSUMER] Sauvegarde de : %s\n", output);
        save_bitmap(stack.image[stack.count], output);
        total++;
        printf("[CONSUMER] J'ai finit, la nouvelle image est sur le disque !\n");
        if (total >= numberEntriesInDir) {
            printf("[CONSUMER] Nombre d'image sorties : %d !\n", numberEntriesInDir);
            break;
        }
        pthread_cond_signal(&stack.can_produce);
        pthread_mutex_unlock(&stack.lock);
    }

    return NULL;
}

void *producer(void *argument) {

    while (1) {
        pthread_mutex_lock(&stack.lock);
        if (stack.count < stack.max) {

            printf("File open : %s \n", stack.fileName[stack.count]);

            Image img = open_bitmap(stack.fileName[stack.count]);
            Image new_i;
            apply_effect(&img, &new_i);

            stack.image[stack.count] = new_i;
            stack.count++;


            printf("J'ai produit !\n");
            pthread_cond_signal(&stack.can_consume);
        } else {
            printf("Je ne peux plus produire :(\n");
            while (stack.count >= stack.max) {
                pthread_cond_wait(&stack.can_produce, &stack.lock);
            }
            printf("Je peux a nouveau produire :)\n");
        }
        pthread_mutex_unlock(&stack.lock);
    }

}

void *readDirectory(void *argument) {
    char *const *argv = (char *const *) argument;

    struct dirent *deInput = NULL;  // Pointer for directory entry

    // opendir() returns a pointer of DIR type.
    DIR *dr = opendir(argv[6]);
    if (NULL == dr)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory");
        return NULL;
    }

    if (0 == countEntriesInDir(argv[6])) {
        printf("Directory is empty");
        return 0;
    }

    while ((deInput = readdir(dr)) != NULL) {
        if ((0 == strcmp(get_filename_ext(deInput->d_name), "bmp"))
            || (0 == strcmp(get_filename_ext(deInput->d_name), "BMP"))) {

            char *inputName = getInputNameFileAbsolute(argv, deInput);

            const unsigned long length2 = strlen(deInput->d_name) + 1;
            char *name = malloc(sizeof(char) * (length2));
            if (name == NULL) {
                printf("[INFO] Allocation impossible :");
            }

            sprintf(name, deInput->d_name);

            printf("[INFO] %s\n", inputName);
            stack.fileName[stack.count_progress] = inputName;
            stack.name[stack.count_progress] = name;
            stack.count_progress++;


        }
    }
    closedir(dr);
}

char *getInputNameFileAbsolute(char *const *argv, const struct dirent *deInput) {
    const unsigned long length = strlen(argv[6]) + strlen(deInput->d_name) + 1;
    char *input = malloc(sizeof(char) * (length));
    if (input == NULL) {
        printf("[INFO] Allocation impossible :");
    }

    char *res = malloc(strlen(argv[6]) + strlen(deInput->d_name) + 1);
    if (res) {
        strcpy(res, argv[6]);
        strcat(res, deInput->d_name);
    }
    input = res;
    return input;
}

char *getoutputNameFileAbsolute(char *const *argv) {
    const unsigned long length = strlen(argv[7]) + strlen(stack.name[stack.count]) + 1;
    char *output = malloc(sizeof(char) * (length));
    if (output == NULL) {
        printf("[INFO] Allocation impossible :");
    }

    strcat(output, argv[7]);
    strcat(output, stack.name[stack.count]);
    return output;
}

int isValidExpectedThread(char *const *argv) {

    if (!(argv[8])) {
        return 0;
    }

    numberEntriesInDir = countEntriesInDir(argv[6]);
    int numberOfThreadExpected = atoi(argv[8]);

    if (numberEntriesInDir == 0 || numberOfThreadExpected == 0 || numberOfThreadExpected >= numberEntriesInDir) {
        return 0;
    }

    return 1;
}

int isValidParameters(char *const *argv) {
    if(isValidExpectedThread(argv) == 0) {
        return 0;
    }

    if ((argv[9]) && 0 != strcmp("-boxblur", argv[9])) {
        return 0;
    }

    if ((argv[10]) && 0 != strcmp("-edgedetect", argv[10])) {
        return 0;
    }

    if ((argv[11]) && 0 != strcmp("-sharpen", argv[11])) {
        return 0;
    }

    return 1;
}

void *clean_directory(void *argument) {
    char *outPutDirectory = (char *) argument;
    DIR *dr = opendir(outPutDirectory);

    struct dirent *deOutPut;

    while (NULL != (deOutPut = readdir(dr))) {
        unsigned long outPutDirectoryLength = strlen(outPutDirectory) + strlen(deOutPut->d_name);
        char *fileOutPut = malloc(sizeof(char) * (outPutDirectoryLength));

        if (NULL == fileOutPut) {
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

int countEntriesInDir(const char *dirname) {
    int n = 0;
    struct dirent *d = NULL;
    DIR *dir = opendir(dirname);
    if (dir == NULL) return 0;
    while ((d = readdir(dir)) != NULL) {
        if ((0 != strcmp(d->d_name, ".")) && (0 != strcmp(d->d_name, ".."))) {
            n++;
        }
    }
    closedir(dir);
    return n;
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
            dest->r = (uint8_t) (c.Red <= 0 ? 0 : c.Red >= 255 ? 255 : c.Red);
            dest->g = (uint8_t) (c.Green <= 0 ? 0 : c.Green >= 255 ? 255 : c.Green);
            dest->b = (uint8_t) (c.Blue <= 0 ? 0 : c.Blue >= 255 ? 255 : c.Blue);
        }
    }
}
