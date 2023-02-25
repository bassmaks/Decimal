#include "s21_decimal.h"

/*
Оператор сравнения - БОЛЬШЕ ИЛИ РАВНО
Возвращаемое значение:
0 - FALSE
1 - TRUE
*/

int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  normalization(&value_1, &value_2);
  res = s21_is_equal(value_1, value_2) == FALSE ? FALSE : TRUE;
  if (res == FALSE)
    res = s21_is_greater(value_1, value_2) == TRUE ? TRUE : FALSE;
  return res;
}
