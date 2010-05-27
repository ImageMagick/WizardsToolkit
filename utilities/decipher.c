/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            DDDD   EEEEE   CCCC  IIIII  PPPP   H   H  EEEEE  RRRR            %
%            D   D  E      C        I    P   P  H   H  E      R   R           %
%            D   D  EEE    C        I    PPPP   HHHHH  EEE    RRRR            %
%            D   D  E      C        I    P      H   H  E      R R             %
%            DDDD   EEEEE   CCCC  IIIII  P      H   H  EEEEE  R  R            %
%                                                                             %
%                                                                             %
%                        Convert Ciphertext to Plaintext                      %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                January 2008                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#include <windows.h>
#endif
#include "content.h"
#include "utility_.h"

/*
  Forward declaraction.
*/
static WizardBooleanType
  DecipherContent(ContentInfo *,const char *,const char *,
    const WizardBooleanType,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e c i p h e r C o m m a n d                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherCommand() reads ciphertext from a file and converts it to plaintext.
%
%  The format of the DecipherCommand method is:
%
%      WizardBooleanType DecipherCommand(int argc,char **argv,
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

static void DecipherUsage(void)
{
  static const char
    *options[]=
    {
      "-authenticate method authenticate with this method: Secret or Public",
      "-chunksize bytes     number of bytes in a chunk",
      "-cipher type         cipher to decipher content",
      "-(de)compress        automagically (de)compress BZIP and ZIP files",
      "-debug events        display copious debugging information",
      "-entropy type        increase content entropy",
      "-help                print program options",
      "-hmac hash           ensure message integrity with this hash",
      "-key hash            strengthen the key with this hash",
      "-key-length value    cipher key length in bits: 256, 512, 1024, or 2048",
      "-keyring filename    add private key to this keyring",
      "-level value         entropy level: 1 (less entropy) to 9 (more entropy)",
      "-list type           print a list of supported option arguments",
      "-log format          format of debugging information",
      "-mode type           mode of deciphering",
      "-passphrase filename get the passphrase from this file",
      "-properties filename get cipher properties from this file",
      "-random hash         strengthen random data with this hash",
      "-true-random         strengthen deciphering with true random numbers",
      "-verbose             print detailed information about the secure content",
      "-version             print version information",
      (char *) NULL
    };

  const char
    **p;

  (void) fprintf(stdout,"Version: %s\n",GetWizardVersion(
    (size_t *) NULL));
  (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
  (void) fprintf(stdout,"Usage: %s [options ...] ciphertext "
    "plaintext\n",GetClientName());
  (void) fprintf(stdout,"\nWhere options include:\n");
  for (p=options; *p != (char *) NULL; p++)
    (void) fprintf(stdout,"  %s\n",*p);
  exit(0);
}

WizardExport WizardBooleanType DecipherCommand(int argc,char **argv,
  ExceptionInfo *exception)
{
#define DestroyCipher() \
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

  ssize_t
    type;

  register ssize_t
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
            (size_t *) NULL));
          (void) fprintf(stdout,"Copyright: %s\n\n",GetWizardCopyright());
          return(WizardTrue);
        }
    }
  if (argc < 3)
    DecipherUsage();
  status=ExpandFilenames(&argc,&argv);
  content_info=AcquireContentInfo();
  if (status == WizardFalse)
    ThrowCipherException(ResourceError,"memory allocation failed: `%s'",
      strerror(errno));
  plain_filename=(const char *) NULL;
  cipher_filename=(const char *) NULL;
  verbose=WizardFalse;
  compress=WizardTrue;
  for (i=1; i < (ssize_t) argc; i++)
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
            DecipherUsage();
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
            ssize_t
               method;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowCipherException(OptionError,"missing chunk size: `%s'",
                option);
            (void) strtod(argv[i],&p);
            if (p == argv[i])
              ThrowInvalidArgumentException(option,argv[i]);
            content_info->chunksize=(size_t) SiPrefixToDouble(argv[i],100.0);
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
          DecipherUsage();
        ThrowCipherException(OptionFatalError,"unrecognized option: `%s'",
          option);
        break;
      }
      case 'k':
      {
        if (LocaleCompare("key",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
              ThrowCipherException(OptionFatalError,"unrecognized list type: "
                "`%s'",argv[i]);
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
            if ((i == (ssize_t) argc) ||
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
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
  }
  /*
    Decipher content.
  */
  if ((plain_filename == (char *) NULL) ||
      (cipher_filename == (char *) NULL))
    DecipherUsage();
  content_info->content=ConstantString(plain_filename);
  status=DecipherContent(content_info,plain_filename,cipher_filename,compress,
    exception);
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
%   D e c i p h e r C o n t e n t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DecipherContent() reads ciphertext from a file and writes it as plaintext.
%
%  The format of the DecipherContent method is:
%
%      WizardBooleanType DecipherContent(ContentInfo *content_info,
%        const char *cipher_filename,const char *plain_filename,
%        const WizardBooleanType compress,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o content_info: the cipher options.
%
%    o cipher_filename: the ciphertext filename.
%
%    o plain_filename: the plaintext filename.
%
%    o compress: automagically (de)compress BZIP and ZIP files.
%
%    o exception: return any exceptions in this structure.
%
*/
static WizardBooleanType DecipherContent(ContentInfo *content_info,
  const char *cipher_filename,const char *plain_filename,
  const WizardBooleanType compress,ExceptionInfo *exception)
{
#define ThrowDecipherContentException(asperity,tag,context) \
{ \
  (void) ThrowWizardException(exception,GetWizardModule(),asperity, \
    tag,context,strerror(errno)); \
  return(WizardFalse); \
}

  char
    message[MaxTextExtent];

  EntropyType
    entropy;

  size_t
    blocksize,
    length,
    pad;

  ssize_t
    count;

  StringInfo
    *ciphertext,
    *hmac,
    *plaintext;

  size_t
    chunk;

  WizardBooleanType
    status;

  /*
    Open plaintext and ciphertext content files.
  */
  content_info->cipherblob=OpenBlob(cipher_filename,ReadBinaryBlobMode,
    compress,exception);
  if (content_info->cipherblob == (BlobInfo *) NULL)
    return(WizardFalse);
  content_info->plainblob=OpenBlob(plain_filename,WriteBinaryBlobMode,compress,
    exception);
  if (content_info->plainblob == (BlobInfo *) NULL)
    return(WizardFalse);
  /*
    Decipher plaintext.
  */
  if (content_info->properties == (char *) NULL)
    status=GetContentInfo(content_info,content_info->cipherblob,exception);
  else
    {
      BlobInfo
        *properties;

      /*
        Cipher properties are stored separately from the content.
      */
      properties=OpenBlob(content_info->properties,ReadBinaryBlobMode,
        WizardTrue,exception);
      if (properties == (BlobInfo *) NULL)
        return(WizardFalse);
      status=GetContentInfo(content_info,properties,exception);
      if (CloseBlob(properties) == WizardTrue)
        ThrowFileException(exception,FileError,content_info->properties);
      properties=DestroyBlob(properties);
    }
  if (status == WizardFalse)
    return(WizardFalse);
  content_info->authenticate_info=AcquireAuthenticateInfo(
    content_info->authenticate_method,content_info->keyring,
    content_info->key_hash);
  SetAuthenticateKeyLength(content_info->authenticate_info,
    content_info->key_length);
  if (content_info->id != (char *) NULL)
    {
      StringInfo
        *id;

      /*
        Set key id.
      */
      id=HexStringToStringInfo(content_info->id);
      SetAuthenticateId(content_info->authenticate_info,id);
      id=DestroyStringInfo(id);
    }
  if (content_info->passphrase != (char *) NULL)
    SetAuthenticatePassphrase(content_info->authenticate_info,
      content_info->passphrase);
  status=AuthenticateKey(content_info->authenticate_info,exception);
  if (status == WizardFalse)
    ThrowDecipherContentException(ResourceError,"authentication failed: `%s': "
      "`%s'",cipher_filename);
  content_info->cipher_info=AcquireCipherInfo(content_info->cipher,
    content_info->mode);
  SetCipherKey(content_info->cipher_info,GetAuthenticateKey(
    content_info->authenticate_info));
  if (content_info->nonce != (char *) NULL)
    {
      StringInfo
        *nonce;

      /*
        Set cipher nonce.
      */
      nonce=HexStringToStringInfo(content_info->nonce);
      SetCipherNonce(content_info->cipher_info,nonce);
      nonce=DestroyStringInfo(nonce);
    }
  hmac=(StringInfo *) NULL;
  if (content_info->hmac != NoHash)
    {
      content_info->hmac_info=AcquireHMACInfo(content_info->hmac);
      hmac=AcquireStringInfo(GetHMACDigestsize(content_info->hmac_info));
    }
  entropy=content_info->entropy;
  if (content_info->entropy != NoEntropy)
    content_info->entropy_info=AcquireEntropyInfo(content_info->entropy,
      content_info->level);
  blocksize=GetCipherBlocksize(content_info->cipher_info);
  pad=blocksize-content_info->chunksize % blocksize;
  if (pad == blocksize)
    pad=0;
  ciphertext=AcquireStringInfo(content_info->chunksize);
  for (chunk=0; ; chunk++)
  {
    if (content_info->hmac != NoHash)
      {
        length=GetStringInfoLength(hmac);
        count=ReadBlobChunk(content_info->cipherblob,length,GetStringInfoDatum(
          hmac));
        if (count <= 0)
          break;
      }
    if (content_info->entropy != NoEntropy)
      entropy=(EntropyType) ReadBlobByte(content_info->cipherblob);
    length=content_info->chunksize;
    if (content_info->mode != CFBMode)
      length+=pad;
    SetStringInfoLength(ciphertext,length);
    count=ReadBlobChunk(content_info->cipherblob,length,GetStringInfoDatum(
      ciphertext));
    if (count <= 0)
      break;
    length=(size_t) count;
    SetStringInfoLength(ciphertext,length);
    plaintext=DecipherCipher(content_info->cipher_info,ciphertext);
    if ((content_info->mode != CFBMode) &&
        ((pad != 0) || (EOFBlob(content_info->cipherblob) != WizardFalse)))
      length-=GetStringInfoDatum(plaintext)[length-1]+1;
    SetStringInfoLength(plaintext,length);
    if (entropy != NoEntropy)
      {
        status=RestoreEntropy(content_info->entropy_info,length,plaintext,
          exception);
        if (status == WizardFalse)
          ThrowDecipherContentException(FileError,"unable to restore entropy "
            "`%s': `%s'",cipher_filename);
        SetStringInfo(plaintext,GetEntropyChaos(content_info->entropy_info));
      }
    if (content_info->hmac != NoHash)
      {
        ConstructHMAC(content_info->hmac_info,GetAuthenticateKey(
          content_info->authenticate_info),plaintext);
        if (CompareStringInfo(hmac,GetHMACDigest(content_info->hmac_info)) != 0)
          {
            (void) FormatWizardString(message,MaxTextExtent,"corrupt cipher "
              "chunk #%lu `%s'",chunk,cipher_filename);
            ThrowDecipherContentException(FileError,"%s: `%s'",message);
          }
      }
    count=WriteBlobChunk(content_info->plainblob,length,GetStringInfoDatum(
      plaintext));
    if (count != (ssize_t) length)
      ThrowDecipherContentException(FileError,"unable to write plaintext "
        "`%s': `%s'",cipher_filename);
  }
  if (CloseBlob(content_info->cipherblob) != WizardFalse)
    ThrowFileException(exception,FileError,content_info->content);
  if (CloseBlob(content_info->plainblob) != WizardFalse)
    ThrowFileException(exception,FileError,content_info->content);
#if defined(WIZARDSTOOLKIT_HAVE_UTIME)
  {
    struct utimbuf
      properties;

    properties.actime=content_info->modify_date;
    properties.modtime=content_info->modify_date;
    if (utime(plain_filename,&properties) < 0)
      ThrowFileException(exception,FileError,plain_filename);
  }
#endif
  ciphertext=DestroyStringInfo(ciphertext);
  if (content_info->hmac != NoHash)
    hmac=DestroyStringInfo(hmac);
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
    status=DecipherCommand(argc,argv,exception);
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
        iterations,1.0*iterations/elapsed_time,user_time,(ssize_t)
        (elapsed_time/60.0+0.5),(ssize_t) floor(fmod(elapsed_time,60.0)),
        (ssize_t) (1000.0*(elapsed_time-floor(elapsed_time))+0.5));
      timer=DestroyTimerInfo(timer);
    }
  exception=DestroyExceptionInfo(exception);
  WizardsToolkitTerminus();
  return(status == WizardFalse ? 1 : 0);
}
