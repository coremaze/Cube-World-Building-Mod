#include "pymod.h"

i64 pymod(i64 a, i64 b) {
	i64 result = a % b;
	if (result < 0) {
		result = b + result;
	}
	return result;
}
