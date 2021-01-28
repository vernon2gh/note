CFLAGS += -Werror -Wall

record.so: record.c
	gcc $(CFLAGS) -fPIC -shared -Xlinker -x -o $@ $<

install:
	sudo cp record.so /lib/security/

clean:
	rm -fr *.so
