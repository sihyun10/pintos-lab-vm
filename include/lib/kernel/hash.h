#ifndef __LIB_KERNEL_HASH_H
#define __LIB_KERNEL_HASH_H

/* Hash table.
 *
 * This data structure is thoroughly documented in the Tour of
 * Pintos for Project 3.
 *
 * This is a standard hash table with chaining.  To locate an
 * element in the table, we compute a hash function over the
 * element's data and use that as an index into an array of
 * doubly linked lists, then linearly search the list.
 *
 * The chain lists do not use dynamic allocation.  Instead, each
 * structure that can potentially be in a hash must embed a
 * struct hash_elem member.  All of the hash functions operate on
 * these `struct hash_elem's.  The hash_entry macro allows
 * conversion from a struct hash_elem back to a structure object
 * that contains it.  This is the same technique used in the
 * linked list implementation.  Refer to lib/kernel/list.h for a
 * detailed explanation. */
/* 해시 테이블
 *
 * 이 자료 구조는 프로젝트 3을 위한 Pintos Tour 문서에 자세히 설명되어 있습니다.
 *
 * 이것은 체이닝을 사용하는 표준 해시 테이블입니다. 테이블에서 요소를 찾기 위해,
 * 요소의 데이터에 대한 해시 함수를 계산하고 그것을 이중 연결 리스트 배열의
 * 인덱스로 사용한 다음, 해당 리스트를 선형으로 검색합니다.
 *
 * 체인 리스트는 동적 할당을 사용하지 않습니다. 대신, 해시에 포함될 가능성이 있는
 * 각 구조체는 `struct hash_elem` 멤버를 내장해야 합니다. 모든 해시 함수는
 * 이러한 `struct hash_elem`에 대해 작동합니다. `hash_entry` 매크로는
 * `struct hash_elem`에서 그것을 포함하는 구조체 객체로 다시 변환할 수 있게
 * 해줍니다. 이것은 연결 리스트 구현에서 사용된 것과 동일한 기법입니다.
 * 자세한 설명은 `lib/kernel/list.h`를 참조하십시오.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "list.h"

/* Hash element. */
// 해시 요소.
struct hash_elem {
    struct list_elem list_elem;
};

/* Converts pointer to hash element HASH_ELEM into a pointer to
 * the structure that HASH_ELEM is embedded inside.  Supply the
 * name of the outer structure STRUCT and the member name MEMBER
 * of the hash element.  See the big comment at the top of the
 * file for an example. */
/* 해시 요소 HASH_ELEM을 가리키는 포인터를 HASH_ELEM이 내장된
 * 구조체를 가리키는 포인터로 변환합니다. 외부 구조체의 이름 STRUCT와
 * 해시 요소의 멤버 이름 MEMBER를 제공하십시오. 예시는 파일 상단의
 * 큰 주석을 참조하십시오. */
#define hash_entry(HASH_ELEM, STRUCT, MEMBER)                   \
    ((STRUCT *) ((uint8_t *) &(HASH_ELEM)->list_elem        \
       - offsetof (STRUCT, MEMBER.list_elem)))

/* Computes and returns the hash value for hash element E, given
 * auxiliary data AUX. */
// 보조 데이터 AUX가 주어졌을 때, 해시 요소 E에 대한 해시 값을 계산하고 반환합니다.
typedef uint64_t hash_hash_func (const struct hash_elem *e, void *aux);

/* Compares the value of two hash elements A and B, given
 * auxiliary data AUX.  Returns true if A is less than B, or
 * false if A is greater than or equal to B. */
/* 보조 데이터 AUX가 주어졌을 때, 두 해시 요소 A와 B의 값을 비교합니다.
 * A가 B보다 작으면 true를 반환하고, A가 B보다 크거나 같으면 false를 반환합니다. */
typedef bool hash_less_func (const struct hash_elem *a,
       const struct hash_elem *b,
       void *aux);

/* Performs some operation on hash element E, given auxiliary
 * data AUX. */
// 보조 데이터 AUX가 주어졌을 때, 해시 요소 E에 대해 어떤 작업을 수행합니다.
typedef void hash_action_func (struct hash_elem *e, void *aux);

/* Hash table. */
// 해시 테이블.
struct hash {
    size_t elem_cnt;            /* Number of elements in table. */
                                // 테이블 내 요소의 수.
    size_t bucket_cnt;          /* Number of buckets, a power of 2. */
                                // 버킷의 수, 2의 거듭제곱.
    struct list *buckets;       /* Array of `bucket_cnt' lists. */
                                // `bucket_cnt` 개수만큼의 리스트 배열.
    hash_hash_func *hash;       /* Hash function. */
                                // 해시 함수.
    hash_less_func *less;       /* Comparison function. */
                                // 비교 함수.
    void *aux;                  /* Auxiliary data for `hash' and `less'. */
                                // `hash`와 `less`를 위한 보조 데이터.
};

/* A hash table iterator. */
// 해시 테이블 반복자.
struct hash_iterator {
    struct hash *hash;          /* The hash table. */
                                // 해시 테이블.
    struct list *bucket;        /* Current bucket. */
                                // 현재 버킷.
    struct hash_elem *elem;     /* Current hash element in current bucket. */
                                // 현재 버킷 내의 현재 해시 요소.
};

/* Basic life cycle. */
// 기본 생명 주기.
bool hash_init (struct hash *, hash_hash_func *, hash_less_func *, void *aux);
void hash_clear (struct hash *, hash_action_func *);
void hash_destroy (struct hash *, hash_action_func *);

/* Search, insertion, deletion. */
// 검색, 삽입, 삭제.
struct hash_elem *hash_insert (struct hash *, struct hash_elem *);
struct hash_elem *hash_replace (struct hash *, struct hash_elem *);
struct hash_elem *hash_find (struct hash *, struct hash_elem *);
struct hash_elem *hash_delete (struct hash *, struct hash_elem *);

/* Iteration. */
// 반복.
void hash_apply (struct hash *, hash_action_func *);
void hash_first (struct hash_iterator *, struct hash *);
struct hash_elem *hash_next (struct hash_iterator *);
struct hash_elem *hash_cur (struct hash_iterator *);

/* Information. */
// 정보.
size_t hash_size (struct hash *);
bool hash_empty (struct hash *);

/* Sample hash functions. */
// 샘플 해시 함수.
uint64_t hash_bytes (const void *, size_t);
uint64_t hash_string (const char *);
uint64_t hash_int (int);

#endif /* lib/kernel/hash.h */
// lib/kernel/hash.h