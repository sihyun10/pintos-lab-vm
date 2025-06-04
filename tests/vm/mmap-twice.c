/* Maps the same file into memory twice and verifies that the
   same data is readable in both. */
/* 동일한 파일을 메모리에 두 번 매핑하고
 * 두 곳 모두에서 동일한 데이터를 읽을 수 있는지 확인합니다. */

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc" // 'sample' 배열 (테스트용 샘플 데이터)을 포함합니다.
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  // 두 개의 메모리 매핑 영역을 위한 타겟 가상 주소 배열입니다.
  char *actual[2] = {(char *) 0x10000000, (char *) 0x20000000};
  size_t i;        // 반복문에서 사용할 인덱스 변수입니다.
  int handle[2];   // 두 개의 파일 디스크립터를 저장할 배열입니다.

  // 2번 반복하여 파일을 열고 매핑합니다.
  for (i = 0; i < 2; i++)
    {
      // "sample.txt" 파일을 엽니다. 파일 핸들을 handle[i]에 저장하고 성공 여부를 CHECK 합니다.
      CHECK ((handle[i] = open ("sample.txt")) > 1,
             "open \"sample.txt\" #%zu", i);
      // actual[i] 주소에 "sample.txt" 파일을 4096 바이트 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
      // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
      CHECK (mmap (actual[i], 4096, 0, handle[i], 0) != MAP_FAILED,
             "mmap \"sample.txt\" #%zu at %p", i, (void *) actual[i]);
    }

  // 2개의 매핑된 영역을 순회하며 내용을 확인합니다.
  for (i = 0; i < 2; i++)
    // 각 매핑된 메모리(actual[i])의 내용이 원본 sample 데이터와 일치하는지 확인합니다.
    CHECK (!memcmp (actual[i], sample, strlen (sample)),
           "compare mmap'd file %zu against data", i);
  // munmap이나 close는 이 테스트에서 명시적으로 호출되지 않을 수 있습니다 (프로세스 종료 시 정리).
}