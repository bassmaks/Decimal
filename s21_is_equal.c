#include "s21_decimal.h"

/*
Оператор сравнения - РАВНО
Возвращаемое значение:
0 - FALSE
1 - TRUE
*/

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  normalization(&value_1, &value_2);
  if (get_sign(value_1) == get_sign(value_2)) {
    res = TRUE;
    for (int i = 0; i < 96 && res == TRUE; i++) {
      res = get_bit(value_1, i) != get_bit(value_2, i) ? FALSE : TRUE;
    }
  } else {
    res = value_1.bits[0] == 0 && value_1.bits[1] == 0 &&
                  value_1.bits[2] == 0 && value_2.bits[0] == 0 &&
                  value_2.bits[1] == 0 && value_2.bits[2] == 0
              ? TRUE
              : FALSE;
  }
  return res;
}
