#include "s21_decimal.h"

void zero_decimal(s21_decimal *dst) {
  dst->bits[0] = 0;
  dst->bits[1] = 0;
  dst->bits[2] = 0;
  dst->bits[3] = 0;
}

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

int get_sign(s21_decimal value) { return (get_bit(value, 127)); }
void set_sign(s21_decimal *value, int sign) { set_bit(value, 127, sign); }

int get_bit(s21_decimal value, int position) {
  return (value.bits[position / 32] >> (position % 32)) & 1U;
}

void set_bit(s21_decimal *value, int position, int digit) {
  int mask = 1U << (position % 32);
  if (digit == 1) {
    value->bits[position / 32] |= mask;
  } else {
    value->bits[position / 32] &= ~mask;
  }
}

// void set_bit(s21_decimal *value, int position, int digit) {
//   if (!digit) value->bits[position / 32] = ~value->bits[position / 32];
//   int mask = 1U << (position % 32);
//   value->bits[position / 32] = value->bits[position / 32] | mask;
//   if (!digit) value->bits[position / 32] = ~value->bits[position / 32];
// }

int get_scale(s21_decimal value) {
  int mask = 255;
  value.bits[3] >>= 16;  // сдвиг в позицию степени по заданию
  int scale = value.bits[3] & mask;
  return scale;
}

void set_scale(s21_decimal *value, int scale) {
  for (int start_position = 96 + 16; start_position <= 118; start_position++) {
    set_bit(value, start_position, scale % 2);
    scale /= 2;
  }
  // так как максимальная степень по заданию
  // 28 это 11100 пять знаков 112+5=117
}

int normalization(s21_decimal *value_1, s21_decimal *value_2) {
  int overflow = 0;
  int scale_1 = get_scale(*value_1);
  int scale_2 = get_scale(*value_2);
  if (scale_1 != scale_2) {
    if (scale_1 < scale_2 && overflow != 1) {
      comparison(value_1, value_2);
    }
    if (scale_1 > scale_2 && overflow != 1) {
      comparison(value_2, value_1);
    }
  }
  return overflow;
}

void comparison(s21_decimal *value_1, s21_decimal *value_2) {
  int scale_1 = get_scale(*value_1);
  int scale_2 = get_scale(*value_2);
  int difference_scale = abs(scale_1 - scale_2);
  for (; difference_scale > 0; difference_scale--) {
    if (!upscale_mantissa(value_1)) {
      scale_1++;
    } else {
      downscale_mantissa(value_2);
      scale_2--;
    }
  }
  set_scale(value_1, scale_1);
  set_scale(value_2, scale_2);
}

int upscale_mantissa(s21_decimal *value) {
  int overflow_flag = 0;
  s21_decimal value_1;
  s21_decimal value_2;
  s21_decimal result;
  value_1 = *value;
  overflow_flag = shift_mantissa_right(&value_1, 1);
  // *10=(*2)+(*8) побитово это сдвиг на 1 и сдвиг на 3
  if (overflow_flag == 0) {
    value_2 = *value;
    overflow_flag = shift_mantissa_right(&value_2, 3);
    if (overflow_flag == 0)
      result = sum_mantissa(value_1, value_2, &overflow_flag);  // складываем
  }
  if (overflow_flag == 0) *value = result;
  return overflow_flag;
}

int downscale_mantissa(s21_decimal *value) {
  int overflow_flag;
  s21_decimal value_1 = *value;
  s21_decimal value_2 = {{10, 0, 0, 0}};
  *value = div_mantissa(value_1, value_2, &overflow_flag);
  return overflow_flag;
}

s21_decimal sum_mantissa(s21_decimal value_1, s21_decimal value_2,
                         int *overflow) {
  int in_memory = 0;
  *overflow = 0;
  s21_decimal result = value_1;
  for (int position = 0; position < 96; position++) {
    int value_1_bit = get_bit(value_1, position);
    int value_2_bit = get_bit(value_2, position);
    if (value_1_bit != value_2_bit) {
      in_memory == 0 ? set_bit(&result, position, 1)
                     : set_bit(&result, position, 0);
    }
    if (value_1_bit == 0 && value_2_bit == 0) {
      in_memory == 0 ? set_bit(&result, position, 0)
                     : set_bit(&result, position, 1);
      in_memory = 0;
    }
    if (value_1_bit == 1 && value_2_bit == 1) {
      in_memory == 0 ? set_bit(&result, position, 0)
                     : set_bit(&result, position, 1);
      in_memory = 1;
    }
    if (position == 95) *overflow = (in_memory == 1) ? 1 : 0;
  }
  return result;
}

s21_decimal sub_mantissa(s21_decimal value_1, s21_decimal value_2,
                         int *overflow) {
  *overflow = 0;
  s21_decimal second = {{1, 0, 0, 0}};
  for (int i = 0; i < 3; i++) value_2.bits[i] = ~value_2.bits[i];
  value_2 = sum_mantissa(value_2, second, overflow);
  s21_decimal result = sum_mantissa(value_1, value_2, overflow);
  return result;
}

