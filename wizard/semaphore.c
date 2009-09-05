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
%                      Wizard's Toolkit Semaphore Methods                     %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.wizards-toolkit.org/script/license.php                        %
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
#include "wizard/client.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/semaphore.h"
#include "wizard/string_.h"
#include "wizard/thread_.h"
#include "wizard/thread-private.h"

/*
  Struct declaractions.
*/
struct SemaphoreInfo
{
  WizardMutexType
    mutex;

  WizardThreadType
    id;

  long
    reference_count;

  unsigned long
    signature;
};

/*
  Static declaractions.
*/
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
static pthread_mutex_t
  semaphore_mutex = PTHREAD_MUTEX_INITIALIZER;
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
static LONG
  semaphore_mutex = 0;
#else
static long
  semaphore_mutex = 0;
#endif

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
%  AcquireSemaphoreInfo() acquires a semaphore.
%
%  The format of the AcquireSemaphoreInfo method is:
%
%      void AcquireSemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport void AcquireSemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  if (pthread_mutex_lock(&semaphore_mutex) != 0)
    {
      (void) fprintf(stderr,"pthread_mutex_lock failed %s\n",strerror(errno));
      _exit(1);
    }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  while (InterlockedCompareExchange(&semaphore_mutex,1L,0L) != 0)
    Sleep(10);
#endif
  if (*semaphore_info == (SemaphoreInfo *) NULL)
    *semaphore_info=AllocateSemaphoreInfo();
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  if (pthread_mutex_unlock(&semaphore_mutex) != 0)
    {
      (void) fprintf(stderr,"pthread_mutex_unlock failed %s\n",strerror(errno));
      _exit(1);
    }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  InterlockedExchange(&semaphore_mutex,0L);
