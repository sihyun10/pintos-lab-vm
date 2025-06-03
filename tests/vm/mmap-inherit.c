/* Maps a file into memory and runs child-inherit to verify that
   mappings are not inherited. */
// 파일을 메모리에 매핑하고 child-inherit를 실행하여
// 매핑이 상속되지 않는다는 것을 확인합니다.

#include <string.h>
#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
	char *actual = (char *) 0x54321000; // 메모리 매핑을 위한 타겟 가상 주소입니다.
	int handle;                         // 파일 핸들을 저장할 변수입니다.
	pid_t child;                        // 자식 프로세스의 ID를 저장할 변수입니다.

	/* Open file, map, verify data. */
	// 파일을 열고, 매핑하고, 데이터를 검증합니다.
	// "sample.txt" 파일을 엽니다. 성공 여부를 CHECK 합니다.
	CHECK ((handle = open ("sample.txt")) > 1, "open \"sample.txt\"");
	// actual 주소에 파일을 4096 바이트 크기만큼, 읽기 전용(세 번째 인자 0)으로 메모리 매핑합니다.
	// 매핑 실패 여부를 CHECK 합니다.
	CHECK (mmap (actual, 4096, 0, handle, 0) != MAP_FAILED, "mmap \"sample.txt\"");
	// 매핑된 메모리(actual)의 내용이 원본 sample 데이터와 일치하는지 확인합니다.
	if (memcmp (actual, sample, strlen (sample)))
		fail ("read of mmap'd file reported bad data"); // 일치하지 않으면 테스트 실패입니다.

	/* Spawn child and wait. */
	// 자식 프로세스를 생성하고 기다립니다.
	child = fork("child-inherit"); // "child-inherit"라는 이름으로 자식 프로세스를 fork 합니다.
	if (child == 0) { // 자식 프로세스인 경우
		// "child-inherit"를 실행합니다. exec 호출 성공 여부를 CHECK 합니다.
		CHECK (exec ("child-inherit") != -1, "exec \"child-inherit\"");
	}	else { // 부모 프로세스인 경우
		quiet = true; // 테스트 라이브러리의 상세 메시지 출력을 잠시 억제합니다.
		// 자식 프로세스가 종료될 때까지 기다립니다. 자식의 종료 코드가 -1인지 CHECK 합니다.
		CHECK (wait (child) == -1, "wait for child (should return -1)");
		quiet = false; // 메시지 출력을 다시 활성화합니다.
	}

	/* Verify data again. */
	// 데이터를 다시 검증합니다.
	// 자식 프로세스의 작업 후에도 부모의 매핑된 메모리 내용이 원본 sample 데이터와 여전히 일치하는지 확인합니다.
	// (자식의 쓰기 시도는 실패했어야 하므로 부모의 데이터나 파일은 변경되지 않았어야 합니다.)
	CHECK (!memcmp (actual, sample, strlen (sample)),
		   "checking that mmap'd file still has same data");
}