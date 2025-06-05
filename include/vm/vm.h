#ifndef VM_VM_H
#define VM_VM_H
#include <stdbool.h>
#include "hash.h"
#include "threads/palloc.h"
#include "lib/kernel/hash.h"

enum vm_type
{
  /* page not initialized */
  /* 페이지가 초기화 되지 않음 */
  VM_UNINIT = 0,
  /* page not related to the file, aka anonymous page */
  /* 파일과 관련 없는 페이지. 즉, 익명 페이지 */
  VM_ANON = 1,
  /* page that realated to the file */
  /* 파일과 관련된 페이지 */
  VM_FILE = 2,
  /* page that hold the page cache, for project 4 */
  /* 페이지 캐시를 보유하는 페이지(프로젝트 4) */
  VM_PAGE_CACHE = 3,

  /* Bit flags to store state */
  /* 상태를 저장하기 위한 비트 플래그 */

  /* Auxillary bit flag marker for store information. You can add more
   * markers, until the value is fit in the int. */
  /* 정보 저장을 위한 보조 비트 플래그 마커.
   * int 값에 맞을 때 까지 더 많은 마커를 추가할 수 있음 */
  VM_MARKER_0 = (1 << 3),
  VM_MARKER_1 = (1 << 4),

  /* DO NOT EXCEED THIS VALUE. */
  VM_MARKER_END = (1 << 31),
};

#include "vm/uninit.h"
#include "vm/anon.h"
#include "vm/file.h"
#ifdef EFILESYS
#include "filesys/page_cache.h"
#endif

struct page_operations;
struct thread;

#define VM_TYPE(type) ((type) & 7)

/* The representation of "page".
 * This is kind of "parent class", which has four "child class"es, which are
 * uninit_page, file_page, anon_page, and page cache (project4).
 * DO NOT REMOVE/MODIFY PREDEFINED MEMBER OF THIS STRUCTURE. */
/* page의 표현
 * 이는 일종의 "부모 클래스"이며, uninit_page, file_page, anon_page,
 * 그리고 페이지 캐시(프로젝트 4)라는 4 개의 "자식 클래스"를 가짐
 * 이 구조체의 미리 정의된 멤버를 제거하거나 수정하지 말것 */
struct page
{
  const struct page_operations *operations;
  void *va;            /* Address in terms of user space, 사용자 공간 관점에서의 주소 */
  struct frame *frame; /* Back reference for frame, 프레임에 대한 역참조 */

  /* Your implementation */
  /* 아래 구현 */
  struct hash_elem hash_elem; // SPT 삽입용 elem
  bool writable;              // 페이지 쓰기 가능여부 플래그

  /* Per-type data are binded into the union.
   * Each function automatically detects the current union */
  /* 타입별 데이터는 공용체(union)에 바인딩됨
   * 각 함수는 현재 union을 자동으로 감지 */
  union
  {
    struct uninit_page uninit;
    struct anon_page anon;
    struct file_page file;
#ifdef EFILESYS
    struct page_cache page_cache;
#endif
  };
};

/* The representation of "frame" */
/* frame의 표현 */
struct frame
{
  void *kva;
  struct page *page;
};

/* The function table for page operations.
 * This is one way of implementing "interface" in C.
 * Put the table of "method" into the struct's member, and
 * call it whenever you needed. */
/* 페이지 연산을 위한 함수 테이블
 * 이는 C에서 "인터페이스"를 구현하는 한 가지 방법임
 * "메서드" 테이블을 구조체에 넣고,
 * 필요할 때마다 호출 */
struct page_operations
{
  bool (*swap_in)(struct page *, void *);
  bool (*swap_out)(struct page *);
  void (*destroy)(struct page *);
  enum vm_type type;
};

#define swap_in(page, v) (page)->operations->swap_in((page), v)
#define swap_out(page) (page)->operations->swap_out(page)
#define destroy(page) if ((page)->operations->destroy) (page)->operations->destroy(page)

/* 최대 스택 크기 1MB 제한 */
#define STACK_LIMIT (USER_STACK - (1 << 20))

/* Representation of current process's memory space.
 * We don't want to force you to obey any specific design for this struct.
 * All designs up to you for this. */
/* 현재 프로세스의 메모리 공간 표현
 * 이 구조체에 대해 특정 디자인을 따르도록 강요하지 않는다.
 * 모든 디자인은 너에게 달려있다 */
struct supplemental_page_table
{
  struct hash spt_hash; // 해당 프로세스가 관리하는 전체 페이지 정보를 담는 해시 테이블
};

#include "threads/thread.h"
void supplemental_page_table_init(struct supplemental_page_table *spt);
bool supplemental_page_table_copy(struct supplemental_page_table *dst,
                                  struct supplemental_page_table *src);
void supplemental_page_table_kill(struct supplemental_page_table *spt);
void destroy_page_entry(struct hash_elem *e, void *aux);
struct page *spt_find_page(struct supplemental_page_table *spt,
                           void *va);
bool spt_insert_page(struct supplemental_page_table *spt, struct page *page);
bool spt_remove_page(struct supplemental_page_table *spt, struct page *page);

void vm_init(void);
bool vm_try_handle_fault(struct intr_frame *f, void *addr, bool user,
                         bool write, bool not_present);

#define vm_alloc_page(type, upage, writable) \
  vm_alloc_page_with_initializer((type), (upage), (writable), NULL, NULL)
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage,
                                    bool writable, vm_initializer *init, void *aux);
void vm_dealloc_page(struct page *page);
bool vm_claim_page(void *va);
enum vm_type page_get_type(struct page *page);

#endif /* VM_VM_H */
