#ifndef _RANDOMIZER_H_
#define _RANDOMIZER_H_

void ccsds_generate_sequence(char * sequence, int length);
void ccsds_xor_sequence(unsigned char * data, char * sequence, int length);

#endif /* _RANDOMIZER_H_ */
