/* Writes to a file through a mapping, and unmaps the file,
   then reads the data in the file back using the read system
   call to verify. */
/* 매핑을 통해 파일에 쓰고, 파일을 언매핑한 다음,
 * read 시스템 콜을 사용하여 파일의 데이터를 다시 읽어 확인합니다.*/

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc" // 'sample' 배열 (테스트용 샘플 데이터)을 포함합니다.
#include "tests/lib.h"
#include "tests/main.h"

#define ACTUAL ((void *) 0x10000000) // 메모리 매핑을 위한 타겟 가상 주소입니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle;       // 파일 핸들을 저장할 변수입니다.
  void *map;        // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.
  char buf[1024];   // 파일에서 데이터를 읽어올 버퍼입니다.

  /* Write file via mmap. */
  // mmap을 통해 파일에 씁니다.
  // "sample.txt"라는 이름으로 sample 데이터 길이만큼의 파일을 생성합니다. 성공 여부를 CHECK 합니다.
  CHECK (create ("sample.txt", strlen (sample)), "create \"sample.txt\"");
  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // ACTUAL 주소에 "sample.txt" 파일을 4096 바이트 크기만큼, 쓰기 가능(세 번째 인자 1)으로 메모리 매핑합니다.
  // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
  CHECK ((map = mmap (ACTUAL, 4096, 1, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");
  // sample 데이터를 매핑된 메모리(ACTUAL)에 복사합니다.
  memcpy (ACTUAL, sample, strlen (sample));
  munmap (map); // 메모리 매핑을 해제합니다. 이때 변경된 내용이 파일에 반영되어야 합니다.

  /* Read back via read(). */
  // read()를 통해 다시 읽습니다.
  // 파일 핸들(handle)을 통해 파일에서 sample 데이터 길이만큼을 buf로 읽어들입니다.
  read (handle, buf, strlen (sample));
  // 읽어온 데이터(buf)가 원본 sample 데이터와 일치하는지 확인합니다.
  CHECK (!memcmp (buf, sample, strlen (sample)),
         "compare read data against written data");
  close (handle); // 파일을 닫습니다.
}