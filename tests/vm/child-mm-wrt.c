/* Child process of mmap-exit.
   Mmaps a file and writes to it via the mmap'ing, then exits
   without calling munmap.  The data in the mapped region must be
   written out at program termination. */
// mmap-exit 테스트의 자식 프로세스입니다.
// 파일을 메모리 맵하고, 메모리 맵을 통해 파일에 쓴 다음,
// munmap을 호출하지 않고 종료합니다. 맵된 영역의 데이터는
// 프로그램 종료 시 파일에 기록되어야 합니다.

#include <string.h>             // memcpy 함수를 사용하기 위해 포함합니다.
#include <syscall.h>            // Pintos 시스템 콜(create, open, mmap 등)을 사용하기 위해 포함합니다.
#include "tests/vm/sample.inc" // 테스트에 사용될 샘플 데이터(sample 배열)를 포함합니다.
#include "tests/lib.h"         // Pintos 테스트 라이브러리(CHECK, MAP_FAILED 등)를 포함합니다.
#include "tests/main.h"        // 테스트 프레임워크의 메인 부분을 포함합니다.

#define ACTUAL ((void *) 0x10000000) // 메모리 매핑을 위한 가상 주소입니다.

void
test_main (void)                    // 테스트의 메인 함수입니다.
{
    int handle;                       // 파일 핸들(디스크립터)을 저장할 변수입니다.

    // "sample.txt"라는 이름으로 sample 데이터 크기만큼의 파일을 생성합니다. 성공 여부를 CHECK 합니다.
    CHECK (create ("sample.txt", sizeof sample), "create \"sample.txt\"");
    // "sample.txt" 파일을 엽니다. 파일 핸들이 1보다 큰지 (유효한지) CHECK 합니다.
    CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
    // "sample.txt" 파일을 ACTUAL 주소에 sample 크기만큼 메모리 매핑합니다. PROT_WRITE (1) 권한을 부여합니다.
    // 매핑 실패 여부를 CHECK 합니다.
    CHECK (mmap (ACTUAL, sizeof sample, 1, handle, 0) != MAP_FAILED, "mmap \"sample.txt\"");
    // sample 데이터를 메모리 매핑된 영역(ACTUAL)으로 복사합니다.
    memcpy (ACTUAL, sample, sizeof sample);
    // munmap을 호출하지 않고 함수가 종료됩니다. 이 때 ACTUAL에 쓰여진 내용이 파일에 반영되어야 합니다.
}