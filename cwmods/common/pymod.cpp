#include "pydiv.h"
#include "pymod.h"
i64 pydiv(i64 a, i64 b) {
	i64 result = a / b;
	if ((a < 0) ^ (b < 0)) {
		if (pymod(a, b) != 0) {
			result -= 1;
		}
	}
	return result;
}