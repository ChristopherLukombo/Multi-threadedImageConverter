Pour démarrer le programme, il faut le compiler avec la commande suivante : 

gcc-9 bitmap.c  edge-detect.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all -lpthread

Puis pour le lancer :

./a.out -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all </chemin des fichiers d'entrée/> </chemin des fichiers de sortie/> 'nombre de threads' -boxblur -edgedetect -sharpen -lpthread

Exemple :
./a.out -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all /mnt/c/Users/christopher/ProgramDev/Multi-threadedImageConverter/Input/ /mnt/c/Users/christopher/ProgramDev/Multi-threadedImageConverter/Output/ 4 -boxblur -edgedetect -sharpen -lpthread

