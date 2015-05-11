#include <stdio.h>    
#include <stdlib.h>    
#include <sys/types.h>    
#include <string.h>    
#include <libusb.h>    

/***** compile with "arm-linux-gnueabi-gcc -static -o libcode libcode.c `pkg-config libusb-1.0 --libs --cflags`" ******/

#define BULK_EP_OUT     0x02    
#define BULK_EP_IN      0x81   

int interface_ref = 0;    
int alt_interface,interface_number;    

int print_configuration(struct libusb_device_handle *hDevice,struct libusb_config_descriptor *config)    
{    
    char *data;    
    int index;    

    data = (char *)malloc(512);    
    memset(data,0,512);    

    index = config->iConfiguration;    

    libusb_get_string_descriptor_ascii(hDevice,index,data,512);    

    printf("\nInterface Descriptors: ");    
    printf("\n\tNumber of Interfaces : %d",config->bNumInterfaces);    
    printf("\n\tLength : %d",config->bLength);    
    printf("\n\tDesc_Type : %d",config->bDescriptorType);    
    printf("\n\tConfig_index : %d",config->iConfiguration);    
    printf("\n\tTotal length : %lu",config->wTotalLength);    
    printf("\n\tConfiguration Value  : %d",config->bConfigurationValue);    
    printf("\n\tConfiguration Attributes : %d",config->bmAttributes);    
    printf("\n\tMaxPower(mA) : %d\n",config->MaxPower);    

    free(data);    
    data = NULL;    
    return 0;    
}    

struct libusb_endpoint_descriptor* active_config(struct libusb_device *dev,struct libusb_device_handle *handle)    
{    
    struct libusb_device_handle *hDevice_req;    
    struct libusb_config_descriptor *config;    
    struct libusb_endpoint_descriptor *endpoint;    
    int altsetting_index,interface_index=0,ret_active;    
    int i,ret_print;    

    hDevice_req = handle;    

    ret_active = libusb_get_active_config_descriptor(dev,&config);    
    ret_print = print_configuration(hDevice_req,config);    

    for(interface_index=0;interface_index<config->bNumInterfaces;interface_index++)    
    {    
        const struct libusb_interface *iface = &config->interface[interface_index];    
        for(altsetting_index=0;altsetting_index<iface->num_altsetting;altsetting_index++)    
        {    
            const struct libusb_interface_descriptor *altsetting = &iface->altsetting[altsetting_index];    

            int endpoint_index;    
            for(endpoint_index=0;endpoint_index<altsetting->bNumEndpoints;endpoint_index++)    
            {    
                const struct libusb_endpoint_desriptor *ep = &altsetting->endpoint[endpoint_index];    
                endpoint = ep;      
                alt_interface = altsetting->bAlternateSetting;    
                interface_number = altsetting->bInterfaceNumber;    
            }    

            printf("\nEndPoint Descriptors: ");    
            printf("\n\tSize of EndPoint Descriptor : %d",endpoint->bLength);    
            printf("\n\tType of Descriptor : %d",endpoint->bDescriptorType);    
            printf("\n\tEndpoint Address : 0x0%x",endpoint->bEndpointAddress);    
            printf("\n\tMaximum Packet Size: %x",endpoint->wMaxPacketSize);    
            printf("\n\tAttributes applied to Endpoint: %d",endpoint->bmAttributes);    
            printf("\n\tInterval for Polling for data Tranfer : %d\n",endpoint->bInterval);    
        }    
    }    
    libusb_free_config_descriptor(NULL);    
    return endpoint;    
}    

