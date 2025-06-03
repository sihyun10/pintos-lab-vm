/* Allocates and writes to a 64 kB object on the stack.
   This must succeed. */
/* 스택에 64KB 크기의 큰 객체(배열)를 할당하고 해당 객체에 데이터를 씁니다.
 * 이 작업은 성공적으로 수행되어야 하며, 이는 스택이 필요에 따라 충분히 확장될 수 있음을 테스트합니다. */

#include <string.h>
#include "tests/arc4.h"
#include "tests/cksum.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  char stk_obj[65536]; // 스택에 64KB (65536 바이트) 크기의 배열을 선언합니다.
  struct arc4 arc4;    // ARC4 암호화(여기서는 데이터 패턴 생성용) 상태 구조체입니다.

  arc4_init (&arc4, "foobar", 6);     // ARC4를 "foobar" 키로 초기화합니다.
  memset (stk_obj, 0, sizeof stk_obj); // 스택 객체(stk_obj)를 0으로 초기화합니다.
  arc4_crypt (&arc4, stk_obj, sizeof stk_obj); // ARC4로 stk_obj의 내용을 암호화(변형)합니다.
  // stk_obj의 체크섬을 계산하고 "cksum: 값" 형식으로 메시지를 출력합니다.
  // 이 과정에서 스택 확장이 필요하면 제대로 이루어져야 합니다.
  msg ("cksum: %lu", cksum (stk_obj, sizeof stk_obj));
}