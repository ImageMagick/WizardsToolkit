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
#ifndef _WIZARDSTOOLKIT_THREAD_H
#define _WIZARDSTOOLKIT_THREAD_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
typedef pthread_t WizardThreadType;
#elif defined(WIZARDSTOOLKIT_HAVE_WINTHREADS)
typedef DWORD WizardThreadType;
#else
typedef pid_t WizardThreadType;
#endif

#if defined(WIZARDSTOOLKIT_THREAD_SUPPORT)
typedef pthread_key_t WizardThreadKey;
#elif defined(MAGICKORE_HAVE_WINTHREADS)
typedef DWORD WizardThreadKey;
#else
typedef size_t *WizardThreadKey;
#endif

extern WizardExport WizardBooleanType
  WizardCreateThreadKey(WizardThreadKey *),
  WizardDeleteThreadKey(WizardThreadKey),
  WizardSetThreadValue(WizardThreadKey,const void *);

extern WizardExport void
  *WizardGetThreadValue(WizardThreadKey);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