#endif
  (void) LockSemaphoreInfo(*semaphore_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e S e m a p h o r e I n f o                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AllocateSemaphoreInfo() initializes the SemaphoreInfo structure.
%
%  The format of the AllocateSemaphoreInfo method is:
%
%      SemaphoreInfo *AllocateSemaphoreInfo(void)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Method AllocateSemaphoreInfo returns a pointer to an
%      initialized SemaphoreInfo structure.
%
%
*/
WizardExport SemaphoreInfo *AllocateSemaphoreInfo(void)
{
  SemaphoreInfo
    *semaphore_info;

  /*
    Allocate semaphore.
  */
  semaphore_info=(SemaphoreInfo *) AcquireAlignedMemory(1,
    sizeof(SemaphoreInfo));
  if (semaphore_info == (SemaphoreInfo *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  (void) ResetWizardMemory(semaphore_info,0,sizeof(SemaphoreInfo));
  /*
    Initialize the semaphore.
  */
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  {
    int
      status;

    pthread_mutexattr_t
      mutex_info;

    status=pthread_mutexattr_init(&mutex_info);
    if (status != 0)
      {
        semaphore_info=(SemaphoreInfo *) RelinquishAlignedMemory(
          semaphore_info);
        return((SemaphoreInfo *) NULL);
      }
    status=pthread_mutex_init(&semaphore_info->mutex,&mutex_info);
    (void) pthread_mutexattr_destroy(&mutex_info);
    if (status != 0)
      {
        semaphore_info=(SemaphoreInfo *) RelinquishAlignedMemory(
          semaphore_info);
        return((SemaphoreInfo *) NULL);
      }
  }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  InitializeCriticalSection(&semaphore_info->mutex);
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
%   D e s t r o y S e m a p h o r e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySemaphore() destroys the semaphore environment.
%
%  The format of the DestroySemaphore method is:
%
%      DestroySemaphore(void)
%
*/
WizardExport void DestroySemaphore(void)
{
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  if (pthread_mutex_destroy(&semaphore_mutex) != 0)
    {
      (void) fprintf(stderr,"pthread_mutex_destroy failed %s\n",
        strerror(errno));
      _exit(1);
    }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S e m a p h o r e I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySemaphoreInfo() destroys a semaphore.
%
%  The format of the DestroySemaphoreInfo method is:
%
%      void DestroySemaphoreInfo(SemaphoreInfo **semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport void DestroySemaphoreInfo(SemaphoreInfo **semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo **) NULL);
  assert((*semaphore_info) != (SemaphoreInfo *) NULL);
  assert((*semaphore_info)->signature == WizardSignature);
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  (void) pthread_mutex_lock(&semaphore_mutex);
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  while (InterlockedCompareExchange(&semaphore_mutex,1L,0L) != 0)
    Sleep(10);
#endif
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  (void) pthread_mutex_destroy(&(*semaphore_info)->mutex);
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  DeleteCriticalSection(&(*semaphore_info)->mutex);
#endif
  (*semaphore_info)->signature=(~WizardSignature);
  *semaphore_info=(SemaphoreInfo *) RelinquishAlignedMemory(*semaphore_info);
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  (void) pthread_mutex_unlock(&semaphore_mutex);
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  InterlockedExchange(&semaphore_mutex,0L);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e S e m a p h o r e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeSemaphore() initializes the semaphore environment.
%
%  The format of the InitializeSemaphore method is:
%
%      InitializeSemaphore(void)
%
*/
WizardExport void InitializeSemaphore(void)
{
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
%      WizardBooleanType LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport WizardBooleanType LockSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == WizardSignature);
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  {
    int
      status;

    status=pthread_mutex_lock(&semaphore_info->mutex);
    if (status != 0)
      return(WizardFalse);
#if defined(WIZARDSTOOLKIT_DEBUG)
    {
      if ((semaphore_info->reference_count > 0) &&
          (IsWizardThreadEqual(semaphore_info->id) != WizardFalse))
        {
          (void) fprintf(stderr,"Warning: unexpected recursive lock!\n");
          (void) fflush(stderr);
        }
    }
#endif
  }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  {
    EnterCriticalSection(&semaphore_info->mutex);
#if defined(WIZARDSTOOLKIT_DEBUG)
    {
      if ((semaphore_info->reference_count > 0) &&
          (IsWizardThreadEqual(semaphore_info->id) != WizardFalse))
        {
          (void) fprintf(stderr,"Warning: unexpected recursive lock!\n");
          (void) fflush(stderr);
        }
      semaphore_info->id=id;
    }
#endif
  }
#endif
  semaphore_info->id=GetWizardThreadId();
  semaphore_info->reference_count++;
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n g u i s h S e m a p h o r e I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishSemaphoreInfo() relinquishes a semaphore.
%
%  The format of the RelinquishSemaphoreInfo method is:
%
%      RelinquishSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport void RelinquishSemaphoreInfo(SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == WizardSignature);
  (void) UnlockSemaphoreInfo(semaphore_info);
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
%      WizardBooleanType UnlockSemaphoreInfo(SemaphoreInfo *semaphore_info)
%
%  A description of each parameter follows:
%
%    o semaphore_info: Specifies a pointer to an SemaphoreInfo structure.
%
*/
WizardExport WizardBooleanType UnlockSemaphoreInfo(
  SemaphoreInfo *semaphore_info)
{
  assert(semaphore_info != (SemaphoreInfo *) NULL);
  assert(semaphore_info->signature == WizardSignature);
#if defined(WIZARDSTOOLKIT_DEBUG)
  assert(IsSizeThreadEqual(semaphore_info->id) != WizardFalse);
  if (semaphore_info->reference_count == 0)
    {
      (void) fprintf(stderr,"Warning: semaphore lock already unlocked!\n");
      (void) fflush(stderr);
      return(WizardFalse);
    }
  semaphore_info->reference_count--;
#endif
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  {
    int
      status;

    status=pthread_mutex_unlock(&semaphore_info->mutex);
    if (status != 0)
      {
        semaphore_info->reference_count++;
        return(WizardFalse);
      }
  }
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
  LeaveCriticalSection(&semaphore_info->mutex);
#endif
  return(WizardTrue);
}
