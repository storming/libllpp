#include <cmath>
#include <cstring>
#include <cctype>
#include <cstdint>
#include <cinttypes>

#include "printf.h"

#define INT64_T_FMT PRId64
#define UINT64_T_FMT PRIu64

typedef enum {
    NO = 0, YES = 1
} boolean_e;

static const char __null_string[] = "(null)";
#define S_NULL ((char *)__null_string)
#define S_NULL_LEN 6

#define FLOAT_DIGITS 6
#define EXPONENT_LENGTH 10

#define NUM_BUF_SIZE 512

#define NDIG 80

static char *__cvt(double arg, int ndigits, int *decpt, int *sign, int eflag, char *buf)
{
    register int r2;
    double fi, fj;
    register char *p, *p1;

    if (ndigits >= NDIG - 1) {
        ndigits = NDIG - 2;
    }

    r2 = 0;
    *sign = 0;
    p = &buf[0];
    if (arg < 0) {
        *sign = 1;
        arg = -arg;
    }
    arg = modf(arg, &fi);
    p1 = &buf[NDIG];
    /*
     * Do integer part
     */
    if (fi != 0) {
        p1 = &buf[NDIG];
        while (p1 > &buf[0] && fi != 0) {
            fj = modf(fi / 10, &fi);
            *--p1 = (int) ((fj + .03) * 10) + '0';
            r2++;
        }
        while (p1 < &buf[NDIG])
            *p++ = *p1++;
    } else if (arg > 0) {
        while ((fj = arg * 10) < 1) {
            arg = fj;
            r2--;
        }
    }
    p1 = &buf[ndigits];
    if (eflag == 0)
        p1 += r2;
    if (p1 < &buf[0]) {
        *decpt = -ndigits;
        buf[0] = '\0';
        return(buf);
    }
    *decpt = r2;
    while (p <= p1 && p < &buf[NDIG]) {
        arg *= 10;
        arg = modf(arg, &fj);
        *p++ = (int) fj + '0';
    }
    if (p1 >= &buf[NDIG]) {
        buf[NDIG - 1] = '\0';
        return(buf);
    }
    p = p1;
    *p1 += 5;
    while (*p1 > '9') {
        *p1 = '0';
        if (p1 > buf)
            ++ * --p1;
        else {
            *p1 = '1';
            (*decpt)++;
            if (eflag == 0) {
                if (p > buf)
                    *p = '0';
                p++;
            }
        }
    }
    *p = '\0';
    return(buf);
}

static char *__ecvt(double arg, int ndigits, int *decpt, int *sign, char *buf)
{
    return(__cvt(arg, ndigits, decpt, sign, 1, buf));
}

static char *__fcvt(double arg, int ndigits, int *decpt, int *sign, char *buf)
{
    return(__cvt(arg, ndigits, decpt, sign, 0, buf));
}

static char *__gcvt(double number, int ndigit, char *buf, int altform)
{
    int sign, decpt;
    register char *p1, *p2;
    register int i;
    char buf1[NDIG];

    p1 = __ecvt(number, ndigit, &decpt, &sign, buf1);
    p2 = buf;
    if (sign)
        *p2++ = '-';
    for (i = ndigit - 1; i > 0 && p1[i] == '0'; i--)
        ndigit--;
    if ((decpt >= 0 && decpt - ndigit > 4)
        || (decpt < 0 && decpt < -3)) {                /* use E-style */
        decpt--;
        *p2++ = *p1++;
        *p2++ = '.';
        for (i = 1; i < ndigit; i++)
            *p2++ = *p1++;
        *p2++ = 'e';
        if (decpt < 0) {
            decpt = -decpt;
            *p2++ = '-';
        } else
            *p2++ = '+';
        if (decpt / 100 > 0)
            *p2++ = decpt / 100 + '0';
        if (decpt / 10 > 0)
            *p2++ = (decpt % 100) / 10 + '0';
        *p2++ = decpt % 10 + '0';
    } else {
        if (decpt <= 0) {
            if (*p1 != '0')
                *p2++ = '.';
            while (decpt < 0) {
                decpt++;
                *p2++ = '0';
            }
        }
        for (i = 1; i <= ndigit; i++) {
            *p2++ = *p1++;
            if (i == decpt)
                *p2++ = '.';
        }
        if (ndigit < decpt) {
            while (ndigit++ < decpt)
                *p2++ = '0';
            *p2++ = '.';
        }
    }
    if (p2[-1] == '.' && !altform)
        p2--;
    *p2 = '\0';
    return(buf);
}

