#include "rexx.h"
#include "hostcmd.h"
#include "lstring.h"
#include "bintree.h"
#include "stack.h"
#include "rxmvsext.h"
#include "fss.h"

#define VALLOC(VN,VL) { (VN)=(char *)malloc((VL)); \
                        memset((VN),0,(VL)); }
#define VFREE(VN) { free((VN)); }
#define VINIT(VN,VL) { memset((VN),0,(VL)); }
#define SNULL(V) { memset((V),0,strlen((V))); }

int RxEXECIO();

char *hcmdargvp[128];
char  hcmdargcp;
char  erx_panel[18];
int   erx_float=0;
int   erx_flrow=0;
int   erx_flcol=0;
short erx_vlist=0;
short erx_trim=0;
short erx_read=0;
short erx_border=0;

struct panelflds {
    char *name;
    int   attr;
};

struct panellvl {
    int  iterate;
};

struct panellvl pnllvl[6];
int    pnllvlix;

struct panelflds pnlfld[128];
int    pnlfldix;
int    floatx=0;
int    floaty=0;
int    pnlw=0;
int    pnlh=0;
int    paninit=0;
int    panend=0;

typedef char  BYTE;

// Field Definition
struct sFields
{
    char *name;            // Field Name or Null String for TXT fields
    char *pname;
    int   bufaddr;         // Field location - offset into 3270 buffer
    int   attr;            // Attribute values
    int   length;          // Field Length
    short typef;
    short resetf;
    short floatf;
    char  sattr[4];
    char *data;            // Field Data
};

char *
trimr(char *strim)
{
    char *ptmp=0;
    ptmp=strim+strlen(strim)-1;
    while((ptmp>=strim)&&(*ptmp==' ')) {
        *ptmp-='\0';
    }
    return strim;
}

void
remlf(char *s)
{
    char *pos;
    if((pos=strchr(s,'\n'))!=NULL) {
        *pos='\0';
    }
    return;
}

void
clearcmd(char *hcmdargvp[])
{
    int i=0;
    for(i=0;i<=128;i++) {
        hcmdargvp[i]=NULL;
    }
    hcmdargcp=0;
    return;
}

int
parsecmd(char scmd[256],char *hcmdargvp[])
{
    int lidx;
    lidx=0;
    clearcmd(hcmdargvp);
    hcmdargvp[lidx]=strtok(scmd," (),");
    while(hcmdargvp[lidx]!=NULL) {
        lidx++;
        hcmdargvp[lidx]=strtok(NULL," (),");
    }
    if(lidx==0) { hcmdargvp[lidx]=(char *)&scmd; }
    return(lidx);
}

int
findcmd(char scmd[255],char *hcmdargvp[])
{
    int lidx=0;
    while(hcmdargvp[lidx]!=NULL) {
        if(strcasecmp(scmd,hcmdargvp[lidx])==0) {
            return(lidx);
        }
        lidx++;
    }
    return(-1);
}

char *
getstem(char *vname,int ix)
{
    char sname[20];

    if(ix<0) { return NULL; }
    sprintf(sname,"%s%i",vname,ix);
    return((char *)getStemVariable(sname));

}

void
setstem(char *vname,int ix,char *data)
{

    char sname[20];
    sprintf(sname,"%s%i",vname,ix);
    setVariable(sname,data);

}

void
rxqueue(char *s)
{
    PLstr pstr;

    LPMALLOC(pstr);
    Lscpy(pstr,s);
    Queue2Stack(pstr);
    return;
}

long
rxqueued()
{
    return(StackQueued());
}


char *
rxpull()
{
    PLstr pstr;
    pstr=PullFromStack();
    return(LSTR(*pstr));
}

char *
copies(char *str, size_t count)
{
    char *ret;
    if(count==0) return NULL;
    ret=malloc(strlen (str) * count + count);
    if(ret==NULL) return NULL;
    strcpy (ret, str);
    while (--count > 0) {
        strcat (ret, " ");
        strcat (ret, str);
    }
    return ret;
}

void
getsba(char *psba,char *srowcol)
{
    char sba[2];
    int  sbacol=0;
    int  sbarow=0;
    int  sbaba1=0;
    int  sbaba2=0;
    int  sbaaddr=0;

    memcpy(sba,psba,1);
    memcpy(sba+1,psba+1,1);
    sbaba1=sba[0] & 0x3f;
    sbaba2=sba[1] & 0x3f;
    sbaaddr=sbaba1*64+sbaba2;
    sbarow=sbaaddr/80+1;
    sbacol=sbaaddr%80+1;
    sprintf(srowcol,"%02d %02d",sbarow,sbacol);

    return;
}

