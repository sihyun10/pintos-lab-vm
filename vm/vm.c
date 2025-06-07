//
/* vm.c: Generic interface for virtual memory objects. */
/* vm.c: 가상 메모리 객체를 위한 일반 인터페이스. */
#define VM // thread 구조체 내부 defif VM의 spt에 접근하기 위한 선언 (vm.c는 VM으로 동작)
#include <stdio.h>
#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/thread.h" // 안전하게 struct thread 내부 구조 접근을 위한 선언
#include "threads/mmu.h"

unsigned page_hash(struct hash_elem *e, void *aux UNUSED);
bool page_less(struct hash_elem *a, struct hash_elem *b, void *aux UNUSED);

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
	switch (ty)
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
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
									vm_initializer *init, void *aux)
{

	ASSERT(VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current()->spt;
	/* Check wheter the upage is already occupied or not. */
	// 이미지가 이미 점유되어 있는지 확인하세요.
	struct page *page = spt_find_page(spt, upage);
	if (page == NULL)
	{
		
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		// 해야될 것: 페이지를 생성하고, VM 유형에 따라 초기화 파일을 가져온 후, uninit_new를 호출하여 "uninit" 페이지 구조체를 생성합니다.
		// 해야될 것: uninit_new를 호출한 후 필드를 수정해야 합니다.

		/* 페이지 하나 생성한다. vm_alloc_page를 이용해서*/
		struct page *page = malloc(sizeof(struct page));
		/* 타입별로 initializer 방식이 다르기 때문에, 그걸 핸들링 해주게 선언해준다. */
		bool (*initializer)(struct page *, enum vm_type, void *kva);
		/* 페이지를 초기화 해주는데, 인자를 어떻게 넘길지 모르겠다. 그냥 anonymous
		* 에 맞춰서 넘겨주면 이렇게 되지 않을까 싶고, Gitbook에도 이런식으로 설명이 나옴
		* 더 공부해서 적당한 인자값 넘겨야 함.*/
		if(VM_TYPE(type) == VM_ANON){
			initializer = anon_initializer;
		} else if(VM_TYPE(type) == VM_FILE){
			initializer = file_backed_initializer;
		}


		
		uninit_new(page, upage, init, 
			type, aux, initializer);
		page->writable = writable;

		/* TODO: Insert the page into the spt. */
		// 해당 페이지를 spt에 삽입합니다.

		/* pml4에 값을 넣긴 넣는데, 실패하면 false*/
		if(!spt_insert_page(spt, page)){
			goto err;
		}
	}
	return true;
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
// spt에서 VA를 찾아 페이지를 반환합니다. 오류가 발생하면 NULL을 반환합니다.
struct page *
spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED)
{
	// struct page p;
	// struct hash_elem *e;

	// p.va = pg_round_down(va);
	// e = hash_find (&spt->spt_hash, &p.hash_elem); 
	// return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
	// struct page p;
	// struct hash_elem *e;

	// p.va = pg_round_down(va);
	// e = hash_find (&spt->spt_hash, &p.hash_elem); 
	// return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
	struct page dummy_page;
	dummy_page.va = pg_round_down(va);
	
	struct hash_elem *e = hash_find(&spt->spt_hash, &dummy_page.hash_elem);
	if (e == NULL)
		return NULL;

	return hash_entry(e, struct page, hash_elem);
}

/* Insert PAGE into spt with validation. */
// 검증을 통해 spt에 PAGE를 삽입합니다.
bool spt_insert_page(struct supplemental_page_table *spt UNUSED,
					 struct page *page UNUSED)
{
	  int succ = false;

	  if (is_user_vaddr(page->va))
	  {
	    if (spt_find_page(spt, page->va) == NULL)
	    {
	      hash_insert(&spt->spt_hash, &page->hash_elem);
	      succ = true;
	    }
	  }

	  return succ;
}

bool spt_remove_page(struct supplemental_page_table *spt, struct page *page)
{
	// spt에서 해당 페이지가 있는지 가져온다.
	struct hash_elem *e = hash_find(&spt->spt_hash, &page->hash_elem);
	// 없으면, false를 반환한다.
	if(e == NULL){
		return false;
	}
	// 있으면, spt에서 가져온 테이블의 값인 e를 지운다.
	hash_delete(&spt->spt_hash, e);
	// 페이지를 해제한다.
	vm_dealloc_page(page);
	return true; // 왜 넣은걸까요?
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
	/* TODO: Fill this function. */
	/* 사용자 풀의 영역을 할당 받을 주소를 저장한다. 할당 받는다고 해서 받는게 아니라
	 * 그냥 시작 주소만 알려줘서 사용해라 라는것임, 이해하기 쉽게 그냥 할당이라고 얘기함*/
	uint64_t *kva = palloc_get_multiple(PAL_USER, 1);
	/* 할당에 실패하게 된다면, kernel 패닉 던지는데, swap out 구현하면 해결될 것임
	 * 그땐 오류 내용 바꿔야 함*/
	if(kva == NULL){
		PANIC(" No Memory, TODO implement Swap Out");
	}
	/* 물리 메모리에 할당하기 위한 공간을 담아놓은 곳을 frame안에 저장 하기 위해 kva와 담긴 page를 저장할
	 * 총 16바이트의 크기를 할당 받는다.*/
	struct frame *frame = (struct frame *)malloc(sizeof(struct frame));
	/* kva 값을 저장하고, page는 채워 넣어진게 아니기 때문에 null로 초기화*/
	frame->kva = kva;
	frame->page = NULL;

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
	// struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
	// struct page *page = NULL;
	// /* TODO: Validate the fault */
	// // 오류를 검증하세요
	// /* TODO: Your code goes here */
	// // 코드를 여기에 적으세요
	// return vm_do_claim_page(page);
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = spt_find_page(spt, addr);
	/* TODO: Validate the fault */
	if (page == NULL) {
		return false ; 
	}
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
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
bool vm_claim_page(void *va UNUSED)
{
	struct thread *curr = thread_current();
	struct page *page = spt_find_page(&curr->spt, va);

	if(page == NULL)
		return false;
	
	/* TODO: Fill this function */
	// 기능을 구현하세요.
	
	return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
// PAGE를 선언하고 mmu를 설정하세요.
static bool
vm_do_claim_page(struct page *page)
{
	struct frame *frame = vm_get_frame();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	// 페이지의 VA를 프레임의 PA에 매핑하기 위해 페이지 테이블 항목을 삽입합니다.

	/* 현재 스레드를 불러오는데, mmu세팅을 위해 pml4를 불러와 추가해줘야 하기 때문이다.*/
	struct thread *curr = thread_current();
	
	/* pml4_set_page 함수를 부럴와서 pml4에 페이지의 가상 주소에, 커널 가상 주소를 올리고,
	 * 현재 페이지가 kernel pool에 올라왔기에 writable을 불러와서 값을 수정할 수 있게 해준다.*/
	if(!pml4_set_page(curr->pml4, page->va, frame->kva, page->writable))
		return false;
	return swap_in(page, frame->kva);
}

/* Initialize new supplemental page table */
// 새로운 보충 페이지 테이블을 초기화합니다.
void supplemental_page_table_init(struct supplemental_page_table *spt)
{
	if (!hash_init(&spt->spt_hash, page_hash, page_less, NULL))
	{
		PANIC("Failed SPT Table Init");
	}
}

/* Copy supplemental page table from src to dst */
// src에서 dst로 보충 페이지 테이블 복사합니다.
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED,
								  struct supplemental_page_table *src UNUSED)
{
	/* hash_iterator 생성하여, src의 머리를 가리키게 하여 초기화 */
	struct hash_iterator i;
	hash_first(&i, src);

	/* iterator가 전부 빌때까지 */
	while(hash_next(&i)){
		/* 페이지를 가져와서 페이지에 대한 추후 필요할 내용들을 복사한다.
		 * 복사된 항목
		 * type
		 * upage(va)
		 * wrtiable */
		struct page *src_page = hash_entry(hash_cur(&i), struct page, hash_elem);
		/* 장래희망 타입을 저장한다. */
		enum vm_type hope_type = VM_TYPE(src_page->uninit.type); 
		/* 현재 페이지의 타입을 저장한다. */
		enum vm_type real_type = page_get_type(src_page);
		void *upage = src_page->va;
		bool writable = src_page->writable; 

		/* 만약, VM_UNINIT일 경우, 페이지를 새로 생성해줘야 한다.*/
		if(real_type = VM_UNINIT){			
			vm_initializer *init = src_page->uninit.init;
			if(!vm_alloc_page_with_initializer(hope_type, upage, writable, init, NULL)){
				return false;
			}
		}
		/* 그 외의 경우, 페이지를 새로 생성하고, 프레임을 만들어주고 내부 내용을 복사한다.*/
		else{
			if(!vm_alloc_page(hope_type, upage, writable)){
				return false;
			}
			if(!vm_claim_page(upage)){
				return false;
			}

			struct page *dst_page = spt_find_page(dst, upage);
			memcpy(dst_page->frame->kva, src_page->frame->kva, PGSIZE);
		}
	}
	return true;
}

void page_destroy(struct hash_elem *e, void *aux UNUSED)
{
	struct page *p = hash_entry(e, struct page, hash_elem);

	if (p->operations && p->operations->destroy)
		destroy(p);
	free(p);
}

/* Free the resource hold by the supplemental page table */
// 보충 페이지 테이블에서 리소스 보류를 해제합니다.
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED)
{
	// hash_destroy(&spt->spt_hash, page_destroy);
}

/* 페이지를 해시값으로 가져오기 위한 함수 */
unsigned page_hash(struct hash_elem *e, void *aux UNUSED)
{
	// e 포인터로부터 그걸 포함하고 있는 struct page 구조체의 시작 주소를 구하는 것
	const struct page *p = hash_entry(e, struct page, hash_elem);
	// p->va값을 해시값으로 가져오겠다는 뜻
	return hash_bytes(&p->va, sizeof(p->va));
}

/* 값의 크기를 비교하는 것(해시값이 같을 때, 같은 버킷일 경우 어떤 항목으로 오름차순, 내림차순 할지를 결정)*/
bool page_less(struct hash_elem *a,
				struct hash_elem *b,
				void *aux UNUSED)
{
	struct page *pa = hash_entry(a, struct page, hash_elem);
	struct page *pb = hash_entry(b, struct page, hash_elem);
	// printf("call page_less fuc\n");
	return pa->va < pb->va;
}

// /* 페이지를 해시값으로 가져오기 위한 함수 */
// static unsigned page_hash(const struct hash_elem *e, void *aux UNUSED)
// {
// 	// e 포인터로부터 그걸 포함하고 있는 struct page 구조체의 시작 주소를 구하는 것
// 	const struct page *p = hash_entry(e, struct page, hash_elem);
// 	// p->va값을 해시값으로 가져오겠다는 뜻
// 	return hash_bytes(&p->va, sizeof(p->va));
// }

// /* 값의 크기를 비교하는 것(해시값이 같을 때, 같은 버킷일 경우 어떤 항목으로 오름차순, 내림차순 할지를 결정)*/
// static bool page_less(const struct hash_elem *a,
// 					  const struct hash_elem *b,
// 					  void *aux UNUSED)
// {
// 	const struct page *pa = hash_entry(a, struct page, hash_elem);
// 	const struct page *pb = hash_entry(b, struct page, hash_elem);

// 	return pa->va < pb->va;
// }