/*
 * The INS_CHAR macro inserts a character in the buffer and writes
 * the buffer back to disk if necessary
 * It uses the char pointers sp and bep:
 *      sp points to the next available character in the buffer
 *      bep points to the end-of-buffer+1
 * While using this macro, note that the nextb pointer is NOT updated.
 *
 * NOTE: Evaluation of the c argument should not have any side-effects
 */
#define INS_CHAR(c, sp, bep, cc)                     \
{                                                    \
    if (sp) {                                        \
        if (sp >= bep) {                             \
            vbuff->curpos = sp;                      \
            if (vbuff->flush())                      \
                return -1;                           \
            sp = vbuff->curpos;                      \
            bep = vbuff->endpos;                     \
        }                                            \
        *sp++ = (c);                                 \
    }                                                \
    cc++;                                            \
}

#define NUM(c) (c - '0')

#define STR_TO_DEC(str, num)                         \
    num = NUM(*str++);                               \
    while (isdigit(*str))                            \
    {                                                \
        num *= 10 ;                                  \
        num += NUM(*str++);                          \
    }

/*
 * This macro does zero padding so that the precision
 * requirement is satisfied. The padding is done by
 * adding '0's to the left of the string that is going
 * to be printed. We don't allow precision to be large
 * enough that we continue past the start of s.
 *
 * NOTE: this makes use of the magic info that s is
 * always based on num_buf with a size of NUM_BUF_SIZE.
 */
#define FIX_PRECISION(adjust, precision, s, s_len)   \
    if (adjust) {                                    \
        size_t p = (precision + 1 < NUM_BUF_SIZE)    \
                     ? precision : NUM_BUF_SIZE - 1; \
        while (s_len < p)                            \
        {                                            \
            *--s = '0';                              \
            s_len++;                                 \
        }                                            \
    }

/*
 * Macro that does padding. The padding is done by printing
 * the character ch.
 */
#define PAD(width, len, ch)                          \
do                                                   \
{                                                    \
    INS_CHAR(ch, sp, bep, cc);                       \
    width--;                                         \
}                                                    \
while (width > len)

/*
 * Prefix the character ch to the string str
 * Increase length
 * Set the has_prefix flag
 */
#define PREFIX(str, length, ch)                      \
    *--str = ch;                                     \
    length++;                                        \
    has_prefix=YES;


/*
 * Convert num to its decimal format.
 * Return value:
 *   - a pointer to a string containing the number (no sign)
 *   - len contains the length of the string
 *   - is_negative is set to TRUE or FALSE depending on the sign
 *     of the number (always set to FALSE if is_unsigned is TRUE)
 *
 * The caller provides a buffer for the string: that is the buf_end argument
 * which is a pointer to the END of the buffer + 1 (i.e. if the buffer
 * is declared as buf[ 100 ], buf_end should be &buf[ 100 ])
 *
 * Note: we have 2 versions. One is used when we need to use quads
 * (conv_10_quad), the other when we don't (conv_10). We're assuming the
 * latter is faster.
 */
