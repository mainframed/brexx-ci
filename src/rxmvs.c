#include <stdlib.h>
#include <stdio.h>

#ifdef JCC
#include <io.h>
#endif
#include "rexx.h"
#include "rxdefs.h"
#include "rxmvsext.h"
#include "util.h"
#ifdef __DEBUG__
#include "bmem.h"
#endif

RX_ENVIRONMENT_CTX_PTR environment = NULL;

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;

/* FLAG2 */
const unsigned char _TSOFG  = 0x1; // hex for 0000 0001
const unsigned char _TSOBG  = 0x2; // hex for 0000 0010
const unsigned char _EXEC   = 0x4; // hex for 0000 0100
const unsigned char _ISPF   = 0x8; // hex for 0000 1000
/* FLAG3 */
const unsigned char _STDIN  = 0x1; // hex for 0000 0001
const unsigned char _STDOUT = 0x2; // hex for 0000 0010
const unsigned char _STDERR = 0x4; // hex for 0000 0100

#ifdef JCC
extern char* _style;
#else
char* _style;
#endif

/* internal function prototypes */
void parseArgs(char **array, char *str);
void parseDCB(FILE *pFile);
void setVariable(char *sName, char *sValue);
int checkNameLength(long lName);
int checkValueLength(long lValue);
int checkVariableBlacklist(PLstr name);
int reopen(int fp);

#ifdef __CROSS__
int __get_ddndsnmemb (int handle, char * ddn, char * dsn,
                      char * member, char * serial, unsigned char * flags);
char *getItem(RX_IKJ441_DUMMY_DICT *dict, char *key);
void  delItem(RX_IKJ441_DUMMY_DICT *dict, char *key);
void  addItem(RX_IKJ441_DUMMY_DICT *dict, char *key, char *value);

RX_IKJ441_DUMMY_DICT_HEAD sDummyDictionary = {0};
#endif

#define BLACKLIST_SIZE 8
char *RX_VAR_BLACKLIST[BLACKLIST_SIZE] = {"RC", "LASTCC", "SIGL", "RESULT", "SYSPREF", "SYSUID", "SYSENV", "SYSISPF"};

void R_wto(int func)
{
    RX_WTO_PARAMS_PTR params;

    char  *msgptr = NULL;
    size_t msglen = 0;
    int      cc     = 0;
    void     *wk;

    if (ARGN != 1)
        Lerror(ERR_INCORRECT_CALL,0);

    LASCIIZ(*ARG1);
    get_s(1);

    msglen = MIN(strlen((char *)LSTR(*ARG1)),80);

    if (msglen > 0) {
        msgptr = malloc(msglen);
        params = malloc(sizeof(RX_WTO_PARAMS));
        wk     = malloc(256);

        memset(msgptr,0,80);
        memcpy(msgptr,(char *)LSTR(*ARG1),msglen);

        params->msgadr       = msgptr;
        params->msgladr      = (unsigned int *)&msglen;
        params->ccadr        = (unsigned *)&cc;
        params->wkadr        = (unsigned *)wk;

        call_rxwto(params);

        free(wk);
        free(params);
        free(msgptr);
    }
}

void R_wait(int func)
{
    RX_WAIT_PARAMS_PTR params;
    void     *wk;
    unsigned time   = 0;
    int      cc     = 0;

    if (ARGN != 1)
        Lerror(ERR_INCORRECT_CALL,0);

    LASCIIZ(*ARG1);
    get_i (1,time);

    params = malloc(sizeof(RX_WAIT_PARAMS));
    wk     = malloc(256);

    params->timeadr      = &time;
    params->ccadr        = (unsigned *)&cc;
    params->wkadr        = (unsigned *)wk;

    call_rxwait(params);

    free(wk);
    free(params);
}

void R_abend(int func)
{
    RX_ABEND_PARAMS_PTR params;
    int ucc = 0;

    if (ARGN != 1)
        Lerror(ERR_INCORRECT_CALL,0);

    LASCIIZ(*ARG1);
    get_i (1,ucc);

    params = malloc(sizeof(RX_ABEND_PARAMS));

    params->ucc          = ucc;

    call_rxabend(params);

    free(params);
}

void R_userid(int func)
{
    char *userid = "n.a.";

    if (ARGN>0) {
        Lerror(ERR_INCORRECT_CALL,0);
    }
#ifdef JCC
    userid = getlogin();
#endif
    Lscpy(ARGR,userid);
}

