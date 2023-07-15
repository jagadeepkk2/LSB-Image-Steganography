#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;                                                        //Declaration of width and height variables
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);                                           //Point file pointer to 18th byte of image header

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);                                 //Read width of image
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);                                //Read height of image
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;                                                 //Return image size
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo -> fptr_src_image = fopen(encInfo -> src_image_fname, "r");         //Open source file image in read mode
    // Do Error handling
    if (encInfo -> fptr_src_image == NULL)                                      //Check whether the file  is opened or not
    {
        perror("fopen");                                                        //Print error if the function failed to open file 
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> src_image_fname);

        return e_failure;                                                       
    }

    // Secret file
    encInfo -> fptr_secret = fopen(encInfo -> secret_fname, "r");                //Open secret file in read mode
    // Do Error handling
    if (encInfo -> fptr_secret == NULL)                                          //Check whether the file is opened or not
    {
        perror("fopen");                                                         //Print error if the fuction failed to open file
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo -> fptr_stego_image = fopen(encInfo -> stego_image_fname, "w");       //Open output image file in write mode
    // Do Error handling
    if (encInfo -> fptr_stego_image == NULL)                                      //Check whether the file is opened or not
    {
        perror("fopen");                                                          //Print error if the function failed to open file
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}


 OperationType check_operation_type(char *argv[])                                  //Function to check 2nd argument                       
{
    if (strcmp(argv[1],"-e") == 0)                                                 //Check whether 2nd argument is -e or not
    {
        return e_encode;                                                           //Return e_encode if argument is -e
    }
    else if(strcmp(argv[1],"-d") == 0)                                             //Check whether 2nd argument is -d or not
    {
        return e_decode;                                                           //Return e_decode if argument is -d
    }
    else
    {
        return e_unsupported;                                                      //Return e_unsupported if 2nd argument is not -e or -d
    }
}


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)     //Function to read and validate arguments
{

    if(strstr(argv[2],".bmp" )!= NULL)                                      //Check whether the 3rd argument is .bmp file or not
    {
        encInfo -> src_image_fname = argv[2];                               //Save source name in encInfo structure
    }
    else
    {
        return e_failure;                                                   //Return e_failure if source file is not .bmp file
    }

    encInfo -> secret_fname = argv[3];                                      //Save secret file name in structure
   
    if (argv[4] != NULL)                                                    //Check whether 4th argument is given or not 
    {
        encInfo -> stego_image_fname = argv[4];                             //Save stego image name in structure
    }
    else
    {
        encInfo -> stego_image_fname = "stego.bmp";                         //Save default name if argument is not given
    }


    return e_success;
}

/*
 * Function to check image capacity
 * Inputs:encInfo
 * Return Value: e_success or e_failure
 */
Status check_capacity(EncodeInfo *encInfo)
{

    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);  //Store source image size in stucture

    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);            //Store secret file size in structure

    if (encInfo -> image_capacity > 54 + 16 + 32 + (8 * sizeof(encInfo->extn_secret_file)) + 32 + (8 * encInfo -> size_secret_file)) 
    {
        return e_success;                               //Return e_success if image capacity is greater that required size             
    }
    else
    {
        return e_failure;
    }
}


uint get_file_size(FILE *fptr)                          //Function to get secret file size 
{
    fseek(fptr, 0, SEEK_END);                           //Move file pointer to end of file
    return ftell(fptr);                                 //Return pointer location 
}

/*
 * Function to do encoding
 * Inputs:EncodeInfo address
 * Return Value: e_success or e_failure
 */
