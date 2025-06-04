/* Writes to a file through a mapping, and unmaps the file,
   then reads the data in the file back using the read system
   call to verify. */
// (주석 설명은 실제 테스트 내용과 약간 다름. 실제로는 읽기 전용 매핑에 쓰기를 시도함)
/* 파일을 읽기 전용(read-only)으로 메모리 매핑한 후, 해당 매핑된 메모리 영역에 쓰기를 시도합니다.
 * 이 쓰기 시도는 실패하거나 세그멘테이션 폴트(segmentation fault)와 같은 오류를 발생시켜야 합니다.
 * (주: 이 테스트 코드 자체는 쓰기 시도 후 프로그램이 비정상 종료될 것을 예상하는 방식으로 작성되어 있으며,
 * msg ("Error should have occured"); 이후 코드가 실행되지 않아야 성공입니다.) */

#include <string.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"

#define ACTUAL ((void *) 0x10000000) // 메모리 매핑을 위한 타겟 가상 주소입니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle;  // 파일 핸들을 저장할 변수입니다.
  void *map;   // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.
  // char buf[1024]; // 이 변수는 실제 코드에서 사용되지 않습니다.

  /* Write file via mmap. */
  // mmap을 통해 파일에 쓰기를 시도합니다. (실제로는 읽기 전용 매핑)
  // "large.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");
  // ACTUAL 주소에 "large.txt" 파일을 4096 바이트 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
  // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
  CHECK ((map = mmap (ACTUAL, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"large.txt\" with writable=0");
  msg ("about to write into read-only mmap'd memory"); // 읽기 전용으로 매핑된 메모리에 쓰기를 시도할 것이라는 메시지를 출력합니다.
  *((int *)map) = 0; // 읽기 전용으로 매핑된 메모리의 시작 부분에 정수 0을 쓰려고 시도합니다. 여기서 오류(예: 페이지 폴트 핸들러에 의한 종료)가 발생해야 합니다.
  msg ("Error should have occured"); // 이 메시지가 출력된다면, 읽기 전용 매핑에 쓰기가 허용된 것이므로 테스트 실패입니다.
  munmap (map);   // 오류로 인해 이 코드가 실행되지 않아야 합니다.
  close (handle); // 오류로 인해 이 코드가 실행되지 않아야 합니다.
}