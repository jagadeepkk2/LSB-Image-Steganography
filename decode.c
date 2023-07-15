#include <stdio.h>
#include "decode.h"
#include "types.h"
#include<string.h>
#include "common.h"

Status open_decode_files(DecodeInfo *decInfo)                                            //Function to open decode files
{
    decInfo -> fptr_stego_image = fopen(decInfo -> stego_image_fname, "r");              //Open stego image in read mode    
    if (decInfo -> fptr_stego_image == NULL)                              
    {
        perror("fopen");                           
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo -> stego_image_fname);

        return e_failure;
    }

    //    decInfo->fptr_decoded_file = fopen(decInfo -> decoded_fname, "w");               

    if (decInfo -> fptr_decoded_file == NULL)              
    {
        perror("fopen");                     
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo -> decoded_fname);

        //        return e_failure;
    }
    return e_success;
}



Status do_decoding(char *argv[],DecodeInfo *decInfo)                           //Function to decode stego image 
{

    printf("Decoding started...\n");
    if (open_decode_files(decInfo) == e_success) 
    {
        printf("Decode file open Successful\n");
        if (decode_magic_string(MAGIC_STRING, decInfo) == e_success)
        {
            printf("Decode magic string Successful-%s\n",decInfo->decode_data);

            if (decode_secret_file_extn_size(decInfo) == e_success)
            {
                printf("secret file extn size decode success\n");
                if(decode_secret_file_extn(decInfo -> image_data_size, decInfo) == e_success)
                {
                    printf("Decode file extn success-%s\n",decInfo->extn_decoded_file);
                    if (argv[3] == NULL)
                    {
                        char sec[30]="secret";                                           
                        strcat(sec,decInfo->extn_decoded_file);                     
                        decInfo -> decoded_fname=sec;                 //Store default name for secret file is 4th argument is not passed
                        decInfo->fptr_decoded_file = fopen(decInfo -> decoded_fname, "w");  
                    }
                    else
                    {
                        strcat(argv[3],decInfo->extn_decoded_file);
                        decInfo -> decoded_fname=argv[3];              
                        decInfo->fptr_decoded_file = fopen(decInfo -> decoded_fname, "w");  


                    }                  
                    if (decode_secret_file_size(decInfo) == e_success)
                    {
                        printf("decode secret file size success\n");
                        if (decode_secret_file_data(decInfo) == e_success)
                        {
                            printf("decode secret file  successful\n");
                        }
                        else
                        {
                            printf("decode secret file  unsuccessful\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("decode secret file size unsuccessful\n");
                        return e_failure;

                    }
                }
                else
                {
                    printf("Decode file extn Unsuccessful\n");
                    return e_failure;
                }
            }
            else
            {
                printf("secret file extn size decode failed\n");
                return e_failure;
            }
        }
        else
        {
            printf("Decode magic string Unsuccessful\n");
            return e_failure;

        }

    }
    else
    {
        printf("Decode file open Unsuccessful\n");
        return e_failure;
    }
    return e_success;
}


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)             //Function to read and validate arguments
{

    if(strstr(argv[2],".bmp")!=NULL)                                                //Check whether the argument is .bmp or not
    {
        decInfo -> stego_image_fname =argv[2];                                      //Store stego image name
    }
    else
    {
        return e_failure;
    }
    if (argv[3] != NULL)                                                            //Check whether 4th argument passed or not
    {         
    //    decInfo -> decoded_fname = argv[3];                                         //store decoded file name in structure
    //    decInfo->fptr_decoded_file = fopen(decInfo -> decoded_fname, "w");               
    }


    return e_success;
}

Status decode_magic_string(const char *magic_string,DecodeInfo *decInfo)                 //Function to decode magic string
{

    fseek(decInfo->fptr_stego_image,54, SEEK_SET);                                       //Move to 54th byte of stego image header
    fread(decInfo->image_data, sizeof(char), 8, decInfo->fptr_stego_image);              //Read 8 bytes from stego image 
    if (decode_byte_from_lsb(decInfo->decode_data, decInfo->image_data) == e_success)    //Decode byte from lsb 
    {
        if (decInfo->decode_data[0] == magic_string[0])                //Check whether the decoded data matching with magic string or not
        {

            fread(decInfo->image_data, sizeof(char), 8, decInfo->fptr_stego_image);              //Read 8 bytes from stego image   
            if (decode_byte_from_lsb(decInfo->decode_data, decInfo->image_data) == e_success)    //Decode byte from lsb
            {

                if (decInfo->decode_data[0] == magic_string[1]);        //Check whether the decoded data matching with magic string or not 
                else
                    return e_failure;
            }
            printf("Decoded data of magic string matched\n");
            return e_success;
        }
        else
        {
            fprintf(stderr,"Error: Decoded data of magic string not matched\n");
            return e_failure;
        }
    }
    else
    {
        printf("decode byte from lsb failed\n");
        return e_failure;

    }


    return e_success;
}

Status decode_byte_from_lsb(char *decode_data, char *image_data)             //Function to decode byte from lsb
{
    for (int i = 0; i < 8; i++)                  
    { 
        decode_data[0] <<= 1;          
        decode_data[0] |=  (image_data[i] & 0x01);                          //decode byte and store in decode_data
    }
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)                       //Function to decode secret file extension size
{
    char str[32];                                    
    fread(str, sizeof(char),32, decInfo->fptr_stego_image);                    //Read 32 bytes from stego image
    decode_size_from_lsb(str, decInfo);                                        //Decode secret file extension size
    return e_success;
}

Status decode_size_from_lsb(char *buffer, DecodeInfo *decInfo)                 //Function to decode secret file extension size from lsb
{
    for (int i = 0; i < 32; i++)             
    {

        decInfo->image_data_size <<= 1;                                        
        decInfo->image_data_size |= (buffer[i] & 0x01);                        //Store decoded size in structure
    }
    return e_success;
}

Status decode_secret_file_extn(uint extn_size, DecodeInfo *decInfo)            //Function to decode secret file extension
{
    int i;
    for ( i = 0; i < extn_size; i++)                                           //Run loop till extension size
    {
        fread(decInfo->image_data, sizeof(char),8, decInfo->fptr_stego_image);  //Read 8 Byte from stego image
        if (decode_byte_from_lsb(decInfo->decode_data, decInfo->image_data) == e_success)    //decode byte from lsb
        {
            decInfo->extn_decoded_file[i] = decInfo->decode_data[0];            //Store decoded extension in structure
        }    
        else
            return e_failure;
    }
    decInfo->extn_decoded_file[i] ='\0';                                         
    printf("Decoded secret file extension matched\n");
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)                                      //Function to decode secret file size
{
    char str[32];                
    fread(str, sizeof(char),32, decInfo->fptr_stego_image);                             //Read 32 byte from stego image
    decode_size_from_lsb(str, decInfo);                                                 //Decode secret file size 
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)                                       //Function to decode secret file data
{

    for (int i = 0; i < decInfo->image_data_size; i++)            
    {
        fread(decInfo->image_data, sizeof(char),8, decInfo->fptr_stego_image);            //Read 8 bytes from stego image

        if(decode_byte_from_lsb(decInfo->decode_data, decInfo->image_data) == e_success)  //decode byte from lsb
        {
            fwrite(decInfo->decode_data, sizeof(char), 1, decInfo->fptr_decoded_file);    //write  data  decoded to decoded file          
        }
        else
            return e_failure;
    }
    return e_success;
}

