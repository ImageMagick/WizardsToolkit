/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  L       OOO    CCCC   AAA   L      EEEEE                   %
%                  L      O   O  C      A   A  L      E                       %
%                  L      O   O  C      AAAAA  L      EEE                     %
%                  L      O   O  C      A   A  L      E                       %
%                  LLLLL   OOO    CCCC  A   A  LLLLL  EEEEE                   %
%                                                                             %
%                                                                             %
%                      WizardsToolkit Locale Methods                          %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/blob.h"
#include "wizard/client.h"
#include "wizard/configure.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hashmap.h"
#include "wizard/locale_.h"
#include "wizard/log.h"
#include "wizard/memory_.h"
#include "wizard/nt-base.h"
#include "wizard/semaphore.h"
#include "wizard/splay-tree.h"
#include "wizard/string_.h"
#include "wizard/token.h"
#include "wizard/utility.h"
#include "wizard/utility-private.h"
#include "wizard/xml-tree.h"
#include "wizard/xml-tree-private.h"

/*
  Define declarations.
*/
#if defined(WIZARDSTOOLKIT_HAVE_NEWLOCALE) || defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
#  define WIZARDSTOOLKIT_LOCALE_SUPPORT
#endif
#define LocaleFilename  "locale.xml"
#define MaxRecursionDepth  200

/*
  Static declarations.
*/
static const char
  *LocaleMap =
    "<?xml version=\"1.0\"?>"
    "<localemap>"
    "  <locale name=\"C\">"
    "    <Exception>"
    "     <Message name=\"\">"
    "     </Message>"
    "    </Exception>"
    "  </locale>"
    "</localemap>";

static SemaphoreInfo
  *locale_semaphore = (SemaphoreInfo *) NULL;

static SplayTreeInfo
  *locale_cache = (SplayTreeInfo *) NULL;

#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT)
static volatile locale_t
  c_locale = (locale_t) NULL;
#endif

/*
  Forward declarations.
*/
static WizardBooleanType
  IsLocaleTreeInstantiated(ExceptionInfo *),
  LoadLocaleCache(SplayTreeInfo *,const char *,const char *,const char *,
    const size_t,ExceptionInfo *);

