all: libcode_host libcode_dev

libcode_dev: libcode_dev.c
	arm-linux-gnueabi-gcc -pthread -static -o libcode_dev libcode_dev.c -L/home/wesleybrooks/uav/libusb-1.0.9/libusb/.libs/ -lusb-1.0

libcode_host: libcode_host.c
	gcc -pthread -o libcode_host libcode_host.c -L/home/wesleybrooks/libusb-1.0.9/libusb/.libs/ -lusb-1.0

clean:
	rm libcode_host libcode_dev