int value1_greater_value2(s21_decimal value_1, s21_decimal value_2) {
  int res = 0;
  for (int i = 95; i >= 0 && res == 0; i--) {
    if (get_bit(value_1, i) - get_bit(value_2, i) == 1) res = 1;
    if (get_bit(value_1, i) - get_bit(value_2, i) == -1) res = 2;
  }
  return res;
}

s21_decimal div_mantissa(s21_decimal value_1, s21_decimal value_2,
                         int *overflow) {
  *overflow = 0;
  int res = 0;
  s21_decimal result;
  for (; value1_greater_value2(value_1, value_2) == 1 ||
         value1_greater_value2(value_1, value_2) == 0;) {
    res += for_div(value_1, value_2, &result);
    value_1 = sub_mantissa(value_1, result, overflow);
  }
  s21_from_int_to_decimal(res, &result);
  return result;
}

int for_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int count = 0;
  for (; value1_greater_value2(value_1, value_2) == 1 ||
         value1_greater_value2(value_1, value_2) == 0;
       count++) {
    *result = value_2;
    shift_mantissa_right(&value_2, 1);
  }
  return pow(2, count - 1);
}

int shift_mantissa_right(s21_decimal *value, int shift) {
  int flag = 0;
  for (; shift > 0 && flag == 0; shift--) {
    if (!get_bit(*value, 95)) {
      value->bits[2] <<= 1;
      set_bit(value, 64, get_bit(*value, 63));
      value->bits[1] <<= 1;
      set_bit(value, 32, get_bit(*value, 31));
      value->bits[0] <<= 1;
    } else {
      flag = 1;
    }
  }
  return flag;
}

int shift_mantissa_left(s21_decimal *value, int shift) {
  int flag = 0;
  for (; shift > 0; shift--) {
    value->bits[0] >>= 1;
    set_bit(value, 31, get_bit(*value, 32));
    value->bits[1] >>= 1;
    set_bit(value, 63, get_bit(*value, 64));
    value->bits[2] >>= 1;
    set_bit(value, 95, 0);
  }
  return flag;
}

void print_decimal(s21_decimal *value) {
  for (int i = 127; i >= 0; i--) {
    printf("%d", get_bit(*value, i));
    if (i % 32 == 0) printf(" ");
  }
  printf("\n");
}

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  normalization(&value_1, &value_2);
  if (get_sign(value_1) == get_sign(value_2)) {
    res = TRUE;
    for (int i = 0; i < 96 && res == TRUE; i++) {
      res = get_bit(value_1, i) != get_bit(value_2, i) ? FALSE : TRUE;
    }
  }
  return res;
}

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  res = s21_is_equal(value_1, value_2) == FALSE ? TRUE : FALSE;
  return res;
}

int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  normalization(&value_1, &value_2);
  res = s21_is_equal(value_1, value_2) == FALSE ? FALSE : TRUE;
  if (res == FALSE)
    res = s21_is_greater(value_1, value_2) == TRUE ? TRUE : FALSE;
  return res;
}

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
  if (get_sign(value_1) == 0 && get_sign(value_2) == 1) res = TRUE;
  return res;
}

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  normalization(&value_1, &value_2);
  if (get_sign(value_1) == get_sign(value_2) && get_sign(value_1) == 1) {
    res = (value1_greater_value2(value_1, value_2) == 1) ? TRUE : FALSE;
  }
  if (get_sign(value_1) == get_sign(value_2) && get_sign(value_1) == 0) {
    res = (value1_greater_value2(value_1, value_2) == 2) ? TRUE : FALSE;
  }
  if (get_sign(value_1) == 0 && get_sign(value_2) == 1) res = FALSE;
  if (get_sign(value_1) == 1 && get_sign(value_2) == 0) res = TRUE;
  print_decimal(&value_1);
  print_decimal(&value_2);
  return res;
}

int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int res = FALSE;
  normalization(&value_1, &value_2);
  res = s21_is_equal(value_1, value_2) == FALSE ? FALSE : TRUE;
  if (res == FALSE) res = s21_is_less(value_1, value_2) == TRUE ? TRUE : FALSE;
  return res;
}

void main() {
  int overflow_flag;
  s21_decimal value_1 = {UINT_MAX, UINT_MAX, 0, 0};
  s21_decimal value_2 = {100, 0, 0, 65536};
  s21_decimal result;
  s21_add(value_1, value_2, &result);
  //  print_decimal(&value_1);
  //  shift_mantissa_right(&value_1, 32);
  // s21_decimal result = div_mantissa(value_1, value_2, &overflow_flag);
  // printf("\n%d\n", overflow_flag);
  print_decimal(&value_1);
  print_decimal(&value_2);
  // int res = s21_is_less_or_equal(value_1, value_2);
  // normalization(&value_1, &value_2);
  //  print_decimal(&value_1);
  //  print_decimal(&value_2);
  print_decimal(&result);
  //    upscale_mantissa(&result);
  //   print_decimal(&result);
  // printf("%d", res);
}
