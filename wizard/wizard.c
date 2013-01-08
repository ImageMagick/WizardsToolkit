/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  W   W  IIIII  ZZZZZ   AAA   RRRR   DDDD                    %
%                  W   W    I       ZZ  A   A  R   R  D  KD                   %
%                  W W W    I     ZZZ   AAAAA  RRRR   D   D                   %
%                  WW WW    I    ZZ     A   A  R R    D   D                   %
%                  W   W  IIIII  ZZZZZ  A   A  R   R  DDDD                    %
%                                                                             %
%                         Wizard's Toolkit Wizard Methods                     %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/client.h"
#include "wizard/configure.h"
#include "wizard/hash.h"
#include "wizard/log.h"
#include "wizard/magick.h"
#include "wizard/memory_.h"
#include "wizard/mime.h"
#include "wizard/random_.h"
#include "wizard/resource_.h"
#include "wizard/semaphore.h"
#include "wizard/semaphore-private.h"
#include "wizard/string_.h"
#include "wizard/utility.h"
#include "wizard/wizard.h"

/*
  Define declarations.
*/
#if !defined(WIZARDSTOOLKIT_RETSIGTYPE)
# define WIZARDSTOOLKIT_RETSIGTYPE  void
#endif
#if !defined(SIG_DFL)
# define SIG_DFL  ((SignalHandler *) 0)
#endif
#if !defined(SIG_ERR)
# define SIG_ERR  ((SignalHandler *) -1)
#endif
#if !defined(SIGMAX)
#define SIGMAX  64
#endif

/*
  Typedef declarations.
*/
typedef WIZARDSTOOLKIT_RETSIGTYPE
  SignalHandler(int);

/*
  Global declarations.
*/
static SignalHandler
  *signal_handlers[SIGMAX] = { (SignalHandler *) NULL };

static volatile WizardBooleanType
  instantiate_wizard = WizardFalse,
  instantiate_wizardstoolkit = WizardFalse;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W i z a r d s T o o l k i t G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardsToolkitGenesis() initializes the Wizard's Toolkit environment.
%
%  The format of the WizardsToolkitGenesis function is:
%
%      WizardsToolkitGenesis(const char *path)
%
%  A description of each parameter follows:
%
%    o path: The execution path of the current Wizard's Toolkit client.
%
*/

static void WizardExitHandler(void)
{
  WizardsToolkitTerminus();
}

static SignalHandler *SetWizardSignalHandler(int signal_number,
  SignalHandler *handler)
{
#if defined(WIZARDSTOOLKIT_HAVE_SIGACTION) && defined(WIZARDSTOOLKIT_HAVE_SIGEMPTYSET)
  int
    status;

  sigset_t
    mask;

  struct sigaction
    action,
    previous_action;

  sigemptyset(&mask);
  sigaddset(&mask,signal_number);
  sigprocmask(SIG_BLOCK,&mask,NULL);
  action.sa_mask=mask;
  action.sa_handler=handler;
  action.sa_flags=0;
#if defined(SA_INTERRUPT)
  action.sa_flags|=SA_INTERRUPT;
#endif
  status=sigaction(signal_number,&action,&previous_action);
  if (status < 0)
    return(SIG_ERR);
  sigprocmask(SIG_UNBLOCK,&mask,NULL);
  return(previous_action.sa_handler);
#else
  return(signal(signal_number,handler));
#endif
}

static void WizardSignalHandler(int signal_number)
{
#if !defined(WIZARDSTOOLKIT_HAVE_SIGACTION)
  (void) signal(signal_number,SIG_IGN);
#endif
  AsynchronousResourceComponentTerminus();
  instantiate_wizard=WizardFalse;
  (void) SetWizardSignalHandler(signal_number,signal_handlers[signal_number]);
#if !defined(WIZARDSTOOLKIT_HAVE__EXIT)
  exit(signal_number);
#else
#if defined(SIGHUP)
  if (signal_number == SIGHUP)
    exit(signal_number);
#endif
#if defined(SIGBREAK)
  if (signal_number == SIGBREAK)
    exit(signal_number);
#endif
#if defined(SIGINT) && !defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  if (signal_number == SIGINT)
    exit(signal_number);
#endif
#if defined(SIGTERM)
  if (signal_number == SIGTERM)
    exit(signal_number);
#endif
#if defined(SIGSEGV)
  if (signal_number == SIGSEGV)
    exit(signal_number);
#endif
#if defined(WIZARDSTOOLKIT_HAVE_RAISE)
  if (signal_handlers[signal_number] != WizardSignalHandler)
    raise(signal_number);
#endif
  _exit(signal_number);  /* do not invoke registered atexit() methods */
#endif
}

static SignalHandler *RegisterWizardSignalHandler(int signal_number)
{
  SignalHandler
    *handler;

  handler=SetWizardSignalHandler(signal_number,WizardSignalHandler);
  if (handler == SIG_ERR)
    return(handler);
  if (handler != SIG_DFL)
    handler=SetWizardSignalHandler(signal_number,handler);
  else
    (void) LogWizardEvent(ConfigureEvent,GetWizardModule(),
      "Register handler for signal: %d",signal_number);
  return(handler);
}

