/* Tries to mmap a zero length. mmap should fail if legnth is 0.
 * In Linux kernels before 2.6.12, mmap succeeded even with length0.
 * In this case: no mapping was created and the call returned addr.
 * Since kernel 2.6.12, mapping zero length fails. We expect
 * mmap in Pintos to fail (i.e. return MAP_FAILED) when length is 0. */
/* 길이가 0인 mmap을 시도합니다. 길이가 0이면 mmap은 실패해야 합니다.
 * 2.6.12 이전 리눅스 커널에서는 길이가 0이어도 mmap이 성공했습니다.
 * 이 경우: 매핑이 생성되지 않았고 호출은 주소(addr)를 반환했습니다.
 * 커널 2.6.12부터는 길이가 0인 매핑은 실패합니다. Pintos의 mmap은
 * 길이가 0일 때 실패할 것(즉, MAP_FAILED를 반환)으로 예상합니다.*/

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc" // sample 데이터는 create 호출에 사용됩니다.
#include "tests/lib.h"
#include "tests/main.h"

#define ACTUAL ((void *) 0x10000000) // 메모리 매핑을 위한 타겟 가상 주소입니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle;  // 파일 핸들을 저장할 변수입니다.
  void *map;   // mmap 호출의 반환 값을 저장할 포인터입니다.

  /* Write file via mmap. */ // 주석은 일반적인 mmap 시나리오를 설명하지만, 여기서는 길이 0 테스트가 주 목적입니다.
  // "sample.txt"라는 이름으로 sample 데이터 길이만큼의 파일을 생성합니다. 성공 여부를 CHECK 합니다.
  CHECK (create ("sample.txt", strlen (sample)), "create \"sample.txt\"");
  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // ACTUAL 주소에 "sample.txt" 파일을 길이 0으로, 읽기 전용(세 번째 인자 0)으로 mmap 시도합니다.
  // 이 호출은 MAP_FAILED를 반환해야 합니다. 성공 여부를 CHECK 합니다.
  CHECK ((map = mmap (ACTUAL, 0, 0, handle, 0)) == MAP_FAILED,
			"try to mmap zero length");
}