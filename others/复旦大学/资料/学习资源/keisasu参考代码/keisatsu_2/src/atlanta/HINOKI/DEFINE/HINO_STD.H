#ifndef STD_H
    #define STD_H
/***********************************************************************
* DESCRIPTION:   変数型定義
************************************************************************/
	#define	BOOL			char			/* 1Byte:TRUE or FALSE */
	#define	BYTE			char			/* 1Byte */
	#define	UBYTE			unsigned char	/* 1Byte:符号無 */
	#define	WORD			short			/* 2Byte */
		#define	WORD_MAX	(WORD)32767
		#define	WORD_MIN	(WORD)(-32768)
	#define	UWORD			unsigned short	/* 2Byte:符号無 */
	#define	DWORD			long			/* 4Byte */
	#define	UDWORD			unsigned long	/* 4Byte:符号無 */
	#define CONST			const			/* ROM */
/***********************************************************************
* DESCRIPTION:   標準定義
************************************************************************/
#define		TRUE	1
#define		FALSE	0

#define		OK		1
#define		NG		0
#define		ERROR	-1

#define		YES		1
#define		NO		0

#define		NULL	0x00
#define		CR		0x0D

#ifndef	SPACE
	#define	SPACE	0x20
#endif

#endif