void R_msg(int func)
{
    char *msg = "not yet implemented";

    Lscpy(ARGR,msg);
}

void R_listdsi(int func)
{
    char *args[2];

    char sFileName[45];
    char sFunctionCOde[3];

    FILE *pFile;
    int iErr;

    QuotationType quotationType;

    memset(sFileName,0,45);
    memset(sFunctionCOde,0,3);

    iErr = 0;

    if (ARGN != 1)
        Lerror(ERR_INCORRECT_CALL,0);

    LASCIIZ(*ARG1);
    get_s(1);
    Lupper(ARG1);

    args[0]= NULL;
    args[1]= NULL;

    parseArgs(args, (char *)LSTR(*ARG1));

    if (args[1] != NULL && strcmp(args[1], "FILE") != 0)
        Lerror(ERR_INCORRECT_CALL,0);

    if (args[1] == NULL) {
        _style = "//DSN:";
        quotationType = CheckQuotation(args[0]);
        switch (quotationType) {
            case UNQUOTED:
                if (environment->SYSPREF[0] != '\0') {
                    strcat(sFileName, environment->SYSPREF);
                    strcat(sFileName, ".");
                    strcat(sFileName, (const char *) LSTR(*ARG1));
                }
                break;
            case PARTIALLY_QUOTED:
                strcat(sFunctionCOde, "16");
                iErr = 2;
                break;
            case FULL_QUOTED:
                strncpy(sFileName, (const char *) (LSTR(*ARG1)) + 1, ARG1->len - 2);
                break;
            default:
                Lerror(ERR_DATA_NOT_SPEC, 0);


        }
    } else {
        strcpy(sFileName,args[0]);
        _style = "//DDN:";
    }

    if (iErr == 0) {
        pFile = FOPEN(sFileName,"R");
        if (pFile != NULL) {
            strcat(sFunctionCOde,"0");
            parseDCB(pFile);
            FCLOSE(pFile);
        } else {
            strcat(sFunctionCOde,"16");
        }
    }

    Lscpy(ARGR,sFunctionCOde);
}

void R_sysdsn(int func)
{
    char sDSName[45];
    char sMessage[256];

    unsigned char *ptr;

    FILE *pFile;
    int iErr;
    QuotationType quotationType;

    const char* MSG_OK                  = "OK";
    const char* MSG_NOT_A_PO            = "MEMBER SPECIFIED, BUT DATASET IS NOT PARTITIONED";
    const char* MSG_MEMBER_NOT_FOUND    = "MEMBER NOT FOUND";
    const char* MSG_DATASET_NOT_FOUND   = "DATASET NOT FOUND";
    const char* MSG_ERROR_READING       = "ERROR PROCESSING REQUESTED DATASET";
    const char* MSG_DATSET_PROTECTED    = "PROTECTED DATASET";
    const char* MSG_VOLUME_NOT_FOUND    = "VOLUME NOT ON SYSTEM";
    const char* MSG_DATASET_UNAVAILABLE = "UNAVAILABLE DATASET";
    const char* MSG_INVALID_DSNAME      = "INVALID DATASET NAME, ";
    const char* MSG_MISSING_DSNAME      = "MISSING DATASET NAME";

    memset(sDSName,0,45);
    memset(sMessage,0,256);

    iErr = 0;

    if (ARGN != 1)
        Lerror(ERR_INCORRECT_CALL,0);

    LASCIIZ(*ARG1);
    get_s(1);
    Lupper(ARG1);

    if (LSTR(*ARG1)[0] == '\0') {
        strcat(sMessage,MSG_MISSING_DSNAME);
        iErr = 1;
    }

    if (iErr == 0) {
        quotationType = CheckQuotation((char *)LSTR(*ARG1));
        switch(quotationType) {
            case UNQUOTED:
                if (environment->SYSPREF[0] != '\0') {
                    strcat(sDSName, environment->SYSPREF);
                    strcat(sDSName, ".");
                    strcat(sDSName, (const char*)LSTR(*ARG1));
                }
                break;
            case PARTIALLY_QUOTED:
                strcat(sMessage,MSG_INVALID_DSNAME);
                strcat(sMessage,(const char*)LSTR(*ARG1));
                iErr = 2;
                break;
            case FULL_QUOTED:
                strncpy(sDSName, (const char *)(LSTR(*ARG1))+1, ARG1->len-2);
                break;
            default:
                Lerror(ERR_DATA_NOT_SPEC,0);
        }
    }

    if (iErr == 0) {
        _style = "//DSN:";
        pFile = FOPEN(sDSName,"R");
        if (pFile != NULL) {
            strcat(sMessage, MSG_OK);
            FCLOSE(pFile);
        } else {
            strcat(sMessage,MSG_DATASET_NOT_FOUND);
        }
    }

    Lscpy(ARGR,sMessage);
}

