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
%                               John Cristy                                   %
%                               March  2003                                   %
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
    { "Undefined", (long) UndefinedAuthenticate },
    { "Public", (long) PublicAuthenticateMethod },
    { "Secret", (long) SecretAuthenticateMethod },
    { (char *) NULL, UndefinedAuthenticate }
  },
  CipherOptions[] =
  {
    { "Undefined", (long) UndefinedCipher },
    { "AES", (long) AESCipher },
    { "None", (long) NoCipher },
    { "Serpent", (long) SerpentCipher },
    { "Twofish", (long) TwofishCipher },
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
    { "Undefined", (long) UndefinedData },
    { "Byte", (long) ByteData },
    { "Long", (long) LongData },
    { "Short", (long) ShortData },
    { "String", (long) StringData },
    { (char *) NULL, (long) UndefinedData }
  },
  EndianOptions[] =
  {
    { "Undefined", (long) UndefinedEndian },
    { "LSB", (long) LSBEndian },
    { "MSB", (long) MSBEndian },
    { (char *) NULL, (long) UndefinedEndian }
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
    { "Undefined", (long) UndefinedEntropy },
    { "None", (long) NoEntropy },
    { "BZip", (long) BZIPEntropy },
    { "Zip", (long) ZIPEntropy },
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
    { "Undefined", (long) UndefinedHash },
    { "CRC64", (long) CRC64Hash },
    { "MD5", (long) MD5Hash },
    { "None", (long) NoHash },
    { "SHA1", (long) SHA1Hash },
    { "SHA224", (long) SHA224Hash },
    { "SHA256", (long) SHA256Hash },
    { "SHA384", (long) SHA384Hash },
    { "SHA512", (long) SHA512Hash },
    { (char *) NULL, UndefinedHash }
  },
  ListOptions[] =
  {
    { "Authenticate", (long) WizardAuthenticateOptions },
    { "Cipher", (long) WizardCipherOptions },
    { "Command", (long) WizardCommandOptions },
    { "DataType", (long) WizardDataTypeOptions },
    { "Debug", (long) WizardDebugOptions },
    { "Endian", (long) WizardEndianOptions },
    { "Entropy", (long) WizardEntropyOptions },
    { "EntropyLevel", (long) WizardEntropyLevelOptions },
    { "Hash", (long) WizardHashOptions },
    { "KeyLength", (long) WizardKeyLengthOptions },
    { "List", (long) WizardListOptions },
    { "Mode", (long) WizardModeOptions },
    { (char *) NULL, (long) WizardUndefinedOptions }
  },
  LogWizardEventOptions[] =
  {
    { "All", (long) (AllEvents &~ TraceEvent) },
    { "Blob", (long) BlobEvent },
    { "Deprecate", (long) DeprecateEvent },
    { "Configure", (long) ConfigureEvent },
    { "Exception", (long) ExceptionEvent },
    { "Locale", (long) LocaleEvent },
    { "None", (long) NoEvents },
    { "Resource", (long) ResourceEvent },
    { "Trace", (long) TraceEvent },
    { "User", (long) UserEvent },
    { "Warning", (long) WarningEvent },
    { (char *) NULL, UndefinedEvents }
  },
  ModeOptions[] =
  {
    { "Undefined", (long) UndefinedMode },
    { "CBC", (long) CBCMode },
    { "CFB", (long) CFBMode },
    { "CTR", (long) CTRMode },
    { "ECB", (long) ECBMode },
    { "OFB", (long) OFBMode },
    { (char *) NULL, UndefinedMode }
  },
  ResourceOptions[] =
  {
    { "Undefined", (long) UndefinedResource },
    { "Area", (long) AreaResource },
    { "Disk", (long) DiskResource },
    { "File", (long) FileResource },
    { "Map", (long) MapResource },
    { "Memory", (long) MemoryResource },
    { (char *) NULL, (long) UndefinedResource }
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

  register long
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

  register long
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
%      long ParseWizardOption(const WizardOption option,
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
WizardExport long ParseWizardOption(const WizardOption option,
  const WizardBooleanType list,const char *options)
{
  char
    token[MaxTextExtent];

  const OptionInfo
    *option_info;

  long
    option_types;

  register char
    *q;

  register const char
    *p;

  register long
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
      if ((q-token) >= MaxTextExtent)
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
          (void) CopyWizardString(q,q+1,MaxTextExtent-strlen(q));
        while ((q=strchr(token+1,'_')) != (char *) NULL)
          (void) CopyWizardString(q,q+1,MaxTextExtent-strlen(q));
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
%        const long type)
%
%  A description of each parameter follows:
%
%    o option: the option.
%
%    o type: one or more options separated by commas.
%
*/
WizardExport const char *WizardOptionToMnemonic(const WizardOption option,
  const long type)
{
  const OptionInfo
    *option_info;

  register long
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
