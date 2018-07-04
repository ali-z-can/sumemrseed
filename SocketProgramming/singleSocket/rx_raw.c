#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <errno.h>
#include <libgen.h>
#include <time.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>

#include "myether.h"
/*****************************************************************************
 * C03:	  PRIVATE CONSTANT DEFINITIONS
 *****************************************************************************/
#define MAX_IFNAMSIZ 32
#define ETHER_TYPE   ((u_int16_t) 0x1234)
#define RX_BUF_SIZE	1600
/*****************************************************************************
 * C04:   PRIVATE DATA TYPES
 *****************************************************************************/
struct rx_pack_hdr {
    struct ether_header eth;
    char name[10];
    char surname[10];
    char filename[32];
    int filesize;
    uint16_t fragmentcount;
    uint16_t fragmanetIndex;
    uint32_t size;
    uint32_t crc;
    char data[0];

}__attribute__((packed));
/*****************************************************************************
 * C05:   PRIVATE DATA DECLARATIONS
 *****************************************************************************/

/*****************************************************************************
 * C06:   PRIVATE (LOCAL) FUNCTION PROTOTYPES
 *****************************************************************************/
static void usage();
/*****************************************************************************
 * C07:   GLOBAL DATA DECLARATIONS
 *****************************************************************************/

/*****************************************************************************
 * C08:   GLOBAL FUNCTIONS
 *****************************************************************************/


uint32_t crc32_for_byte(uint32_t r){
    int j;
   
    for (j = 0; j < 8; ++j)
        r = (r & 1 ? 0 : (uint32_t)0xEDB88320L) ^ r >> 1;
    return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc){
    size_t i;
    static uint32_t table[0x100];
    if (!*table)
        for (i = 0; i < 0x100; ++i)
            table[i] = crc32_for_byte(i);

    for (i = 0; i < n_bytes; ++i){
	       
	*crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;

	}
}

void write_to_file(int *sizes, char **fragments, char *fname, int fragsize) {

    FILE* outp;
    outp = fopen(fname,"w");
    for (int i = 0; i < fragsize; ++i) {
        fwrite(fragments[i],sizes[i],1,outp);
        free(fragments[i]);
    }
    free(sizes);
    free(fragments);
    fclose(outp);
}



    int main(int argc, char *argv[])
{
	
	uint32_t *CRCcheck; 
    int sfd;
    char ifname[MAX_IFNAMSIZ] = {0};
    int ret;
    //struct timespec sleep_time;
    struct rx_pack_hdr *hdr;
    char *buffer;
    char *arg_ifname;

    if (argc != 2) {
        usage();
        goto bail;
    }
	
    arg_ifname = argv[1];

    snprintf(ifname, MAX_IFNAMSIZ, "%s", arg_ifname);

    if (!net_device_up(ifname)) {
        fprintf(stderr, "%s is not up\n", ifname);
        goto bail;
    }

    sfd = net_create_raw_socket(ifname, ETHER_TYPE, 0);
    if (sfd == -1) {
        fprintf(stderr, "failed to init socket\n");
        goto bail;
    }

    buffer = malloc(RX_BUF_SIZE);
    if (!buffer) {
        fprintf(stderr, "memory allocation error!\n");
        goto bail;
    }
    hdr = (struct rx_pack_hdr*)buffer;

    while (1) {
		
        ret = recv(sfd, hdr, RX_BUF_SIZE, 0);
        if (ret <= 0) {
            fprintf(stderr, "ERROR: recv failed ret: %d, errno: %d\n", ret, errno);
            goto bail;
        }

        char **indexing;
	int *arr;
	arr = calloc(hdr->fragmentcount,sizeof(int));
	indexing = calloc(hdr->fragmentcount,sizeof(char *));
        int i;
        
        struct rx_pack_hdr *hdr2;
	hdr2 = malloc(RX_BUF_SIZE);
	memcpy(hdr2,hdr,RX_BUF_SIZE);        
	
        int done;
        done = 0;
        int count;
        count = 0;
	
        while(!done){

		
		
            ret = recv(sfd, hdr, RX_BUF_SIZE, 0);
            if (ret <= 0) {
                fprintf(stderr, "ERROR: recv failed ret: %d, errno: %d\n", ret, errno);
                goto bail;
            }

            if(strcmp(hdr->name,hdr2->name) == 0 && strcmp(hdr2->surname, hdr->surname) == 0 && strcmp(hdr->filename, hdr2->filename) == 0 && hdr->filesize == hdr2->filesize) {
		              
		int checkcrc;
               CRCcheck = &checkcrc;
		checkcrc = 0;		 
		
                crc32(hdr->data, (size_t)hdr->size, CRCcheck);
			//fprintf(stderr,"here\n");
                if(*CRCcheck == hdr->crc){
			fprintf(stderr, "%d bytes received \t", ret);	
			fprintf(stderr, "written to index %d %d\n", hdr->fragmanetIndex,hdr->size);
			indexing[hdr->fragmanetIndex - 1] = (char *) calloc(hdr->size,sizeof(char));
                   	//indexing[hdr->fragmanetIndex - 1] = hdr->data;
			memcpy(indexing[hdr->fragmanetIndex -1],hdr->data,hdr->size);
                     if(arr[hdr->fragmanetIndex-1] == 0){
                        count++;
                        arr[hdr->fragmanetIndex-1] =hdr->size;
			
			
                    }
			
			//fprintf(stderr,"received \n");
                }
            }
		
            if(count == hdr->fragmentcount){
                done = 1;
            }
		
           



        }
	
	 write_to_file(arr, indexing,hdr->filename, hdr->fragmentcount);

	//write_buffer_to_file(hdr->filename,indexing,hdr->size);

	 //write_buffer_to_file(hdr->filename,(char *)indexing,hdr->size);
            goto bail;
        //
        //fprintf(stderr, "data: %s\n", hdr->data);
    }

    return 0;

    bail:
    return -1;
}
/*****************************************************************************
 * C09:   PRIVATE FUNCTIONS
 *****************************************************************************/
static void usage()
{
    fprintf(stderr, "\nUsage:\n./rx_raw <ifname>\n");
    fprintf(stderr, "Example:\n./rx_raw eth0\n");
}
/*****************************************************************************
 * C10:   END OF CODE
 *****************************************************************************/
