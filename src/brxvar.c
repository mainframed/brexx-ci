#define __BMEM_H__
#include "lstring.h"
#include "variable.h"
#include "irx.h"

int __libc_arch = 0;

#define	MALLOC(s,d)	    malloc_or_die(s)
#define	REALLOC(p,s)    realloc_or_die(p,s)

#ifndef SVC
#define SVC
struct SVCREGS
{
    unsigned int R0;
    unsigned int R1;
    unsigned int R15;
};
#endif

#ifndef MAX
#	define MAX(a,b)	(((a)>(b))?(a):(b))
#	define MIN(a,b)	(((a)<(b))?(a):(b))
#endif

/* ---------------------------------------------------------- */
/* environment block RXENVBLK                                 */
/* ---------------------------------------------------------- */
typedef  struct envblock RX_ENVIRONMENT_BLK, *RX_ENVIRONMENT_BLK_PTR;

void BRXSVC(int svc, struct SVCREGS *regs);
void _tput(const char *data, int length);
void _itoa(int n, char s[]);
BinTree *findTree(const unsigned char *name);
void Lsccpy( const PLstr to, unsigned char *from );
void *malloc_or_die(size_t size);
void *realloc_or_die(void *ptr, size_t size);

int
GETVAR(unsigned char *name, size_t nameLength, unsigned char *pBuffer, size_t bufferSize, unsigned int *pValueLength)
{
    int rc;
    int	found;

    BinTree *tree;
    PBinLeaf leaf;

    Lstr lName;
    tree = findTree(name);

    if (tree != NULL) {
        lName.pstr = name;
        lName.len  = nameLength;
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
SETVAR(unsigned char *name, size_t nameLength, unsigned char *value, size_t valueLength)
{
    int rc = 0;
    int	found;

    BinTree *tree;
    PBinLeaf leaf;

    Lstr  lName;
    Lstr lValue;
    Lstr	aux;

    Variable *var;

    tree = findTree(name);

    if (tree != NULL) {

        lName.pstr = name;
        lName.len  = nameLength;
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
            //Lsccpy(LEAFVAL(leaf), value);

            rc = 0;
        } else {
            /* added it to the tree */
            /* create memory */
            LINITSTR(aux)
            Lsccpy(&aux,name);
            LASCIIZ(aux)
            tree = findTree(name);
            var = (Variable *) malloc_or_die(sizeof(Variable));
            LINITSTR(var->value)
            Lfx(&(var->value),lValue.len);
            var->exposed = NO_PROC;
            var->stem    = NULL;

            leaf = BinAdd(tree,&aux,var);
            Lstrcpy(LEAFVAL(leaf), &lValue);
            //Lsccpy(LEAFVAL(leaf),value);

            rc = 0;
        }
    } else {
        rc = 12;
    }

    return rc;
}

void
_reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void
_itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    _reverse(s);
}

void *
_getEctEnvBk()
{
    void ** psa;           // PAS      =>   0 / 0x00
    void ** ascb;          // PSAAOLD  => 548 / 0x224
    void ** asxb;          // ASCBASXB => 108 / 0x6C
    void ** lwa;           // ASXBLWA  =>  20 / 0x14
    void ** ect;           // LWAPECT  =>  32 / 0x20
    void ** ectenvbk;      // ECTENVBK =>  48 / 0x30

    psa      = 0;
    ascb     = psa[137];
    asxb     = ascb[27];
    lwa      = asxb[5];

    ect      = lwa[8];
    ectenvbk = ect + 48;

    return ectenvbk;
}

