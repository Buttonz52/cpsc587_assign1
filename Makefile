#Variables
CC = g++
CFLAGS = -std=c++11
LIBS = -lGL -lglfw -lGraphicsMagick++
INCLUDES = -I/usr/include/GraphicsMagick
EXE = -o a.out
all:
	$(CC) $(CFLAGS) *.cpp $(EXE) $(LIBS) $(INCLUDES)

run:
	./a.out
clean:
	rm -rf *.o
