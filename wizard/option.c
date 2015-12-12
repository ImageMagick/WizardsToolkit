/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                   OOO   PPPP   TTTTT  IIIII   OOO   N   N                   %
%                  O   O  P   P    T      I    O   O  NN  N                   %
%                  O   O  PPPP     T      I    O   O  N N N                   %
%                  O   O  P        T      I    O   O  N  NN                   %
%                   OOO   P        T    IIIII   OOO   N   N                   %
%                                                                             %
%                                                                             %
%                       Wizard's Toolkit Option Methods                       %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/authenticate.h"
#include "wizard/cipher.h"
#include "wizard/entropy.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/mime.h"
#include "wizard/mime-private.h"
#include "wizard/option.h"
#include "wizard/resource_.h"
#include "wizard/string_.h"
#include "wizard/token.h"

/*
  Option declarations.
*/
static const OptionInfo
  AuthenticateOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedAuthenticate },
    { "Public", (ssize_t) PublicAuthenticateMethod },
    { "Secret", (ssize_t) SecretAuthenticateMethod },
    { (char *) NULL, UndefinedAuthenticate }
  },
  CipherOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedCipher },
    { "AES", (ssize_t) AESCipher },
    { "Chacha", (ssize_t) ChachaCipher },
    { "None", (ssize_t) NoCipher },
    { "Serpent", (ssize_t) SerpentCipher },
    { "Twofish", (ssize_t) TwofishCipher },
    { (char *) NULL, UndefinedCipher }
  },
  CommandOptions[] =
  {
    { "+authenticate", 1L },
    { "-authenticate", 1L },
    { "+chunksize", 1L },
    { "-chunksize", 1L },
    { "+decipher", 0L },
    { "-decipher", 0L },
    { "+debug", 1L },
    { "-debug", 1L },
    { "+entropy", 1L },
    { "-entropy", 1L },
    { "+hash", 1L },
    { "-hash", 1L },
    { "+help", 0L },
    { "-help", 0L },
    { "+key", 1L },
    { "-key", 1L },
    { "+key-length", 1L },
    { "-key-length", 1L },
    { "+mac", 1L },
    { "-mac", 1L },
    { "+level", 1L },
    { "-level", 1L },
    { "+list", 1L },
    { "-list", 1L },
    { "+log", 1L },
    { "-log", 1L },
    { "+mode", 1L },
    { "-mode", 1L },
    { "+properties", 1L },
    { "-properties", 1L },
    { "+random", 1L },
    { "-random", 1L },
    { "+verbose", 0L },
    { "-verbose", 0L },
    { "+version", 1L },
    { "-version", 1L },
    { (char *) NULL, 0L }
  },
  DataTypeOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedData },
    { "Byte", (ssize_t) ByteData },
    { "Long", (ssize_t) LongData },
    { "Short", (ssize_t) ShortData },
    { "String", (ssize_t) StringData },
    { (char *) NULL, (ssize_t) UndefinedData }
  },
  EndianOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedEndian },
    { "LSB", (ssize_t) LSBEndian },
    { "MSB", (ssize_t) MSBEndian },
    { (char *) NULL, (ssize_t) UndefinedEndian }
  },
  EntropyLevelOptions[] =
  {
    { "0", 0L },
    { "1", 1L },
    { "2", 2L },
    { "3", 3L },
    { "4", 4L },
    { "5", 5L },
    { "6", 6L },
    { "7", 7L },
    { "8", 8L },
    { "9", 9L },
    { (char *) NULL, 0L }
  },
  EntropyOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedEntropy },
    { "None", (ssize_t) NoEntropy },
    { "BZip", (ssize_t) BZIPEntropy },
    { "LZMA", (ssize_t) LZMAEntropy },
    { "Zip", (ssize_t) ZIPEntropy },
    { (char *) NULL, UndefinedEntropy }
  },
  KeyLengthOptions[] =
  {
    { "256", 256L },
    { "512", 512L },
    { "1024", 1024L },
    { "2048", 2048L },
    { (char *) NULL, 0L }
  },
  HashOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedHash },
    { "CRC64", (ssize_t) CRC64Hash },
    { "MD5", (ssize_t) MD5Hash },
    { "None", (ssize_t) NoHash },
    { "SHA1", (ssize_t) SHA1Hash },
    { "SHA224", (ssize_t) SHA2224Hash },
    { "SHA256", (ssize_t) SHA2256Hash },
    { "SHA384", (ssize_t) SHA2384Hash },
    { "SHA512", (ssize_t) SHA2512Hash },
    { "SHA2", (ssize_t) SHA2Hash },
    { "SHA2-224", (ssize_t) SHA2224Hash },
    { "SHA2-256", (ssize_t) SHA2256Hash },
    { "SHA2-384", (ssize_t) SHA2384Hash },
    { "SHA2-512", (ssize_t) SHA2512Hash },
    { "SHA3", (ssize_t) SHA3Hash },
    { "SHA3-224", (ssize_t) SHA3224Hash },
    { "SHA3-256", (ssize_t) SHA3256Hash },
    { "SHA3-384", (ssize_t) SHA3384Hash },
    { "SHA3-512", (ssize_t) SHA3512Hash },
    { (char *) NULL, UndefinedHash }
  },
  ListOptions[] =
  {
    { "Authenticate", (ssize_t) WizardAuthenticateOptions },
    { "Cipher", (ssize_t) WizardCipherOptions },
    { "Command", (ssize_t) WizardCommandOptions },
    { "DataType", (ssize_t) WizardDataTypeOptions },
    { "Debug", (ssize_t) WizardDebugOptions },
    { "Endian", (ssize_t) WizardEndianOptions },
    { "Entropy", (ssize_t) WizardEntropyOptions },
    { "EntropyLevel", (ssize_t) WizardEntropyLevelOptions },
    { "Hash", (ssize_t) WizardHashOptions },
    { "KeyLength", (ssize_t) WizardKeyLengthOptions },
    { "List", (ssize_t) WizardListOptions },
    { "Mode", (ssize_t) WizardModeOptions },
    { (char *) NULL, (ssize_t) WizardUndefinedOptions }
  },
  LogWizardEventOptions[] =
  {
    { "All", (ssize_t) (AllEvents &~ TraceEvent) },
    { "Blob", (ssize_t) BlobEvent },
    { "Deprecate", (ssize_t) DeprecateEvent },
    { "Configure", (ssize_t) ConfigureEvent },
    { "Exception", (ssize_t) ExceptionEvent },
    { "Locale", (ssize_t) LocaleEvent },
    { "None", (ssize_t) NoEvents },
    { "Resource", (ssize_t) ResourceEvent },
    { "Trace", (ssize_t) TraceEvent },
    { "User", (ssize_t) UserEvent },
    { "Warning", (ssize_t) WarningEvent },
    { (char *) NULL, UndefinedEvents }
  },
  ModeOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedMode },
    { "CBC", (ssize_t) CBCMode },
    { "CFB", (ssize_t) CFBMode },
    { "CTR", (ssize_t) CTRMode },
    { "ECB", (ssize_t) ECBMode },
    { "OFB", (ssize_t) OFBMode },
    { (char *) NULL, UndefinedMode }
  },
  ResourceOptions[] =
  {
    { "Undefined", (ssize_t) UndefinedResource },
    { "Area", (ssize_t) AreaResource },
    { "Disk", (ssize_t) DiskResource },
    { "File", (ssize_t) FileResource },
    { "Map", (ssize_t) MapResource },
    { "Memory", (ssize_t) MemoryResource },
    { (char *) NULL, (ssize_t) UndefinedResource }
  };

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a r d O p t i o n s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardOptions() returns a list of options.
%
%  The format of the GetWizardOptions method is:
%
%      const char **GetWizardOptions(const WizardOption option)
%
%  A description of each parameter follows:
%
%    o option: The option.
%
*/

