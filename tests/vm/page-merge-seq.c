/* Generates about 1 MB of random data that is then divided into
   16 chunks.  A separate subprocess sorts each chunk in
   sequence.  Then we merge the chunks and verify that the result
   is what it should be. */
/* 약 1MB의 랜덤 데이터를 생성하여 16개의 청크로 나눕니다.
 * 각 청크는 별도의 자식 프로세스(child-sort)에 의해 순차적으로 정렬됩니다.
 * 그 후, 부모 프로세스는 정렬된 청크들을 병합하고 최종 결과가 올바른지
 * (원래 데이터의 모든 요소가 정렬된 순서로 포함되어 있는지) 확인합니다. */

#include <syscall.h>
#include "tests/arc4.h"
#include "tests/lib.h"
#include "tests/main.h"

/* This is the max file size for an older version of the Pintos
   file system that had 126 direct blocks each pointing to a
   single disk sector.  We could raise it now. */
// 이것은 각 디스크 섹터를 가리키는 126개의 직접 블록을 가졌던
// 이전 버전 Pintos 파일 시스템의 최대 파일 크기입니다. 지금은 늘릴 수 있습니다.
#define CHUNK_SIZE (126 * 512)                  // 청크 크기를 정의합니다 (126 * 512 바이트).
#define CHUNK_CNT 16                            /* Number of chunks. */ // 청크의 수 (16개).
#define DATA_SIZE (CHUNK_CNT * CHUNK_SIZE)      /* Buffer size. */    // 전체 데이터 버퍼 크기 (16 * CHUNK_SIZE).

unsigned char buf1[DATA_SIZE], buf2[DATA_SIZE]; // 원본/정렬된 청크용 버퍼(buf1)와 최종 병합된 결과용 버퍼(buf2)를 선언합니다.
size_t histogram[256];                          // 데이터의 바이트 값 빈도수를 저장할 히스토그램입니다.

/* Initialize buf1 with random data,
   then count the number of instances of each value within it. */
// buf1을 랜덤 데이터로 초기화한 다음,
// 그 안의 각 값의 발생 횟수를 셉니다.
static void
init (void) // 데이터 초기화 함수입니다.
{
  struct arc4 arc4; // ARC4 암호화(여기서는 랜덤 데이터 생성용) 상태 구조체입니다.
  size_t i;         // 반복문 인덱스입니다.

  msg ("init"); // "init" 메시지를 출력합니다.

  arc4_init (&arc4, "foobar", 6);      // ARC4를 "foobar" 키로 초기화합니다.
  arc4_crypt (&arc4, buf1, sizeof buf1); // buf1을 ARC4로 암호화하여 랜덤 데이터처럼 만듭니다.
  for (i = 0; i < sizeof buf1; i++)   // buf1의 모든 바이트를 순회합니다.
    histogram[buf1[i]]++;             // 각 바이트 값의 빈도수를 히스토그램에 기록합니다 (검증용).
}

/* Sort each chunk of buf1 using a subprocess. */
// 하위 프로세스를 사용하여 buf1의 각 청크를 정렬합니다.
static void
sort_chunks (void) // 청크 정렬 함수입니다.
{
  size_t i; // 반복문 인덱스입니다.

  create ("buffer", CHUNK_SIZE); // 자식 프로세스가 사용할 임시 파일 "buffer"를 CHUNK_SIZE로 생성합니다.
  for (i = 0; i < CHUNK_CNT; i++) // 각 청크에 대해 반복합니다.
    {
      pid_t child; // 자식 프로세스 ID를 저장할 변수입니다.
      int handle;  // 파일 핸들을 저장할 변수입니다.

      msg ("sort chunk %zu", i); // 현재 정렬할 청크 번호를 출력합니다.

      /* Write this chunk to a file. */
      // 이 청크를 파일에 씁니다.
      quiet = true; // 메시지 출력을 잠시 억제합니다.
      // "buffer" 파일을 엽니다. 성공 여부를 CHECK 합니다.
      CHECK ((handle = open ("buffer")) > 1, "open \"buffer\"");
      // 현재 청크(buf1 + CHUNK_SIZE * i)를 "buffer" 파일에 씁니다.
      write (handle, buf1 + CHUNK_SIZE * i, CHUNK_SIZE);
      close (handle); // 파일을 닫습니다.

      /* Sort with subprocess. */
      // 하위 프로세스로 정렬합니다.
      child = fork("child-sort"); // "child-sort"라는 이름으로 자식 프로세스를 fork 합니다.
      // 아래는 자식/부모 프로세스 분기입니다.
			if (child == 0) { // 자식 프로세스인 경우
				quiet = false; // 자식 프로세스 메시지 출력을 활성화합니다.
				msg ("child[%zu] exec", i); // 자식 실행 메시지를 출력합니다.
        // "child-sort buffer"를 실행하여 "buffer" 파일 내용을 정렬합니다.
				if (exec ("child-sort buffer") == -1)
					fail ("child[%zu] exec fail", i); // exec 실패 시 테스트 실패를 알립니다.
				quiet = true; // 자식 프로세스 메시지 출력을 다시 억제합니다. (실제로는 exec 후 실행 안됨)
			} else { // 부모 프로세스인 경우
				quiet = false; // 부모 프로세스 메시지 출력을 활성화합니다.
        // 자식 프로세스가 종료될 때까지 기다립니다. 종료 코드가 123인지 확인합니다.
				if (wait (child) != 123)
					fail ("child[%zu] wait fail", i); // 예상 종료 코드가 아니면 테스트 실패를 알립니다.
				msg ("child[%zu] wait success", i); // 자식 대기 성공 메시지를 출력합니다.
				quiet = true; // 부모 프로세스 메시지 출력을 다시 억제합니다.

				/* Read chunk back from file. */
        // 파일에서 정렬된 청크를 다시 읽습니다.
        // "buffer" 파일을 다시 엽니다. 성공 여부를 CHECK 합니다.
				CHECK ((handle = open ("buffer")) > 1, "open \"buffer\"");
        // 정렬된 파일 내용을 원래 청크 위치(buf1 + CHUNK_SIZE * i)로 다시 읽어들입니다.
				read (handle, buf1 + CHUNK_SIZE * i, CHUNK_SIZE);
				close (handle); // 파일을 닫습니다.

				quiet = false; // 부모 프로세스 메시지 출력을 다시 활성화합니다. (다음 반복을 위해)
			}
    }
}

