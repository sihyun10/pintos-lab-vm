/* Checks if anonymous pages are lazy loaded  */
// 익명 페이지가 지연 로딩되는지 확인합니다.

#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include <stdint.h>
#include "tests/lib.h"
#include "tests/main.h"

#define PAGE_SIZE 4096                             // 페이지 크기를 4096 바이트로 정의합니다.
#define CHUNK_PAGE_COUNT 3                         // 테스트에 사용할 페이지의 개수를 정의합니다.
#define CHUNK_SIZE (CHUNK_PAGE_COUNT * PAGE_SIZE)  // 전체 청크 크기를 계산합니다 (3 * 4KB = 12KB).

static char buf[CHUNK_SIZE]; // 익명 페이지로 사용될 정적 버퍼를 선언합니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
	size_t i, j;   // 반복문에서 사용할 인덱스 변수입니다.
	void *pa;      // 물리 주소를 저장할 포인터입니다.

	msg ("initial pages status"); // 초기 페이지 상태 메시지를 출력합니다.
	for (i = 0 ; i < CHUNK_PAGE_COUNT ; i++) { // 버퍼 내 각 페이지를 순회합니다.
		// All pages for buf should not be loaded yet.
		// buf의 모든 페이지는 아직 로드되지 않았어야 합니다.
		pa = get_phys_addr(&buf[i*PAGE_SIZE]);      // 현재 페이지의 시작 가상 주소에 대한 물리 주소를 가져옵니다.
		CHECK (pa == 0, "check if page is not loaded"); // 물리 주소가 0인지 (즉, 아직 물리 메모리에 할당되지 않았는지) 확인합니다.
	}

	msg ("load pages"); // 페이지 로드 시작 메시지를 출력합니다.
	for (i = 0 ; i < CHUNK_PAGE_COUNT ; i++) { // 각 페이지를 순차적으로 접근하여 로드합니다.
		msg ("load page [%zu]", i); // 현재 로드할 페이지 번호를 출력합니다.
		// Pages are loaded here.
		// 여기서 페이지가 로드됩니다.
		buf[i*PAGE_SIZE] = i; // 현재 페이지의 첫 바이트에 페이지 번호(i)를 씁니다. 이 접근으로 인해 페이지 폴트가 발생하고 페이지가 로드됩니다.
		for (j = 0 ; j < CHUNK_PAGE_COUNT ; j++) { // 모든 페이지의 상태를 다시 확인합니다.
			// Pages that have been accessed should be loaded
			// 접근된 페이지는 로드되어 있어야 합니다.
			// Pages that have not been accessed should not be loaded.
			// 접근되지 않은 페이지는 로드되지 않았어야 합니다.
			pa = get_phys_addr(&buf[j*PAGE_SIZE]); // j번째 페이지의 물리 주소를 가져옵니다.
			if (j <= i) { // 현재까지 접근한 페이지들(0부터 i번째 페이지까지)은
				CHECK (pa != 0, "check if page is loaded"); // 물리 주소가 0이 아닌지(로드되었는지) 확인합니다.
				CHECK (buf[j*PAGE_SIZE] == (char) j, "check memory content"); // 메모리 내용이 올바르게 쓰였는지 확인합니다.
			}
			else { // 아직 접근하지 않은 페이지들은
				CHECK (pa == 0, "check if page is not loaded"); // 물리 주소가 0인지(로드되지 않았는지) 확인합니다.
			}
		}
	}
}