int main(void)    
{    
    int r = 1;    
    struct libusb_device **devs;    
    struct libusb_device_handle *handle = NULL, *hDevice_expected = NULL;    
    struct libusb_device *dev,*dev_expected;    

    struct libusb_device_descriptor desc;    
    struct libusb_endpoint_descriptor *epdesc;    
    struct libusb_interface_descriptor *intdesc;    

    ssize_t cnt;    
    int e = 0,config2;    
    int i = 0,index;    
    char str1[64], str2[64];    
    char found = 0;    

// Init libusb     
    r = libusb_init(NULL);    
    if(r < 0)    
    {    
        printf("\nfailed to initialise libusb\n");    
        return 1;    
    }    
    else    
        printf("\nInit Successful!\n");    

// Get a list os USB devices    
    cnt = libusb_get_device_list(NULL, &devs);    
    if (cnt < 0)    
    {    
        printf("\nThere are no USB devices on bus\n");    
        return -1;    
    }    
    printf("\nDevice Count : %d\n-------------------------------\n",cnt);    

    while ((dev = devs[i++]) != NULL)    
    {    
        r = libusb_get_device_descriptor(dev, &desc);    
        if (r < 0)    
            {    
            printf("failed to get device descriptor\n");    
            libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            break;    
        }    

        printf("\nDevice Descriptors: ");    
        printf("\n\tVendor ID : %x",desc.idVendor);    
        printf("\n\tProduct ID : %x",desc.idProduct);    
        printf("\n\tSerial Number : %x",desc.iSerialNumber);    
        printf("\n\tSize of Device Descriptor : %d",desc.bLength);    
        printf("\n\tType of Descriptor : %d",desc.bDescriptorType);    
        printf("\n\tUSB Specification Release Number : %d",desc.bcdUSB);    
        printf("\n\tDevice Release Number : %d",desc.bcdDevice);    
        printf("\n\tDevice Class : %d",desc.bDeviceClass);    
        printf("\n\tDevice Sub-Class : %d",desc.bDeviceSubClass);    
        printf("\n\tDevice Protocol : %d",desc.bDeviceProtocol);    
        printf("\n\tMax. Packet Size : %d",desc.bMaxPacketSize0);    
        printf("\n\tNo. of Configuraions : %d\n",desc.bNumConfigurations);  

        e = libusb_open(dev,&handle);

        printf("idVendor: %d\n",desc.idVendor); 
        if (e < 0)    
        {   
            printf("error code: %d\n",e);  
            printf("error opening device\n");    
            libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            break;    
        }    

        if(desc.idVendor == 1105 && desc.idProduct == 8043)    
        {    
        found = 1;    
        break;    
        } 

        e = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char*) str1, sizeof(str1));    
        if (e < 0)    
        {    
        libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            break;    
        }    
        printf("\nManufactured : %s",str1);    

        e = libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char*) str2, sizeof(str2));    
        if(e < 0)    
        {    
        libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            break;    
        }    
        printf("\nProduct : %s",str2);    
        printf("\n----------------------------------------");

        
           
    }//end of while    
    if(found == 0)    
    {    
        printf("\nDevice NOT found\n");    
        libusb_free_device_list(devs,1);    
        libusb_close(handle);    
        return 1;    
    }    
    else    
    {    
        printf("\nDevice found");    
        dev_expected = dev;    
        hDevice_expected = handle;    
    }    

    e = libusb_get_configuration(handle,&config2);    
    if(e!=0)    
    {    
        printf("\n***Error in libusb_get_configuration\n");    
        libusb_free_device_list(devs,1);    
        libusb_close(handle);    
        return -1;    
    }    
    printf("\nConfigured value : %d",config2);    

    if(config2 != 1)    
    {    
        libusb_set_configuration(handle, 1);    
        if(e!=0)    
        {    
            printf("Error in libusb_set_configuration\n");    
            libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            return -1;    
        }    
        else    
            printf("\nDevice is in configured state!");    
    }    

    libusb_free_device_list(devs, 1);    

    if(libusb_kernel_driver_active(handle, 0) == 1)    
    {    
        printf("\nKernel Driver Active");    
        if(libusb_detach_kernel_driver(handle, 0) == 0)    
            printf("\nKernel Driver Detached!");    
        else    
        {    
            printf("\nCouldn't detach kernel driver!\n");    
            libusb_free_device_list(devs,1);    
            libusb_close(handle);    
            return -1;    
        }    
    }    

    e = libusb_claim_interface(handle, 0);    
    if(e < 0)    
    {    
        printf("\nCannot Claim Interface");    
        libusb_free_device_list(devs,1);    
        libusb_close(handle);    
        return -1;    
    }    
    else    
        printf("\nClaimed Interface\n");    

    active_config(dev_expected,hDevice_expected);    

    //   Communicate     

    char my_string[65], my_string1[65];    
    int transferred = 0;    
    int received = 0;    
    int length = 0;    

    //my_string = (char *)malloc(nbytes + 1);    
    //my_string1 = (char *)malloc(nbytes + 1);    
    memset(my_string,'\0',64);    
    memset(my_string1,'\0',64);    
    char ch;
    // Get sensor data
    FILE *fp = fopen("/dev/iio:device4","r");
    if( fp == NULL )
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    printf("File /dev/iio:device4 opened successfully\n");
    char accel_data[1000];
    //printf("The contents of %s file are :\n", file_name);
    memset(accel_data,'\0',1000);
    i = 0;
    while( !feof(fp)){
        ch = fgetc(fp);
        accel_data[i] = ch;
        i++;
    }
 
    fclose(fp);



    //strcpy(my_string,"prasad divesd");    
    length = strlen(accel_data);    

    printf("\nTo be sent : %s",accel_data);    

    while(1){
 
  
        e = libusb_bulk_transfer(handle,BULK_EP_OUT,my_string1,64,&received,3000);  //64 : Max Packet Lenght    
        if(e == 0)    
        {    
            printf("\nReceived: ");    
            printf("%s",my_string1);   
        }   
        else if(e == -7){
            printf("timeout, waiting longer.\n");
            continue;
        }
        else 
        {    
            printf("\nError in read! e = %d and received = %d\n",e,received);    
            return -1;    
        } 


    
        e = libusb_bulk_transfer(handle,BULK_EP_IN,accel_data,length,&transferred,0);    
        if(e == 0 && transferred == length)    
        {    
            printf("\nWrite successful!");    
            printf("\nSent %d bytes with string: %s\n", transferred, my_string);    
        }    
        else    
            printf("\nError in write! e = %d and transferred = %d\n",e,transferred);  

        e = libusb_release_interface(handle, 0);    

        libusb_close(handle);    
        libusb_exit(NULL); 
    }
       

    printf("\n");    
    return 0;    
}
