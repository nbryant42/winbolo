#include <stdint.h>

typedef int32_t Fixed;   // 16.16 signed fixed
typedef int32_t Fract;   // 2.30  signed fixed
typedef int32_t LongInt; // classic long

extern Fract FracSqrt(Fract x);
extern Fract FracCos(Fixed x);
extern Fract FracSin(Fixed x);
extern Fixed FixATan2(LongInt x, LongInt y);