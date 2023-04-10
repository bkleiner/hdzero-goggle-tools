CFLAGS = -g -I. 
CXXFLAGS = -g -I. 

objects = sunxi-image.o main.o

all: $(objects)
	g++ -o sunxi-mbr $(objects) -static

clean:
	rm -rf sunxi-mbr $(objects)
