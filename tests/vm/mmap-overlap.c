/* Verifies that overlapping memory mappings are disallowed. */
/* 이미 사용 중인 메모리 영역에 또 다른 메모리 매핑(mmap)을 시도하여,
 * 즉 메모리 매핑이 서로 겹치도록 하려는 시도가 금지되어 실패하는지 확인합니다. */

#include <syscall.h>
#include "tests/vm/sample.inc" // sample 데이터는 직접 사용되지 않으나, 파일 시스템 테스트를 위한 파일 생성 시 필요할 수 있음.
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  char *start = (char *) 0x10000000; // 메모리 매핑을 위한 시작 가상 주소입니다.
  int fd[2];                          // 두 개의 파일 디스크립터를 저장할 배열입니다.

  // "zeros"라는 파일을 엽니다 (이 파일은 0으로 채워진 파일이거나, 테스트 전에 생성되어야 함).
  // 파일 핸들을 fd[0]에 저장하고 성공 여부를 CHECK 합니다.
  CHECK ((fd[0] = open ("zeros")) > 1, "open \"zeros\" once");
  // start 주소에 "zeros" 파일을 4096 바이트 크기로 mmap 합니다.
  // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
  CHECK (mmap (start, 4096, 0, fd[0], 0) != MAP_FAILED, "mmap \"zeros\"");

  // "zeros" 파일을 다시 엽니다. 이전 fd[0]과 다른 핸들인지, 그리고 열기 성공 여부를 CHECK 합니다.
  CHECK ((fd[1] = open ("zeros")) > 1 && fd[0] != fd[1],
         "open \"zeros\" again");
  // 이전에 매핑된 동일한 start 주소에 "zeros" 파일을 다시 mmap 하려고 시도합니다 (겹치는 매핑).
  // 이 호출은 MAP_FAILED를 반환해야 합니다.
  CHECK (mmap (start, 4096, 0, fd[1], 0) == MAP_FAILED,
         "try to mmap \"zeros\" again");
}