static const OptionInfo *GetOptionInfo(const WizardOption option)
{
  switch (option)
  {
    case WizardAuthenticateOptions: return(AuthenticateOptions);
    case WizardCipherOptions: return(CipherOptions);
    case WizardCommandOptions: return(CommandOptions);
    case WizardDataTypeOptions: return(DataTypeOptions);
    case WizardDebugOptions: return(LogWizardEventOptions);
    case WizardEndianOptions: return(EndianOptions);
    case WizardEntropyOptions: return(EntropyOptions);
    case WizardEntropyLevelOptions: return(EntropyLevelOptions);
    case WizardKeyLengthOptions: return(KeyLengthOptions);
    case WizardHashOptions: return(HashOptions);
    case WizardListOptions: return(ListOptions);
    case WizardLogEventOptions: return(LogWizardEventOptions);
    case WizardModeOptions: return(ModeOptions);
    case WizardResourceOptions: return(ResourceOptions);
    default: break;
  }
  return((const OptionInfo *) NULL);
}

WizardExport char **GetWizardOptions(const WizardOption option)
{
  char
    **options;

  const OptionInfo
    *option_info;

  register ssize_t
    i;

  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return((char **) NULL);
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++) ;
  options=(char **) AcquireQuantumMemory((size_t) i+1UL,sizeof(*options));
  if (options == (char **) NULL)
    ThrowFatalException(ResourceFatalError,"unable to acquire string `%s'");
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++)
    options[i]=AcquireString(option_info[i].mnemonic);
  options[i]=(char *) NULL;
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s W i z a r d O p t i o n                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWizardOption() returns WizardTrue if the option begins with a - or + and
%  the first character that follows is alphanumeric.
%
%  The format of the IsWizardOption method is:
%
%      WizardBooleanType IsWizardOption(const char *option)
%
%  A description of each parameter follows:
%
%    o option: The option.
%
*/
WizardExport WizardBooleanType IsWizardOption(const char *option)
{
  assert(option != (const char *) NULL);
  if ((*option != '-') && (*option != '+'))
    return(WizardFalse);
  if (strlen(option) == 1)
    return(WizardFalse);
  option++;
  if (isalpha((int) ((unsigned char) *option)) == 0)
    return(WizardFalse);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i s t W i z a r d O p t i o n s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListWizardOptions() lists the contents of enumerated option type(s).
%
%  The format of the ListWizardOptions method is:
%
%      WizardBooleanType ListWizardOptions(FILE *file,const WizardOption option,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o file:  list options to this file handle.
%
%    o option:  which option list to display.
%
%    o exception:  return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ListWizardOptions(FILE *file,
  const WizardOption option,ExceptionInfo *wizard_unused(exception))
{
  const OptionInfo
    *option_info;

  register ssize_t
    i;

  if (file == (FILE *) NULL)
    file=stdout;
  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return(WizardFalse);
  for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
  {
    if ((i == 0) && (strcmp(option_info[i].mnemonic,"Undefined") == 0))
      continue;
    (void) fprintf(file,"%s\n",option_info[i].mnemonic);
  }
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e W i z a r d O p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseWizardOption() parses an option string and returns an enumerated option
%  type(s).
%
%  The format of the ParseWizardOption method is:
%
%      ssize_t ParseWizardOption(const WizardOption option,
%        const WizardBooleanType list,const char *options)
%
%  A description of each parameter follows:
%
%    o option: The option.
%
%    o list: A option other than zero permits more than one option separated by
%      commas.
%   
%    o options: One or more options separated by commas.
%
*/
WizardExport ssize_t ParseWizardOption(const WizardOption option,
  const WizardBooleanType list,const char *options)
{
  char
    token[WizardPathExtent];

  const OptionInfo
    *option_info;

  ssize_t
    option_types;

  register char
    *q;

  register const char
    *p;

  register ssize_t
    i;

  WizardBooleanType
    negate;

  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return(-1);
  option_types=0;
  for (p=options; p != (char *) NULL; p=strchr(p,','))
  {
    while (((isspace((int) ((unsigned char) *p)) != 0) || (*p == ',')) &&
           (*p != '\0'))
      p++;
    if ((*p == '-') || (*p == '+'))
      p++;
    negate=(*p == '!') ? WizardTrue : WizardFalse;
    if (negate != WizardFalse)
      p++;
    q=token;
    while (((isspace((int) ((unsigned char) *p)) == 0) && (*p != ',')) &&
           (*p != '\0'))
    {
      if ((q-token) >= (WizardPathExtent-1))
        break;
      *q++=(*p++);
    }
    *q='\0';
    for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
    {
      if (LocaleCompare(token,option_info[i].mnemonic) == 0)
        {
          if (*token == '!')
            option_types=option_types &~ option_info[i].type;
          else
            option_types=option_types | option_info[i].type;
          break;
        }
    }
    if ((option_info[i].mnemonic == (char *) NULL) &&
        ((strchr(token+1,'-') != (char *) NULL) || 
         (strchr(token+1,'_') != (char *) NULL)))
      {
        while ((q=strchr(token+1,'-')) != (char *) NULL)
          (void) CopyWizardString(q,q+1,WizardPathExtent-strlen(q));
        while ((q=strchr(token+1,'_')) != (char *) NULL)
          (void) CopyWizardString(q,q+1,WizardPathExtent-strlen(q));
        for (i=0; option_info[i].mnemonic != (char *) NULL; i++)
          if (LocaleCompare(token,option_info[i].mnemonic) == 0)
            {
              if (*token == '!')
                option_types=option_types &~ option_info[i].type;
              else
                option_types=option_types | option_info[i].type;
              break;
            }
      }
    if (option_info[i].mnemonic == (char *) NULL)
      return(-1);
    if (list == WizardFalse)
      break;
  }
  return(option_types);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W i z a r d O p t i o n T o M n e m o n i c                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardOptionToMnemonic() returns an enumerated option as a mnemonic.
%
%  The format of the WizardOptionToMnemonic method is:
%
%      const char *WizardOptionToMnemonic(const WizardOption option,
%        const ssize_t type)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
%    o type: one or more options separated by commas.
%
*/
WizardExport const char *WizardOptionToMnemonic(const WizardOption option,
  const ssize_t type)
{
  const OptionInfo
    *option_info;

  register ssize_t
    i;

  option_info=GetOptionInfo(option);
  if (option_info == (const OptionInfo *) NULL)
    return((const char *) NULL);
  for (i=0; option_info[i].mnemonic != (const char *) NULL; i++)
    if (type == option_info[i].type)
      break;
  if (option_info[i].mnemonic == (const char *) NULL)
    return("undefined");
  return(option_info[i].mnemonic);
}
