        MACRO
        VOLMAC  &VOLUME,&USE=YES,&UNIT=3350
        GBLA   &COUNTY,&COUNTN
        LCLA   &NEXT,&NOW,&LOOP
        LCLC   &USES,&BIT1,&BIT2,&BIT3
.* THE MACRO VOLMAC IS USED TO CONSTRUCT A TABLE OF VOLUMES
.* THAT ARE ALLOCATED IN RESPONSE TO A USESR'S REQUEST FOR DEFAULT
.* VOLUME.
.* THE LIST IS IN TWO PARTS THAT ARE INTERMINGLED. EACH PART IS
.* CHAINED SEPARATELY AND A SEQUENTIAL SEARCH CAN SCAN ENTRIES.
.* PART ONE IS ELIGIBLE VOLUMES WHERE ALLOCATION WILL TAKE PLACE.
.* PART TWO IS ELIGIBLE VOLUMES BUT ALLOCATION WILL CONVERT TO PART
.* ONE.
&USES    SETC  '&USE'
       AIF (N'&VOLUME EQ 0).LASTUP
       AIF (&SYSNDX GT 1).DOIT
VOLTABLE CSECT
         DC  A(VOLY1)
         DC  A(VOLN1)
.DOIT  ANOP
       AIF ('&USES' EQ 'Y').TRY1
       AIF ('&USES' EQ 'YES').TRY1
       AIF ('&USES' EQ 'NO').TRY2
       AIF ('&USES' EQ 'N').TRY2
       MNOTE 4,'USES IS INVALID. VOLUMES ARE CONSIDERED INELIGIBLE'
       AGO   .TRY2
.TRY1  ANOP
&USES   SETC  'Y'
&COUNTY SETA &COUNTY+1
&NEXT   SETA &COUNTY+1
&NOW    SETA &COUNTY
&BIT1   SETC '1'
       AGO   .OK
.TRY2   ANOP
&USES  SETC  'N'
&COUNTN SETA &COUNTN+1
&NEXT   SETA &COUNTN+1
&NOW    SETA &COUNTN
&BIT1   SETC '0'
.OK      ANOP
       AIF ('&UNIT' EQ '').M1
       AIF ('&UNIT' EQ '3350').M1
       AIF ('&UNIT' EQ '3330-1').M2
       AIF ('&UNIT' EQ '3380').M4
       AIF ('&UNIT' EQ '3330').M2
       AIF ('&UNIT' EQ '3330-11').M2
       MNOTE 4,'INVALID UNIT TYPE, ASSUME 3350'
       AGO  .M1
.M2    ANOP
&BIT2  SETC '0'
&BIT3  SETC '0'
       AGO   .M3
.M4    ANOP
&BIT2  SETC '0'
&BIT3  SETC '1'
       AGO   .M3
.M1    ANOP
&BIT2  SETC '1'
&BIT3  SETC '0'
.M3    ANOP
       AIF (N'&VOLUME GT 1).MULTI
       AIF ('&VOLUME'(1,1) EQ '(').MULTI
       AIF (K'&VOLUME GT 6).DROP
       SPACE 1
VOL&USES&NOW DC  A(VOL&USES&NEXT)
             DC  CL6'&VOLUME'
             DC  B'&BIT1.&BIT2.&BIT3.00000',XL1'0'
        MEXIT
.MULTI  ANOP
&LOOP   SETA  1
.AGAIN  ANOP
        AIF  (&LOOP GT  N'&VOLUME).END
        AIF  (K'&VOLUME(&LOOP) GT 6).DROPM
        SPACE 1
VOL&USES&NOW DC  A(VOL&USES&NEXT)
             DC  CL6'&VOLUME(&LOOP)'
             DC  B'&BIT1.&BIT2.&BIT3.00000',XL1'0'
&NEXT   SETA &NEXT+1
&NOW    SETA &NOW+1
&LOOP   SETA &LOOP+1
        AGO  .AGAIN
.END    ANOP
        AIF  ('&USES' EQ 'N').ENDN
&COUNTY SETA &NOW-1
        MEXIT
.ENDN   ANOP
&COUNTN SETA &NOW-1
        MEXIT
.DROP   ANOP
        MNOTE 8,'&VOLUME IS LONGER THAN 6 CHARACTERS'
        AIF   ('&USES' EQ 'N').DROPN
&COUNTY SETA  &COUNTY-1
        MEXIT
.DROPN  ANOP
&COUNTN SETA  &COUNTN-1
        MEXIT
.DROPM  ANOP
        MNOTE 8,'&VOLUME(&LOOP) IS LONGER THAN 6 CHARACTERS'
&LOOP   SETA  &LOOP+1
        AGO   .AGAIN
.LASTUP ANOP
&COUNTY SETA  &COUNTY+1
&COUNTN SETA  &COUNTN+1
        AIF  (&COUNTY EQ 1).LASTA
        SPACE 1
VOLY&COUNTY DC A(VOLY1)
         DC   CL6' '
         DC   XL6'8000'
         AGO  .LASTB
.LASTA   ANOP
VOLY1    DC   A(0),CL6' ',XL6'0'
.LASTB   ANOP
        AIF  (&COUNTN EQ 1).LASTC
        SPACE 1
VOLN&COUNTN DC A(VOLN1)
         DC   CL6' '
         DC   XL6'0'
         MEXIT
.LASTC   ANOP
VOLN1    DC   A(0),CL6' ',XL6'0'
         MEXIT
        MEND
