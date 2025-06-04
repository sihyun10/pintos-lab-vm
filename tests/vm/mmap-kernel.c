/* Verifies that mapping over the kernel is disallowed. */
// 커널 위로 매핑하는 것이 금지되는지 확인합니다.

#include <stdint.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle; // 파일 핸들을 저장할 변수입니다.

  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");

  // 커널 공간으로 예상되는 주소 0x8004000000 에 mmap을 시도합니다.
  // 이 호출은 MAP_FAILED를 반환해야 합니다.
  void *kernel = (void *) 0x8004000000;
  CHECK (mmap (kernel, 4096, 0, handle, 0) == MAP_FAILED,
         "try to mmap over kernel 0");

  // 또 다른 커널 공간으로 예상되는 높은 주소 0xfffffffffffff000 에 mmap을 시도합니다.
  // 이 호출은 MAP_FAILED를 반환해야 합니다.
  kernel = (void *) 0xfffffffffffff000;
  CHECK (mmap (kernel, 0x2000, 0, handle, 0) == MAP_FAILED,
         "try to mmap over kernel 1");

  // 커널 시작 주소 바로 이전부터 시작하여 커널 공간을 침범하는 영역에 mmap을 시도합니다.
  // (주: 세 번째 mmap의 크기 인자가 음수처럼 보이나, size_t로 해석되면 매우 큰 양수가 될 수 있습니다.
  // 의도는 커널 영역을 겹치도록 매핑하는 것입니다.)
  // 이 호출은 MAP_FAILED를 반환해야 합니다.
  kernel = (void *) 0x8004000000 - 0x1000;
  CHECK (mmap (kernel, (size_t)((uintptr_t)-0x8004000000 + (uintptr_t)0x1000), 0, handle, 0) == MAP_FAILED,
         "try to mmap over kernel 2");
}