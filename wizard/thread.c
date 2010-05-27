/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                  TTTTT  H   H  RRRR   EEEEE   AAA   DDDD                    %
%                    T    H   H  R   R  E      A   A  D   D                   %
%                    T    HHHHH  RRRR   EEE    AAAAA  D   D                   %
%                    T    H   H  R R    E      A   A  D   D                   %
%                    T    H   H  R  R   EEEEE  A   A  DDDD                    %
%                                                                             %
%                                                                             %
%                         WizardCore Thread Methods                           %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.wizards-toolkit.org/script/license.php                            %
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
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/memory_.h"
#include "wizard/thread_.h"
#include "wizard/thread-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k C r e a t e T h r e a d K e y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardCreateThreadKey() creates a thread key and returns it.
%
%  The format of the WizardCreateThreadKey method is:
%
%      WizardThreadKey WizardCreateThreadKey(WizardThreadKey *key)
%
*/
WizardExport WizardBooleanType WizardCreateThreadKey(WizardThreadKey *key)
{
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  return(pthread_key_create(key,NULL) == 0 ? WizardTrue : WizardFalse);
#elif defined(MAGICKORE_HAVE_WINTHREADS)
  *key=TlsAlloc();
  return(*key != TLS_OUT_OF_INDEXES ? WizardTrue : WizardFalse);
#else
  *key=AcquireAlignedMemory(1,sizeof(key));
  return(*key != (void *) NULL ? WizardTrue : WizardFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k D e l e t e T h r e a d K e y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardDeleteThreadKey() deletes a thread key.
%
%  The format of the AcquireAESInfo method is:
%
%      WizardBooleanType WizardDeleteThreadKey(WizardThreadKey key)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
*/
WizardExport WizardBooleanType WizardDeleteThreadKey(WizardThreadKey key)
{
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  return(pthread_key_delete(key) == 0 ? WizardTrue : WizardFalse);
#elif defined(MAGICKORE_HAVE_WINTHREADS)
  return(TlsFree(key) != 0 ? WizardTrue : WizardFalse);
#else
  key=(WizardThreadKey) RelinquishWizardMemory(key);
  return(WizardTrue);
#endif

}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t T h r e a d V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardGetThreadValue() returns a value associated with the thread key.
%
%  The format of the WizardGetThreadValue method is:
%
%      void *WizardGetThreadValue(WizardThreadKey key)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
*/
WizardExport void *WizardGetThreadValue(WizardThreadKey key)
{
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  return(pthread_getspecific(key));
#elif defined(MAGICKORE_HAVE_WINTHREADS)
  return(TlsGetValue(key));
#else
  return((void *) (*key));
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t T h r e a d V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardSetThreadValue() associates a value with the thread key.
%
%  The format of the WizardSetThreadValue method is:
%
%      WizardBooleanType WizardSetThreadValue(WizardThreadKey key,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o key: the thread key.
%
%    o value: the value
%
*/
WizardExport WizardBooleanType WizardSetThreadValue(WizardThreadKey key,
  const void *value)
{
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  return(pthread_setspecific(key,value) == 0 ? WizardTrue : WizardFalse);
#elif defined(MAGICKORE_HAVE_WINTHREADS)
  return(TlsSetValue(key,(void *) value) != 0 ? WizardTrue : WizardFalse);
#else
  *key=(size_t) value;
  return(WizardTrue);
#endif
}
