/* anon.c: Implementation of page for non-disk image (a.k.a. anonymous page). */

#include "vm/vm.h"
#include "devices/disk.h"

/* DO NOT MODIFY BELOW LINE */
/* 아래 라인은 수정하지 말 것 */
static struct disk *swap_disk;
static bool anon_swap_in (struct page *page, void *kva);
static bool anon_swap_out (struct page *page);
static void anon_destroy (struct page *page);

/* DO NOT MODIFY this struct */
/* 아래 구조체는 수정하지 말 것 */
static const struct page_operations anon_ops = {
	.swap_in = anon_swap_in,
	.swap_out = anon_swap_out,
	.destroy = anon_destroy,
	.type = VM_ANON,
};

/* Initialize the data for anonymous pages */
/* 익명 페이지에 대한 데이터 초기화 수행 */
void
vm_anon_init (void) {
	/* TODO: Set up the swap_disk. */
	/* TODO: swap_disk 셋업 */
	swap_disk = NULL;
}

/* Initialize the file mapping */
/* 파일 매핑을 초기화 */
bool
anon_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	/* 핸들러 셋업 */
	page->operations = &anon_ops;

	struct anon_page *anon_page = &page->anon;
}

/* Swap in the page by read contents from the swap disk. */
/* 스왑 디스크로부터 내용을 읽어 페이지를 swap in */
static bool
anon_swap_in (struct page *page, void *kva) {
	struct anon_page *anon_page = &page->anon;
}

/* Swap out the page by writing contents to the swap disk. */
/* 스왑 디스크로 써넣은 페이지를 swap out */
static bool
anon_swap_out (struct page *page) {
	struct anon_page *anon_page = &page->anon;
}

/* Destroy the anonymous page. PAGE will be freed by the caller. */
/* 익명 페이지를 파기. 호출자에 의해 페이지가 free됨. */
static void
anon_destroy (struct page *page) {
	struct anon_page *anon_page = &page->anon;
}
