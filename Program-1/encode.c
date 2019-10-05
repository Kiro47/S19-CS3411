/*
 * Returns the absolute value of a signed integer
 *
 * int value: signed int to get absolute of
 *
 * Returns:
 *  int: absolute of value
 */
int abs(int value)
{
    // value shifted right against mask bit left one
    return (1 - ((( value >> 31) & 0x1) << 1)) * value; // NOLINT(hicpp-signed-bitwise)
}

/*
 * Pads a given zero initialized buffer, with spaces for all
 * non-replaced characters
 *
 * char *buf: Buffer char array to pad
 * char int:  length of the buffer
 */
void spacePad(char *buf, int length)
{
    int i;
    if (length == 0)
    {
        // You're passing a single '\0' in
        // watcha doing there bud?
        return;
    }
    // 2 As offset, 1 for 0 index, and 1 for null terminator
    for (i = 0; i < length-2; i++)
    {
        if (buf[i] == '\0')
        {
            buf[i] = ' ';
        }
    }
}

/*
 * Clears a given buffer by setting the
 * entire contents of a given buffer to '\0'
 *
 * char *buf:   buffer to be clear
 * int length:  length of the buffer
 */
void blank(char *buf, int length)
{
    int i;
    for (i = 0; i < (length ); i++)
    {
        buf[i] = '\0';
    }
}

/*
 * This procedure should encode the given binary number in value into the buffer buf right-justified as an ASCII encoded decimal number. The buffer can be assumed to have at least length bytes of space. Any bytes to the left of the number must be blank (i.e., ASCII space character, 0x20).
 */
void int_to_ascii(char *buf, int value, int length)
{
    char *writebuf;
    int base;
    int boundsOffset;
    int remainder;
    int whole;

    // Can't trust users with this lib apparently
    blank(buf, length);

    // We're exclusively working in ASCII, hard setting
    base = 10;
    // 2 As offset, 1 for 0 index, and 1 for null terminator
    boundsOffset = 2;
    writebuf = buf + length - boundsOffset;

    buf[length - 1] = '\0';

    // Single zero edge case
    if (value == 0)
    {
        buf[length-boundsOffset] = '0';
        spacePad(buf, length);
        return;
    }
    whole = abs(value);

    do
    {
        if (whole == 0)
        {
            break;
        }
        remainder = whole % base;
        whole   /= base;
        *(writebuf)-- = (remainder % base) + '0'; // NOLINT(cppcoreguidelines-narrowing-conversions)
    } while (remainder != 0);

    if (value < 0)
    {
        *(writebuf) = '-';
    }

    spacePad(buf,length);
}

/* This procedure should encode the given binary floating point number in value (IEEE 754) into the buffer buf right-justified. The buffer can be assumed to have at least length bytes of space. Any bytes to the left of the number must be blank. The desired format is nnnn.mm (i.e., two digits after the decimal point, as many digits as needed to the left of the decimal point). You may assume that length will be sufficient to hold the encoded number.
 */
void int_to_ascii_hex(char *buf, int value, int length)
{
    char *writebuf;
    int base;
    int boundsOffset;
    int remainder;
    int whole;

    // Can't trust users with this lib apparently
    blank(buf, length);

    // We're exclusively working in Hex, hard setting
    base = 16;
    // 2 As offset, 1 for 0 index, and 1 for null terminator
    boundsOffset = 2;
    writebuf = buf + length - boundsOffset;

    buf[length - 1] = '\0';

    // Single zero edge case
    if (value == 0)
    {
        buf[length-boundsOffset] = '0';
        spacePad(buf, length);
        return;
    }
    whole = abs(value);

    do
    {
        if (whole == 0)
        {
            break;
        }
        remainder = whole % base;
        whole   /= base;
        // Ints 0-9
        if (remainder < 10)
        {
            // I could just hard set '0' as INT, but that isn't portable
            *(writebuf)-- = (remainder % base) + '0'; // NOLINT(cppcoreguidelines-narrowing-conversions)
        }
        // Ascii char a-f
        else
        {
            remainder %= 10;
            // I could just hard set 'A' as INT, but that isn't portable
            *(writebuf)-- = (remainder % base) + 'A'; // NOLINT(cppcoreguidelines-narrowing-conversions)
        }
    } while (remainder != 0);

    if (value < 0)
    {
        *(writebuf) = '-';
    }

    spacePad(buf,length);
}

