#define __BMEM_H__

#include "lstring.h"
#include "variable.h"
#include "irx.h"

int __libc_arch = 0;

#define    MALLOC(s, d)        malloc_or_die(s)
#define    REALLOC(p, s)    realloc_or_die(p,s)

#ifndef SVC
#define SVC
struct SVCREGS {
    int R0;
    int R1;
    int R15;
};
#endif

#ifndef MIN
# define MIN(a,b)	(((a)<(b))?(a):(b))
#endif
#ifndef MAX
# define MAX(a,b)	(((a)>(b))?(a):(b))
#endif

/* ---------------------------------------------------------- */
/* environment block RXENVBLK                                 */
/* ---------------------------------------------------------- */
typedef struct envblock RX_ENVIRONMENT_BLK, *RX_ENVIRONMENT_BLK_PTR;

void BRXSVC(int svc, struct SVCREGS *regs);
void _tput(const char *data);
BinTree *findTree(const unsigned char *name);
void *malloc_or_die(size_t size);
void *realloc_or_die(void *ptr, size_t size);
void Lsccpy(const PLstr to, unsigned char *from);
int printf(const char *msg, ...);

int
GETVAR(unsigned char *name, size_t nameLength, unsigned char *pBuffer, size_t bufferSize, unsigned int *pValueLength) {
    int rc;
    int found;

    BinTree *tree;
    PBinLeaf leaf;

    Lstr lName;

    printf("BRXVAR: GETVAR %.*s\n", (int) nameLength, name);

    tree = findTree(name);

    if (tree != NULL) {
        lName.pstr = name;
        lName.len = nameLength;
        lName.maxlen = nameLength;
        lName.type = LSTRING_TY;

        LASCIIZ(lName)

        leaf = BinFind(tree, &lName);
        found = (leaf != NULL);
        if (found) {
            memcpy(pBuffer, (LEAFVAL(leaf))->pstr, MIN(bufferSize, (LEAFVAL(leaf))->len));
            *pValueLength = MIN(bufferSize, (LEAFVAL(leaf))->len);

            rc = 0;
        } else {
            rc = 8;
        }
    } else {
        rc = 12;
    }

    return rc;
}

int
SETVAR(unsigned char *name, size_t nameLength, unsigned char *value, size_t valueLength) {
    int rc;
    int found;

    BinTree *tree;
    PBinLeaf leaf;

    Lstr lName;
    Lstr lValue;
    Lstr aux;

    Variable *var;

    printf("IRXEXCOM> SETVAR %.*s=%.*s\n", (int) nameLength, name, (int) valueLength, value);

    tree = findTree(name);

    if (tree != NULL) {

        lName.pstr = name;
        lName.len = nameLength;
        lName.maxlen = nameLength;
        lName.type = LSTRING_TY;

        lValue.pstr = value;
        lValue.len = valueLength;
        lValue.maxlen = valueLength;
        lValue.type = LSTRING_TY;

        leaf = BinFind(tree, &lName);
        found = (leaf != NULL);

        if (found) {
            /* Just copy the new value */
            Lstrcpy(LEAFVAL(leaf), &lValue);

            rc = 0;
        } else {
            /* added it to the tree */
            /* create memory */
            LINITSTR(aux)
            Lsccpy(&aux, name);
            LASCIIZ(aux)
            tree = findTree(name);
            var = (Variable *) malloc_or_die(sizeof(Variable));
            LINITSTR(var->value)
            Lfx(&(var->value), lValue.len);
            var->exposed = NO_PROC;
            var->stem = NULL;

            leaf = BinAdd(tree, &aux, var);
            Lstrcpy(LEAFVAL(leaf), &lValue);
            //Lsccpy(LEAFVAL(leaf),value);

            rc = 0;
        }
    } else {
        rc = 12;
    }

    return rc;
}

