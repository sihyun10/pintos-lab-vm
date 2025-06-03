/* Runs 4 child-linear processes at once. */
/* child-linear라는 자식 프로세스 4개를 동시에 실행하고,
 * 각 자식 프로세스가 성공적으로 완료(종료 코드 0x42 반환)될 때까지 기다립니다.
 * 이는 여러 프로세스가 병렬로 메모리 작업을 수행하는 상황을 시뮬레이션합니다. */

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

#define CHILD_CNT 4 // 실행할 자식 프로세스의 수를 4로 정의합니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
  pid_t children[CHILD_CNT]; // 자식 프로세스들의 ID를 저장할 배열입니다.
  int i;                     // 반복문에서 사용할 인덱스 변수입니다.

  // CHILD_CNT 만큼 반복하여 자식 프로세스를 생성하고 실행합니다.
  for (i = 0; i < CHILD_CNT; i++) {
    children[i] = fork ("child-linear"); // "child-linear"라는 이름으로 자식 프로세스를 fork 합니다.
    if (children[i] == 0) { // 자식 프로세스인 경우
      // "child-linear"를 실행합니다. exec 호출이 실패하면 (-1 반환)
      if (exec ("child-linear") == -1)
        fail ("failed to exec child-linear"); // 테스트 실패를 알립니다.
    }
  }
  // 모든 자식 프로세스가 종료될 때까지 기다립니다.
  for (i = 0; i < CHILD_CNT; i++) {
    // i번째 자식 프로세스(children[i])를 기다리고, 반환된 종료 코드가 0x42인지 확인합니다.
    CHECK (wait (children[i]) == 0x42, "wait for child %d", i);
  }
}