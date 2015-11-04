/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%          CCCC   OOO   N   N  FFFFF IIIII   GGGG  U   U  RRRR   EEEEE        %
%         C      O   O  NN  N  F       I    G      U   U  R   R  E            %
%         C      O   O  N N N  FFF     I    G GG   U   U  RRRR   EEE          %
%         C      O   O  N  NN  F       I    G   G  U   U  R R    E            %
%          CCCC   OOO   N   N  F     IIIII   GGG    UUU   R  R   EEEEE        %
%                                                                             %
%                                                                             %
%                     Wizards's Toolkit Configure Methods                     %
%                                                                             %
%                              Software Design                                %
%                                  Cristy                                     %
%                                 July 2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/blob.h"
#include "wizard/client.h"
#include "wizard/configure.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hashmap.h"
#include "wizard/log.h"
#include "wizard/memory_.h"
#include "wizard/semaphore.h"
#include "wizard/string_.h"
#include "wizard/token.h"
#include "wizard/utility.h"
#include "wizard/xml-tree.h"
#include "wizard/xml-tree-private.h"

/*
  Define declarations.
*/
#define ConfigureFilename  "configure.xml"

/*
  Typedef declarations.
*/
typedef struct _ConfigureMapInfo
{
  const char
    *name,
    *value;
} ConfigureMapInfo;

/*
  Static declarations.
*/
static const ConfigureMapInfo
  ConfigureMap[] =
  {
    { "NAME", "ImageMagick" }
  };

static LinkedListInfo
  *configure_cache = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *configure_semaphore = (SemaphoreInfo *) NULL;

