/* Try to write to the code segment using a system call.
   The process must be terminated with -1 exit code. */
/* 시스템 콜(read)을 사용하여 프로그램의 코드 세그먼트에 데이터를 쓰려고 시도합니다.
 * 열려 있는 파일("sample.txt")의 내용을 코드 세그먼트 주소(test_main 함수의 시작 주소)로 읽어들이려고 합니다.
 * 이 작업은 실패해야 하며, 프로세스는 -1 종료 코드로 종료되어야 합니다 */
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  int handle; // 파일 핸들을 저장할 변수입니다.

  // "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
  // 파일(handle)에서 1 바이트를 읽어, test_main 함수의 시작 주소(코드 세그먼트)에 저장하려고 시도합니다.
  // 이 작업은 메모리 보호 위반으로 인해 시스템 콜이 실패하거나 프로세스가 종료되어야 합니다.
  read (handle, (void *) test_main, 1);
  // 만약 read 호출 이후에도 프로그램이 계속 실행된다면, 테스트는 실패한 것입니다.
  fail ("survived reading data into code segment");
}