#ifdef __CROSS__
#include <string.h>

int rac_user_auth(char *userName, char *password)
{
    int rc = 0;
    if ( (strcmp(userName, "user1") == 0) && (strcmp(password, "pass1") == 0) )
        rc = 1;

    return rc;
}

#endif
