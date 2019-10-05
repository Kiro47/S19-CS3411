#include "unistd.h"
#include "encode.h"
#include "stdio.h"

// Copied in because I wasn't aware if I could modify encode.h
// and it was getting rather late.
/*
 * Clears a given buffer by setting the
 * entire contents of a given buffer to '\0'
 *
 * char *buf:   buffer to be clear
 * int length:  length of the buffer
 */
void blank_buf(char *buf, int length)
{
    return;
    int i;
    for (i = 0; i < (length - 1); i++)
    {
        buf[i] = '\0';
    }
}

/*
 * Prints the binary representation of an integer
 *
 * int v: Integer to be printed in binary form
 *
 */
void bin_print(int v)
{
    unsigned int mask=1<<((sizeof(int)<<3)-1);
    while(mask) {
        printf("%d", (v&mask ? 1 : 0));
        mask >>= 1;
    }
    printf("\n");
}



void gTest()
{
	char buf[100];
    int v;
    float f;

//    buf = calloc(100, sizeof(char));

    write(1, "Enter a number (7 digits max) : ", 32);
    scanf("%d", &v);

    int_to_ascii(buf, v, 8);

    write(1, buf, 8);
    write(1, "\\n", 1);

    write(1, "Enter a number (no maximum) : ", 30);
    scanf("%d", &v);

    int_to_ascii_hex(buf, v, 8);

    write(1, buf, 8);
    write(1, "\\n", 1);

    write(1, "Enter a float number (7 digits max) : ", 37);
    scanf("%f", &f);

    float_to_ascii(buf, v, 8);

    write(1, buf, 8);
    write(1, "\\n", 1);

    write(1, "Enter a number (7 digits max) : ", 32);
    scanf("%d", &v);

    v = int_to_bcd(v);
    int_to_ascii_hex(buf, v, 8);

    write(1, buf, 8);
    write(1, "\\n", 1);

    bcd_to_ascii(buf, v);

    write(1, buf, 8);
    write(1, "\\n", 1);
//    free(buf);
}


