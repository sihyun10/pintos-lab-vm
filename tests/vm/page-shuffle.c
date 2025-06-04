/* Shuffles a 128 kB data buffer 10 times, printing the checksum
after each time. */
// 128KB 데이터 버퍼를 10번 섞고, 매번 그 후의
// 체크섬을 출력합니다.

#include <stdbool.h>
#include "tests/arc4.h"
#include "tests/cksum.h"
#include "tests/lib.h"
#include "tests/main.h"

#define SIZE (128 * 1024) // 버퍼 크기를 128KB로 정의합니다.

static char buf[SIZE]; // 128KB 크기의 정적 버퍼를 선언합니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
  size_t i; // 반복문에서 사용할 인덱스 변수입니다.

  /* Initialize. */
  // 초기화합니다.
  for (i = 0; i < sizeof buf; i++) // 버퍼의 모든 바이트를 순회합니다.
    buf[i] = i * 257;             // 각 바이트를 i * 257 값으로 초기화합니다. (데이터 패턴 생성)
  // 초기화된 데이터의 체크섬을 계산하고 "init: cksum=값" 형식으로 메시지를 출력합니다.
  msg ("init: cksum=%lu", cksum (buf, sizeof buf));

  /* Shuffle repeatedly. */
  // 반복적으로 섞습니다.
  for (i = 0; i < 10; i++) // 10번 반복합니다.
  {
    // 버퍼(buf)의 데이터를 섞습니다. (세 번째 인자 1은 섞는 강도 또는 횟수를 의미할 수 있음)
    shuffle (buf, sizeof buf, 1);
    // 섞인 후 데이터의 체크섬을 계산하고 "shuffle 인덱스: cksum=값" 형식으로 메시지를 출력합니다.
    msg ("shuffle %zu: cksum=%lu", i, cksum (buf, sizeof buf));
  }
}