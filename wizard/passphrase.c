/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    PPPP    AAA   SSSSS  SSSSS  PPPP   H   H  RRRR    AAA   SSSSS  EEEEE     %
%    P   P  A   A  SS     SS     P   P  H   H  R   R  A   A  SS     E         %
%    PPPP   AAAAA   SSS    SSS   PPPP   HHHHH  RRRR   AAAAA   SSS   EEE       %
%    P      A   A     SS     SS  P      H   H  R R    A   A     SS  E         %
%    P      A   A  SSSSS  SSSSS  P      H   H  R  R   A   A  SSSSS  EEEEE     %
%                                                                             %
%                                                                             %
%                     Wizard's Toolkit Passphrase Methods                     %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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

#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/passphrase.h"
#if defined(WIZARDSTOOLKIT_HAVE_TERMIOS_H)
#include <termios.h>
#endif

/*
  Define declarations.
*/
#ifdef TCSASOFT
# define _T_FLUSH  (TCSAFLUSH | TCSASOFT)
#else
# define _T_FLUSH  (TCSAFLUSH)
#endif

#if !defined(_POSIX_VDISABLE) && defined(VDISABLE)
# define _POSIX_VDISABLE  VDISABLE
#endif

#define _PATH_TTY "/dev/tty"

/*
  Typedef declarations.
*/
typedef enum
{
  EchoOffMode    = 0x00,  /* Turn off echo (default). */
  EchoOnMode     = 0x01,  /* Leave echo on. */
  RequireTTYMode = 0x02,  /* Fail if there is no tty. */
  ForceLowerMode = 0x04,  /* Force input to lower case. */
  ForceUpperMode = 0x08,  /* Force input to upper case. */
  SevenBitMode   = 0x10,  /* Strip the high bit from input. */
  StdinMode      = 0x20   /* Read from stdin, not /dev/tty */
} PassphraseMode;

/*
  Global declarations.
*/
static volatile sig_atomic_t
  signal_number = 0;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P h r a s e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPhrase() gets a phrase from the terminal.
%
%  The format of the GetPhrase method is:
%
%      WizardBooleanType GetPhrase(const char *prompt,PassphraseMode flags,
%        StringInfo *phrase)
%
%  A description of each parameter follows:
%
%    o prompt: Prompt the user for the passphrase.
%
%    o flags: modify the behavior of this method by setting one or more of
%      these flags:
%
%        EchoOffMode     Turn off echo (default).
%        EchoOnMode      Leave echo on.
%        RequireTTYMode  Fail if there is no tty.
%        ForceLowerMode  Force input to lower case.
%        ForceUpperMode  Force input to upper case.
%        SevenBitMode    Strip the high bit from input.
%        StdinMode       Read from stdin, not /dev/tty.
%
%    o phrase: The passphrase is returned in this buffer.
%
*/

static void SignalHandler(int signal)
{
  signal_number=(sig_atomic_t) signal;
}

