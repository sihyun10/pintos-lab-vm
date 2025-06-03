/* qsort_bytes라는 바이트 배열을 위한 퀵 정렬(quick-sort) 알고리즘 구현을 제공합니다.
 * 실제 테스트 케이스라기보다는 다른 테스트(child-qsort.c, child-qsort-mm.c 등)에서
 * 사용되는 유틸리티 함수들의 모음입니다. 주요 기능은 피벗 선택, 파티셔닝, 정렬 여부 확인, 바이트 스왑 등입니다. */

#include "tests/vm/qsort.h" // 이 파일에서 제공하는 함수의 선언을 포함합니다 (qsort_bytes).
#include <stdbool.h>
#include <debug.h>
#include <random.h>

/* Picks a pivot for the quicksort from the SIZE bytes in BUF. */
// BUF 내의 SIZE 바이트 중에서 퀵정렬을 위한 피벗을 선택합니다.
static unsigned char
pick_pivot (unsigned char *buf, size_t size) // 피벗 선택 함수입니다.
{
  ASSERT (size >= 1); // 배열 크기가 최소 1 이상이어야 함을 단언합니다.
  // 랜덤하게 선택된 인덱스의 바이트 값을 피벗으로 반환합니다.
  return buf[random_ulong () % size];
}

/* Checks whether the SIZE bytes in ARRAY are divided into an
   initial LEFT_SIZE elements all less than PIVOT followed by
   SIZE - LEFT_SIZE elements all greater than or equal to
   PIVOT. */
// ARRAY 내의 SIZE 바이트가 PIVOT보다 작은 초기 LEFT_SIZE 개의 요소들과
// 그 뒤에 PIVOT보다 크거나 같은 SIZE - LEFT_SIZE 개의 요소들로
// 나누어져 있는지 확인합니다.
static bool
is_partitioned (const unsigned char *array, size_t size,
                unsigned char pivot, size_t left_size) // 파티션 검증 함수입니다.
{
  size_t i; // 반복문 인덱스입니다.

  // 왼쪽 부분(0 ~ left_size-1)의 모든 요소가 pivot보다 작은지 확인합니다.
  for (i = 0; i < left_size; i++)
    if (array[i] >= pivot) // pivot보다 크거나 같은 요소가 발견되면 false를 반환합니다.
      return false;

  // 오른쪽 부분(left_size ~ size-1)의 모든 요소가 pivot보다 크거나 같은지 확인합니다.
  for (; i < size; i++)
    if (array[i] < pivot) // pivot보다 작은 요소가 발견되면 false를 반환합니다.
      return false;

  return true; // 모든 조건 만족 시 true를 반환합니다.
}

/* Swaps the bytes at *A and *B. */
// *A와 *B 위치의 바이트를 교환합니다.
static void
swap (unsigned char *a, unsigned char *b) // 두 바이트 값을 교환하는 함수입니다.
{
  unsigned char t = *a; // 임시 변수 t에 a의 값을 저장합니다.
  *a = *b;              // a에 b의 값을 저장합니다.
  *b = t;               // b에 원래 a의 값(t)을 저장합니다.
}

/* Partitions ARRAY in-place in an initial run of bytes all less
   than PIVOT, followed by a run of bytes all greater than or
   equal to PIVOT.  Returns the length of the initial run. */