static char *__conv_10(register std::int32_t num, register int is_unsigned,
                       register int *is_negative, char *buf_end,
                       register size_t *len)
{
    register char *p = buf_end;
    register std::uint32_t magnitude = num;

    if (is_unsigned) {
        *is_negative = 0;
    } else {
        *is_negative = (num < 0);

        /*
         * On a 2's complement machine, negating the most negative integer 
         * results in a number that cannot be represented as a signed integer.
         * Here is what we do to obtain the number's magnitude:
         *      a. add 1 to the number
         *      b. negate it (becomes positive)
         *      c. convert it to unsigned
         *      d. add 1
         */
        if (*is_negative) {
            std::int32_t t = num + 1;
            magnitude = ((std::uint32_t) -t) + 1;
        }
    }

    /*
     * We use a do-while loop so that we write at least 1 digit 
     */
    do {
        register std::uint32_t new_magnitude = magnitude / 10;

        *--p = (char) (magnitude - new_magnitude * 10 + '0');
        magnitude = new_magnitude;
    }
    while (magnitude);

    *len = buf_end - p;
    return(p);
}

static char *__conv_10_quad(std::int64_t num, register int is_unsigned,
                            register int *is_negative, char *buf_end,
                            register size_t *len)
{
    register char *p = buf_end;
    std::uint64_t magnitude = num;

    /*
     * We see if we can use the faster non-quad version by checking the
     * number against the largest long value it can be. If <=, we
     * punt to the quicker version.
     */
    if ((magnitude <= UINT32_MAX && is_unsigned)
        || (num <= INT32_MAX && num >= INT32_MIN && !is_unsigned))
        return(__conv_10((std::int32_t)num, is_unsigned, is_negative, buf_end, len));

    if (is_unsigned) {
        *is_negative = 0;
    } else {
        *is_negative = (num < 0);

        /*
         * On a 2's complement machine, negating the most negative integer 
         * results in a number that cannot be represented as a signed integer.
         * Here is what we do to obtain the number's magnitude:
         *      a. add 1 to the number
         *      b. negate it (becomes positive)
         *      c. convert it to unsigned
         *      d. add 1
         */
        if (*is_negative) {
            std::int64_t t = num + 1;
            magnitude = ((std::uint64_t) -t) + 1;
        }
    }

    /*
     * We use a do-while loop so that we write at least 1 digit 
     */
    do {
        std::uint64_t new_magnitude = magnitude / 10;

        *--p = (char) (magnitude - new_magnitude * 10 + '0');
        magnitude = new_magnitude;
    }
    while (magnitude);

    *len = buf_end - p;
    return(p);
}

/*
 * Convert a floating point number to a string formats 'f', 'e' or 'E'.
 * The result is placed in buf, and len denotes the length of the string
 * The sign is returned in the is_negative argument (and is not placed
 * in buf).
 */
static char *__conv_fp(register char format, register double num,
                       int add_dp, int precision, int *is_negative,
                       char *buf, size_t *len)
{
    register char *s = buf;
    register char *p;
    int decimal_point;
    char buf1[NDIG];

    if (format == 'f') {
        p = __fcvt(num, precision, &decimal_point, is_negative, buf1);
    } else {
        p = __ecvt(num, precision + 1, &decimal_point, is_negative, buf1);
    }

    /*
     * Check for Infinity and NaN
     */
    if (isalpha(*p)) {
        *len = strlen(p);
        memcpy(buf, p, *len + 1);
        *is_negative = 0;
        return(buf);
    }

    if (format == 'f') {
        if (decimal_point <= 0) {
            *s++ = '0';
            if (precision > 0) {
                *s++ = '.';
                while (decimal_point++ < 0)
                    *s++ = '0';
            } else if (add_dp)
                *s++ = '.';
        } else {
            while (decimal_point-- > 0)
                *s++ = *p++;
            if (precision > 0 || add_dp)
                *s++ = '.';
        }
    } else {
        *s++ = *p++;
        if (precision > 0 || add_dp)
            *s++ = '.';
    }

    /*
     * copy the rest of p, the NUL is NOT copied
     */
    while (*p)
        *s++ = *p++;

    if (format != 'f') {
        char temp[EXPONENT_LENGTH];        /* for exponent conversion */
        size_t t_len;
        int exponent_is_negative;

        *s++ = format;                /* either e or E */
        decimal_point--;
        if (decimal_point != 0) {
            p = __conv_10((std::int32_t) decimal_point, 0, &exponent_is_negative,
                          &temp[EXPONENT_LENGTH], &t_len);
            *s++ = exponent_is_negative ? '-' : '+';

            /*
             * Make sure the exponent has at least 2 digits
             */
            if (t_len == 1)
                *s++ = '0';
            while (t_len--)
                *s++ = *p++;
        } else {
            *s++ = '+';
            *s++ = '0';
            *s++ = '0';
        }
    }

    *len = s - buf;
    return(buf);
}


