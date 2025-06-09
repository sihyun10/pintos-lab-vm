#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "filesys/off_t.h"

tid_t process_create_initd (const char *file_name);
tid_t process_fork (const char *name, struct intr_frame *if_);
int process_exec (void *f_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (struct thread *next);

// for VM
bool install_page (void *upage, void *kpage, bool writable);
static bool lazy_load_segment (struct page *page, void *aux);
static bool setup_stack (struct intr_frame *if_);

/* 지연 로딩을 위한 정보 할당,
 * 어떤 파일에서 - *file
 * 얼마만큼 - read_bytes
 * 읽고 남은 0으로 초기화해줄 공간 - zero_bytes
 * 어떤 오프셋부터 읽어야 하는지 - offset */
struct file_load_info
{
    struct file *file;
    off_t offset;
    uint32_t read_bytes;
    uint32_t zero_bytes;
};

#endif /* userprog/process.h */
