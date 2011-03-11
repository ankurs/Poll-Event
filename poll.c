#include "poll.h"
#include "debug.h"
#include "hash_table/hashtable.h"
#include <stdlib.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

//poll_event_element functions
poll_event_element_t * poll_event_element_new(int fd, uint32_t events)
{
    INFO("Creating a new poll event element");
    poll_event_element_t *elem = calloc(1, poll_event_element_s);
    if (elem)
    {
        elem->fd = fd;
        elem->events = events;
    }
    return elem;
}

void poll_event_element_delete(poll_event_element_t * poll_event)
{
    INFO("Deleting a poll event element");
    free(poll_event);
}

// poll_event function
poll_event_t * poll_event_new(int timeout)
{
    poll_event_t * poll_event = calloc(1, poll_event_s);
    if (!poll_event)
    {
        INFO("calloc failed at poll_event");
        return NULL; // No Memory
    }
    poll_event->table = hash_table_new(MODE_VALUEREF);
    if (!poll_event->table)
    {
        free(poll_event);
        INFO("calloc failed at hashtble");
        return NULL;
    }
    poll_event->timeout = timeout;
    poll_event->epoll_fd = epoll_create(MAX_EVENTS);
    poll_event->cur_event_count = 0;
    INFO("Created a new poll event");
    return poll_event;
}

void poll_event_delete(poll_event_t* poll_event)
{
    INFO("deleting a poll_event");
    hash_table_delete(poll_event->table);
    free(poll_event);
}

int poll_event_loop(poll_event_t* poll_event)
{
    struct epoll_event events[MAX_EVENTS];
    while(1) // yes loop forever
    {
        INFO("Entering the main event loop for epoll lib");
        INFO("May the source be with you!!");
        int fds = epoll_wait(poll_event->epoll_fd, events, MAX_EVENTS, poll_event->timeout);
        if (fds == 0)
        {
            INFO("event loop timed out");
            if (!poll_event->timeout_callback(poll_event))
            {
                return 0;
            }
        }
        int i = 0;
        for(;i<fds;i++)
        {
            poll_event_element_t * value = NULL;
            if ((value = (poll_event_element_t *) HT_LOOKUP(poll_event->table, &events[i].data.fd)) != NULL)
            {
                LOG("started processing for event id(%d) and sock(%d)", i, events[i].data.fd);
                // when data avaliable for read or urgent flag is set
                if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
                {
                    if (events[i].events & EPOLLIN)
                    {
                        LOG("found EPOLLIN for event id(%d) and sock(%d)", i, events[i].data.fd);
                        value->cur_event &= EPOLLIN;
                    }
                    else
                    {
                        LOG("found EPOLLPRI for event id(%d) and sock(%d)", i, events[i].data.fd);
                        value->cur_event &= EPOLLPRI;
                    }
                    /// connect or accept callbacks also go through EPOLLIN
                    value->read_callback(poll_event, value, events[i]);
                }
                // when write possible
                if (events[i].events & EPOLLOUT)
                {
                    LOG("found EPOLLOUT for event id(%d) and sock(%d)", i, events[i].data.fd);
                    value->cur_event &= EPOLLOUT;
                    value->write_callback(poll_event, value, events[i]);
                }
                // shutdown or error
                if ( (events[i].events & EPOLLRDHUP) || (events[i].events & EPOLLERR) )
                {
                    if (events[i].events & EPOLLRDHUP)
                    {
                        LOG("found EPOLLRDHUP for event id(%d) and sock(%d)", i, events[i].data.fd);
                        value->cur_event &= EPOLLRDHUP;
                    }
                    else
                    {
                        LOG("found EPOLLERR for event id(%d) and sock(%d)", i, events[i].data.fd);
                        value->cur_event &= EPOLLERR;
                    }
                    value->close_callback(poll_event, value, events[i]);
                }
            }
            else // not in table
            {
                LOG("WARNING: NOT FOUND hash table value for event id(%d) and sock(%d)", i, events[i].data.fd);
            }
        } // for
    } // while
} // function

int poll_event_add(poll_event_t* poll_event, int fd, uint32_t flags)
{
    poll_event_element_t *elem = NULL;
    elem = (poll_event_element_t *) HT_LOOKUP(poll_event->table, &fd);
    if (elem)
    {
        LOG("fd (%d) already added updating flags", fd);
        elem->events |= flags;
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = elem->events;
        return epoll_ctl(poll_event->epoll_fd, EPOLL_CTL_MOD, fd, &ev);
    }
    else
    {
        elem = poll_event_element_new(fd, flags);
        if (HT_ADD(poll_event->table, &fd, elem))
        {
            // error in hash table
            return -1;
        }
        struct epoll_event ev;
        memset(&ev, 0, sizeof(struct epoll_event));
        ev.data.fd = fd;
        ev.events = elem->events;
        return epoll_ctl(poll_event->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
    }
}

int poll_event_remove(poll_event_t* poll_event, int fd)
{
    return 0;
}

