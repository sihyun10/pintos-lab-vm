/* Verifies that mapping over the data segment is disallowed. */
// 프로그램의 데이터 세그먼트(초기화된 전역 변수나 정적 변수들이 있는 메모리 영역)
// 위로 메모리 매핑(mmap)을 시도하는 것이 금지되어 실패하는지 확인합니다.

#include <stdint.h>
#include <round.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

static char x; // 데이터 세그먼트에 위치할 정적 변수입니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
  // 정적 변수 x의 주소를 가져와 페이지 경계로 내림(ROUND_DOWN)합니다.
  // 이것이 데이터 세그먼트 내의 한 페이지 주소가 됩니다.
  uintptr_t x_page = ROUND_DOWN ((uintptr_t) &x, 4096);
  int handle; // 파일 핸들을 저장할 변수입니다.

  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // 데이터 세그먼트 내의 페이지(x_page)에 "sample.txt" 파일을 4096 바이트 크기로 mmap 하려고 시도합니다.
  // 이 호출은 MAP_FAILED를 반환해야 합니다. (데이터 영역 덮어쓰기 시도)
  CHECK (mmap ((void *) x_page, 4096, 0, handle, 0) == MAP_FAILED,
         "try to mmap over data segment");
}