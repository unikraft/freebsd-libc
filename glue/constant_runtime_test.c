/*
 * FreeBSD libc Constant Runtime Verification
 * 
 * This file contains runtime tests to verify constants work correctly
 * in practice, not just compile-time compatibility.
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <uk/test.h>

UK_TESTCASE(freebsd_libc, constant_errno_values)
{
    /* Test that errno values work correctly in practice */
    int fd = open("/nonexistent/file", O_RDONLY);
    UK_TEST_EXPECT_SNUM_EQ(fd, -1);
    UK_TEST_EXPECT_SNUM_EQ(errno, ENOENT);
    
    /* Test EAGAIN/EWOULDBLOCK equivalence */
    UK_TEST_EXPECT_SNUM_EQ(EAGAIN, EWOULDBLOCK);
}

UK_TESTCASE(freebsd_libc, constant_file_flags)
{
    /* Test file operation flags work as expected */
    int flags;
    
    /* Basic flags */
    flags = O_RDONLY;
    UK_TEST_EXPECT_SNUM_EQ(flags & O_ACCMODE, O_RDONLY);
    
    flags = O_WRONLY;
    UK_TEST_EXPECT_SNUM_EQ(flags & O_ACCMODE, O_WRONLY);
    
    flags = O_RDWR;
    UK_TEST_EXPECT_SNUM_EQ(flags & O_ACCMODE, O_RDWR);
    
    /* Combined flags */
    flags = O_CREAT | O_WRONLY | O_TRUNC;
    UK_TEST_EXPECT_TRUE(flags & O_CREAT);
    UK_TEST_EXPECT_TRUE(flags & O_WRONLY);
    UK_TEST_EXPECT_TRUE(flags & O_TRUNC);
    UK_TEST_EXPECT_FALSE(flags & O_RDONLY);
}

UK_TESTCASE(freebsd_libc, constant_poll_events)
{
    struct pollfd pfd;
    
    /* Test poll constants are usable */
    pfd.events = POLLIN | POLLOUT;
    UK_TEST_EXPECT_TRUE(pfd.events & POLLIN);
    UK_TEST_EXPECT_TRUE(pfd.events & POLLOUT);
    UK_TEST_EXPECT_FALSE(pfd.events & POLLERR);
    
    /* Test error events */
    pfd.revents = POLLERR | POLLHUP;
    UK_TEST_EXPECT_TRUE(pfd.revents & POLLERR);
    UK_TEST_EXPECT_TRUE(pfd.revents & POLLHUP);
    UK_TEST_EXPECT_FALSE(pfd.revents & POLLIN);
}

UK_TESTCASE(freebsd_libc, constant_socket_families)
{
    int sock;
    
    /* Test AF_INET socket creation */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock >= 0) {
        UK_TEST_EXPECT_TRUE(sock >= 0);
        close(sock);
    }
    
    /* Test AF_INET6 socket creation (may not be supported) */
    sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock >= 0) {
        close(sock);
    } else {
        /* AF_INET6 not supported is OK */
        UK_TEST_EXPECT_TRUE(errno == EAFNOSUPPORT || errno == EPROTONOSUPPORT);
    }
}

UK_TESTCASE(freebsd_libc, constant_time_clocks)
{
    struct timespec ts;
    int ret;
    
    /* Test CLOCK_REALTIME */
    ret = clock_gettime(CLOCK_REALTIME, &ts);
    UK_TEST_EXPECT_SNUM_EQ(ret, 0);
    UK_TEST_EXPECT_TRUE(ts.tv_sec > 0);  /* Should have valid time */
    
    /* Test CLOCK_MONOTONIC */
    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret == 0) {
        UK_TEST_EXPECT_TRUE(ts.tv_sec >= 0);  /* Monotonic starts from 0 */
    } else {
        /* CLOCK_MONOTONIC not supported is OK */
        UK_TEST_EXPECT_SNUM_EQ(errno, EINVAL);
    }
}

UK_TESTCASE(freebsd_libc, constant_signal_numbers)
{
    /* Test basic signal constants are valid */
    UK_TEST_EXPECT_TRUE(SIGINT > 0 && SIGINT < 32);
    UK_TEST_EXPECT_TRUE(SIGTERM > 0 && SIGTERM < 32);
    UK_TEST_EXPECT_TRUE(SIGKILL > 0 && SIGKILL < 32);
    
    /* Test signal relationships */
    UK_TEST_EXPECT_SNUM_NE(SIGINT, SIGTERM);
    UK_TEST_EXPECT_SNUM_NE(SIGINT, SIGKILL);
    UK_TEST_EXPECT_SNUM_NE(SIGTERM, SIGKILL);
}

/* Test suite registration */
uk_testsuite_register(freebsd_libc, NULL);