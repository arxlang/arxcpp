#include <arrow-glib/arrow-glib.h>
#include <iostream>

extern "C" {
GArrowFloatScalar* get_constant(double);
}

int main() {
  gint32 myIntValue = 42;
  GArrowInt32Scalar* myIntScalar = garrow_int32_scalar_new(myIntValue);
  gint32 myIntResult = garrow_int32_scalar_get_value(myIntScalar);
  printf("(arrow-glib) The scalar integer value is: %d\n", myIntResult);

  gfloat myFloatValue = 42.0;
  GArrowFloatScalar* myFloatScalar = garrow_float_scalar_new(myFloatValue);
  gfloat myFloatResult = garrow_float_scalar_get_value(myFloatScalar);
  printf("(arrow-glib) The scalar float value is: %f\n", myFloatResult);

  GArrowFloatScalar* myArxFloatScalar = get_constant(3.0);
  // gfloat myArxFloatResult = garrow_float_scalar_get_value(myArxFloatScalar);
  // printf("(arx) get_constant(3): %f\n", myArxFloatResult);

  return 0;
}
