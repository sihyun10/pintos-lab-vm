/* Child process of page-parallel.
Encrypts 1 MB of zeros, then decrypts it, and ensures that
   the zeros are back. */
// page-parallel 테스트의 자식 프로세스입니다.
// 1MB 크기의 0으로 채워진 데이터를 암호화한 후 복호화하여,
// 다시 0으로 돌아왔는지 확인합니다.

#include <string.h>        // strlen 함수를 사용하기 위해 포함합니다.
#include "tests/arc4.h"    // ARC4 암호화 알고리즘 관련 함수를 사용하기 위해 포함합니다.
#include "tests/lib.h"     // Pintos 테스트 라이브러리를 포함합니다. (fail 등)
#include "tests/main.h"    // 테스트 프레임워크의 메인 부분을 포함합니다.

const char *test_name = "child-linear"; // 테스트의 이름을 정의합니다.

#define SIZE (1024 * 1024) // 버퍼의 크기를 1MB로 정의합니다.
static char buf[SIZE];     // 1MB 크기의 정적 버퍼를 선언합니다.

int
main (int argc, char *argv[]) // 프로그램의 주 진입점입니다.
{
    const char *key = argv[argc - 1]; // 명령행 인자의 마지막 값을 암호화 키로 사용합니다.
    struct arc4 arc4;                 // ARC4 암호화 상태를 저장할 구조체입니다.
    size_t i;                         // 반복문에서 사용할 인덱스 변수입니다.

    /* Encrypt zeros. */
    // 0으로 채워진 데이터를 암호화합니다.
    arc4_init (&arc4, key, strlen (key)); // ARC4 구조체를 주어진 키로 초기화합니다.
    arc4_crypt (&arc4, buf, SIZE);        // buf의 내용을 SIZE만큼 암호화합니다. (초기 buf는 전역변수이므로 0으로 초기화됨)

    /* Decrypt back to zeros. */
    // 다시 0으로 복호화합니다.
    arc4_init (&arc4, key, strlen (key)); // ARC4 구조체를 동일한 키로 다시 초기화합니다. (암호화와 동일한 상태에서 시작)
    arc4_crypt (&arc4, buf, SIZE);        // 암호화된 buf의 내용을 SIZE만큼 복호화합니다.

    /* Check that it's all zeros. */
    // 모든 내용이 0인지 확인합니다.
    for (i = 0; i < SIZE; i++)          // 버퍼의 모든 바이트를 순회합니다.
        if (buf[i] != '\0')             // 만약 해당 바이트가 0이 아니면,
            fail ("byte %zu != 0", i);    // 테스트 실패를 알립니다.

    return 0x42; // 성공 시 특정 값(0x42)을 반환합니다.
}