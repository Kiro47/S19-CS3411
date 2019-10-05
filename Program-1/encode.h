#ifndef PROG1_ENCODE_H
#define PROG1_ENCODE_H
/*
 * This procedure should encode the given binary number in value into the buffer buf right-justified as an ASCII encoded decimal number. The buffer can be assumed to have at least length bytes of space. Any bytes to the left of the number must be blank (i.e., ASCII space character, 0x20).
 */
void int_to_ascii(char *buf, int value, int length);

/**
 * This procedure should store the given binary number in value into the buffer buf right-justified as an ASCII encoded hexadecimal number. The buffer can be assumed to have at least length bytes of space. Any bytes to the left of the hexadecimal number must be blank.
 */
void int_to_ascii_hex(char *buf, int value, int length);

/* This procedure should encode the given binary floating point number in value (IEEE 754) into the buffer buf right-justified. The buffer can be assumed to have at least length bytes of space. Any bytes to the left of the number must be blank. The desired format is nnnn.mm (i.e., two digits after the decimal point, as many digits as needed to the left of the decimal point). You may assume that length will be sufficient to hold the encoded number.
 */
void float_to_ascii(char *buf, float value, int length);

/*
 * This procedure should return a packed binary coded decimal representation of the given binary number. Assume that a sign digit of 1100 (hex C) for positive (+) and a 1101 (hex D) for negative is stored in the lower nibble of the rightmost byte. The largest packed decimal number which can be stored into a 32 bit value is +9999999 and the smallest is -9999999.
 */
int int_to_bcd(int value);

/*
 * This procedure should store the given packed BCD number in value into the buffer buf right-justified as an ascii encoded decimal number. The buffer can be assumed to have exactly 9 bytes of space. Any bytes to the left of the number must be blank.
 */
void bcd_to_ascii(char *buf, int value);

#endif
