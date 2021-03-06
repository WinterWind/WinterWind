/*
 * Copyright (c) 2016-2017, Loic Blot <loic.blot@unix-experience.fr>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "utils/threads.h"
#if defined(__FreeBSD__) || defined(__OpenBSD__)
#include <string>
#include <pthread_np.h>
#endif

Thread::Thread()
{
	m_request_stop = false;
	m_running = false;
	started = false;
}

Thread::~Thread() { kill(); }

void Thread::wait()
{
	if (started) {
		if (m_thread) {
			m_thread->join();
			m_thread = nullptr;
		}
		started = false;
	}
}

const bool Thread::start()
{
	if (m_running) {
		return false;
	}

	m_request_stop = false;

	continuemutex.lock();
	m_thread = std::make_unique<std::thread>(the_thread, this);
	if (!m_thread) {
		continuemutex.unlock();
		return false;
	}

	// Wait until 'm_running' flag is set
	while (!m_running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	started = true;

	continuemutex.unlock();

	continuemutex2.lock();
	continuemutex2.unlock();
	return true;
}

const bool Thread::kill()
{
	if (!m_running) {
		wait();
		return false;
	}

	if (m_thread) {
		// We need to pthread_kill instead on Android since NDKv5's pthread
		// implementation is incomplete.
		pthread_cancel(m_thread->native_handle());
	}

	wait();
	m_running = false;

	return true;
}

void *Thread::the_thread(void *data)
{
	auto *thread = (Thread *) data;

	thread->continuemutex2.lock();
	thread->m_running = true;

	thread->continuemutex.lock();
	thread->continuemutex.unlock();

	thread->run();

	thread->m_running = false;
	return nullptr;
}

void Thread::ThreadStarted() { continuemutex2.unlock(); }

void Thread::set_thread_name(const std::string &name)
{
#if defined(__FreeBSD__) || defined(__OpenBSD__)
	pthread_set_name_np(pthread_self(), name.c_str());
#else
	pthread_setname_np(pthread_self(), name.c_str());
#endif
}
