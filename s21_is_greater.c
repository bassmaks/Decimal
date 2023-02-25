#include "s21_decimal.h"

/*
Оператор сравнения - БОЛЬШЕ
Возвращаемое значение:
0 - FALSE
1 - TRUE
*/

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  normalization(&value_1, &value_2);
  if (get_sign(value_1) == get_sign(value_2) && get_sign(value_1) == 0) {
    res = (value1_greater_value2(value_1, value_2) == 1) ? TRUE : FALSE;
  }
  if (get_sign(value_1) == get_sign(value_2) && get_sign(value_1) == 1) {
    res = (value1_greater_value2(value_1, value_2) == 2) ? TRUE : FALSE;
  }
  if (get_sign(value_1) == 1 && get_sign(value_2) == 0) res = FALSE;
  if (get_sign(value_1) == 0 && get_sign(value_2) == 1) {
    res = value_1.bits[0] == 0 && value_1.bits[1] == 0 &&
                  value_1.bits[2] == 0 && value_2.bits[0] == 0 &&
                  value_2.bits[1] == 0 && value_2.bits[2] == 0
              ? FALSE
              : TRUE;
  }
  return res;
}
