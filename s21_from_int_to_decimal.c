#include "s21_decimal.h"

/*
Перевод переменной src из типа int в тип s21_decimal
Возвращаемое значение - код ошибки:
0 - OK
1 - ошибка конвертации
*/

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  int res = 1;
  if (!(src == S21_INF || src == S21_NAN || dst == NULL ||
        src == S21_NEGATIVE_INF)) {
    zero_decimal(dst);
    dst->bits[3] = 0;
    if (src < 0) {
      set_sign(dst, 1);
      src *= -1;
    }
    dst->bits[0] = src;
    res = 0;
  }
  return res;
}
