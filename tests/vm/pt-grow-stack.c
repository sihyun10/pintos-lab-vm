/* Demonstrate that the stack can grow.
   This must succeed. */
/* 스택이 필요에 따라 증가할 수 있음을 보여줍니다.
 * 스택에 4KB 크기의 객체를 할당하고 데이터를 쓴 후 체크섬을 계산합니다.
 * 이 과정은 반드시 성공적으로 완료되어야 하며,
 * 이는 스택이 해당 크기만큼 정상적으로 확장되었음을 의미합니다. */
#include <string.h>
#include "tests/arc4.h"
#include "tests/cksum.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) // 테스트의 메인 함수입니다.
{
  char stack_obj[4096]; // 스택에 4KB (4096 바이트) 크기의 배열을 선언합니다.
  struct arc4 arc4;     // ARC4 암호화(여기서는 데이터 패턴 생성용) 상태 구조체입니다.

  arc4_init (&arc4, "foobar", 6);        // ARC4를 "foobar" 키로 초기화합니다.
  memset (stack_obj, 0, sizeof stack_obj); // 스택 객체(stack_obj)를 0으로 초기화합니다.
  arc4_crypt (&arc4, stack_obj, sizeof stack_obj); // ARC4로 stack_obj의 내용을 암호화(변형)합니다.
  // stack_obj의 체크섬을 계산하고 "cksum: 값" 형식으로 메시지를 출력합니다.
  // 이 과정에서 스택 공간이 할당(필요시 확장)되어야 합니다.
  msg ("cksum: %lu", cksum (stack_obj, sizeof stack_obj));
}