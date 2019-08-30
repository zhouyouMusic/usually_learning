all:
	gcc -g source/*.c  source/*.so  -I include/ -L curl/lib/ -lcurl  -L lib/ -lvoice -lasound  -lm -lpthread -lmad -o json


clean:
	rm json hello play client
