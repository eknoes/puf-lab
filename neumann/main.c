#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IN_LEN 4

#define LEFT_MASK 0x80
#define LEFT_PAIR 0xc0
#define RIGHT_PAIR 0x30


int main() {

    unsigned char *output, *input;
    int i, bitpair, byte_pos, copied_bits, output_pos, output_size;
    unsigned char curr_pair;

    input = malloc(IN_LEN);
    strcpy(input, "\x6c\x6c\x6c\x6c");
 
    printf("Input: 0x");   
    for(i = 0; i < IN_LEN; i++) {
        printf("%x", *(input + i));
    }
    printf("\n");
    
    output_size = 1;
    output_pos = 0;
    copied_bits = 0;
    output = malloc(1);

    for(i = 0; i < IN_LEN; i++) {
        
        for( bitpair = 0; bitpair < 4; bitpair++) {
            printf("Next pair\n");
            // Copy current byte, shift to current pair, mask
            curr_pair = *(input + i);
            printf("%x\n", curr_pair);
            curr_pair = curr_pair << (bitpair * 2);
            printf("%x\n", curr_pair);
            curr_pair = curr_pair & LEFT_PAIR;
            printf("%x\n", curr_pair);

            // Copy if not 00 or 01
            if ( !(curr_pair == 0xc0 || curr_pair == 0x00 ) ) {
                printf("COPY: 0x%x\n", curr_pair);


                *(output + output_pos) = *(output + output_pos) | (curr_pair >> 7);
                
                // If 8 bits copied, move on, otherwise shift
                if(++copied_bits % 8 == 0) {
                    output_pos++;
                
                    if(output_size < (output_pos)) {
                        output = realloc(output, output_pos);
                        output_size = output_pos;
                        
                        if(output == NULL) {
                            printf("Could not realloc space");
                            return 0;
                        }
                    }

                    *(output+output_pos) = 0x00;
                } else {
                    *(output + output_pos) = *(output + output_pos) << 1;
                }
            } else {
                printf("DISCARD: 0x%x\n", curr_pair);
            }
        }
    }
    
    printf("Output (Copied %i bits): 0x", copied_bits);   
    for(i = 0; i < copied_bits / 8; i++) {
        printf("%x", *(output + i));
    }
    printf("\n");

}
