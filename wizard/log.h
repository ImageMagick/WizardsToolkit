/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit log methods.
*/
#ifndef _WIZARDSTOOLKIT_LOG_H
#define _WIZARDSTOOLKIT_LOG_H

#include <stdarg.h>
#include "wizard/exception.h"
#include "wizard/timer.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(GetWizardModule)
# define GetWizardModule()  __FILE__,__func__,(unsigned long) __LINE__
#endif

#define WizardLogFilename  "log.xml"

typedef enum
{
  UndefinedEvents,
  NoEvents = 0x0000,
  TraceEvent = 0x0001,
  AnnotateEvent = 0x0002,
  BlobEvent = 0x0004,
  CacheEvent = 0x0008,
  CoderEvent = 0x0010,
  ConfigureEvent = 0x0020,
  DeprecateEvent = 0x0040,
  ExceptionEvent = 0x0080,
  LocaleEvent = 0x0100,
  ModuleEvent = 0x0200,
  ResourceEvent = 0x0400,
  TransformEvent = 0x0800,
  WarningEvent = 0x1000,
  UserEvent = 0x2000,
  AllEvents = 0x7fffffff
} LogEventType;

typedef struct _LogInfo
  LogInfo;

extern WizardExport char
  **GetLogList(const char *,size_t *,ExceptionInfo *);

extern WizardExport const LogInfo
  *GetLogInfo(const char *,ExceptionInfo *),
  **GetLogInfoList(const char *,size_t *,ExceptionInfo *);

extern WizardExport LogEventType
  SetLogEventMask(const char *);

extern WizardExport WizardBooleanType
  IsEventLogging(void),
  ListLogInfo(FILE *,ExceptionInfo *),
  LogComponentGenesis(void),
  LogWizardEvent(const LogEventType,const char *,const char *,
    const size_t,const char *,...) 
    wizard_attribute((__format__ (__printf__,5,6))),
  LogWizardEventList(const LogEventType,const char *,const char *,
    const size_t,const char *,va_list) 
    wizard_attribute((__format__ (__printf__,5,0)));

extern WizardExport void
  CloseWizardLog(void),
  LogComponentTerminus(void),
  SetLogFormat(const char *),
  SetLogPreamble(const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