WizardExport void WizardsToolkitGenesis(const char *path)
{
  char
    *events,
    execution_path[MaxTextExtent],
    filename[MaxTextExtent];

  /*
    Initialize the Wizard environment.
  */
  LockWizardMutex();
  if (instantiate_wizardstoolkit != WizardFalse)
    {
      UnlockWizardMutex();
      return;
    }
  (void) setlocale(LC_ALL,"");
  (void) setlocale(LC_NUMERIC,"C");
  (void) SemaphoreComponentGenesis();
  (void) LogComponentGenesis();
  (void) RandomComponentGenesis();
  events=GetEnvironmentValue("WIZARD_DEBUG");
  if (events != (char *) NULL)
    {
      (void) SetLogEventMask(events);
      events=(char *) RelinquishWizardMemory(events);
    }
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#if defined(_DEBUG) && !defined(__BORLANDC__)
  if (IsEventLogging() != WizardFalse)
    {
      int
        debug;

      debug=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
      debug|=_CRTDBG_CHECK_ALWAYS_DF |_CRTDBG_DELAY_FREE_MEM_DF |
        _CRTDBG_LEAK_CHECK_DF;
      if (0)
        {
          debug=_CrtSetDbgFlag(debug);
          _ASSERTE(_CrtCheckMemory());
        }
    }
#endif
#endif
  /*
    Set client name and execution path.
  */
  (void) GetExecutionPath(execution_path,MaxTextExtent);
  if ((path != (const char *) NULL) && (*path != '\0'))
    (void) CopyWizardString(execution_path,path,MaxTextExtent);
  GetPathComponent(execution_path,TailPath,filename);
  (void) SetClientName(filename);
  GetPathComponent(execution_path,HeadPath,execution_path);
  (void) SetClientPath(execution_path);
  /*
    Set signal handlers.
  */
#if defined(SIGABRT)
  if (signal_handlers[SIGABRT] == (SignalHandler *) NULL)
    signal_handlers[SIGABRT]=RegisterWizardSignalHandler(SIGABRT);
#endif
#if defined(SIGBREAK)
  if (signal_handlers[SIGBREAK] == (SignalHandler *) NULL)
    signal_handlers[SIGBREAK]=RegisterWizardSignalHandler(SIGBREAK);
#endif
#if defined(SIGFPE)
  if (signal_handlers[SIGFPE] == (SignalHandler *) NULL)
    signal_handlers[SIGFPE]=RegisterWizardSignalHandler(SIGFPE);
#endif
#if defined(SIGHUP)
  if (signal_handlers[SIGHUP] == (SignalHandler *) NULL)
    signal_handlers[SIGHUP]=RegisterWizardSignalHandler(SIGHUP);
#endif
#if defined(SIGINT) && !defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  if (signal_handlers[SIGINT] == (SignalHandler *) NULL)
    signal_handlers[SIGINT]=RegisterWizardSignalHandler(SIGINT);
#endif
#if defined(SIGQUIT)
  if (signal_handlers[SIGQUIT] == (SignalHandler *) NULL)
    signal_handlers[SIGQUIT]=RegisterWizardSignalHandler(SIGQUIT);
#endif
#if defined(SIGTERM)
  if (signal_handlers[SIGTERM] == (SignalHandler *) NULL)
    signal_handlers[SIGTERM]=RegisterWizardSignalHandler(SIGTERM);
#endif
#if defined(SIGSEGV)
  if (signal_handlers[SIGSEGV] == (SignalHandler *) NULL)
    signal_handlers[SIGSEGV]=RegisterWizardSignalHandler(SIGSEGV);
#endif
#if defined(SIGXCPU)
  if (signal_handlers[SIGXCPU] == (SignalHandler *) NULL)
    signal_handlers[SIGXCPU]=RegisterWizardSignalHandler(SIGXCPU);
#endif
#if defined(SIGXFSZ)
  if (signal_handlers[SIGXFSZ] == (SignalHandler *) NULL)
    signal_handlers[SIGXFSZ]=RegisterWizardSignalHandler(SIGXFSZ);
#endif
  /*
    Initialize wizard resources.
  */
  (void) ConfigureComponentGenesis();
  (void) ResourceComponentGenesis();
  (void) MimeComponentGenesis();
  instantiate_wizardstoolkit=WizardTrue;
  UnlockWizardMutex();
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W i z a r d s T o o l k i t T e r m i n u s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardsToolkitTerminus() destroys the Wizard's Toolkit environment.
%
%  The format of the WizardsToolkitTerminus method is:
%
%      WizardsToolkitTerminus(void)
%
*/
WizardExport void WizardsToolkitTerminus(void)
{
  LockWizardMutex();
  if (instantiate_wizardstoolkit == WizardFalse)
    {
      UnlockWizardMutex();
      return;
    }
  MimeComponentTerminus();
  ResourceComponentTerminus();
  RandomComponentTerminus();
  LogComponentTerminus();
  SemaphoreComponentTerminus();
  instantiate_wizardstoolkit=WizardFalse;
  UnlockWizardMutex();
}