Status do_encoding(EncodeInfo *encInfo)
{
    int i; 
    printf("Encoding started...\n");
    if(open_files(encInfo) == e_success)
    {
        printf("Open files successful\n");
        if(check_capacity(encInfo) == e_success)
        {
            printf("Check capacity completed\n");
            if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
            {
                printf("bmp header is copied\n");
                if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)
                {
                    printf("Encoded Magic string is success\n");

                    strcpy(encInfo -> extn_secret_file, strstr(encInfo -> secret_fname, "."));

                    if (encode_secret_file_extn(encInfo -> extn_secret_file, encInfo) == e_success)
                    {

                        printf("Encoded secret file ext\n");

                        if(encode_secret_file_size(encInfo -> size_secret_file,encInfo) == e_success)
                        {

                            printf("Encoded secret file msg size\n");
                            if(encode_secret_file_data(encInfo) == e_success)
                            {

                                printf("Encoded secret file msg\n");
                                if (copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
                                {
                                    printf("Copied remaining data is success\n");
                                }
                                else
                                {
                                    printf("Failed to copy remaining data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Encoded secret file msg failed\n");
                                    return e_failure;
                            }

                        }
                        else
                        {
                            printf("Encoded secret file msg size failed\n");
                                    return e_failure;

                        }


                    }
                    else
                    {
                        printf("Secret file ext  encode failed\n");
                        return e_failure;
                    }

                    return e_success;
                }
                else
                {
                    printf("Encoded Magic string failed\n");
                    return e_failure;

                }

            }
            else
            {
                printf("bmp header is copying failed\n");
                return e_failure;

            }


        }
        else
        {
            printf("Check capacity Failed\n");
            return e_failure;

        }

    }
    else
    {
        printf("Open files failed\n");

      return e_failure;
    }
}

Status copy_bmp_header(FILE * fptr_src_image, FILE *fptr_dest_image)          //Function to copy bmp header 
{ 
    char bmpheader[54];           
    fseek(fptr_src_image, 0, SEEK_SET);                                       //Move file pointer to start of source image           
    fread(bmpheader, sizeof(char), 54, fptr_src_image);                       //Read 54 byte header from source image
    fwrite(bmpheader, sizeof(char), 54, fptr_dest_image);                     //Write 54 byte header to stego image
    return e_success;
}


Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)      //Function to encode magic string
{
    encode_data_to_image(MAGIC_STRING, strlen(MAGIC_STRING), encInfo -> fptr_src_image, encInfo -> fptr_stego_image);
    return e_success;
}



Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)   //Function to encode date to image
{
    char buff[8];                                                     
    for (int i = 0; i < size; i++)
    {
        fread(buff, sizeof(char), 8,fptr_src_image);                            //Read 8 Bytes from source image
        encode_byte_to_lsb(data[i],buff);                                       //Encode msb of secret data in lsb of source image bytes
        fwrite(buff, sizeof(char), 8,fptr_stego_image);                         //Write encoded data to stego image
    }
    return e_success;
}


Status encode_byte_to_lsb(char ch, char *image_buffer)
{
    int mask = 1 << 7;
    for (int i = 0; i < 8; i++)                                                 
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | (ch & mask) >> (7 - i);    //Encode msb of secret data in lsb of source image
        mask = mask >> 1;  
    }
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)        //Function to encode secret file extension
{
    int size = strlen(file_extn);
    char str[size * 8];
    fread(str,sizeof(char),size*8, encInfo -> fptr_src_image);                    //Read size * 8 bytes from source image
    unsigned  int mask = 1 << 31;
    for (int i = 0; i < 32; i++)           
    {
        str[i] = (str[i] & 0xFE) | ((size &mask) >> (31 - i));                    //Encode bytes to stego image
        mask = mask >> 1;  
    }
    fwrite(str,sizeof(char),32,encInfo -> fptr_stego_image);                      //write encoded bytes to stego image
    encode_data_to_image(encInfo -> extn_secret_file, strlen(file_extn), encInfo -> fptr_src_image, encInfo -> fptr_stego_image);
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)               //Function to encode secret file size
{
  
    char str[file_size * 8];
    fread(str,sizeof(char),32, encInfo -> fptr_src_image);                        //Read 32 Bytes from source image
    unsigned  int mask = 1 << 31;
    for (int i = 0; i < 32; i++)           
    {
        str[i] = (str[i] & 0xFE) | ((file_size & mask) >> (31 - i));              //Encode secret file size 
        mask = mask >> 1;  
    }
    fwrite(str,sizeof(char),32,encInfo -> fptr_stego_image);                      //Write encoded bytes to stego image
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)                                //Function to encode secret file data 
{
    fseek(encInfo -> fptr_secret, 0, SEEK_SET);                                     //Move  secret data file pointer to start
    char ch;                                                                              
    char buff[8];
    for (int i = 0; i < encInfo -> size_secret_file; i++)                                 
    {
        fread(buff, 8, 1, encInfo -> fptr_src_image);                               //Read 8 byte from source image
        fread(&ch, 1, 1, encInfo -> fptr_secret);                                   //Read 1 byte from secret file      
        encode_byte_to_lsb(ch, buff);                                               //Function call to encode bytes to lsb 
        fwrite(buff, 8, 1, encInfo -> fptr_stego_image);                            //write encoded bytes to stego image
    }
    return e_success;

}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)                      //Function to copy remaining data to stego image 
{
    char ch; 
    while (fread(&ch,1,1, fptr_src) > 0)                                             //Read remaining bytes from source image                                
    {
        fwrite(&ch,1,1,fptr_dest);                                                   //Write remaining bytes to stego image
    }
    return e_success;
}