static WizardBooleanType GetPhrase(const char *prompt,PassphraseMode flags,
  StringInfo *phrase)
{
  ssize_t
    count;

#if defined(WIZARDSTOOLKIT_HAVE_TERMIOS_H)
  char
    c;

  int
    input,
    output;

  register unsigned char
    *p;

  struct sigaction
    action,
    sigalrm,
    sighup,
    sigint,
    sigpipe,
    sigquit,
    sigterm,
    sigtstp,
    sigttin,
    sigttou;

  struct termios
    attributes,
    save_attributes;

  /*
    Read and write to _PATH_TTY if available, otherwise from stdin.
  */
  signal_number=0;
  input=STDIN_FILENO;
  output=STDERR_FILENO;
  if ((flags & StdinMode) == 0)
    {
      input=open(_PATH_TTY,O_RDWR);
      output=input;
    }
  if (((flags & StdinMode) != 0) || (input == -1) || (output == -1))
    {
      if ((flags & RequireTTYMode) != 0)
        {
          errno=ENOTTY;
          return(WizardFalse);
        }
    }
  /*
    Set our signal handler.
  */
  (void) sigemptyset(&action.sa_mask);
  action.sa_flags=0;  /* don't restart system calls */
  action.sa_handler=SignalHandler;
  (void) sigaction(SIGALRM,&action,&sigalrm);
  (void) sigaction(SIGHUP,&action,&sighup);
  (void) sigaction(SIGINT,&action,&sigint);
  (void) sigaction(SIGPIPE,&action,&sigpipe);
  (void) sigaction(SIGQUIT,&action,&sigquit);
  (void) sigaction(SIGTERM,&action,&sigterm);
  (void) sigaction(SIGTSTP,&action,&sigtstp);
  (void) sigaction(SIGTTIN,&action,&sigttin);
  (void) sigaction(SIGTTOU,&action,&sigttou);
  if ((input == STDIN_FILENO) || (tcgetattr(input,&save_attributes) != 0))
    {
      (void) ResetWizardMemory(&attributes,0,sizeof(attributes));
      attributes.c_lflag|=ECHO;
      (void) CopyWizardMemory(&save_attributes,&attributes,sizeof(attributes));
    }
  else
    {
      /*
        Turn off echo.
      */
      (void) CopyWizardMemory(&attributes,&save_attributes,sizeof(attributes));
      if ((flags & EchoOnMode) == 0)
        attributes.c_lflag&=(~(ECHO | ECHONL));
#if defined(VSTATUS)
      if (attributes.c_cc[VSTATUS] != _POSIX_VDISABLE)
        attributes.c_cc[VSTATUS]=_POSIX_VDISABLE;
#endif
      (void) tcsetattr(input,_T_FLUSH,&attributes);
    }
  if ((flags & StdinMode) == 0)
    count=write(output,prompt,strlen(prompt));
  SetStringInfoLength(phrase,MaxTextExtent);
  p=GetStringInfoDatum(phrase);
  while (((count=read(input,&c,1)) == 1) && (c != '\n') && (c != '\r'))
  {
    if ((size_t) (p-GetStringInfoDatum(phrase)) <
        (GetStringInfoLength(phrase)-1))
      {
        if ((flags & SevenBitMode) != 0)
          c&=0x7f;
        if (isalpha((int) c) != 0)
          {
            if ((flags & ForceLowerMode) != 0)
              c=(char) tolower((int) c);
            if ((flags & ForceUpperMode) != 0)
              c=(char) toupper((int) c);
          }
        *p++=(unsigned char) c;
      }
  }
  *p=(unsigned char) '\0';
  SetStringInfoLength(phrase,(size_t) (p-GetStringInfoDatum(phrase)));
  if ((attributes.c_lflag & ECHO) == 0)
    count=write(output,"\n",1);
  /*
    Restore old terminal settings and signals.
  */
  if (memcmp(&attributes,&save_attributes,sizeof(attributes)) != 0)
    (void) tcsetattr(input,_T_FLUSH,&save_attributes);
  (void) sigaction(SIGALRM,&sigalrm,(struct sigaction *) NULL);
  (void) sigaction(SIGHUP,&sighup,(struct sigaction *) NULL);
  (void) sigaction(SIGINT,&sigint,(struct sigaction *) NULL);
  (void) sigaction(SIGQUIT,&sigquit,(struct sigaction *) NULL);
  (void) sigaction(SIGPIPE,&sigpipe,(struct sigaction *) NULL);
  (void) sigaction(SIGTERM,&sigterm,(struct sigaction *) NULL);
  (void) sigaction(SIGTSTP,&sigtstp,(struct sigaction *) NULL);
  (void) sigaction(SIGTTIN,&sigttin,(struct sigaction *) NULL);
  if (input != STDIN_FILENO)
    input=close(input)-1;
  if (signal_number != 0)
    {
      /*
        We were interrupted by a signal.
      */
      (void) kill((pid_t) getpid(),(int) signal_number);
      if ((signal_number == (sig_atomic_t) SIGTSTP) ||
          (signal_number == (sig_atomic_t) SIGTTIN) ||
          (signal_number == (sig_atomic_t) SIGTTOU))
        return(GetPhrase(prompt,flags,phrase));
    }
#else
  (void) fputs(prompt,stdout);
  SetStringInfoLength(phrase,MaxTextExtent);
  count=read(fileno(stdin),GetStringInfoDatum(phrase),MaxTextExtent);
  SetStringInfoLength(phrase,count);
#endif
  return(count != -1 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P a s s p h r a s e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPassphrase() gets a passphrase from the terminal and returns it.
%
%  The format of the GetPassphrase method is:
%
%      StringInfo GetPassphrase(ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o prompt: Prompt the user for the passphrase.
%
*/
WizardExport StringInfo *GetPassphrase(ExceptionInfo *exception)
{
  char
    prompt[MaxTextExtent];

  WizardBooleanType
    status;

  StringInfo
    *phrase,
    *rephrase;

  phrase=AcquireStringInfo(MaxTextExtent);
  rephrase=AcquireStringInfo(MaxTextExtent);
  (void) FormatLocaleString(prompt,MaxTextExtent,
    "Enter the passphrase (maximum of %d characters)\n",MaxTextExtent);
  status=write(STDERR_FILENO,prompt,strlen(prompt)) < 0 ? WizardFalse :
    WizardTrue;
  for ( ; ; )
  {
    status=GetPhrase("Enter passphrase: ",EchoOffMode,phrase);
    if (status == WizardFalse)
      {
        (void) ThrowWizardException(exception,GetWizardModule(),
          AuthenticateError,"unable to get pass phrase `%s'",strerror(errno));
        return((StringInfo *) NULL);
      }
    status=GetPhrase("Enter same passphrase again: ",EchoOffMode,rephrase);
    if (status == WizardFalse)
      {
        (void) ThrowWizardException(exception,GetWizardModule(),
          AuthenticateError,"unable to get pass phrase `%s'",strerror(errno));
        return((StringInfo *) NULL);
      }
    if (CompareStringInfo(phrase,rephrase) == 0)
      break;
    (void) PrintWizardString(stderr,"Passphrases are different.  Try again.\n");
  }
  rephrase=DestroyStringInfo(rephrase);
  SetStringInfoLength(phrase,GetStringInfoLength(phrase));
  return(phrase);
}
