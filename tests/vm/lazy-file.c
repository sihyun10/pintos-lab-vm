/* Checks if file-mapped pages are lazy loaded  */
// 파일 매핑된 페이지가 지연 로딩되는지 확인합니다.

#include <string.h>  // memcmp 함수를 사용하기 위해 포함합니다.
#include <syscall.h> // Pintos 시스템 콜 (open, mmap, munmap, close, get_phys_addr 등)을 사용하기 위해 포함합니다.
#include <stdio.h>   // msg 함수 (Pintos 테스트 라이브러리 내 구현)를 사용하기 위해 포함합니다.
#include <stdint.h>  // 정수형 타입 (size_t 등)
#include "tests/lib.h"  // Pintos 테스트 라이브러리 (CHECK, fail, msg, MAP_FAILED 등)를 포함합니다.
#include "tests/main.h" // 테스트 프레임워크의 메인 부분을 포함합니다.
#include "tests/vm/small.inc" // 테스트용 작은 데이터(small 배열)를 포함합니다.

#define PAGE_SIZE 4096       // 페이지 크기를 4096 바이트로 정의합니다.
#define PAGE_SHIFT 12        // 페이지 크기를 위한 시프트 값 (2^12 = 4096)
// 주어진 크기 x를 페이지 크기의 배수로 올림하는 매크로입니다.
#define PAGE_ALIGN_CEIL(x) ((x % PAGE_SIZE ? (x+PAGE_SIZE) : x) >> PAGE_SHIFT << PAGE_SHIFT)

void
test_main (void)             // 테스트의 메인 함수입니다.
{
	size_t handle;             // 파일 핸들을 저장할 변수입니다.
	size_t small_size;         // small 데이터의 크기를 저장할 변수입니다.
	char *actual = (char *) 0x10000000; // 메모리 매핑을 위한 타겟 가상 주소입니다.
	void *map;                 // mmap 호출의 반환 값(매핑된 주소)을 저장할 포인터입니다.
	size_t i, j;               // 반복문에서 사용할 인덱스 변수입니다.
	size_t page_cnt;           // 매핑된 파일의 페이지 수를 저장할 변수입니다.
	void *pa;                  // 물리 주소를 저장할 포인터입니다.

	/* Map pages to a file */
	// 페이지를 파일에 매핑합니다.
	// "small.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다. (small.txt는 small 데이터로 미리 만들어져 있어야 함)
	CHECK ((handle = open ("small.txt")) > 1, "open \"small.txt\"");
	small_size = sizeof small; // small 데이터의 크기를 가져옵니다.
	msg ("sizeof small: %zu", small_size); // small 데이터의 크기를 출력합니다.
	msg ("page aligned size of small: %zu", PAGE_ALIGN_CEIL(small_size)); // 페이지 정렬된 small 데이터 크기를 출력합니다.
	// actual 주소에 small 파일을 페이지 정렬된 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
	// 매핑 실패 여부를 CHECK 합니다.
	CHECK ((map = mmap (actual, PAGE_ALIGN_CEIL(small_size), 0, handle, 0)) != MAP_FAILED, "mmap \"small.txt\"");
	page_cnt = PAGE_ALIGN_CEIL(small_size) / PAGE_SIZE; // 매핑된 영역의 페이지 수를 계산합니다.

	msg ("initial pages status"); // 초기 페이지 상태 메시지를 출력합니다.
	for (i = 0 ; i < page_cnt ; i++) { // 모든 매핑된 페이지를 순회합니다.
		// All pages for the file should not be loaded yet.
		// 파일에 대한 모든 페이지는 아직 로드되지 않았어야 합니다.
		pa = get_phys_addr(&actual[i*PAGE_SIZE]); // 현재 페이지의 물리 주소를 가져옵니다.
		CHECK (pa == 0, "check if page is not loaded"); // 물리 주소가 0인지(즉, 아직 로드되지 않았는지) 확인합니다.
	}
	msg ("load pages (%zu)", page_cnt); // 페이지 로드 시작 메시지를 출력합니다.
	for (i = 0 ; i < page_cnt ; i++) { // 모든 페이지를 순차적으로 접근하여 로드합니다.
		msg ("load page [%zu]", i); // 현재 로드할 페이지 번호를 출력합니다.
		// 현재 페이지(actual + i*PAGE_SIZE)의 내용 10바이트를 원본 small 데이터와 비교합니다.
		// 이 접근으로 인해 해당 페이지가 메모리에 로드(페이지 폴트 처리)되어야 합니다.
		if (memcmp (actual+i*PAGE_SIZE, small+i*PAGE_SIZE, 10))
			fail ("read of mmap'd file reported bad data"); // 데이터가 다르면 테스트 실패를 알립니다.

		for (j = 0 ; j < page_cnt ; j++) { // 모든 페이지의 상태를 다시 확인합니다.
			pa = get_phys_addr(&actual[j*PAGE_SIZE]); // j번째 페이지의 물리 주소를 가져옵니다.
			if (j <= i) { // 현재까지 접근한 페이지들(0부터 i번째 페이지까지)은
				// Pages that have been accessed should be loaded
				// 접근된 페이지는 로드되어 있어야 합니다.
				CHECK (pa != 0, "check if page is loaded"); // 물리 주소가 0이 아닌지(로드되었는지) 확인합니다.
			}
			else { // 아직 접근하지 않은 페이지들은
				// Pages that have not been accessed should not be loaded.
				// 접근되지 않은 페이지는 로드되지 않았어야 합니다.
				CHECK (pa == 0, "check if page is not loaded"); // 물리 주소가 0인지(로드되지 않았는지) 확인합니다.
			}
		}
	}
	munmap (map); // 메모리 매핑을 해제합니다.
	close (handle); // 파일을 닫습니다.
}