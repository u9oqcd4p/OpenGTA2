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

/* UTF-8 string functions */
/* Generated strings are always nul terminated with proper UTF-8 encoding */
/* of characters outside of the 1..127 range and never include a BOM. */
/* Destination strings are always specified with a length to avoid */
/* buffer overflow problems. */

#include <stdlib.h>
#include <string.h>

#include "utf8.h"

int
utf8seqvalid(char const *src)
{
	if (src == NULL)
	return 0;

	return UTF8_SEQ_VALID(src);
}

int
utf8seqlen(char const *src)
{
	if (src == NULL)
	return 0;

	return UTF8_SEQ_LENGTH(src);
}

int
utf8getchar(char const *src)
{
	if (src == NULL)
	return -1;

	return UTF8_GET_CHAR(src);
}

int
utf8charlen(int ch)
{
	return UTF8_CHAR_LENGTH(ch);
}

void
utf8putchar(char *dest, int ch)
{
	if (dest)
	UTF8_PUT_CHAR(dest, ch);
}

//#define UTF8_OPTIMIZE_SPACE
#ifdef UTF8_OPTIMIZE_SPACE
#undef UTF8_SEQ_VALID
#undef UTF8_SEQ_LENGTH
#undef UTF8_GET_CHAR
#undef UTF8_CHAR_LENGTH
#undef UTF8_PUT_CHAR
#define UTF8_SEQ_VALID(s)	utf8seqvalid(s)
#define UTF8_SEQ_LENGTH(s)	utf8seqlen(s)
#define UTF8_GET_CHAR(s)	utf8getchar(s)
#define UTF8_CHAR_LENGTH(c)	utf8charlen(c)
#define UTF8_PUT_CHAR(d, c)	utf8putchar(d, c)
#endif

int
utf8len(char const *str)
{
	int len = 0;

	if (str == NULL)
	return 0;

	while (*str)
	{
	int sl = UTF8_SEQ_LENGTH(str);

	if (sl)
	{
		len++;
		str += sl;
	}
	else
		str++;
	}

	return len;
}

int
utf8cpy(char *dest, int destlen, char const *str)
{
	int n = 0;

	while (*str)
	{
	int ch = UTF8_GET_CHAR(str);

	if (ch > 0)
	{
		int sl = UTF8_CHAR_LENGTH(ch);

		if (destlen <= sl)
		break;

		UTF8_PUT_CHAR(dest, ch);
		str += sl;
		dest += sl;
		destlen -= sl;
		n++;
	}
	else if (ch)
		str++;
	else
		break;
	}

	if (destlen > 1)
	*dest = '\0';

	return n;
}

int
utf8ncpy(char *dest, int destlen, char const *src, int numchars)
{
	int n = 0;

	while (*src && n < numchars)
	{
	int ch = UTF8_GET_CHAR(src);

	if (ch > 0)
	{
		int sl = UTF8_CHAR_LENGTH(ch);

		if (destlen <= sl)
		break;

		UTF8_PUT_CHAR(dest, ch);
		src += sl;
		dest += sl;
		destlen -= sl;
		n++;
	}
	else if (ch)
		src++;
	else
		break;
	}

	if (destlen > 1)
	*dest = '\0';

	return n;
}

int
utf8cat(char *dest, int destlen, char const *str)
{
	int l = strlen(dest);

	if (l >= destlen)
	return 0;

	return utf8cpy(&dest[l], destlen - l, str);
}

int
utf8ncat(char *dest, int destlen, char const *str, int numchars)
{
	int l = strlen(dest);

	if (l >= destlen)
	return 0;

	return utf8ncpy(&dest[l], destlen - l, str, numchars);
}

char *
utf8dup(char const *src)
{
	int l = strlen(src);
	char *s = (char *)malloc(l + 1);

	if (s == NULL)
	return NULL;

	utf8cpy(s, l + 1, src);
	return s;
}

int
utf8cmp(char const *str1, char const *str2)
{
	int ch1;
	int ch2;
	int l;

	while (*str1 && *str2)
	{
	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);

	if (ch1 != ch2)
		return ch1 - ch2;

	l = UTF8_CHAR_LENGTH(ch1);
	str1 += l;
	str2 += l;
	}

	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);
	return ch1 - ch2;
}

int
utf8ncmp(char const *str1, char const *str2, int numchars)
{
	int ch1;
	int ch2;
	int l;
	int n = 0;

	while (*str1 && *str2 && n < numchars)
	{
	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);

	if (ch1 != ch2)
		return ch1 - ch2;

	l = UTF8_CHAR_LENGTH(ch1);
	str1 += l;
	str2 += l;
	}

	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);
	return ch1 - ch2;
}

#define UTF8_TOUPPER(ch)		\
	((ch >= 'a' && ch <= 'z') ? (ch - 'a' + 'A') : ch)

int
utf8casecmp(char const *str1, char const *str2)
{
	int ch1;
	int ch2;
	int l;

	while (*str1 && *str2)
	{
	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);

	ch1 = UTF8_TOUPPER(ch1);
	ch2 = UTF8_TOUPPER(ch2);

	if (ch1 != ch2)
		return ch1 - ch2;

	l = UTF8_CHAR_LENGTH(ch1);
	str1 += l;
	str2 += l;
	}

	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);

	ch1 = UTF8_TOUPPER(ch1);
	ch2 = UTF8_TOUPPER(ch2);

	return ch1 - ch2;
}

