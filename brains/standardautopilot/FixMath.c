#include <stdint.h>
#include <math.h>
#include "FixMath.h"

#define kFixedOne  (1L << 16)
#define kFractOne  (1L << 30)

static inline double FixedToDouble(Fixed x) { return (double)x / (double)kFixedOne; }
static inline Fixed  DoubleToFixed(double x) {
    double s = x * (double)kFixedOne;
    if (s > 2147483647.0) s = 2147483647.0;
    if (s < -2147483648.0) s = -2147483648.0;
    return (Fixed)llround(s);
}
static inline Fract  DoubleToFract(double x) {
    // assumes x in roughly [-2, +2]
    double s = x * (double)kFractOne;
    if (s > 2147483647.0) s = 2147483647.0;
    if (s < -2147483648.0) s = -2147483648.0;
    return (Fract)llround(s);
}

// FracSin/FracCos: input angle is Fixed radians; result is Fract (sin/cos in [-1,1])
extern Fract FracSin(Fixed angleRad) {
    return DoubleToFract(sin(FixedToDouble(angleRad)));
}
extern Fract FracCos(Fixed angleRad) {
    return DoubleToFract(cos(FixedToDouble(angleRad)));
}

// FixATan2: returns Fixed radians angle (atan2(y, x))
extern Fixed FixATan2(LongInt x, LongInt y) {
    double a = atan2((double)y, (double)x);
    return DoubleToFixed(a);
}
