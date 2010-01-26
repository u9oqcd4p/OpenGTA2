/*
 * Copyright (c) 2007 Thomas J. Merritt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain this noticein its entirety,
 *    including the above copyright notice, this list of conditions and the
 *    following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of any
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 4. Any distribution that includes this software must be registered at
 *    <http://www.merritts.org/utf8str/>.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __UTF8STR_H__
#define __UTF8STR_H__

/* UTF-8 string functions */
/* Generated strings are always nul terminated with proper UTF-8 encoding */
/* of characters outside of the 1..127 range and never include a BOM. */
/* Destination strings are always specified with a length to avoid */
/* buffer overflow problems. */

/*
 * UTF-8 info
 *
 *    0XXX XXXX						00000 - 0007F
 *    110X XXXX 10XX XXXX				00080 - 007FF
 *    1110 XXXX 10XX XXXX 10XX XXXX			00800 - 0FFFF
 *    1111 0XXX 10XX XXXX 10XX XXXX 10XX XXXX		10000 - 3FFFF
 *    1111 10XX 10XX XXXX 10XX XXXX 10XX XXXX 10XX XXXX	invalid
 *    1111 11XX						invalid
 *
 *			XXX XXXX		00000-0007F
 *		   XXX XXXX XXXX		00080-007FF
 *	     XXXX XXXX XXXX XXXX		00800-0FFFF
 *    X XXXX XXXX XXXX XXXX XXXX		10000-3FFFF
 */

#define UTF8_LEGAL_STARTCHAR(src)		\
	((src[0] & 0x80) == 0 ||		\
	 ((src[0] & 0xFF) >= 0xC2 &&		\
	    (src[0] & 0xFF) <= 0xDF) ||		\
	 (src[0] & 0xF0) == 0xE0 ||		\
	 (src[0] & 0xFF) == 0xF0)

#define UTF8_SEQ_LENGTH(src)			\
	((src[0] & 0x80) == 0 ? 1 : 		\
	 (((src[0] & 0xFF) >= 0xC2 &&		\
	    (src[0] & 0xFF) <= 0xDF) ? 2 : 	\
	 ((src[0] & 0xF0) == 0xE0 ? 3 : 	\
	 ((src[0] & 0xFF) == 0xF0 ? 4 : 0))))

#define UTF8_SEQ_VALID(src)			\
	((src[0] & 0x80) == 0 ? 1 : 		\
	 (((src[0] & 0xFF) >= 0xC2 &&		\
	    (src[0] & 0xFF) <= 0xDF &&		\
	    (src[1] & 0xC0) == 0x80) ? 2 : 	\
	 ((src[0] & 0xF0) == 0xE0 &&		\
	   (src[1] & 0xC0) == 0x80 &&		\
	   (src[2] & 0xC0) == 0x80 ? 3 : 	\
	 ((src[0] & 0xFF) == 0xF0 &&		\
	   (src[1] & 0xC0) == 0x80 &&		\
	   (src[2] & 0xC0) == 0x80 &&		\
	   (src[3] & 0xC0) == 0x80 ? 4 : 0))))

#define UTF8_GET_CHAR(src)			\
	((src[0] & 0x80) == 0 ? src[0] : 	\
	 (((src[0] & 0xFF) >= 0xC2 &&		\
	    (src[0] & 0xFF) <= 0xDF &&		\
	    (src[1] & 0xC0) == 0x80) ?		\
	      ((src[0] & 0x1F) << 6) |		\
	       (src[1] & 0x3F) : 		\
	 ((src[0] & 0xF0) == 0xE0 &&		\
	   (src[1] & 0xC0) == 0x80 &&		\
	   (src[2] & 0xC0) == 0x80 ?		\
	     ((src[0] & 0x0F) << 12) |		\
	     ((src[1] & 0x3F) << 6) |		\
	      (src[2] & 0x3F) :			\
	 ((src[0] & 0xFF) == 0xF0 &&		\
	   (src[1] & 0xC0) == 0x80 &&		\
	   (src[2] & 0xC0) == 0x80 &&		\
	   (src[3] & 0xC0) == 0x80 ?		\
	     ((src[1] & 0x3F) << 12) |		\
	     ((src[2] & 0x3F) << 6) |		\
	      (src[3] & 0x3F) : -1))))

#define UTF8_CHAR_LENGTH(ch)			\
	((ch >= 0x00000 && ch <= 0x0007F) ? 1 : \
	 (ch >= 0x00080 && ch <= 0x007FF) ? 2 :	\
	 (ch >= 0x00800 && ch <= 0x0FFFF) ? 3 :	\
	 (ch >= 0x10000 && ch <= 0x3FFFF) ? 4 : 0)

#define UTF8_PUT_CHAR(dest, ch)				\
	((ch >= 0x00000 && ch <= 0x0007F) ? 		\
	    (dest[0] = ch) :				\
	 (ch >= 0x00080 && ch <= 0x007FF) ? 		\
	    ((dest[0] = ((ch >>  6) & 0x1F) | 0xC0),	\
	     (dest[1] = ( ch        & 0x3F) | 0x80)) :	\
	 (ch >= 0x00800 && ch <= 0x0FFFF) ? 		\
	    ((dest[0] = ((ch >> 12) & 0x0F) | 0xE0),	\
	     (dest[1] = ((ch >>  6) & 0x3F) | 0x80),	\
	     (dest[2] = ( ch        & 0x3F) | 0x80)) :	\
	 (ch >= 0x10000 && ch <= 0x3FFFF) ? 		\
	    ((dest[0] = 0xF0),				\
	     (dest[1] = ((ch >> 12) & 0x3F) | 0x80),	\
	     (dest[2] = ((ch >>  6) & 0x3F) | 0x80),	\
	     (dest[3] = ( ch        & 0x3F) | 0x80)) : 0)

extern int utf8len(char const *str);
extern int utf8seqvalid(char const *src);
extern int utf8seqlen(char const *src);
extern int utf8getchar(char const *src);
extern int utf8charlen(int ch);
extern void utf8putchar(char *dest, int ch);
extern char *utf8index(char const *s, int n);
extern int utf8cpy(char *dest, int destlen, char const *str);
extern int utf8ncpy(char *dst, int destlen, char const *src, int numchars);
extern int utf8cat(char *dest, int destlen, char const *str);
extern int utf8ncat(char *dest, int destlen, char const *str, int numchars);
extern char *utf8dup(char const *src);
extern int utf8cmp(char const *str1, char const *str2);
extern int utf8ncmp(char const *str1, char const *str2, int numchars);
extern int utf8casecmp(char const *str1, char const *str2);
extern int utf8ncasecmp(char const *str1, char const *str2, int numchars);
extern char *utf8chr(char *str, int c);
extern char *utf8rchr(char *str, int c);
extern char *utf8str(char const *big, char const *little);
extern char *utf8nstr(char const *big, char const *little, int numchars);
extern char *utf8casestr(char const *big, char const *little);
extern char *utf8ncasestr(char const *big, char const *little, int numchars);
extern int utf8spn(char const *s, char const *charset);
extern int utf8cspn(char const *s, char const *charset);
extern char *utf8pbrk(char const *s, char *charset);
extern char *utf8sep(char **stringp, char *delim);
extern char *utf8tok(char *str, char *sep, char **last);

#endif /* __UTF8STR_H__ */
