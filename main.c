#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utest.h"

// exam 2
// 2.1
/**
 * @author: wujie
 * @param: int32_t arr[], the source array
 * @param: size_t length, the length of source array
 * @return: Nothing
 */
void reverse_array(int32_t arr[], size_t length);

// 2.2
/**
 * @author: wujie
 * @param: const char* src, source string pointer
 * @param: char* dst, destination string pointer, the first `sep` character
 * separated word
 * @param: const char sep, the separated character, such as " ", ",", "\"
 * @param: char* next, the next address after the dst string
 * @return: size_t, the size of the first `sep` character separated word
 */
size_t strsplit(const char *src, char *dst, const char sep, char *next);

/**
 * @author: wujie
 * @brief: swap two value that have same type
 */
#define Swap(lhs, rhs)                                                         \
  do {                                                                         \
    (void)(&(lhs) == &(rhs));                                                  \
    size_t _size = sizeof(lhs);                                                \
    if (_size > 1024) {                                                        \
      void *_temp = malloc(_size);                                             \
      memcpy(_temp, &(lhs), _size);                                            \
      memcpy(&(lhs), &(rhs), _size);                                           \
      memcpy(&(rhs), _temp, _size);                                            \
      free(_temp);                                                             \
    } else {                                                                   \
      char _temp[_size];                                                       \
      memcpy(_temp, &(lhs), _size);                                            \
      memcpy(&(lhs), &rhs, _size);                                             \
      memcpy(&(rhs), _temp, _size);                                            \
    }                                                                          \
  } while (false); // `false` required standard ISO C23

// 2.1 implementation

void reverse_array(int32_t arr[], size_t length) {
  if (length < 2)
    return;

  size_t left = 0;
  size_t right = length - 1;
  while (left < right) {
    Swap(arr[left++], arr[right--]);
  }
}

// 2.2 implementation

size_t strsplit(const char *src, char *dst, const char sep, char *next_ptr) {
  return 0;
}

bool check_arr(int32_t arr1[], int32_t arr2[], size_t length1, size_t length2) {
  if (length1 != length2) {
    printf("the arrays length are not equal");
    return false;
  }

  for (size_t i = 0; i < length1; ++i, length2--) {
    if (arr1[i] != arr2[length2 - 1]) {
      printf("the arrays value are not equal: expect: %d, got: %d", arr1[i],
             arr2[length2 - 1]);
      return false;
    }
  }
  return true;
}

TEST(reverse_array) {
  size_t bytes = sizeof(int32_t);
  // ==== test case 1 ====;
  int32_t arr1[5] = {1, 2, 3, 4, 5};
  int32_t ans1[5] = {5, 4, 3, 2, 1};
  EXPECT_TRUE(
      check_arr(arr1, ans1, sizeof(arr1) / bytes, sizeof(ans1) / bytes));
  // ==== test case 2 ====;
  int32_t arr2[] = {1};
  EXPECT_TRUE(check_arr(arr2, arr2, sizeof(arr2) / bytes, 1));
  // ==== test case 2 ====;
  int32_t arr3[] = {1, 2, 3};
  int32_t ans3[] = {3, 2, 1};
  EXPECT_TRUE(
      check_arr(arr3, ans3, sizeof(arr3) / bytes, sizeof(ans3) / bytes));
  // ==== test case 2 ====;
  int32_t arr4[] = {};
  EXPECT_TRUE(check_arr(arr4, arr4, sizeof(arr4) / bytes, 0));
  return 0;
}

int main(int argc, char *argv[]) {
  RUN_TESTS();
  return EXIT_SUCCESS;
}
