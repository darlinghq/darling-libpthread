project(libc-pthreads)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_SOURCE_DIR}/src/libc/derived)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers/spawn)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../private)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(-DPRIVATE)

set(pthreads_sources
	pthread_atfork.c
	pthread.c
	pthread_cond.c
	pthread_cwd.c
	pthread_mutex.c
	pthread_mutex_up.c
	pthread_rwlock.c
	pthread_support.c
	pthread_tsd.c
	pthread_cancelable.c
	qos.c
	resolver.c
	thread_setup.c
	pthread_asm.S
	variants/pthread_cancelable_cancel.c
	variants/pthread_cancelable_legacy.c
	variants/pthread_cond_legacy.c
	variants/pthread_mutex_legacy.c
	variants/pthread_rwlock_legacy.c
)

SET_SOURCE_FILES_PROPERTIES(pthread_mutex.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}")
SET_SOURCE_FILES_PROPERTIES(pthread.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_CANCEL -DLIBC_ALIAS_PTHREAD_SETCANCELSTATE -DLIBC_ALIAS_PTHREAD_SETCANCELTYPE -DLIBC_ALIAS_PTHREAD_SIGMASK -DLIBC_ALIAS_PTHREAD_TESTCANCEL")
SET_SOURCE_FILES_PROPERTIES(pthread_cancelable.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_COND_TIMEDWAIT -DLIBC_ALIAS_PTHREAD_COND_WAIT -DLIBC_ALIAS_PTHREAD_JOIN -DLIBC_ALIAS_SIGWAIT -D__DARWIN_NON_CANCELABLE=1")
#SET_SOURCE_FILES_PROPERTIES(thread_setup.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(pthread_cond.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_COND_TIMEDWAIT -DLIBC_ALIAS_PTHREAD_COND_INIT -DLIBC_ALIAS_PTHREAD_COND_WAIT")
SET_SOURCE_FILES_PROPERTIES(pthread_rwlock.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_RWLOCK_INIT -DLIBC_ALIAS_PTHREAD_RWLOCK_WRLOCK -DLIBC_ALIAS_PTHREAD_RWLOCK_RDLOCK -DLIBC_ALIAS_PTHREAD_RWLOCK_UNLOCK -DLIBC_ALIAS_PTHREAD_RWLOCK_DESTROY -DLIBC_ALIAS_PTHREAD_RWLOCK_TRYRDLOCK -DLIBC_ALIAS_PTHREAD_RWLOCK_TRYWRLOCK")

set_source_files_properties(
	#variants/pthread_cancelable_cancel.c
	variants/pthread_cancelable_legacy.c
	variants/pthread_cond_legacy.c
	variants/pthread_mutex_legacy.c
	variants/pthread_rwlock_legacy.c
	PROPERTIES COMPILE_FLAGS
		"-include ${CMAKE_CURRENT_SOURCE_DIR}/../nonstd.h ${CMAKE_C_FLAGS}"
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_pthread.dylib")
add_darling_library(system_pthread SHARED ${pthreads_sources})
target_link_libraries(system_pthread system_kernel platform system_dyld system_c)
make_fat(system_pthread)

install(TARGETS system_pthread DESTINATION libexec/darling/usr/lib/system)

# DYLD variant

add_library(libc-pthreads_dyld OBJECT pthread.c pthread_mutex.c pthread_tsd.c pthread_cond.c pthread_cancelable.c)
SET_TARGET_PROPERTIES(libc-pthreads_dyld PROPERTIES COMPILE_FLAGS "-UBUILDING_VARIANT -DVARIANT_DYLD -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN")
