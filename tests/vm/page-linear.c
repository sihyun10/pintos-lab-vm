/* Encrypts, then decrypts, 2 MB of memory and verifies that the
values are as they should be. */
// (주석은 2MB를 언급하지만, 코드는 SIZE (5MB)를 사용합니다.)
/* 5MB 크기의 메모리 버퍼를 특정 값(0x5a)으로 초기화하고, 그 내용이 올바른지 확인합니다.
 * 그런 다음 ARC4 알고리즘을 사용하여 버퍼를 암호화하고 다시 복호화한 후,
 * 최종적으로 버퍼의 내용이 다시 원래의 값(0x5a)으로 돌아왔는지 검증합니다.
 * 이는 대용량 메모리 접근 및 수정을 테스트합니다. */

#include <string.h>
#include "tests/arc4.h"
#include "tests/lib.h"
#include "tests/main.h"

#define SIZE (5 * 1024 * 1024) // 버퍼 크기를 5MB로 정의합니다.

static char buf[SIZE]; // 5MB 크기의 정적 버퍼를 선언합니다.

void
test_main (void) // 테스트의 메인 함수입니다.
{
  struct arc4 arc4; // ARC4 암호화 상태를 저장할 구조체입니다.
  size_t i;         // 반복문에서 사용할 인덱스 변수입니다.

  /* Initialize to 0x5a. */
  // 0x5a로 초기화합니다.
  msg ("initialize"); // "initialize" 메시지를 출력합니다.
  memset (buf, 0x5a, sizeof buf); // 버퍼 전체를 0x5a 값으로 채웁니다.

  /* Check that it's all 0x5a. */
  // 모든 내용이 0x5a인지 확인합니다.
  msg ("read pass"); // "read pass" 메시지를 출력합니다.
  for (i = 0; i < SIZE; i++) // 버퍼의 모든 바이트를 순회합니다.
    if (buf[i] != 0x5a)    // 만약 해당 바이트가 0x5a가 아니면,
      fail ("byte %zu != 0x5a", i); // 테스트 실패를 알립니다.

  /* Encrypt zeros. */ // (주석은 "Encrypt zeros"이지만 실제로는 0x5a로 채워진 데이터를 암호화합니다.)
  // 0x5a 데이터를 암호화합니다.
  msg ("read/modify/write pass one"); // "read/modify/write pass one" 메시지를 출력합니다.
  arc4_init (&arc4, "foobar", 6);   // ARC4 구조체를 "foobar" 키로 초기화합니다.
  arc4_crypt (&arc4, buf, SIZE);    // buf의 내용을 SIZE만큼 암호화합니다.

  /* Decrypt back to zeros. */ // (주석은 "Decrypt back to zeros"이지만 실제로는 0x5a로 복호화합니다.)
  // 다시 0x5a로 복호화합니다.
  msg ("read/modify/write pass two"); // "read/modify/write pass two" 메시지를 출력합니다.
  arc4_init (&arc4, "foobar", 6);   // ARC4 구조체를 동일한 키로 다시 초기화합니다.
  arc4_crypt (&arc4, buf, SIZE);    // 암호화된 buf의 내용을 SIZE만큼 복호화합니다.

  /* Check that it's all 0x5a. */
  // 모든 내용이 다시 0x5a인지 확인합니다.
  msg ("read pass"); // "read pass" 메시지를 출력합니다.
  for (i = 0; i < SIZE; i++) // 버퍼의 모든 바이트를 순회합니다.
    if (buf[i] != 0x5a)    // 만약 해당 바이트가 0x5a가 아니면,
      fail ("byte %zu != 0x5a", i); // 테스트 실패를 알립니다.
}