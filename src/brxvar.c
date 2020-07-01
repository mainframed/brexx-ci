#define __BMEM_H__

#include "printf.h"
#include "lstring.h"
#include "variable.h"
#include "irx.h"

int __libc_arch = 0;

// this is needed to enable the linker to find printf
#define printf printf

#define MALLOC(s, d)        malloc_or_die(s)
#define REALLOC(p, s)    realloc_or_die(p,s)

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
int myprintf(const char *msg, ...);

int
GETVAR(unsigned char *name, size_t nameLength, unsigned char *pBuffer, size_t bufferSize, unsigned int *pValueLength) {
    int rc;
    int found;

    BinTree *tree;
    PBinLeaf leaf;

    Lstr lName;

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

int printf(const char *msg, ...) {

    int ret = 0;

    va_list va;

    char line[80];
    bzero(line, 80);


    va_start(va, msg);

    ret = vsnprintf(line, 80, msg, va);

    _tput(line);

    va_end(va);

    return ret;
}

void     _putchar(char character) {
    _tput("_putchar() called");
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