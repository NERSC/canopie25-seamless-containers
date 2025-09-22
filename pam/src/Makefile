CC=gcc
CFLAGS=-fPIC -fno-stack-protector -shared
LDFLAGS=-lpam
TARGET=pam_container.so

$(TARGET): pam_container.c
	$(CC) $(CFLAGS) -o $(TARGET) pam_container.c $(LDFLAGS)

install:
	cp $(TARGET) /lib/security/

clean:
	rm -f $(TARGET)
