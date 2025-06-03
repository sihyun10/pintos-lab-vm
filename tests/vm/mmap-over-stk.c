/* Verifies that mapping over the stack segment is disallowed. */
// 프로그램의 스택 세그먼트(함수 호출 시 지역 변수, 반환 주소 등이 저장되는 메모리 영역)
// 위로 메모리 매핑(mmap)을 시도하는 것이 금지되어 실패하는지 확인합니다.

#include <stdint.h>
#include <round.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle; // 파일 핸들을 저장할 변수 (스택에 위치).
  // 지역 변수 handle의 주소를 가져와 페이지 경계로 내림(ROUND_DOWN)합니다.
  // 이것이 스택 세그먼트 내의 한 페이지 주소가 됩니다.
  uintptr_t handle_page = ROUND_DOWN ((uintptr_t) &handle, 4096);

  // "sample.txt" 파일을 엽니다. (이 handle 변수는 위에서 주소 계산에 사용된 handle과 다름)
  // 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // 스택 세그먼트 내의 페이지(handle_page)에 "sample.txt" 파일을 4096 바이트 크기로 mmap 하려고 시도합니다.
  // 이 호출은 MAP_FAILED를 반환해야 합니다. (스택 영역 덮어쓰기 시도)
  CHECK (mmap ((void *) handle_page, 4096, 0, handle, 0) == MAP_FAILED,
         "try to mmap over stack segment");
}