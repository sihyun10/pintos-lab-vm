/* Verifies that memory mappings at address 0 are disallowed. */
// 주소 0에서의 메모리 매핑이 금지되는지 확인합니다.

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle; // 파일 핸들을 저장할 변수입니다.

  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // NULL 주소 (0)에 mmap을 시도합니다.
  // 이 호출은 MAP_FAILED를 반환해야 합니다.
  CHECK (mmap (NULL, 4096, 0, handle, 0) == MAP_FAILED, "try to mmap at address 0");
}