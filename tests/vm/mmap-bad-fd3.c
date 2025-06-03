/* Tries to mmap with fd 1,
   which is the file descriptor for console output.
   mmap must fail silently or terminate the process with
   exit code -1. */
// fd 1 (콘솔 출력의 파일 디스크립터)으로 mmap을 시도합니다.
// mmap은 조용히 실패하거나 프로세스를 -1 종료 코드로
// 종료해야 합니다.

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
    // 가상 주소 0x10000000에 4096 바이트 크기로 fd 1 (stdout)을 읽기 전용(세 번째 인자 0)으로 mmap 시도합니다.
    // 이 호출은 MAP_FAILED를 반환해야 합니다. 성공 여부를 CHECK 합니다.
    CHECK (mmap ((void *) 0x10000000, 4096, 0, 1, 0) == MAP_FAILED,
           "try to mmap stdout");
}