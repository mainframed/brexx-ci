#include <ctype.h>
#include <stdio.h>
#include "bintree.h"

BinLeaf*
leftMostLeaf( BinLeaf *node )
{
    if (node == NULL) {
        return NULL;
    }

    while (node -> left != NULL) {
        node = node -> left;
    }

    return node;
}

BinLeaf*
nextLeaf( BinLeaf *node )
{
    if (node == NULL)
    {
        return NULL;
    }

    // if isThreaded is set, we can quickly find
    if (node->isThreaded == TRUE)
    {
        return node->right;
    }

    // else return leftmost child of right subtree
    node = node -> right;
    while (node && node->left)  // why?? thought of while node -> left != NULL
    {
        node = node->left;
    }
    return node;
}

void
printBinTree(BinLeaf *root)
{
    if (root == NULL)
        printf("Tree is empty");

    // Reach leftmost node
    BinLeaf *ptr = leftMostLeaf(root);

    // One by one print successors
    while (ptr != NULL)
    {
        printf(" %s \n",LSTR((Lstr )(ptr->key)));
        ptr = nextLeaf(ptr);
    }
}

int main()
{
    printf("FOO> A test function!");
}
