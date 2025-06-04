/*  parallel_merge라는 유틸리티 함수를 호출하여 병렬 정렬 및 병합 테스트를 수행합니다.
 *  child-sort (계수 정렬하는 자식 프로세스)를 자식 프로세스로 사용하며,
 *  자식 프로세스의 예상 종료 코드는 123입니다. */
#include "tests/main.h"
#include "tests/vm/parallel-merge.h" // parallel_merge 함수를 포함합니다.

void
test_main (void)
{
  parallel_merge ("child-sort", 123);
}