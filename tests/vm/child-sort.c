/* Reads a 128 kB file into static data and "sorts" the bytes in
   it, using counting sort, a single-pass algorithm.  The sorted
   data is written back to the same file in-place. */
// 128KB 파일을 정적 데이터로 읽어들여 그 안의 바이트들을
// 계수 정렬(단일 패스 알고리즘)을 사용하여 "정렬"합니다.
// 정렬된 데이터는 동일한 파일에 원래 위치에 다시 쓰여집니다.

#include <debug.h>      // DEBUG 매크로 등을 사용하기 위해 포함합니다 (여기선 직접 사용 안됨).
#include <syscall.h>    // Pintos 시스템 콜(open, read, write, seek, close 등)을 사용하기 위해 포함합니다.
#include "tests/lib.h"  // Pintos 테스트 라이브러리(CHECK, quiet 등)를 포함합니다.
#include "tests/main.h" // 테스트 프레임워크의 메인 부분을 포함합니다.

const char *test_name = "child-sort"; // 테스트의 이름을 정의합니다.

unsigned char buf[128 * 1024];     // 128KB 크기의 정적 데이터 버퍼입니다.
size_t histogram[256];             // 각 바이트 값(0-255)의 빈도수를 저장할 히스토그램 배열입니다.

int
main (int argc UNUSED, char *argv[]) // 프로그램의 주 진입점입니다. argc는 사용되지 않음을 표시합니다.
{
  int handle;                        // 파일 핸들을 저장할 변수입니다.
  unsigned char *p;                  // 버퍼를 순회하며 정렬된 값을 채워넣을 포인터입니다.
  size_t size;                       // 파일에서 읽은 실제 데이터의 크기를 저장할 변수입니다.
  size_t i;                          // 반복문에서 사용할 인덱스 변수입니다.

  quiet = true;                      // 테스트 라이브러리의 상세 메시지 출력을 억제합니다.

  // 명령행 인자로 받은 파일(argv[1])을 엽니다. 성공 여부를 CHECK 합니다.
  CHECK ((handle = open (argv[1])) > 1, "open \"%s\"", argv[1]);

  // 파일로부터 buf 크기만큼 데이터를 읽어들입니다. 실제 읽은 크기를 size에 저장합니다.
  size = read (handle, buf, sizeof buf);
  // 읽어들인 데이터(buf)의 각 바이트 값에 해당하는 히스토그램 인덱스의 값을 1 증가시킵니다.
  for (i = 0; i < size; i++)
    histogram[buf[i]]++;

  p = buf; // 포인터 p를 버퍼의 시작으로 설정합니다.
  // 히스토그램 배열을 순회합니다 (0부터 255까지의 바이트 값).
  for (i = 0; i < sizeof histogram / sizeof *histogram; i++)
    {
      size_t j = histogram[i]; // 현재 바이트 값(i)의 빈도수를 j에 저장합니다.
      // 해당 바이트 값(i)을 빈도수(j)만큼 버퍼 p에 순차적으로 기록합니다.
      while (j-- > 0)
        *p++ = i;
    }
  // 파일 포인터를 파일의 시작으로 이동합니다.
  seek (handle, 0);
  // 정렬된 데이터(buf)를 파일에 씁니다. (실제 읽었던 size 만큼)
  write (handle, buf, size);
  // 파일을 닫습니다.
  close (handle);

  return 123; // 성공 시 특정 값(123)을 반환합니다.
}