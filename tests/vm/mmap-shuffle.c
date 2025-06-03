/* Creates a 128 kB file and repeatedly shuffles data in it
through a memory mapping. */
/* 128KB 크기의 파일을 생성하고, 해당 파일을 메모리에 매핑한 후,
 * 매핑된 메모리 영역 내의 데이터를 반복적으로 섞습니다(shuffle).
 * 각 셔플 단계 후 데이터의 체크섬(cksum)을 계산하여 데이터가 실제로 변경되고 있음을 (간접적으로) 보여줍니다. */

#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include "tests/arc4.h"    // shuffle 함수를 사용하기 위해 포함합니다.
#include "tests/cksum.h"   // cksum 함수를 사용하기 위해 포함합니다.
#include "tests/lib.h"
#include "tests/main.h"

#define SIZE (128 * 1024) // 파일 및 버퍼의 크기를 128KB로 정의합니다.

static char *buf = (char *) 0x10000000; // 메모리 매핑을 위한 타겟 가상 주소 (128KB 영역을 가리킴)

void
test_main (void) // 테스트의 메인 함수입니다.
{
  size_t i;      // 반복문에서 사용할 인덱스 변수입니다.
  int handle;    // 파일 핸들을 저장할 변수입니다.

  /* Create file, mmap. */
  // 파일을 생성하고 매핑합니다.
  // "buffer"라는 이름으로 SIZE만큼의 파일을 생성합니다. 성공 여부를 CHECK 합니다.
  CHECK (create ("buffer", SIZE), "create \"buffer\"");
  // "buffer" 파일을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open ("buffer")) > 1, "open \"buffer\"");
  // buf 주소에 "buffer" 파일을 SIZE만큼, 쓰기 가능(세 번째 인자 1)으로 메모리 매핑합니다.
  // 매핑 성공 여부(MAP_FAILED가 아닌지)를 CHECK 합니다.
  CHECK (mmap (buf, SIZE, 1, handle, 0) != MAP_FAILED, "mmap \"buffer\"");

  /* Initialize. */
  // 데이터를 초기화합니다.
  // 매핑된 버퍼(buf)의 각 바이트를 특정 값 (i * 257)으로 초기화합니다.
  for (i = 0; i < SIZE; i++)
    buf[i] = i * 257;
  // 초기화된 데이터의 체크섬을 계산하고 출력합니다.
  msg ("init: cksum=%lu", cksum (buf, SIZE));

  /* Shuffle repeatedly. */
  // 반복적으로 섞습니다.
  // 10번 반복합니다.
  for (i = 0; i < 10; i++)
  {
    shuffle (buf, SIZE, 1); // 매핑된 버퍼(buf)의 데이터를 섞습니다. (1은 섞는 횟수 또는 강도를 의미할 수 있음)
    // 섞인 후 데이터의 체크섬을 계산하고 출력합니다.
    msg ("shuffle %zu: cksum=%lu", i, cksum (buf, SIZE));
  }
  // munmap이나 close는 이 테스트에서 명시적으로 호출되지 않을 수 있습니다 (프로세스 종료 시 정리).
}