/* Tries to mmap an invalid offset,
   which must either fail silently or terminate the process with
   exit code -1. */
// 유효하지 않은 오프셋으로 mmap을 시도합니다.
// 이는 조용히 실패하거나 프로세스를 -1 종료 코드로
// 종료해야 합니다.

#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
    int handle; // 파일 핸들을 저장할 변수입니다.
    // "large.txt" 파일을 엽니다. 파일 핸들이 1보다 큰지 (유효한지) CHECK 합니다.
    CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");

    // 가상 주소 0x10000000에 4096 바이트 크기로, 파일 핸들 handle에 대해 오프셋 0x1234 (페이지 정렬 안됨)로 mmap을 시도합니다.
    // 이 호출은 MAP_FAILED를 반환해야 합니다. 성공 여부를 CHECK 합니다.
    CHECK (mmap ((void *) 0x10000000, 4096, 0, handle, 0x1234) == MAP_FAILED,
           "try to mmap invalid offset");
}