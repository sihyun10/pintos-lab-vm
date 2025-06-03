/* Read from an address 4,096 bytes below the stack pointer.
   The process must be terminated with -1 exit code. */
/* 현재 스택 포인터(rsp)보다 4096바이트 아래 주소에서 데이터를 읽으려고 시도합니다.
 * 스택은 일반적으로 아래로(낮은 주소로) 자라지만, 한 번에 너무 큰 간격으로 접근하거나
 * 아직 할당되지 않은 영역에 접근하면 실패해야 합니다.
 * 이 경우, 프로세스는 -1 종료 코드로 종료되어야 합니다. */

#include <string.h>
#include "tests/arc4.h"  // 이 파일에서 직접 사용되지 않으나, 유사한 테스트 파일들과의 일관성을 위해 포함될 수 있음.
#include "tests/cksum.h" // 이 파일에서 직접 사용되지 않으나, 유사한 테스트 파일들과의 일관성을 위해 포함될 수 있음.
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  // 인라인 어셈블리를 사용하여 현재 스택 포인터(rsp)에서 4096을 뺀 주소의 내용을 rax 레지스터로 이동(읽기)하려고 시도합니다.
  // 이 주소는 일반적으로 아직 할당되지 않은 스택 영역이거나 접근 권한이 없는 영역일 수 있으므로,
  // 이 접근은 페이지 폴트를 유발하고 커널에 의해 프로세스가 비정상 종료(-1)되어야 합니다.
  asm volatile ("movq -4096(%rsp), %rax");
}