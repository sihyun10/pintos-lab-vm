/* Checks if file-mapped pages 
 * are properly swapped out and swapped in 
 * For this test, Pintos memory size is 128MB 
 * First, fills the memory with with anonymous pages
 * and then tries to map a file into the page */
/* 파일 매핑된 페이지(file-mapped pages)가 제대로 스왑 아웃(swap out) 및 스왑 인(swap in) 되는지 확인합니다.
 * Pintos 메모리 크기가 128MB로 설정된 환경에서 테스트합니다.
 * 먼저 메모리를 익명 페이지로 채운 다음 (주석에는 그렇게 되어 있지만, 코드에서는
 * 이 부분이 명시적으로 구현되어 있지 않고 바로 파일 매핑을 시도합니다),
 * 파일을 페이지에 매핑하고 데이터 일관성을 확인합니다. */

#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include <stdint.h>
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/vm/large.inc" // 'large' 배열 (테스트용 대용량 데이터)을 포함합니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
    size_t handle; // 파일 핸들을 저장할 변수입니다.
    char *actual = (char *) 0x10000000; // 메모리 매핑을 위한 타겟 가상 주소입니다.
    void *map;     // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.
    size_t i;      // 반복문 인덱스 변수입니다.

    /* Map a page to a file */
    // 페이지를 파일에 매핑합니다.
    // "large.txt" 파일을 엽니다. (large.txt는 large 데이터로 미리 만들어져 있어야 함)
    // 성공 여부를 CHECK 합니다.
    CHECK ((handle = open ("large.txt")) > 1, "open \"large.txt\"");
    // actual 주소에 "large.txt" 파일을 large 데이터 전체 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
    // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
    CHECK ((map = mmap (actual, sizeof(large), 0, handle, 0)) != MAP_FAILED, "mmap \"large.txt\"");

    /* Check that data is correct. */
    // 데이터가 올바른지 확인합니다.
    // 매핑된 메모리(actual)의 내용이 원본 large 데이터와 (large 데이터의 문자열 길이만큼) 일치하는지 확인합니다.
    if (memcmp (actual, large, strlen (large)))
        fail ("read of mmap'd file reported bad data"); // 일치하지 않으면 테스트 실패를 알립니다.

    /* Verify that data is followed by zeros. */
    // 데이터 뒤에 0이 오는지 확인합니다. (파일 크기가 페이지 크기의 배수가 아닐 경우, 나머지 부분은 0으로 채워짐)
    size_t len = strlen(large); // large 데이터의 실제 문자열 길이를 가져옵니다.
    size_t page_end;            // 파일 내용이 끝나는 지점 다음의 첫 페이지 경계 오프셋을 계산하기 위한 변수입니다.
    // len을 포함하는 마지막 페이지의 끝 오프셋을 계산합니다.
    for(page_end = 0; page_end < len; page_end+=4096); // page_end는 len 이상인 최소 4096의 배수가 됩니다.

    // 실제 파일 내용(len) 다음부터 해당 내용이 포함된 마지막 페이지의 끝(page_end)까지를 순회합니다.
    for (i = len+1; i < page_end; i++)
    {
        if (actual[i] != 0) { // 해당 바이트가 0이 아니면 (페이지의 나머지 부분이 0으로 채워지지 않음)
            // 오류 메시지를 출력하고 테스트 실패를 알립니다.
            fail ("byte %zu of mmap'd region has value %02hhx (should be 0)", i, actual[i]);
        }
    }

    /* Unmap and close opend file */
    // 매핑을 해제하고 열린 파일을 닫습니다.
    munmap (map);   // 메모리 매핑을 해제합니다.
    close (handle); // 파일을 닫습니다.
}