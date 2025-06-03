/* Child process for mmap-inherit test.
Tries to write to a mapping present in the parent.
   The process must be terminated with -1 exit code. */
// mmap-inherit 테스트를 위한 자식 프로세스입니다.
// 부모에 있는 매핑에 쓰기를 시도합니다.
// 프로세스는 -1 종료 코드로 종료되어야 합니다.

#include <string.h>             // memset 함수를 사용하기 위해 포함합니다.
#include "tests/vm/sample.inc" // 테스트에 사용될 샘플 데이터를 포함합니다. (sample 배열 등)
#include "tests/lib.h"         // Pintos 테스트 라이브러리를 포함합니다. (fail, CHECK 등)
#include "tests/main.h"        // Pintos 테스트 프레임워크의 메인 부분을 포함합니다. (test_main 등)

void
test_main (void)                // 테스트의 메인 함수입니다.
{
    // 0x54321000 주소부터 4096 바이트(한 페이지)를 0으로 채우려고 시도합니다.
    // 이 주소는 부모 프로세스로부터 상속받은 메모리 매핑 영역이어야 하며,
    // 자식 프로세스가 이 영역에 쓰기를 시도할 때 접근 권한 등에 의해 실패해야 합니다.
    memset ((char *) 0x54321000, 0, 4096);

    // 만약 위의 memset 작업이 성공했다면 (즉, 자식 프로세스가 부모의 메모리 매핑을 수정할 수 있다면),
    // 테스트는 실패한 것입니다. 이 경우 fail 함수를 호출하여 테스트 실패를 알립니다.
    fail ("child can modify parent's memory mappings");
}