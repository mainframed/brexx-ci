/*
 * Binary Tree
 *  ~~~~~~ ~~~~
 * Very general purpose routines for binary tree implementation.
 * Each leaf contains a (PLstr)key with the name of the leaf
 * and a (void*)value which contains the value of the leaf.
 *
 * The searching is done with the key's checked with _Lstrcmp
 * that means that an INTEGER or a REAL is stored according
 * to its binary representation in memory.
 *
 * When adding a leaf no memory allocation is done for the key
 * and the value.
 */

#include <stdio.h>
#include <string.h>
#include "os.h"
#include "bmem.h"
#include "variable.h"
#include "bintree.h"

/* ------------------ BinAdd ------------------ */
BinLeaf* __CDECL
BinAdd( BinTree *tree, PLstr name, void *dat )
{
	BinLeaf	*thisEntry;
	BinLeaf	*lastEntry;
	BinLeaf	*leaf;
	bool	leftTaken=FALSE;
	int	cmp, dep=0;

	/* If tree is NULL then it will produce an error */
	thisEntry = tree->parent;
	while (thisEntry != NULL) {
		lastEntry = thisEntry;
		cmp = _Lstrcmp(name,&(thisEntry->key));
		if (cmp < 0) {
            leftTaken = TRUE;
            thisEntry = thisEntry->left;
		} else
		if (cmp > 0) {
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
	leaf = (BinLeaf *) MALLOC( sizeof(BinLeaf),"BinLeaf" );

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

/* ----------------- BinDisposeLeaf -------------------- */
void __CDECL
BinDisposeLeaf( BinTree *tree, BinLeaf *leaf,
		void (__CDECL *BinFreeData)(void *) )
{
	if (!leaf) {
	    return;
	}

    if (leaf->left) {
        BinDisposeLeaf(tree,leaf->left,BinFreeData);
    }

    if (leaf->isThreaded == FALSE && leaf->right) {
        BinDisposeLeaf(tree,leaf->right,BinFreeData);
    }

    LFREESTR(leaf->key);
	if (leaf->value && BinFreeData)
		BinFreeData(leaf->value);

	if (leaf==tree->parent)
		tree->parent = NULL;
	FREE(leaf);
	tree->items--;
} /* BinDisposeLeaf */

/* ----------------- BinDisposeTree -------------------- */
void __CDECL
BinDisposeTree( BinTree *tree, void (__CDECL *BinFreeData)(void *) )
{
	if (tree != NULL) {
		BinDisposeLeaf(tree,tree->parent,BinFreeData);
		FREE(tree);
	}
} /* BinDisposeTree */

/* -------------------------------------------------------------- */
/* To correctly delete a pointer from a Binary tree we must not   */
/* change the tree structure, that is the smaller values are the  */
/* left most. In order to satisfy this with few steps we must     */
/* replace the pointer that is to be erased with the one which is */
/* closest with a smaller value (the right most from the left     */
/* branch, as you can see below                                   */
/*                     ...                                        */
/*                    /                                           */
/*                 (name)   <-- to be dropped                     */
/*                 /    \                                         */
/*              (a)      (d)       (c)= newid from left branch    */
/*             /   \       ...          where  c->right=NULL      */
/*          ...     (c)            (a)= par_newidt parent of newid*/
/*                 /   \                                          */
/*              (b)     NIL       newid will become the new       */
/*             ...                sub-head when (name) is dropped */
/*                    |                                           */
/*                    |                                           */
/*                   \|/                                          */
/*                    V   ...                                     */
/*                      /                                         */
/*                    (c)           but in the case that          */
/*                   /   \          (a)=(c) when a->right = NULL  */
/*                 (a)    (d)       then the tree is very simple  */
/*                /   \    ....     we simply replace a->right=d  */
/*              ...   (b)                                         */
/*                    ....                                        */
/*                                                                */
/* -------------------------------------------------------------- */
void __CDECL
BinDel( BinTree *tree, PLstr name, void (__CDECL *BinFreeData)(void *) )
{
	BinLeaf	*thisid, *previous=NULL, *par_newid, *newid;
	bool	lefttaken=FALSE;
	int	cmp;

	thisid = tree->parent;
	while (thisid != NULL) {
		cmp=_Lstrcmp(name,&(thisid->key));
		if (cmp < 0) {
			previous = thisid;
			thisid = thisid->left;
			lefttaken = TRUE;
		} else
		if (cmp > 0) {
			previous = thisid;
			thisid = thisid->right;
			lefttaken = FALSE;
		} else
			break;
	}
	if (thisid == NULL) return;	/* Not Found */

	if (thisid->right == NULL)
		newid = thisid->left;
	else
	if (thisid->left == NULL)
		newid = thisid->right;
	else {		/* when no node is empty */

		/* find the right most id of the */
		/* left branch of thisid         */

		par_newid = thisid;
		newid = thisid->left;
		while (newid->right != NULL) {
			par_newid = newid;
			newid = newid->right;
		}

		/* newid must now replace thisid */
		newid->right = thisid->right;

		if (par_newid != thisid) {
			par_newid->right = newid->left;
			newid->left = thisid->left;
		}
	}

	if (thisid == tree->parent)
		tree->parent = newid;
	else {
		if (lefttaken)
			previous->left = newid;
		else
			previous->right = newid;
	}
	thisid->left = NULL;
	thisid->right = NULL;
	BinDisposeLeaf(tree,thisid,BinFreeData);
} /* BinDel */

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

/* -------------------- BinPrint ------------------------ */
static bool BinPrintStem=FALSE;
void __CDECL
BinPrint(PBinLeaf leaf )
{
    PBinLeaf ptr;
    int i = 0;

    if (leaf == NULL) {
        printf("Tree is empty");
        return;
    }

    // Reach leftmost node
    ptr = BinMin(leaf);

    // One by one print successors
    while (ptr != NULL)
    {
        if (BinPrintStem == TRUE ) {
           switch (LTYPE(ptr->key)) {
               case LINTEGER_TY:
                   printf("[%04d]  \"|.%ld\" => ", ++i, LINT (ptr->key));
                   break;
               case LREAL_TY:
                   printf("[%04d]  \"|.%f\" => ", ++i, LREAL(ptr->key));
                   break;
               case LSTRING_TY:
                   printf("[%04d]  \"|.%s\" => ", ++i, LSTR (ptr->key));
                   break;
           }
        }  else {
           switch (LTYPE(ptr->key)) {
               case LINTEGER_TY:
                  printf("[%04d] \"%ld\" => ", ++i, LINT (ptr->key));
                  break;
                  case LREAL_TY:
                  printf("[%04d] \"%f\" => ", ++i, LREAL(ptr->key));
                  break;
               case LSTRING_TY:
                  printf("[%04d] \"%s\" => ", ++i, LSTR (ptr->key));
                  break;
           }
        }
        if (ptr->value) {

            Variable *var = (Variable *)ptr->value;
            if (var->stem) {
                printf("\n");
                BinPrintStem=TRUE;
                BinPrint(var->stem->parent);
                BinPrintStem=FALSE;
            } else {
                switch (LTYPE(*(Lstr *)ptr->value)) {
                    case LINTEGER_TY:
                        printf("\"%ld\" \n",LINT(*(PLstr) ptr->value));
                        break;
                    case LREAL_TY:
                        printf("\"%f\" \n",LREAL(*(PLstr) ptr->value));
                        break;
                    case LSTRING_TY:
                        printf("\"%s\" \n",LSTR (*(PLstr) ptr->value));
                        break;
                }
            }

        }

        ptr = BinSuccessor(ptr);
    }
} /* BinPrint */