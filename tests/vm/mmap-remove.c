/* Deletes and closes file that is mapped into memory
   and verifies that it can still be read through the mapping. */
/* 파일을 메모리에 매핑한 후, 해당 파일을 닫고 시스템에서 삭제합니다.
 * 그 후에도 여전히 매핑된 메모리를 통해 파일의 원본 데이터를 읽을 수 있는지 확인합니다.
 * 이는 파일이 삭제되더라도, 해당 파일 내용을 참조하는 활성 메모리 매핑이 있다면
 * 그 내용은 계속 유효해야 함을 테스트합니다 */

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc" // 'sample' 배열 (테스트용 샘플 데이터)을 포함합니다.
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  char *actual = (char *) 0x10000000; // 메모리 매핑을 위한 타겟 가상 주소입니다.
  int handle;                         // 파일 핸들을 저장할 변수입니다.
  void *map;                          // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.
  size_t i;                           // 반복문에서 사용할 인덱스 변수입니다.

  /* Map file. */
  // 파일을 매핑합니다.
  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // actual 주소에 "sample.txt" 파일을 4096 바이트 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
  // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
  CHECK ((map = mmap (actual, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");

  /* Close file and delete it. */
  // 파일을 닫고 삭제합니다.
  close (handle); // 파일 핸들을 닫습니다.
  CHECK (remove ("sample.txt"), "remove \"sample.txt\""); // "sample.txt" 파일을 삭제합니다. 성공 여부를 CHECK 합니다.
  // 삭제된 "sample.txt" 파일을 다시 열려고 시도합니다. open 호출이 -1을 반환하는지(실패하는지) CHECK 합니다.
  CHECK (open ("sample.txt") == -1, "try to open \"sample.txt\"");

  /* Create a new file in hopes of overwriting data from the old
     one, in case the file system has incorrectly freed the
     file's data. */
  // 파일 시스템이 이전 파일의 데이터를 잘못 해제했을 경우를 대비하여,
  // 이전 파일의 데이터를 덮어쓸 목적으로 새 파일을 생성합니다.
  CHECK (create ("another", 4096 * 10), "create \"another\""); // "another"라는 이름으로 새 파일을 생성합니다.

  /* Check that mapped data is correct. */
  // 매핑된 데이터가 올바른지 확인합니다.
  // (파일이 삭제되었음에도 불구하고) 매핑된 메모리(actual)의 내용이 원본 sample 데이터와 일치하는지 확인합니다.
  if (memcmp (actual, sample, strlen (sample)))
    fail ("read of mmap'd file reported bad data"); // 일치하지 않으면 테스트 실패를 알립니다.

  /* Verify that data is followed by zeros. */
  // 데이터 뒤에 0이 오는지 확인합니다.
  // sample 데이터 길이 이후부터 페이지 끝(4096 바이트)까지 순회합니다.
  for (i = strlen (sample); i < 4096; i++)
    if (actual[i] != 0) // 해당 바이트가 0이 아니면,
      // 오류 메시지를 출력하고 테스트 실패를 알립니다.
      fail ("byte %zu of mmap'd region has value %02hhx (should be 0)",
            i, actual[i]);

  munmap (map); // 메모리 매핑을 해제합니다.
}