/* Executes child-mm-wrt and verifies that the writes that should
   have occurred really did. */
// child-mm-wrt를 실행하고, 발생했어야 하는 쓰기 작업이
// 실제로 이루어졌는지 확인합니다.

#include <syscall.h>
#include "tests/vm/sample.inc"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
	pid_t child;   // 자식 프로세스의 ID를 저장할 변수입니다.

	/* Make child write file. */
	// 자식 프로세스가 파일을 쓰도록 합니다.
	quiet = true; // 테스트 라이브러리의 상세 메시지 출력을 잠시 억제합니다.
	child = fork("child-mm-wrt"); // "child-mm-wrt"라는 이름으로 자식 프로세스를 fork 합니다.
	if (child == 0) { // 자식 프로세스인 경우
		// "child-mm-wrt"를 실행합니다. exec 호출 성공 여부를 CHECK 합니다.
		CHECK ((child = exec ("child-mm-wrt")) != -1, "exec \"child-mm-wrt\"");
	} else { // 부모 프로세스인 경우
		// 자식 프로세스가 종료될 때까지 기다립니다. 자식의 종료 코드가 0인지 CHECK 합니다.
		CHECK (wait (child) == 0, "wait for child (should return 0)");
		quiet = false; // 메시지 출력을 다시 활성화합니다.

		/* Check file contents. */
		// 파일 내용을 확인합니다.
		// "sample.txt" 파일의 내용이 예상되는 sample 데이터와 일치하는지 확인합니다.
		check_file ("sample.txt", sample, sizeof sample);
	}
}