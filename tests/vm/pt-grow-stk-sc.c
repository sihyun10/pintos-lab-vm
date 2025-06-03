/* This test checks that the stack is properly extended even if
   the first access to a stack location occurs inside a system
   call.

   From Godmar Back. */
/* 스택 위치에 대한 첫 번째 접근이 시스템 콜 내부에서 발생하더라도 스택이 올바르게 확장되는지 확인합니다.
 * 파일을 생성하고 데이터를 쓴 다음, 그 파일을 다시 열어서 스택의 특정 깊은 위치에 있는 버퍼로 파일 내용을 읽어들입니다.
 * 이 read 시스템 콜 시점에 해당 스택 영역이 유효하게 확장되어야 합니다.*/

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc" // 'sample' 배열 (테스트용 샘플 데이터)을 포함합니다.
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle;              // 파일 핸들을 저장할 변수입니다.
  int slen = strlen (sample); // sample 데이터의 길이를 저장합니다.
  // 스택에 64KB 크기의 버퍼를 선언합니다. 이 버퍼의 특정 위치에 시스템 콜을 통해 접근할 것입니다.
  char buf2[65536];

  /* Write file via write(). */
  // write()를 통해 파일에 씁니다.
  // "sample.txt"라는 이름으로 slen 길이만큼의 파일을 생성합니다. 성공 여부를 CHECK 합니다.
  CHECK (create ("sample.txt", slen), "create \"sample.txt\"");
  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // sample 데이터를 파일에 씁니다. 쓰여진 바이트 수가 slen과 같은지 CHECK 합니다.
  CHECK (write (handle, sample, slen) == slen, "write \"sample.txt\"");
  close (handle); // 파일을 닫습니다.

  /* Read back via read(). */
  // read()를 통해 다시 읽습니다.
  // "sample.txt" 파일을 다시 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "2nd open \"sample.txt\"");
  // 파일 내용을 buf2 배열의 중간 지점(buf2 + 32768)부터 slen 길이만큼 읽어들입니다.
  // 이 read 시스템 콜 시점에 buf2 + 32768 주소에 해당하는 스택 페이지가 유효하게 확장되어야 합니다.
  CHECK (read (handle, buf2 + 32768, slen) == slen, "read \"sample.txt\"");

  // 읽어들인 데이터(buf2 + 32768)가 원본 sample 데이터와 일치하는지 확인합니다.
  CHECK (!memcmp (sample, buf2 + 32768, slen), "compare written data against read data");
  close (handle); // 파일을 닫습니다.
}