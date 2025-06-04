/* Checks if anonymous pages 
 * are swapped out and swapped in properly 
 * For this test, Pintos memory size is 10MB 
 * First, allocates big chunks of memory, 
 * does some write operations on each chunk, 
 * then check if the data is consistent
 * Lastly, frees the allocated memory. */
// 익명 페이지가
// 제대로 스왑 아웃되고 스왑 인 되는지 확인합니다.
// 이 테스트를 위해 Pintos 메모리 크기는 10MB입니다.
// 먼저 큰 메모리 청크를 할당하고,
// 각 청크에 쓰기 작업을 수행한 다음,
// 데이터가 일관성이 있는지 확인합니다.
// 마지막으로 할당된 메모리를 해제합니다.

#include <string.h>
#include <stdint.h>
#include <syscall.h>
#include "tests/lib.h"
#include "tests/main.h"


#define PAGE_SHIFT 12                // 페이지 크기를 위한 시프트 값 (2^12 = 4096)
#define PAGE_SIZE (1 << PAGE_SHIFT)  // 페이지 크기 (4KB)
#define ONE_MB (1 << 20)             // 1MB 크기
#define CHUNK_SIZE (20*ONE_MB)       // 테스트에서 사용할 전체 메모리 청크 크기 (20MB)
#define PAGE_COUNT (CHUNK_SIZE / PAGE_SIZE) // 청크 내 총 페이지 수 (20MB / 4KB = 5120 페이지)

static char big_chunks[CHUNK_SIZE]; // 20MB 크기의 정적 데이터 배열입니다. 익명 페이지로 취급됩니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
	size_t i;    // 반복문 인덱스 변수입니다.
    // void* pa;    // 이 변수는 코드에서 사용되지 않습니다.
    char *mem;   // 메모리 특정 위치를 가리킬 포인터입니다.

    // CHUNK_SIZE (20MB) 만큼의 메모리 영역을 페이지 단위로 순회하며 값을 씁니다.
    // 메모리 크기가 10MB로 제한되어 있으므로, 이 과정에서 스왑 아웃이 발생해야 합니다.
    for (i = 0 ; i < PAGE_COUNT ; i++) {
        if(!(i % 512)) // 512 페이지마다 (약 2MB 간격) 메시지를 출력합니다.
            msg ("write sparsely over page %zu", i);
        mem = (big_chunks+(i*PAGE_SIZE)); // 현재 페이지의 시작 주소를 계산합니다.
        *mem = (char)i;                   // 해당 페이지의 첫 바이트에 페이지 번호(i)의 하위 바이트를 저장합니다.
    }

    // 다시 메모리 영역을 페이지 단위로 순회하며 값을 읽고 확인합니다.
    // 스왑 아웃된 페이지는 이 과정에서 다시 스왑 인 되어야 합니다.
    for (i = 0 ; i < PAGE_COUNT ; i++) {
        mem = (big_chunks+(i*PAGE_SIZE)); // 현재 페이지의 시작 주소를 계산합니다.
        if((char)i != *mem) {             // 이전에 저장한 값과 현재 읽은 값이 다르면,
		    fail ("data is inconsistent");  // 데이터 불일치로 테스트 실패를 알립니다.
        }
        if(!(i % 512)) // 512 페이지마다 메시지를 출력합니다.
            msg ("check consistency in page %zu", i);
    }
    // 명시적인 메모리 해제는 없으며, 프로세스 종료 시 OS가 정리합니다.
}