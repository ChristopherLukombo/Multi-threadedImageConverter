Pour démarrer le programme, lancez la commande suivante : 

gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all </chemin du fichier/bmp_tank.bmp>

Exemple :
gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all /mnt/c/Users/christopher/CLionProjects/Multi-threadedImageConverter/bmp_tank.bmp