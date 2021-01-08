/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        EEEEE  X   X   CCCC  EEEEE  PPPP  TTTTT  IIIII   OOO   N   N         %
%        E       X X   C      E      P   P   T      I    O   O  NN  N         %
%        EEE      X    C      EEE    PPPP    T      I    O   O  N N N         %
%        E       X X   C      E      P       T      I    O   O  N  NN         %
%        EEEEE   X  X   CCCC  EEEEE  P       T    IIIII   OOO   N   N         %
%                                                                             %
%                                                                             %
%                    Wizards Toolkit Exception Methods                        %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                                July 1993                                    %
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
#include "wizard/client.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hashmap.h"
#include "wizard/log.h"
#include "wizard/memory_.h"
#include "wizard/string_.h"
#include "wizard/utility.h"
#include "wizard/wizard.h"

/*
  Typedef declarations.
*/
struct _ExceptionInfo
{
  ExceptionType
    severity;

  char
    *reason,
    *description;

  void
    *exceptions;

  WizardBooleanType
    relinquish;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

/*
  Forward declarations.
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static void
  DefaultErrorHandler(const ExceptionType,const char *,const char *),
  DefaultFatalErrorHandler(const ExceptionType,const char *,const char *),
  DefaultWarningHandler(const ExceptionType,const char *,const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

/*
  Global declarations.
*/
static ErrorHandler
  error_handler = DefaultErrorHandler;

static FatalErrorHandler
  fatal_error_handler = DefaultFatalErrorHandler;

static WarningHandler
  warning_handler = DefaultWarningHandler;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e E x c e p t i o n I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireExceptionInfo() allocates the ExceptionInfo structure.
%
%  The format of the AcquireExceptionInfo method is:
%
%      ExceptionInfo *AcquireExceptionInfo(void)
%
*/
WizardExport ExceptionInfo *AcquireExceptionInfo(void)
{
  ExceptionInfo
    *exception;

  exception=(ExceptionInfo *) AcquireWizardMemory(sizeof(*exception));
  if (exception == (ExceptionInfo *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  (void) ResetWizardMemory(exception,0,sizeof(*exception));
  GetExceptionInfo(exception);
  exception->relinquish=WizardTrue;
  exception->signature=WizardSignature;
  return(exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l e a r W i z a r d E x c e p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearWizardException() clears any exception that may not have been caught
%  yet.
%
%  The format of the ClearWizardException method is:
%
%      ClearWizardException(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: The exception info.
%
*/

static void *DestroyExceptionElement(void *exception)
{
  register ExceptionInfo
    *p;

  p=(ExceptionInfo *) exception;
  if (p->reason != (char *) NULL)
    p->reason=(char *) RelinquishWizardMemory(p->reason);
  if (p->description != (char *) NULL)
    p->description=(char *) RelinquishWizardMemory(p->description);
  p=(ExceptionInfo *) RelinquishWizardMemory(p);
  return((void *) NULL);
}

WizardExport void ClearWizardException(ExceptionInfo *exception)
{
  register ExceptionInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == WizardSignature);
  if (exception->exceptions == (void *) NULL)
    return;
  LockSemaphoreInfo(exception->semaphore);
  p=(ExceptionInfo *) RemoveLastElementFromLinkedList((LinkedListInfo *)
    exception->exceptions);
  while (p != (ExceptionInfo *) NULL)
  {
    p=(ExceptionInfo *) DestroyExceptionElement(p);
    p=(ExceptionInfo *) RemoveLastElementFromLinkedList((LinkedListInfo *)
      exception->exceptions);
  }
  exception->severity=UndefinedException;
  exception->reason=(char *) NULL;
  exception->description=(char *) NULL;
  UnlockSemaphoreInfo(exception->semaphore);
  errno=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C a t c h E x c e p t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CatchException() returns if no exceptions is found otherwise it reports
%  the exception as a warning, error, or fatal depending on the severity.
%
%  The format of the CatchException method is:
%
%      CatchException(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: The exception info.
%
*/
WizardExport void CatchException(ExceptionInfo *exception)
{
  register const ExceptionInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == WizardSignature);
  if (exception->exceptions == (void *) NULL)
    return;
  LockSemaphoreInfo(exception->semaphore);
  ResetLinkedListIterator((LinkedListInfo *) exception->exceptions);
  p=(const ExceptionInfo *) GetNextValueInLinkedList((LinkedListInfo *)
    exception->exceptions);
  while (p != (const ExceptionInfo *) NULL)
  {
    if ((p->severity >= WarningException) && (p->severity < ErrorException))
      WizardWarning(p->severity,p->reason,p->description);
    if ((p->severity >= ErrorException) && (p->severity < FatalErrorException))
      WizardError(p->severity,p->reason,p->description);
    if (exception->severity >= FatalErrorException)
      WizardFatalError(p->severity,p->reason,p->description);
    p=(const ExceptionInfo *) GetNextValueInLinkedList((LinkedListInfo *)
      exception->exceptions);
  }
  UnlockSemaphoreInfo(exception->semaphore);
  ClearWizardException(exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f a u l t E r r o r H a n d l e r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefaultErrorHandler() displays an error reason.
%
%  The format of the DefaultErrorHandler method is:
%
%      void WizardError(const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
static void DefaultErrorHandler(const ExceptionType wizard_unused(severity),
  const char *reason,const char *description)
{
  if (reason == (char *) NULL)
    return;
  (void) fprintf(stderr,"%s: %s",GetClientName(),reason);
  if (description != (char *) NULL)
    (void) fprintf(stderr," (%s)",description);
  (void) fprintf(stderr,".\n");
  (void) fflush(stderr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f a u l t F a t a l E r r o r H a n d l e r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefaultFatalErrorHandler() displays an error reason and then terminates the
%  program.
%
%  The format of the DefaultFatalErrorHandler method is:
%
%      void WizardFatalError(const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o severity: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
static void DefaultFatalErrorHandler(const ExceptionType severity,
  const char *reason,const char *description)
{
  if (reason == (char *) NULL)
    return;
  (void) fprintf(stderr,"%s: %s",GetClientName(),reason);
  if (description != (char *) NULL)
    (void) fprintf(stderr," (%s)",description);
  (void) fprintf(stderr,".\n");
  (void) fflush(stderr);
  WizardsToolkitTerminus();
  exit(severity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e f a u l t W a r n i n g H a n d l e r                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DefaultWarningHandler() displays a warning reason.
%
%  The format of the DefaultWarningHandler method is:
%
%      void DefaultWarningHandler(const ExceptionType warning,
%        const char *reason,const char *description)
%
%  A description of each parameter follows:
%
%    o warning: Specifies the numeric warning category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
static void DefaultWarningHandler(const ExceptionType wizard_unused(severity),
  const char *reason,const char *description)
{
  if (reason == (char *) NULL)
    return;
  (void) fprintf(stderr,"%s: %s",GetClientName(),reason);
  if (description != (char *) NULL)
    (void) fprintf(stderr," (%s)",description);
  (void) fprintf(stderr,".\n");
  (void) fflush(stderr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y E x c e p t i o n I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyExceptionInfo() deallocates memory associated with an exception.
%
%  The format of the DestroyExceptionInfo method is:
%
%      ExceptionInfo *DestroyExceptionInfo(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: The exception info.
%
*/
WizardExport ExceptionInfo *DestroyExceptionInfo(ExceptionInfo *exception)
{
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == WizardSignature);
  LockSemaphoreInfo(exception->semaphore);
  exception->severity=UndefinedException;
  if (exception->exceptions != (void *) NULL)
    exception->exceptions=(void *) DestroyLinkedList((LinkedListInfo *)
      exception->exceptions,DestroyExceptionElement);
  exception->signature=(~WizardSignature);
  UnlockSemaphoreInfo(exception->semaphore);
  RelinquishSemaphoreInfo(&exception->semaphore);
  if (exception->relinquish != WizardFalse)
    exception=(ExceptionInfo *) RelinquishWizardMemory(exception);
  return(exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E x c e p t i o n I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetExceptionInfo() initializes an exception to default values.
%
%  The format of the GetExceptionInfo method is:
%
%      GetExceptionInfo(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: The exception info.
%
*/
WizardExport void GetExceptionInfo(ExceptionInfo *exception)
{
  assert(exception != (ExceptionInfo *) NULL);
  (void) ResetWizardMemory(exception,0,sizeof(*exception));
  exception->severity=UndefinedException;
  exception->exceptions=(void *) NewLinkedList(0);
  exception->semaphore=AcquireSemaphoreInfo();
  exception->signature=WizardSignature;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E x c e p t i o n M e s s a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetExceptionMessage() returns the error message defined by the specified
%  error code.
%
%  The format of the GetExceptionMessage method is:
%
%      char *GetExceptionMessage(const int error)
%
%  A description of each parameter follows:
%
%    o error: the error code.
%
*/
WizardExport char *GetExceptionMessage(const int error)
{
 char
   exception[WizardPathExtent];

  *exception='\0';
#if defined(WIZARDSTOOLKIT_HAVE_STRERROR_R)
#if !defined(WIZARDSTOOLKIT_STRERROR_R_CHAR_P)
  (void) strerror_r(error,exception,sizeof(exception));
#else
  (void) CopyWizardString(exception,strerror_r(error,exception,
    sizeof(exception)),sizeof(exception));
#endif
#else
  (void) CopyWizardString(exception,strerror(error),sizeof(exception));
#endif
  return(ConstantString(exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e E x c e p t i o n M e s s a g e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleExceptionMessage() converts a enumerated exception severity and tag
%  to a message in the current locale.
%
%  The format of the GetLocaleExceptionMessage method is:
%
%      const char *GetLocaleExceptionMessage(const ExceptionType severity,
%        const char *tag)
%
%  A description of each parameter follows:
%
%    o severity: the severity of the exception.
%
%    o tag: the message tag.
%
*/

static const char *ExceptionSeverityToTag(const ExceptionType severity)
{
  switch (severity)
  {
    case OptionWarning: return("Option/Warning/");
    case RandomWarning: return("Random/Warning/");
    case HashWarning: return("Hash/Warning/");
    case MACWarning: return("MAC/Warning/");
    case EntropyWarning: return("Entropy/Warning/");
    case ConfigureWarning: return("Configure/Warning/");
    case CipherWarning: return("Cipher/Warning/");
    case KeymapWarning: return("Keymap/Warning/");
    case AuthenticateWarning: return("Authenticate/Warning/");
    case KeyringWarning: return("Keyring/Warning/");
    case ParseWarning: return("Parse/Warning/");
    case UserWarning: return("User/Warning/");
    case SplayTreeWarning: return("SplayTree/Warning/");
    case HashmapWarning: return("Hashmap/Warning/");
    case LogWarning: return("Log/Warning/");
    case StringWarning: return("String/Warning/");
    case FileWarning: return("File/Warning/");
    case BlobWarning: return("Blob/Warning/");
    case ResourceWarning: return("ResourceLimit/Warning/");
    case OptionError: return("Option/Error/");
    case RandomError: return("Random/Error/");
    case HashError: return("Hash/Error/");
    case MACError: return("MAC/Error/");
    case EntropyError: return("Entropy/Error/");
    case ConfigureError: return("Configure/Error/");
    case CipherError: return("Cipher/Error/");
    case KeymapError: return("Keymap/Error/");
    case AuthenticateError: return("Authenticate/Error/");
    case KeyringError: return("Keyring/Error/");
    case ParseError: return("Parse/Error/");
    case UserError: return("User/Error/");
    case SplayTreeError: return("SplayTree/Error/");
    case StringError: return("String/Error/");
    case FileError: return("File/Error/");
    case BlobError: return("Blob/Error/");
    case ResourceError: return("ResourceLimit/Error/");
    case OptionFatalError: return("Option/FatalError/");
    case RandomFatalError: return("Random/FatalError/");
    case HashFatalError: return("Hash/FatalError/");
    case MACFatalError: return("MAC/FatalError/");
    case EntropyFatalError: return("Entropy/FatalError/");
    case ConfigureFatalError: return("Configure/FatalError/");
    case CipherFatalError: return("Cipher/FatalError/");
    case KeymapFatalError: return("Keymap/FatalError/");
    case AuthenticateFatalError: return("Authenticate/FatalError/");
    case KeyringFatalError: return("Keyring/FatalError/");
    case ParseFatalError: return("Parse/FatalError/");
    case UserFatalError: return("User/FatalError/");
    case SplayTreeFatalError: return("SplayTree/FatalError/");
    case HashmapFatalError: return("Hashmap/FatalError/");
    case LogFatalError: return("Log/FatalError/");
    case StringFatalError: return("String/FatalError/");
    case FileFatalError: return("File/FatalError/");
    case BlobFatalError: return("Blob/FatalError/");
    case ResourceFatalError: return("ResourceLimit/FatalError/");
    default: break;
  }
  return("");
}

WizardExport const char *GetLocaleExceptionMessage(const ExceptionType severity,
  const char *tag)
{
#if defined(WIZARDSTOOLKIT_LOCALE)
  char
    message[WizardPathExtent];

  const char
    *locale_message;

  assert(tag != (const char *) NULL);
  (void) FormatLocaleString(message,WizardPathExtent,"Exception/%s%s",
    ExceptionSeverityToTag(severity),tag);
  locale_message=GetLocaleMessage(message);
  if (locale_message == (const char *) NULL)
    return(tag);
  if (locale_message == message)
    return(tag);
  return(locale_message);
#else
  return(tag);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E x c e p t i o n S e v e r i t y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetExceptionSeverity() returns the exception severity.
%
%  The format of the GetExceptionSeverity method is:
%
%      ExceptionType GetExceptionSeverity(const ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o exception: The exception.
%
*/
WizardExport ExceptionType GetExceptionSeverity(const ExceptionInfo *exception)
{
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == WizardSignature);
  return(exception->severity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n h e r i t E x c e p t i o n                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InheritException() inherits an exception from a related exception.
%
%  The format of the InheritException method is:
%
%      InheritException(ExceptionInfo *exception,const ExceptionInfo *relative)
%
%  A description of each parameter follows:
%
%    o exception: The exception info.
%
%    o relative: The related exception info.
%
%
*/
WizardExport void InheritException(ExceptionInfo *exception,
  const ExceptionInfo *relative)
{
  register const ExceptionInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == WizardSignature);
  assert(relative != (ExceptionInfo *) NULL);
  assert(relative->signature == WizardSignature);
  if (relative->exceptions == (void *) NULL)
    return;
  LockSemaphoreInfo(relative->semaphore);
  ResetLinkedListIterator((LinkedListInfo *) relative->exceptions);
  p=(const ExceptionInfo *) GetNextValueInLinkedList((LinkedListInfo *)
    relative->exceptions);
  while (p != (const ExceptionInfo *) NULL)
  {
    (void) ThrowException(exception,p->severity,p->reason,p->description);
    p=(const ExceptionInfo *) GetNextValueInLinkedList((LinkedListInfo *)
      relative->exceptions);
  }
  UnlockSemaphoreInfo(relative->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W i z a r d E r r o r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardError() calls the exception handler methods with an error reason.
%
%  The format of the WizardError method is:
%
%      void WizardError(const ExceptionType error,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o exception: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
%
*/
WizardExport void WizardError(const ExceptionType error,const char *reason,
  const char *description)
{
  if (error_handler != (ErrorHandler) NULL)
    (*error_handler)(error,reason,description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W i z a r d F a t al E r r o r                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardFatalError() calls the fatal exception handler methods with an error
%  reason.
%
%  The format of the WizardError method is:
%
%      void WizardFatalError(const ExceptionType error,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o exception: Specifies the numeric error category.
%
%    o reason: Specifies the reason to display before terminating the
%      program.
%
%    o description: Specifies any description to the reason.
%
*/
WizardExport void WizardFatalError(const ExceptionType error,const char *reason,
  const char *description)
{
  if (fatal_error_handler != (ErrorHandler) NULL)
    (*fatal_error_handler)(error,reason,description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W i z a r d W a r n i n g                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardWarning() calls the warning handler methods with a warning reason.
%
%  The format of the WizardWarning method is:
%
%      void WizardWarning(const ExceptionType warning,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o warning: The warning severity.
%
%    o reason: Define the reason for the warning.
%
%    o description: Describe the warning.
%
*/
WizardExport void WizardWarning(const ExceptionType warning,const char *reason,
  const char *description)
{
  if (warning_handler != (WarningHandler) NULL)
    (*warning_handler)(warning,reason,description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t E r r o r H a n d l e r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetErrorHandler() sets the exception handler to the specified method
%  and returns the previous exception handler.
%
%  The format of the SetErrorHandler method is:
%
%      ErrorHandler SetErrorHandler(ErrorHandler handler)
%
%  A description of each parameter follows:
%
%    o handler: The method to handle errors.
%
*/
WizardExport ErrorHandler SetErrorHandler(ErrorHandler handler)
{
  ErrorHandler
    previous_handler;

  previous_handler=error_handler;
  error_handler=handler;
  return(previous_handler);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t F a t a l E r r o r H a n d l e r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetFatalErrorHandler() sets the fatal exception handler to the specified
%  method and returns the previous fatal exception handler.
%
%  The format of the SetErrorHandler method is:
%
%      ErrorHandler SetErrorHandler(ErrorHandler handler)
%
%  A description of each parameter follows:
%
%    o handler: The method to handle errors.
%
*/
WizardExport FatalErrorHandler SetFatalErrorHandler(FatalErrorHandler handler)
{
  FatalErrorHandler
    previous_handler;

  previous_handler=fatal_error_handler;
  fatal_error_handler=handler;
  return(previous_handler);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t W a r n i n g H a n d l e r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetWarningHandler() sets the warning handler to the specified method
%  and returns the previous warning handler.
%
%  The format of the SetWarningHandler method is:
%
%      ErrorHandler SetWarningHandler(ErrorHandler handler)
%
%  A description of each parameter follows:
%
%    o handler: The method to handle warnings.
%
*/
WizardExport WarningHandler SetWarningHandler(WarningHandler handler)
{
  WarningHandler
    previous_handler;

  previous_handler=warning_handler;
  warning_handler=handler;
  return(previous_handler);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T h r o w E x c e p t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ThrowException() throws an exception with the specified severity code,
%  reason, and optional description.
%
%  The format of the ThrowException method is:
%
%      WizardBooleanType ThrowException(ExceptionInfo *exception,
%        const ExceptionType severity,const char *reason,
%        const char *description)
%
%  A description of each parameter follows:
%
%    o exception: The exception info.
%
%    o severity: The severity of the exception.
%
%    o reason: The reason for the exception.
%
%    o description: The exception description.
%
*/
WizardExport WizardBooleanType ThrowException(ExceptionInfo *exception,
  const ExceptionType severity,const char *reason,const char *description)
{
  register ExceptionInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == WizardSignature);
  if (exception->exceptions == (void *) NULL)
    return(WizardTrue);
  LockSemaphoreInfo(exception->semaphore);
  p=(ExceptionInfo *) GetLastValueInLinkedList((LinkedListInfo *)
    exception->exceptions);
  if ((p != (ExceptionInfo *) NULL) && (p->severity == severity) &&
      (LocaleCompare(exception->reason,reason) == 0) &&
      (LocaleCompare(exception->description,description) == 0))
    {
      UnlockSemaphoreInfo(exception->semaphore);
      return(WizardTrue);
    }
  p=(ExceptionInfo *) AcquireWizardMemory(sizeof(*p));
  if (p == (ExceptionInfo *) NULL)
    {
      UnlockSemaphoreInfo(exception->semaphore);
      ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
    }
  (void) ResetWizardMemory(p,0,sizeof(*p));
  p->severity=severity;
  if (reason != (const char *) NULL)
    p->reason=ConstantString(reason);
  if (description != (const char *) NULL)
    p->description=ConstantString(description);
  p->signature=WizardSignature;
  (void) AppendValueToLinkedList((LinkedListInfo *) exception->exceptions,p);
  exception->severity=p->severity;
  exception->reason=p->reason;
  exception->description=p->description;
  UnlockSemaphoreInfo(exception->semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T h r o w W i z a r d E x c e p t i o n                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ThrowWizardException logs an exception as determined by the log configuration
%  file.  If an error occurs, WizardFalse is returned otherwise WizardTrue.
%
%  The format of the ThrowWizardException method is:
%
%      WizardBooleanType ThrowWizardException(ExceptionInfo *exception,
%        const char *module,const char *function,const size_t line,
%        const ExceptionType severity,const char *format,...)
%
%  A description of each parameter follows:
%
%    o exception: The exception info.
%
%    o filename: The source module filename.
%
%    o function: The function name.
%
%    o line: The line number of the source module.
%
%    o severity: Specifies the numeric error category.
%
%    o format: The output format.
%
*/

WizardExport WizardBooleanType ThrowWizardExceptionList(
  ExceptionInfo *exception,const char *module,const char *function,
  const size_t line,const ExceptionType severity,const char *format,
  va_list operands)
{
  char
    message[WizardPathExtent],
    reason[WizardPathExtent];

  int
    n;

  WizardBooleanType
    status;

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == WizardSignature);
#if defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF)
  n=vsnprintf(reason,WizardPathExtent,format,operands);
#else
  n=vsprintf(reason,format,operands);
#endif
  if (n < 0)
    reason[WizardPathExtent-1]='\0';
  status=LogWizardEvent(exception->severity >= ErrorException ?
    ExceptionEvent : WarningEvent,module,function,line,"%s",reason);
  (void) FormatLocaleString(message,WizardPathExtent,"%s @ %s/%s/%.20g",reason,
    module,function,(double) line);
  (void) ThrowException(exception,severity,message,(char *) NULL);
  return(status);
}

WizardExport WizardBooleanType ThrowWizardException(ExceptionInfo *exception,
  const char *module,const char *function,const size_t line,
  const ExceptionType severity,const char *format,...)
{
  WizardBooleanType
    status;

  va_list
    operands;

  va_start(operands,format);
  status=ThrowWizardExceptionList(exception,module,function,line,severity,
    format,operands);
  va_end(operands);
  return(status);
}
