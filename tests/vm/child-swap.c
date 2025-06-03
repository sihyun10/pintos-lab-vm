/* Each thread will read 5MB of anonymous pages
 * Lastly, frees the allocated memory.
 */
// 각 스레드는 5MB의 익명 페이지를 읽을 것입니다. (주석은 스레드를 언급하지만, 이 코드는 단일 test_main)
// 마지막으로 할당된 메모리를 해제합니다. (이 코드에서는 명시적 해제보다는 exit에 의한 OS 정리를 의미하는 듯)

#include <string.h>  // 문자열 함수 (여기서는 직접 사용되지 않음)
#include <stdint.h>  // 정수형 타입 (size_t 등)
#include <syscall.h> // Pintos 시스템 콜 (exit, fail 등)
#include "tests/lib.h"  // Pintos 테스트 라이브러리 (fail 등)
#include "tests/main.h" // 테스트 프레임워크의 메인 부분을 포함합니다.


#define PAGE_SHIFT 12                // 페이지 크기를 위한 시프트 값 (2^12 = 4096)
#define PAGE_SIZE (1 << PAGE_SHIFT)  // 페이지 크기 (4KB)
#define ONE_MB (1 << 20)             // 1MB 크기
#define CHUNK_SIZE (1*ONE_MB)        // 테스트에서 사용할 메모리 청크 크기 (1MB)
#define PAGE_COUNT (CHUNK_SIZE / PAGE_SIZE) // 청크 내 페이지 수 (1MB / 4KB = 256 페이지)

static char big_chunks[CHUNK_SIZE]; // 1MB 크기의 정적 데이터 배열입니다. 익명 페이지로 취급될 것입니다.

void
test_main (void)                    // 테스트의 메인 함수입니다.
{
    size_t i;                         // 반복문 인덱스 변수입니다.
    char *mem;                        // 메모리 특정 위치를 가리킬 포인터입니다.

    // CHUNK_SIZE (1MB) 만큼의 메모리 영역을 페이지 단위로 순회하며 값을 씁니다.
    // 이 과정에서 페이지 폴트가 발생하고, 물리 메모리가 할당되며, 필요시 스왑 아웃될 수 있습니다.
    for (i = 0 ; i < PAGE_COUNT ; i++) {
        mem = (big_chunks+(i*PAGE_SIZE)); // 현재 페이지의 시작 주소를 계산합니다.
        *mem = (char)i;                   // 해당 페이지의 첫 바이트에 페이지 번호(i)를 저장합니다.
    }

    // 다시 메모리 영역을 페이지 단위로 순회하며 값을 읽고 확인합니다.
    // 이 과정에서 스왑 아웃된 페이지는 다시 스왑 인 되어야 합니다.
    for (i = 0 ; i < PAGE_COUNT ; i++) {
        mem = (big_chunks+(i*PAGE_SIZE)); // 현재 페이지의 시작 주소를 계산합니다.
        if((char)i != *mem) {             // 이전에 저장한 값과 현재 읽은 값이 다르면,
            fail ("data is inconsistent");  // 데이터 불일치로 테스트 실패를 알립니다.
        }
    }
    exit(0); // 모든 검사를 통과하면 성공적으로 종료합니다 (종료 코드 0).
}