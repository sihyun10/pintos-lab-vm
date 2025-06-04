/* Verifies that memory mappings persist after file close. */
// 파일이 닫힌 후에도 메모리 매핑이 지속되는지 확인합니다.

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/arc4.h" // arc4 관련 기능은 이 파일에서 직접 사용되지 않는 것으로 보입니다.
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
  // ACTUAL 주소에 파일을 4096 바이트 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
  // 매핑 실패 여부를 CHECK 합니다.
  CHECK ((map = mmap(ACTUAL, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");

  close (handle); // 파일 핸들을 닫습니다.

  // 파일 핸들을 닫은 후에도 매핑된 메모리(ACTUAL)의 내용이 원본 sample 데이터와 일치하는지 확인합니다.
  if (memcmp (ACTUAL, sample, strlen (sample)))
    fail ("read of mmap'd file reported bad data"); // 일치하지 않으면 테스트 실패입니다.

  munmap (map); // 메모리 매핑을 해제합니다.
}