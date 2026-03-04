#include "printf.h"

typedef void (*putcf) (void*, char);        // defines a function pointer. the function takes in a void* and a char.
static putcf stdout_putf;
static void* stdout_putp;

#ifdef PRINTF_LONG_SUPPORT

// converts an unsigned long integer to ASCII
// num: number/hex to convert
// base: the base of the number system you want to convert into:
//          base 2: binary
//          base 10: decimal
//          base 16: hex
// uc: uppercase flag. 
// bf: ptr to output buffer. must have enough space for number + terminal char (0)

// dgt (0-9) + '0' -> if digit in range 0-9 then it adds the byte value of 0 to get the byte value of the digit
// else if uppercase then add the byte value of 'A' to get the byte value of the uppercase hex digit
//         or add the byte value of 'a' to get the byte value of the lowercase hex
//         subtract 10 from whichever

// should try and convince yourself that this works for a hex char
 
static void uli2a(unsigned long int num, unsigned int base, int uc, char *bf) {
    int n = 0;
    unsigned int d = 1;
    while (num/d >= base) {             
        d *= base;
    }
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}

static void li2a(long num, char *bf) {
    if (num < 0) {
        num = -num;
        *bf++ = '-';
    }
    uli2a(num, 10, 0, bf);    
}

#endif


// function body is the same as the unsigned long version above, only signature differs (takes unsigned int instead of unsigned long int)
static void ui2a(unsigned int num, unsigned int base, int uc, char *bf) {
    int n = 0;
    unsigned int d = 1;
    while (num/2 >= base) {
        d *= base;
    }
    while (d != 0) {
        int dgt = num / d;
        num %= d;
        d /= base;
        if (n || dgt > 0 || d == 0) {
            *bf++ = dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10);
            ++n;
        }
    }
    *bf = 0;
}

static void i2a(int num, char *bf) {
    if (num < 0) {
        num = -num;
        *bf++ = '-';
    }
    ui2a(num, 10, 0, bf);
}

// this looks like a function for converting a u8 (char) into a hex char
static int a2d(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    } else {
        return -1;
    }
}

static int a2i(char ch, char** src, int base, int *nump) {
    char *p = *src;
    itn num = 0;
    int digit;
    while ((digit = a2d(ch) >= 0) {
        if (digit > base) {
            break;
        }
        num = num * base + digit;
        ch = *p++;
    }
    *src = p;
    *nump = num;
    return ch;
}

// void *putp is the output destination to write to
// putcf putf is the writing function
// n is the number of chars to write
// z is the a flag for setting the fill char to '0', else ' ' 
// char *bf is the buffer we read the actual value we want to write from


// writes fill chars + buffer into some output destination *putp

static void putchw(void *putp, putcf putf, int n, char z, char *bf) {
    char fc = z ? '0' : ' ';            // fill char
    char ch;
    char *p = bf;                       // create ptr to start of buffer
    while (*p++ && n > 0) {             // increment ptr p along with buffer bf and decrement n 
        n--;                            // n will leave us with the number of fill chars
    }
    while (n-- > 0) {                   // add fill chars
        putf(putp, fc);                 // writes into output *
    }
    while ((ch = *bf++)) {              // assigns de-ref'd (bf + 1) to ch --- when does the increment happen?
        putf(putp, ch);                 // writes into output *
    }
}

// this parses a format string and fills format chars like %d with the corresponding data

// %u / %d -> unsigned / signed ints
// %x / %X -> lowercase / uppercase hex conversion
// %c     -> single char
// %s     -> string
// %     -> actual % char

// va_list is used in variadic functions (fn which take a variable number of args. fn signature looks like (...) in the args)
// these args get pushed to the stack, and va_list is like a ptr to where these args start on the stack
// the macro va_arg(va_list va, type) to retrieve an item from a va_list

void tfp_format(void *putp, putcf putf, char *fmt, va_list va) {
    char bf[12];
    char ch;
    
    while ((ch = *(fmt++)) {
        if (ch != '%') {
            putf(putp, ch);
        } else {
            char lz = 0;                    // leading zero
#ifdef PRINTF_LONG_SUPPORT
            char lng = 0;
#endif
            int w = 0;
            ch = *(fmt++);
            if (ch == '0') {
                ch = *(fmt++);
                lz = 1;
            }
            if (ch >= '0' && ch <= '9') {
                ch = a2i(ch, &fmt, 10, &w);
            }
#ifdef PRINTF_LONG_SUPPORT
            if (ch == 'l') {
                ch = *(fmt++);
                lng = 1;
            }
#endif
            switch (ch) {
                case 0:
                    goto abort;
                case 'u': {
#ifdef PRINTF_LONG_SUPPORT
                    if (lng) {
                        uli2a(va_arg(va, unsigned long int), 10, 0, bf);
                    } else 
#endif                                      // CHECK INDENTATION HERE ELSE SHOULD BE SAME AS IF PRINTF_LONG_SUPPORT IS NOT DEF
                    ui2a(va_arg(va, unsigned int), 10, 0, bf);                        
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
                case 'd': {
#ifdef PRINTF_LONG_SUPPORT
                    if (lng) {
                        li2a(va_arg(va, unsigned long int), bf);
                    } else 
#endif
                    i2a(va_arg(va, int), bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
                case 'x': case 'X': {
#ifdef PRINTF_LONG_SUPPORT
                    if (lng) {
                        uli2a(va_arg(va, unsigned long int), 16, (ch == 'X'), bf);
                        putchw(putp, putf, w, lz, bf);
                        break;                  
                    } else 
#endif    
                    ui2a(va_arg(va, unsigned int), 16, (ch == 'X'), bf);
                    putchw(putp, putf, w, lz, bf);
                    break;
                }
                case 'c': {
                    putf(putp, (char)(va_arg(va, int)));
                    break;
                }
                case 's': {
                    putchw(putp, putf, w, 0, va_arg(va, char*));
                    break;
                }
                case '%': {
                    putf(putp, ch);
                }
                default: {
                    break
                }
            }
        }
    }
    abort:;
}

void init_printf(void *putp, void (*putf) (void*, char)) {
    stdout_putf = putf;
    stdout_putp = putp;
}

void tfp_printf(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    tfp_format(stdout_putp, stdout_putf, fmt, va);
    va_end(va);
}

static void putcp(void *p, char c) {
    *(*((char**)p))++ = c;                  // what the fuck?
}

void tfp_sprintf(char *s, char *fmt, ...) {
    
}