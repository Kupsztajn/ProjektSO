EXEC_PSZCZELARZ = pszczelarz
EXEC_KROLOWA = krolowa
EXEC_PSZCZOLA = pszczola
EXEC_TEST_SHM = init
EXEC_MASTER = master

CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lpthread

# Pliki Ÿród³owe
SRCS = shm.c #sem.c
OBJS = shm.o #sem.o


all: $(EXEC_PSZCZELARZ) $(EXEC_KROLOWA) $(EXEC_PSZCZOLA) $(EXEC_TEST_SHM) $(EXEC_MASTER)


$(EXEC_PSZCZELARZ): pszczelarz.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ pszczelarz.c $(OBJS)


$(EXEC_KROLOWA): krolowa.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ krolowa.c $(OBJS) $(LDFLAGS)


$(EXEC_PSZCZOLA): pszczola.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ pszczola.c $(OBJS)

$(EXEC_TEST_SHM): init.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ init.c $(OBJS)


$(EXEC_MASTER): master.c
	$(CC) $(CFLAGS) -o $@ master.c $(LDFLAGS)


shm.o: shm.c shm.h
	$(CC) $(CFLAGS) -c shm.c

sem.o: sem.c sem.h
	$(CC) $(CFLAGS) -c sem.c

clean:
	rm -f $(EXEC_PSZCZELARZ) $(EXEC_KROLOWA) $(EXEC_PSZCZOLA) $(EXEC_TEST_SHM) $(EXEC_MASTER) $(OBJS)
