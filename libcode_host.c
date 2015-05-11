#include <stdio.h>    
#include <stdlib.h>    
#include <sys/types.h>    
#include <string.h>    
#include <libusb.h>    

/***** compile with "arm-linux-gnueabi-gcc -static -o libcode libcode.c `pkg-config libusb-1.0 --libs --cflags`" ******/
#define EP_IN 0x81
#define EP_OUT 0x02
int main(int argc, char const *argv[])
{
    /* code */
    char data[1000];
    int transfer_size, err;

    libusb_device_handle* dev;

    //Initialize libusb with default context
    libusb_init(NULL);

    //Open device with default libusb context
    //dragon board VID = 0xXXXX PID = 0xXXXX
    dev = libusb_open_device_with_vid_pid(NULL, 0x1111,0x2222);

    if (dev == NULL){
        printf("Device not found, exiting...\n");
        return -1;
    }

    err = libusb_set_configuration(dev,1);

    if(err){
        printf("Cannot set configuration, exiting...\n");
        return  -1;
    }

    //Claim interface 0 on the device
    err = libusb_claim_interface(dev,0);

    if (err) {
        printf("Cannot claim interface, exiting...\n");
        return -1;
    }

    char* request = "Requesting sensor data.";

    while(1){
        err = libusb_bulk_transfer(dev,EP_OUT,request,strlen(data),&transfer_size,2000);
       if(err == -7){
            printf("Timeout, reattempting.\n");
            continue;
        }
        else if(err){
            printf("Error %s. Exiting...\n",libusb_error_name(err));
            return -1;
        }
        else
            break;
    }
    while(1){
        //receive data
        err = libusb_bulk_transfer(dev,EP_IN,data,sizeof(data),&transfer_size,2000);
        if(err == -7){
            printf("Timeout, reattempting.\n");
            continue;
        }
        else if(err){
            printf("Error %s. Exiting...\n",libusb_error_name(err));
            return -1;
        }
    }

    return 0;
}
