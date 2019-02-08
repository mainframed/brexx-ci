#include <stdio.h>
#include <string.h>

#include "lstring.h"
#include "rexx.h"
#include "rxdefs.h"

/* ------- Includes for any other external library ------- */
#if defined(__MVS__) || defined(__CROSS__)
extern void __CDECL RxMvsInitialize();
#endif

#ifndef __CROSS__
extern int __libc_arch;
#else
int __libc_arch = 0;
#endif

#include "dbginfo.h"

#ifdef __DEBUG__
/* --------- global debug / trace structure -------- */
P_DebugInfo debugInfo;
#endif

/* --------------------- main ---------------------- */
int __CDECL
main(int ac, char *av[])
{
	Lstr	args[MAXARGS], tracestr, file;
	int	ia,ir,iaa;
	bool	input, loop_over_stdin, parse_args, interactive;

	input           = FALSE;
	loop_over_stdin = FALSE;
	parse_args      = FALSE;
	interactive     = FALSE;

	for (ia=0; ia<MAXARGS; ia++) LINITSTR(args[ia]);
	LINITSTR(tracestr);
	LINITSTR(file);

    if (ac<2) {
        puts("\nsyntax: rexx [-[trace]|-F|-a|-i|-m] <filename> <args>...\n");
        puts("options:");
        puts("  -   to use stdin as input file");
        puts("  -a  break words into multiple arguments");
        puts("  -i  enter interactive mode");
        puts("  -F  loop over standard input");
        puts("      \'linein\' contains each line from stdin");
        puts("  -m  machine architecture: 0=S/370, 1=Hercules s37x, 2=S/390, 3=z/Arch.\n");
        puts(VERSIONSTR);
        puts("Author: "AUTHOR);
        puts("Maintainer: "MAINTAINER);
        puts("Please report bugs, errors or comments at https://github.com/mgrossmann/brexx370\n");

        return 0;
    }
#ifdef __DEBUG__
	__debug__ = FALSE;
#endif

	/* --- Initialise --- */
#ifdef __DEBUG__
	debugInfo = malloc(sizeof(DebugInfo));
	memset(debugInfo,0,sizeof(DebugInfo));
	debugInfo->magic_eye = MAGIC_EYE;
#endif

	RxInitialize(av[0]);

	/* --- Register functions of external libraries --- */
#if defined(__MVS__) || defined(__CROSS__)
    RxMvsInitialize();
#endif

	/* --- scan arguments --- */
	ia = 1;
	if (av[ia][0]=='-') {
		if (av[ia][1]==0)
			input = TRUE;
		else
		if (av[ia][1]=='F')
			loop_over_stdin = input = TRUE;
		else
		if (av[ia][1]=='a')
			parse_args = TRUE;
		else
		if (av[ia][1]=='i')
			interactive = TRUE;
#ifndef __CROSS__
		else
		if (av[ia][1]=='m')
			__libc_arch = atoi(av[ia]+2);
#endif
		else
			Lscpy(&tracestr,av[ia]+1);
		ia++;
	} else
	if (av[ia][0]=='?' || av[ia][0]=='!') {
		Lscpy(&tracestr,av[ia]);
		ia++;
	}

	/* --- let's read a normal file --- */
	if (!input && !interactive && ia<ac) {
		/* prepare arguments for program */
		iaa = 0;
		for (ir=ia+1; ir<ac; ir++) {
			if (parse_args) {
				Lscpy(&args[iaa], av[ir]);
				if (++iaa >= MAXARGS) break;
			} else {
				Lcat(&args[0], av[ir]);
				if (ir<ac-1) Lcat(&args[0]," ");
			}
		}
		RxRun(av[ia],NULL,args,&tracestr,NULL);
	} else {
		if (interactive)
			Lcat(&file,
				"signal on syntax;"
				"signal on error;"
				"signal on halt;"
				"start:do forever;"
				"call write ,\">>> \";"
				" parse pull _;"
				" result=@r;"
				" interpret _;"
				" @r=result;"
				"end;"
				"signal start;"
				"syntax:;error: say \"+++ Error\" RC\":\" errortext(RC);"
				"signal start;"
				"halt:");
		else
		if (ia>=ac) {
			Lread(STDIN,&file,LREADFILE);
		} else {
			/* Copy a small header */
			if (loop_over_stdin)
				Lcat(&file,"do forever;"
					"linein=read();"
					"if eof(0) then exit;");
			for (;ia<ac; ia++) {
				Lcat(&file,av[ia]);
				if (ia<ac-1) Lcat(&file," ");
			}
			/* and a footer */
			if (loop_over_stdin)
				Lcat(&file,";end");
		}
		RxRun(NULL,&file,args,&tracestr,NULL);
	}

	/* --- Free everything --- */
	RxFinalize();
	for (ia=0; ia<MAXARGS; ia++) LFREESTR(args[ia]);
	LFREESTR(tracestr);
	LFREESTR(file);

#ifdef __DEBUG__
	if (mem_allocated()!=0) {
		fprintf(STDERR,"\nMemory left allocated: %ld\n",mem_allocated());
		mem_list();
	}
	if(debugInfo != NULL) {
		free(debugInfo);
	}
#endif

	return rxReturnCode;
} /* main */