void jTest()
{
	char buf[100];
    int bcd;
//    buf = calloc(100, sizeof(char));

    printf("\n\n\n");
	printf("int_to_ascii\n");

	int_to_ascii(buf, 1, 2);
    printf("Expecting: %d\n Received: ", 1);
	puts(buf);
    blank_buf(buf, 100);

	int_to_ascii(buf, -1, 3);
    printf("Expecting: %d\n Received: ", -1);
	puts(buf);
    blank_buf(buf, 100);

	int_to_ascii(buf, 0, 2);
    printf("Expecting: %d\n Received: ", 0);
	puts(buf);
    blank_buf(buf, 100);

    int_to_ascii(buf, 25, 3);
    printf("Expecting: %d\n Received: ", 25);
    puts(buf);
    blank_buf(buf, 100);

	int_to_ascii(buf, 2147483647, 11);
    printf("Expecting: %d\n Received: ", 2147483647);;
	puts(buf);
    blank_buf(buf, 100);

	int_to_ascii(buf, -2147483647, 12);
    printf("Expecting: %d\n Received: ", -2147483647);
	puts(buf);
    blank_buf(buf, 100);

    printf("\n\n\n");
    printf("int_to_ascii_hex\n");
    int_to_ascii_hex(buf, 1, 2);
    printf("Expecting: %s\n Received: ", "1");
    puts(buf);
    blank_buf(buf, 100);

    int_to_ascii_hex(buf, 10, 2);
    printf("Expecting: %s\n Received: ", "A");
    puts(buf);
    blank_buf(buf, 100);

    int_to_ascii_hex(buf, 12, 2);
    printf("Expecting: %s\n Received: ", "C");
    puts(buf);
    blank_buf(buf, 100);

    int_to_ascii_hex(buf, 31, 3);
    printf("Expecting: %s\n Received: ", "1F");
    puts(buf);
    blank_buf(buf, 100);

    int_to_ascii_hex(buf, 1111, 4);
    printf("Expecting: %s\n Received: ", "457");
    puts(buf);
    blank_buf(buf, 100);

    printf("\n\n\n");
    printf("int_to_bcd() & bcd_to_ascii\n");

    bcd = int_to_bcd(1);
    bin_print(bcd);
    printf("1 (28): %d\n", bcd); // 0000000000000000000000000001 + 1100
    blank_buf(buf,100);
    bcd_to_ascii(buf, bcd);
    printf("bcd_to_ascii(%d) => %s\n", bcd, buf);


    bcd = int_to_bcd(-1);
    bin_print(bcd);
    printf("-1 (29): %d\n", bcd); // 0000000000000000000000000001 + 1101
    blank_buf(buf,100);
    bcd_to_ascii(buf, bcd);
    printf("bcd_to_ascii(%d) => %s\n", bcd, buf);


    bcd = int_to_bcd(0);
    bin_print(bcd);
    printf("0 (0): %d\n", bcd); // 0 hard
    blank_buf(buf,100);
    bcd_to_ascii(buf, bcd);
    printf("bcd_to_ascii(%d) => %s\n", bcd, buf);


    bcd = int_to_bcd(9999999);
    bin_print(bcd);
    printf("9999999 (-1717986916): %d\n", bcd); // 100110001001011001111111 + 1100
    blank_buf(buf,100);
    bcd_to_ascii(buf, bcd);
    printf("bcd_to_ascii(%d) => %s\n", bcd, buf);


    bcd = int_to_bcd(-9999999);
    bin_print(bcd);
    printf("-9999999 (-1717986915): %d\n", bcd); // 100110001001011001111111 + 1101
    blank_buf(buf,100);
    bcd_to_ascii(buf, bcd);
    printf("bcd_to_ascii(%d) => %s\n", bcd, buf);


    bcd = int_to_bcd(1);
    bin_print(bcd);
    printf("1 (1): %d\n", bcd);
    blank_buf(buf,100);
    bcd_to_ascii(buf,bcd);
    printf("bcd_to_ascii(%d) => %s\n", bcd, buf);


    blank_buf(buf,100);
    printf("\n\n\n");
    printf("float_to_ascii");

    float_to_ascii(buf, 1.00f, 5);
    printf("Input: 1.00f, Results:\n");
    printf("   1.00 => %s\n", buf);
    blank_buf(buf, 100);

    float_to_ascii(buf, -1.00f, 6);
    printf("Input: -1.00f, Results:\n");
    printf("  -1.00 => %s\n", buf);
    blank_buf(buf, 100);

    float_to_ascii(buf, 0.00f, 5);
    printf("Input: 0.00f, Results:\n");
    printf("   0.00 => %s\n", buf);
    blank_buf(buf, 100);

    float_to_ascii(buf, 123.45f, 7);
    printf("Input: 123.45f, Results:\n");
    printf(" 123.45 => %s\n", buf);
    blank_buf(buf, 100);

    float_to_ascii(buf, -123.45, 8);
    printf("Input: -123.45f, Results:\n");
    printf("-123.45 => %s\n", buf);
    blank_buf(buf, 100);

    float_to_ascii(buf, 12.345, 6);
    printf("Input: 12.345f, Results:\n");
    printf("  12.35 => %s\n", buf);
    blank_buf(buf, 100);

    float_to_ascii(buf, -12.345, 7);
    printf("Input: -12.345f, Results:\n");
    printf(" -12.35 => %s\n", buf);
    blank_buf(buf, 100);

    float_to_ascii(buf, -12.341, 7);
    printf("Input: -12.341f, Results:\n");
    printf(" -12.34 => %s\n", buf);
    blank_buf(buf, 100);

//	free(buf);
}

int main()
{
    //gTest();
	jTest();
	return 0;
}
