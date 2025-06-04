/* Checks if any type of memory is properly swapped out and swapped in 
 * For this test, Pintos memory size is 128MB */ 
/* 다양한 유형의 메모리(익명 페이지와 파일 매핑 페이지)가 제대로 스왑 아웃 및 스왑 인 되는지를 반복적으로 확인합니다.
 * Pintos 메모리 크기가 128MB로 설정된 환경에서 테스트합니다.
 * 먼저 큰 익명 페이지 청크에 데이터를 쓰고, 파일을 메모리에 매핑하여 읽습니다.
 * 그 다음 다시 익명 페이지의 데이터를 확인하고, 마지막으로 파일 매핑 페이지의 데이터를 다시 확인합니다.
 * 이 과정에서 메모리 부족으로 스왑이 발생했을 때 데이터 일관성이 유지되는지를 검증합니다. */

#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/vm/large.inc" // 'large' 배열 (테스트용 대용량 데이터)을 포함합니다.


#define PAGE_SHIFT 12                // 페이지 크기를 위한 시프트 값 (2^12 = 4096)
#define PAGE_SIZE (1 << PAGE_SHIFT)  // 페이지 크기 (4KB)
#define ONE_MB (1 << 20)             // 1MB 크기
#define CHUNK_SIZE (20*ONE_MB)       // 익명 페이지로 사용할 전체 메모리 청크 크기 (20MB)
#define PAGE_COUNT (CHUNK_SIZE / PAGE_SIZE) // 익명 페이지 청크 내 총 페이지 수 (20MB / 4KB = 5120 페이지)

static char big_chunks[CHUNK_SIZE]; // 20MB 크기의 정적 데이터 배열 (익명 페이지용).

void
test_main (void) // 테스트의 메인 함수입니다.
{
    size_t i, handle;                 // i는 반복문 인덱스, handle은 파일 핸들입니다.
    char *actual = (char *) 0x10000000; // 파일 매핑을 위한 타겟 가상 주소입니다.
    void *map;                        // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.

    // 익명 페이지(big_chunks)에 희소하게(sparsely) 값을 씁니다.
    // 메모리 압박 상황을 만들기 위해 많은 페이지에 접근합니다.
    for (i = 0 ; i < PAGE_COUNT ; i++) {
        if ((i & 0x1ff) == 0) // 512 페이지마다 (0x1ff == 511) 메시지를 출력합니다.
            msg ("write sparsely over page %zu", i);
        big_chunks[i*PAGE_SIZE] = (char) i; // 각 페이지의 첫 바이트에 페이지 번호(i)의 하위 바이트를 저장합니다.
    }

    // "large.txt" 파일을 엽니다. (large.txt는 large 데이터로 미리 만들어져 있어야 함)
    // 성공 여부를 CHECK 합니다.
    CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");
    // actual 주소에 "large.txt" 파일을 large 데이터 전체 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
    // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
    CHECK ((map = mmap (actual, sizeof(large), 0, handle, 0)) != MAP_FAILED, "mmap \"large.txt\"");

    /* Read in file map'd page */
    // 파일 매핑된 페이지를 읽습니다.
    // 매핑된 메모리(actual)의 내용이 원본 large 데이터와 (large 데이터의 문자열 길이만큼) 일치하는지 확인합니다.
    if (memcmp (actual, large, strlen (large)))
        fail ("read of mmap'd file reported bad data"); // 일치하지 않으면 테스트 실패를 알립니다.


    /* Read in anon page */
    // 익명 페이지를 읽습니다.
    // 이전에 썼던 익명 페이지(big_chunks)의 내용을 다시 확인합니다.
    // 스왑 아웃/인 후에도 데이터가 유지되는지 검사합니다.
    for (i = 0; i < PAGE_COUNT; i++) {
        if (big_chunks[i*PAGE_SIZE] != (char) i) // 저장했던 값과 현재 읽은 값이 다르면,
            fail ("data is inconsistent");          // 데이터 불일치로 테스트 실패를 알립니다.
        if ((i & 0x1ff) == 0) // 512 페이지마다 메시지를 출력합니다.
            msg ("check consistency in page %zu", i);
    }

    /* Check file map'd page again */
    // 파일 매핑된 페이지를 다시 확인합니다.
    // 익명 페이지 접근 후에도 파일 매핑된 페이지의 내용이 여전히 올바른지 다시 확인합니다.
    if (memcmp (actual, large, strlen (large)))
        fail ("read of mmap'd file reported bad data"); // 일치하지 않으면 테스트 실패를 알립니다.

    /* Unmap and close opend file */
    // 매핑을 해제하고 열린 파일을 닫습니다.
    munmap (map);   // 메모리 매핑을 해제합니다.
    close (handle); // 파일을 닫습니다.
}