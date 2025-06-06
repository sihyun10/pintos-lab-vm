/* vm.c: Generic interface for virtual memory objects. */
/* vm.c: 가상 메모리 객체를 위한 일반 인터페이스. */
#define VM // thread 구조체 내부 defif VM의 spt에 접근하기 위한 선언 (vm.c는 VM으로 동작)
#include "threads/malloc.h"
#include "vm/vm.h"

#include "stdio.h"
#include "vm/inspect.h"
#include "threads/thread.h" // 안전하게 struct thread 내부 구조 접근을 위한 선언
#include "threads/mmu.h"
#include "userprog/process.h"

static unsigned page_hash(const struct hash_elem *e, void *aux UNUSED);
static bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED);

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
// 각 하위 시스템의 초기화 코드를 호출하여 가상 메모리 하위 시스템을 초기화합니다.
void vm_init(void)
{
    vm_anon_init();
    vm_file_init();
#ifdef EFILESYS /* For project 4 */
  pagecache_init();
#endif
    register_inspect_intr();
    /* DO NOT MODIFY UPPER LINES. */
    // ※ 위의 라인은 수정하지마세요. ※
    /* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
// 페이지 유형을 가져옵니다. 이 함수는 페이지가 초기화된 후 페이지의 유형을 알고 싶을 때 유용합니다.
// 이 함수는 현재 완전히 구현되었습니다.
enum vm_type
page_get_type(struct page *page)
{
    int ty = VM_TYPE(page->operations->type);
    switch(ty)
    {
    case VM_UNINIT:
        return VM_TYPE(page->uninit.type);
    default:
        return ty;
    }
}

/* Helpers */
static struct frame *vm_get_victim(void);
static bool vm_do_claim_page(struct page *page);
static struct frame *vm_evict_frame(void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
// 초기화 함수를 사용하여 보류 중인 페이지 객체를 생성합니다.
// 페이지를 생성하려면 직접 생성하지 말고 이 함수나 `vm_alloc_page`를 통해 생성하세요.
/* type : VM_UNINIT, VM_ANON, VM_FILE
 * upage : 사용자 가상 주소
 * writable : 쓰기 가능 여부
 * init : unint 단계에서 호출할 initializer (ex : anon_initializer)
 * aux : initializer에 전달할 추가 인수 (ex : file offset struct) */
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
                                    vm_initializer *init, void *aux)
{
    ASSERT(VM_TYPE(type) != VM_UNINIT)

    struct supplemental_page_table *spt = &thread_current()->spt;
    struct page *page;

    if (!is_user_vaddr(upage))
        goto err;
    /* Check wheter the upage is already occupied or not. */
    // 이미지가 이미 점유되어 있는지 확인하세요.
    if(spt_find_page(spt, upage) == NULL)
    {
        /* TODO: Create the page, fetch the initialier according to the VM type,
         * TODO: and then create "uninit" page struct by calling uninit_new. You
         * TODO: should modify the field after calling the uninit_new. */
        // 해야될 것: 페이지를 생성하고, VM 유형에 따라 초기화 파일을 가져온 후, uninit_new를 호출하여 "uninit" 페이지 구조체를 생성합니다.
        // 해야될 것: uninit_new를 호출한 후 필드를 수정해야 합니다.
        page = malloc(sizeof *page);
        if(page == NULL)
        {
            printf("📃vm_alloc_page_with_initializer: malloc for page failed\n");
            goto err;
        }

        /* uninit_new 내부에서는 새로 할당한 page 내부에 unitit 인자 초기화,  */
        uninit_new(page, upage, NULL, VM_UNINIT, NULL, uninit_initialize);
        page->writable = writable;

        /* TODO: Insert the page into the spt. */
        // 해당 페이지를 spt에 삽입합니다.
        bool success = spt_insert_page(spt, page);
        if (!success)
        {
            free(page);
            printf("📩vm_alloc_page_with_init spt_insert_page failed\n");
            goto err;
        }
        return true;
    }
    goto err;
err:
    return false;
}

/* Find VA from spt and return page. On error, return NULL. */
// spt에서 VA를 찾아 페이지를 반환합니다. 오류가 발생하면 NULL을 반환합니다.
struct page *
spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED)
{
    struct page dummy_page;
    dummy_page.va = pg_round_down(va);

    struct hash_elem *e = hash_find(&spt->spt_hash, &dummy_page.hash_elem);
    if(e == NULL)
        return NULL;

    return hash_entry(e, struct page, hash_elem);
}

/* Insert PAGE into spt with validation. */
// 검증을 통해 spt에 PAGE를 삽입합니다.
bool spt_insert_page(struct supplemental_page_table *spt UNUSED,
                     struct page *page UNUSED)
{
    int succ = false;

    if(is_user_vaddr(page->va))
    {
        if(spt_find_page(spt, page->va) == NULL)
        {
            hash_insert(&spt->spt_hash, &page->hash_elem);
            succ = true;
        }
    }

    return succ;
}

void spt_remove_page(struct supplemental_page_table *spt, struct page *page)
{
    vm_dealloc_page(page);
    return true;
}

