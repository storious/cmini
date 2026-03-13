#ifndef UTEST_H
#define UTEST_H
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==========================================
// 1. Unit Test Core
// ==========================================

typedef int (*test_func_t)(void);

typedef struct test_case {
  const char *name;
  test_func_t func;
  struct test_case *next;
} test_case_t;

static test_case_t *test_list_head = nullptr;

static inline void register_test(test_case_t *tc) {
  tc->next = test_list_head;
  test_list_head = tc;
}

// ==========================================
// 2. Auto Register
// ==========================================

#define TEST(test_case_name)                                                   \
  int test_func_##test_case_name(void);                                        \
  __attribute__((constructor)) static void register_func_##test_case_name(     \
      void) {                                                                  \
    static test_case_t tc = {.name = #test_case_name,                          \
                             .func = test_func_##test_case_name};              \
    register_test(&tc);                                                        \
  }                                                                            \
  int test_func_##test_case_name(void)

// ==========================================
// 3. Virtual Memory Allocator
// ==========================================

typedef struct mem_record {
  void *ptr;
  size_t size;
  const char *file;
  int line;
  bool freed;
  struct mem_record *next;
} mem_record_t;

static mem_record_t *mem_list_head = nullptr;
static int mem_fail_counter = 0;

static inline void *_ut_malloc_internal(size_t size, const char *file,
                                        int line) {
  if (mem_fail_counter > 0) {
    mem_fail_counter--;
    return nullptr;
  }

  mem_record_t *node = (mem_record_t *)malloc(sizeof(mem_record_t));
  void *ptr = malloc(size);

  if (!ptr || !node) {
    free(node);
    free(ptr);
    return nullptr;
  }

  node->ptr = ptr;
  node->size = size;
  node->file = file;
  node->line = line;
  node->freed = false;

  node->next = mem_list_head;
  mem_list_head = node;

  return ptr;
}

static inline void _ut_free_internal(void *ptr, const char *file, int line) {
  if (!ptr)
    return;

  mem_record_t *curr = mem_list_head;
  while (curr) {
    if (curr->ptr == ptr) {
      if (curr->freed) {
        fprintf(stderr, "[ERROR] Double Free detected at %s:%d\n", file, line);
        fprintf(stderr, "[ERROR] Originally allocated at %s:%d\n", curr->file,
                curr->line);
        return;
      }
      curr->freed = true;
      free(ptr);
      return;
    }
    curr = curr->next;
  }
  fprintf(stderr, "[ERROR] Invalid Free (untracked pointer) at %s:%d\n", file,
          line);
}

static inline void _ut_reset_allocator() {
  mem_record_t *curr = mem_list_head;
  while (curr) {
    mem_record_t *next = curr->next;
    if (!curr->freed)
      free(curr->ptr);
    free(curr);
    curr = next;
  }
  mem_list_head = nullptr;
  mem_fail_counter = 0;
}

static inline int _ut_check_leaks() {
  mem_record_t *curr = mem_list_head;
  int leaks = 0;
  while (curr) {
    if (!curr->freed) {
      fprintf(stderr, "[ERROR] Memory Leak: %zu bytes at %p\n", curr->size,
              curr->ptr);
      fprintf(stderr, "        Allocated at %s:%d\n", curr->file, curr->line);
      leaks++;
    }
    curr = curr->next;
  }
  return leaks;
}

#define test_malloc(size) _ut_malloc_internal(size, __FILE__, __LINE__)
#define test_free(ptr) _ut_free_internal(ptr, __FILE__, __LINE__)
#define test_simulate_oom(n) (mem_fail_counter = n)

// ==========================================
// 4. Assert Macros
// ==========================================

#define _UT_FAIL(msg, file, line, ...)                                         \
  do {                                                                         \
    fprintf(stderr, "  [FAIL] %s\n", msg);                                     \
    __VA_OPT__(fprintf(stderr, "        [MSG] %s\n", __VA_ARGS__);)            \
    fprintf(stderr, "        [AT] %s:%d\n", file, line);                       \
  } while (0)

#define EXPECT_TRUE(expr, ...)                                                 \
  do {                                                                         \
    if (!(expr)) {                                                             \
      _UT_FAIL(#expr, __FILE__, __LINE__ __VA_OPT__(, __VA_ARGS__));           \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define EXPECT_EQ(expected, actual, ...)                                       \
  do {                                                                         \
    typeof(expected) _e = (expected);                                          \
    typeof(actual) _a = (actual);                                              \
    if (_e != _a) {                                                            \
      fprintf(stderr, "  [FAIL] %s == %s\n", #expected, #actual);              \
      fprintf(stderr, "        Values: %ld != %ld\n", (long)_e, (long)_a);     \
      __VA_OPT__(fprintf(stderr, "        [MSG] %s\n", __VA_ARGS__);)          \
      fprintf(stderr, "        [AT] %s:%d\n", __FILE__, __LINE__);             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define EXPECT_NEAR(expected, actual, tolerance, ...)                          \
  do {                                                                         \
    double _e = (double)(expected);                                            \
    double _a = (double)(actual);                                              \
    if (fabs(_e - _a) > (tolerance)) {                                         \
      fprintf(stderr, "  [FAIL] |%f - %f| > %f\n", _e, _a,                     \
              (double)(tolerance));                                            \
      __VA_OPT__(fprintf(stderr, "        [MSG] %s\n", __VA_ARGS__);)          \
      fprintf(stderr, "        [AT] %s:%d\n", __FILE__, __LINE__);             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

// Memory Assert
#define EXPECT_NULLPTR(ptr, ...) EXPECT_TRUE((ptr) == nullptr, __VA_ARGS__)
#define EXPECT_NOT_NULLPTR(ptr, ...) EXPECT_TRUE((ptr) != nullptr, __VA_ARGS__)

// ==========================================
// 5. Runner
// ==========================================

#define RUN_TESTS()                                                            \
  do {                                                                         \
    int passed = 0, failed = 0;                                                \
    printf("\n[START] Running Tests...\n\n");                                  \
                                                                               \
    test_case_t *current = test_list_head;                                     \
    while (current) {                                                          \
      printf("-> Running: %s ... ", current->name);                            \
                                                                               \
      _ut_reset_allocator();                                                   \
      int result = current->func();                                            \
                                                                               \
      if (result == 0) {                                                       \
        int leaks = _ut_check_leaks();                                         \
        if (leaks > 0) {                                                       \
          printf("[LEAK] Memory Leaked\n");                                    \
          failed++;                                                            \
        } else {                                                               \
          printf("[PASS]\n");                                                  \
          passed++;                                                            \
        }                                                                      \
      } else {                                                                 \
        _ut_check_leaks();                                                     \
        printf("[FAIL]\n");                                                    \
        failed++;                                                              \
      }                                                                        \
                                                                               \
      current = current->next;                                                 \
    }                                                                          \
                                                                               \
    printf("\n-------------------\n");                                         \
    printf("Total: %d | [PASS]: %d | [FAIL]: %d\n", passed + failed, passed,   \
           failed);                                                            \
    return failed > 0 ? 1 : 0;                                                 \
  } while (false);

#endif // UTEST_H
