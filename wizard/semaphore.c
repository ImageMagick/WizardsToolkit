/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        SSSSS  EEEEE  M   M   AAA   PPPP   H   H   OOO   RRRR   EEEEE        %
%        SS     E      MM MM  A   A  P   P  H   H  O   O  R   R  E            %
%         SSS   EEE    M M M  AAAAA  PPPP   HHHHH  O   O  RRRR   EEE          %
%           SS  E      M   M  A   A  P      H   H  O   O  R R    E            %
%        SSSSS  EEEEE  M   M  A   A  P      H   H   OOO   R  R   EEEEE        %
%                                                                             %
%                                                                             %
%                        WizardCore Semaphore Methods                         %
%                                                                             %
%                              Software Design                                %
%                             William Radcliffe                               %
%                                  Cristy                                     %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/memory-private.h"
#include "wizard/semaphore.h"
#include "wizard/semaphore-private.h"
#include "wizard/string_.h"
#include "wizard/thread_.h"
#include "wizard/thread-private.h"
#include "wizard/utility-private.h"

/*
  Struct declaractions.
*/
struct SemaphoreInfo
{
  WizardMutexType
    mutex;

  WizardThreadType
    id;

  ssize_t
    reference_count;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S e m a p h o r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSemaphoreInfo() initializes the SemaphoreInfo structure.
%
%  The format of the AcquireSemaphoreInfo method is:
%
%      SemaphoreInfo *AcquireSemaphoreInfo(void)
%
*/

static void *AcquireSemaphoreMemory(const size_t count,const size_t quantum)
{
#define AlignedExtent(size,alignment) \
  (((size)+((alignment)-1)) & ~((alignment)-1))

  size_t
    alignment,
    extent,
    size;

  void
    *memory;

  size=count*quantum;
  if ((count == 0) || (quantum != (size/count)))
    {
      errno=ENOMEM;
      return((void *) NULL);
    }
  memory=NULL;
  alignment=CACHE_LINE_SIZE;
  extent=AlignedExtent(size,CACHE_LINE_SIZE);
  if ((size == 0) || (extent < size))
    return((void *) NULL);
#if defined(WIZARDSTOOLKIT_HAVE_POSIX_MEMALIGN)
  if (posix_memalign(&memory,alignment,extent) != 0)
    memory=NULL;
#elif defined(WIZARDSTOOLKIT_HAVE__ALIGNED_MALLOC)
  memory=_aligned_malloc(extent,alignment);
#else
  {
    void
      *p;

    extent=(size+alignment-1)+sizeof(void *);
    if (extent > size)
      {
        p=malloc(extent);
        if (p != NULL)
          {
            memory=(void *) AlignedExtent((size_t) p+sizeof(void *),alignment);
            *((void **) memory-1)=p;
          }
      }
  }
#endif
  return(memory);
}

static void *RelinquishSemaphoreMemory(void *memory)
{
  if (memory == (void *) NULL)
    return((void *) NULL);
#if defined(WIZARDSTOOLKIT_HAVE_POSIX_MEMALIGN)
  free(memory);
#elif defined(WIZARDSTOOLKIT_HAVE__ALIGNED_MALLOC)
  _aligned_free(memory);
#else
  free(*((void **) memory-1));
#endif
  return(NULL);
}

WizardExport SemaphoreInfo *AcquireSemaphoreInfo(void)
{
  SemaphoreInfo
    *semaphore_info;

  /*
    Allocate semaphore.
  */
  semaphore_info=(SemaphoreInfo *) AcquireSemaphoreMemory(1,
    sizeof(*semaphore_info));
  if (semaphore_info == (SemaphoreInfo *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  (void) ResetWizardMemory(semaphore_info,0,sizeof(SemaphoreInfo));
  /*
    Initialize the semaphore.
  */
#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT)
  omp_init_lock((omp_lock_t *) &semaphore_info->mutex);
#elif defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  {
    int
      status;

    pthread_mutexattr_t
      mutex_info;

    status=pthread_mutexattr_init(&mutex_info);
    if (status != 0)
      {
        errno=status;
        perror("unable to initialize mutex attributes");
        _exit(1);
      }
    status=pthread_mutex_init(&semaphore_info->mutex,&mutex_info);
    if (status != 0)
      {
        errno=status;
        perror("unable to initialize mutex");
        _exit(1);
      }
    status=pthread_mutexattr_destroy(&mutex_info);
    if (status != 0)
      {
        errno=status;
        perror("unable to destroy mutex");
        _exit(1);
      }
  }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  {
    int
      status;

    status=InitializeCriticalSectionAndSpinCount(&semaphore_info->mutex,0x400);
    if (status == 0)
      {
        errno=status;
        perror("unable to initialize critical section");
        _exit(1);
      }
  }
#endif
  semaphore_info->id=GetWizardThreadId();
  semaphore_info->reference_count=0;
  semaphore_info->signature=WizardSignature;
  return(semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c t i v a t e S e m a p h o r e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ActivateSemaphoreInfo() activates a semaphore under protection of a mutex
%  to ensure only one thread allocates the semaphore.
%
%  The format of the ActivateSemaphoreInfo method is:
%
%      void ActivateSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport void ActivateSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
  if (*semaphore_info == (SemaphoreInfo *) NULL)
    {
      InitializeWizardMutex();
      LockWizardMutex();
      if (*semaphore_info == (SemaphoreInfo *) NULL)
        *semaphore_info=AcquireSemaphoreInfo();
      UnlockWizardMutex();
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o c k S e m a p h o r e I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LockSemaphoreInfo() locks a semaphore.
%
%  The format of the LockSemaphoreInfo method is:
%
%      void LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport void LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == WizardSignature);
#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT)
  omp_set_lock((omp_lock_t *) &semaphore_info->mutex);
#elif defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_lock(&semaphore_info->mutex);
    if (status != 0)
      {
        errno=status;
        perror("unable to lock mutex");
        _exit(1);
      }
  }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  EnterCriticalSection(&semaphore_info->mutex);
#endif
#if defined(WIZARDSTOOLKIT_DEBUG)
  if ((semaphore_info->reference_count > 0) &&
      (IsWizardThreadEqual(semaphore_info->id) != WizardFalse))
    {
      perror("unexpected recursive lock!");
      _exit(1);
    }
#endif
  semaphore_info->id=GetWizardThreadId();
  semaphore_info->reference_count++;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h S e m a p h o r e I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishSemaphoreInfo() destroys a semaphore.
%
%  The format of the RelinquishSemaphoreInfo method is:
%
%      void RelinquishSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport void RelinquishSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
  assert((*semaphore_info) != (SemaphoreInfo *) NULL);
  assert((*semaphore_info)->signature == WizardSignature);
  InitializeWizardMutex();
  LockWizardMutex();
#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT)
  omp_destroy_lock((omp_lock_t *) &(*semaphore_info)->mutex);
#elif defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_destroy(&(*semaphore_info)->mutex);
    if (status != 0)
      {
        errno=status;
        perror("unable to destroy mutex");
        _exit(1);
      }
  }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  DeleteCriticalSection(&(*semaphore_info)->mutex);
#endif
  (*semaphore_info)->signature=(~WizardSignature);
  *semaphore_info=(SemaphoreInfo *) RelinquishSemaphoreMemory(*semaphore_info);
  UnlockWizardMutex();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e m a p h o r e C o m p o n e n t G e n e s i s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SemaphoreComponentGenesis() instantiates the semaphore component.
%
%  The format of the SemaphoreComponentGenesis method is:
%
%      WizardBooleanType SemaphoreComponentGenesis(void)
%
*/
WizardExport WizardBooleanType SemaphoreComponentGenesis(void)
{
  InitializeWizardMutex();
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e m a p h o r e C o m p o n e n t T e r m i n u s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SemaphoreComponentTerminus() destroys the semaphore environment.
%
%  The format of the SemaphoreComponentTerminus method is:
%
%      SemaphoreComponentTerminus(void)
%
*/
WizardExport void SemaphoreComponentTerminus(void)
{
  DestroyWizardMutex();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n l o c k S e m a p h o r e I n f o                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnlockSemaphoreInfo() unlocks a semaphore.
%
%  The format of the UnlockSemaphoreInfo method is:
%
%      void UnlockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport void UnlockSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == WizardSignature);
#if defined(WIZARDSTOOLKIT_DEBUG)
  assert(IsWizardThreadEqual(semaphore_info->id) != WizardFalse);
  if (semaphore_info->reference_count == 0)
    {
      (void) fprintf(stderr,"Warning: semaphore lock already unlocked!\n");
      (void) fflush(stderr);
      return;
    }
  semaphore_info->reference_count--;
#endif
#if defined(WIZARDSTOOLKIT_OPENMP_SUPPORT)
  omp_unset_lock((omp_lock_t *) &semaphore_info->mutex);
#elif defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
  {
    int
      status;

    status=pthread_mutex_unlock(&semaphore_info->mutex);
    if (status != 0)
      {
        errno=status;
        perror("unable to unlock mutex");
        _exit(1);
      }
  }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  LeaveCriticalSection(&semaphore_info->mutex);
#endif
}