/* Get the struct frame, that will be evicted. */
// 내보낼 구조체 프레임을 가져옵니다.
static struct frame *
vm_get_victim(void)
{
    struct frame *victim = NULL;
    /* TODO: The policy for eviction is up to you. */
    // 내보내는 규칙은 본인이 정하세요.

    return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
// 한 페이지를 제거하고 해당 프레임을 반환합니다.
// 오류 발생 시 NULL을 반환합니다.
static struct frame *
vm_evict_frame(void)
{
    struct frame *victim UNUSED = vm_get_victim();
    /* TODO: swap out the victim and return the evicted frame. */

    return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
// palloc() 함수는 프레임을 가져옵니다. 사용 가능한 페이지가 없으면 해당 페이지를 제거하고 반환합니다.
// 이 함수는 항상 유효한 주소를 반환합니다.
// 즉, 사용자 풀 메모리가 가득 차면 이 함수는 프레임을 제거하여 사용 가능한 메모리 공간을 가져옵니다.
static struct frame *
vm_get_frame(void)
{
    struct frame *frame = NULL;
    /* TODO: Fill this function. */
    void *kva = palloc_get_page(PAL_USER); // 물리 페이지(커널 가상 주소)를 가져옴
    if(kva == NULL) // 이후 get victim page -> swap out
    {
        PANIC("no space in userPool");
    }
    frame = malloc(sizeof *frame); // kva가 확보되면, malloc으로 frame 공간 할당
    if(frame == NULL)
        PANIC("out of memory");
    frame->kva = kva;
    frame->page = NULL; // 아직 어떤 struct page에도 매핑 되지 않음

    ASSERT(frame != NULL);
    ASSERT(frame->page == NULL);
    return frame;
}

/* Growing the stack. */
// 스택을 키웁니다.
static void
vm_stack_growth(void *addr UNUSED)
{
}

/* Handle the fault on write_protected page */
// write_protected 페이지에서 오류를 처리합니다.
static bool
vm_handle_wp(struct page *page UNUSED)
{
}

/* Return true on success */
// 성공 시 true를 반환합니다.
bool vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED,
                         bool user UNUSED, bool write UNUSED, bool not_present UNUSED)
{
    struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
    struct page *page = NULL;
    /* TODO: Validate the fault */
    // 오류를 검증하세요
    /* TODO: Your code goes here */
    // 코드를 여기에 적으세요

    return vm_do_claim_page(page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
// 페이지를 비웁니다.
// ※ 해당 함수는 수정하지 마세요! ※
void vm_dealloc_page(struct page *page)
{
    destroy(page);
    free(page);
}

/* Claim the page that allocate on VA. */
// VA로 할당된 페이지를 선언합니다.
// TODO: frame management in memory management
bool vm_claim_page(void *va UNUSED)
{
    struct page *page = NULL;
    /* TODO: Fill this function */
    // 기능을 구현하세요.
    page = spt_find_page(&thread_current()->spt, va); // va 주소를 가진 struct page를 spt 내부에서 찾음
    if(page == NULL)
        return false;
    return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
// PAGE를 선언하고 mmu를 설정하세요.
// TODO: frame management in memory management
static bool
vm_do_claim_page(struct page *page)
{
    struct frame *frame = vm_get_frame();
    ASSERT(frame != NULL);
    ASSERT(frame->page == NULL);

    /* Set links */
    /* 양방향 링킹 구현(기존 제공 코드)*/
    frame->page = page;
    page->frame = frame;

    /* TODO: Insert page table entry to map page's VA to frame's PA. */
    // 페이지의 VA를 프레임의 PA에 매핑하기 위해 페이지 테이블 항목을 삽입합니다.
    if (!install_page(page->va, frame->kva, page->writable))
        return false;
    return swap_in(page, frame->kva);
}

/* Initialize new supplemental page table */
// 새로운 보충 페이지 테이블을 초기화합니다.
void
supplemental_page_table_init(struct supplemental_page_table *spt)
{
    if(!hash_init(&spt->spt_hash, page_hash, page_less, NULL))
    { // hash init 호출로 SPT 초기화
        PANIC("Failed SPT Table Init");
    }
}

/* Copy supplemental page table from src to dst */
// src에서 dst로 보충 페이지 테이블 복사합니다.
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED,
                                  struct supplemental_page_table *src UNUSED)
{
}

/* Free the resource hold by the supplemental page table */
// 보충 페이지 테이블에서 리소스 보류를 해제합니다.
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED)
{
    /* TODO: Destroy all the supplemental_page_table hold by thread and
     * TODO: writeback all the modified contents to the storage. */
    // 스레드가 보유한 supplemental_page_table을 모두 파괴하고 수정된 내용을 모두 저장소에 다시 쓰게 구현하세요.
}

/* 페이지를 해시값으로 가져오기 위한 함수 */
static unsigned page_hash(const struct hash_elem *e, void *aux UNUSED)
{
    // e 포인터로부터 그걸 포함하고 있는 struct page 구조체의 시작 주소를 구하는 것
    const struct page *p = hash_entry(e, struct page, hash_elem);
    // p->va값을 해시값으로 가져오겠다는 뜻
    return hash_bytes(&p->va, sizeof(p->va));
}

/* 값의 크기를 비교하는 것(해시값이 같을 때, 같은 버킷일 경우 어떤 항목으로 오름차순, 내림차순 할지를 결정)*/
static bool page_less(const struct hash_elem *a,
                      const struct hash_elem *b,
                      void *aux UNUSED)
{
    const struct page *pa = hash_entry(a, struct page, hash_elem);
    const struct page *pb = hash_entry(b, struct page, hash_elem);

    return pa->va < pb->va;
}