void *_getEctEnvBk() {
    void **psa;           // PAS      =>   0 / 0x00
    void **ascb;          // PSAAOLD  => 548 / 0x224
    void **asxb;          // ASCBASXB => 108 / 0x6C
    void **lwa;           // ASXBLWA  =>  20 / 0x14
    void **ect;           // LWAPECT  =>  32 / 0x20
    void **ectenvbk;      // ECTENVBK =>  48 / 0x30

    psa = 0;
    ascb = psa[137];
    asxb = ascb[27];
    lwa = asxb[5];

    ect = lwa[8];
    ectenvbk = ect + 48;

    return ectenvbk;
}

void *getEnvBlock() {
    void **ectenvbk;
    RX_ENVIRONMENT_BLK_PTR envblock;

    ectenvbk = _getEctEnvBk();
    envblock = *ectenvbk;

    if (envblock != NULL) {
        if (strncmp((char *) envblock->envblock_id, "ENVBLOCK", 8) == 0) {
            return envblock;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

BinTree *findTree(const unsigned char *name) {

    BinTree *tree;

    RX_ENVIRONMENT_BLK_PTR envBlock;

    Scope scope;

    int i = 0;

    int hashchar[256];    /* use the first char as hash value */

    envBlock = getEnvBlock();

    if (envBlock != NULL) {
        scope = envBlock->envblock_userfield;
        tree = scope;
    } else {
        tree = NULL;
    }

    return tree;
}

/* needed LSTRING functions */
void Lfx(const PLstr s, const size_t len) {
    size_t max;

    if (LISNULL(*s)) {
        LSTR(*s) = (unsigned char *) malloc_or_die((max = LNORMALISE(len)) + LEXTRA);
        memset(LSTR(*s), 0, max);
        LLEN(*s) = 0;
        LMAXLEN(*s) = max;
        LTYPE(*s) = LSTRING_TY;
    } else if (LMAXLEN(*s) < len) {
        LSTR(*s) = (unsigned char *) realloc_or_die(LSTR(*s), (max = LNORMALISE(len)) + LEXTRA);
        LMAXLEN(*s) = max;
    }
}

void Lsccpy(const PLstr to, unsigned char *from) {
    size_t len;

    if (!from)
        Lfx(to, len = 0);
    else {
        Lfx(to, len = strlen((const char *) from));
        MEMCPY(LSTR(*to), from, len);
    }
    LLEN(*to) = len;
    LTYPE(*to) = LSTRING_TY;
}

void Lstrcpy(const PLstr to, const PLstr from) {
    if (LISNULL(*to)) {
        Lfx(to, 31);
    }

    if (LLEN(*from) == 0) {
        LLEN(*to) = 0;
        LTYPE(*to) = LSTRING_TY;
    } else {
        if (LMAXLEN(*to) <= LLEN(*from)) Lfx(to, LLEN(*from));
        switch (LTYPE(*from)) {
            case LSTRING_TY:
                MEMCPY(LSTR(*to), LSTR(*from), LLEN(*from));
                break;

            case LINTEGER_TY:
                LINT(*to) = LINT(*from);
                break;

            case LREAL_TY:
                LREAL(*to) = LREAL(*from);
                break;
        }
        LTYPE(*to) = LTYPE(*from);
        LLEN(*to) = LLEN(*from);
    }
}

int _Lstrcmp(const PLstr a, const PLstr b) {
    int r;

    if ((r = MEMCMP(LSTR(*a), LSTR(*b), MIN(LLEN(*a), LLEN(*b)))) != 0)
        return r;
    else {
        if (LLEN(*a) > LLEN(*b))
            return 1;
        else if (LLEN(*a) == LLEN(*b)) {
            if (LTYPE(*a) > LTYPE(*b))
                return 1;
            else if (LTYPE(*a) < LTYPE(*b))
                return -1;
            return 0;
        } else
            return -1;
    }
}

/* mvs memory allocation */
void * _getmain(size_t length) {
    long *ptr;

    struct SVCREGS registers;
    registers.R0 = (unsigned) (length + 12);
    registers.R1 = -1;
    registers.R15 = 0;

    BRXSVC(10, &registers);

    if (registers.R15 == 0) {
        ptr = (void *) registers.R1;
        ptr[0] = 0xDEADBEAF;
        ptr[1] = (((long) (ptr)) + 12);
        ptr[2] = length;
    } else {
        ptr = NULL;
    }
    return (void *) (((int) (ptr)) + 12);
}

void * malloc_or_die(size_t size) {
    void *nPtr;

    nPtr = _getmain(size);
    if (!nPtr) {
        _tput("ERR>   GETMAIN FAILED");
        return NULL;
    }

    return nPtr;
}

void * realloc_or_die(void *oPtr, size_t size) {
    void *nPtr;

    nPtr = _getmain(size);

    if (!nPtr) {
        _tput("ERR>   GETMAIN FAILED");
        return NULL;
    }

    return nPtr;

    /* TODO: added beacause get rid of failed realloc's */
    /*
    size++;

    ptr = realloc(ptr,size);

    if (!ptr) {
        Lstr lerrno;

        LINITSTR(lerrno)
        Lfx(&lerrno,31);

        Lscpy(&lerrno,strerror(errno));

        Lerror(ERR_REALLOC_FAILED,0);
        fprintf(stderr, "errno: %s\n",strerror(errno));

        LFREESTR(lerrno);
        raise(SIGSEGV);
    }

    return ptr;
    */
}

void free_or_die(void *ptr) {

}

/* terminal i/o */
void _tput(const char *data) {
    struct SVCREGS registers;
    registers.R0 = strlen(data);
    registers.R1 = (unsigned int) data & 0x00FFFFFF;
    registers.R15 = 0;

    BRXSVC(93, &registers);
}


/* ************TEMP******************* */

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#define PRINTF_NTOA_BUFFER_SIZE    32U
#endif

// internal flag definitions
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_LONG      (1U <<  8U)
#define FLAGS_LONG_LONG (1U <<  9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

typedef void (*out_fct_type)(char character, void *buffer, size_t idx, size_t maxlen);

static void _out_null(char character, void *buffer, size_t idx, size_t maxlen) {
    (void) character;
    (void) buffer;
    (void) idx;
    (void) maxlen;
}

static void _out_buffer(char character, void *buffer, size_t idx, size_t maxlen) {
    if (idx < maxlen) {
        ((char *) buffer)[idx] = character;
    }
}

static size_t _out_rev(out_fct_type out, char *buffer, size_t idx, size_t maxlen, const char *buf,
                       size_t len, unsigned int width, unsigned int flags) {
    const size_t start_idx = idx;
    size_t i;

    // pad spaces up to given width
    if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
        for (i = len; i < width; i++) {
            out(' ', buffer, idx++, maxlen);
        }
    }

    // reverse string
    while (len) {
        out(buf[--len], buffer, idx++, maxlen);
    }

    // append pad spaces up to given width
    if (flags & FLAGS_LEFT) {
        while (idx - start_idx < width) {
            out(' ', buffer, idx++, maxlen);
        }
    }

    return idx;
}

static bool _is_digit(char ch) {
    return (ch >= '0') && (ch <= '9');
}

static unsigned int _atoi(const char **str) {
    unsigned int i = 0U;
    while (_is_digit(**str)) {
        i = i * 10U + (unsigned int) (*((*str)++) - '0');
    }
    return i;
}

static unsigned int _strnlen_s(const char *str, size_t maxsize) {
    const char *s;
    for (s = str; *s && maxsize--; ++s);
    return (unsigned int) (s - str);
}

static size_t _ntoa_format(out_fct_type out, char *buffer, size_t idx, size_t maxlen, char *buf, size_t len, bool negative,
             unsigned int base, unsigned int prec, unsigned int width, unsigned int flags) {
    // pad leading zeros
    if (!(flags & FLAGS_LEFT)) {
        if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
            width--;
        }
        while ((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
        while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
    }

    // handle hash
    if (flags & FLAGS_HASH) {
        if (!(flags & FLAGS_PRECISION) && len && ((len == prec) || (len == width))) {
            len--;
            if (len && (base == 16U)) {
                len--;
            }
        }
        if ((base == 16U) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'x';
        } else if ((base == 16U) && (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'X';
        } else if ((base == 2U) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'b';
        }
        if (len < PRINTF_NTOA_BUFFER_SIZE) {
            buf[len++] = '0';
        }
    }

    if (len < PRINTF_NTOA_BUFFER_SIZE) {
        if (negative) {
            buf[len++] = '-';
        } else if (flags & FLAGS_PLUS) {
            buf[len++] = '+';  // ignore the space if the '+' exists
        } else if (flags & FLAGS_SPACE) {
            buf[len++] = ' ';
        }
    }

    return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}

static size_t _ntoa_long(out_fct_type out, char *buffer, size_t idx, size_t maxlen, unsigned long value, bool negative,
                         unsigned long base, unsigned int prec, unsigned int width, unsigned int flags) {
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!value) {
        flags &= ~FLAGS_HASH;
    }

    // write if precision != 0 and value is != 0
    if (!(flags & FLAGS_PRECISION) || value) {
        do {
            const char digit = (char) (value % base);
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            value /= base;
        } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int) base, prec, width, flags);
}

static int myvsnprintf(out_fct_type out, char *buffer, const size_t maxlen, const char *format, va_list va) {
    unsigned int flags, width, precision, n;
    size_t idx = 0U;

    if (!buffer) {
        // use null output function
        out = _out_null;
    }

    while (*format) {
        // format specifier?  %[flags][width][.precision][length]
        if (*format != '%') {
            // no
            out(*format, buffer, idx++, maxlen);
            format++;
            continue;
        } else {
            // yes, evaluate it
            format++;
        }

        // evaluate flags
        flags = 0U;
        do {
            switch (*format) {
                case '0':
                    flags |= FLAGS_ZEROPAD;
                    format++;
                    n = 1U;
                    break;
                case '-':
                    flags |= FLAGS_LEFT;
                    format++;
                    n = 1U;
                    break;
                case '+':
                    flags |= FLAGS_PLUS;
                    format++;
                    n = 1U;
                    break;
                case ' ':
                    flags |= FLAGS_SPACE;
                    format++;
                    n = 1U;
                    break;
                case '#':
                    flags |= FLAGS_HASH;
                    format++;
                    n = 1U;
                    break;
                default :
                    n = 0U;
                    break;
            }
        } while (n);

        // evaluate width field
        width = 0U;
        if (_is_digit(*format)) {
            width = _atoi(&format);
        } else if (*format == '*') {
            const int w = va_arg(va, int);
            if (w < 0) {
                flags |= FLAGS_LEFT;    // reverse padding
                width = (unsigned int) -w;
            } else {
                width = (unsigned int) w;
            }
            format++;
        }

        // evaluate precision field
        precision = 0U;
        if (*format == '.') {
            flags |= FLAGS_PRECISION;
            format++;
            if (_is_digit(*format)) {
                precision = _atoi(&format);
            } else if (*format == '*') {
                const int prec = (int) va_arg(va, int);
                precision = prec > 0 ? (unsigned int) prec : 0U;
                format++;
            }
        }

        // evaluate length field
        switch (*format) {
            case 'l' :
                flags |= FLAGS_LONG;
                format++;
                if (*format == 'l') {
                    flags |= FLAGS_LONG_LONG;
                    format++;
                }
                break;
            case 'h' :
                flags |= FLAGS_SHORT;
                format++;
                if (*format == 'h') {
                    flags |= FLAGS_CHAR;
                    format++;
                }
                break;
            case 'j' :
                flags |= FLAGS_LONG;
                format++;
                break;
            case 'z' :
                flags |= FLAGS_LONG;
                format++;
                break;
            default :
                break;
        }

        // evaluate specifier
        switch (*format) {
            case 'd' :
            case 'i' :
            case 'u' :
            case 'x' :
            case 'X' :
            case 'o' :
            case 'b' : {
                // set the base
                unsigned int base;
                if (*format == 'x' || *format == 'X') {
                    base = 16U;
                } else if (*format == 'o') {
                    base = 8U;
                } else if (*format == 'b') {
                    base = 2U;
                } else {
                    base = 10U;
                    flags &= ~FLAGS_HASH;   // no hash for dec format
                }
                // uppercase
                if (*format == 'X') {
                    flags |= FLAGS_UPPERCASE;
                }

                // no plus or space flag for u, x, X, o, b
                if ((*format != 'i') && (*format != 'd')) {
                    flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
                }

                // ignore '0' flag when precision is given
                if (flags & FLAGS_PRECISION) {
                    flags &= ~FLAGS_ZEROPAD;
                }

                // convert the integer
                if ((*format == 'i') || (*format == 'd')) {
                    // signed
                    if (flags & FLAGS_LONG_LONG) {

                    } else if (flags & FLAGS_LONG) {
                        const long value = va_arg(va, long);
                        idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long) (value > 0 ? value : 0 - value),
                                         value < 0, base, precision, width, flags);
                    } else {
                        const int value = (flags & FLAGS_CHAR) ? (char) va_arg(va, int) : (flags & FLAGS_SHORT)
                                                                                          ? (short int) va_arg(va, int)
                                                                                          : va_arg(va, int);
                        idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned int) (value > 0 ? value : 0 - value),
                                         value < 0, base, precision, width, flags);
                    }
                } else {
                    // unsigned
                    if (flags & FLAGS_LONG_LONG) {

                    } else if (flags & FLAGS_LONG) {
                        idx = _ntoa_long(out, buffer, idx, maxlen, va_arg(va, unsigned long), FALSE, base, precision,
                                         width, flags);
                    } else {
                        const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char) va_arg(va, unsigned int)
                                                                        : (flags & FLAGS_SHORT)
                                                                          ? (unsigned short int) va_arg(va,
                                                                                                        unsigned int)
                                                                          : va_arg(va, unsigned int);
                        idx = _ntoa_long(out, buffer, idx, maxlen, value, FALSE, base, precision, width, flags);
                    }
                }
                format++;
                break;
            }
            case 'c' : {
                unsigned int l = 1U;
                // pre padding
                if (!(flags & FLAGS_LEFT)) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                // char output
                out((char) va_arg(va, int), buffer, idx++, maxlen);
                // post padding
                if (flags & FLAGS_LEFT) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                format++;
                break;
            }

            case 's' : {
                const char *p = va_arg(va, char*);
                unsigned int l = _strnlen_s(p, precision ? precision : (size_t) -1);
                // pre padding
                if (flags & FLAGS_PRECISION) {
                    l = (l < precision ? l : precision);
                }
                if (!(flags & FLAGS_LEFT)) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                // string output
                while ((*p != 0) && (!(flags & FLAGS_PRECISION) || precision--)) {
                    out(*(p++), buffer, idx++, maxlen);
                }
                // post padding
                if (flags & FLAGS_LEFT) {
                    while (l++ < width) {
                        out(' ', buffer, idx++, maxlen);
                    }
                }
                format++;
                break;
            }

            case 'p' : {
                width = sizeof(void *) * 2U;
                flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;

                idx = _ntoa_long(out, buffer, idx, maxlen, (unsigned long) ((unsigned long) va_arg(va, void*)), FALSE,
                                 16U, precision, width, flags);

                format++;
                break;
            }

            case '%' :
                out('%', buffer, idx++, maxlen);
                format++;
                break;

            default :
                out(*format, buffer, idx++, maxlen);
                format++;
                break;
        }
    }

    // termination
    out((char) 0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);

    // return written chars without terminating \0
    return (int) idx;
}

/* ********************************** */

int printf(const char *msg, ...) {

    int ret = 0;

    va_list va;

    char line[80];
    bzero(line, 80);


    va_start(va, msg);

    ret = myvsnprintf(_out_buffer, line, 80, msg, va);

    _tput(line);

    va_end(va);

    return ret;
}

/* dummy impls for bintree */
int Lstrbeg(const PLstr str, const PLstr pre) {
    int i = 1;
    return i++;
}

#ifdef __CROSS__
void BRXSVC(int svc, struct SVCREGS *regs) {

}
#endif