/*
  Forward declarations.
*/
static WizardBooleanType
  IsConfigureCacheInstantiated(ExceptionInfo *),
  LoadConfigureCache(LinkedListInfo *,const char *,const char *,const size_t,
    ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A c q u i r e C o n f i g u r e C a c h e                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireConfigureCache() caches one or more configure configurations which
%  provides a mapping between configure attributes and a configure name.
%
%  The format of the AcquireConfigureCache method is:
%
%      LinkedListInfo *AcquireConfigureCache(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The font file name.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static LinkedListInfo *AcquireConfigureCache(const char *filename,
  ExceptionInfo *exception)
{
  char
    path[WizardPathExtent];

  const StringInfo
    *option;

  LinkedListInfo
    *configure_cache,
    *options;

  register ssize_t
    i;

  WizardStatusType
    status;

  /*
    Load built-in configure map.
  */
  configure_cache=NewLinkedList(0);
  if (configure_cache == (LinkedListInfo *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s`");
  status=WizardTrue;
  for (i=0; i < (ssize_t) (sizeof(ConfigureMap)/sizeof(*ConfigureMap)); i++)
  {
    ConfigureInfo
      *configure_info;

    register const ConfigureMapInfo
      *p;

    p=ConfigureMap+i;
    configure_info=(ConfigureInfo *) AcquireWizardMemory(
      sizeof(*configure_info));
    if (configure_info == (ConfigureInfo *) NULL)
      {
        (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
          "memory allocation failed `%s'",strerror(errno));
        continue;
      }
    (void) ResetWizardMemory(configure_info,0,sizeof(*configure_info));
    configure_info->path=(char *) "[built-in]";
    configure_info->name=(char *) p->name;
    configure_info->value=(char *) p->value;
    configure_info->exempt=WizardTrue;
    configure_info->signature=WizardSignature;
    status=AppendValueToLinkedList(configure_cache,configure_info);
    if (status == WizardFalse)
      (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
        "memory allocation failed `%s'",strerror(errno));
  }
  /*
    Load external configure map.
  */
  *path='\0';
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    (void) CopyWizardString(path,GetStringInfoPath(option),WizardPathExtent);
    status&=LoadConfigureCache(configure_cache,(const char *)
      GetStringInfoDatum(option),GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  return(configure_cache);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n f i g u r e C o m p o n e n t G e n e s i s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConfigureComponentGenesis() instantiates the configure component.
%
%  The format of the ConfigureComponentGenesis method is:
%
%      WizardBooleanType ConfigureComponentGenesis(void)
%
*/
WizardExport WizardBooleanType ConfigureComponentGenesis(void)
{
  if (configure_semaphore == (SemaphoreInfo *) NULL)
    configure_semaphore=AcquireSemaphoreInfo();
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n f i g u r e C o m p o n e n t T e r m i n u s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConfigureComponentTerminus() deallocates memory associated with the
%  configure list.
%
%  The format of the ConfigureComponentTerminus method is:
%
%      ConfigureComponentTerminus(void)
%
*/

static void *DestroyConfigureElement(void *configure_info)
{
  register ConfigureInfo
    *p;

  p=(ConfigureInfo *) configure_info;
  if (p->exempt == WizardFalse)
    {
      if (p->value != (char *) NULL)
        p->value=(char *) RelinquishWizardMemory(p->value);
      if (p->name != (char *) NULL)
        p->name=(char *) RelinquishWizardMemory(p->name);
      if (p->path != (char *) NULL)
        p->path=(char *) RelinquishWizardMemory(p->path);
    }
  p=(ConfigureInfo *) RelinquishWizardMemory(p);
  return((void *) NULL);
}

WizardExport void ConfigureComponentTerminus(void)
{
  if (configure_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&configure_semaphore);
  LockSemaphoreInfo(configure_semaphore);
  if (configure_cache != (LinkedListInfo *) NULL)
    configure_cache=DestroyLinkedList(configure_cache,DestroyConfigureElement);
  configure_cache=(LinkedListInfo *) NULL;
  UnlockSemaphoreInfo(configure_semaphore);
  RelinquishSemaphoreInfo(&configure_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C o n f i g u r e O p t i o n s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyConfigureOptions() releases memory associated with an configure
%  options.
%
%  The format of the DestroyProfiles method is:
%
%      LinkedListInfo *DestroyConfigureOptions(Image *image)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
*/

static void *DestroyOptions(void *option)
{
  return(DestroyStringInfo((StringInfo *) option));
}

WizardExport LinkedListInfo *DestroyConfigureOptions(LinkedListInfo *options)
{
  assert(options != (LinkedListInfo *) NULL);
  return(DestroyLinkedList(options,DestroyOptions));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t C o n f i g u r e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureInfo() searches the configure list for the specified name and if
%  found returns attributes for that element.
%
%  The format of the GetConfigureInfo method is:
%
%      const ConfigureInfo *GetConfigureInfo(const char *name,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o configure_info: GetConfigureInfo() searches the configure list for the
%      specified name and if found returns attributes for that element.
%
%    o name: The configure name.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport const ConfigureInfo *GetConfigureInfo(const char *name,
  ExceptionInfo *exception)
{
  register const ConfigureInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if (IsConfigureCacheInstantiated(exception) == WizardFalse)
    return((const ConfigureInfo *) NULL);
  /*
    Search for named configure.
  */
  LockSemaphoreInfo(configure_semaphore);
  ResetLinkedListIterator(configure_cache);
  p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_cache);
  if ((name == (const char *) NULL) || (strcasecmp(name,"*") == 0))
    {
      UnlockSemaphoreInfo(configure_semaphore);
      return(p);
    }
  while (p != (const ConfigureInfo *) NULL)
  {
    if (strcasecmp(name,p->name) == 0)
      break;
    p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_cache);
  }
  if (p == (ConfigureInfo *) NULL)
    (void) ThrowWizardException(exception,GetWizardModule(),OptionWarning,
      "no such configure list `%s'",name);
  else
    (void) InsertValueInLinkedList(configure_cache,0,
      RemoveElementByValueFromLinkedList(configure_cache,p));
  UnlockSemaphoreInfo(configure_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n f i g u r e I n f o L i s t                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureInfoList() returns any configure options that match the
%  specified pattern.
%
%  The format of the GetConfigureInfoList function is:
%
%      const ConfigureInfo **GetConfigureInfoList(const char *pattern,
%        size_t *number_options,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_options:  This integer returns the number of configure options in
%    the list.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ConfigureInfoCompare(const void *x,const void *y)
{
  const ConfigureInfo
    **p,
    **q;

  p=(const ConfigureInfo **) x,
  q=(const ConfigureInfo **) y;
  if (strcasecmp((*p)->path,(*q)->path) == 0)
    return(strcasecmp((*p)->name,(*q)->name));
  return(strcasecmp((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

WizardExport const ConfigureInfo **GetConfigureInfoList(const char *pattern,
  size_t *number_options,ExceptionInfo *exception)
{
  const ConfigureInfo
    **options;

  register const ConfigureInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate configure list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_options != (size_t *) NULL);
  *number_options=0;
  p=GetConfigureInfo("*",exception);
  if (p == (const ConfigureInfo *) NULL)
    return((const ConfigureInfo **) NULL);
  options=(const ConfigureInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(configure_cache)+1UL,sizeof(*options));
  if (options == (const ConfigureInfo **) NULL)
    return((const ConfigureInfo **) NULL);
  /*
    Generate configure list.
  */
  LockSemaphoreInfo(configure_semaphore);
  ResetLinkedListIterator(configure_cache);
  p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_cache);
  for (i=0; p != (const ConfigureInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->name,pattern,WizardFalse) != WizardFalse))
      options[i++]=p;
    p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_cache);
  }
  UnlockSemaphoreInfo(configure_semaphore);
  qsort((void *) options,(size_t) i,sizeof(*options),ConfigureInfoCompare);
  options[i]=(ConfigureInfo *) NULL;
  *number_options=(size_t) i;
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n f i g u r e L i s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureList() returns any configure options that match the specified
%  pattern.
%
%  The format of the GetConfigureList function is:
%
%      char **GetConfigureList(const char *pattern,
%        size_t *number_options,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_options:  This integer returns the number of options in the list.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int ConfigureCompare(const void *x,const void *y)
{
  register char
    **p,
    **q;

  p=(char **) x;
  q=(char **) y;
  return(strcasecmp(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

WizardExport char **GetConfigureList(const char *pattern,
  size_t *number_options,ExceptionInfo *exception)
{
  char
    **options;

  register const ConfigureInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate configure list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_options != (size_t *) NULL);
  *number_options=0;
  p=GetConfigureInfo("*",exception);
  if (p == (const ConfigureInfo *) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(configure_semaphore);
  UnlockSemaphoreInfo(configure_semaphore);
  options=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(configure_cache)+1UL,sizeof(*options));
  if (options == (char **) NULL)
    return((char **) NULL);
  LockSemaphoreInfo(configure_semaphore);
  ResetLinkedListIterator(configure_cache);
  p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_cache);
  for (i=0; p != (const ConfigureInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->name,pattern,WizardFalse) != WizardFalse))
      options[i++]=ConstantString(p->name);
    p=(const ConfigureInfo *) GetNextValueInLinkedList(configure_cache);
  }
  UnlockSemaphoreInfo(configure_semaphore);
  qsort((void *) options,(size_t) i,sizeof(*options),ConfigureCompare);
  options[i]=(char *) NULL;
  *number_options=(size_t) i;
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t C o n f i g u r e O p t i o n s                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureOptions() returns any Wizard configuration options associated
%  with the specified filename.
%
%  The format of the GetConfigureOptions method is:
%
%      LinkedListInfo *GetConfigureOptions(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The configure file name.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport LinkedListInfo *GetConfigureOptions(const char *filename,
  ExceptionInfo *exception)
{
  char
    path[WizardPathExtent];

  const char
    *element;

  LinkedListInfo
    *options,
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
  options=NewLinkedList(0);
  paths=GetConfigurePaths(filename,exception);
  if (paths != (LinkedListInfo *) NULL)
    {
      ResetLinkedListIterator(paths);
      element=(const char *) GetNextValueInLinkedList(paths);
      while (element != (const char *) NULL)
      {
        (void) FormatLocaleString(path,WizardPathExtent,"%s%s",element,filename);
        (void) LogWizardEvent(ConfigureEvent,GetWizardModule(),
          "Searching for configure file: \"%s\"",path);
        xml=ConfigureFileToStringInfo(path);
        if (xml != (StringInfo *) NULL)
          (void) AppendValueToLinkedList(options,xml);
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
        SetStringInfoDatum(xml,blob);
        SetStringInfoPath(xml,filename);
        (void) AppendValueToLinkedList(options,xml);
      }
  }
#endif
  if (GetNumberOfElementsInLinkedList(options) == 0)
    (void) ThrowWizardException(exception,GetWizardModule(),ConfigureWarning,
      "unable to open configure file `%s'",filename);
  ResetLinkedListIterator(options);
  return(options);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t C o n f i g u r e P a t h s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigurePaths() returns any Wizard configuration paths associated
%  with the specified filename.
%
%  The format of the GetConfigurePaths method is:
%
%      LinkedListInfo *GetConfigurePaths(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The configure file name.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport LinkedListInfo *GetConfigurePaths(const char *filename,
  ExceptionInfo *exception)
{
#define RegistryKey  "ConfigurePath"
#define WizardsToolkitDLL  "Wizard's Toolkit.dll"

  char
    path[WizardPathExtent];

  LinkedListInfo
    *paths;

  assert(filename != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) CopyWizardString(path,filename,WizardPathExtent);
  paths=NewLinkedList(0);
  {
    char
      *configure_path;

    register char
      *q;

    register const char
      *p;

    /*
      Search WIZARD_CONFIGURE_PATH.
    */
    configure_path=GetEnvironmentValue("WIZARD_CONFIGURE_PATH");
    if (configure_path != (char *) NULL)
      {
        for (p=configure_path-1; p != (char *) NULL; )
        {
          (void) CopyWizardString(path,p+1,WizardPathExtent);
          q=strchr(path,DirectoryListSeparator);
          if (q != (char *) NULL)
            *q='\0';
          q=path+strlen(path)-1;
          if ((q >= path) && (*q != *DirectorySeparator))
            (void) ConcatenateWizardString(path,DirectorySeparator,
              WizardPathExtent);
          (void) AppendValueToLinkedList(paths,AcquireString(path));
          p=strchr(p+1,DirectoryListSeparator);
        }
        configure_path=(char *) RelinquishWizardMemory(configure_path);
      }
  }
#if defined(WIZARDSTOOLKIT_INSTALLED_SUPPORT)
#if defined(WIZARDSTOOLKIT_CONFIGURE_PATH)
  (void) AppendValueToLinkedList(paths,AcquireString(
    WIZARDSTOOLKIT_CONFIGURE_PATH));
#endif
#if defined(WIZARDSTOOLKIT_SHARE_PATH)
  (void) AppendValueToLinkedList(paths,AcquireString(
    WIZARDSTOOLKIT_SHARE_PATH));
#endif
#if defined(WIZARDSTOOLKIT_SHAREARCH_PATH)
  (void) AppendValueToLinkedList(paths,ConstantString(
    WIZARDSTOOLKIT_SHAREARCH_PATH));
#endif
#if defined(WIZARDSTOOLKIT_DOCUMENTATION_PATH)
  (void) AppendValueToLinkedList(paths,AcquireString(
    WIZARDSTOOLKIT_DOCUMENTATION_PATH));
#endif
#if defined(WIZARDSTOOLKIT_SHARE_PATH)
  (void) AppendValueToLinkedList(paths,AcquireString(
    WIZARDSTOOLKIT_SHARE_PATH));
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT) && !(defined(WIZARDSTOOLKIT_CONFIGURE_PATH) || defined(WIZARDSTOOLKIT_SHARE_PATH))
  {
    unsigned char
      *key_value;

    /*
      Locate file via registry key.
    */
    key_value=NTRegistryKeyLookup(RegistryKey);
    if (key_value != (unsigned char *) NULL)
      {
        (void) FormatLocaleString(path,WizardPathExtent,"%s%s",(char *) key_value,
          DirectorySeparator);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        key_value=(unsigned char *) RelinquishWizardMemory(key_value);
      }
  }
#endif
#else
  {
    char
      *home;

    /*
      Search under WIZARD_HOME.
    */
    home=GetEnvironmentValue("WIZARD_HOME");
		if (home != (char *) NULL)
      {
#if !defined(WIZARDSTOOLKIT_POSIX_SUPPORT)
        (void) FormatLocaleString(path,WizardPathExtent,
          "%s%s.config%sWizardsToolkit%s",home,DirectorySeparator,
          DirectorySeparator,DirectorySeparator);
        (void) AppendValueToLinkedList(paths,AcquireString(path));
#else
        (void) FormatLocaleString(path,WizardPathExtent,"%s/etc/%s/",home,
          WIZARDSTOOLKIT_CONFIGURE_RELATIVE_PATH);
        (void) AppendValueToLinkedList(paths,AcquireString(path));
        (void) FormatLocaleString(path,WizardPathExtent,"%s/share/%s/",home,
          WIZARDSTOOLKIT_SHARE_RELATIVE_PATH);
        (void) AppendValueToLinkedList(paths,AcquireString(path));
        (void) FormatLocaleString(path,WizardPathExtent,"%s",
          WIZARDSTOOLKIT_SHAREARCH_PATH);
        (void) AppendValueToLinkedList(paths,AcquireString(path));
#endif
        home=(char *) RelinquishWizardMemory(home);
      }
    }
  if (*GetClientPath() != '\0')
    {
#if !defined(WIZARDSTOOLKIT_POSIX_SUPPORT)
      (void) FormatLocaleString(path,WizardPathExtent,"%s%s",GetClientPath(),
        DirectorySeparator);
      (void) AppendValueToLinkedList(paths,AcquireString(path));
#else
      char
        prefix[WizardPathExtent];

      /*
        Search based on executable directory if directory is known.
      */
      (void) CopyWizardString(prefix,GetClientPath(),WizardPathExtent);
      ChopPathComponents(prefix,1);
      (void) FormatLocaleString(path,WizardPathExtent,"%s/etc/%s/",prefix,
        WIZARDSTOOLKIT_CONFIGURE_RELATIVE_PATH);
      (void) AppendValueToLinkedList(paths,AcquireString(path));
      (void) FormatLocaleString(path,WizardPathExtent,"%s/share/%s/",prefix,
        WIZARDSTOOLKIT_SHARE_RELATIVE_PATH);
      (void) AppendValueToLinkedList(paths,AcquireString(path));
      (void) FormatLocaleString(path,WizardPathExtent,"%s",
        WIZARDSTOOLKIT_SHAREARCH_PATH);
      (void) AppendValueToLinkedList(paths,AcquireString(path));
#endif
    }
  /*
    Search current directory.
  */
  (void) AppendValueToLinkedList(paths,AcquireString(""));
#endif
  {
    char
      *home;

    home=GetEnvironmentValue("XDG_CONFIG_HOME");
    if (home == (char *) NULL)
      home=GetEnvironmentValue("LOCALAPPDATA");
    if (home == (char *) NULL)
      home=GetEnvironmentValue("APPDATA");
    if (home == (char *) NULL)
      home=GetEnvironmentValue("USERPROFILE");
    if (home != (char *) NULL)
      {
        /*
          Search $XDG_CONFIG_HOME/ImageMagick.
        */
        (void) FormatLocaleString(path,WizardPathExtent,
          "%s%sWizardsToolkit%s%s",home,DirectorySeparator,DirectorySeparator,
          filename);
        (void) AppendValueToLinkedList(paths,ConstantString(path));
        home=DestroyString(home);
      }
    home=GetEnvironmentValue("HOME");
    if (home != (char *) NULL)
      {
        /*
          Search $HOME/.config/WizardsToolkit.
        */
        (void) FormatLocaleString(path,WizardPathExtent,
          "%s%s.config%sWizardsToolkit%s",home,DirectorySeparator,
          DirectorySeparator,DirectorySeparator);
        (void) AppendValueToLinkedList(paths,AcquireString(path));
        home=(char *) RelinquishWizardMemory(home);
      }
  }
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  {
    char
      module_path[WizardPathExtent];

    if (NTGetModulePath(WizardsToolkitDLL,module_path) != WizardFalse)
      {
        unsigned char
          *key_value;

        /*
          Search module path.
        */
        (void) FormatLocaleString(path,WizardPathExtent,"%s%s",module_path,
          DirectorySeparator);
        key_value=NTRegistryKeyLookup(RegistryKey);
        if (key_value == (unsigned char *) NULL)
          (void) AppendValueToLinkedList(paths,ConstantString(path));
        else
          key_value=(unsigned char *) RelinquishWizardMemory(key_value);
       }
  }
#endif
  return(paths);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C o n f i g u r e V a l u e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetConfigureValue() returns the value associated with the configure info.
%
%  The format of the GetConfigureValue method is:
%
%      const char *GetConfigureValue(const ConfigureInfo *configure_info)
%
%  A description of each parameter follows:
%
%    o configure_info:  The configure info.
%
*/
WizardExport const char *GetConfigureValue(const ConfigureInfo *configure_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(configure_info != (ConfigureInfo *) NULL);
  assert(configure_info->signature == WizardSignature);
  return(configure_info->value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I s C o n f i g u r e L i s t I n s t a n t i a t e d                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsConfigureCacheInstantiated() determines if the configure list is
%  instantiated.  If not, it instantiates the list and returns it.
%
%  The format of the IsConfigureInstantiated method is:
%
%      WizardBooleanType IsConfigureCacheInstantiated(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType IsConfigureCacheInstantiated(ExceptionInfo *exception)
{
  if (configure_cache == (LinkedListInfo *) NULL)
    {
      if (configure_semaphore == (SemaphoreInfo *) NULL)
        ActivateSemaphoreInfo(&configure_semaphore);
      LockSemaphoreInfo(configure_semaphore);
      if (configure_cache == (LinkedListInfo *) NULL)
        configure_cache=AcquireConfigureCache(ConfigureFilename,exception);
      UnlockSemaphoreInfo(configure_semaphore);
    }
  return(configure_cache != (LinkedListInfo *) NULL ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t C o n f i g u r e I n f o                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListConfigureInfo() lists the configure info to a file.
%
%  The format of the ListConfigureInfo method is:
%
%      WizardBooleanType ListConfigureInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ListConfigureInfo(FILE *file,
  ExceptionInfo *exception)
{
  const char
    *name,
    *path,
    *value;

  const ConfigureInfo
    **configure_info;

  ssize_t
    j;

  register ssize_t
    i;

  size_t
    number_options;

  if (file == (FILE *) NULL)
    file=stdout;
  configure_info=GetConfigureInfoList("*",&number_options,exception);
  if (configure_info == (const ConfigureInfo **) NULL)
    return(WizardFalse);
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_options; i++)
  {
    if (configure_info[i]->stealth != WizardFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (strcasecmp(path,configure_info[i]->path) != 0))
      {
        if (configure_info[i]->path != (char *) NULL)
          (void) fprintf(file,"\nPath: %s\n\n",configure_info[i]->path);
        (void) fprintf(file,"Name          Value\n");
        (void) fprintf(file,"-------------------------------------------------"
          "------------------------------\n");
      }
    path=configure_info[i]->path;
    name="unknown";
    if (configure_info[i]->name != (char *) NULL)
      name=configure_info[i]->name;
    (void) fprintf(file,"%s",name);
    for (j=(ssize_t) strlen(name); j <= 12; j++)
      (void) fprintf(file," ");
    (void) fprintf(file," ");
    value="unknown";
    if (configure_info[i]->value != (char *) NULL)
      value=configure_info[i]->value;
    (void) fprintf(file,"%s",value);
    (void) fprintf(file,"\n");
  }
  (void) fflush(file);
  configure_info=(const ConfigureInfo **) RelinquishWizardMemory((void *)
    configure_info);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d C o n f i g u r e L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadConfigureCache() loads the configure configurations which provides a
%  mapping between configure attributes and a configure name.
%
%  The format of the LoadConfigureCache method is:
%
%      WizardBooleanType LoadConfigureCache(LinkedListInfo *configure_cache,
%        const char *xml,const char *filename,const size_t depth,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The configure list in XML format.
%
%    o filename:  The configure list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType LoadConfigureCache(LinkedListInfo *configure_cache,
  const char *xml,const char *filename,const size_t depth,
  ExceptionInfo *exception)
{
  const char
    *attribute;

  ConfigureInfo
    *configure_info = (ConfigureInfo *) NULL;

  WizardStatusType
    status;

  XMLTreeInfo
    *configure,
    *configure_map,
    *include;

  /*
    Load the configure map file.
  */
  (void) LogWizardEvent(ConfigureEvent,GetWizardModule(),
    "Loading configure map \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(WizardFalse);
  configure_map=NewXMLTree(xml,exception);
  if (configure_map == (XMLTreeInfo *) NULL)
    return(WizardFalse);
  status=WizardFalse;
  include=GetXMLTreeChild(configure_map,"include");
  while (include != (XMLTreeInfo *) NULL)
  {
    /*
      Process include element.
    */
    attribute=GetXMLTreeAttribute(include,"file");
    if (attribute != (const char *) NULL)
      {
        if (depth > 200)
          (void) ThrowWizardException(exception,GetWizardModule(),
            ConfigureError,"include element nested too deeply `%s'",filename);
        else
          {
            char
              path[WizardPathExtent],
              *xml;

            GetPathComponent(filename,HeadPath,path);
            if (*path != '\0')
              (void) ConcatenateWizardString(path,DirectorySeparator,
                WizardPathExtent);
            (void) ConcatenateWizardString(path,attribute,WizardPathExtent);
            xml=FileToXML(path,~0UL);
            status&=LoadConfigureCache(configure_cache,xml,path,depth+1,
              exception);
            xml=(char *) RelinquishWizardMemory(xml);
          }
      }
    include=GetNextXMLTreeTag(include);
  }
  configure=GetXMLTreeChild(configure_map,"configure");
  while (configure != (XMLTreeInfo *) NULL)
  {
    const char
      *attribute;

    /*
      Process configure element.
    */
    configure_info=(ConfigureInfo *) AcquireWizardMemory(
      sizeof(*configure_info));
    if (configure_info == (ConfigureInfo *) NULL)
      ThrowFatalException(ResourceFatalError,"memory allocation failed `%s`");
    (void) ResetWizardMemory(configure_info,0,sizeof(*configure_info));
    configure_info->path=ConstantString(filename);
    configure_info->exempt=WizardFalse;
    configure_info->signature=WizardSignature;
    attribute=GetXMLTreeAttribute(configure,"name");
    if (attribute != (const char *) NULL)
      configure_info->name=ConstantString(attribute);
    attribute=GetXMLTreeAttribute(configure,"stealth");
    if (attribute != (const char *) NULL)
      configure_info->stealth=IsWizardTrue(attribute);
    attribute=GetXMLTreeAttribute(configure,"value");
    if (attribute != (const char *) NULL)
      configure_info->value=ConstantString(attribute);
    status=AppendValueToLinkedList(configure_cache,configure_info);
    if (status == WizardFalse)
      (void) ThrowWizardException(exception,GetWizardModule(),
        ResourceFatalError,"memory allocation failed `%s`",strerror(errno));
    configure=GetNextXMLTreeTag(configure);
  }
  configure_map=DestroyXMLTree(configure_map);
  return(status != 0 ? WizardTrue : WizardFalse);
}
