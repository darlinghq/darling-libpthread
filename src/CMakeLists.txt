project(libc-pthreads)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -std=gnu11")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/resolver)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../private)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_definitions(
	-DPRIVATE
	-D__LIBC__
	-D__DARWIN_UNIX03=1
	-D__DARWIN_64_BIT_INO_T=1
	-D__DARWIN_NON_CANCELABLE=1
	-D__DARWIN_VERS_1050=1
	-D_FORTIFY_SOURCE=0
	-D__PTHREAD_BUILDING_PTHREAD__=1
	-D__PTHREAD_EXPOSE_INTERNALS__

	-DDARLING_BUILDING_OLD_PTHREAD=1
)

set(pthread_common_sources
	pthread_atfork.c
	pthread.c
	pthread_cond.c
	pthread_cwd.c
	pthread_dependency.c
	#pthread_mutex.c
	old-pthread/pthread_mutex.c
	pthread_rwlock.c
	pthread_tsd.c
	pthread_cancelable.c
	qos.c
	resolver/resolver.c
	pthread_asm.S
)
set(pthread_sources
	${pthread_common_sources}
	variants/pthread_cancelable_cancel.c
	variants/pthread_cancelable_legacy.c
	variants/pthread_cond_legacy.c
	variants/pthread_mutex_legacy.c
	variants/pthread_rwlock_legacy.c
)

SET_SOURCE_FILES_PROPERTIES(pthread_mutex.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}")
SET_SOURCE_FILES_PROPERTIES(pthread.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_CANCEL -DLIBC_ALIAS_PTHREAD_SETCANCELSTATE -DLIBC_ALIAS_PTHREAD_SETCANCELTYPE -DLIBC_ALIAS_PTHREAD_SIGMASK -DLIBC_ALIAS_PTHREAD_TESTCANCEL")
SET_SOURCE_FILES_PROPERTIES(pthread_cancelable.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_PTHREAD_COND_TIMEDWAIT -DLIBC_ALIAS_PTHREAD_COND_WAIT -DLIBC_ALIAS_PTHREAD_JOIN -DLIBC_ALIAS_SIGWAIT")
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
add_circular(system_pthread FAT
	SOURCES
		${pthread_sources}
	SIBLINGS
		platform system_kernel system_dyld system_c
)
#target_link_libraries(system_pthread system_kernel platform)
#make_fat(system_pthread)

install(TARGETS system_pthread DESTINATION libexec/darling/usr/lib/system)

# DYLD variant

add_darling_static_library(pthread_static SOURCES ${pthread_common_sources})
SET_TARGET_PROPERTIES(pthread_static PROPERTIES COMPILE_FLAGS "-DVARIANT_DYLD=1 -DVARIANT_STATIC=1")
make_fat(pthread_static)
add_dependencies(pthread_static ranlib ${APPLE_TARGET_TRIPLET_PRIMARY}-ar)

