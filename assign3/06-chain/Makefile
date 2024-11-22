CFLAGS = -no-pie -fno-stack-protector
LDFLAGS = -lseccomp
all: target

target: target.c
	gcc $(CFLAGS) -o target target.c $(LDFLAGS)

clean:
	rm -f target
