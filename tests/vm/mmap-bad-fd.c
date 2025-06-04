/* Tries to mmap an invalid fd,
which must either fail silently or terminate the process with
   exit code -1. */
// 유효하지 않은 fd로 mmap을 시도합니다.
// 이는 조용히 실패하거나 프로세스를 -1 종료 코드로
// 종료해야 합니다.

#include <syscall.h>    // Pintos 시스템 콜 (mmap, CHECK, MAP_FAILED 등)을 사용하기 위해 포함합니다.
#include "tests/lib.h"  // Pintos 테스트 라이브러리를 포함합니다.
#include "tests/main.h" // 테스트 프레임워크의 메인 부분을 포함합니다.

void
test_main (void)        // 테스트의 메인 함수입니다.
{
    // 가상 주소 0x10000000에 4096 바이트 크기로 유효하지 않은 fd (0x5678)를 mmap 시도합니다.
    // 이 호출은 MAP_FAILED를 반환해야 합니다. 성공 여부를 CHECK 합니다.
    CHECK (mmap ((void *) 0x10000000, 4096, 0, 0x5678, 0) == MAP_FAILED,
           "try to mmap invalid fd");
}