/* Tries to map a zero-length file, which may or may not work but
   should not terminate the process or crash.
   Then dereferences the address that we tried to map,
   and the process must be terminated with -1 exit code. */
/* 길이가 0인 파일을 매핑하려고 시도합니다. 이는 작동할 수도 있고 안 할 수도 있지만
 * 프로세스를 종료시키거나 충돌을 일으켜서는 안 됩니다.
 * 그런 다음 매핑하려고 시도했던 주소를 역참조하며,
 * 프로세스는 -1 종료 코드로 종료되어야 합니다.*/

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  char *data = (char *) 0x7f000000; // 메모리 매핑을 시도할 타겟 가상 주소입니다.
  int handle;                        // 파일 핸들을 저장할 변수입니다.

  // "empty"라는 이름으로 길이가 0인 파일을 생성합니다. 성공 여부를 CHECK 합니다.
  CHECK (create ("empty", 0), "create empty file \"empty\"");
  // "empty" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("empty")) > 1, "open \"empty\"");

  /* Calling mmap() might succeed or fail.  We don't care. */
  // mmap() 호출은 성공할 수도 있고 실패할 수도 있습니다. 신경 쓰지 않습니다.
  msg ("mmap \"empty\""); // "empty" 파일에 대한 mmap 시도 메시지를 출력합니다.
  // data 주소에 "empty" 파일을 길이 0으로, 읽기 전용(세 번째 인자 0)으로 mmap 시도합니다.
  // 반환 값은 확인하지 않습니다.
  mmap (data, 0, 0, handle, 0);

  /* Regardless of whether the call worked, *data should cause
     the process to be terminated. */
  // 호출이 작동했는지 여부에 관계없이, *data 접근은
  // 프로세스를 종료시켜야 합니다.
  // data 주소의 첫 바이트를 읽으려고 시도합니다. 이 접근은 오류(예: 페이지 폴트 후 종료)를 발생시켜야 합니다.
  // 따라서 fail 메시지가 출력되지 않고 프로세스가 종료되는 것이 이 테스트의 성공 조건입니다.
  fail ("unmapped memory is readable (%d)", *data);
}