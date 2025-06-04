/* Verifies that mmap'd regions are only written back on munmap
   if the data was actually modified in memory. */
// 메모리 맵된 영역의 데이터가 실제로 메모리에서 수정된 경우에만
// munmap 시 파일에 다시 쓰여지는지 확인합니다.

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  // 파일에 덮어쓸 새로운 데이터입니다.
  static const char overwrite[] = "Now is the time for all good...";
  // 파일 내용을 다시 읽어올 버퍼입니다. (sample 데이터의 크기 - 1 만큼)
  static char buffer[sizeof sample - 1];
  char *actual = (char *) 0x54321000; // 메모리 매핑을 위한 타겟 가상 주소입니다.
  int handle;                         // 파일 핸들을 저장할 변수입니다.
  void *map;                          // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.

  /* Open file, map, verify data. */
  // 파일을 열고, 매핑하고, 데이터를 검증합니다.
  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");

  // actual 주소에 파일을 4096 바이트 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
  // 매핑 실패 여부를 CHECK 합니다.
	CHECK ((map = mmap (actual, 4096, 0, handle, 0)) != MAP_FAILED, "mmap \"sample.txt\"");
  // 매핑된 메모리(actual)의 내용이 원본 sample 데이터와 일치하는지 확인합니다.
  if (memcmp (actual, sample, strlen (sample)))
    fail ("read of mmap'd file reported bad data"); // 일치하지 않으면 테스트 실패입니다.

  /* Modify file. */
  // 파일을 (매핑을 통하지 않고) 직접 수정합니다.
  // overwrite 데이터를 파일에 직접 씁니다. 쓰기 성공 여부와 쓰여진 바이트 수를 CHECK 합니다.
  CHECK (write (handle, overwrite, strlen (overwrite))
         == (int) strlen (overwrite),
         "write \"sample.txt\"");

  /* Close mapping.  Data should not be written back, because we
     didn't modify it via the mapping. */
  // 매핑을 닫습니다. 데이터는 다시 쓰여져서는 안 됩니다. 왜냐하면
  // 매핑을 통해 수정하지 않았기 때문입니다.
  msg ("munmap \"sample.txt\""); // munmap 메시지를 출력합니다.
  munmap (map); // 메모리 매핑을 해제합니다. `actual`은 수정되지 않았으므로, 이 때 파일에 쓰기 작업이 발생하면 안 됩니다.

  /* Read file back. */
  // 파일을 다시 읽습니다.
  msg ("seek \"sample.txt\""); // seek 메시지를 출력합니다.
  seek (handle, 0); // 파일 포인터를 파일의 시작으로 이동합니다.
  // 파일 내용을 buffer로 읽어들입니다. 읽기 성공 여부와 읽은 바이트 수를 CHECK 합니다.
  CHECK (read (handle, buffer, sizeof buffer) == sizeof buffer,
         "read \"sample.txt\"");

  /* Verify that file overwrite worked. */
  // 파일 덮어쓰기가 제대로 작동했는지 확인합니다.
  // buffer의 내용이 overwrite 데이터와 일치하는지, 그리고 그 이후 부분은 원본 sample 데이터와 일치하는지 확인합니다.
  if (memcmp (buffer, overwrite, strlen (overwrite))
      || memcmp (buffer + strlen (overwrite), sample + strlen (overwrite),
                 strlen (sample) - strlen (overwrite)))
    {
      // 만약 buffer 내용이 원본 sample 데이터와 같다면, munmap이 깨끗한 페이지를 잘못 덮어쓴 것입니다.
      if (!memcmp (buffer, sample, strlen (sample)))
        fail ("munmap wrote back clean page");
      else // 그렇지 않다면, 파일에서 예상치 못한 데이터를 읽은 것입니다.
        fail ("read surprising data from file");
    }
  else // 모든 것이 정상이라면, 파일 변경 사항이 munmap 이후에도 유지된 것입니다.
    msg ("file change was retained after munmap");
}