/* Merge the sorted chunks in buf1 into a fully sorted buf2. */
// buf1의 정렬된 청크들을 완전히 정렬된 buf2로 병합합니다.
static void
merge (void) // 병합 함수입니다.
{
  // 각 청크의 현재 병합 위치를 가리키는 포인터 배열입니다.
  unsigned char *mp[CHUNK_CNT];
  size_t mp_left;             // 아직 병합할 데이터가 남은 청크의 수입니다.
  unsigned char *op;          // buf2에 데이터를 쓸 위치를 가리키는 포인터입니다.
  size_t i;                   // 반복문 인덱스입니다.

  msg ("merge"); // "merge" 메시지를 출력합니다.

  /* Initialize merge pointers. */
  // 병합 포인터를 초기화합니다.
  mp_left = CHUNK_CNT; // 남은 청크 수를 초기화합니다.
  for (i = 0; i < CHUNK_CNT; i++) // 각 청크에 대해
    mp[i] = buf1 + CHUNK_SIZE * i; // 해당 청크의 시작 주소로 포인터를 설정합니다.

  /* Merge. */
  // 병합합니다.
  op = buf2; // 출력 포인터를 buf2의 시작으로 설정합니다.
  while (mp_left > 0) // 병합할 청크가 남아있는 동안 반복합니다.
    {
      /* Find smallest value. */
      // 가장 작은 값을 찾습니다.
      size_t min = 0; // 가장 작은 값을 가진 청크의 인덱스를 저장합니다.
      // 현재 남아있는 청크들 중에서 (mp[0] ~ mp[mp_left-1])
      for (i = 1; i < mp_left; i++)
        if (*mp[i] < *mp[min]) // 현재 최소값보다 더 작은 값을 찾으면
          min = i;             // min 인덱스를 갱신합니다.

      /* Append value to buf2. */
      // 찾은 가장 작은 값을 buf2에 추가합니다.
      *op++ = *mp[min];

      /* Advance merge pointer.
         Delete this chunk from the set if it's emptied. */
      // 병합 포인터를 진행시킵니다.
      // 이 청크가 비워졌으면 집합에서 삭제합니다.
      // 해당 청크의 포인터(mp[min])를 1 증가시키고,
      if ((++mp[min] - buf1) % CHUNK_SIZE == 0) // 만약 해당 청크의 끝에 도달했다면
        mp[min] = mp[--mp_left]; // 해당 청크를 유효한 마지막 청크로 대체하고 남은 청크 수를 줄입니다.
    }
}

// 최종 정렬된 결과(buf2)가 올바른지(원래 데이터의 히스토그램과 일치하는지) 확인합니다.
static void
verify (void) // 검증 함수입니다.
{
  size_t buf_idx;  // buf2를 순회하기 위한 인덱스입니다.
  size_t hist_idx; // 히스토그램을 순회하기 위한 인덱스입니다 (0-255 바이트 값).

  msg ("verify"); // "verify" 메시지를 출력합니다.

  buf_idx = 0; // buf2 인덱스를 0으로 초기화합니다.
  // 히스토그램의 모든 바이트 값(0부터 255까지)에 대해 반복합니다.
  for (hist_idx = 0; hist_idx < sizeof histogram / sizeof *histogram;
       hist_idx++)
    {
      // 현재 바이트 값(hist_idx)이 원래 데이터에 있었던 횟수(histogram[hist_idx])만큼 반복합니다.
      while (histogram[hist_idx]-- > 0)
        {
          // buf2의 현재 위치(buf_idx)에 있는 값이 예상되는 바이트 값(hist_idx)과 다르면
          if (buf2[buf_idx] != hist_idx)
            fail ("bad value %d in byte %zu", buf2[buf_idx], buf_idx); // 테스트 실패를 알립니다.
          buf_idx++; // buf2 인덱스를 증가시킵니다.
        }
    }

  // 모든 검증이 통과하고 buf_idx가 원래 데이터 크기와 같으면 성공입니다.
  msg ("success, buf_idx=%'zu", buf_idx);
}

void
test_main (void) // 테스트의 메인 함수입니다.
{
  init ();        // 데이터 초기화 함수를 호출합니다.
  sort_chunks (); // 청크 정렬 함수를 호출합니다.
  merge ();       // 병합 함수를 호출합니다.
  verify ();      // 검증 함수를 호출합니다.
}