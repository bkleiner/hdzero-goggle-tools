CFLAGS = -g -I. 
CXXFLAGS = -g -I. 

objects = main.o dlinfo.o mbr.o image.o

all: $(objects)
	g++ -o sunxi-image $(objects) -static

clean:
	rm -rf sunxi-image $(objects)