#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e C L o c a l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCLocale() allocates the C locale object, or (locale_t) 0 with
%  errno set if it cannot be acquired.
%
%  The format of the AcquireCLocale method is:
%
%      locale_t AcquireCLocale(void)
%
*/
static locale_t AcquireCLocale(void)
{
#if defined(WIZARDSTOOLKIT_HAVE_NEWLOCALE)
  if (c_locale == (locale_t) NULL)
    c_locale=newlocale(LC_ALL_MASK,"C",(locale_t) 0);
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !defined(__MINGW32__)
  if (c_locale == (locale_t) NULL)
    c_locale=_create_locale(LC_ALL,"C");
#endif
  return(c_locale);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e L o c a l e S p l a y T r e e                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireLocaleSplayTree() caches one or more locale configurations which
%  provides a mapping between locale attributes and a locale tag.
%
%  The format of the AcquireLocaleSplayTree method is:
%
%      SplayTreeInfo *AcquireLocaleSplayTree(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the font file tag.
%
%    o locale: the actual locale.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void *DestroyLocaleNode(void *locale_info)
{
   LocaleInfo
    *p;

  p=(LocaleInfo *) locale_info;
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->tag != (char *) NULL)
    p->tag=DestroyString(p->tag);
  if (p->message != (char *) NULL)
    p->message=DestroyString(p->message);
  return(RelinquishWizardMemory(p));
}

static SplayTreeInfo *AcquireLocaleSplayTree(const char *filename,
  const char *locale,ExceptionInfo *exception)
{
  WizardStatusType
    status;

  SplayTreeInfo
    *locale_cache;

  locale_cache=NewSplayTree(CompareSplayTreeString,(void *(*)(void *)) NULL,
    DestroyLocaleNode);
  if (locale_cache == (SplayTreeInfo *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  status=WizardTrue;
#if !defined(WIZARDSTOOLKIT_ZERO_CONFIGURATION_SUPPORT)
  {
    const StringInfo
      *option;

    LinkedListInfo
      *options;

    options=GetLocaleOptions(filename,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
    while (option != (const StringInfo *) NULL)
    {
      status&=LoadLocaleCache(locale_cache,(const char *)
        GetStringInfoDatum(option),GetStringInfoPath(option),locale,0,
        exception);
      option=(const StringInfo *) GetNextValueInLinkedList(options);
    }
    options=DestroyLocaleOptions(options);
    if (GetNumberOfNodesInSplayTree(locale_cache) == 0)
      {
        options=GetLocaleOptions("english.xml",exception);
        option=(const StringInfo *) GetNextValueInLinkedList(options);
        while (option != (const StringInfo *) NULL)
        {
          status&=LoadLocaleCache(locale_cache,(const char *)
            GetStringInfoDatum(option),GetStringInfoPath(option),locale,0,
            exception);
          option=(const StringInfo *) GetNextValueInLinkedList(options);
        }
        options=DestroyLocaleOptions(options);
      }
  }
#endif
  if (GetNumberOfNodesInSplayTree(locale_cache) == 0)
    status&=LoadLocaleCache(locale_cache,LocaleMap,"built-in",locale,0,
      exception);
  return(locale_cache);
}

#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y C L o c a l e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCLocale() releases the resources allocated for a locale object
%  returned by a call to the AcquireCLocale() method.
%
%  The format of the DestroyCLocale method is:
%
%      void DestroyCLocale(void)
%
*/
static void DestroyCLocale(void)
{
  if (c_locale != (locale_t) NULL)
    freelocale(c_locale);
  c_locale=(locale_t) NULL;
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y L o c a l e O p t i o n s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyLocaleOptions() releases memory associated with an locale
%  messages.
%
%  The format of the DestroyProfiles method is:
%
%      LinkedListInfo *DestroyLocaleOptions(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/

static void *DestroyOptions(void *message)
{
  return(DestroyStringInfo((StringInfo *) message));
}

WizardExport LinkedListInfo *DestroyLocaleOptions(LinkedListInfo *messages)
{
  assert(messages != (LinkedListInfo *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  return(DestroyLinkedList(messages,DestroyOptions));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  F o r m a t L o c a l e F i l e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatLocaleFile() prints formatted output of a variable argument list to a
%  file in the "C" locale.
%
%  The format of the FormatLocaleFile method is:
%
%      ssize_t FormatLocaleFile(FILE *file,const char *format,...)
%
%  A description of each parameter follows.
%
%   o file:  the file.
%
%   o format:  A file describing the format to use to write the remaining
%     arguments.
%
*/

WizardExport ssize_t FormatLocaleFileList(FILE *file,const char *format,
  va_list operands)
{
  ssize_t
    n;

#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT) && defined(WIZARDSTOOLKIT_HAVE_VFPRINTF_L)
  {
    locale_t
      locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vfprintf(file,format,operands);
    else
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
      n=(ssize_t) vfprintf_l(file,format,locale,operands);
#else
      n=(ssize_t) vfprintf_l(file,locale,format,operands);
#endif
  }
#else
#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT) && defined(WIZARDSTOOLKIT_HAVE_USELOCALE)
  {
    locale_t
      locale,
      previous_locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vfprintf(file,format,operands);
    else
      {
        previous_locale=uselocale(locale);
        n=(ssize_t) vfprintf(file,format,operands);
        uselocale(previous_locale);
      }
  }
#else
  n=(ssize_t) vfprintf(file,format,operands);
#endif
#endif
  return(n);
}

WizardExport ssize_t FormatLocaleFile(FILE *file,const char *format,...)
{
  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=FormatLocaleFileList(file,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  F o r m a t L o c a l e S t r i n g                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FormatLocaleString() prints formatted output of a variable argument list to
%  a string buffer in the "C" locale.
%
%  The format of the FormatLocaleString method is:
%
%      ssize_t FormatLocaleString(char *string,const size_t length,
%        const char *format,...)
%
%  A description of each parameter follows.
%
%   o string:  FormatLocaleString() returns the formatted string in this
%     character buffer.
%
%   o length: the maximum length of the string.
%
%   o format:  A string describing the format to use to write the remaining
%     arguments.
%
*/

WizardExport ssize_t FormatLocaleStringList(char *string,const size_t length,
  const char *format,va_list operands)
{
  ssize_t
    n;

#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT) && defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF_L)
  {
    locale_t
      locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vsnprintf(string,length,format,operands);
    else
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
      n=(ssize_t) vsnprintf_l(string,length,format,locale,operands);
#else
      n=(ssize_t) vsnprintf_l(string,length,locale,format,operands);
#endif
  }
#elif defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF)
#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT) && defined(WIZARDSTOOLKIT_HAVE_USELOCALE)
  {
    locale_t
      locale,
      previous_locale;

    locale=AcquireCLocale();
    if (locale == (locale_t) NULL)
      n=(ssize_t) vsnprintf(string,length,format,operands);
    else
      {
        previous_locale=uselocale(locale);
        n=(ssize_t) vsnprintf(string,length,format,operands);
        uselocale(previous_locale);
      }
  }
#else
  n=(ssize_t) vsnprintf(string,length,format,operands);
#endif
#else
  n=(ssize_t) vsprintf(string,format,operands);
#endif
  if (n < 0)
    string[length-1]='\0';
  return(n);
}

WizardExport ssize_t FormatLocaleString(char *string,const size_t length,
  const char *format,...)
{
  ssize_t
    n;

  va_list
    operands;

  va_start(operands,format);
  n=FormatLocaleStringList(string,length,format,operands);
  va_end(operands);
  return(n);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t L o c a l e I n f o _                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleInfo_() searches the locale list for the specified tag and if
%  found returns attributes for that element.
%
%  The format of the GetLocaleInfo method is:
%
%      const LocaleInfo *GetLocaleInfo_(const char *tag,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o tag: the locale tag.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WizardExport const LocaleInfo *GetLocaleInfo_(const char *tag,
  ExceptionInfo *exception)
{
  const LocaleInfo
    *locale_info;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsLocaleTreeInstantiated(exception) == WizardFalse)
    return((const LocaleInfo *) NULL);
  LockSemaphoreInfo(locale_semaphore);
  if ((tag == (const char *) NULL) || (LocaleCompare(tag,"*") == 0))
    {
      ResetSplayTreeIterator(locale_cache);
      locale_info=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
      UnlockSemaphoreInfo(locale_semaphore);
      return(locale_info);
    }
  locale_info=(const LocaleInfo *) GetValueFromSplayTree(locale_cache,tag);
  UnlockSemaphoreInfo(locale_semaphore);
  return(locale_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e I n f o L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleInfoList() returns any locale messages that match the
%  specified pattern.
%
%  The format of the GetLocaleInfoList function is:
%
%      const LocaleInfo **GetLocaleInfoList(const char *pattern,
%        size_t *number_messages,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_messages:  This integer returns the number of locale messages in
%    the list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LocaleInfoCompare(const void *x,const void *y)
{
  const LocaleInfo
    **p,
    **q;

  p=(const LocaleInfo **) x,
  q=(const LocaleInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->tag,(*q)->tag));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

WizardExport const LocaleInfo **GetLocaleInfoList(const char *pattern,
  size_t *number_messages,ExceptionInfo *exception)
{
  const LocaleInfo
    **messages;

   const LocaleInfo
    *p;

   ssize_t
    i;

  /*
    Allocate locale list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_messages != (size_t *) NULL);
  *number_messages=0;
  p=GetLocaleInfo_("*",exception);
  if (p == (const LocaleInfo *) NULL)
    return((const LocaleInfo **) NULL);
  messages=(const LocaleInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(locale_cache)+1UL,sizeof(*messages));
  if (messages == (const LocaleInfo **) NULL)
    return((const LocaleInfo **) NULL);
  /*
    Generate locale list.
  */
  LockSemaphoreInfo(locale_semaphore);
  ResetSplayTreeIterator(locale_cache);
  p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  for (i=0; p != (const LocaleInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->tag,pattern,WizardTrue) != WizardFalse))
      messages[i++]=p;
    p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  }
  UnlockSemaphoreInfo(locale_semaphore);
  qsort((void *) messages,(size_t) i,sizeof(*messages),LocaleInfoCompare);
  messages[i]=(LocaleInfo *) NULL;
  *number_messages=(size_t) i;
  return(messages);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e L i s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleList() returns any locale messages that match the specified
%  pattern.
%
%  The format of the GetLocaleList function is:
%
%      char **GetLocaleList(const char *pattern,size_t *number_messages,
%        Exceptioninfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_messages:  This integer returns the number of messages in the
%      list.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LocaleTagCompare(const void *x,const void *y)
{
   char
    **p,
    **q;

  p=(char **) x;
  q=(char **) y;
  return(LocaleCompare(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

WizardExport char **GetLocaleList(const char *pattern,
  size_t *number_messages,ExceptionInfo *exception)
{
  char
    **messages;

   const LocaleInfo
    *p;

   ssize_t
    i;

  /*
    Allocate locale list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_messages != (size_t *) NULL);
  *number_messages=0;
  p=GetLocaleInfo_("*",exception);
  if (p == (const LocaleInfo *) NULL)
    return((char **) NULL);
  messages=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfNodesInSplayTree(locale_cache)+1UL,sizeof(*messages));
  if (messages == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(locale_semaphore);
  p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  for (i=0; p != (const LocaleInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->tag,pattern,WizardTrue) != WizardFalse))
      messages[i++]=ConstantString(p->tag);
    p=(const LocaleInfo *) GetNextValueInSplayTree(locale_cache);
  }
  UnlockSemaphoreInfo(locale_semaphore);
  qsort((void *) messages,(size_t) i,sizeof(*messages),LocaleTagCompare);
  messages[i]=(char *) NULL;
  *number_messages=(size_t) i;
  return(messages);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e M e s s a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleMessage() returns a message in the current locale that matches the
%  supplied tag.
%
%  The format of the GetLocaleMessage method is:
%
%      const char *GetLocaleMessage(const char *tag)
%
%  A description of each parameter follows:
%
%    o tag: Return a message that matches this tag in the current locale.
%
*/
WizardExport const char *GetLocaleMessage(const char *tag)
{
  char
    name[WizardLocaleExtent];

  const LocaleInfo
    *locale_info;

  ExceptionInfo
    *exception;

  if ((tag == (const char *) NULL) || (*tag == '\0'))
    return(tag);
  exception=AcquireExceptionInfo();
  (void) FormatLocaleString(name,WizardLocaleExtent,"%s/",tag);
  locale_info=GetLocaleInfo_(name,exception);
  exception=DestroyExceptionInfo(exception);
  if (locale_info != (const LocaleInfo *) NULL)
    return(locale_info->message);
  return(tag);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t L o c a l e O p t i o n s                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleOptions() returns any Wizard configuration messages associated
%  with the specified filename.
%
%  The format of the GetLocaleOptions method is:
%
%      LinkedListInfo *GetLocaleOptions(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the locale file tag.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WizardExport LinkedListInfo *GetLocaleOptions(const char *filename,
  ExceptionInfo *exception)
{
  char
    path[WizardPathExtent];

  const char
    *element;

  LinkedListInfo
    *messages,
    *paths;

  StringInfo
    *xml;

  assert(filename != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) CopyWizardString(path,filename,WizardPathExtent);
  /*
    Load XML from configuration files to linked-list.
  */
  messages=NewLinkedList(0);
  paths=GetConfigurePaths(filename,exception);
  if (paths != (LinkedListInfo *) NULL)
    {
      ResetLinkedListIterator(paths);
      element=(const char *) GetNextValueInLinkedList(paths);
      while (element != (const char *) NULL)
      {
        (void) FormatLocaleString(path,WizardPathExtent,"%s%s",element,
          filename);
        (void) LogWizardEvent(LocaleEvent,GetWizardModule(),
          "Searching for locale file: \"%s\"",path);
        xml=ConfigureFileToStringInfo(path);
        if (xml != (StringInfo *) NULL)
          (void) AppendValueToLinkedList(messages,xml);
        element=(const char *) GetNextValueInLinkedList(paths);
      }
      paths=DestroyLinkedList(paths,RelinquishWizardMemory);
    }
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  {
    char
      *blob;

    blob=(char *) NTResourceToBlob(filename);
    if (blob != (char *) NULL)
      {
        xml=AcquireStringInfo(0);
        SetStringInfoLength(xml,strlen(blob)+1);
        SetStringInfoDatum(xml,(unsigned char *) blob);
        blob=(char *) RelinquishMagickMemory(blob);
        SetStringInfoPath(xml,filename);
        (void) AppendValueToLinkedList(messages,xml);
      }
  }
#endif
  ResetLinkedListIterator(messages);
  return(messages);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o c a l e V a l u e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLocaleValue() returns the message associated with the locale info.
%
%  The format of the GetLocaleValue method is:
%
%      const char *GetLocaleValue(const LocaleInfo *locale_info)
%
%  A description of each parameter follows:
%
%    o locale_info:  The locale info.
%
*/
WizardExport const char *GetLocaleValue(const LocaleInfo *locale_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(locale_info != (LocaleInfo *) NULL);
  assert(locale_info->signature == WizardSignature);
  return(locale_info->message);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s L o c a l e T r e e I n s t a n t i a t e d                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsLocaleTreeInstantiated() determines if the locale tree is instantiated.
%  If not, it instantiates the tree and returns it.
%
%  The format of the IsLocaleInstantiated method is:
%
%      WizardBooleanType IsLocaleTreeInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static WizardBooleanType IsLocaleTreeInstantiated(ExceptionInfo *exception)
{
  if (locale_cache == (SplayTreeInfo *) NULL)
    {
      if (locale_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&locale_semaphore);
      LockSemaphoreInfo(locale_semaphore);
      if (locale_cache == (SplayTreeInfo *) NULL)
        {
          char
            *locale;

           const char
            *p;

          locale=(char *) NULL;
          p=setlocale(LC_CTYPE,(const char *) NULL);
          if (p != (const char *) NULL)
            locale=ConstantString(p);
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LC_ALL");
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LC_MESSAGES");
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LC_CTYPE");
          if (locale == (char *) NULL)
            locale=GetEnvironmentValue("LANG");
          if (locale == (char *) NULL)
            locale=ConstantString("C");
          locale_cache=AcquireLocaleSplayTree(LocaleFilename,locale,exception);
          locale=DestroyString(locale);
        }
      UnlockSemaphoreInfo(locale_semaphore);
    }
  return(locale_cache != (SplayTreeInfo *) NULL ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n t e r p r e t L o c a l e V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretLocaleValue() interprets the string as a floating point number in
%  the "C" locale and returns its value as a double. If sentinal is not a null
%  pointer, the method also sets the value pointed by sentinal to point to the
%  first character after the number.
%
%  The format of the InterpretLocaleValue method is:
%
%      double InterpretLocaleValue(const char *value,char **sentinal)
%
%  A description of each parameter follows:
%
%    o value: the string value.
%
%    o sentinal:  if sentinal is not NULL, a pointer to the character after the
%      last character used in the conversion is stored in the location
%      referenced by sentinal.
%
*/
WizardExport double InterpretLocaleValue(const char *string,char **sentinal)
{
  char
    *q;

  double
    value;

  if ((*string == '0') && ((string[1] | 0x20)=='x'))
    value=(double) strtoul(string,&q,16);
  else
    {
#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT) && defined(WIZARDSTOOLKIT_HAVE_STRTOD_L)
      locale_t
        locale;

      locale=AcquireCLocale();
      if (locale == (locale_t) NULL)
        value=strtod(string,&q);
      else
        value=strtod_l(string,&q,locale);
#else
      value=strtod(string,&q);
#endif
    }
  if (sentinal != (char **) NULL)
    *sentinal=q;
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t L o c a l e I n f o                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListLocaleInfo() lists the locale info to a file.
%
%  The format of the ListLocaleInfo method is:
%
%      WizardBooleanType ListLocaleInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ListLocaleInfo(FILE *file,
  ExceptionInfo *exception)
{
  const char
    *path;

  const LocaleInfo
    **locale_info;

   ssize_t
    i;

  size_t
    number_messages;

  if (file == (const FILE *) NULL)
    file=stdout;
  number_messages=0;
  locale_info=GetLocaleInfoList("*",&number_messages,exception);
  if (locale_info == (const LocaleInfo **) NULL)
    return(WizardFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_messages; i++)
  {
    if (locale_info[i]->stealth != WizardFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,locale_info[i]->path) != 0))
      {
        if (locale_info[i]->path != (char *) NULL)
          (void) FormatLocaleFile(file,"\nPath: %s\n\n",locale_info[i]->path);
        (void) FormatLocaleFile(file,"Tag/Message\n");
        (void) FormatLocaleFile(file,
          "-------------------------------------------------"
          "------------------------------\n");
      }
    path=locale_info[i]->path;
    (void) FormatLocaleFile(file,"%s\n",locale_info[i]->tag);
    if (locale_info[i]->message != (char *) NULL)
      (void) FormatLocaleFile(file,"  %s",locale_info[i]->message);
    (void) FormatLocaleFile(file,"\n");
  }
  (void) fflush(file);
  locale_info=(const LocaleInfo **)
    RelinquishWizardMemory((void *) locale_info);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d L o c a l e L i s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadLocaleCache() loads the locale configurations which provides a mapping
%  between locale attributes and a locale name.
%
%  The format of the LoadLocaleCache method is:
%
%      WizardBooleanType LoadLocaleCache(SplayTreeInfo *locale_cache,
%        const char *xml,const char *filename,const size_t depth,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The locale list in XML format.
%
%    o filename:  The locale list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static void ChopLocaleComponents(char *path,const size_t components)
{
   char
    *p;

  ssize_t
    count;

  if (*path == '\0')
    return;
  p=path+strlen(path)-1;
  if (*p == '/')
    *p='\0';
  for (count=0; (count < (ssize_t) components) && (p > path); p--)
    if (*p == '/')
      {
        *p='\0';
        count++;
      }
  if (count < (ssize_t) components)
    *path='\0';
}

static void LocaleFatalErrorHandler(
  const ExceptionType wizard_unused(severity),
  const char *reason,const char *description)
{
  wizard_unreferenced(severity);

  if (reason == (char *) NULL)
    return;
  (void) FormatLocaleFile(stderr,"%s: %s",GetClientName(),reason);
  if (description != (char *) NULL)
    (void) FormatLocaleFile(stderr," (%s)",description);
  (void) FormatLocaleFile(stderr,".\n");
  (void) fflush(stderr);
  exit(1);
}

static WizardBooleanType LoadLocaleCache(SplayTreeInfo *locale_cache,
  const char *xml,const char *filename,const char *locale,const size_t depth,
  ExceptionInfo *exception)
{
  char
    keyword[WizardLocaleExtent],
    message[WizardLocaleExtent],
    tag[WizardLocaleExtent],
    *token;

  const char
    *q;

  FatalErrorHandler
    fatal_handler;

  LocaleInfo
    *locale_info;

   char
    *p;

  size_t
    extent;

  WizardStatusType
    status;

  /*
    Read the locale configure file.
  */
  (void) LogWizardEvent(ConfigureEvent,GetWizardModule(),
    "Loading locale configure file \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(WizardFalse);
  status=WizardTrue;
  locale_info=(LocaleInfo *) NULL;
  *tag='\0';
  *message='\0';
  *keyword='\0';
  fatal_handler=SetFatalErrorHandler(LocaleFatalErrorHandler);
  token=AcquireString(xml);
  extent=strlen(token)+WizardPathExtent;
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetNextToken(q,&q,extent,token);
    if (*token == '\0')
      break;
    (void) CopyWizardString(keyword,token,WizardLocaleExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
        {
          GetNextToken(q,&q,extent,token);
          while (isspace((int) ((unsigned char) *q)) != 0)
            q++;
        }
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
        {
          GetNextToken(q,&q,extent,token);
          while (isspace((int) ((unsigned char) *q)) != 0)
            q++;
        }
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyWizardString(keyword,token,WizardLocaleExtent);
          GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          GetNextToken(q,&q,extent,token);
          if (LocaleCompare(keyword,"locale") == 0)
            {
              if (LocaleCompare(locale,token) != 0)
                break;
              continue;
            }
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > 200)
                (void) ThrowWizardException(exception,GetWizardModule(),
                  ConfigureError,"include element nested too deeply: `%s'",
                    token);
              else
                {
                  char
                    path[WizardPathExtent],
                    *xml;

                  *path='\0';
                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateWizardString(path,DirectorySeparator,
                      WizardPathExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyWizardString(path,token,WizardPathExtent);
                  else
                    (void) ConcatenateWizardString(path,token,WizardPathExtent);
                  xml=FileToXML(path,~0UL);
                  if (xml != (char *) NULL)
                    {
                      status&=LoadLocaleCache(locale_cache,xml,path,locale,
                        depth+1,exception);
                      xml=(char *) RelinquishWizardMemory(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<locale") == 0)
      {
        /*
          Locale element.
        */
        while ((*token != '>') && (*q != '\0'))
        {
          (void) CopyWizardString(keyword,token,WizardLocaleExtent);
          GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          GetNextToken(q,&q,extent,token);
        }
        continue;
      }
    if (LocaleCompare(keyword,"</locale>") == 0)
      {
        ChopLocaleComponents(tag,1);
        (void) ConcatenateWizardString(tag,"/",WizardLocaleExtent);
        continue;
      }
    if (LocaleCompare(keyword,"<localemap>") == 0)
      continue;
    if (LocaleCompare(keyword,"</localemap>") == 0)
      continue;
    if (LocaleCompare(keyword,"<message") == 0)
      {
        /*
          Message element.
        */
        while ((*token != '>') && (*q != '\0'))
        {
          (void) CopyWizardString(keyword,token,WizardLocaleExtent);
          GetNextToken(q,&q,extent,token);
          if (*token != '=')
            continue;
          GetNextToken(q,&q,extent,token);
          if (LocaleCompare(keyword,"name") == 0)
            {
              (void) ConcatenateWizardString(tag,token,WizardLocaleExtent);
              (void) ConcatenateWizardString(tag,"/",WizardLocaleExtent);
            }
        }
        for (p=(char *) q; (*q != '<') && (*q != '\0'); q++) ;
        while (isspace((int) ((unsigned char) *p)) != 0)
          p++;
        q--;
        while ((isspace((int) ((unsigned char) *q)) != 0) && (q > p))
          q--;
        (void) CopyWizardString(message,p,WizardMin(q-p+2,WizardLocaleExtent));
        locale_info=(LocaleInfo *) AcquireWizardMemory(sizeof(*locale_info));
        if (locale_info == (LocaleInfo *) NULL)
          ThrowFatalException(ResourceFatalError,
            "memory allocation failed `%s'");
        (void) memset(locale_info,0,sizeof(*locale_info));
        locale_info->path=ConstantString(filename);
        locale_info->tag=ConstantString(tag);
        locale_info->message=ConstantString(message);
        locale_info->signature=WizardSignature;
        status=AddValueToSplayTree(locale_cache,locale_info->tag,locale_info);
        if (status == WizardFalse)
          ThrowFatalException(ResourceFatalError,
            "memory allocation failed `%s'");
        (void) ConcatenateWizardString(tag,message,WizardLocaleExtent);
        (void) ConcatenateWizardString(tag,"\n",WizardLocaleExtent);
        q++;
        continue;
      }
    if (LocaleCompare(keyword,"</message>") == 0)
      {
        ChopLocaleComponents(tag,2);
        (void) ConcatenateWizardString(tag,"/",WizardLocaleExtent);
        continue;
      }
    if (*keyword == '<')
      {
        /*
          Subpath element.
        */
        if (*(keyword+1) == '?')
          continue;
        if (*(keyword+1) == '/')
          {
            ChopLocaleComponents(tag,1);
            if (*tag != '\0')
              (void) ConcatenateWizardString(tag,"/",WizardLocaleExtent);
            continue;
          }
        token[strlen(token)-1]='\0';
        (void) CopyWizardString(token,token+1,WizardLocaleExtent);
        (void) ConcatenateWizardString(tag,token,WizardLocaleExtent);
        (void) ConcatenateWizardString(tag,"/",WizardLocaleExtent);
        continue;
      }
    GetNextToken(q,(const char **) NULL,extent,token);
    if (*token != '=')
      continue;
  }
  token=(char *) RelinquishWizardMemory(token);
  (void) SetFatalErrorHandler(fatal_handler);
  return(status != 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o c a l e C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleComponentGenesis() instantiates the locale component.
%
%  The format of the LocaleComponentGenesis method is:
%
%      WizardBooleanType LocaleComponentGenesis(void)
%
*/
WizardExport WizardBooleanType LocaleComponentGenesis(void)
{
  if (locale_semaphore == (SemaphoreInfo *) NULL)
    locale_semaphore=AcquireSemaphoreInfo();
#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT)
  (void) AcquireCLocale();
#endif
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o c a l e C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LocaleComponentTerminus() destroys the locale component.
%
%  The format of the LocaleComponentTerminus method is:
%
%      LocaleComponentTerminus(void)
%
*/
WizardExport void LocaleComponentTerminus(void)
{
  if (locale_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&locale_semaphore);
  LockSemaphoreInfo(locale_semaphore);
  if (locale_cache != (SplayTreeInfo *) NULL)
    locale_cache=DestroySplayTree(locale_cache);
#if defined(WIZARDSTOOLKIT_LOCALE_SUPPORT)
  DestroyCLocale();
#endif
  UnlockSemaphoreInfo(locale_semaphore);
  RelinquishSemaphoreInfo(&locale_semaphore);
}
