CC = gcc
SRC_DIR = src
OBJ_DIR = obj

all: stnc

stnc: $(OBJ_DIR)/stnc.o $(OBJ_DIR)/handlers.o $(OBJ_DIR)/util.o
	$(CC) $(OBJ_DIR)/stnc.o $(OBJ_DIR)/handlers.o $(OBJ_DIR)/util.o -o stnc -lrt

$(OBJ_DIR)/stnc.o: $(SRC_DIR)/stnc.c $(SRC_DIR)/stnc.h
	$(CC) -c $(SRC_DIR)/stnc.c -o $(OBJ_DIR)/stnc.o

$(OBJ_DIR)/handlers.o: $(SRC_DIR)/handlers.c $(SRC_DIR)/handlers.h
	$(CC) -c $(SRC_DIR)/handlers.c -o $(OBJ_DIR)/handlers.o 

$(OBJ_DIR)/util.o: $(SRC_DIR)/util.c $(SRC_DIR)/util.h
	$(CC) -c $(SRC_DIR)/util.c -o $(OBJ_DIR)/util.o

clean:
	rm -f stnc *.o *.txt
