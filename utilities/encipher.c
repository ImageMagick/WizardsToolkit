/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            EEEEE  N   N   CCCC  IIIII  PPPP   H   H  EEEEE  RRRR            %
%            E      NN  N  C        I    P   P  H   H  E      R   R           %
%            EEE    N N N  C        I    PPPP   HHHHH  EEE    RRRR            %
%            E      N  NN  C        I    P      H   H  E      R R             %
%            EEEEE  N   N   CCCC  IIIII  P      H   H  EEEEE  R  R            %
%                                                                             %
%                                                                             %
%                        Convert Plaintext To Ciphertext                      %
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
  Forward declaraction.
*/
static WizardBooleanType
  EncipherContent(ContentInfo *,const char *,const char *,
    const WizardBooleanType,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E n c i p h e r C o m m a n d                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherCommand() reads plaintext from a file and converts it to ciphertext.
%
%  The format of the EncipherCommand method is:
%
%      WizardBooleanType EncipherCommand(int argc,char **argv,
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

static void EncipherUsage()
{
  static const char
    *options[]=
    {
      "-authenticate method authenticate with this method: Secret or Public",
      "-chunksize bytes     number of bytes in a chunk",
      "-cipher type         cipher to encipher content",
      "-debug events        display copious debugging information",
      "-(de)compress        automagically (de)compress BZIP and ZIP files",
      "-entropy type        increase content entropy",
      "-help                print program options",
      "-hmac hash           ensure message integrity with this hash",
      "-key hash            strengthen the key with this hash",
      "-key-length value    cipher key length in bits: 256, 512, 1024, or 2048",
      "-keyring filename    get private key from this keyring",
      "-level value         entropy level: 1 (less entropy) to 9 (more entropy)",
      "-list type           print a list of supported option arguments",
      "-log format          format of debugging information",
      "-mode type           mode of enciphering",
      "-passphrase filename get the passphrase from this file",
      "-properties filename put cipher properties to this file",
      "-random hash         strengthen random data with this hash",
      "-true-random         strengthen enciphering with true random numbers",
      "-verbose             print detailed information about the secure content",
      "-version             print version information",
      (char *) NULL
    };

  const char
    **p;

  (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
    (unsigned long *) NULL));
  (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
  (void) fprintf(stdout,"Usage: %s [options ...] plaintext ciphertext\n",
    GetClientName());
  (void) fprintf(stdout,"\nWhere options include:\n");
  for (p=options; *p != (char *) NULL; p++)
    (void) fprintf(stdout,"  %s\n",*p);
  exit(0);
}

static double StringToDouble(const char *string,const double interval)
{
  char
    *q;

  double
    value;

  assert(string != (char *) NULL);
  value=strtod(string,&q);
  switch (tolower((int) ((unsigned char) *q)))
  {
    case '%': value*=pow(1024.0,0)*interval/100.0; break;
    case 'k': value*=pow(1024.0,1); break;
    case 'm': value*=pow(1024.0,2); break;
    case 'g': value*=pow(1024.0,3); break;
    case 't': value*=pow(1024.0,4); break;
    case 'p': value*=pow(1024.0,5); break;
    case 'e': value*=pow(1024.0,6); break;
    case 'z': value*=pow(1024.0,7); break;
    case 'y': value*=pow(1024.0,8); break;
    default:  break;
  }
  return(value);
}

WizardExport WizardBooleanType EncipherCommand(int argc,char **argv,
  ExceptionInfo *exception)
{
#define DestroyCipher() \
{ \
  content_info=DestroyContentInfo(content_info); \
  for (i=0; i < (long) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishWizardMemory(argv); \
}
#define ThrowCipherException(asperity,tag,context) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),asperity,tag, \
    context); \
  DestroyCipher(); \
  return(WizardFalse); \
}
#define ThrowInvalidArgumentException(option,argument) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),OptionError, \
    "invalid argument: `%s': %s",argument,option); \
  DestroyCipher(); \
  return(WizardFalse); \
}

  const char
    *cipher_filename,
    *option,
    *plain_filename;

  ContentInfo
    *content_info;

  long
    type;

  register long
    i;

  WizardBooleanType
    compress,
    status,
    verbose;

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
  if (argc < 3)
    EncipherUsage();
  status=ExpandFilenames(&argc,&argv);
  content_info=AcquireContentInfo();
  if (status == WizardFalse)
    ThrowCipherException(ResourceError,"memory allocation failed: `%s'",
      strerror(errno));
  plain_filename=(const char *) NULL;
  cipher_filename=(const char *) NULL;
  compress=WizardTrue;
  verbose=WizardFalse;
  for (i=1; i < (long) argc; i++)
  {
    option=argv[i];
    if (IsWizardOption(option) == WizardFalse)
      {
        if (plain_filename == (char *) NULL)
          plain_filename=option;
        else
          if (cipher_filename == (char *) NULL)
            cipher_filename=option;
          else
            EncipherUsage();
        continue;
      }
    switch (*(option+1))
    {
      case '(':
      {
        if (LocaleCompare(option+1,"(de)compress") == 0)
          {
            compress=(*option == '-') ? WizardTrue : WizardFalse;
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'a':
      {
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            long
               method;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing authentication method: "
                "`%s'",option);
            method=ParseWizardOption(WizardAuthenticateOptions,WizardFalse,
              argv[i]);
            if (method < 0)
              ThrowCipherException(OptionFatalError,"unrecognized "
                "authentication method: `%s'",argv[i]);
            content_info->authenticate_method=(AuthenticateMethod) method;
            if (method == PublicAuthenticateMethod)
              ThrowCipherException(OptionFatalError,"Public authentication "
                "method not available yet: `%s'",argv[i]);
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'c':
      {
        if (LocaleCompare("cipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing cipher type: `%s'",
                option);
            type=ParseWizardOption(WizardCipherOptions,WizardFalse,argv[i]);
            if (type < 0)
              ThrowCipherException(OptionFatalError,"unrecognized cipher "
                "type: `%s'",argv[i]);
            content_info->cipher=(CipherType) type;
            break;
          }
        if (LocaleCompare(option,"-chunksize") == 0)
          {
            char
              *p;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing chunk size: `%s'",
                option);
            (void) strtod(argv[i],&p);
            if (p == argv[i])
              ThrowInvalidArgumentException(option,argv[i]);
            content_info->chunksize=(size_t) StringToDouble(argv[i],100.0);
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'd':
      {
        if (LocaleCompare(option,"-debug") == 0)
          {
            LogEventType
              event_mask;

            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing log event mask: `%s'",
                option);
            event_mask=SetLogEventMask(argv[i]);
            if (event_mask == UndefinedEvents)
              ThrowCipherException(OptionFatalError,"unrecognized log event "
                "type: `%s'",argv[i]);
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'e':
      {
        if (LocaleCompare("entropy",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing entropy type: `%s'",
                option);
            type=ParseWizardOption(WizardEntropyOptions,WizardFalse,argv[i]);
            if (type < 0)
              ThrowCipherException(OptionFatalError,"unrecognized entropy "
                "type: `%s'",argv[i]);
            content_info->entropy=(EntropyType) type;
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'h':
      {
        if (LocaleCompare("hmac",option+1) == 0)
          {
            long
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing HMAC hash: `%s'",
                option);
            type=ParseWizardOption(WizardHashOptions,WizardFalse,argv[i]);
            if (type < 0)
              ThrowCipherException(OptionFatalError,"unrecognized HMAC hash: "
                "`%s'",argv[i]);
            content_info->hmac=(HashType) type;
            break;
          }
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          EncipherUsage();
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'k':
      {
        if (LocaleCompare("key",option+1) == 0)
          {
            long
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing key hash: `%s'",option);
            type=ParseWizardOption(WizardHashOptions,WizardFalse,argv[i]);
            if (type < 0)
              ThrowCipherException(OptionFatalError,"unrecognized key hash: "
                "`%s'",argv[i]);
            content_info->key_hash=(HashType) type;
            break;
          }
        if (LocaleCompare("key-length",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing key length: `%s'",
                option);
            content_info->key_length=(unsigned int) strtod(option,(char **)
              NULL);
            break;
          }
        if (LocaleCompare("keyring",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing keyring filename: `%s'",
                option);
            content_info->keyring=ConstantString(argv[i]);
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'l':
      {
        if (LocaleCompare(option,"-level") == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing entropy level: `%s'",
                option);
            type=ParseWizardOption(WizardEntropyLevelOptions,WizardFalse,
              argv[i]);
            if (type < 0)
              ThrowCipherException(OptionFatalError,"unrecognized entropy "
                "level: `%s'",argv[i]);
            content_info->level=(unsigned int) type;
            break;
          }
        if (LocaleCompare(option,"-list") == 0)
          {
            long
              list;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing list type: `%s'",
                option);
            if (LocaleCompare(argv[i],"configure") == 0)
              {
                (void) ListConfigureInfo((FILE *) NULL,exception);
                Exit(0);
              }
            list=ParseWizardOption(WizardListOptions,WizardFalse,argv[i]);
            if (list < 0)
              ThrowCipherException(OptionFatalError,"unrecognized list type: "
                "`%s'",argv[i]);
            (void) ListWizardOptions((FILE *) NULL,(WizardOption) list,
              exception);
            Exit(0);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (long) argc) ||
                (strchr(argv[i],'%') == (char *) NULL))
              ThrowCipherException(OptionFatalError,"missing argument: `%s'",
                option);
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'm':
      {
        if (LocaleCompare("mode",option+1) == 0)
          {
            long
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing cipher mode: `%s'",
                option);
            type=ParseWizardOption(WizardModeOptions,WizardFalse,argv[i]);
            if (type < 0)
              ThrowCipherException(OptionFatalError,"unrecognized cipher "
                "mode: `%s'",argv[i]);
            content_info->mode=(CipherMode) type;
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'p':
      {
        if (LocaleCompare("passphrase",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing passphrase filename: "
                "`%s'",option);
            content_info->passphrase=ConstantString(argv[i]);
            break;
          }
        if (LocaleCompare("properties",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCipherException(OptionError,"missing properties filename: "
                "`%s'",option);
            content_info->properties=ConstantString(argv[i]);
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 't':
      {
        if (LocaleCompare(option+1,"true-random") == 0)
          {
            SetRandomTrueRandom(*option == '-' ? WizardTrue : WizardFalse);
            break;
          }
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'v':
      {
        if (LocaleCompare(option+1,"verbose") == 0)
          {
            verbose=(*option == '-') ? WizardTrue : WizardFalse;
            break;
          }
        if (LocaleCompare(option,"-version") == 0)
          {
            (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
              (unsigned long *) NULL));
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
  }
  /*
    Encipher content.
  */
  if ((plain_filename == (char *) NULL) ||
      (cipher_filename == (char *) NULL))
    EncipherUsage();
  content_info->content=ConstantString(plain_filename);
  status=EncipherContent(content_info,plain_filename,cipher_filename,
    compress,exception);
  if ((status != WizardFalse) && (verbose != WizardFalse))
    (void) PrintCipherProperties(content_info,stdout);
  /*
    Free resources.
  */
  DestroyCipher();
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c i p h e r C o n t e n t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EncipherContent() reads plaintext from a file and writes it as ciphertext.
%
%  The format of the EncipherContent method is:
%
%      WizardBooleanType EncipherContent(ContentInfo *content_info,
%        const char *plain_filename,const char *cipher_filename,
%        const WizardBooleanType compress,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o content_info: the cipher options.
%
%    o plain_filename: the plaintext filename.
%
%    o cipher_filename: the ciphertext filename.
%
%    o compress: automagically (de)compress BZIP and ZIP files.
%
%    o exception: return any exceptions in this structure.
%
*/
static WizardBooleanType EncipherContent(ContentInfo *content_info,
  const char *plain_filename,const char *cipher_filename,
  const WizardBooleanType compress,ExceptionInfo *exception)
{
#define ThrowEncipherContentException(asperity,tag,context) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),asperity, \
    tag,context,strerror(errno)); \
  return(WizardFalse); \
}

  char
    *cipher_packet,
    *cipher_rdf,
    *digest,
    message[MaxTextExtent],
    timestamp[MaxTextExtent];

  const StringInfo
    *chaos,
    *hmac;

  const struct stat
    *properties;

  HashInfo
    *hash_info;

  size_t
    length,
    pad;

  ssize_t
    count;

  StringInfo
    *ciphertext,
    *plaintext;

  unsigned long
    blocksize;

  WizardBooleanType
    status;

  /*
    Open plaintext and ciphertext content files.
  */
  content_info->plainblob=OpenBlob(plain_filename,ReadBinaryBlobMode,compress,
    exception);
  if (content_info->plainblob == (BlobInfo *) NULL)
    return(WizardFalse);
  content_info->cipherblob=OpenBlob(cipher_filename,WriteBinaryBlobMode,
    compress,exception);
  if (content_info->cipherblob == (BlobInfo *) NULL)
    return(WizardFalse);
  content_info->cipher_info=AcquireCipherInfo(content_info->cipher,
    content_info->mode);
  content_info->authenticate_info=AcquireAuthenticateInfo(
    content_info->authenticate_method,content_info->keyring,
    content_info->key_hash);
  if (content_info->passphrase != (char *) NULL)
    SetAuthenticatePassphrase(content_info->authenticate_info,
      content_info->passphrase);
  SetAuthenticateKeyLength(content_info->authenticate_info,
    content_info->key_length);
  status=GenerateAuthenticateKey(content_info->authenticate_info,exception);
  if (status == WizardFalse)
    return(WizardFalse);
  content_info->id=StringInfoToHexString(GetAuthenticateId(
    content_info->authenticate_info));
  SetCipherKey(content_info->cipher_info,GetAuthenticateKey(
    content_info->authenticate_info));
  content_info->nonce=StringInfoToHexString(GetCipherNonce(
    content_info->cipher_info));
  content_info->random_info=AcquireRandomInfo(content_info->random_hash);
  properties=GetBlobProperties(content_info->plainblob);
  content_info->access_date=properties->st_atime;
  content_info->modify_date=properties->st_mtime;
  content_info->create_date=properties->st_ctime;
  /*
    Generate cipher RDF blob.
  */
  cipher_rdf=AcquireString("<rdf:RDF xmlns:rdf=\""
    "http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n");
  (void) ConcatenateString(&cipher_rdf,"         xmlns:cipher=\""
     "http://www.wizards-toolkit.org/cipher/1.0/\">\n");
  (void) ConcatenateString(&cipher_rdf,"  <cipher:Content rdf:about=\"");
  (void) ConcatenateString(&cipher_rdf,plain_filename);
  (void) ConcatenateString(&cipher_rdf,"\">\n");
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:type>%s</cipher:type>\n",WizardOptionToMnemonic(
    WizardCipherOptions,content_info->cipher));
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:mode>%s</cipher:mode>\n",WizardOptionToMnemonic(
    WizardModeOptions,content_info->mode));
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:nonce>%s</cipher:nonce>\n",content_info->nonce);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:authenticate>%s</cipher:authenticate>\n",
    WizardOptionToMnemonic(WizardAuthenticateOptions,
    content_info->authenticate_method));
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:id>%s</cipher:id>\n",content_info->id);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:key-hash>%s</cipher:key-hash>\n",WizardOptionToMnemonic(
    WizardHashOptions,content_info->key_hash));
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:key-length>%u</cipher:key-length>\n",content_info->key_length);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:entropy>%s</cipher:entropy>\n",WizardOptionToMnemonic(
    WizardEntropyOptions,content_info->entropy));
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:level>%u</cipher:level>\n",content_info->level);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:hmac>%s</cipher:hmac>\n",WizardOptionToMnemonic(
    WizardHashOptions,content_info->hmac));
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:chunksize>%lu</cipher:chunksize>\n",1UL*
    content_info->chunksize);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardTime(content_info->modify_date,MaxTextExtent,timestamp);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:modify-date>%s</cipher:modify-date>\n",timestamp);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardTime(content_info->create_date,MaxTextExtent,timestamp);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:create-date>%s</cipher:create-date>\n",timestamp);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardTime(content_info->timestamp,MaxTextExtent,timestamp);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:timestamp>%s</cipher:timestamp>\n",timestamp);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:protocol>%u.%u</cipher:protocol>\n",
    content_info->protocol_major,content_info->protocol_minor);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) FormatWizardString(message,MaxTextExtent,
    "    <cipher:version>%s</cipher:version>\n",content_info->version);
  (void) ConcatenateString(&cipher_rdf,message);
  (void) ConcatenateString(&cipher_rdf,"  </cipher:Content>\n");
  (void) ConcatenateString(&cipher_rdf,"</rdf:RDF>\n");
  /*
    Wrap cipher RDF in a cipher-packet.
  */
  length=strlen(cipher_rdf);
  hash_info=AcquireHashInfo(SHA256Hash);
  InitializeHash(hash_info);
  plaintext=StringToStringInfo(cipher_rdf);
  UpdateHash(hash_info,plaintext);
  plaintext=DestroyStringInfo(plaintext);
  FinalizeHash(hash_info);
  digest=GetHashHexDigest(hash_info);
  length=(size_t) FormatWizardString(message,MaxTextExtent,
    "<?cipherpacket digest=\"%s\" bytes=\"%u\"?>\n",digest,(unsigned int)
    length);
  digest=DestroyString(digest);
  hash_info=DestroyHashInfo(hash_info);
  cipher_packet=AcquireString(message);
  (void) ConcatenateString(&cipher_packet,cipher_rdf);
  cipher_rdf=DestroyString(cipher_rdf);
  (void) ConcatenateString(&cipher_packet,"<?cipherpacket?>\f\n");
  length=strlen(cipher_packet);
  if (content_info->properties == (char *) NULL)
    count=WriteBlob(content_info->cipherblob,length,(unsigned char *)
      cipher_packet);
  else
    {
      BlobInfo
        *properties;

      /*
        Cipher properties are stored separately from the content.
      */
      properties=OpenBlob(content_info->properties,WriteBinaryBlobMode,
        WizardTrue,exception);
      if (properties == (BlobInfo *) NULL)
        return(WizardFalse);
      count=WriteBlob(properties,length,(unsigned char *) cipher_packet);
      if (CloseBlob(properties) == WizardTrue)
        ThrowFileException(exception,FileError,content_info->properties);
      properties=DestroyBlob(properties);
    }
  cipher_packet=DestroyString(cipher_packet);
  if (count != (ssize_t) length)
    ThrowEncipherContentException(FileError,"unable to write cipher properties "
      "`%s': `%s'",cipher_filename);
  /*
    Encipher plaintext.
  */
  if (content_info->hmac != NoHash)
    content_info->hmac_info=AcquireHMACInfo(content_info->hmac);
  content_info->entropy_info=(EntropyInfo *) NULL;
  if (content_info->entropy != NoEntropy)
    content_info->entropy_info=AcquireEntropyInfo(content_info->entropy,
      content_info->level);
  pad=0;
  blocksize=GetCipherBlocksize(content_info->cipher_info);
  ciphertext=(StringInfo *) NULL;
  for (plaintext=AcquireStringInfo(content_info->chunksize); ; )
  {
    SetStringInfoLength(plaintext,content_info->chunksize);
    count=ReadBlobChunk(content_info->plainblob,content_info->chunksize,
      GetStringInfoDatum(plaintext));
    if (count <= 0)
      break;
    length=(size_t) count;
    SetStringInfoLength(plaintext,length);
    if (content_info->hmac != NoHash)
      {
        ConstructHMAC(content_info->hmac_info,GetAuthenticateKey(
          content_info->authenticate_info),plaintext);
        hmac=GetHMACDigest(content_info->hmac_info);
        count=WriteBlobChunk(content_info->cipherblob,GetStringInfoLength(hmac),
          GetStringInfoDatum(hmac));
        if (count != (ssize_t) GetStringInfoLength(hmac))
          ThrowEncipherContentException(FileError,"unable to write ciphertext "
            "`%s': `%s'",cipher_filename);
      }
    if (content_info->entropy != NoEntropy)
      {
        status=IncreaseEntropy(content_info->entropy_info,plaintext,exception);
        chaos=GetEntropyChaos(content_info->entropy_info);
        if (status == WizardFalse)
          ThrowEncipherContentException(FileError,"unable to increase entropy "
            "`%s': `%s'",cipher_filename);
        if (GetStringInfoLength(chaos) > length)
          count=WriteBlobByte(content_info->cipherblob,(unsigned char)
            NoEntropy);
        else
          {
            SetStringInfo(plaintext,chaos);
            SetRandomKey(content_info->random_info,length-GetStringInfoLength(
              chaos),GetStringInfoDatum(plaintext)+GetStringInfoLength(chaos));
            count=WriteBlobByte(content_info->cipherblob,(unsigned char)
              content_info->entropy);
          }
        if (count != 1)
          ThrowEncipherContentException(FileError,"unable to write ciphertext "
            "`%s': `%s'",cipher_filename);
      }
    ciphertext=EncipherCipher(content_info->cipher_info,plaintext);
    if (content_info->mode != CFBMode)
      {
        pad=blocksize-length % blocksize;
        if (pad != blocksize)
          length+=pad;
      }
    count=WriteBlobChunk(content_info->cipherblob,length,GetStringInfoDatum(
      ciphertext));
    if (count != (ssize_t) length)
      ThrowEncipherContentException(FileError,"unable to write ciphertext "
        "`%s': `%s'",cipher_filename);
    if (SyncBlob(content_info->cipherblob) != WizardFalse)
      ThrowEncipherContentException(FileError,"unable to sync ciphertext `%s': "
        "`%s'",cipher_filename);
  }
  if ((content_info->mode != CFBMode) && (pad == blocksize))
    {
      /*
        Cryptographic padding.
      */
      count=WriteBlobChunk(content_info->cipherblob,blocksize,
        GetStringInfoDatum(ciphertext)+length);
      if (count != (ssize_t) blocksize)
        ThrowEncipherContentException(FileError,"unable to write ciphertext "
          "`%s': `%s'",cipher_filename);
    }
  if (CloseBlob(content_info->cipherblob) != WizardFalse)
    ThrowFileException(exception,FileError,content_info->content);
  if (CloseBlob(content_info->plainblob) != WizardFalse)
    ThrowFileException(exception,FileError,content_info->content);
  plaintext=DestroyStringInfo(plaintext);
  return(WizardTrue);
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
      iterations=(unsigned int) atol(argv[++i]);
    if (LocaleCompare("debug",option+1) == 0)
      (void) SetLogEventMask(argv[++i]);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=WizardTrue;
  }
  timer=AcquireTimerInfo();
  for (i=0; i < (long) iterations; i++)
  {
    status=EncipherCommand(argc,argv,exception);
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
      (void) fprintf(stderr,"Performance: %gips %0.3fu %ld:%02ld\n",1.0*
        iterations/elapsed_time,user_time,(long) (elapsed_time/60.0+0.5),(long)
        ceil(fmod(elapsed_time,60.0)));
    }
  timer=DestroyTimerInfo(timer);
  exception=DestroyExceptionInfo(exception);
  WizardsToolkitTerminus();
  return(status == WizardFalse ? 1 : 0);
}
