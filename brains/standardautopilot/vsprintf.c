/* Parts of this code (C) Symantec */

#include "vsprintf.h"

static strlen(char *s)
	{
	int i=0;
	while (*s++) i++;
	return(i);
	}

static struct format
	{
	unsigned 		leftJustify : 1;
	unsigned 		forceSign : 1;
	unsigned 		altForm : 1;
	unsigned 		zeroPad : 1;
	unsigned 		havePrecision : 1;
	unsigned 		hSize : 1;
	unsigned 		lSize : 1;
	unsigned 		LSize : 1;
	char			sign;
	char			exponent;
	int				fieldWidth;
	int				precision;
	} default_format;

struct decrec
	{
	char			sgn;
	short			exp;
//	char			sig[SIGDIGLEN];
	short			pad;
	// following fields aren't used by SANE
	short			min;
	short			dot;
	short			max;
	};

#define BUFLEN			512

int vsprintf(char *sbuffer, char *fmt, va_list arg)
	{
	register int c, i, j, nwritten = 0;
	register unsigned long n;
	register char *s;
	char buf[BUFLEN], *digits, *t;
	struct format F;
	struct decrec D;

	for (c = *fmt; c; c = *++fmt)
		{
		if (c != '%') goto copy1;
		F = default_format;

			//  decode flags

		for (;;)
			{
			c = *++fmt;
			if      (c == '-')	F.leftJustify = TRUE;
			else if (c == '+')	F.forceSign = TRUE;
			else if (c == ' ')	F.sign = ' ';
			else if (c == '#')	F.altForm = TRUE;
			else if (c == '0')	F.zeroPad = TRUE;
			else break;
			}

			//  decode field width

		if (c == '*')
			{
			if ((F.fieldWidth = va_arg(arg, int)) < 0)
				{
				F.leftJustify = TRUE;
				F.fieldWidth = -F.fieldWidth;
				}
			c = *++fmt;
			}
		else
			{
			for (; c >= '0' && c <= '9'; c = *++fmt)
				F.fieldWidth = (10 * F.fieldWidth) + (c - '0');
			}

			//  decode precision

		if (c == '.')
			{
			if ((c = *++fmt) == '*')
				{ F.precision = va_arg(arg, int); c = *++fmt; }
			else for (; c >= '0' && c <= '9'; c = *++fmt)
					F.precision = (10 * F.precision) + (c - '0');
			if (F.precision >= 0) F.havePrecision = TRUE;
			}

			//  perform appropriate conversion

		s = &buf[BUFLEN];
		if (F.leftJustify) F.zeroPad = FALSE;

conv:	switch (c)
			{
			case 'h' :	F.hSize = TRUE; c = *++fmt; goto conv;
			case 'l' :	F.lSize = TRUE; c = *++fmt; goto conv;
			case 'L' :	F.LSize = TRUE; c = *++fmt; goto conv;
			case 'd' :
			case 'i' :	if (F.lSize) n = va_arg(arg, long);
						else n = va_arg(arg, int);
						if (F.hSize) n = (short) n;
						if ((long) n < 0) { n = -n; F.sign = '-'; }
						else if (F.forceSign) F.sign = '+';
						goto decimal;
			case 'u' :	if (F.lSize) n = va_arg(arg, unsigned long);
						else n = va_arg(arg, unsigned int);
						if (F.hSize) n = (unsigned short) n;
						F.sign = 0;
						goto decimal;
			decimal:	if (!F.havePrecision)
							{
							if (F.zeroPad)
								{
								F.precision = F.fieldWidth;
								if (F.sign) --F.precision;
								}
							if (F.precision < 1) F.precision = 1;
							}
						for (i = 0; n; n /= 10, i++) *--s = n % 10 + '0';
						for (; i < F.precision; i++) *--s = '0';
						if (F.sign) { *--s = F.sign; i++; }
						break;

			case 'o' :	if (F.lSize) n = va_arg(arg, unsigned long);
						else n = va_arg(arg, unsigned int);
						if (F.hSize) n = (unsigned short) n;
						if (!F.havePrecision)
							{
							if (F.zeroPad) F.precision = F.fieldWidth;
							if (F.precision < 1) F.precision = 1;
							}
						for (i = 0; n; n /= 8, i++) *--s = n % 8 + '0';
						if (F.altForm && i && *s != '0') { *--s = '0'; i++; }
						for (; i < F.precision; i++) *--s = '0';
						break;

			case 'p' :	F.havePrecision = F.lSize = TRUE;
						F.precision = 8;
			case 'X' :	digits = "0123456789ABCDEF";
						goto hexadecimal;
			case 'x' :	digits = "0123456789abcdef";
			hexadecimal:if (F.lSize) n = va_arg(arg, unsigned long);
						else n = va_arg(arg, unsigned int);
						if (F.hSize) n = (unsigned short) n;
						if (!F.havePrecision)
							{
							if (F.zeroPad)
								{
								F.precision = F.fieldWidth;
								if (F.altForm) F.precision -= 2;
								}
							if (F.precision < 1) F.precision = 1;
							}
						for (i = 0; n; n /= 16, i++) *--s = digits[n % 16];
						for (; i < F.precision; i++) *--s = '0';
						if (F.altForm) { *--s = c; *--s = '0'; i += 2; }
						break;

			case 'c' :	*--s = va_arg(arg, int); i = 1; break;

			case 's' :	s = va_arg(arg, char *);
						if (F.altForm)
							{
							i = (unsigned char) *s++;
							if (F.havePrecision && i > F.precision) i = F.precision;
							}
						else
							{
							i = strlen(s);
							if (F.havePrecision && i > F.precision) i = F.precision;
							}
						break;

			case 'n' :	s = va_arg(arg, void *);
						if      (F.hSize) * (short *) s = nwritten;
						else if (F.lSize) * (long  *) s = nwritten;
						else              * (int   *) s = nwritten;
						continue;

				//  oops - unknown conversion, abort

			case 'M': case 'N': case 'O': case 'P': case 'Q':
			case 'R': case 'S': case 'T': case 'U': case 'V':
			// (extra cases force this to be an indexed switch)
			default: goto done;

			case '%' :
			copy1    :	*sbuffer++ = c; ++nwritten; continue;
			}

			//  pad on the left

		if (i < F.fieldWidth && !F.leftJustify)
			do { *sbuffer++ = ' '; ++nwritten; } while (i < --F.fieldWidth);

			//  write the converted result

		for (j=0; j<i; j++) *sbuffer++ = *s++;
		nwritten += i;

			//  pad on the right

		for (; i < F.fieldWidth; i++)
			{ *sbuffer++ = ' '; ++nwritten; }
		}

done: return(nwritten);
	}
