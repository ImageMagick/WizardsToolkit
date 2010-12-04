/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit Exception Methods.
*/
#ifndef _WIZARDSTOOLKIT_EXCEPTION_H
#define _WIZARDSTOOLKIT_EXCEPTION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/semaphore.h"

typedef enum
{
  RandomDomain = 0,
  HashDomain = 1,
  EntropyDomain = 2,
  RepairDomain = 3,
  MACDomain = 4,
  CipherDomain = 5,
  KeymapDomain = 6,
  AuthenticateDomain = 7,
  KeyringDomain = 8,
  ParseDomain = 9,
  UserDomain = 10,
  CacheDomain = 11,
  LogDomain = 12,
  StringDomain = 13,
  ResourceDomain = 14,
  MiscellaneousDomain = 15
} DomainType;

typedef enum
{
  MemoryError = 0,
  EnumerateError = 1,
  AuthenticateUserError = 2,
  KeyError = 3,
  HashIOError = 4,
  TimeError = 5,
  TamperError = 6,
  AssertError = 7
} ErrorType;

typedef enum
{
  UndefinedException,
  WarningException = 300,
  OptionWarning = 305,
  RandomWarning = 310,
  HashWarning = 315,
  MACWarning = 320,
  EntropyWarning = 325,
  ConfigureWarning = 330,
  CipherWarning = 335,
  KeymapWarning = 340,
  AuthenticateWarning = 345,
  KeyringWarning = 350,
  ParseWarning = 355,
  UserWarning = 360,
  SplayTreeWarning = 365,
  HashmapWarning = 370,
  LogWarning = 375,
  StringWarning = 380,
  FileWarning = 385,
  BlobWarning = 390,
  ResourceWarning = 395,
  ErrorException = 400,
  OptionError = 405,
  RandomError = 410,
  HashError = 415,
  MACError = 420,
  EntropyError = 425,
  ConfigureError = 430,
  CipherError = 435,
  KeymapError = 440,
  AuthenticateError = 445,
  KeyringError = 450,
  ParseError = 455,
  UserError = 460,
  SplayTreeError = 465,
  HashmapError = 470,
  LogError = 475,
  StringError = 480,
  FileError = 485,
  BlobError = 490,
  ResourceError = 495,
  FatalErrorException = 700,
  OptionFatalError = 705,
  RandomFatalError = 710,
  HashFatalError = 715,
  MACFatalError = 720,
  EntropyFatalError = 725,
  ConfigureFatalError = 730,
  CipherFatalError = 735,
  KeymapFatalError = 740,
  AuthenticateFatalError = 745,
  KeyringFatalError = 750,
  ParseFatalError = 755,
  UserFatalError = 760,
  SplayTreeFatalError = 765,
  HashmapFatalError = 770,
  LogFatalError = 775,
  StringFatalError = 780,
  FileFatalError = 785,
  BlobFatalError = 790,
  ResourceFatalError = 795
} ExceptionType;

typedef struct _ExceptionInfo
  ExceptionInfo;

typedef void
  (*ErrorHandler)(const ExceptionType,const char *,const char *);

typedef void
  (*FatalErrorHandler)(const ExceptionType,const char *,const char *);

typedef void
  (*WarningHandler)(const ExceptionType,const char *,const char *);

/*
  Exception declarations.
*/
extern WizardExport char
  *GetExceptionMessage(const int);

extern WizardExport const char
  *GetLocaleExceptionMessage(const ExceptionType,const char *),
  *GetLocaleMessage(const char *);

extern WizardExport ErrorHandler
  SetErrorHandler(ErrorHandler);

extern WizardExport ExceptionInfo
  *AcquireExceptionInfo(void),
  *DestroyExceptionInfo(ExceptionInfo *);

extern WizardExport ExceptionType
  GetExceptionSeverity(const ExceptionInfo *);

extern WizardExport FatalErrorHandler
  SetFatalErrorHandler(FatalErrorHandler);

extern WizardExport WizardBooleanType
  ListLocaleInfo(FILE *,ExceptionInfo *),
  ThrowException(ExceptionInfo *,const ExceptionType,const char *,
    const char *),
  ThrowWizardException(ExceptionInfo *,const char *,const char *,
    const size_t,const ExceptionType,const char *,...)
    wizard_attribute((format (printf,6,7)));

extern WizardExport void
  CatchException(ExceptionInfo *),
  ClearWizardException(ExceptionInfo *),
  DestroyLocaleInfo(void),
  GetExceptionInfo(ExceptionInfo *),
  InheritException(ExceptionInfo *,const ExceptionInfo *),
  WizardError(const ExceptionType,const char *,const char *),
  WizardFatalError(const ExceptionType,const char *,const char *),
  WizardWarning(const ExceptionType,const char *,const char *);

extern WizardExport WarningHandler
  SetWarningHandler(WarningHandler);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
