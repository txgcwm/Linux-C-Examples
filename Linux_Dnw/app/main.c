/*
 * dnw.c
 * Author: Sam <txgcwm@163.com>
 * License: GPL
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <usb.h>
    


#define DNW_USB_IDVENDOR 		0x067b
#define DNW_USB_IDPRODUCT 		0x2303
#define FILENAME_MAXLEN			100
#define DNW_BLOCK_SIZE			512
#define DNW_MODE_KERNEL_USB		0
#define DNW_MODE_LIB_USB		1

static void usage(void)
{
    printf("Usage: dnw [-f filename]\n");
    printf("       -f      finename\n");

    return;
}

static unsigned char *dnw_read_filedata2buff(char *filename,int *len)
{
	int i = 0;
	int fd = -1;
	int checksum = 0;
	unsigned char *buffer = NULL;
	struct stat file_stat;

	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		printf("Can not open file %s\n", filename);
		return NULL;
	}

	if(fstat(fd, &file_stat) < 0)
	{
		printf("Get file size filed!\n");
		close(fd);
		return NULL;
	}

	buffer = (unsigned char *)malloc(file_stat.st_size+10);
	if(buffer == NULL)
	{
		printf("malloc failed!\n");
		close(fd);
		return NULL;
	}
	memset(buffer, '\0', sizeof(buffer));

	if(read(fd, buffer+8, file_stat.st_size) != file_stat.st_size)
	{
		printf("Read file failed!\n");
		close(fd);
		free(buffer);
		buffer = NULL;
		return NULL;
	}

	*((u_int32_t*)buffer) = 0x30000000; //download address
	*((u_int32_t*)buffer+1) = file_stat.st_size + 10; //download size;
	for(i=8; i<file_stat.st_size+8; i++)
	{
		checksum += buffer[i];
	}	
	*((unsigned short *)(buffer+8+file_stat.st_size)) = checksum;
	*len = file_stat.st_size+10;
	printf("file name: %s,file size: %d bytes\n",filename,*len);

	close(fd);

	return buffer;
}

struct usb_dev_handle *dnw_open_libdev(void)
{
	int ret = 0;
	struct usb_device *dev = NULL;
	struct usb_dev_handle *hdev = NULL;
	struct usb_bus *busses = NULL, *bus = NULL;
	usb_set_debug(5);
	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for(bus=busses;bus;bus=bus->next)
	{   	
    	for(dev=bus->devices;dev;dev=dev->next)
    	{
        	if(dev->descriptor.idVendor == DNW_USB_IDVENDOR 
				&& dev->descriptor.idProduct == DNW_USB_IDPRODUCT)
			{
            	printf("Target usb device found!\n");
            	hdev = usb_open(dev);
            	if(hdev == NULL)
            	{
                 	printf("Cannot open device\n");
            	}
             	else
             	{
					ret = usb_set_configuration(hdev, 1);
					if (ret < 0) 
					{ // Linux only
						ret = usb_detach_kernel_driver_np(hdev,0);
						if (ret < 0) 
						{
						    fprintf(stderr, "Detach_kernel failed.\n");
						    return NULL;
						}
						usb_reset(hdev);
						
						ret = usb_set_configuration(hdev, 1);
						if (ret < 0) 
						{
						    fprintf(stderr, "Set_configuration failed.\n");
						    return NULL;
						}
					} 

                	if(usb_claim_interface(hdev, 0) != 0)
                	{
                    	printf("Cannot claim interface\n");
                    	usb_close(hdev);
                    	hdev = NULL;
                	}
            	}
            	return hdev;
        	}
    	}
	}

	return NULL;
}

static int dnw_write_buffdata2dev(struct usb_dev_handle *ldev,int kdev,char *buffer,int total,int mode)
{
	int len = 0;
	int remain = 0;
	
	if((buffer == NULL) || (total <= 0) || (!(ldev == NULL)&&!(kdev <= 0)))
	{
		printf("Params error!\n");
		return -1;
	}

	remain = total;
	
	while(remain > 0)
	{
		len = remain > DNW_BLOCK_SIZE ? DNW_BLOCK_SIZE:remain;
		
		if(mode == DNW_MODE_KERNEL_USB)
		{			
			if((len = write(kdev, buffer+(total-remain), len)) < 0)
			{
				printf("write data to dev failed!\n");
				return -1;
			}	
		}
		else
		{
			if((len = usb_bulk_write(ldev, 0x02, buffer+(total-remain), len, 3000)) < 0)
			{
		    	printf("usb_bulk_write failed,len:%d,errno:%d %s!\n",len,errno,strerror(errno));
		    	return -1;
			}
		}
		
		remain -= len;
		printf("remain:%d,total:%d\n",remain,total);
		//printf("\r%d£¥ \t %d bytes", (int)((total-remain)*100)/total, total-remain);
		fflush(stdout);		
	}

	return 0;
}

int main(int argc,char **argv)
{
	int c;
	int flag = 0;
	int kdev = -1;
	size_t total = 0;
	int mode = 0;
	char filename[FILENAME_MAXLEN+1];
	unsigned char *buffer = NULL;
	struct usb_dev_handle *ldev = NULL;

	if(argc < 3)
	{
		usage();
		exit(-1);
	}
	memset(filename,'\0',FILENAME_MAXLEN+1);

	while((c = getopt(argc, argv, "?f:")) > 0)
    {
    	switch(c)
    	{
        	case 'f':
            	memcpy(filename,optarg,strlen(optarg));
            	break;
            default:
                usage();
				exit(-1);
    	}
	}
	
	buffer = dnw_read_filedata2buff(filename,&total);
	if(buffer == NULL)
	{
		printf("Can not read the file data!\n");
		return -1;
	}
	
	kdev = open("/dev/dnwbulk0", O_WRONLY);
	if(kdev < 0)
	{
		printf("Can not open kernel dev,try to open lib dev!\n");
		ldev = dnw_open_libdev();
		if(ldev == NULL)
		{
			printf("Can not open lib dev,please check if you have install usb driver!\n");
			flag = -1;
			goto error;
		}
		else
		{
			mode = DNW_MODE_LIB_USB;
		}
	}
	else
	{
		mode = DNW_MODE_KERNEL_USB;
	}

	printf("Wirte data start...\n");

	if(dnw_write_buffdata2dev(ldev,kdev,buffer,total,mode) < 0)
	{
		printf("Can not write the buff data to dev!\n");
		flag = -1;
		goto error;
	}

	printf("Wirte data success!\n");

error:
	free(buffer);
	buffer = NULL;
	if(kdev > 0)
		close(kdev);
	if(ldev != NULL)
	{
		usb_release_interface(ldev, 0); 
		usb_close(ldev);
	}

	return flag;	
}