/*
 * This procedure should return a packed binary coded decimal representation of the given binary number. Assume that a sign digit of 1100 (hex C) for positive (+) and a 1101 (hex D) for negative is stored in the lower nibble of the rightmost byte. The largest packed decimal number which can be stored into a 32 bit value is +9999999 and the smallest is -9999999.
 */
int int_to_bcd(int value)
{
    int segments;
    int originalValue;
    int returnValue;
    int remainder;
    int whole;
    int currentSegment;
    int base;
    int shiftedRemainder;

    // Number of shifts needed for up to 9 rep
    // * 2 is due to looking at nibble components
    segments = (sizeof(int) / sizeof(char)) * 2;
    // Store originalValue
    originalValue = value;
    // Dealing with digits
    base = 10;
    // Init
    currentSegment = 0;
    returnValue = 0;
    // Single zero edge case
    if (value == 0)
    {
        return 0;
    }

    whole = abs(value);

    if (value > 0)
    {
        // 1100 nibble, pos
        returnValue |= (0xc << (currentSegment++ * 4));
    }
    else
    {
        // 1101 nibble, neg
        returnValue |= (0xd << (currentSegment++ * 4));
    }

    // If impossible value, return
    // MAGIC_NUMBER: Defined in assignment
    // logically it's because for x64 we only
    // have 8 nibbles to work with, one of which
    // is signed nibble.  Leaving only 7 nibbles
    // of digits to work with.
    if (whole > 9999999)
    {
        return value;
    }

    do
    {
        /*
        if (whole == 0)
        {
            break;
        }
        */
        remainder = whole % base;
        whole   /= base;
        // Ints 0-9
        // write nibble
        //returnValue = returnValue & ((value >> (currentSegment * 4) & 0xf) << (currentSegment++ * 4));
        shiftedRemainder = remainder << (currentSegment++ * 4);
        returnValue |= shiftedRemainder;
    } while (remainder != 0);

    return returnValue;
}

/* This procedure should encode the given binary floating point number in value (IEEE 754) into the buffer buf right-justified. The buffer can be assumed to have at least length bytes of space. Any bytes to the left of the number must be blank. The desired format is nnnn.mm (i.e., two digits after the decimal point, as many digits as needed to the left of the decimal point). You may assume that length will be sufficient to hold the encoded number.
 */
void float_to_ascii(char *buf, float value, int length)
{
    char *writebuf;
    char *writebufPlaceholder;
    int base;
    int boundsOffset;
    float remainder;
    int whole;
    int precision;
    int decimalMod;
    float precision_round;
    float absolute;

    // Predefined to always have at least two rem
    precision = 2;
    precision_round = 0.005;

    // standard dec
    base = 10;

    // Assign temp number
    absolute = value;

    // Bounds offset, 1 for index, 1 for null terminator
    boundsOffset = 2;
    writebuf = buf + length - boundsOffset - boundsOffset ;

    //Can't trust users of this lib
    blank(buf, length);

    buf[length - 1] = '\0';

    // Welp, my abs func doesn't handle floats.
    if (value < 0)
    {
        absolute = -value;
    }
    // Now we're rounding at
    absolute += precision_round;

    // Force int casting for whole numbers
    whole = (int) absolute;
    // Gather remainder
    remainder = absolute - whole;

    // Store placeholder
    writebufPlaceholder = writebuf;
    // Jumping by precision, otherwise they'll appear backwards
    (writebuf) += precision -1;

    // Decimal range
    // Conv remainder into whole, decided by precision * base
    //remainder *= (base * precision);
    while (precision--)
    {
        /*
        decimalMod = ((int) remainder) % base;
        remainder /= base;
        *(writebuf)-- = '0' + decimalMod;
        */

        // Handle decimal number
        remainder *= base;
        // write char in, and force cast to int
        decimalMod = (int) remainder;
        *(writebuf)++ = '0' + decimalMod;
        remainder -= decimalMod;
    }
    writebuf = writebufPlaceholder;


    // Handle decimal point switch
    *(writebuf)-- = '.';

    // Check if whole is 0, else
    if (whole == 0)
    {
        *(writebuf)-- = '0';
    }
    else
    {
        // whole isn't 0,
        while (whole)
        {
            // I could just hard set '0' as INT but not as portable
            *(writebuf)-- = '0' + whole % base;
            whole /= base;
        }
    }
    // Handle negatives
    if (value < 0)
    {
        *(writebuf) = '-';
    }
    spacePad(buf, length);
}

