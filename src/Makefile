OBS = Extended2.o main.o
CC = g++
DEBUG = -g 
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

recupero : $(OBS)
	$(CC) $(LFLAGS) $(OBS) -o recupero

Extended2.o : ext2_fs.h Extended2.h
	$(CC) $(CFLAGS) Extended2.cpp

main.o : ext2_fs.h main.h Extended2.h
	$(CC) $(CFLAGS) main.cpp
clean :
	rm $(OBS) recupero
