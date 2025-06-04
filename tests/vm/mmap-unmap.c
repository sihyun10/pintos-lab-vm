/* Maps and unmaps a file and verifies that the mapped region is
   inaccessible afterward. */
/* 파일을 매핑하고 언매핑한 후, 매핑되었던 영역이
 * 이후에 접근 불가능한지 확인합니다.*/

#include <syscall.h>
#include "tests/vm/sample.inc" // sample 데이터는 직접 사용되지 않으나, 파일 시스템 테스트를 위한 파일 생성 시 필요할 수 있음.
#include "tests/lib.h"
#include "tests/main.h"

#define ACTUAL ((void *) 0x10000000) // 메모리 매핑을 위한 타겟 가상 주소입니다.


void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle;  // 파일 핸들을 저장할 변수입니다.
  void *map;   // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.

  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // ACTUAL 주소에 "sample.txt" 파일을 0x2000 (8KB) 바이트 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
  // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
  CHECK ((map = mmap (ACTUAL, 0x2000, 0, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");
  // 매핑된 메모리(ACTUAL)의 첫 부분을 읽을 수 있는지 확인하는 메시지 (실제 접근 시도).
  msg ("memory is readable %d", *(int *) ACTUAL);
  // 매핑된 메모리(ACTUAL + 0x1000)의 다른 부분을 읽을 수 있는지 확인하는 메시지 (실제 접근 시도).
  msg ("memory is readable %d", *(int *) (ACTUAL + 0x1000)); // 여기서 (ACTUAL + 0x1000)는 (char*)ACTUAL + 0x1000을 의미해야 할 수 있음.

  munmap (map); // 메모리 매핑을 해제합니다.

  // munmap 이후에는 해당 메모리 영역에 접근할 수 없어야 합니다.
  // 아래 fail 호출 내에서 ACTUAL + 0x1000 주소에 접근을 시도하며, 이 접근은 오류(예: 페이지 폴트 후 종료)를 발생시켜야 합니다.
  // 따라서 fail 메시지가 출력되지 않고 프로세스가 종료되는 것이 이 테스트의 성공 조건입니다.
  fail ("unmapped memory is readable (%d)", *(int *) (ACTUAL + 0x1000));
  // 위와 마찬가지로 ACTUAL 주소에 접근을 시도합니다. 이 역시 오류를 발생시켜야 합니다.
  fail ("unmapped memory is readable (%d)", *(int *) (ACTUAL));
}