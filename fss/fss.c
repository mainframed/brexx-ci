/*-----------------------------------------------------------------------------
|  Copyright (c) 2012-2013, Tommy Sprinkle (tommy@tommysprinkle.com)  
|
|  All rights reserved. Redistribution and use in source and binary forms, 
|  with or without modification, are permitted provided that the following 
|  conditions are met:
|
|    * Redistributions of source code must retain the above copyright notice, 
|      this list of conditions and the following disclaimer. 
|    * Redistributions in binary form must reproduce the above copyright  
|      notice, this list of conditions and the following disclaimer in the  
|      documentation and/or other materials provided with the distribution. 
|    * Neither the name of the author nor the names of its contributors may  
|      be used toendorse or promote products derived from this software  
|      without specific prior written permission. 
|
|   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS  
|   "AS IS" AND ANYEXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT  
|   LIMITED TO, THE IMPLIED WARRANTIESOF MERCHANTABILITY AND FITNESS  
|   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENTSHALL THE 
|   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
|   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
|   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
|   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
|   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
|   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  
|   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
|   OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------
// FSS - Full Screen Services for TSO
//
// Tommy Sprinkle - tommy@tommysprinkle.com
// December, 2012
//
//---------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fss.h"
#include "rxtso.h"


// Limit To a 3270 Mod 2
#define MAX_ROW 24
#define MAX_COL 80

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


int fssFlagDebug=0;
int fssFlagTrim=0;
int fssVal1=0;            // control flag to floating map
int fssVal2=0;            // floating row
int fssVal3=0;            // floating col
int fssVal4=0;
int fssVal5=0;
char fsspname[9];         // panel name

// Calculate buffer offset from Row and Column
#define bufPos(row,col) ( ((row-1) * 80) + (col - 1) )

// Return Attribute Values
#define XH(attr) ((attr >> 16) & 0xFF)    // Return Extended Highlighting Attrib
#define XC(attr) ((attr >>  8) & 0xFF)    // Return Extended Color Attribute
#define BA(attr) (attr & 0xFF)            // Return Basic 3270 Attribute

// Convert buffer offset to a 3270 buffer address
#define bufAddr(p) ((xlate3270((p >> 6) & 0x3F) << 8) | (xlate3270(p & 0x3f)))
// Convert a 3270 buffer address to an offset
#define bufOff(addr) ( (addr & 0x3F) | ((addr & 0x3F00) >> 2) )

char  a3270[4096];
int   l3270;

// FSS Environment Values
static struct sFields fssFields[1024];   // Array of fields
static int            fssFieldCnt;      // Count of fields defined in array
static int            fssAID;           // Last 3270 AID value returned
static int            fssCSR;           // Position of Cursor at last read
static int            fssCSRPOS;        // Buffer Position to place Cursor at ne

// Debugging TPUT - Used only for debugging purposes
static void dput(char *txt)
{
   tput(txt,strlen(txt));
}

int
fssFieldPos()
{
  return fssCSR;
}

int
fsststyx(int y,int x)
{
  int i=0;
  for(i=0;i<fssAFields();i++) {
    if(fssFields[i].bufaddr==bufPos(y,x)) {
      return i;
    }
  }
  return -1;
}

void
fssSetPanel(char *s)
{
  memset(fsspname,0,sizeof(fsspname));
  strcpy(fsspname,s);
}

struct sFields *
fssAField(int idf)
{
  return (struct sFields *)&fssFields[idf];
}

struct sFields *
fssAFieldName(char *n)
{
  int i=0;
  if(fsspname==NULL) {
    for(i=0;i<fssAFields();i++) {
      if(fssFields[i].typef==2) {
        if(strcasecmp(n,fssFields[i].name)==0) {
          return (struct sFields *)&fssFields[i];
        }
      }
    }
    return NULL;
  } else {
    for(i=0;i<fssAFields();i++) {
      if(fssFields[i].typef==2) {
        if(strcasecmp(n,fssFields[i].name)==0&&
           strcasecmp(fsspname,fssFields[i].pname)==0) {
          return (struct sFields *)&fssFields[i];
        }
      }
    }
    return NULL;
  }
}

void
fssParms(int p1,int p2,int p3,int p4,int p5) {
  fssVal1=p1;
  fssVal2=p2;
  fssVal3=p3;
  fssVal4=p4;
  fssVal5=p5;
}

void
fssDebug(short p) {
  fssFlagDebug=p;
}

int fssAFields() {
    return fssFieldCnt;
}


void
dumpdata(char *pin,int ilen) {
  int   ix;
  char  buff[4096];
  int   il;
  char  *mark;
  int   iy;

  memcpy(buff,pin,ilen);
  il=0;

  for(ix=0;ix<ilen;ix++) {
    if(il==0) {
      printf("%08x ",pin);
      mark=pin;
    }
    printf("%02x ",buff[ix]);
    il++;
    if(il>15) {
      il=0;
      pin+=15;
      printf("\n");
    }
  }
  printf("\n");

  return;
}

//----------------------------------------
// Find the field located at a "pos"
// offset.
// Returns the INDEX + 1 of the field
//----------------------------------------
static int findFieldPos(int pos)
{
   int ix;

   if(fssFieldCnt < 1)                     // If no fields
      return 0;

   for(ix = 0; ix < fssFieldCnt; ix++)     // Loop through Field Array
      if(pos == fssFields[ix].bufaddr)     // Check for match
         return (ix + 1);                  // Return index + 1

   return 0;                               // No match found
}

void fssDump()
{
  int ii;
  for(ii=0;ii<=fssFieldCnt;ii++) {
    printf("#%d n=%s l=%d fl=%d a=%d d=%s\n",
           ii,fssFields[ii].name,strlen(fssFields[ii].name),
           fssFields[ii].length,
           fssFields[ii].attr,
           fssFields[ii].data
          );
  }
}

//----------------------------------------
// Find a field by Field Name
// Returns the INDEX + 1 of the field
//
//----------------------------------------
static int findField(char *fldName)
{
   int  ix;

   if(fssFieldCnt < 1)                     // If no fields
      return 0;

   for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
      if(!strcasecmp(fldName,fssFields[ix].name)) {
         return (ix + 1);
      }
   }
   return 0;                               // No match found
}


//----------------------------------------
//  Update Field Data
//
//
//----------------------------------------
static int updtFld(int pos, char *data, int len)
{
   int ix;

   ix = findFieldPos(pos);                 // Locate Field by start position

   if(!ix)                                 // Exit if no field found
      return -1;

   ix--;                                   // Adjust to get actual field index

   if(len  <= fssFields[ix].length)        // If data fits
   {
      memcpy(fssFields[ix].data, data, len);   // Copy data
      *(fssFields[ix].data+len) = '\0';
   }
   else                                    // ELSE - truncate data
   {
      memcpy(fssFields[ix].data, data, fssFields[ix].length);  // Copy max amoun
      *(fssFields[ix].data + fssFields[ix].length) = '\0';     // Terminate stri
   }

   return 0;
}


//----------------------------------------
//  Translate non-printable characters
//  in a string to a "."
//
//----------------------------------------
static char * makePrint(char *str)
{
	char *p;

	p = str;
	while(*p)                              // Loop through string
	{
		if(!isprint(*p))                   // If not a printable character
			*p = '.';                      // Replace with "."
		p++;                               // Next char
	}
	return str;
}


//----------------------------------------
//  Check a string to see if it is all
//  Numeric
//
//----------------------------------------
int fssIsNumeric(char * data)
{
    int len;
    int i;

    len = strlen(data);                    // Get string length
    if(len < 1)                            // Empty string is NOT Numeric
       return 0;

    for(i=0; i<len; i++)                   // Check each character
       if(!isdigit( *(data+i) ))
          return 0;

    return 1;                              // All characters are numbers
}


//----------------------------------------
// Check a string to see if it is all
// Hex digits
//
//----------------------------------------
int fssIsHex(char * data)
{
    int len;
    int i;

    len = strlen(data);                    // Get string length
    if(len < 1)                            // Empty string is not HEX
       return 0;

    for(i=0; i<len; i++)                   // Check each character
       if(!isxdigit( *(data+i) ))
          return 0;

    return 1;
}


//----------------------------------------
//  Check to see if a string is all blanks
//
//
//----------------------------------------
int fssIsBlank(char * data)
{
   int i;

   i = 0;

   do
   {
      if (!(*(data+i)))                    // Empty string is blank
         return 1;

      if( *(data+i) != ' ')                // Check each character
         return 0;
      i++;
    } while(1);

    return 1;                              // String is blank
}


//----------------------------------------
// Trim trailing blanks from a string
//
//
//----------------------------------------
char * fssTrim(char * data)
{
   int len;

   len = strlen(data);                     // Get string length
   if(len == 0)
      return data;

   while(len)                              // Start at end of string
   {
      if(*(data+len-1) != ' ')             // Exit on first non-blank
         return data;
      *(data+len-1) = '\0';                // Remove trailing blank
      len--;
   }

   return data;                            // Return trimmed string
}


//----------------------------------------
//  Initialize FSS Environment
//
//
//----------------------------------------
int fssInit(void)
{
   if(fssFieldCnt>0) {
     fssReset();
     fssTerm();
   }

   fssFieldCnt  = 0;                       // Set Field Count to Zero
   fssCSRPOS    = 0;                       // Reset Cursor Position for next wri

   stfsmode(1);                            // Begin TSO Fullscreen Mode
   sttmpmd(1);

   return 0;
}


//----------------------------------------
//  Destroy Current Screen
//  Begin a new empty screen
//
//----------------------------------------
int fssReset(void)
{
   int ix;

   for(ix=0; ix < fssFieldCnt; ix++)       // Loop through Field Array
   {
      fssFields[ix].floatf=0;
      if(fssFields[ix].name)
         free(fssFields[ix].name);         // Free field name
      if(fssFields[ix].pname)
         free(fssFields[ix].pname);        // Free panel name
      if(fssFields[ix].data)
         free(fssFields[ix].data);         // Free field data
   }

   fssFieldCnt = 0;                        // Reset field count
   fssAID      = 0;                        // Reset last AID value
   fssCSR      = 0;                        // Reset last Cursor position
   fssCSRPOS   = 0;                        // Reset Cursor position

   return 0;
}


//----------------------------------------
// Terminate FSS Environment
//
//
//----------------------------------------
int fssTerm(void)
{

   stlineno(1);                            // Exit TSO Full Screen Mode
   stfsmode(0);
   sttmpmd(0);

   return 0;
}


//----------------------------------------
// Return Last AID value
//
//
//----------------------------------------
int fssGetAID(void)
{
   return fssAID;
}

//----------------------------------------
// Translate an attribute value
//
//
//----------------------------------------
int fssAttr(int attr)
{
   return ((attr & 0xFFFF00) | xlate3270( attr & 0xFF));
}


//----------------------------------------
//  Define a Text Field
//     row  - Beginning Row position of field
//     col  - Beginning Col position of field
//     attr - Field Attribute
//     text - Field data contents
//
//----------------------------------------
int fssTxt(int row, int col, int attr, char * text)
{
   int txtlen;
   int ix;

   if(fsststyx(row,col)!=-1) {
     goto skip_assign_value;
   }

   makePrint(text);                        // Eliminate non-printable characters
   txtlen = strlen(text);                  // get text length

   // Validate Field Starting Position
   if(row < 1 || col < 2 || row > MAX_ROW || col > MAX_COL)
      return -1;

   if(txtlen < 1 || txtlen > 79)           // Validate Maximum Length
      return -2;


   ix = fssFieldCnt++;                     // Increment field count

   //----------------------------
   // Fill In Field Array Values
   //----------------------------
   fssFields[ix].name    =  0;             // no name for a text field
   fssFields[ix].pname   =  (char *) malloc(strlen(fsspname)+1);
   strcpy(fssFields[ix].pname, fsspname);
   fssFields[ix].bufaddr =  bufPos(row,col);
   fssFields[ix].attr    =  ((attr & 0xFFFF00) | xlate3270( attr & 0xFF));
   fssFields[ix].length  =  txtlen;
   fssFields[ix].data    =  (char *) malloc(txtlen+1);
   fssFields[ix].typef   =  1;
   if(fssVal1==1) {         // setting val1 to on for floating map
     fssFields[ix].floatf=1;
   }
   strcpy(fssFields[ix].data, text);

   skip_assign_value:

   return 0;
}

int fssTxa(int row, int col, char *sattr, char * text)
{
   int txtlen;
   int ix;

   if(fsststyx(row,col)!=-1) {
     goto skip_assign_value;
   }

   makePrint(text);                        // Eliminate non-printable characters
   txtlen = strlen(text);                  // get text length

   // Validate Field Starting Position
   if(row < 1 || col < 2 || row > MAX_ROW || col > MAX_COL)
      return -1;

   if(txtlen < 1 || txtlen > 79)           // Validate Maximum Length
      return -2;

   ix = fssFieldCnt++;                     // Increment field count

   //----------------------------
   // Fill In Field Array Values
   //----------------------------
   fssFields[ix].name    =  0;             // no name for a text field
   fssFields[ix].pname   =  (char *) malloc(strlen(fsspname)+1);
   strcpy(fssFields[ix].pname, fsspname);
   fssFields[ix].bufaddr =  bufPos(row,col);
   fssFields[ix].attr    =  0;
   strcpy(fssFields[ix].sattr,sattr);
   fssFields[ix].length  =  txtlen;
   fssFields[ix].data    =  (char *) malloc(txtlen+1);
   fssFields[ix].typef   =  1;
   if(fssVal1==1) {         // setting val1 to on for floating map
     fssFields[ix].floatf=1;
   }
   strcpy(fssFields[ix].data, text);

   skip_assign_value:

   return 0;
}


//----------------------------------------
// Define a Dynamic Field
//     row     - Beginning Row position of field
//     col     - Beginning Col position of field
//     attr    - Field Attribute
//     fldName - Field name - to allow access
//     len     - Field length
//     text    - Field initial data contents
//
//----------------------------------------
int fssFld(int row, int col, int attr, char * fldName, int len, char *text)
{
   int ix;

   // Validate Field Start Position
   if(row < 1 || col < 2 || row > MAX_ROW || col > MAX_COL)
      return -1;

   // Validate Field Length
   if(len < 1 || len > 79)
      return -2;

   if(fsststyx(row,col)!=-1) {
     goto skip_assign_value;
   }

// if(findField(fldName))                  // Check for duplicate Field Name
//    return -3;

   ix = fssFieldCnt++;                     // Increment Field Count

   //----------------------------
   // Fill In Field Array Values
   //----------------------------
   fssFields[ix].name    =  (char *) malloc(strlen(fldName)+1);
   strcpy(fssFields[ix].name, fldName);
   fssFields[ix].pname   =  (char *) malloc(strlen(fsspname)+1);
   strcpy(fssFields[ix].pname, fsspname);
   fssFields[ix].bufaddr =  bufPos(row,col);
   fssFields[ix].attr    =  ((attr & 0xFFFF00) | xlate3270( attr & 0xFF));
   fssFields[ix].length  =  len;
   fssFields[ix].typef   =  2;
   fssFields[ix].data    =  (char *) malloc(len + 1);
   memset(fssFields[ix].sattr,0,4);

   skip_assign_value:

   makePrint(text);                        // Eliminate non-printable characters

   if(strlen(text) <= fssFields[ix].length)   // Copy text if it fits into field
      strcpy( fssFields[ix].data, text);
   else                                       // Truncate text if too long
   {
      strncpy(fssFields[ix].data, text, fssFields[ix].length);
      *(fssFields[ix].data + fssFields[ix].length) = '\0';
   }

   return 0;
}

int fssFla(int row, int col, int attr, char * fldName, int len, char *text,
           char *sattr)
{
   int ix;

   // Validate Field Start Position
   if(row < 1 || col < 2 || row > MAX_ROW || col > MAX_COL)
      return -1;

   // Validate Field Length
   if(len < 1 || len > 79)
      return -2;

   ix=fsststyx(row,col);
   if(ix!=-1) {
     goto skip_assign_value;
   }

// if(findField(fldName))                  // Check for duplicate Field Name
//    return -3;

   ix = fssFieldCnt++;                     // Increment Field Count

   //----------------------------
   // Fill In Field Array Values
   //----------------------------
   fssFields[ix].name    =  (char *) malloc(strlen(fldName)+1);
   strcpy(fssFields[ix].name, fldName);
   fssFields[ix].pname   =  (char *) malloc(strlen(fsspname)+1);
   strcpy(fssFields[ix].pname, fsspname);
   fssFields[ix].bufaddr =  bufPos(row,col);
   fssFields[ix].attr    =  ((attr & 0xFFFF00) | xlate3270( attr & 0xFF));
   fssFields[ix].length  =  len;
   fssFields[ix].typef   =  2;
   fssFields[ix].data    =  (char *) malloc(len + 1);
   strcpy(fssFields[ix].sattr,sattr);

   skip_assign_value:

   makePrint(text);                        // Eliminate non-printable characters

   if(strlen(text) <= fssFields[ix].length)   // Copy text if it fits into field
      strcpy( fssFields[ix].data, text);
   else                                       // Truncate text if too long
   {
      strncpy(fssFields[ix].data, text, fssFields[ix].length);
      *(fssFields[ix].data + fssFields[ix].length) = '\0';
   }

   return 0;
}


//----------------------------------------
//  Set Dynamic Field Contents
//
//
//----------------------------------------
int fssSetData(char *fldName, char *text)
{
   int ix;
   int txtLen;


   for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
      if((!strcasecmp(fldName,fssFields[ix].name))&&
         (!strcasecmp(fsspname,fssFields[ix].pname))) {
        goto skip_bad_return;
      }
   }
   return -1;

   skip_bad_return:

   makePrint(text);                        // Eliminate non-printable characters

   if(strlen(text) < fssFields[ix].length) { // If text fits, copy it
      memcpy(fssFields[ix].data,text,strlen(text));
      memset(fssFields[ix].data+fssFields[ix].length,
             ' ',
             fssFields[ix].length-strlen(text));
   } else {
      strncpy(fssFields[ix].data, text, fssFields[ix].length);
      *(fssFields[ix].data + fssFields[ix].length) = '\0';
   }

   return 0;
}

//int fssSetData(char *fldName, char *text)
//{
//   int ix;
//   int txtLen;

// if(fsspname!=NULL) {
// for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
//    if((!strcasecmp(fldName,fssFields[ix].name))&&
//       (!strcasecmp(fsspname,fssFields[ix].pname))) {
//      goto skip_to_setdata;
//    }
// }

// skip_to_setdata:

//   makePrint(text);                  // Eliminate non-printable ch    aracters

// if(strlen(text) <= fssFields[ix].length)   // If text fits, copy it
//      strcpy( fssFields[ix].data, text);
//   else                                    // Truncate if too long
//   {
//    strncpy(fssFields[ix].data, text, fssFields[ix].length);
//    *(fssFields[ix].data + fssFields[ix].length) = '\0';
// }

// return 0;
//}


//----------------------------------------
//  Return pointer to Dynamic Field Contents
//
//
//----------------------------------------
//char * fssGetField(char *fldName)
//{
//   int ix;
//
//   ix = findField(fldName);                // Find Field by Name
//   if(!ix)
//      return (char *) 0;
//
//   return fssFields[ix-1].data;            // Return pointer to data
//}

char * fssGetData(char *fldName)
{
   int ix;

   if(fsspname==NULL) {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if(!strcasecmp(fldName,fssFields[ix].name)) {
          if(fssFlagTrim==1) {
            return fssTrim(fssFields[ix].data);
          } else {
            return fssFields[ix].data;
          }
        }
     }
     return NULL;
   }

   if(fsspname!=NULL) {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if((!strcasecmp(fldName,fssFields[ix].name))&&
           (!strcasecmp(fsspname,fssFields[ix].pname))) {
          if(fssFlagTrim==1) {
            return fssTrim(fssFields[ix].data);
          } else {
            return fssFields[ix].data;
          }
        }
     }
     return NULL;
   }

   return NULL;
}

struct sFields * fssGetStru(char *fldName)
{
   int ix;

   if(fsspname==NULL) {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if(!strcasecmp(fldName,fssFields[ix].name)) {
          return &fssFields[ix];
        }
     }
     return NULL;
   }

   if(fsspname!=NULL) {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if((!strcasecmp(fldName,fssFields[ix].name))&&
           (!strcasecmp(fsspname,fssFields[ix].pname))) {
          return &fssFields[ix];
        }
     }
     return NULL;
   }

   return NULL;
}

//----------------------------------------
//  Set Cursor position for next write
//
//
//----------------------------------------
int fssSetCursor(char *fldName)
{
   int ix;

   ix = findField(fldName);                // Find Field by Name
   if(!ix)
      return -1;

   fssCSRPOS = bufAddr(fssFields[ix-1].bufaddr);   // Cursor pos = field start p
   return 0;
}

int fssCursor(char *fldName)
{
   int ix;

   if(fsspname==NULL) {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if(!strcasecmp(fldName,fssFields[ix].name)) {
           fssCSRPOS = bufAddr(fssFields[ix].bufaddr);
           return 0;
        }
     }
     return -1;
   } else {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if((!strcasecmp(fldName,fssFields[ix].name))&&
           (!strcasecmp(fsspname,fssFields[ix].pname))) {
           fssCSRPOS = bufAddr(fssFields[ix].bufaddr);
           return 0;
        }
     }
   }
   return 0;
}


//----------------------------------------
// Replace Field Attribute Value
// This only replaces the 3270 basic attribute
// Extended attributes are not modified
//----------------------------------------
int fssSetAttr(char *fldName, int attr)
{
   int ix;

   if(fsspname==NULL) {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if(!strcasecmp(fldName,fssFields[ix].name)) {
          // Replace Basic 3270 Attribute data
   fssFields[ix].attr = ((attr & 0xFFFF00) | xlate3270( attr & 0xFF));
           return 0;
        }
     }
     return -1;
   } else {
     for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
        if((!strcasecmp(fldName,fssFields[ix].name))&&
           (!strcasecmp(fsspname,fssFields[ix].pname))) {
   fssFields[ix].attr = ((attr & 0xFFFF00) | xlate3270( attr & 0xFF));
           return 0;
        }
     }
   }


   return 0;
}


//----------------------------------------
// Replace Field Color Attribute
//
//
//----------------------------------------
int fssSetColor(char *fldName, int color)
{
   int ix;
   int attr;

   for(ix=0; ix <= fssFieldCnt; ix++) {     // Loop through Field Array
      if(strcasecmp(fldName,fssFields[ix].name)==0&&
         strcasecmp(fsspname,fssFields[ix].pname)==0) {
        attr = (fssFields[ix].attr & 0xFF00FF) | (color & 0xFF00);
        fssFields[ix].attr = attr;
        return 0;
      }
   }

   return -1;
}


//----------------------------------------
// Replace Extended Formatting Attribute
//
//
//----------------------------------------
int fssSetXH(char *fldName, int xha)
{
   int ix;
   int attr;

   for(ix=0; ix < fssFieldCnt; ix++) {     // Loop through Field Array
     if(strcasecmp(fldName,fssFields[ix].name)==0&&
        strcasecmp(fsspname,fssFields[ix].pname)==0) {
       attr = (fssFields[ix-1].attr & 0xFFFF) | (xha & 0xFF0000);
       fssFields[ix-1].attr = attr;
       return 0;
     }
   }

   return -1;
}


//----------------------------------------
//  Process a 3270 Input Data Stream
//
//
//----------------------------------------
static int doInput(char * buf, int len)
{
   int   l;
   int   bufpos;
   int   fldLen;
   char *p;
   char *s;

   p = buf;
   l = len;

   if(len < 3)                             // Must be at least 3 bytes long
   {
      fssAID = 0;
      fssCSR = 0;
      return -1;
   }

   fssAID = *p;                            // Save AID Value
   p++;                                    // Skip over AID
   l--;

   fssCSR = bufOff( (*p << 8) + *(p+1) );  // Save Cursor Position

   p += 2;                                 // skip over Cursor Position
   l -= 2;

   while( l > 3 )                          // Min field length is 3 (0x11 + buff
   {
      if( *p != 0x11 )                     // Expecting Start Field sequence
         return -2;

      p++;                                 // Skip over
      l--;

      bufpos = bufOff( (*p << 8) + *(p+1) );  // Get buffer position
      p  += 2;                             // Skip over
      l  -= 2;

      s   = p;                             // Save start of field data

      while( l && *p != 0x11 )             // Scan for end of field
      {
         p++;
         l--;
      }

      fldLen = p - s;                      // Calculate field position

      updtFld( bufpos, s, fldLen );        // Update field Contents
   }

   return 0;
}

void
fssBuffer(char *buff,int *bufflen)
{
  buff=&a3270[0];
  bufflen=&l3270;

  return;
}

//----------------------------------------
// Write Screen to TSO Terminal and
// get input
//
//----------------------------------------
int fssRefresh(void)
{
   int   ba;
   int   ix;
   int   i;
   int   inLen;
   int   xHilight;
   int   xColor;
   int   BUFLEN;
   char *outBuf;
   char *inBuf;
   char *p;
   char *calc;

   BUFLEN = (MAX_ROW * MAX_COL * 2);       // Max buffer length

   outBuf = malloc(BUFLEN);                // alloc output buffer
   inBuf  = malloc(BUFLEN);                // alloc input buffer

   p = outBuf;                             // current position in 3270 data stre

   *p++ = 0x27;                            // Escape
   *p++ = 0xF5;                            // Write/Erase
   *p++ = 0xC3;                            // WCC

   for(ix = 0; ix < fssFieldCnt; ix++)     // Loop through fields
   {
      ba   = bufAddr(fssFields[ix].bufaddr - 1);  // Back up one from field star
      *p++ = 0x11;                         // SBA
      *p++ = (ba >> 8) & 0xFF;             // 3270 Buffer address
      *p++ = ba & 0xFF;                    // 3270 Buffer address
      *p++ = 0x1D;                         // Start Field
      if(BA(fssFields[ix].attr)==64) {
      *p++ = 193;                          // Basic Attribute
      } else {
      *p++ = BA(fssFields[ix].attr);       // Basic Attribute
      }

      xHilight = XH(fssFields[ix].attr);   // Get Extended Highlighting Attribut
      xColor   = XC(fssFields[ix].attr);   // Get Extended Color Attribute

 //   printf("-- a=%x %d ex=%x color=%x\n",BA(fssFields[ix].attr),
 //                                     BA(fssFields[ix].attr),
 //                                     XH(fssFields[ix].attr),
 //                                     XC(fssFields[ix].attr)
 //                                     );

      if(xHilight&&fssFields[ix].typef==1)
      {
         *p++ = 0x28;                      // Set Attribute
         *p++ = 0x41;                      // Extended
         *p++ = xHilight;                  // Value
      }

      if(xHilight&&fssFields[ix].typef==2)
      {
         *p++ = 0x28;                      // Set Attribute
         *p++ = 0x41;                      // Extended
         *p++ = xHilight;
      }

      if(xColor&&fssFields[ix].typef==1)   // If any Extended Color
      {
         *p++ = 0x28;                      // Set Attribute
         *p++ = 0x42;                      // Extended
         *p++ = xColor;                    // Value
      }

      if(fssFields[ix].typef==2) {
         *p++ = 0x28;                      // Set Attribute
         *p++ = 0x42;                      // Extended
         *p++ = fssFields[ix].sattr[0];
      }

      i = 0;
      if(fssFields[ix].data)               // Insert field data contents
      {
         i    = strlen(fssFields[ix].data);  // length of data
         if(fssFields[ix].length < i)      // truncate if too long
            i = fssFields[ix].length;
         memcpy(p, fssFields[ix].data, i); // copy to 3270 data stream
         p += i;                           // update position in data stream
      }

	  // End of field position
      ba   = bufAddr(fssFields[ix].bufaddr + fssFields[ix].length);
      *p++ = 0x11;                         // SBA
      *p++ = (ba >> 8) & 0xFF;             // 3270 buffer address
      *p++ = ba & 0xFF;
      *p++ = 0x1D;                         // start field
      *p++ = xlate3270( fssPROT );         // attrubute = protected

      if(xHilight || xColor)               // If field had Extended Attribute va
      {
         *p++ = 0x28;                      // Set Attrubite
         *p++ = 0x00;                      // Reset all
         *p++ = 0x00;                      // Reset all
      }
   }

   if (fssCSRPOS)                          // If Cursor position was specified
   {
      *p++ = 0x11;                         // SBA
      *p++ = (fssCSRPOS >> 8) & 0xFF;      // Buffer position
      *p++ = fssCSRPOS & 0xFF;
      *p++ = 0x13;                         // Insert Cursor
      fssCSRPOS = 0;
   }

   l3270=p-outBuf;
   memcpy(a3270,outBuf,l3270);
   if(fssFlagDebug==2) {
     printf("-- Input\n");
     dumpdata(a3270,l3270);
   }

   // Write Screen and Get Input
   do
   {
      tput_fullscr(outBuf, p-outBuf);      // Fullscreen TPUT

      inLen = tget_asis(inBuf, BUFLEN);    // TGET-ASIS
      if( *inBuf != 0x6E )                 // Check for reshow
         break;                            //   no - break out
   } while(1);                             // Display Screen until no reshow

   if(fssFlagDebug==2) {
     printf("-- Output\n");
     dumpdata(a3270,l3270);
   }

   doInput(inBuf, inLen);                  // Process Input Data Stream

   free(outBuf);                           // Free Output Buffer
   free(inBuf);                            // Free Input Buffer
   return 0;
}

int fssRefresh2(int rfflag)
{
   int   ba;
   int   ix;
   int   i;
   int   inLen;
   int   xHilight;
   int   xColor;
   int   BUFLEN;
   char *outBuf;
   char *inBuf;
   char *p;
   char *calc;

   BUFLEN = (MAX_ROW * MAX_COL * 2);       // Max buffer length

   outBuf = malloc(BUFLEN);                // alloc output buffer
   inBuf  = malloc(BUFLEN);                // alloc input buffer

   p = outBuf;                             // current position in 3270 data stre

   *p++ = 0x27;                            // Escape
   *p++ = 0xF5;                            // Write/Erase
   *p++ = 0xC3;                            // WCC

   for(ix = 0; ix < fssFieldCnt; ix++)     // Loop through fields
   {
      ba   = bufAddr(fssFields[ix].bufaddr - 1);  // Back up one from field star
      *p++ = 0x11;                         // SBA
      *p++ = (ba >> 8) & 0xFF;             // 3270 Buffer address
      *p++ = ba & 0xFF;                    // 3270 Buffer address

      //printf("-- p=%s n=%s b=%d attr=%s data=%s\n",
      //       fssFields[ix].pname,
      //       fssFields[ix].name,
      //       fssFields[ix].bufaddr,
      //       fssFields[ix].sattr,
      //       fssFields[ix].data
      //       );
      *p++ = 0x1d;
      if(fssFields[ix].sattr[1]=='U') {
        *p++ = 0xc1;
      }
      if(fssFields[ix].sattr[1]=='P') {
        *p++ = 0xf0;
      }
      if(fssFields[ix].sattr[1]=='D') {
        *p++ = 0x4d;
      }

      *p++ = 0x28;
      *p++ = 0x41;
      if(fssFields[ix].sattr[2]=='D'||fssFields[ix].sattr[2]=='0') {
        *p++ = 0x00;
      }
      if(fssFields[ix].sattr[2]=='B') {
        *p++ = 0xF1;
      }
      if(fssFields[ix].sattr[2]=='R') {
        *p++ = 0xF2;
      }
      if(fssFields[ix].sattr[2]=='U') {
        *p++ = 0xF4;
      }

      *p++ = 0x28;
      *p++ = 0x42;
      if(fssFields[ix].sattr[0]!='D') {
        *p++ = fssFields[ix].sattr[0];
      } else {
        *p++ = 0x00;
      }

      i = 0;
      if(fssFields[ix].data)               // Insert field data contents
      {
         i    = strlen(fssFields[ix].data);  // length of data
         if(fssFields[ix].length < i)      // truncate if too long
            i = fssFields[ix].length;
         memcpy(p, fssFields[ix].data, i); // copy to 3270 data stream
         p += i;                           // update position in data stream
      }

	  // End of field position
      ba   = bufAddr(fssFields[ix].bufaddr + fssFields[ix].length);
      *p++ = 0x11;                         // SBA
      *p++ = (ba >> 8) & 0xFF;             // 3270 buffer address
      *p++ = ba & 0xFF;

      *p++ = 0x1d;
      *p++ = 0xf0;
      *p++ = 0x28;
      *p++ = 0x00;
      *p++ = 0x00;

   }

   if (fssCSRPOS)                          // If Cursor position was specified
   {
      *p++ = 0x11;                         // SBA
      *p++ = (fssCSRPOS >> 8) & 0xFF;      // Buffer position
      *p++ = fssCSRPOS & 0xFF;
      *p++ = 0x13;                         // Insert Cursor
      fssCSRPOS = 0;
   }

   l3270=p-outBuf;
   memcpy(a3270,outBuf,l3270);
   if(fssFlagDebug==2) {
     printf("-- Input\n");
     dumpdata(a3270,l3270);
   }

   // Write Screen and Get Input

   if(rfflag==0) {
     do
     {
        tput_fullscr(outBuf, p-outBuf);    // Fullscreen TPUT

        inLen = tget_asis(inBuf, BUFLEN);  // TGET-ASIS
        if( *inBuf != 0x6E )               // Check for reshow
           break;                          //   no - break out
     } while(1);                           // Display Screen until no re  show
   }

   if(rfflag==1) {
     tput_fullscr(outBuf, p-outBuf);    // Fullscreen TPUT
   }

   if(fssFlagDebug==2) {
     printf("-- Output\n");
     dumpdata(a3270,l3270);
   }

   if(rfflag==0||rfflag==2) {
     doInput(inBuf, inLen);                // Process Input Data Stream
   }

   free(outBuf);                           // Free Output Buffer
   free(inBuf);                            // Free Input Buffer
   return 0;
}