void R_sysvar(int func)
{
    char *msg = "not yet implemented";

    if (ARGN != 1) {
        Lerror(ERR_INCORRECT_CALL,0);
    }

    LASCIIZ(*ARG1);
    get_s(1);
    Lupper(ARG1);

    if (strcmp((const char*)ARG1->pstr, "SYSUID") == 0) {
        Lscpy(ARGR,environment->SYSUID);
    } else if (strcmp((const char*)ARG1->pstr, "SYSPREF") == 0) {
        Lscpy(ARGR, environment->SYSPREF);
    } else if (strcmp((const char*)ARG1->pstr, "SYSENV") == 0) {
        Lscpy(ARGR,environment->SYSENV);
    } else if (strcmp((const char*)ARG1->pstr, "SYSISPF") == 0) {
        Lscpy(ARGR,environment->SYSISPF);
    } else {
        Lscpy(ARGR,msg);
    }
}

#ifdef __DEBUG__
void R_magic(int func)
{
    void *pointer;
    long decAddr;
    int  count;
    char magicstr[64];

    char option='F';

    if (ARGN>1)
        Lerror(ERR_INCORRECT_CALL,0);
    if (exist(1)) {
        L2STR(ARG1);
        option = l2u[(byte)LSTR(*ARG1)[0]];
    }

    option = l2u[(byte)option];

    switch (option) {
        case 'F':
            pointer = mem_first();
            decAddr = (long) pointer;
            sprintf(magicstr,"%ld", decAddr);
            break;
        case 'L':
            pointer = mem_last();
            decAddr = (long) pointer;
            sprintf(magicstr,"%ld", decAddr);
            break;
        case 'C':
            count = mem_count();
            sprintf(magicstr,"%d", count);
            break;
        default:
            sprintf(magicstr,"%s", "ERROR");
    }

    Lscpy(ARGR,magicstr);
}
#endif

int RxMvsInitialize()
{
    RX_INIT_PARAMS_PTR parameters;

    int      rc     = 0;

    parameters   = malloc(sizeof(RX_INIT_PARAMS));
    environment  = malloc(sizeof(RX_ENVIRONMENT_CTX));

    memset(parameters,00, sizeof(RX_INIT_PARAMS));
    memset(environment,00, sizeof(RX_ENVIRONMENT_CTX));

    parameters->rxctxadr     = (unsigned *)environment;

    rc = call_rxinit(parameters);

    if ((environment->flags3 & _STDIN) == _STDIN) {
        reopen(_STDIN);
    }
    if ((environment->flags3 & _STDOUT) == _STDOUT) {
        reopen(_STDOUT);
    }
    if ((environment->flags3 & _STDERR) == _STDERR) {
        reopen(_STDERR);
    }

    free(parameters);

#ifdef __DEBUG__
    DumpHex((unsigned char*)environment, sizeof(RX_ENVIRONMENT_CTX));
#endif

    return rc;
}

int reopen(int fp) {

    int new_fp, rc = 0;

#ifdef JCC
    _style = "//DDN:";
    switch(fp) {
        case 0x01:
            if (stdin != NULL) {
              fclose(stdin);
            }

            new_fp = _open("STDIN", O_TEXT | O_RDONLY);
            rc = _dup2(new_fp, 0);
            _close(new_fp);

            stdin = fdopen(0,"rt");

            break;
        case 0X02:
            if (stdout != NULL) {
              fclose(stdout);
            }

            new_fp = _open("STDOUT", O_TEXT | O_WRONLY);
            rc = _dup2(new_fp, 1);
            _close(new_fp);

            stdout = fdopen(1,"at");

            break;
        case 0x04:
            if (stderr != NULL) {
              fclose(stderr);
            }

            new_fp = _open("STDERR", O_TEXT | O_WRONLY);
            rc = _dup2(new_fp, 2);
            _close(new_fp);

            stderr = fdopen(2, "at");

            break;
        default:
            rc = ERR_INITIALIZATION;
            break;
    }
#endif
    return 0;

}

