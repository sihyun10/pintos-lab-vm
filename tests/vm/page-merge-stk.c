/* parallel_merge라는 유틸리티 함수를 호출하여 병렬 정렬 및 병합 테스트를 수행합니다.
 * child-qsort (스택 기반 버퍼를 사용하여 퀵 정렬하는 자식 프로세스)를 자식 프로세스로 사용하며,
 * 자식 프로세스의 예상 종료 코드는 72입니다. */
#include "tests/main.h"
#include "tests/vm/parallel-merge.h"

void
test_main (void)
{
  parallel_merge ("child-qsort", 72);
}