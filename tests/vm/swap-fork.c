/* child-swap이라는 자식 프로세스를 여러 개(CHILD_CNT 만큼, 여기서는 10개) 생성(fork)하고 실행(exec)합니다.
 * 각 자식 프로세스는 메모리 할당 및 접근을 통해 스왑 기능을 테스트할 것으로 예상됩니다 (이름으로 유추).
 * 부모 프로세스는 모든 자식 프로세스가 성공적으로 종료(종료 코드 0 반환)될 때까지 기다립니다.
 * 하나라도 다른 코드로 종료되면, 스택 손상 등의 문제를 의심하며 테스트를 실패시킵니다 */

#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/vm/large.inc" // large 데이터는 이 파일에서 직접 사용되지 않습니다.

#define CHILD_CNT 10 // 생성할 자식 프로세스의 수를 10으로 정의합니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
	pid_t child[CHILD_CNT]; // 자식 프로세스들의 ID를 저장할 배열입니다.
	size_t i;               // 반복문 인덱스 변수입니다.

	/* Spawn children */
	// 자식들을 생성합니다.
	for(i =0; i < CHILD_CNT; i++) { // CHILD_CNT 만큼 반복합니다.
		child[i] = fork("child-swap"); // "child-swap"라는 이름으로 자식 프로세스를 fork하고 PID를 저장합니다.
		if (child[i] == 0) { // 자식 프로세스인 경우
			if(exec ("child-swap") == -1) // "child-swap"을 실행(exec)합니다. exec이 -1을 반환하면 (실패하면)
				fail("exec \"child-swap\""); // 테스트 실패를 알립니다.
		}
	}
	/* Wait for children */
	// 자식들을 기다립니다.
	for(i =0; i < CHILD_CNT; i++) { // CHILD_CNT 만큼 반복합니다.
		if(wait (child[i]) != 0) // i번째 자식 프로세스가 종료될 때까지 기다리고, 반환된 종료 코드가 0이 아니면
			// 하나 이상의 자식 프로세스 스택이 손상되었다고 가정하고 테스트 실패를 알립니다.
				fail("More than one child process' stack is corrupted");
	}
}