int __CDECL
parsemap(char *pname)
{
    struct sFields *nf;
    int   ii;
    int   ij;
    int   iy;
    int   ix;
    int   jj;
    int   istem;
    char  fldstr[19];
    char  fldatr[6];
    char  sattr[4];
    char *fldptr;
    char *vszmap;
    char *vtmp;
    char *dtmp;
    char *s;
    char *sl;
    char  b[1];
    char *dtxt;
    char *dhdr=0;
    char *dtrl=0;
    int   txtpos=0;
    int   fldpos=0;
    int   mrkpos=0;
    int   sizepos=0;
    int   fldix=0;
    short ftype=0;
    int   fattr=0;
    char *vdata;
    char *vnattr;
    char  cursor[19];
    char  vn1[19];
    int   aid;
    char  spname[19];
    char  editstr[256];
    int   nrow;
    int   ncol;
    char  saidpos[6];
    int   zfld=0;

    VALLOC(vtmp,81)
    VALLOC(dtxt,81)
    VALLOC(dhdr,79)
    VALLOC(dtrl,79)

    // defaults
    erx_trim=0;
    if(strcasecmp((char *)getVariable("ZSTRIP"),"ON")==0) {
        erx_trim=1;
    }

    memset(erx_panel,0,sizeof(erx_panel));
    strcpy(erx_panel,(char *)getStemVariable("ZPANEL"));

    memset(cursor,0,sizeof(cursor));
    strcpy(cursor,(char *)getStemVariable("ZCURSOR"));

    if(erx_float==0) {                // initialize for base panel
        fssReset();
        fssInit();
        pnlfldix=0;
        pnllvlix++;
        pnllvl[pnllvlix].iterate=0;
    } else {                          // increment array for float panel
        if(pnllvl[pnllvlix].iterate==1) {
            pnllvlix++;
            pnllvl[pnllvlix].iterate=0;
        }
    }

    if(pnllvl[pnllvlix].iterate>0) {  // depends on number of converse
        ii=fssAFields();
        for(jj=0;jj<ii;jj++) {
            nf=(struct sFields *)fssAField(jj);
            if(strcasecmp(pname,nf->pname)==0&&nf->typef==2) {
                vdata=(char *)getStemVariable(nf->name);
                if(vdata==NULL||strlen(vdata)==0) {
                    strcpy(editstr,copies(" ",nf->length));
                    strcpy(nf->data,editstr);
                } else {
                    SNULL(editstr);
                    sprintf(editstr,"%s%s",
                            vdata,
                            copies(" ",nf->length-strlen(vdata))
                    );
                    strcpy(nf->data,editstr);
                }
            }
        }
        goto skip_to_refresh;           // a new parse is not necessary
    }

    fssSetPanel(pname);

    sprintf(spname,"%s.0",pname);     // retrieve total of records in stem
    istem=getIntegerVariable(spname);

    // prepare border for popup
    //if(pnlw>0&&pnlh>0) {
    if(erx_border==1) {
        SNULL(dtxt);
        SNULL(dhdr);
        SNULL(dtrl);
        SNULL(sattr);
        strcpy(sattr,"7PR");
        fattr=fssPROT+fssREVERSE+fssWHITE;
        // get header title for popup window
        strcpy(dhdr,(char *)getVariable("ZHDR"));
        memset(dtxt,' ',pnlw+2);
        memset(dtxt+(pnlw+2)+1,0,1);
        if(strlen(dhdr)>0) {
            memcpy(dtxt+1,dhdr,strlen(dhdr));
        }
        fssTxa(floaty,floatx-1,sattr,dtxt);
        // get trailer title for popup window
        SNULL(dtxt);
        strcpy(dtrl,(char *)getVariable("ZTRL"));
        memset(dtxt,' ',pnlw+2);
        memset(dtxt+(pnlw+2)+1,0,1);
        if(strlen(dhdr)>0) {
            memcpy(dtxt+1,dtrl,strlen(dtrl));
        }
        fssTxa(floaty+pnlh+1,floatx-1,sattr,dtxt);
        // fill window borders
        memset(dtxt,' ',1);
        memset(dtxt+1,0,1);
        for(ii=floaty+1;ii<=floaty+pnlh;ii++) {
            fssTxa(ii,floatx-1,sattr,dtxt);
            fssTxa(ii,floatx+pnlw,sattr,dtxt);
        }
    }

    for(ii=1;ii<=istem;ii++) {

        VALLOC(sl,80);
        VALLOC(s,80);

        sprintf(vtmp,"%s.%d",pname,ii);
        s=(char *)getStemVariable(vtmp);
        sl=s;

        txtpos=-1;
        fldpos=-1;

        for(ix=0;ix<strlen(s);ix++) {
            strncpy(b,sl,1);
            b[1]='\0';

            if(strncmp(b,"#",1)==0) {
                if(txtpos>0) {
                    sizepos=ix-txtpos;
                    memset(dtxt,0,81);
                    memcpy(dtxt,(s+txtpos),sizepos);
                    fssTxa(floaty+ii,floatx+txtpos,sattr,dtxt);
                }
                strcpy(sattr,"1PD");
                txtpos=ix+1;
                fldpos=-1;
                ftype=1;
            }

            if(strncmp(b,"%",1)==0) {
                if(txtpos>0) {
                    sizepos=ix-txtpos;
                    memset(dtxt,0,81);
                    memcpy(dtxt,s+txtpos,sizepos);
                    fssTxa(floaty+ii,floatx+txtpos,sattr,dtxt);
                }
                strcpy(sattr,"7PD");
                txtpos=ix+1;
                fldpos=-1;
                ftype=2;
            }

            if(strncmp(b,"_",1)==0) {
                fldix++;
                if(txtpos>0) {
                    sizepos=ix-txtpos;
                    memset(dtxt,0,81);
                    memcpy(dtxt,s+txtpos,sizepos);
                    fssTxa(floaty+ii,floatx+txtpos,sattr,dtxt);
                    txtpos=-1;
                    ftype=0;
                }
                if(fldpos>0) {
                }
                fattr=0;
                strcpy(sattr,"***");
                txtpos=-1;
                fldpos=ix+1;
                ftype=0;
            }

            if(strncmp(b,"@",1)==0) {
                fldix++;
                if(txtpos>0) {
                    sizepos=ix-txtpos;
                    memset(dtxt,0,81);
                    memcpy(dtxt,s+txtpos,sizepos);
                    fssTxa(floaty+ii,floatx+txtpos,sattr,dtxt);
                    txtpos=-1;
                    ftype=0;
                }
                if(fldpos>0) {
                }
                strcpy(sattr,"6PD");
                txtpos=-1;
                fldpos=ix+1;
                ftype=0;
            }

            if(strncmp(b,"+",1)==0) {
                if(txtpos>0) {
                    sizepos=ix-txtpos;
                    memset(dtxt,0,81);
                    memcpy(dtxt,s+txtpos,sizepos);
                    fssTxa(floaty+ii,floatx+txtpos,sattr,dtxt);
                }
                if(fldpos>0) {
                    sizepos=ix-fldpos;
                    memset(dtxt,0,81);
                    memcpy(dtxt,s+fldpos,sizepos);
                    dtxt=fssTrim(dtxt);
                    if(strcasecmp(dtxt,"Z")==0) {
                        zfld++;
                        sprintf(vn1,"_%s_lst.%d",erx_panel,zfld);
                        strcpy(dtxt,(char *)getVariable(vn1));
//          strcpy(dtxt,(char *)getstem("ZVARS.",zfld));
                    }
                    VALLOC(pnlfld[pnlfldix].name,18);
                    strcpy(pnlfld[pnlfldix].name,dtxt);

                    // next field in field array
                    pnlfldix++;
                    vdata=(char *)getStemVariable(dtxt);
                    memset(fldstr,0,sizeof(fldstr));
                    sprintf(fldstr,"_%s",dtxt);
                    strcpy(fldatr,(char *)getStemVariable(fldstr));

                    // consider field and its attributes
//        fattr=0;
//        if(fldatr[1]=='U') {
//          fattr=0;
//        }
//        if(fldatr[1]=='P') {
//          fattr=fattr+fssPROT;
//        }
//        if(fldatr[1]=='B') {
//          fattr=fattr+fssPROT+fssHI;
//        }
//        if(fldatr[1]=='H') {
//          fattr=fattr+fssHI;
//        }
//        if(fldatr[1]=='D') {
//          fattr=fssNON;
//        }
//        if(fldatr[1]=='N') {
//          fattr=fssNUM;
//        }
//        if(fldatr[0]=='0') {
//          fattr=0;
//        }
//        if(fldatr[0]=='1') {
//          fattr=fattr+fssBLUE;
//        }
//        if(fldatr[0]=='2') {
//          fattr=fattr+fssRED;
//        }
//        if(fldatr[0]=='3') {
//          fattr=fattr+fssPINK;
//        }
//        if(fldatr[0]=='4') {
//          fattr=fattr+fssGREEN;
//        }
//        if(fldatr[0]=='5') {
//          fattr=fattr+fssTURQ;
//        }
//        if(fldatr[0]=='6') {
//          fattr=fattr+fssYELLOW;
//        }
//        if(fldatr[0]=='7') {
//          fattr=fattr+fssWHITE;
//        }
//        if(fldatr[2]=='D') {
//          fattr=fattr+fssREVERSE;
//        }
//        if(fldatr[2]=='R') {
//          fattr=fattr+fssREVERSE;
//        }
//        if(fldatr[2]=='U') {
//          fattr=fattr+fssUSCORE;
//        }
//        if(fldatr[2]=='B') {
//          fattr=fattr+fssBLINK;
//        }
                    // aki
                    fattr=0;
                    if(vdata==NULL||strlen(vdata)==0) {
                        fssFla(floaty+ii,floatx+fldpos,fattr,dtxt,sizepos,
                               copies(" ",sizepos),
                               fldatr
                        );
                    } else {
                        SNULL(editstr);
                        if(strlen(vdata)<sizepos) {
                            sprintf(editstr,"%s%s",
                                    vdata,
                                    copies(" ",sizepos-strlen(vdata))
                            );
                            fssFla(floaty+ii,floatx+fldpos,fattr,dtxt,sizepos,editstr,
                                   fldatr
                            );
                        } else {
                            fssFla(floaty+ii,floatx+fldpos,fattr,dtxt,sizepos,vdata,
                                   fldatr
                            );
                        }
                    }
                }
                mrkpos=ix;
                txtpos=-1;
                fldpos=-1;
                mrkpos=0;
                ftype=0;
                fattr=0;
            }

            *sl++;
        }

        VFREE(s)
        VFREE(sl)

    }

    //printf("-- parsemap w=%d h=%d\n",pnlw,pnlh);

    skip_to_refresh:

    // cursor set based on variable name
    fssCursor(cursor);

    // refresh current 3270 area and make the bew buffer
    fssRefresh2(erx_read);

    pnllvl[pnllvlix].iterate++;

    aid=fssGetAID();
    if(aid==fssPFK01) {
        setVariable("AID","PF01");
    }
    if(aid==fssPFK02) {
        setVariable("AID","PF02");
    }
    if(aid==fssPFK03) {
        setVariable("AID","PF03");
    }
    if(aid==fssPFK04) {
        setVariable("AID","PF04");
    }
    if(aid==fssPFK05) {
        setVariable("AID","PF05");
    }
    if(aid==fssPFK06) {
        setVariable("AID","PF06");
    }
    if(aid==fssPFK07) {
        setVariable("AID","PF07");
    }
    if(aid==fssPFK08) {
        setVariable("AID","PF08");
    }
    if(aid==fssPFK09) {
        setVariable("AID","PF09");
    }
    if(aid==fssPFK10) {
        setVariable("AID","PF10");
    }
    if(aid==fssPFK11) {
        setVariable("AID","PF11");
    }
    if(aid==fssPFK12) {
        setVariable("AID","PF12");
    }
    if(aid==fssENTER) {
        setVariable("AID","ENTER");
    }
    if(aid==fssCLEAR) {
        setVariable("AID","CLEAR");
    }

    // compute sba returned from tget buffer
    nrow=fssFieldPos()/80+1;
    ncol=fssFieldPos()%80+1;
    sprintf(saidpos,"%02d %02d",nrow,ncol);
    setVariable("AIDPOS",saidpos);

    // setting rexx variables based on panel IO area
    for(ii=0;ii<pnlfldix;ii++) {
        //printf("-- %03d %18s s=%d d=%s\n",ii,
        //                                  pnlfld[ii].name,
        //                                  erx_trim,
        //                                  fssGetData(pnlfld[ii].name)
        //                                  );
        //  nf=(struct sFields *)fssGetStru(pnlfld[ii].name);
        //  rxsvar(pnlfld[ii].name,nf->data);
        if(erx_trim==0) {
            setVariable(pnlfld[ii].name,fssGetData(pnlfld[ii].name));
        } else {
            setVariable(pnlfld[ii].name,
                        fssTrim((char *)fssGetData(pnlfld[ii].name))
            );
        }
        VFREE(pnlfld[ii].name)
    }

    // local gargabe collector
    VFREE(vtmp)
    VFREE(dtxt)
    VFREE(dhdr)
    VFREE(dtrl)
    VFREE(s)
    VFREE(sl)

    return(0);
}