void RxMvsRegFunctions()
{
    /* MVS specific functions */
    RxRegFunction("WAIT",    R_wait,   0);
    RxRegFunction("WTO",     R_wto ,   0);
    RxRegFunction("ABEND",   R_abend , 0);
    RxRegFunction("USERID",  R_userid, 0);
    RxRegFunction("LISTDSI", R_listdsi, 0);
    RxRegFunction("MSG",     R_msg,    0);
    RxRegFunction("SYSDSN",  R_sysdsn, 0);
    RxRegFunction("SYSVAR",  R_sysvar, 0);

#ifdef __DEBUG__
    RxRegFunction("MAGIC",  R_magic,  0);
#endif
}

int isTSO() {
    int ret = 0;

    if ((environment->flags2 & _TSOFG) == _TSOFG ||
        (environment->flags2 & _TSOBG) == _TSOBG) {
        ret = 1;
    }

    return ret;
}

int isTSOFG() {
    int ret = 0;

    if ((environment->flags2 & _TSOFG) == _TSOFG) {
        ret = 1;
    }

    return ret;
}

int isTSOBG() {
    int ret = 0;

    if ((environment->flags2 & _TSOBG) == _TSOBG &&
        (environment->flags2 & _TSOFG) != _TSOFG) {
        ret = 1;
    }

    return ret;
}

int isEXEC() {
    int ret = 0;

    if ((environment->flags2 & _EXEC) == _EXEC) {
        /* ret = 1; */
        ret = 0;
    }

    return ret;
}

int isISPF() {
    int ret = 0;

    if ((environment->flags2 & _ISPF) == _ISPF) {
        ret = 1;
    }

    return ret;
}


void parseArgs(char **array, char *str)
{
    int i = 0;
    char *p = strtok (str, " ");
    while (p != NULL)
    {
        array[i++] = p;
        p = strtok (NULL, " ");
    }
}

void parseDCB(FILE *pFile)
{
    unsigned char *flags;
    unsigned char  sDsn[45];
    unsigned char  sDdn[9];
    unsigned char  sMember[9];
    unsigned char  sSerial[7];
    unsigned char  sLrecl[6];
    unsigned char  sBlkSize[6];

    flags = malloc(11);
    __get_ddndsnmemb(fileno(pFile), (char *)sDdn, (char *)sDsn, (char *)sMember, (char *)sSerial, flags);

    /* DSN */
    if (sDsn[0] != '\0')
        setVariable("SYSDSNAME", (char *)sDsn);

    /* DDN */
    if (sDdn[0] != '\0')
        setVariable("SYSDDNAME", (char *)sDdn);

    /* MEMBER */
    if (sMember[0] != '\0')
        setVariable("SYSMEMBER", (char *)sMember);

    /* VOLSER */
    if (sSerial[0] != '\0')
        setVariable("SYSVOLUME", (char *)sSerial);

    /* DSORG */
    if(flags[4] == 0x40)
        setVariable("SYSDSORG", "PS");
    else if (flags[4] == 0x02)
        setVariable("SYSDSORG", "PO");
    else
        setVariable("SYSDSORG", "???");

    /* RECFM */
    if(flags[6] == 0x40)
        setVariable("SYSRECFM", "V");
    else if(flags[6] == 0x50)
        setVariable("SYSRECFM", "VB");
    else if(flags[6] == 0x54)
        setVariable("SYSRECFM", "VBA");
    else if(flags[6] == 0x80)
        setVariable("SYSRECFM", "F");
    else if(flags[6] == 0x90)
        setVariable("SYSRECFM", "FB");
    else if(flags[6] == 0xC0)
        setVariable("SYSRECFM", "U");
    else
        setVariable("SYSRECFM", "??????");

    /* BLKSIZE */
    sprintf((char *)sBlkSize, "%d", flags[8] | flags[7] << 8);
    setVariable("SYSBLKSIZE", (char *)sBlkSize);

    /* LRECL */
    sprintf((char *)sLrecl, "%d", flags[10] | flags[9] << 8);
    setVariable("SYSLRECL", (char *)sLrecl);

    free(flags);
}

