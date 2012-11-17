#include <stdio.h>
#include <string.h>

void ccsds_generate_sequence(char * sequence, int length)
{
    char x[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    int i;

    /* Generate the sequence */	
    memset(sequence, 0, length);

    /* The pseudo random sequence shall be generated using the polynomial
     * h(x) = x8 + x7 + x5 + x3 + 1 */
    for (i = 0; i < length*8; i++) {
        sequence[i/8] = sequence[i/8] | x[1] << 7 >> (i%8);
        x[0] = (x[8] + x[6] + x[4] + x[1]) % 2;
        x[1] = x[2];
        x[2] = x[3];
        x[3] = x[4];
        x[4] = x[5];
        x[5] = x[6];
        x[6] = x[7];
        x[7] = x[8];
        x[8] = x[0];
    }
}

void ccsds_xor_sequence(unsigned char * data, char * sequence, int length)
{
    int i;

    for (i = 0; i < length; i++)
        data[i] ^= sequence[i];
}