void
setmapatr(int func,char *pname,char ccolor,char cattr,char cext) {

    struct sFields *nf;
    int    ii;
    int    jj;
    char   vname[20];

    if(func==1) {

        ii=fssAFields();
        for(jj=0;jj<ii;jj++) {
            nf=(struct sFields *)fssAField(jj);
            if(nf->typef==2) {
                if(strcasecmp(pname,nf->pname)==0) {
                    if(cattr!=NULL) {
                        nf->sattr[1]='P';
                    }
                    fssSetAttr(nf->name,fssPROT);
                    //printf("n=%d t=%d n=%s l=%d a=%s l=%d\n",
                    //       jj,
                    //       nf->typef,
                    //       nf->name,
                    //       nf->length,
                    //       nf->sattr,
                    //       nf->length
                    //      );
                }
            }
        }
    }

}

int
RxForceRC(int rc) {
    RxSetSpecialVar(RCVAR,rc);
    return(rc);
}

int
aFunc(PLstr cmd, PLstr env)
{
    PBinLeaf l;
    Lstr str;
    PLstr pstr;
    int localrc;
    int istem;
    int ii;
    int jj;
    int lclattr = 0;
    PLstr vd;
    char lclcmd[1025];
    char cursor[9];
    char ddname[9];
    char str1[64];
    unsigned char pbuff[1025];
    unsigned char vname1[19];
    unsigned char vname2[19];
    unsigned char vname3[19];
    unsigned char pname[19];
    unsigned char vdata[1025];
    unsigned char obuff[4097];
    char sattr[3];      // attribute sequence
    int ip1 = 0;
    int ip2 = 0;
    int ip3 = 0;
    int ip4 = 0;
    int ip5 = 0;
    int iflag1 = 0;
    int iflag2 = 0;
    int sleepv = 0;
    int recs = 0;
    int vars = 0;
    int fgdbg = 0;
    long lrecs = 0;
    FILE *f;
    struct sFields *nf;

    rxReturnCode = 0;

    strcpy(lclcmd, LSTR(*cmd));
    hcmdargcp = parsecmd(lclcmd, hcmdargvp);

    if (strcasecmp(hcmdargvp[0], "TEST") == 0) {
        printf("stack=%i\n", rxqueued());
        //for(ii=1;ii<=queued();ii++) {
        //  printf(">>%s\n",rxpull());
        //}
        return (RxForceRC(0));
    }

    if (strcmp(LSTR(*cmd), "LOCK") == 0) {
        ii = fssAFields();
        for (jj = 0; jj < ii; jj++) {
            nf = (struct sFields *) fssAField(jj);
            if (nf->typef == 2) {
                nf->sattr[1] = 'P';
                printf("n=%d t=%d n=%s l=%d a=%s l=%d\n",
                       jj,
                       nf->typef,
                       nf->name,
                       nf->length,
                       nf->sattr,
                       nf->length
                );
            }
        }
        return (RxForceRC(0));
    }

    if (strcmp(LSTR(*cmd), "DUMP") == 0) {
        ii = fssAFields();
        for (jj = 0; jj < ii; jj++) {
            nf = (struct sFields *) fssAField(jj);
            printf("n=%03d t=%d p=%-8s n=%-10s l=%05d a=%3s d=%s\n",
                   jj,
                   nf->typef,
                   nf->pname,
                   nf->name,
                   nf->length,
                   nf->sattr,
                   nf->data
            );
        }
        return (RxForceRC(0));
    }

    if (strcasecmp(hcmdargvp[0], "SET") == 0) {
        if (strcasecmp(hcmdargvp[2], "LOCK") == 0) {
            setmapatr(1, hcmdargvp[1], NULL, 'P', NULL);
            return (RxForceRC(0));
        }
        if (strcasecmp(hcmdargvp[2], "UNLOCK") == 0) {
            setmapatr(1, hcmdargvp[1], NULL, 'U', NULL);
            return (RxForceRC(0));
        }
        if (strcasecmp(hcmdargvp[1], "CURSOR") == 0) {
            setVariable("ZCURSOR", hcmdargvp[2]);
            return (RxForceRC(0));
        }
        if (strcasecmp(hcmdargvp[1], "PANLIB") == 0) {
            setVariable("ZPANLIB", hcmdargvp[2]);
            sprintf(str1, "//DDN:%s", hcmdargvp[2]);
            f = fopen(str1, "r");
            if (f == NULL) {
                return (RxForceRC(8));
            }
            recs = 0;
            iflag1 = 0;       // section control
            iflag2 = 0;       // proc control
            while (fgets(pbuff, 1024, f)) {
                remlf(&pbuff[0]);
                if (pbuff[0] == ')') {
                    hcmdargcp = parsecmd(pbuff, hcmdargvp);
                    if (strcasecmp(hcmdargvp[0], "PANEL") == 0) {
                        strcpy(pname, hcmdargvp[1]);
                        iflag1 = 0;
                        iflag2 = 0;
                        recs = 0;
                    }
                    if (strcasecmp(hcmdargvp[0], "BODY") == 0) {
                        iflag1 = 1;                     // enable panel scan
                        recs = 0;
                        vars = 0;
                    }
                    if (strcasecmp(hcmdargvp[0], "PROC") == 0) {
                        iflag2 = 1;
                        iflag1 = 0;
                    }
                    if (strcasecmp(hcmdargvp[0], "END") == 0) {
                        iflag1 = 0;                     // disable panel scan
                        iflag2 = 0;
                    }
                    if (strcasecmp(hcmdargvp[0], "PROC") == 0 ||
                        strcasecmp(hcmdargvp[0], "END") == 0) {
                        if (recs > 0) {
                            sprintf(vname1, "%s.0", pname, recs);
                            setIntegerVariable(vname1, recs);
                        }
                    }
                } else {
                    if (iflag1 == 1) {
                        recs++;
                        sprintf(vname2, "%s.%d", pname, recs);
                        setVariable(vname2, pbuff);
                    }
                    if (iflag2 == 1) {
                        hcmdargcp = parsecmd(pbuff, hcmdargvp);
                        if (strcasecmp(hcmdargvp[0], "FIELDS") == 0) {
                            for (ii = 1; ii <= hcmdargcp - 1; ii++) {
                                vars++;    // keep zlist list incremented
                                sprintf(vname1, "_%s_lst.%d", pname,
                                        vars);
                                setVariable(vname1, hcmdargvp[ii]);
                            }
                            sprintf(vname1, "_%s_lst.0", pname);
                            setIntegerVariable(vname1, vars);
                        }
                    }
                }
            }
            fclose(f);
            return (RxForceRC(0));
        }
        if (strcasecmp(hcmdargvp[1], "DEBUG") == 0) {
            for (ii = 2; ii <= hcmdargcp; ii++) {
                if (strcasecmp(hcmdargvp[ii], "PANEL") == 0) {
                    fssDebug(2);
                }
            }
            return (RxForceRC(0));
        }
        if (strcasecmp(hcmdargvp[1], "PANEL") == 0) {
            fssSetPanel(hcmdargvp[2]);
            setVariable("ZPANEL", hcmdargvp[2]);
            return (RxForceRC(0));
        }
        // default values
        lclattr = 0;
        sattr[0] = '1';
        sattr[1] = 'U';
        sattr[2] = 'D';
        sattr[3] = NULL;
        // attribute sequence for panel resources
        ip1 = findcmd("NOCOLOR", hcmdargvp);
        if (ip1 > 0) { sattr[0] = '0'; }
        ip1 = findcmd("DFLT", hcmdargvp);
        if (ip1 > 0) { sattr[0] = '0'; }
        ip1 = findcmd("BLUE", hcmdargvp);
        if (ip1 > 0) {
            sattr[0] = '1';
            lclattr += fssBLUE;
        }
        ip1 = findcmd("RED", hcmdargvp);
        if (ip1 > 0) {
            sattr[0] = '2';
            lclattr += fssRED;
        }
        ip1 = findcmd("PINK", hcmdargvp);
        if (ip1 > 0) {
            sattr[0] = '3';
            lclattr += fssPINK;
        }
        ip1 = findcmd("GREEN", hcmdargvp);
        if (ip1 > 0) {
            sattr[0] = '4';
            lclattr += fssGREEN;
        }
        ip1 = findcmd("TURQ", hcmdargvp);
        if (ip1 > 0) {
            sattr[0] = '5';
            lclattr += fssTURQ;
        }
        ip1 = findcmd("YELLOW", hcmdargvp);
        if (ip1 > 0) {
            sattr[0] = '6';
            lclattr += fssYELLOW;
        }
        ip1 = findcmd("WHITE", hcmdargvp);
        if (ip1 > 0) {
            sattr[0] = '7';
            lclattr += fssWHITE;
        }
        // extended attributes
        ip1 = findcmd("NOEXT", hcmdargvp);
        if (ip1 > 0) { sattr[2] = 'D'; }
        ip1 = findcmd("BLINK", hcmdargvp);
        if (ip1 > 0) {
            sattr[2] = 'B';
            lclattr += fssBLINK;
        }
        ip1 = findcmd("REV", hcmdargvp);
        if (ip1 > 0) {
            sattr[2] = 'R';
            lclattr += fssREVERSE;
        }
        ip1 = findcmd("USCORE", hcmdargvp);
        if (ip1 > 0) {
            sattr[2] = 'U';
            lclattr += fssUSCORE;
        }
        // basic attributes
        ip1 = findcmd("UNP", hcmdargvp);
        if (ip1 > 0) {
            sattr[1] = 'U';
            lclattr += fssUNP;
        }
        ip1 = findcmd("PROT", hcmdargvp);
        if (ip1 > 0) {
            sattr[1] = 'P';
            lclattr += fssPROT;
        }
        ip1 = findcmd("DARK", hcmdargvp);
        if (ip1 > 0) { sattr[1] = 'D'; }
        ip1 = findcmd("BRI", hcmdargvp);
        if (ip1 > 0) { sattr[1] = 'B'; }
        ip1 = findcmd("NORMAL", hcmdargvp);
        if (ip1 > 0) {
            sattr[1] = 'N';
            lclattr += fssNON;
        }
        //
        sprintf(vname1, "_%s", hcmdargvp[1]);
        setVariable(vname1, sattr);
        nf = (struct sFields *) fssAFieldName(hcmdargvp[1]);
        if (nf != NULL) {
            memcpy(nf->sattr, sattr, 4);
        }
        //
        return (RxForceRC(0));
    }

    if (strcasecmp(hcmdargvp[0], "SHOW") == 0) {
        nf = (struct sFields *) fssAFieldName(hcmdargvp[1]);
        printf("field n=%s l=%d\n", nf->name, nf->length);
        return (RxForceRC(0));
    }

    // CONVERSE stempnl POPUP row col
    //                  WINDOW row col
    //                  *NOSTRIP|STRIP
    //                  *READ|NOREAD
    //                  WAIT milisec
    //                  CURSOR field
    // RC(08) .. panel not defined
    // RC(12) .. current panel different of converse panel
    if (strcasecmp(hcmdargvp[0], "CONVERSE") == 0) {
        if (erx_panel[0] = '\0') {
            printf("HCMD(PANEL) PANEL name is null\n");
            return (RxForceRC(8));
        }
        strcpy(erx_panel, (char *) getStemVariable("ZPANEL"));
        if (strcasecmp(erx_panel, hcmdargvp[1]) != 0) {
            return (RxForceRC(12));
        }
        ip1 = findcmd("POPUP", hcmdargvp);
        if (ip1 != -1) {
            floaty = atoi(hcmdargvp[ip1 + 1]);
            floatx = atoi(hcmdargvp[ip1 + 2]);
            erx_float = 1;
        } else {
            floaty = 0;
            floatx = 0;
            pnlw = 0;
            pnlh = 0;
            erx_float = 0;
            pnllvlix = 0;
        }
        erx_border = 0;
        ip1 = findcmd("BORDER", hcmdargvp);
        if (ip1 != -1) {
            erx_border = 1;
        }
        erx_vlist = 1;
        ip1 = findcmd("NOLIST", hcmdargvp);
        if (ip1 != -1) {
            erx_vlist = 0;
        }
        ip1 = findcmd("CURSOR", hcmdargvp);
        if (ip1 != -1) {
            setVariable("ZCURSOR", hcmdargvp[ip1 + 1]);
        }
        ip1 = findcmd("WINDOW", hcmdargvp);
        if (ip1 != -1) {
            pnlh = atoi(hcmdargvp[ip1 + 1]);
            pnlw = atoi(hcmdargvp[ip1 + 2]);
            erx_float = 1;
        }
        erx_trim = 0;   // default no strip
        setVariable("ZSTRIP", "OFF");
        ip1 = findcmd("STRIP", hcmdargvp);
        if (ip1 != -1) {
            setVariable("ZSTRIP", "ON");
        }
        erx_read = 0;
        ip1 = findcmd("NOREAD", hcmdargvp);
        if (ip1 != -1) {
            erx_read = 1;
        }
        sleepv = 0;
        ip1 = findcmd("WAIT", hcmdargvp);
        if (ip1 != -1) {
            sleepv = atoi(hcmdargvp[ip1 + 1]);
        }
        localrc = parsemap(hcmdargvp[1]);
        if (sleepv > 0) {
#ifdef JCC
            Sleep(sleepv);
#endif
        }
        return (RxForceRC(0));
    }

    if (strcmp(LSTR(*cmd), "VVV") == 0) {
        printf("-- vvv=%s\n", getVariable("a1"));
    }
//    rxReturnCode = system(LSTR(*cmd));

}

