/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                   CCCC  IIIII  PPPP   H   H  EEEEE  RRRR                    %
%                  C        I    P   P  H   H  E      R   R                   %
%                  C        I    PPPP   HHHHH  EEE    RRRR                    %
%                  C        I    P      H   H  E      R R                     %
%                   CCCC  IIIII  P      H   H  EEEEE  R  R                    %
%                                                                             %
%                                                                             %
%                         IIIII  N   N  FFFFF   OOO                           %
%                           I    NN  N  F      O   O                          %
%                           I    N N N  FFF    O   O                          %
%                           I    N  NN  F      O   O                          %
%                         IIIII  N   N  F       OOO                           %
%                                                                             %
%                                                                             %
%                       Identify Ciphertext Properties.                       %
%                                                                             %
%                              Software Design                                %
%                                  Cristy                                     %
%                                January 2008                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "wizard/studio.h"
#include "wizard/WizardsToolkit.h"
#include "wizard/blob-private.h"
#include "wizard/exception-private.h"
#include "wizard/string-private.h"
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#include <windows.h>
#endif
#include "content.h"
#include "utility_.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C i p h e r I n f o C o m m a n d                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CipherInfoCommand() reads one or more files and displays its cipher
%  properties (i.e. enciphering algorithm, hash, etc.).
%
%  The format of the CipherInfoCommand method is:
%
%      WizardBooleanType CipherInfoCommand(int argc,char **argv,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o argc: The number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static void CipherInfoUsage(void)
{
  static const char
    *options[]=
    {
      "-debug events        display copious debugging information",
      "-help                print program options",
      "-list type           print a list of supported option arguments",
      "-version             print version information",
      (char *) NULL
    };

  const char
    **p;

  (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
    (size_t *) NULL));
  (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
  (void) fprintf(stdout,"Usage: %s [options ...] ciphertext [ ciphertext ... "
    "]\n",GetClientName());
  (void) fprintf(stdout,"\nWhere options include:\n");
  for (p=options; *p != (char *) NULL; p++)
    (void) fprintf(stdout,"  %s\n",*p);
  Exit(0);
}

WizardExport WizardBooleanType CipherInfoCommand(int argc,char **argv,
  ExceptionInfo *exception)
{
#define DestroyCipherInfo() \
{ \
  content_info=DestroyContentInfo(content_info); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishWizardMemory(argv); \
}
#define ThrowCipherException(asperity,tag,context) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),asperity,tag, \
    context); \
  DestroyCipherInfo(); \
  return(WizardFalse); \
}
#define ThrowInvalidArgumentException(option,argument) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),OptionError, \
    "invalid argument: `%s': %s",argument,option); \
  DestroyCipherInfo(); \
  return(WizardFalse); \
}

  char
    *option;

  ContentInfo
    *content_info;

  register ssize_t
    i;

  WizardBooleanType
    status;

  /*
    Parse command-line options.
  */
  if (argc < 2)
    CipherInfoUsage();
  status=ExpandFilenames(&argc,&argv);
  content_info=AcquireContentInfo();
  if (status == WizardFalse)
    ThrowCipherException(ResourceError,"memory allocation failed: `%s'",
      strerror(errno));
  for (i=1; i < argc; i++)
  {
    option=argv[i];
    if (*option == '-')
      {
        switch(*(option+1))
        {
          case 'd':
          {
            if (strcasecmp(option,"-debug") == 0)
              {
                LogEventType
                  event_mask;

                i++;
                if (i == argc)
                  ThrowCipherException(OptionError,"missing log event mask: "
                    "`%s'",option);
                event_mask=SetLogEventMask(argv[i]);
                if (event_mask == UndefinedEvents)
                  ThrowCipherException(OptionFatalError,"unrecognized log "
                    "event type: `%s'",argv[i]);
                break;
              }
            ThrowCipherException(OptionFatalError,"unrecognized option: "
              "`%s'",option);
            break;
          }
          case 'h':
          {
            if ((LocaleCompare("help",option+1) == 0) ||
                (LocaleCompare("-help",option+1) == 0))
              CipherInfoUsage();
            ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
              option);
            break;
          }
          case 'l':
          {
            if (LocaleCompare(option,"-list") == 0)
              {
                ssize_t
                  list;

                if (*option == '+')
                  break;
                i++;
                if (i == (ssize_t) argc)
                  ThrowCipherException(OptionError,"missing list type: `%s'",
                    option);
                if (LocaleCompare(argv[i],"configure") == 0)
                  {
                    (void) ListConfigureInfo((FILE *) NULL,exception);
                    Exit(0);
                  }
                list=ParseWizardOption(WizardListOptions,WizardFalse,argv[i]);
                if (list < 0)
                  ThrowCipherException(OptionFatalError,"unrecognized list "
                    "type: `%s'",argv[i]);
                (void) ListWizardOptions((FILE *) NULL,(WizardOption) list,
                  exception);
                Exit(0);
              }
            if (LocaleCompare("log",option+1) == 0)
              {
                if (*option == '+')
                  break;
                i++;
                if ((i == (ssize_t) argc) ||
                    (strchr(argv[i],'%') == (char *) NULL))
                  ThrowCipherException(OptionFatalError,"missing argument: "
                    "`%s'",option);
                break;
              }
            ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
              option);
            break;
          }
          case 'v':
          {
            if (strcasecmp(option,"-version") == 0)
              {
                (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
                  (size_t *) NULL));
                (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
                exit(0);
              }
            ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
              option);
            break;
          }
          default:
          {
            ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
              option);
            break;
          }
        }
        continue;
      }
    /*
      Identify cipher properties.
    */
    content_info->content=ConstantString(argv[i]);
    content_info->cipherblob=OpenBlob(content_info->content,ReadBinaryBlobMode,
      WizardTrue,exception);
    if (content_info->cipherblob != (BlobInfo *) NULL)
      {
        status=GetContentInfo(content_info,content_info->cipherblob,exception);
        if (status != WizardFalse)
          (void) PrintCipherProperties(content_info,stdout);
        if (CloseBlob(content_info->cipherblob) != WizardFalse)
          ThrowFileException(exception,FileError,content_info->content);
      }
  }
  /*
    Free resources.
  */
  DestroyCipherInfo();
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M a i n                                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

