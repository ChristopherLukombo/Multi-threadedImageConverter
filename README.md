Pour démarrer le programme, lancez la commande suivante : 

gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all </chemin des fichiers d'entrée/> </chemin des fichiers de sortie/>

Exemple :
gcc edge-detect.c bitmap.c -O2 -ftree-vectorize -fopt-info -mavx2 -fopt-info-vec-all /mnt/c/Users/christopher/ProgramDev/Multi-threadedImageConverter/Input/ /mnt/c/Users/christopher/ProgramDev/Multi-threadedImageConverter/Output/