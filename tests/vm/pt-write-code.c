/* Try to write to the code segment.
   The process must be terminated with -1 exit code. */
/* 프로그램의 코드 세그먼트(실행 가능한 명령어들이 있는 메모리 영역)에 직접 쓰기를 시도합니다.
 * 이 작업은 실패해야 하며, 프로세스는 -1 종료 코드로 종료되어야 합니다. */
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  // 현재 실행 중인 test_main 함수의 시작 주소(코드 세그먼트)에 정수 0을 쓰려고 시도합니다.
  // 이 작업은 메모리 보호 위반으로 인해 페이지 폴트를 유발하고,
  // 커널에 의해 프로세스가 비정상 종료(-1)되어야 합니다.
  *(int *) test_main = 0;
  // 만약 위의 쓰기 작업 이후에도 프로그램이 계속 실행된다면, 테스트는 실패한 것입니다.
  fail ("writing the code segment succeeded");
}