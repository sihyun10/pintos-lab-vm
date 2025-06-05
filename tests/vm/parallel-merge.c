/* Generates about 1 MB of random data that is then divided into
   16 chunks.  A separate subprocess sorts each chunk; the
   subprocesses run in parallel.  Then we merge the chunks and
   verify that the result is what it should be. */
/* parallel_merge라는 핵심 함수를 제공하며, 이 함수는 약 1MB의 랜덤 데이터를 생성하여 여러 청크로 나눕니다.
 * 각 청크는 별도의 자식 프로세스에 의해 병렬로 정렬됩니다.
 * 그 후, 부모 프로세스는 정렬된 청크들을 병합하고 최종 결과가 올바른지
 * (원래 데이터의 모든 요소가 정렬된 순서로 포함되어 있는지) 확인합니다.
 * 이 파일 자체는 parallel_merge 함수의 구현부이며, 다른 page-merge-xx.c  테스트 파일들이
 * 이 함수를 호출하여 특정 자식 프로세스(정렬 방식)와 예상 종료 코드를 지정하여 테스트를 실행합니다*/
// (주석은 16 청크를 언급하지만, 아래 CHUNK_CNT는 8로 정의됨)

#include "tests/vm/parallel-merge.h" // 이 파일에서 제공하는 parallel_merge 함수의 선언을 포함합니다.
#include <stdio.h>
#include <syscall.h>
#include "tests/arc4.h"
#include "tests/lib.h"
#include "tests/main.h"

#define CHUNK_SIZE (128 * 1024)                 // 각 청크의 크기를 128KB로 정의합니다.
#define CHUNK_CNT 8                             /* Number of chunks. */ // 청크의 수를 8로 정의합니다.
#define DATA_SIZE (CHUNK_CNT * CHUNK_SIZE)      /* Buffer size. */    // 전체 데이터 버퍼 크기를 계산합니다 (8 * 128KB = 1MB).

unsigned char buf1[DATA_SIZE], buf2[DATA_SIZE]; // 원본/정렬된 청크용 버퍼(buf1)와 최종 병합된 결과용 버퍼(buf2)를 선언합니다.
size_t histogram[256];                          // 데이터의 바이트 값 빈도수를 저장할 히스토그램입니다 (검증용).

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
    histogram[buf1[i]]++;             // 각 바이트 값의 빈도수를 히스토그램에 기록합니다.
}

/* Sort each chunk of buf1 using SUBPROCESS,
   which is expected to return EXIT_STATUS. */
// SUBPROCESS를 사용하여 buf1의 각 청크를 정렬하며,
// SUBPROCESS는 EXIT_STATUS를 반환할 것으로 예상됩니다.
static void
sort_chunks (const char *subprocess, int exit_status) // 청크 정렬 함수입니다. 자식 프로세스 이름과 예상 종료 코드를 인자로 받습니다.
{
  pid_t children[CHUNK_CNT]; // 자식 프로세스들의 ID를 저장할 배열입니다.
  size_t i;                  // 반복문 인덱스입니다.

  for (i = 0; i < CHUNK_CNT; i++) // 각 청크에 대해 반복합니다.
    {
      char fn[128];  // 각 청크 데이터를 저장할 파일 이름을 만들 버퍼입니다.
      char cmd[128]; // 자식 프로세스 실행 명령 문자열을 만들 버퍼입니다.
      int handle;    // 파일 핸들을 저장할 변수입니다.

      msg ("sort chunk %zu", i); // 현재 정렬할 청크 번호를 출력합니다.

      /* Write this chunk to a file. */
      // 이 청크를 파일에 씁니다.
      snprintf (fn, sizeof fn, "buf%zu", i); // 파일 이름을 "buf0", "buf1", ... 형식으로 만듭니다.
      create (fn, CHUNK_SIZE);                // 해당 이름으로 청크 크기만큼의 파일을 생성합니다.
      quiet = true; // 메시지 출력을 잠시 억제합니다.
      CHECK ((handle = open (fn)) > 1, "open \"%s\"", fn); // 생성된 파일을 엽니다.
      // 현재 청크(buf1 + CHUNK_SIZE * i)를 해당 파일에 씁니다.
      write (handle, buf1 + CHUNK_SIZE * i, CHUNK_SIZE);
      close (handle); // 파일을 닫습니다.

      /* Sort with subprocess. */
      // 하위 프로세스로 정렬합니다.
      snprintf (cmd, sizeof cmd, "%s %s", subprocess, fn); // 실행할 명령 문자열을 만듭니다 (예: "child-sort buf0").
      children[i] = fork (subprocess); // `subprocess` 이름으로 fork하여 자식 프로세스 ID를 저장합니다.
      if (children[i] == 0) // 자식 프로세스인 경우
        // 만들어진 명령(cmd)으로 자식 프로세스를 실행(exec)합니다. exec 실패 여부를 CHECK 합니다.
        CHECK ((children[i] = exec (cmd)) != -1, "exec \"%s\"", cmd);
      quiet = false; // 부모 프로세스에서 메시지 출력을 다시 활성화합니다.
    }

  for (i = 0; i < CHUNK_CNT; i++) // 모든 자식 프로세스에 대해 반복합니다.
    {
      char fn[128]; // 파일 이름을 만들 버퍼입니다.
      int handle;   // 파일 핸들을 저장할 변수입니다.

      // i번째 자식 프로세스(children[i])가 종료될 때까지 기다리고, 반환된 종료 코드가 exit_status와 같은지 CHECK 합니다.
      CHECK (wait (children[i]) == exit_status, "wait for child %zu", i);

      /* Read chunk back from file. */
      // 파일에서 정렬된 청크를 다시 읽습니다.
      quiet = true; // 메시지 출력을 잠시 억제합니다.
      snprintf (fn, sizeof fn, "buf%zu", i); // 파일 이름을 다시 만듭니다.
      CHECK ((handle = open (fn)) > 1, "open \"%s\"", fn); // 파일을 엽니다.
      // 정렬된 파일 내용을 원래 청크 위치(buf1 + CHUNK_SIZE * i)로 다시 읽어들입니다.
      read (handle, buf1 + CHUNK_SIZE * i, CHUNK_SIZE);
      close (handle); // 파일을 닫습니다.
      quiet = false; // 메시지 출력을 다시 활성화합니다.
    }
}

/* Merge the sorted chunks in buf1 into a fully sorted buf2. */
// buf1의 정렬된 청크들을 완전히 정렬된 buf2로 병합합니다.
static void
merge (void) // 병합 함수입니다.
{
    unsigned char *mp[CHUNK_CNT]; // 각 청크의 현재 병합 위치를 가리키는 포인터 배열입니다.
    size_t mp_left;               // 아직 병합할 데이터가 남은 청크의 수입니다.
    unsigned char *op;            // buf2에 데이터를 쓸 위치를 가리
}