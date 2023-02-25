#include "s21_decimal.h"

/*
Оператор сравнения - НЕ РАВНО
Возвращаемое значение:
0 - FALSE
1 - TRUE
*/

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  res = s21_is_equal(value_1, value_2) == FALSE ? TRUE : FALSE;
  return res;
}
