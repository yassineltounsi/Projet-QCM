CC = gcc
CFLAGS = -Wall -Wextra -std=c11
OBJ = main.o interface.o qcm.o

qcm: $(OBJ)
	$(CC) $(CFLAGS) -o qcm_app $(OBJ)

main.o: main.c interface.h
	$(CC) $(CFLAGS) -c main.c

interface.o: interface.c interface.h qcm.h
	$(CC) $(CFLAGS) -c interface.c

qcm.o: qcm.c qcm.h
	$(CC) $(CFLAGS) -c qcm.c

clean:
	rm -f $(OBJ) qcm_app
