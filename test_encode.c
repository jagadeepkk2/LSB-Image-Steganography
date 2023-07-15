#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "decode.h"
int main(int argc, char **argv)
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    uint img_size;

    if (argc < 3 || argc > 5)                                                   //Check argument count 
    {
        printf("Error:Please pass 4 arguments\n");
        return 1;
    }
    else
    {

        if(check_operation_type(argv)==e_encode)                                //Check operation type is encode or not
        {

            if (read_and_validate_encode_args(argv, &encInfo) == e_success)     //Read and validate encode arguments
            {

                printf("Validation successful\n");
               if(do_encoding(&encInfo)==e_success)                             //Call encoding function
                {
                    printf("Encoding successful\n");
                }
                else
                {
                    printf("Encoding unsuccessful\n");
                }

            }
           else 
            {
                printf("Validation failed\n");
                return 1;
            }
            
        }
        else if(check_operation_type(argv)==e_decode)                          //Check operation type is decode or not
        {
            if (read_and_validate_decode_args(argv, &decInfo) == e_success)    //Read and validate decode arguments
            {

                if(do_decoding(argv,&decInfo)==e_success)                      //Call decoding function
                {
                    printf("Decoding successful\n");

                }
                else
                {
                    printf("Decoding unsuccessful\n");

                }

            }
            else
            {
                printf("Decode Validation  failed\n");
                return 1;
            }
        }
        else
        {
            printf("Error:Please enter correct option\n");
            return 1;
        }
    }

    return 0;
}