/*
 * Strips leading 0's from buf ignoring
 * the first char for signage. Also moves
 * the sign in infront of the stripped number
 *
 * buf: Buf to strip from
 * length: length of buf
 */
void zeroStrip(char* buf, int length)
{
    int i;
    for (i = 1; i < length; i++)
    {
        if (buf[i] != '0')
        {
            // Insert sign at previous position
            buf[(i--) - 1] = buf[0];
            break;
        }
    }
    while (i--)
    {
        // Replace all previous zeros with nullTerms
        buf[i] = '\0';
    }

}

/*
 * This procedure should store the given packed BCD number in value into the buffer buf right-justified as an ascii encoded decimal number. The buffer can be assumed to have exactly 9 bytes of space. Any bytes to the left of the number must be blank.
 */
void bcd_to_ascii(char *buf, int value)
{
    char *writebuf;
    int base;
    int boundsOffset;
    int nibble;
    int nibbleSelector;
    int currentSegment;
    int totalSegments;
    int isNegative;
    int i;
    int length;

    // MAGIC_NUMBER: Defined by assignment
    // Logically due to there being a char per nibble
    // plus one for null temrminator
    length = 9;
    // Can't trust users to use this lib properly
    blank(buf,length);

    // Exclusively working with ascii, hard setting
    base = 10;

    // 2 as offset, 1 for 0 index, 1 for null terminator
    boundsOffset = 2;

    // Init
    currentSegment = 0;
    nibble = 0;
    nibbleSelector = 0;

    writebuf = buf + length - boundsOffset;

    buf[length - 1] = '\0';

    // Number of shifts needed to output all nibbles of an int
    // * 2 is translation from bytes to nibbles
    totalSegments = (sizeof(int) / sizeof(char)) * 2;
    // defaults to no
    isNegative = 0;

    // Single zero edge case
    if (value == 0)
    {

        buf[length-boundsOffset] = '0';
        spacePad(buf, length);
        return;
    }

    // Grab signed nibble
    nibbleSelector = 0xf << (currentSegment * 4);
    nibble = 0xf & (nibbleSelector & value) >> (currentSegment++ * 4);
    // Grab sign
    if (nibble == 0xd)
    {
        isNegative = 1;
    }
    // Now actually go and grab the numbers
    while (currentSegment < totalSegments)
    {
        // Reset nibble
        nibbleSelector = 0;
        // Select nibble to grab
        nibbleSelector = 0xf << (currentSegment * 4);
        nibble = 0xf & (value & nibbleSelector) >> (currentSegment++ * 4);
        // Could hard set '0' as INT, but not as portable
        *(writebuf)-- = '0' + nibble;
    }

    // Apply neg/pos sign
    isNegative ? (*(writebuf) = '-') : (*(writebuf) = '+');
    zeroStrip(buf,length);
    spacePad(buf, length);
}