void *
getEnvBlock()
{
    void **ectenvbk;
    RX_ENVIRONMENT_BLK_PTR  envblock;

    ectenvbk = _getEctEnvBk();
    envblock = *ectenvbk;

    if (envblock != NULL) {
        if(strncmp((char *)envblock->envblock_id, "ENVBLOCK", 8) == 0) {
            return envblock;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

BinTree *
findTree(const unsigned char *name) {

    BinTree *tree;

    RX_ENVIRONMENT_BLK_PTR envBlock;

    Scope scope;

    int i = 0;

    int	hashchar[256];	/* use the first char as hash value */

    envBlock = getEnvBlock();

    if (envBlock != NULL) {
        scope = envBlock->envblock_userfield;
        tree = scope;
    } else {
        tree = NULL;
    }

    return tree;
}

/* ---------------- Lfx -------------------- */
void
Lfx( const PLstr s, const size_t len )
{
    size_t	max;

    if (LISNULL(*s)) {
        LSTR(*s) = (unsigned char *) malloc_or_die( (max = LNORMALISE(len))+LEXTRA);
        memset(LSTR(*s),0,max);
        LLEN(*s) = 0;
        LMAXLEN(*s) = max;
        LTYPE(*s) = LSTRING_TY;
    } else
    if (LMAXLEN(*s)<len) {
        LSTR(*s) = (unsigned char *) realloc_or_die( LSTR(*s), (max=LNORMALISE(len))+LEXTRA);
        LMAXLEN(*s) = max;
    }
} /* Lfx */

/* ---------------- Lsccpy ------------------ */
void
Lsccpy( const PLstr to, unsigned char *from )
{
    size_t	len;

    if (!from)
        Lfx(to,len=0);
    else {
        Lfx(to,len = strlen((const char *)from));
        MEMCPY( LSTR(*to), from, len );
    }
    LLEN(*to) = len;
    LTYPE(*to) = LSTRING_TY;
} /* Lsccpy */

/* ---------------- Lstrcpy ----------------- */
void __CDECL
Lstrcpy( const PLstr to, const PLstr from )
{
   if ( LISNULL(*to) ) {
        Lfx(to,31);
    }

    if (LLEN(*from)==0) {
        LLEN(*to) = 0;
        LTYPE(*to) = LSTRING_TY;
    } else {
        if (LMAXLEN(*to)<=LLEN(*from)) Lfx(to,LLEN(*from));
        switch ( LTYPE(*from) ) {
            case LSTRING_TY:
                MEMCPY( LSTR(*to), LSTR(*from), LLEN(*from) );
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
} /* Lstrcpy */

/* ---------------- _Lstrcmp --------------- */
int
_Lstrcmp( const PLstr a, const PLstr b )
{
    int	r;

    if ( (r=MEMCMP( LSTR(*a), LSTR(*b), MIN(LLEN(*a),LLEN(*b))))!=0 )
        return r;
    else {
        if (LLEN(*a) > LLEN(*b))
            return 1;
        else
        if (LLEN(*a) == LLEN(*b)) {
            if (LTYPE(*a) > LTYPE(*b))
                return 1;
            else
            if (LTYPE(*a) < LTYPE(*b))
                return -1;
            return 0;
        } else
            return -1;
    }
} /* _Lstrcmp */

/* ---------------- _getmain --------------- */
void *
_getmain(size_t length) {
    long *ptr;

    struct SVCREGS registers;
    registers.R0 = (unsigned) (length + 12);
    registers.R1 = -1;
    registers.R15 = 0;

    BRXSVC(10, &registers);

    if (registers.R15 == 0) {
        ptr = (void *) registers.R1;
        ptr[0] = 0xDEADBEAF;
        ptr[1] = (((long)(ptr)) + 12);
        ptr[2] = length;
    } else {
        ptr = NULL;
    }
    return (void *) (((int)(ptr)) + 12);
} /* _getmain */

/* -------------- malloc_or_die ---------------- */
void *
malloc_or_die(size_t size)
{
    void *nPtr;

    nPtr = _getmain(size);
    if (!nPtr) {
        _tput("ERR>   GETMAIN FAILED", 21);
       return NULL;
    }

    return nPtr ;
}

/* -------------- realloc_or_die ---------------- */
void *
realloc_or_die(void *oPtr, size_t size)
{
    void *nPtr;

    nPtr = _getmain(size);

    if (!nPtr) {
        _tput("ERR>   GETMAIN FAILED", 21);
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



// TODO: to be removed later
/* -------------------- toThreaded ---------------------- */
static PBinLeaf
toThreaded(PBinLeaf leaf)
{
    if (leaf == NULL) {
        return NULL;
    }

    if (leaf->left == NULL &&
        leaf->right == NULL) {
        return leaf;
    }

    // Find predecessor if it exists
    if (leaf->left != NULL)
    {
        // Find predecessor of root (Rightmost
        // child in left subtree)
        BinLeaf* l = toThreaded(leaf->left);

        // Link a thread from predecessor to
        // root.
        l->right = leaf;
        l->isThreaded = TRUE;
    }

    // If current node is rightmost child
    if (leaf->right == NULL) {
        return leaf;
    }

    // Recur for right subtree.
    return toThreaded(leaf->right);
} /* toThreaded */

/* -------------------- toLinked ------------------------ */
static void
toLinked(BinTree *tree)
{
    PBinLeaf root;
    PBinLeaf current;
    PBinLeaf next;

    if (tree == NULL || tree->parent == NULL) {
        return;
    }

    root = BinMin(tree->parent);

    current = root;
    while ((next = BinSuccessor(current)) != NULL) {
        current->isThreaded = FALSE;
        current->right = next;
        next->left     = current;
        current        = next;
    }

    tree->parent = root;
} /* toLinked */

/* -------------------- LeafConstruct ------------------- */
static BinLeaf *
LeafConstruct( BinLeaf *head, BinLeaf *tail, int n, int *maxdepth )
{
    int	Lmaxd, Rmaxd, i, mid;
    BinLeaf	*Lleaf, *Rleaf, *LMidleaf, *Midleaf, *RMidleaf;

    if (n==0) return NULL;
    if (n==1) {
        /* then head must be equal to tail */
        head->left = NULL;
        head->right = NULL;
        return head;
    }
    if (n==2) {
        (*maxdepth)++;
        head->left = NULL;
        head->right = tail;
        tail->left = NULL;
        tail->right = NULL;
        return head;
    }

    /* --- find middle --- */
    mid = n/2;
    LMidleaf = head;
    for (i=0; i<mid-1; i++)
        LMidleaf = LMidleaf->right;
    Midleaf = LMidleaf->right;
    RMidleaf = Midleaf->right;

    /* --- do the same for left and right branch --- */
    Lmaxd = Rmaxd = *maxdepth+1;

    Lleaf = LeafConstruct(head,LMidleaf,mid,&Lmaxd);
    Rleaf = LeafConstruct(RMidleaf,tail,n-mid-1,&Rmaxd);

    *maxdepth = MAX(Lmaxd, Rmaxd);

    Midleaf->left = Lleaf;
    Midleaf->right = Rleaf;

    return Midleaf;
} /* LeafConstruct */

/* -------------------- BinMin -------------------------- */
PBinLeaf __CDECL
BinMin( PBinLeaf leaf )
{
    if (leaf == NULL) {
        return NULL;
    }

    while (leaf->left != NULL) {
        leaf = leaf -> left;
    }

    return leaf;
} /* BinMin */

/* -------------------- BinMax -------------------------- */
PBinLeaf  __CDECL
BinMax( PBinLeaf leaf )
{
    PBinLeaf tmp;

    if (leaf == NULL) {
        return NULL;
    }

    tmp  = leaf;
    while ((tmp = BinSuccessor(tmp))!= NULL) {
        leaf = tmp;
    }

    return leaf;
} /* BinMax */

/* ------------------ BinAdd ------------------ */
PBinLeaf __CDECL
BinAdd(BinTree *tree, PLstr name, void *dat) {
    BinLeaf *thisEntry;
    BinLeaf *lastEntry;
    BinLeaf *leaf;
    bool leftTaken = FALSE;
    int cmp, dep = 0;

    /* If tree is NULL then it will produce an error */
    thisEntry = tree->parent;
    while (thisEntry != NULL) {
        lastEntry = thisEntry;
        cmp = _Lstrcmp(name, &(thisEntry->key));
        if (cmp < 0) {
            leftTaken = TRUE;
            thisEntry = thisEntry->left;
        } else if (cmp > 0) {
            leftTaken = FALSE;
            if (thisEntry->isThreaded == FALSE) {
                thisEntry = thisEntry->right;
            } else {
                thisEntry = NULL;
            }
        } else {
            return thisEntry;
        }
        dep++;
    }

    /* Create a new entry */
    leaf = (BinLeaf *) MALLOC(sizeof(BinLeaf), "BinLeaf");

    /* just move the data to the new Lstring */
    /* and initialise the name LSTR(*name)=NULL */
    LMOVESTR(leaf->key, *name);

    leaf->value = dat;
    leaf->left = NULL;
    leaf->right = NULL;
    leaf->isThreaded = TRUE;
    if (tree->parent==NULL)
        tree->parent = leaf;
    else {
        if (leftTaken) {
            //leaf->left = lastEntry ->left;
            leaf->right = lastEntry;
            lastEntry->left = leaf;
        }
        else {
            //leaf->left  = lastEntry;
            leaf->right = lastEntry->right;
            lastEntry->right = leaf;
            lastEntry->isThreaded = FALSE;
        }
    }
    tree->items++;

    if (dep>tree->maxdepth) {
        tree->maxdepth = dep;
        if (tree->maxdepth > tree->balancedepth)
            BinBalance(tree);
    }

    return leaf;
} /* BinAdd */

/* ------------------ BinFind ----------------- */
BinLeaf* __CDECL
BinFind( BinTree *tree, PLstr name )
{
    BinLeaf	*leaf;
    int	cmp;

    leaf = tree->parent;
    while (leaf != NULL) {
        cmp = _Lstrcmp(name, &(leaf->key));
        if (cmp < 0)
            leaf = leaf->left;
        else
        if (cmp > 0) {
            if (leaf->isThreaded == FALSE)
                leaf = leaf->right;
            else
                leaf = NULL;
        }
        else
            return leaf;
    }
    return NULL;
} /* BinFind */

/* -------------------- BinSuccessor -------------------- */
PBinLeaf __CDECL
BinSuccessor( PBinLeaf leaf )
{
    if (leaf == NULL)
    {
        return NULL;
    }

    // if isThreaded is set, we can quickly find
    if (leaf->isThreaded == TRUE)
    {
        return leaf->right;
    }

    // else return leftmost child of right subtree
    leaf = leaf->right;
    while (leaf && leaf->left)  // why?? thought of while node -> left != NULL
    {
        leaf = leaf->left;
    }

    return leaf;
} /* BinSuccessor */

/* -------------------- BinBalance ---------------------- */
void __CDECL
BinBalance( BinTree *tree )
{
    PBinLeaf head, tail;
    int	maxDepth = 1;

    head = BinMin(tree->parent);
    tail = BinMax(tree->parent);

    /* first we convert the tree to an sorted double linked list */
    toLinked(tree);

    /* now we have to balance / reconstruct the tree */
    tree->parent = LeafConstruct( head, tail, tree->items, &maxDepth );
    tree->maxdepth = maxDepth;
    tree->balancedepth = maxDepth + BALANCE_INC;

    /* finally we have to all threads */
    toThreaded(tree->parent);
} /* BinBalance */

void _tput(const char *data, int length)
{
    struct SVCREGS registers;
    registers.R0  = length;
    registers.R1  = (unsigned int) data & 0x00FFFFFF;
    registers.R15 = 0;

    BRXSVC(93, &registers);
}

#ifdef __CROSS__
void BRXSVC(int svc, struct SVCREGS *regs) {

}
#endif