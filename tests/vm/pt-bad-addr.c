/* Accesses a bad address.
   The process must be terminated with -1 exit code. */
/* 유효하지 않은 메모리 주소(0x04000000)에 접근(읽기)을 시도합니다.
 * 이 접근 시도는 실패해야 하며, 프로세스는 -1 종료 코드로 종료되어야 합니다. */

#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  // 0x04000000 주소에서 정수 값을 읽으려고 시도합니다.
  // 이 주소는 일반적으로 유효하지 않은(매핑되지 않은) 주소이므로, 이 접근은 페이지 폴트를 유발하고
  // 커널에 의해 프로세스가 비정상 종료(-1)되어야 합니다.
  // 따라서 fail 메시지가 실제로 출력되지 않고 프로세스가 종료되는 것이 성공입니다.
  fail ("bad addr read as %d", *(int *) 0x04000000);
}