/* Tries to mmap with offset > 0. */
/* 파일의 특정 오프셋(0보다 큰 값)에서부터 메모리 매핑(mmap)을 시도하고,
 * 해당 매핑된 메모리 영역을 읽고 쓰는 기능을 테스트합니다.
 * 오프셋으로 매핑된 영역의 데이터가 파일의 해당 부분과 일치하는지,
 * 그리고 매핑된 영역에 쓴 내용이 실제로 파일에 반영되는지 확인합니다. */

#include <syscall.h>
#include <string.h>
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/vm/large.inc" // 'large' 배열 (테스트용 대용량 데이터)을 포함합니다.

static char zeros[0x1000]; // 0x1000 (4096) 바이트 크기의 0으로 채워진 배열을 선언합니다 (bss 영역).

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle;             // 파일 핸들을 저장할 변수입니다.
  char buf[0x1000];       // 파일에서 데이터를 읽어올 4KB 크기의 버퍼입니다.

  // "large.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다. (large.txt는 large 데이터로 미리 만들어져 있어야 함)
  CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");

  // 가상 주소 0x10000000에 "large.txt" 파일의 오프셋 0x1000 (4096번째 바이트)부터 4096 바이트를 쓰기 가능(prot=1)으로 매핑합니다.
  // mmap 호출 결과가 매핑 요청 주소와 같은지 (성공적인지) CHECK 합니다.
  CHECK (mmap ((void *) 0x10000000, 4096, 1, handle, 0x1000) == (void *) 0x10000000,
          "try to mmap with offset 0x1000");
  close (handle); // 파일 핸들을 닫습니다. mmap 이후에는 일반적으로 닫아도 매핑은 유지됩니다.

  msg ("validate mmap."); // mmap 유효성 검사 메시지를 출력합니다.
  // 매핑된 메모리 영역(0x10000000)의 내용이 large 데이터의 0x1000 오프셋부터의 내용과 일치하는지 확인합니다.
  if (!memcmp ((void *) 0x10000000, &large[0x1000], 0x1000))
      msg ("validated."); // 일치하면 유효성 검사 성공 메시지를 출력합니다.
  else
      fail ("validate fail."); // 일치하지 않으면 테스트 실패를 알립니다.

  msg ("write to mmap"); // mmap에 쓰기 작업 메시지를 출력합니다.
  memset (zeros, 0, 0x1000); // zeros 배열을 0으로 채웁니다 (이미 0으로 초기화되어 있지만 명시적으로).
  // 매핑된 메모리 영역(0x10000000)에 zeros 배열의 내용(0으로 채워진 4KB)을 복사합니다.
  memcpy ((void *) 0x10000000, zeros, 0x1000);
  munmap ((void *) 0x10000000); // 메모리 매핑을 해제합니다. 이때 변경된 내용이 파일에 반영되어야 합니다.

  msg ("validate contents."); // 내용 유효성 검사 메시지를 출력합니다.

  // "large.txt" 파일을 다시 엽니다.
  CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");
  // 파일의 첫 번째 페이지(0~0x1000-1)를 읽습니다. 읽은 바이트 수가 0x1000인지 CHECK 합니다.
  CHECK (0x1000 == read (handle, buf, 0x1000), "read \"large.txt\" Page 0");

  msg ("validate page 0."); // 페이지 0 유효성 검사 메시지를 출력합니다.
  // 읽어온 첫 번째 페이지(buf)의 내용이 원본 large 데이터의 첫 페이지 내용과 일치하는지 확인합니다.
  // (이 부분은 mmap 오프셋 쓰기와는 직접 관련 없으나, 파일의 다른 부분이 변경되지 않았는지 확인)
  if (!memcmp ((void *) buf, large, 0x1000))
      msg ("validated."); // 일치하면 유효성 검사 성공 메시지를 출력합니다.
  else
      fail ("validate fail."); // 일치하지 않으면 테스트 실패를 알립니다.

  // 파일의 두 번째 페이지(0x1000~0x2000-1)를 읽습니다. 읽은 바이트 수가 0x1000인지 CHECK 합니다.
  CHECK (0x1000 == read (handle, buf, 0x1000), "read \"large.txt\" Page 1");

  msg ("validate page 1."); // 페이지 1 유효성 검사 메시지를 출력합니다.
  // 읽어온 두 번째 페이지(buf)의 내용이 이전에 mmap을 통해 썼던 zeros 배열의 내용(모두 0)과 일치하는지 확인합니다.
  if (!memcmp ((void *) buf, zeros, 0x1000))
      msg ("validated."); // 일치하면 유효성 검사 성공 메시지를 출력합니다.
  else
      fail ("validate fail."); // 일치하지 않으면 테스트 실패를 알립니다.
  close (handle); // 파일 핸들을 닫습니다.

  msg ("success"); // 모든 검사를 통과하면 성공 메시지를 출력합니다.
}