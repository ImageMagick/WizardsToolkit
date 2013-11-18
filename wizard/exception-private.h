/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit private exception methods.
*/
#ifndef _WIZARDSTOOLKIT_EXCEPTION_PRIVATE_H
#define _WIZARDSTOOLKIT_EXCEPTION_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/log.h"
#include "wizard/string_.h"
#include "wizard/wizard.h"

#define CatchWizardException(severity,tag,context) \
{ \
  ExceptionInfo \
    *exception; \
 \
  exception=AcquireExceptionInfo(); \
  (void) ThrowWizardException(exception,GetWizardModule(),severity,tag,context); \
  CatchException(exception); \
  exception=DestroyExceptionInfo(exception); \
}

#if defined(WIZARDSTOOLKIT_Debug)
#define WizardAssert(domain,predicate) \
{ \
  assert(predicate); \
}
#else 
#define WizardAssert(domain,predicate) \
{ \
  if ((WizardBooleanType) (predicate) == WizardFalse) \
    ThrowWizardFatalError(domain,AssertError); \
}
#endif 

#define ThrowFileException(exception,severity,context) \
{ \
  char \
     *message; \
 \
  message=GetExceptionMessage(errno); \
  (void) ThrowWizardException(exception,GetWizardModule(),severity, \
    "file exception `%s': %s",context,message); \
  message=DestroyString(message); \
}

#define ThrowWizardFatalError(domain,error) \
{ \
  char \
    context[MaxTextExtent], \
    tag[MaxTextExtent]; \
 \
  (void) FormatLocaleString(tag,MaxTextExtent,"%ld\n",(long) domain); \
  (void) FormatLocaleString(context,MaxTextExtent,"%ld\n",(long) error); \
  CatchWizardException(UndefinedException,tag,context); \
  WizardsToolkitTerminus(); \
  _exit((domain << 3) | error); \
}

#define ThrowFatalException(severity,tag) \
{ \
  char \
     *message; \
 \
  message=GetExceptionMessage(errno); \
  CatchWizardException(severity,tag,message); \
  message=DestroyString(message); \
  WizardsToolkitTerminus(); \
  _exit(127); \
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
