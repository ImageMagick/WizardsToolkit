/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               K   K  EEEEE  Y   Y  RRRR   IIIII  N   N   GGGG               %
%               K  K   E       Y Y   R   R    I    NN  N  G                   %
%               KKK    EEE      Y    RRRR     I    N N N  G GGG               %
%               K  K   E        Y    R R      I    N  NN  G   G               %
%               K   K  EEEEE    Y    R  R   IIIII  N   N   GGGG               %
%                                                                             %
%                                                                             %
%          Import, Export, or List Properties of Keys in the Keyring          %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                January 2008                                 %
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
#if defined(WIZARDSTOOLKIT_HAVE_UTIME)
#if defined(WIZARDSTOOLKIT_HAVE_UTIME_H)
#include <utime.h>
#else
#include <sys/utime.h>
#endif
#endif
#if defined(__WINDOWS__)
#include <windows.h>
#endif
#include "content.h"
#include "utility_.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   K e y r i n g C o m m a n d                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  KeyringCommand() imports, exports, or lists properties of keys in your
%  keyring.
%
%  The format of the KeyringCommand method is:
%
%      WizardBooleanType KeyringCommand(int argc,char **argv,
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

static void KeyringUsage()
{
  static const char
    *options[]=
    {
      "-debug events        display copious debugging information",
      "-export id           export a key from your keyring",
      "-help                print program options",
      "-list type           print a list of supported option arguments",
      "-log format          format of debugging information",
      "-version             print version information",
      (char *) NULL
    };

  const char
    **p;

  (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
    (unsigned long *) NULL));
  (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
  (void) fprintf(stdout,"Usage: %s [options ...] [ keyring ...] keyring.rdf\n",
    GetClientName());
  (void) fprintf(stdout,"       %s [options ...] -export id [ keyring ...] "
    "keyring.xdm\n",GetClientName());
  (void) fprintf(stdout,"\nWhere options include:\n");
  for (p=options; *p != (char *) NULL; p++)
    (void) fprintf(stdout,"  %s\n",*p);
  exit(0);
}

WizardExport WizardBooleanType ExportKeyring(int argc,char **argv,
  ExceptionInfo *exception)
{
#define DestroyKeyring() \
{ \
  for (i=0; i < (long) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishWizardMemory(argv); \
}
#define ThrowKeyringException(asperity,tag,context) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),asperity,tag, \
    context); \
  DestroyKeyring(); \
  return(WizardFalse); \
}
#define ThrowInvalidArgumentException(option,argument) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),OptionError, \
    "invalid argument: `%s': %s",argument,option); \
  DestroyKeyring(); \
  return(WizardFalse); \
}

  const char
    *filename,
    *keyring,
    *option;

  KeyringInfo
    *keyring_info;

  register long
    i;

  StringInfo
    *id;

  WizardBooleanType
    status;

  /*
    Parse command-line options.
  */
  status=WizardFalse;
  keyring=(const char *) NULL;
  id=(StringInfo *) NULL;
  filename=argv[argc-1];
  for (i=1; i < (long) (argc-1); i++)
  {
    option=argv[i];
    if (IsWizardOption(option) != WizardFalse)
      switch (*(option+1))
      {
        case 'd':
        {
          if (LocaleCompare(option,"-debug") == 0)
            {
              LogEventType
                event_mask;

              i++;
              if (i == (long) argc)
                ThrowKeyringException(OptionError,"missing log event mask: "
                  "`%s'",option);
              event_mask=SetLogEventMask(argv[i]);
              if (event_mask == UndefinedEvents)
                ThrowKeyringException(OptionFatalError,"unrecognized log event "
                  "type: `%s'",argv[i]);
              break;
            }
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        case 'e':
        {
          if (LocaleCompare("export",option+1) == 0)
            {
              if (*option == '+')
                break;
              i++;
              if (i == (long) argc)
                ThrowKeyringException(OptionError,"missing key id: `%s'",
                  option);
              id=HexStringToStringInfo(argv[i]);
              break;
            }
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        case 'h':
        {
          if ((LocaleCompare("help",option+1) == 0) ||
              (LocaleCompare("-help",option+1) == 0))
            KeyringUsage();
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        case 'l':
        {
          if (LocaleCompare(option,"-list") == 0)
            {
              long
                list;

              if (*option == '+')
                break;
              i++;
              if (i == (long) argc)
                ThrowKeyringException(OptionError,"missing list type: `%s'",
                  option);
              if (LocaleCompare(argv[i],"configure") == 0)
                {
                  (void) ListConfigureInfo((FILE *) NULL,exception);
                  Exit(0);
                }
              list=ParseWizardOption(WizardListOptions,WizardFalse,argv[i]);
              if (list < 0)
                ThrowKeyringException(OptionFatalError,"unrecognized list "
                  "type: `%s'",argv[i]);
              (void) ListWizardOptions((FILE *) NULL,(WizardOption) list,
                exception);
              Exit(0);
              break;
            }
          if (LocaleCompare("log",option+1) == 0)
            {
              if (*option == '+')
                break;
              i++;
              if ((i == (long) argc) ||
                  (strchr(argv[i],'%') == (char *) NULL))
                ThrowKeyringException(OptionFatalError,"missing argument: `%s'",
                  option);
              break;
            }
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        case 'v':
        {
          if (LocaleCompare(option,"-version") == 0)
            {
              (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
                (unsigned long *) NULL));
              (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
              exit(0);
            }
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        default:
        {
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
      }
    /*
      Export key from keyring.
    */
    keyring_info=AcquireKeyringInfo(keyring);
    SetKeyringId(keyring_info,id);
    if (ExportKeyringKey(keyring_info,exception) == WizardFalse)
      {
        char
          *hex_id;

        hex_id=StringInfoToHexString(id);
        (void) ThrowWizardException(exception,GetWizardModule(),KeyringError,
          "no such key `%s': `%s'",hex_id,keyring);
        hex_id=DestroyString(hex_id);
      }
    else
      {
        SetKeyringPath(keyring_info,filename);
        status=ImportKeyringKey(keyring_info,exception);
      }
    id=DestroyStringInfo(id);
    keyring_info=DestroyKeyringInfo(keyring_info);
  }
  /*
    Free resources.
  */
  DestroyKeyring();
  return(status);
}

WizardExport WizardBooleanType KeyringCommand(int argc,char **argv,
  ExceptionInfo *exception)
{
#define DestroyKeyring() \
{ \
  for (i=0; i < (long) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishWizardMemory(argv); \
}
#define ThrowKeyringException(asperity,tag,context) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),asperity,tag, \
    context); \
  DestroyKeyring(); \
  return(WizardFalse); \
}
#define ThrowInvalidArgumentException(option,argument) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),OptionError, \
    "invalid argument: `%s': %s",argument,option); \
  DestroyKeyring(); \
  return(WizardFalse); \
}

  BlobInfo
    *keyring_blob;

  const char
    *option;

  register long
    i;

  WizardBooleanType
    status;

  /*
    Parse command-line options.
  */
  if (argc == 2)
    {
      option=argv[1];
      if ((LocaleCompare("version",option+1) == 0) ||
          (LocaleCompare("-version",option+1) == 0))
        {
          (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
            (unsigned long *) NULL));
          (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
          return(WizardTrue);
        }
    }
  if (argc < 2)
    KeyringUsage();
  status=ExpandFilenames(&argc,&argv);
  if (status == WizardFalse)
    ThrowKeyringException(ResourceError,"memory allocation failed: `%s'",
      strerror(errno));
  for (i=1; i < (argc-1); i++)
  {
    option=argv[i];
    if (LocaleCompare(option+1,"export") == 0)
      {
        status=ExportKeyring(argc,argv,exception);
        DestroyKeyring();
        return(status);
      }
  }
  keyring_blob=OpenBlob(argv[argc-1],WriteBinaryBlobMode,WizardTrue,exception);
  if (keyring_blob == (BlobInfo *) NULL)
    return(WizardFalse);
  (void) WriteBlobString(keyring_blob,"<?xml version=\"1.0\"?>\n");
  (void) WriteBlobString(keyring_blob,"<rdf:RDF xmlns:rdf=\""
    "http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n");
  (void) WriteBlobString(keyring_blob,"         xmlns:keyring=\""
    "http://www.wizards-toolkit.org/keyring/1.0/\">\n");
  for (i=1; i < (long) (argc-1); i++)
  {
    option=argv[i];
    if (IsWizardOption(option) != WizardFalse)
      switch (*(option+1))
      {
        case 'd':
        {
          if (LocaleCompare(option,"-debug") == 0)
            {
              LogEventType
                event_mask;

              i++;
              if (i == (long) argc)
                ThrowKeyringException(OptionError,"missing log event mask: "
                  "`%s'",option);
              event_mask=SetLogEventMask(argv[i]);
              if (event_mask == UndefinedEvents)
                ThrowKeyringException(OptionFatalError,"unrecognized log event "
                  "type: `%s'",argv[i]);
              break;
            }
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        case 'h':
        {
          if ((LocaleCompare("help",option+1) == 0) ||
              (LocaleCompare("-help",option+1) == 0))
            KeyringUsage();
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        case 'l':
        {
          if (LocaleCompare(option,"-list") == 0)
            {
              long
                list;

              if (*option == '+')
                break;
              i++;
              if (i == (long) argc)
                ThrowKeyringException(OptionError,"missing list type: `%s'",
                  option);
              if (LocaleCompare(argv[i],"configure") == 0)
                {
                  (void) ListConfigureInfo((FILE *) NULL,exception);
                  Exit(0);
                }
              list=ParseWizardOption(WizardListOptions,WizardFalse,argv[i]);
              if (list < 0)
                ThrowKeyringException(OptionFatalError,"unrecognized list "
                  "type: `%s'",argv[i]);
              (void) ListWizardOptions((FILE *) NULL,(WizardOption) list,
                exception);
              Exit(0);
              break;
            }
          if (LocaleCompare("log",option+1) == 0)
            {
              if (*option == '+')
                break;
              i++;
              if ((i == (long) argc) ||
                  (strchr(argv[i],'%') == (char *) NULL))
                ThrowKeyringException(OptionFatalError,"missing argument: `%s'",
                  option);
              break;
            }
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        case 'v':
        {
          if (LocaleCompare(option,"-version") == 0)
            {
              (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
                (unsigned long *) NULL));
              (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
              exit(0);
            }
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
        default:
        {
          ThrowKeyringException(OptionFatalError,"unrecognized option: `%s'",
            option);
          break;
        }
      }
    status=PrintKeyringProperties(argv[i],keyring_blob,exception);
  }
  if (argc == 2)
    status=PrintKeyringProperties((const char *) NULL,keyring_blob,exception);
  (void) WriteBlobString(keyring_blob,"</rdf:RDF>\n");
  status=CloseBlob(keyring_blob);
  /*
    Free resources.
  */
  DestroyKeyring();
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
int main(int argc,char **argv)
{
  char
    *option;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  register long
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
  for (i=1; i < (long) (argc-1); i++)
  {
    option=argv[i];
    if ((strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (LocaleCompare("bench",option+1) == 0)
      iterations=(unsigned int) StringToLong(argv[++i]);
    if (LocaleCompare("debug",option+1) == 0)
      (void) SetLogEventMask(argv[++i]);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=WizardTrue;
  }
  timer=(TimerInfo *) NULL;
  if (iterations > 1)
    timer=AcquireTimerInfo();
  for (i=0; i < (long) iterations; i++)
  {
    status=KeyringCommand(argc,argv,exception);
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
      (void) fprintf(stderr,"Performance: %ui %gips %0.3fu %ld:%02ld.%03ld\n",
        iterations,1.0*iterations/elapsed_time,user_time,(long)
        (elapsed_time/60.0+0.5),(long) floor(fmod(elapsed_time,60.0)),
        (long) (1000.0*(elapsed_time-floor(elapsed_time))+0.5));
      timer=DestroyTimerInfo(timer);
    }
  exception=DestroyExceptionInfo(exception);
  WizardsToolkitTerminus();
  return(status == WizardFalse ? 1 : 0);
}
