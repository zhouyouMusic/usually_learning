all:test

test:cloud_sounds_recognition.c
	gcc $^ ../libs/libusc.so  -Wall -O2 -o $@ -L. -lrt
clean:
	rm -f test
	rm -f test.o