static int WizardMain(int argc,char **argv)
{
  char
    *option;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  register ssize_t
    i;

  TimerInfo
    *timer;

  unsigned int
    iterations;

  WizardBooleanType
    regard_warnings,
    status;

  WizardsToolkitGenesis(*argv);
  exception=AcquireExceptionInfo();
  iterations=1;
  status=WizardTrue;
  regard_warnings=WizardFalse;
  for (i=1; i < (ssize_t) (argc-1); i++)
  {
    option=argv[i];
    if ((strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (LocaleCompare("bench",option+1) == 0)
      iterations=(unsigned int) StringToUnsignedLong(argv[++i]);
    if (LocaleCompare("debug",option+1) == 0)
      (void) SetLogEventMask(argv[++i]);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=WizardTrue;
  }
  timer=(TimerInfo *) NULL;
  if (iterations > 1)
    timer=AcquireTimerInfo();
  for (i=0; i < (ssize_t) iterations; i++)
  {
    status=CipherInfoCommand(argc,argv,exception);
    if ((status == WizardFalse) ||
        (GetExceptionSeverity(exception) != UndefinedException))
      {
        if ((GetExceptionSeverity(exception) < ErrorException) &&
            (regard_warnings == WizardFalse))
          status=WizardTrue;
        CatchException(exception);
      }
  }
  if (iterations > 1)
    {
      elapsed_time=GetElapsedTime(timer);
      user_time=GetUserTime(timer);
      (void) fprintf(stderr,"Performance: %ui %gips %0.3fu %.20g:%02g.%03g\n",
        iterations,1.0*iterations/elapsed_time,user_time,(double)
        (elapsed_time/60.0+0.5),floor(fmod(elapsed_time,60.0)),
        (double) (1000.0*(elapsed_time-floor(elapsed_time))+0.5));
      timer=DestroyTimerInfo(timer);
    }
  exception=DestroyExceptionInfo(exception);
  WizardsToolkitTerminus();
  return(status == WizardFalse ? 1 : 0);
}

#if !defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
int main(int argc,char **argv)
{
  return(WizardMain(argc,argv));
}
#else
int wmain(int argc,wchar_t *argv[])
{
  char
    **utf8;

  int
    status;

  register int
    i;

  utf8=NTArgvToUTF8(argc,argv);
  status=WizardMain(argc,utf8);
  for (i=0; i < argc; i++)
    utf8[i]=DestroyString(utf8[i]);
  utf8=(char **) RelinquishWizardMemory(utf8);
  return(status);
}
#endif
