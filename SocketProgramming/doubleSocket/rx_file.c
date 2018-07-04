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
#include <poll.h>
#define POLL_TIMEOUT 1000 // msec

#include "myether.h"
/*****************************************************************************
 * C03:	  PRIVATE CONSTANT DEFINITIONS
 *****************************************************************************/
#define MAX_IFNAMSIZ 32
#define ETHER_TYPE   ((u_int16_t) 0x1234)
#define ETHER_TYPE2  ((u_int16_t) 0x4321)
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

void write_to_file(int *sizes, char **fragments, char *fname, int fragsize,int *sizes2, char **fragments2, char *fname2, int fragsize2) {

    FILE* outp;
    outp = fopen(fname,"w");
    for (int i = 0; i < fragsize; ++i) {
        fwrite(fragments[i],sizes[i],1,outp);
        free(fragments[i]);
    }
    for (int i = 0; i < fragsize2; ++i) {
        fwrite(fragments2[i],sizes2[i],1,outp);
        free(fragments2[i]);
    }
    free(sizes);
    free(fragments);

    free(sizes2);
    free(fragments2);
    fclose(outp);
}



int main(int argc, char *argv[])
{

    uint32_t *CRCcheck;
    int sfd,sfd2;
    char ifname[MAX_IFNAMSIZ] = {0};
    char ifname2[MAX_IFNAMSIZ] = {0};
    int ret,ret2;
    //struct timespec sleep_time;
    struct rx_pack_hdr *hdr;
    struct rx_pack_hdr *hdr2;
    char *buffer;
    char *buffer2;
    char *arg_ifname;
    char *arg_ifname2;
    struct pollfd ufds[2];

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

    arg_ifname2 = argv[1];

    snprintf(ifname2, MAX_IFNAMSIZ, "%s", arg_ifname2);

    if (!net_device_up(ifname2)) {
        fprintf(stderr, "%s is not up\n", ifname2);
        goto bail;
    }
        fprintf(stderr,"if1: %s if2: %s\n",ifname,ifname2);

    sfd = net_create_raw_socket(ifname, ETHER_TYPE, 0);
    sfd2 = net_create_raw_socket(ifname2, ETHER_TYPE2, 0);


    if (sfd == -1) {
        fprintf(stderr, "failed to init socket\n");
        goto bail;
    }

    if (sfd2 == -1) {
        fprintf(stderr, "failed to init socket\n");
        goto bail;
    }

    ufds[0].fd = sfd;
    ufds[0].events = POLLIN;

    ufds[1].fd = sfd2;
    ufds[1].events = POLLIN;



    buffer = malloc(RX_BUF_SIZE);
    if (!buffer) {
        fprintf(stderr, "memory allocation error!\n");
        goto bail;
    }
    hdr = (struct rx_pack_hdr*)buffer;

    buffer2 = malloc(RX_BUF_SIZE);
    if (!buffer2) {
        fprintf(stderr, "memory allocation error!\n");
        goto bail;
    }
    hdr2 = (struct rx_pack_hdr*)buffer2;


    while (1) {



        ret = recv(sfd, hdr, RX_BUF_SIZE, 0);
        ret2 = recv(sfd2, hdr2, RX_BUF_SIZE, 0);
        if (ret <= 0) {
            fprintf(stderr, "ERROR: recv failed ret: %d, errno: %d\n", ret, errno);
            goto bail;
        }

        if (ret2 <= 0) {
            fprintf(stderr,"this\n");
            fprintf(stderr, "ERROR: recv failed ret: %d, errno: %d\n", ret2, errno);
            goto bail;
        }

        char **indexing;
        int *arr;
        arr = calloc(hdr->fragmentcount,sizeof(int));
        indexing = calloc(hdr->fragmentcount,sizeof(char *));

        char **indexing2;
        int *arr2;
        arr2 = calloc(hdr2->fragmentcount,sizeof(int));
        indexing2 = calloc(hdr2->fragmentcount,sizeof(char *));


        struct rx_pack_hdr *hdr3;
        hdr3 = malloc(RX_BUF_SIZE);
        memcpy(hdr3,hdr,RX_BUF_SIZE);

        struct rx_pack_hdr *hdr4;
        hdr4 = malloc(RX_BUF_SIZE);
        memcpy(hdr4,hdr2,RX_BUF_SIZE);

        int done;
        done = 0;
        int count;
        count = 0;

        while(!done){


            int pp= poll(ufds, 2, POLL_TIMEOUT);
            if (pp == -1) {
                perror("poll");
                continue;
            }


            if (ufds[0].revents & POLLIN) {

                ret = recv(sfd, hdr, RX_BUF_SIZE, 0);
                if (ret <= 0) {
                    fprintf(stderr, "ERROR: recv failed ret: %d, errno: %d\n", ret, errno);
                    goto bail;
                }

                if(strcmp(hdr->name,hdr3->name) == 0 && strcmp(hdr3->surname, hdr->surname) == 0 && strcmp(hdr->filename, hdr3->filename) == 0 && hdr->filesize == hdr3->filesize) {

                    int checkcrc;
                    CRCcheck = &checkcrc;
                    checkcrc = 0;

                    crc32(hdr->data, (size_t)hdr->size, CRCcheck);
                    //fprintf(stderr,"here\n");
                    if(*CRCcheck == hdr->crc){

                        indexing[hdr->fragmanetIndex - 1] = (char *) calloc(hdr->size,sizeof(char));
                        //indexing[hdr->fragmanetIndex - 1] = hdr->data;
                        memcpy(indexing[hdr->fragmanetIndex -1],hdr->data,hdr->size);
                        if(arr[hdr->fragmanetIndex-1] == 0){
                            count++;
                            arr[hdr->fragmanetIndex-1] =hdr->size;
                            fprintf(stderr, "%d bytes received \t", ret);
                            fprintf(stderr, "written to index %d of first arr %d\n", hdr->fragmanetIndex,hdr->size);


                        }

                        //fprintf(stderr,"received \n");
                    }
                }

            }

            if (ufds[1].revents & POLLIN) {

                ret2 = recv(sfd2, hdr2, RX_BUF_SIZE, 0);
                if (ret <= 0) {
                    fprintf(stderr, "ERROR: recv failed ret: %d, errno: %d\n", ret, errno);
                    goto bail;
                }

                if(strcmp(hdr2->name,hdr4->name) == 0 && strcmp(hdr2->surname, hdr4->surname) == 0 && strcmp(hdr2->filename, hdr4->filename) == 0 && hdr2->filesize == hdr4->filesize) {

                    int checkcrc;
                    CRCcheck = &checkcrc;
                    checkcrc = 0;

                    crc32(hdr2->data, (size_t)hdr2->size, CRCcheck);
                    //fprintf(stderr,"here\n");
                    if(*CRCcheck == hdr2->crc){

                        indexing2[hdr2->fragmanetIndex - 1] = (char *) calloc(hdr2->size,sizeof(char));
                        //indexing[hdr->fragmanetIndex - 1] = hdr->data;
                        memcpy(indexing2[hdr2->fragmanetIndex -1],hdr2->data,hdr2->size);
                        if(arr2[hdr2->fragmanetIndex-1] == 0){
                            count++;
                            arr2[hdr2->fragmanetIndex-1] =hdr->size;
                            fprintf(stderr, "%d bytes received \t", ret2);
                            fprintf(stderr, "written to index %d of second arr %d\n", hdr2->fragmanetIndex,hdr2->size);


                        }

                        //fprintf(stderr,"received \n");
                    }
                }



            }





            if(count == hdr->fragmentcount + hdr2->fragmentcount){
                done = 1;
            }





        }




        write_to_file(arr, indexing,hdr->filename, hdr->fragmentcount,arr2, indexing2,hdr2->filename, hdr2->fragmentcount);

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
