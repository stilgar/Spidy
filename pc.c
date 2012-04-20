#include <stdio.h>
#include <stdint.h>
#include "spidy.h"
#include "pc.h"

/* Transform from engine number 00-01-02-10-11-12-...-50-51-52 to
 * a sequential counter 0-1-2-3-4-5-...-17 or -1 if it doesn't recognize
 * engine code.
 */
int eng2num(int engine)
{
	if (engine >= 0 && engine <= 52 && (engine%10) <= 2)
		return ((engine / 10) * 3 + (engine % 10));
	else
		return -1;
}

/* It's a reverse function of eng2num */
int num2eng(int number)
{
	if (number >= 0 && number <= 17)
		return ((number / 3) * 10 + (number % 3));
	else
		return -1;
}

/* Transform from degree (from -90 to 90) to corresponding
 * char sequence (from DEG_NEG90 to DEG_POS90)
 */
int deg2chr(int degree)
{
	if (degree >= -90 && degree <= 90)
		return (degree + 90 + DEG_NEG90);
	else
		return 0;
}

/* It's a reverse function of eng2num
 * using -91 as an error
 */
int chr2deg(int character)
{
	if (character >= DEG_NEG90 && character <= DEG_POS90)
		return (character - 90 - DEG_NEG90);
	else
		return -91;
}
