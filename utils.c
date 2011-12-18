#include "utils.h"

/* convert n to characters in s */
void itoa(int n, char s[])
{
	int i, sign;
	if ((sign = n) < 0)	/* record sign */
		n = -n;		/* make n positive */
	i = 0;
	do {			/* generate digits in reverse order */
		s[i++] = n % 10 + '0';	/* get next digit */
	} while ((n /= 10) > 0);	/* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

/* converts s to integer in n base */
int atoi(char s[])
{
	int i = 0;
	int n = 0;
	int l = strlen(s);
	reverse(s);
	do {
		n = n + (s[i]*(pow(10,i)));
	i++;
	} while (i<l);
	// n = s0*10^0 + s1*10^1 + s2*10^2
	return n;
}

/* calculate power of a number in a certain base */
int pow(int base, int n)
{
	int i, p;
	i = 1;
	p = 1;
	do {
		p = p * base;
		i++;
	} while(i<=n);
	return p;
}

/* reverse string s in place */
void reverse(char s[])
{
	int i, j;
	char c;
	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/* determines the length of a C character string */
int strlen(char s[])
{
	int i = 0;
	while (s[i] != '\0') {
		i++;
	}
	return i;
}

/* compares two strings */
int strcmp(char* a, char* b)
{
	if (!a || !b ) return 0;
	while ((char) *a == (char) *b ) {
		if ((((char) *a) == '\0' ) && (((char) *b) == '\0'))
			return 1;
		a++; b++;
	}
	return 0;
}

/* parse int to hex */
char *inthex(unsigned val)
{
	static char hexdigit[20] = "0123456789abcdef";
	static char string[12];
	int i;
	for (i = 0; i < 8; i++)
		string[i] = hexdigit[(val >>((7-i)*4)) & 0xf];
	string[i] = 0;
	return string;
}

/* parse hex to int */
unsigned hexint(char *s)
{
	unsigned res = 0, i;
	for (i=0; i<8; i++) {
		if (s[i]>='0' && s[i]<='9') {
			res = (res << 4) | (s[i]-'0');
		} else if (s[i]>='a' && s[i]<='f') {
			res = (res << 4) | (s[i]-'a'+10);
		} else if (s[i]>='A' && s[i]<='F') {
			res = (res << 4) | (s[i]-'A'+10);
		} else
			break;
	}
	return res;
}

