/* Mmaps a 128 kB file "sorts" the bytes in it, using quick sort,
   a multi-pass divide and conquer algorithm.  */
// 128KB 파일을 메모리 맵하고 그 안의 바이트들을
// 퀵 정렬(다중 패스 분할 정복 알고리즘)을 사용하여 "정렬"합니다.

#include <debug.h>      // DEBUG 매크로 등을 사용하기 위해 포함합니다 (여기선 직접 사용 안됨).
#include <syscall.h>    // Pintos 시스템 콜(open, mmap 등)을 사용하기 위해 포함합니다.
#include "tests/lib.h"  // Pintos 테스트 라이브러리(CHECK, quiet, MAP_FAILED 등)를 포함합니다.
#include "tests/main.h" // 테스트 프레임워크의 메인 부분을 포함합니다.
#include "tests/vm/qsort.h" // qsort_bytes 함수를 사용하기 위해 포함합니다.

const char *test_name = "child-qsort-mm"; // 테스트의 이름을 정의합니다.

int
main (int argc UNUSED, char *argv[]) // 프로그램의 주 진입점입니다. argc는 사용되지 않음을 표시합니다.
{
    int handle;                        // 파일 핸들을 저장할 변수입니다.
    // 메모리 매핑을 위한 타겟 가상 주소입니다. (0x10000000)
    unsigned char *p = (unsigned char *) 0x10000000;

    quiet = true;                      // 테스트 라이브러리의 상세 메시지 출력을 억제합니다.

    // 명령행 인자로 받은 파일(argv[1])을 엽니다. 성공 여부를 CHECK 합니다.
    CHECK ((handle = open (argv[1])) > 1, "open \"%s\"", argv[1]);
    // 파일을 p 주소에 4096*33 (132KB) 바이트 크기만큼, 쓰기 가능(1)으로 메모리 매핑합니다.
    // 매핑 실패 여부를 CHECK 합니다.
    CHECK (mmap (p, 4096*33, 1, handle, 0) != MAP_FAILED, "mmap \"%s\"", argv[1]);
    // 메모리 매핑된 영역(p)의 데이터를 128KB (1024*128) 만큼 바이트 단위로 퀵 정렬합니다.
    qsort_bytes (p, 1024 * 128);

    // munmap이나 close 없이 종료될 수 있습니다 (테스트 시나리오에 따라).
    return 80; // 성공 시 특정 값(80)을 반환합니다.
}