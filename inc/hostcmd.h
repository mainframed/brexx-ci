#ifndef BREXX_HOSTCMD_H
#define BREXX_HOSTCMD_H

bool isHostCmd(PLstr cmd, PLstr env);
int  handleHostCmd(PLstr cmd, PLstr env);

#endif //BREXX_HOSTCMD_H