bool
isHostCmd(PLstr cmd)
{
    bool isHostCmd = FALSE;
    char lclcmd[1025];

    strcpy(lclcmd,(const char*)LSTR(*cmd));
    hcmdargcp=parsecmd(lclcmd,hcmdargvp);

    if(strcasecmp(hcmdargvp[0], "EXECIO") == 0) {
        isHostCmd = TRUE;
    }

    return isHostCmd;
}

int
handleHostCmd(PLstr cmd)
{
    int returnCode = 0;
    char lclcmd[1025];

    strcpy(lclcmd,(const char*)LSTR(*cmd));
    hcmdargcp=parsecmd(lclcmd,hcmdargvp);

    if(strcasecmp(hcmdargvp[0], "EXECIO") == 0) {
        returnCode = RxEXECIO();
    }


    return returnCode;
}

int RxEXECIO()
{
    int ii;
    char str1[64];
    unsigned char pbuff[1025];
    unsigned char vname1[19];
    unsigned char vname2[19];
    unsigned char vname3[19];
    unsigned char obuff[4097];
    int ip1 = 0;
    int recs = 0;
    FILE *f;

    // DISKR
    if (strcasecmp(hcmdargvp[2], "DISKR") == 0) {
        ip1 = findcmd("STEM", hcmdargvp);
        if (ip1 != -1) {
            ip1++;
            strcpy(vname1, hcmdargvp[ip1]);  // name of stem variable
        }
        SNULL(str1);
        f = fopen(hcmdargvp[3], "r");
        if (f == NULL) {
            return (RxForceRC(8));
        }
        recs = 0;
        while (fgets(pbuff, 1024, f)) {
            recs++;
            remlf(&pbuff[0]);                   // remove linefeed
            sprintf(vname2, "%s%d", vname1, recs); // edited stem name
            if (ip1 != -1) {
                setVariable(vname2, pbuff);               // set rexx variable
            }
            if (ip1 == -1) {
                rxqueue(pbuff);
            }
        }
        if (ip1 > 0) {
            sprintf(vname2, "%s0", vname1);
            sprintf(vname3, "%d", recs);
            setVariable(vname2, vname3);
        }
        fclose(f);
        return (RxForceRC(0));
    }

    // DISKW
    if (strcasecmp(hcmdargvp[2], "DISKW") == 0) {
        ip1 = findcmd("STEM", hcmdargvp);
        if (ip1 != -1) {
            ip1++;
            strcpy(vname1, hcmdargvp[ip1]);  // name of stem variable
        }
        f = fopen(hcmdargvp[3], "w");
        if (f == NULL) {
            return (RxForceRC(8));
        }
        if (ip1 != -1) {
            sprintf(vname2, "%s0", vname1);
            recs = getIntegerVariable(vname2);
        }
        if (ip1 == -1) {
            recs = rxqueued();
        }
        for (ii = 1; ii <= recs; ii++) {
            if (ip1 != -1) {
                SNULL(vname2);
                sprintf(vname2, "%s%d", vname1, ii);
                sprintf(obuff, "%s\n", getStemVariable(vname2));
                fputs(obuff, f);
            }
            if (ip1 == -1) {
                SNULL(obuff);
                sprintf(obuff, "%s\n", rxpull());
                fputs(obuff, f);
            }
        }
        fclose(f);
        return (RxForceRC(0));
    }

    // DISKA
    if (strcasecmp(hcmdargvp[2], "DISKA") == 0) {
        ip1 = findcmd("STEM", hcmdargvp);
        if (ip1 > 0) {
            ip1++;
            strcpy(vname1, hcmdargvp[ip1]);  // name of stem variable
        }
        f = fopen(hcmdargvp[3], "a");
        if (f == NULL) {
            return (RxForceRC(8));
        }
        sprintf(vname2, "%s0", vname1);
        recs = getIntegerVariable(vname2);
        for (ii = 1; ii <= recs; ii++) {
            SNULL(vname2);
            sprintf(vname2, "%s%d", vname1, ii);
            sprintf(obuff, "%s\n", getStemVariable(vname2));
            fputs(obuff, f);
        }
        fclose(f);
        return (RxForceRC(0));
    }
}