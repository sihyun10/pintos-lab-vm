/* Generates about 1 MB of random data that is then divided into
   16 chunks.  A separate subprocess sorts each chunk; the
   subprocesses run in parallel.  Then we merge the chunks and
   verify that the result is what it should be. */
// 약 1MB의 랜덤 데이터를 생성하여 16개의 청크로 나눕니다.
// 각 청크는 별도의 하위 프로세스에 의해 정렬되며, 하위 프로세스들은
// 병렬로 실행됩니다. 그런 다음 청크들을 병합하고 결과가
// 올바른지 확인합니다.
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
  unsigned char *op;            // buf2에 데이터를 쓸 위치를 가리키는 포인터입니다.
  size_t i;                     // 반복문 인덱스입니다.

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

// 다른 테스트 파일에서 호출될 주 함수입니다.
// child_name: 자식 프로세스로 실행할 정렬 프로그램 이름.
// exit_status: 해당 자식 프로세스의 예상 성공 종료 코드.
void
parallel_merge (const char *child_name, int exit_status)
{
  init (); // 데이터 초기화 함수를 호출합니다.
  sort_chunks (child_name, exit_status); // 청크 정렬 함수를 호출합니다.
  merge (); // 병합 함수를 호출합니다.
  verify (); // 검증 함수를 호출합니다.
}