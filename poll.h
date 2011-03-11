#ifndef _POLL_H
#define _POLL_H

#include "hash_table/hashtable.h"
#include <sys/epoll.h>

#define MAX_EVENTS 100
#define CALLBACK(x) void (*x) (poll_event_t *, poll_event_element_t *, struct epoll_event)

typedef struct poll_event_element poll_event_element_t;
typedef struct poll_event poll_event_t;

struct poll_event_element
{
    int fd;
    CALLBACK(write_callback);
    CALLBACK(read_callback);
    CALLBACK(close_callback);
    void * data;
    uint32_t events;
    uint32_t cur_event;
};
#define poll_event_element_s sizeof(poll_event_element_t)

struct poll_event
{
    hash_table_t *table;
    int (*timeout_callback)(poll_event_t *);
    size_t event_count;
    size_t timeout;
    int epoll_fd;
    int cur_event_count;
};
#define poll_event_s sizeof(poll_event_t)

//poll_event_element functions
poll_event_element_t * poll_event_element_new(int, uint32_t);
void poll_event_element_delete(poll_event_element_t *);

// poll_event function
poll_event_t * poll_event_new(int);
void poll_event_delete(poll_event_t*);
#define use_the_force(x) poll_event_loop(x)
int poll_event_loop(poll_event_t*);
int poll_event_add(poll_event_t*, int, uint32_t);
int poll_event_remove(poll_event_t*, int);

#endif
