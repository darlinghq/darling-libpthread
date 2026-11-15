/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * 32-bit macOS programs built against old SDKs import the pthread cancellation
 * points and a few other functions without the $UNIX2003 suffix. This test
 * references those undecorated names explicitly through asm labels (so it
 * binds to them regardless of the SDK the test is compiled with) and checks
 * that they resolve and behave like the documented functions.
 *
 * Build for i386: guest-cc.sh -a i386 -t 10.13 -o i386_legacy_symbols i386_legacy_symbols.c
 * Prints PASS and exits 0 on success.
 */
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#define LEGACY(ret, name, args) extern ret legacy_##name args __asm__("_" #name)

LEGACY(int, pthread_join, (pthread_t, void **));
LEGACY(int, pthread_cancel, (pthread_t));
LEGACY(int, pthread_cond_init, (pthread_cond_t *, const pthread_condattr_t *));
LEGACY(int, pthread_cond_wait, (pthread_cond_t *, pthread_mutex_t *));
LEGACY(int, pthread_cond_timedwait, (pthread_cond_t *, pthread_mutex_t *, const struct timespec *));
LEGACY(int, pthread_setcanceltype, (int, int *));
LEGACY(int, pthread_sigmask, (int, const sigset_t *, sigset_t *));

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond;
static int ready;

static void *waiter(void *arg)
{
	pthread_mutex_lock(&mtx);
	while (!ready)
		legacy_pthread_cond_wait(&cond, &mtx);
	pthread_mutex_unlock(&mtx);
	return arg;
}

static void *sleeper(void *arg)
{
	for (;;)
		nanosleep(&(struct timespec){ 0, 10 * 1000 * 1000 }, NULL);
	return arg;
}

#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

int main(void)
{
	pthread_t t;
	void *rv = NULL;
	int old;
	sigset_t set, oldset;
	struct timespec ts;

	CHECK(legacy_pthread_cond_init(&cond, NULL) == 0);

	/* cond_wait + join */
	CHECK(pthread_create(&t, NULL, waiter, (void *)0x1234) == 0);
	nanosleep(&(struct timespec){ 0, 50 * 1000 * 1000 }, NULL);
	pthread_mutex_lock(&mtx);
	ready = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mtx);
	CHECK(legacy_pthread_join(t, &rv) == 0);
	CHECK(rv == (void *)0x1234);

	/* timed wait times out with ETIMEDOUT */
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += 20 * 1000 * 1000;
	if (ts.tv_nsec >= 1000000000) { ts.tv_sec++; ts.tv_nsec -= 1000000000; }
	pthread_mutex_lock(&mtx);
	CHECK(legacy_pthread_cond_timedwait(&cond, &mtx, &ts) == ETIMEDOUT);
	pthread_mutex_unlock(&mtx);

	/* setcanceltype + cancel + join reports PTHREAD_CANCELED */
	CHECK(legacy_pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old) == 0);
	CHECK(pthread_create(&t, NULL, sleeper, NULL) == 0);
	nanosleep(&(struct timespec){ 0, 50 * 1000 * 1000 }, NULL);
	CHECK(legacy_pthread_cancel(t) == 0);
	CHECK(legacy_pthread_join(t, &rv) == 0);
	CHECK(rv == PTHREAD_CANCELED);

	/* sigmask round trip */
	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);
	CHECK(legacy_pthread_sigmask(SIG_BLOCK, &set, &oldset) == 0);
	CHECK(legacy_pthread_sigmask(SIG_SETMASK, &oldset, &set) == 0);
	CHECK(sigismember(&set, SIGUSR2) == 1);

	printf("PASS\n");
	return 0;
}
