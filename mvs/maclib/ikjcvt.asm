         MACRO
&N       IKJCVT
CVTPTR   EQU   16
CVTMAP   DSECT
         ORG   CVTMAP+X'9C'
CVTTVT   DS    A
         ORG   CVTMAP+480
CVTSCAN  DS    A
         ORG   CVTMAP+524
CVTPARS  DS    A
         ORG   CVTMAP+732
CVTDAIR  DS    A
CVTEHDEF DS    A
CVTEHCIR DS    A
         MEND