/*
 * Convert num to a base X number where X is a power of 2. nbits determines X.
 * For example, if nbits is 3, we do base 8 conversion
 * Return value:
 *      a pointer to a string containing the number
 *
 * The caller provides a buffer for the string: that is the buf_end argument
 * which is a pointer to the END of the buffer + 1 (i.e. if the buffer
 * is declared as buf[ 100 ], buf_end should be &buf[ 100 ])
 *
 * As with conv_10, we have a faster version which is used when
 * the number isn't quad size.
 */
static char *__conv_p2(register std::uint32_t num, register int nbits,
                       char format, char *buf_end, register size_t *len)
{
    register int mask = (1 << nbits) - 1;
    register char *p = buf_end;
    static const char low_digits[] = "0123456789abcdef";
    static const char upper_digits[] = "0123456789ABCDEF";
    register const char *digits = (format == 'X') ? upper_digits : low_digits;

    do {
        *--p = digits[num & mask];
        num >>= nbits;
    }
    while (num);

    *len = buf_end - p;
    return(p);
}

static char *__conv_p2_quad(std::uint64_t num, register int nbits,
                            char format, char *buf_end, register size_t *len)
{
    register int mask = (1 << nbits) - 1;
    register char *p = buf_end;
    static const char low_digits[] = "0123456789abcdef";
    static const char upper_digits[] = "0123456789ABCDEF";
    register const char *digits = (format == 'X') ? upper_digits : low_digits;

    if (num <= UINT32_MAX)
        return(__conv_p2((std::uint32_t)num, nbits, format, buf_end, len));

    do {
        *--p = digits[num & mask];
        num >>= nbits;
    }
    while (num);

    *len = buf_end - p;
    return(p);
}