void setVariable(char *sName, char *sValue)
{
    Lstr lsScope,lsName,lsValue;

    LINITSTR(lsScope);
    LINITSTR(lsName);
    LINITSTR(lsValue);

    Lfx(&lsScope,sizeof(dword));
    Lfx(&lsName, strlen(sName));
    Lfx(&lsValue, strlen(sValue));

    Licpy(&lsScope,_rx_proc);
    Lscpy(&lsName, sName);
    Lscpy(&lsValue, sValue);

    RxPoolSet(&lsScope, &lsName, &lsValue);

    LFREESTR(lsScope);
    LFREESTR(lsName);
    LFREESTR(lsValue);
}

int GetClistVar(PLstr name, PLstr value)
{
    int rc = 0;
    void *wk;

    RX_IKJCT441_PARAMS_PTR params;

    /* do not handle special vars here */
    if (checkVariableBlacklist(name) != 0)
        return -1;

    /* NAME LENGTH < 1 OR > 252 */
    if (checkNameLength(name->len) != 0)
        return -2;

    params = malloc(sizeof(RX_IKJCT441_PARAMS));
    wk     = malloc(256);

    memset(wk,     0, sizeof(wk));
    memset(params, 0, sizeof(RX_IKJCT441_PARAMS)),

    params->ecode    = 18;
    params->nameadr  = (char *)name->pstr;
    params->namelen  = name->len;
    params->valueadr = 0;
    params->valuelen = 0;
    params->wkadr    = wk;

    rc = call_rxikj441 (params);

    if (value->maxlen < params->valuelen) {
        Lfx(value,params->valuelen);
    }
    if (value->pstr != params->valueadr) {
        strncpy((char *)value->pstr,params->valueadr,params->valuelen);
    }

    value->len    = params->valuelen;
    value->maxlen = params->valuelen;
    value->type   = LSTRING_TY;

    free(wk);
    free(params);

    return rc;
}

int SetClistVar(PLstr name, PLstr value)
{
    int rc = 0;
    void *wk;

    RX_IKJCT441_PARAMS_PTR params;

    /* convert numeric values to a string */
    if (value->type != LSTRING_TY) {
        L2str(value);
    }

    /* terminate all strings with a binary zero */
    LASCIIZ(*name);
    LASCIIZ(*value);

    /* do not handle special vars here */
    if (checkVariableBlacklist(name) != 0)
        return -1;

    /* NAME LENGTH < 1 OR > 252 */
    if (checkNameLength(name->len) != 0)
        return -2;

    /* VALUE LENGTH < 0 OR > 32767 */
    if (checkValueLength(value->len) != 0)
        return -3;

    params = malloc(sizeof(RX_IKJCT441_PARAMS));
    wk     = malloc(256);

    memset(wk,     0, sizeof(wk));
    memset(params, 0, sizeof(RX_IKJCT441_PARAMS)),

            params->ecode    = 2;
    params->nameadr  = (char *)name->pstr;
    params->namelen  = name->len;
    params->valueadr = (char *)value->pstr;
    params->valuelen = value->len;
    params->wkadr    = wk;

    rc = call_rxikj441(params);

    free(wk);
    free(params);

    return rc;
}

/* internal functions */
int checkNameLength(long lName)
{
    int rc = 0;
    if (lName < 1)
        rc = -1;
    if (lName > 252)
        rc =  1;

    return rc;
}

int checkValueLength(long lValue)
{
    int rc = 0;

    if (lValue == 0)
        rc = -1;
    if (lValue > 32767)
        rc =  1;

    return rc;
}

int checkVariableBlacklist(PLstr name)
{
    int rc = 0;
    int i  = 0;

    Lupper(name);

    for (i = 0; i < BLACKLIST_SIZE; ++i) {
        if (strcmp((char *)name->pstr,RX_VAR_BLACKLIST[i]) == 0)
            return -1;
    }

    return rc;
}

/* dummy implementations for cross development */
#ifdef __CROSS__
int call_rxinit(RX_INIT_PARAMS_PTR params)
{
    int rc = 0;

    RX_ENVIRONMENT_CTX_PTR env;

#ifdef __DEBUG__
    printf("DBG> DUMMY RXINIT ...\n");
#endif

    if (params != NULL) {
        if (params->rxctxadr != NULL) {
            env = (RX_ENVIRONMENT_CTX_PTR)params->rxctxadr;
            env->flags1 = 0x0F;
            env->flags2 = 0x07;
            env->flags3 = 0x00;

            strncpy(env->SYSENV,  "DEVEL",   5);
            strncpy(env->SYSPREF, "MIG",     3);
            strncpy(env->SYSUID,  "FIX0MIG", 7);

        } else {
            rc = -42;
        }
    } else {
        rc = -43;
    }
    return rc;
}

