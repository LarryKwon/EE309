CFLAGS = -fno-stack-protector
all: target

target: target.c
	gcc $(CFLAGS) -o target target.c

clean:
	rm -f target
