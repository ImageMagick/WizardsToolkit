/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  WizardCore private methods for internal threading.
*/
#ifndef _WIZARDSTOOLKIT_THREAD_PRIVATE_H
#define _WIZARDSTOOLKIT_THREAD_PRIVATE_H

#include <wizard/thread_.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__clang__) || (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ > 10))
#define WizardCachePrefetch(address,mode,locality) \
  __builtin_prefetch(address,mode,locality)
#else
#define WizardCachePrefetch(address,mode,locality)
#endif

#define omp_throttle(factor)  num_threads(omp_get_max_threads() >> \
   (factor) == 0 ? 1 : omp_get_max_threads() >> (factor))

#if defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  typedef pthread_mutex_t WizardMutexType;
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  typedef CRITICAL_SECTION WizardMutexType;
#else
  typedef size_t WizardMutexType;
#endif

static inline WizardThreadType GetWizardThreadId(void)
{
#if defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  return(pthread_self());
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  return(GetCurrentThreadId());
#else
  return(getpid());
#endif
}

static inline size_t GetWizardThreadSignature(void)
{
#if defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  {
    union
    {
      pthread_t
        id;

      size_t
        signature;
    } wizard_thread;

    wizard_thread.signature=0UL;
    wizard_thread.id=pthread_self();
    return(wizard_thread.signature);
  }
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  return((size_t) GetCurrentThreadId());
#else
  return((size_t) getpid());
#endif
}

static inline WizardBooleanType IsWizardThreadEqual(const WizardThreadType id)
{
#if defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  if (pthread_equal(id,pthread_self()) != 0)
    return(WizardTrue);
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  if (id == GetCurrentThreadId())
    return(WizardTrue);
#else
  if (id == getpid())
    return(WizardTrue);
#endif
  return(WizardFalse);
}

/*
  Lightweight OpenMP methods.
*/
static inline size_t GetOpenMPMaximumThreads(void)
{
  static size_t
    maximum_threads = 1;

#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT)
  if (omp_get_max_threads() > (ssize_t) maximum_threads)
    maximum_threads=omp_get_max_threads();
#endif
  return(maximum_threads);
}

static inline ssize_t GetOpenMPThreadId(void)
{
#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT)
  return(omp_get_thread_num());
#else
  return(0);
#endif
}

static inline void SetOpenMPMaximumThreads(const size_t threads)
{
#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT)
  omp_set_num_threads(threads);
#else
  (void) threads;
#endif
}

static inline void SetOpenMPNested(const int value)
{
#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT) && (_OPENMP >= 200203)
  omp_set_nested(value);
#else
  (void) value;
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