unsigned int call_rxikj441 (RX_IKJCT441_PARAMS_PTR params)
{
    char *value = NULL;
    unsigned int rc = 0;

    if (sDummyDictionary.first == NULL) {
        sDummyDictionary.first = (RX_IKJ441_DUMMY_DICT_PTR)malloc(sizeof(RX_IKJ441_DUMMY_DICT));
        memset(sDummyDictionary.first, 0, sizeof(RX_IKJ441_DUMMY_DICT));
    }

    if (params->ecode == 2) {
        addItem(sDummyDictionary.first,params->nameadr,params->valueadr);
#ifdef __DEBUG__
        printf("DBG> DUMMY RXIKJCT441 set %s to %s\n", params->nameadr, params->valueadr);
#endif
    } else if (params->ecode == 18) {
        value = getItem(sDummyDictionary.first, params->nameadr);
        if (value != NULL) {
            params->valueadr = value;
            params->valuelen = strlen(value);
#ifdef __DEBUG__
            printf("DBG> DUMMY RXIKJCT441 returned %s for %s\n", params->valueadr, params->nameadr);
#endif
        } else {
#ifdef __DEBUG__
            printf("DBG> DUMMY RXIKJCT441 found no value for %s\n", params->nameadr);
#endif
            rc = 8;
        }
    }

    return rc;
}

int call_rxptime (RX_PTIME_PARAMS_PTR params)
{
#ifdef __DEBUG__
    if (params != NULL)
        printf("DBG> DUMMY RXPTIME ...\n");

#endif
    return 0;
}

int call_rxstime (RX_STIME_PARAMS_PTR params)
{
#ifdef __DEBUG__
    if (params != NULL)
        printf("DBG> DUMMY RXSTIME ...\n");
#endif
    return 0;
}

int call_rxwto (RX_WTO_PARAMS_PTR params)
{
#ifdef __DEBUG__
    if (params != NULL)
        printf("DBG> DUMMY RXWTO tell %s\n", params->msgadr);
#endif
    return 0;
}

int call_rxwait (RX_WAIT_PARAMS_PTR params)
{
#ifdef __DEBUG__
    if (params != NULL)
        printf("DBG> DUMMY RXWAIT for %d seconds.\n", (*params->timeadr)/100);
#endif
    return 0;
}

unsigned int call_rxabend (RX_ABEND_PARAMS_PTR params)
{
#ifdef __DEBUG__
    if (params != NULL)
        printf("DBG> DUMMY RXABEND with ucc %d\n", params->ucc);
#endif
    return 0;
}

/* dummy dictionary */
char *getItem(RX_IKJ441_DUMMY_DICT_PTR dict, char *key)
{
    RX_IKJ441_DUMMY_DICT_PTR ptr;
    for (ptr = dict; ptr != NULL; ptr = ptr->next) {
        if (ptr->key !=NULL && strcmp(ptr->key, key) == 0) {
            return ptr->value;
        }
    }

    return NULL;
}

void delItem(RX_IKJ441_DUMMY_DICT_PTR dict, char *key)
{
    RX_IKJ441_DUMMY_DICT_PTR ptr, prev;
    for (ptr = dict, prev = NULL; ptr != NULL; prev = ptr, ptr = ptr->next) {
        if (ptr->key != NULL && strcmp(ptr->key, key) == 0) {
            if (ptr->next != NULL) {
                if (prev == NULL) {
                    dict->next = ptr->next;
                } else {
                    prev->next = ptr->next;
                }
            } else if (prev != NULL) {
                prev->next = NULL;
            } else {
                dict->next = NULL;
            }

            free(ptr->key);
            free(ptr);

            return;
        }
    }
}

void addItem(RX_IKJ441_DUMMY_DICT_PTR dict, char *key, char *value)
{
    delItem(dict, key); /* If we already have a item with this key, delete it. */
    RX_IKJ441_DUMMY_DICT_PTR d = (RX_IKJ441_DUMMY_DICT_PTR)malloc(sizeof(RX_IKJ441_DUMMY_DICT));
    d->key = malloc(strlen(key)+1);
    strcpy(d->key, key);
    d->value = value;
    d->next = dict->next;
    dict->next = d;
}

#endif



