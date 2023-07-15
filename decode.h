#ifndef DECODE_H
#define DECODE_H

#include "types.h"   
#include "common.h" 

typedef struct  _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    uint image_data_size;
    char image_data[8];

    /*  Decoded file Info */
    char *decoded_fname;
    FILE *fptr_decoded_file;
    char extn_decoded_file[4];
    char decode_data[1];
}	DecodeInfo;



Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

Status do_decoding(char *argv[],DecodeInfo *decInfo);

Status open_decode_files(DecodeInfo *decInfo);

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

Status decode_secret_file_extn_size(DecodeInfo *decInfo);

Status decode_secret_file_extn(uint extn_size, DecodeInfo *decInfo);

Status decode_secret_file_size(DecodeInfo *decInfo);

Status decode_secret_file_data(DecodeInfo *decInfo);

Status decode_size_from_lsb(char *buffer, DecodeInfo *decInfo);

Status decode_byte_from_lsb(char *decode_data, char *image_data);


#endif
