objects = crc.o script.o update_mbr.o

all: $(objects)
	gcc -o update_mbr $(objects) -static

clean:
	rm -rf update_mbr $(objects)