int
utf8ncasecmp(char const *str1, char const *str2, int numchars)
{
	int ch1;
	int ch2;
	int l;
	int n = 1;

	while (*str1 && *str2 && n < numchars)
	{
	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);

	ch1 = UTF8_TOUPPER(ch1);
	ch2 = UTF8_TOUPPER(ch2);

	if (ch1 != ch2)
		return ch1 - ch2;

	l = UTF8_CHAR_LENGTH(ch1);
	str1 += l;
	str2 += l;

	n++;
	}

	ch1 = UTF8_GET_CHAR(str1);
	ch2 = UTF8_GET_CHAR(str2);

	ch1 = UTF8_TOUPPER(ch1);
	ch2 = UTF8_TOUPPER(ch2);

	return ch1 - ch2;
}

char *
utf8chr(char *str, int c)
{
	if (c <= 127)
	return strchr(str, c);

	while (*str)
	{
	int ch = UTF8_GET_CHAR(str);
	int l = UTF8_CHAR_LENGTH(ch);

	if (ch == c)
		return (char *)str;

	str += l;
	}

	return NULL;
}

char *
utf8rchr(char *str, int c)
{
	char *last = NULL;

	if (c <= 127)
	return strrchr(str, c);

	while (*str)
	{
	int ch = UTF8_GET_CHAR(str);
	int l = UTF8_CHAR_LENGTH(ch);

	if (ch == c)
		last = (char *)str;

	str += l;
	}

	return last;
}

char *
utf8str(char const *big, char const *little)
{
	int ch1 = UTF8_GET_CHAR(big);
	int ll = utf8len(little);

	while (ch1)
	{
	if (utf8ncmp(big, little, ll) == 0)
		return (char *)big;

	big += UTF8_CHAR_LENGTH(ch1);
	ch1 = UTF8_GET_CHAR(big);
	}

	return NULL;
}

char *
utf8nstr(char const *big, char const *little, int numchars)
{
	int ch1 = UTF8_GET_CHAR(big);
	int ll = utf8len(little);

	while (ch1 && numchars >= ll)
	{
	if (utf8ncmp(big, little, ll) == 0)
		return (char *)big;

	big += UTF8_CHAR_LENGTH(ch1);
	ch1 = UTF8_GET_CHAR(big);
	numchars--;
	}

	return NULL;
}

char *
utf8casestr(char const *big, char const *little)
{
	int ch1 = UTF8_GET_CHAR(big);
	int ll = utf8len(little);

	while (ch1)
	{
	if (utf8ncasecmp(big, little, ll) == 0)
		return (char *)big;

	big += UTF8_CHAR_LENGTH(ch1);
	ch1 = UTF8_GET_CHAR(big);
	}

	return NULL;
}

char *
utf8ncasestr(char const *big, char const *little, int numchars)
{
	int ch1 = UTF8_GET_CHAR(big);
	int ll = utf8len(little);

	while (ch1 && numchars >= ll)
	{
	if (utf8ncasecmp(big, little, ll) == 0)
		return (char *)big;

	big += UTF8_CHAR_LENGTH(ch1);
	ch1 = UTF8_GET_CHAR(big);
	numchars--;
	}

	return NULL;
}

int
utf8spn(char const *s, char *charset)
{
	int ch = UTF8_GET_CHAR(s);
	int n = 0;
	int l;

	while (ch)
	{
	if (utf8chr(charset, ch) == NULL)
		break;

	n++;
	l = UTF8_CHAR_LENGTH(ch);
	s += l;
	ch = UTF8_GET_CHAR(s);
	}

	return n;
}

int
utf8cspn(char const *s, char *charset)
{
	int ch = UTF8_GET_CHAR(s);
	int n = 0;
	int l;

	while (ch)
	{
	if (utf8chr(charset, ch) != NULL)
		break;

	n++;
	l = UTF8_CHAR_LENGTH(ch);
	s += l;
	ch = UTF8_GET_CHAR(s);
	}

	return n;
}

char *
utf8pbrk(char const *s, char *charset)
{
	int ch = UTF8_GET_CHAR(s);
	int l;

	while (ch)
	{
	if (utf8chr(charset, ch) != NULL)
		return (char *)s;

	l = UTF8_CHAR_LENGTH(ch);
	s += l;
	ch = UTF8_GET_CHAR(s);
	}

	return NULL;
}

char *
utf8index(char const *s, int n)
{
	while (n > 0)
	{
	int ch = UTF8_GET_CHAR(s);

	if (!ch)
		return NULL;

	s += UTF8_CHAR_LENGTH(ch);
	n--;
	}

	return (char *)s;
}


char *
utf8sep(char **stringp, char *delim)
{
	char *s = *stringp;
	char *r = s;
	char *n = NULL;
	int ch;
	int l;

	if (s == NULL)
	return NULL;

	ch = UTF8_GET_CHAR(s);

	while (ch)
	{
	l = UTF8_CHAR_LENGTH(ch);

	if (utf8chr(delim, ch) != NULL)
	{
		*s = '\0';
		s += l;
		*stringp = s;
		return r;
	}

	s += l;
	ch = UTF8_GET_CHAR(s);
	}

	*stringp = NULL;
	return r;
}

char *
utf8tok(char *str, char *sep, char **last)
{
	int ch;
	char *s = NULL;

	if (str == NULL)
	{
	if (last == NULL)
		return NULL;

	str = *last;
	}

	if (str == NULL)
	return NULL;

	ch = UTF8_GET_CHAR(str);

	while (ch)
	{
	char *r = utf8chr(sep, ch);

	if (s == NULL && r == NULL)
		s = str;

	if (s != NULL && r != NULL)
		break;

	str += UTF8_CHAR_LENGTH(ch);
	ch = UTF8_GET_CHAR(str);
	}

	if (s == str)
	s = NULL;

	if (ch)
	{
	*str = '\0';

	if (last)
	{
		str += UTF8_CHAR_LENGTH(ch);
		*last = str;
	}
	}
	else if (last)
	*last = NULL;

	return s;
}