// ARRAY를 PIVOT보다 작은 바이트들의 초기 실행 부분과
// 그 뒤에 PIVOT보다 크거나 같은 바이트들의 실행 부분으로 제자리에서 분할합니다.
// 초기 실행 부분의 길이를 반환합니다.
static size_t
partition (unsigned char *array, size_t size, int pivot) // 배열 파티셔닝 함수입니다.
{
  size_t left_size = size; // 왼쪽 부분의 크기를 일단 전체 크기로 초기화합니다. (실제로는 pivot보다 작은 요소들의 개수)
  unsigned char *first = array;          // 배열의 시작을 가리키는 포인터입니다.
  unsigned char *last = first + left_size; // 배열의 끝 다음을 가리키는 포인터입니다. (실제로는 오른쪽 부분의 시작)

  for (;;) // 무한 루프 (탈출 조건은 내부에서 처리)
    {
      /* Move FIRST forward to point to first element greater than
         PIVOT. */
      // FIRST를 PIVOT보다 큰 첫 번째 요소를 가리키도록 앞으로 이동시킵니다.
      for (;;) // 내부 루프
        {
          if (first == last) // first와 last가 만나면 (모든 요소 검사 완료)
            {
              ASSERT (is_partitioned (array, size, pivot, left_size)); // 파티션이 올바르게 되었는지 단언합니다.
              return left_size; // 왼쪽 부분의 크기를 반환합니다.
            }
          else if (*first >= pivot) // first가 가리키는 값이 pivot보다 크거나 같으면
            break;                  // 내부 루프를 탈출합니다.

          first++; // first 포인터를 다음 요소로 이동합니다.
        }
      left_size--; // pivot보다 크거나 같은 요소를 찾았으므로, left_size를 하나 줄입니다. (해당 요소는 왼쪽 부분에 속하지 않음)

      /* Move LAST backward to point to last element no bigger
         than PIVOT. */
      // LAST를 PIVOT보다 크지 않은(작거나 같은) 마지막 요소를 가리키도록 뒤로 이동시킵니다.
      for (;;) // 내부 루프
        {
          last--; // last 포인터를 이전 요소로 이동합니다.

          if (first == last) // first와 last가 만나면
            {
              ASSERT (is_partitioned (array, size, pivot, left_size)); // 파티션이 올바르게 되었는지 단언합니다.
              return left_size; // 왼쪽 부분의 크기를 반환합니다.
            }
          else if (*last < pivot) // last가 가리키는 값이 pivot보다 작으면
            break;                // 내부 루프를 탈출합니다.
          else // last가 가리키는 값이 pivot보다 크거나 같으면 (오른쪽 부분에 속함)
            left_size--; // left_size를 하나 줄입니다. (해당 요소는 왼쪽 부분에 속하지 않음)
        }

      /* By swapping FIRST and LAST we extend the starting and
         ending sequences that pass and fail, respectively,
         PREDICATE. */
      // FIRST와 LAST를 교환함으로써, 각각 PREDICATE를 통과하고 실패하는
      // 시작 및 종료 시퀀스를 확장합니다. (즉, pivot보다 작은 값을 왼쪽으로, 큰 값을 오른쪽으로 보냄)
      swap (first, last); // first와 last가 가리키는 값을 교환합니다.
      first++;            // first 포인터를 다음 요소로 이동합니다.
    }
}

/* Returns true if the SIZE bytes in BUF are in nondecreasing
   order, false otherwise. */
// BUF 내의 SIZE 바이트가 비감소 순서(오름차순)이면 true를, 그렇지 않으면 false를 반환합니다.
static bool
is_sorted (const unsigned char *buf, size_t size) // 정렬 여부 확인 함수입니다.
{
  size_t i; // 반복문 인덱스입니다.

  // 배열의 두 번째 요소부터 마지막 요소까지 순회합니다.
  for (i = 1; i < size; i++)
    if (buf[i - 1] > buf[i]) // 이전 요소가 현재 요소보다 크면 (정렬되지 않음)
      return false;          // false를 반환합니다.

  return true; // 모든 요소가 정렬되어 있으면 true를 반환합니다.
}

/* Sorts the SIZE bytes in BUF into nondecreasing order, using
   the quick-sort algorithm. */
// 퀵정렬 알고리즘을 사용하여 BUF 내의 SIZE 바이트를 비감소 순서로 정렬합니다.
void
qsort_bytes (unsigned char *buf, size_t size) // 바이트 배열 퀵정렬 함수입니다.
{
  if (!is_sorted (buf, size)) // 배열이 이미 정렬되어 있지 않다면 (정렬할 필요가 있다면)
    {
      int pivot = pick_pivot (buf, size); // 피벗을 선택합니다.

      unsigned char *left_half = buf;                // 왼쪽 부분 배열의 시작 포인터입니다.
      size_t left_size = partition (buf, size, pivot); // 배열을 피벗 기준으로 파티셔닝하고 왼쪽 부분의 크기를 얻습니다.
      unsigned char *right_half = left_half + left_size; // 오른쪽 부분 배열의 시작 포인터입니다.
      size_t right_size = size - left_size;            // 오른쪽 부분의 크기입니다.

      // 더 작은 부분 배열을 먼저 재귀적으로 정렬하여 스택 깊이를 줄이려는 시도입니다.
      if (left_size <= right_size)
        {
          qsort_bytes (left_half, left_size);   // 왼쪽 부분을 재귀적으로 정렬합니다.
          qsort_bytes (right_half, right_size); // 오른쪽 부분을 재귀적으로 정렬합니다.
        }
      else
        {
          qsort_bytes (right_half, right_size); // 오른쪽 부분을 재귀적으로 정렬합니다.
          qsort_bytes (left_half, left_size);   // 왼쪽 부분을 재귀적으로 정렬합니다.
        }
    }
}