namespace ll {

int printf_formatter::format(printf_formatter::buff *vbuff, const char *fmt, va_list ap)
{
    register char *sp;
    register char *bep;
    register int cc = 0;
    register size_t i;

    register char *s = NULL;
    char *q;
    size_t s_len = 0;

    register size_t min_width = 0;
    size_t precision = 0;
    enum {
        LEFT, RIGHT
    } adjust;
    char pad_char;
    char prefix_char;

    double fp_num;
    std::int64_t i_quad = 0;
    std::uint64_t ui_quad;
    std::int32_t i_num = 0;
    std::uint32_t ui_num;

    char num_buf[NUM_BUF_SIZE];
    char char_buf[2];                /* for printing %% and %<unknown> */

    enum var_type_enum {
        IS_QUAD, IS_LONG, IS_SHORT, IS_INT
    };
    enum var_type_enum var_type = IS_INT;

    /*
     * Flag variables
     */
    int alternate_form;
    int print_sign;
    int print_blank;
    int adjust_precision;
    int adjust_width;
    int is_negative;

    sp = vbuff->curpos;
    bep = vbuff->endpos;

    while (*fmt) {
        if (*fmt != '%') {
            INS_CHAR(*fmt, sp, bep, cc);
        } else {
            /*
             * Default variable settings
             */
            int print_something = 1;
            adjust = RIGHT;
            alternate_form = print_sign = print_blank = 0;
            pad_char = ' ';
            prefix_char = 0;

            fmt++;

            /*
             * Try to avoid checking for flags, width or precision
             */
            if (!islower(*fmt)) {
                /*
                 * Recognize flags: -, #, BLANK, +
                 */
                for (;; fmt++) {
                    if (*fmt == '-')
                        adjust = LEFT;
                    else if (*fmt == '+')
                        print_sign = 1;
                    else if (*fmt == '#')
                        alternate_form = YES;
                    else if (*fmt == ' ')
                        print_blank = 1;
                    else if (*fmt == '0')
                        pad_char = '0';
                    else
                        break;
                }

                /*
                 * Check if a width was specified
                 */
                if (isdigit(*fmt)) {
                    STR_TO_DEC(fmt, min_width);
                    adjust_width = 1;
                } else if (*fmt == '*') {
                    int v = va_arg(ap, int);
                    fmt++;
                    adjust_width = 1;
                    if (v < 0) {
                        adjust = LEFT;
                        min_width = (size_t)(-v);
                    } else
                        min_width = (size_t)v;
                } else
                    adjust_width = NO;

                /*
                 * Check if a precision was specified
                 */
                if (*fmt == '.') {
                    adjust_precision = 1;
                    fmt++;
                    if (isdigit(*fmt)) {
                        STR_TO_DEC(fmt, precision);
                    } else if (*fmt == '*') {
                        int v = va_arg(ap, int);
                        fmt++;
                        precision = (v < 0) ? 0 : (size_t)v;
                    } else
                        precision = 0;
                } else
                    adjust_precision = 0;
            } else
                adjust_precision = adjust_width = 0;

            /*
             * Modifier check.  Note that if INT64_T_FMT is "d",
             * the first if condition is never true.
             */
            if ((sizeof(INT64_T_FMT) == 4 &&
                 fmt[0] == INT64_T_FMT[0] &&
                 fmt[1] == INT64_T_FMT[1]) ||
                (sizeof(INT64_T_FMT) == 3 &&
                 fmt[0] == INT64_T_FMT[0]) ||
                (sizeof(INT64_T_FMT) > 4 &&
                 strncmp(fmt, INT64_T_FMT, 
                         sizeof(INT64_T_FMT) - 2) == 0)) {
                /* Need to account for trailing 'd' and null in sizeof() */
                var_type = IS_QUAD;
                fmt += (sizeof(INT64_T_FMT) - 2);
            } else if (*fmt == 'q') {
                var_type = IS_QUAD;
                fmt++;
            } else if (*fmt == 'l') {
                var_type = IS_LONG;
                fmt++;
            } else if (*fmt == 'h') {
                var_type = IS_SHORT;
                fmt++;
            } else {
                var_type = IS_INT;
            }

            /*
             * Argument extraction and printing.
             * First we determine the argument type.
             * Then, we convert the argument to a string.
             * On exit from the switch, s points to the string that
             * must be printed, s_len has the length of the string
             * The precision requirements, if any, are reflected in s_len.
             *
             * NOTE: pad_char may be set to '0' because of the 0 flag.
             *   It is reset to ' ' by non-numeric formats
             */
            switch (*fmt) {
            case 'u':
                if (var_type == IS_QUAD) {
                    i_quad = va_arg(ap, std::uint64_t);
                    s = __conv_10_quad(i_quad, 1, &is_negative, &num_buf[NUM_BUF_SIZE], &s_len);
                } else {
                    if (var_type == IS_LONG)
                        i_num = (std::int32_t) va_arg(ap, std::uint32_t);
                    else if (var_type == IS_SHORT)
                        i_num = (std::int32_t) (unsigned short) va_arg(ap, unsigned int);
                    else
                        i_num = (std::int32_t) va_arg(ap, unsigned int);
                    s = __conv_10(i_num, 1, &is_negative, &num_buf[NUM_BUF_SIZE], &s_len);
                }
                FIX_PRECISION(adjust_precision, precision, s, s_len);
                break;

            case 'd':
            case 'i':
                if (var_type == IS_QUAD) {
                    i_quad = va_arg(ap, std::int64_t);
                    s = __conv_10_quad(i_quad, 0, &is_negative, &num_buf[NUM_BUF_SIZE], &s_len);
                } else {
                    if (var_type == IS_LONG)
                        i_num = va_arg(ap, std::int32_t);
                    else if (var_type == IS_SHORT)
                        i_num = (short) va_arg(ap, int);
                    else
                        i_num = va_arg(ap, int);
                    s = __conv_10(i_num, 0, &is_negative, &num_buf[NUM_BUF_SIZE], &s_len);
                }
                FIX_PRECISION(adjust_precision, precision, s, s_len);

                if (is_negative)
                    prefix_char = '-';
                else if (print_sign)
                    prefix_char = '+';
                else if (print_blank)
                    prefix_char = ' ';
                break;


            case 'o':
                if (var_type == IS_QUAD) {
                    ui_quad = va_arg(ap, std::uint64_t);
                    s = __conv_p2_quad(ui_quad, 3, *fmt, &num_buf[NUM_BUF_SIZE], &s_len);
                } else {
                    if (var_type == IS_LONG)
                        ui_num = va_arg(ap, std::uint32_t);
                    else if (var_type == IS_SHORT)
                        ui_num = (unsigned short) va_arg(ap, unsigned int);
                    else
                        ui_num = va_arg(ap, unsigned int);
                    s = __conv_p2(ui_num, 3, *fmt, &num_buf[NUM_BUF_SIZE], &s_len);
                }
                FIX_PRECISION(adjust_precision, precision, s, s_len);
                if (alternate_form && *s != '0') {
                    *--s = '0';
                    s_len++;
                }
                break;


            case 'x':
            case 'X':
                if (var_type == IS_QUAD) {
                    ui_quad = va_arg(ap, std::uint64_t);
                    s = __conv_p2_quad(ui_quad, 4, *fmt, &num_buf[NUM_BUF_SIZE], &s_len);
                } else {
                    if (var_type == IS_LONG)
                        ui_num = va_arg(ap, std::uint32_t);
                    else if (var_type == IS_SHORT)
                        ui_num = (unsigned short) va_arg(ap, unsigned int);
                    else
                        ui_num = va_arg(ap, unsigned int);
                    s = __conv_p2(ui_num, 4, *fmt, &num_buf[NUM_BUF_SIZE], &s_len);
                }
                FIX_PRECISION(adjust_precision, precision, s, s_len);
                if (alternate_form && i_num != 0) {
                    *--s = *fmt;        /* 'x' or 'X' */
                    *--s = '0';
                    s_len += 2;
                }
                break;


            case 's':
                s = va_arg(ap, char *);
                if (s != NULL) {
                    if (!adjust_precision) {
                        s_len = strlen(s);
                    } else {
                        /* From the C library standard in section 7.9.6.1:
                         * ...if the precision is specified, no more then
                         * that many characters are written.  If the
                         * precision is not specified or is greater
                         * than the size of the array, the array shall
                         * contain a null character.
                         *
                         * My reading is is precision is specified and
                         * is less then or equal to the size of the
                         * array, no null character is required.  So
                         * we can't do a strlen.
                         *
                         * This figures out the length of the string
                         * up to the precision.  Once it's long enough
                         * for the specified precision, we don't care
                         * anymore.
                         *
                         * NOTE: you must do the length comparison
                         * before the check for the null character.
                         * Otherwise, you'll check one beyond the
                         * last valid character.
                         */
                        const char *walk;

                        for (walk = s, s_len = 0;
                            (s_len < precision) && (*walk != '\0');
                            ++walk, ++s_len);
                    }
                } else {
                    s = S_NULL;
                    s_len = S_NULL_LEN;
                }
                pad_char = ' ';
                break;


            case 'f':
            case 'e':
            case 'E':
                fp_num = va_arg(ap, double);
                /*
                 * We use &num_buf[ 1 ], so that we have room for the sign
                 */
                s = NULL;
                if (isnan(fp_num)) {
                    s = (char*)"nan";
                    s_len = 3;
                }
                if (!s && isinf(fp_num)) {
                    s = (char*)"inf";
                    s_len = 3;
                }
                if (!s) {
                    s = __conv_fp(*fmt, fp_num, alternate_form,
                                  (int)((adjust_precision == 0) ? FLOAT_DIGITS : precision),
                                  &is_negative, &num_buf[1], &s_len);
                    if (is_negative)
                        prefix_char = '-';
                    else if (print_sign)
                        prefix_char = '+';
                    else if (print_blank)
                        prefix_char = ' ';
                }
                break;


            case 'g':
            case 'G':
                if (adjust_precision == 0)
                    precision = FLOAT_DIGITS;
                else if (precision == 0)
                    precision = 1;
                /*
                 * * We use &num_buf[ 1 ], so that we have room for the sign
                 */
                s = __gcvt(va_arg(ap, double), (int) precision, &num_buf[1], alternate_form);
                if (*s == '-')
                    prefix_char = *s++;
                else if (print_sign)
                    prefix_char = '+';
                else if (print_blank)
                    prefix_char = ' ';

                s_len = strlen(s);

                if (alternate_form && (q = strchr(s, '.')) == NULL) {
                    s[s_len++] = '.';
                    s[s_len] = '\0'; /* delimit for following strchr() */
                }
                if (*fmt == 'G' && (q = strchr(s, 'e')) != NULL)
                    *q = 'E';
                break;


            case 'c':
                char_buf[0] = (char) (va_arg(ap, int));
                s = &char_buf[0];
                s_len = 1;
                pad_char = ' ';
                break;


            case '%':
                char_buf[0] = '%';
                s = &char_buf[0];
                s_len = 1;
                pad_char = ' ';
                break;


            case 'n':
                if (var_type == IS_QUAD)
                    *(va_arg(ap, std::int64_t *)) = cc;
                else if (var_type == IS_LONG)
                    *(va_arg(ap, long *)) = cc;
                else if (var_type == IS_SHORT)
                    *(va_arg(ap, short *)) = cc;
                else
                    *(va_arg(ap, int *)) = cc;
                print_something = 0;
                break;

            case 'p':
#if (SIZEOF_VOID_P == 8)
                if (sizeof(void *) <= sizeof(std::uint64_t)) {
                    ui_quad = (std::uint64_t) va_arg(ap, void *);
                    s = __conv_p2_quad(ui_quad, 4, 'x', &num_buf[NUM_BUF_SIZE], &s_len);
                }
#else
                if (sizeof(void *) <= sizeof(std::uint32_t)) {
                    ui_num = (std::uint32_t) va_arg(ap, void *);
                    s = __conv_p2(ui_num, 4, 'x', &num_buf[NUM_BUF_SIZE], &s_len);
                }
#endif
                else {
                    s = (char*)"%p";
                    s_len = 2;
                    prefix_char = 0;
                }
                pad_char = ' ';
                break;

            case 0:
                /*
                 * The last character of the format string was %.
                 * We ignore it.
                 */
                continue;


                /*
                 * The default case is for unrecognized %'s.
                 * We print %<char> to help the user identify what
                 * option is not understood.
                 * This is also useful in case the user wants to pass
                 * the output of format_converter to another function
                 * that understands some other %<char> (like syslog).
                 * Note that we can't point s inside fmt because the
                 * unknown <char> could be preceded by width etc.
                 */
            default:
                char_buf[0] = '%';
                char_buf[1] = *fmt;
                s = char_buf;
                s_len = 2;
                pad_char = ' ';
                break;
            }

            if (prefix_char != 0 && s != S_NULL && s != char_buf) {
                *--s = prefix_char;
                s_len++;
            }

            if (adjust_width && adjust == RIGHT && min_width > s_len) {
                if (pad_char == '0' && prefix_char != 0) {
                    INS_CHAR(*s, sp, bep, cc);
                    s++;
                    s_len--;
                    min_width--;
                }
                PAD(min_width, s_len, pad_char);
            }

            /*
             * Print the string s. 
             */
            if (print_something == YES) {
                for (i = s_len; i != 0; i--) {
                    INS_CHAR(*s, sp, bep, cc);
                    s++;
                }
            }

            if (adjust_width && adjust == LEFT && min_width > s_len)
                PAD(min_width, s_len, pad_char);
        }
        fmt++;
    }
    vbuff->curpos = sp;

    return cc;
}

};
