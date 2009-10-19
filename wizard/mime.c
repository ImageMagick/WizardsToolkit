/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                        M   M  IIIII  M   M  EEEEE                           %
%                        MM MM    I    MM MM  E                               %
%                        M M M    I    M M M  EEE                             %
%                        M   M    I    M   M  E                               %
%                        M   M  IIIII  M   M  EEEEE                           %
%                                                                             %
%                                                                             %
%                         ImageMagick Mime Methods                            %
%                                                                             %
%                              Software Design                                %
%                                 July 2000                                   %
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
#include "wizard/blob.h"
#include "wizard/client.h"
#include "wizard/configure.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hashmap.h"
#include "wizard/memory_.h"
#include "wizard/mime.h"
#include "wizard/mime-private.h"
#include "wizard/option.h"
#include "wizard/semaphore.h"
#include "wizard/string_.h"
#include "wizard/token.h"
#include "wizard/utility.h"
#include "wizard/xml-tree.h"

/*
  Define declarations.
*/
#define MimeFilename  "mime.xml"

/*
  Typedef declaration.
*/
struct _MimeInfo
{
  char
    *path,
    *type,
    *description,
    *pattern;

  long
    priority;

  WizardOffsetType
    offset;

  size_t
    extent;

  DataType
    data_type;

  long
    mask,
    value;

  EndianType
    endian;

  size_t
    length;

  unsigned char
    *magic;

  WizardBooleanType
    stealth;

  unsigned long
    signature;
};

/*
  Static declarations.
*/
static const char
  *MimeMap = (char *)
    "<?xml version=\"1.0\"?>"
    "<mimemap>"
    "</mimemap>";

static LinkedListInfo
  *mime_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *mime_semaphore = (SemaphoreInfo *) NULL;

static volatile WizardBooleanType
  instantiate_mime = WizardFalse;

/*
  Forward declarations.
*/
static WizardBooleanType
  InitializeMimeList(ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y M i m e F a c i l i t y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMimeFacility() destroys the mime facility.
%
%  The format of the DestroyMimeFacility method is:
%
%      DestroyMimeFacility(void)
%
*/

static void *DestroyMimeElement(void *mime_info)
{
  register MimeInfo
    *p;

  p=(MimeInfo *) mime_info;
  if (p->magic != (unsigned char *) NULL)
    p->magic=(unsigned char *) RelinquishWizardMemory(p->magic);
  if (p->pattern != (char *) NULL)
    p->pattern=DestroyString(p->pattern);
  if (p->description != (char *) NULL)
    p->description=DestroyString(p->description);
  if (p->type != (char *) NULL)
    p->type=DestroyString(p->type);
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  p=(MimeInfo *) RelinquishWizardMemory(p);
  return((void *) NULL);
}

WizardExport void DestroyMimeFacility(void)
{
  AcquireSemaphoreInfo(&mime_semaphore);
  if (mime_list != (LinkedListInfo *) NULL)
    mime_list=DestroyLinkedList(mime_list,DestroyMimeElement);
  instantiate_mime=WizardFalse;
  RelinquishSemaphoreInfo(mime_semaphore);
  DestroySemaphoreInfo(&mime_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t M i m e I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeInfo() attempts to classify the content to identify which mime type
%  is associated with the content, if any.
%
%  The format of the GetMimeInfo method is:
%
%      const MimeInfo *GetMimeInfo(const char *filename,
%        const unsigned char *magic,const size_t length,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename:  If we cannot not classify the string, we attempt to classify
%      based on the filename (e.g. *.pdf returns application/pdf).
%
%    o magic: A binary string generally representing the first few characters
%      of the image file or blob.
%
%    o length: The length of the binary signature.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport const MimeInfo *GetMimeInfo(const char *filename,
  const unsigned char *magic,const size_t length,ExceptionInfo *exception)
{
  const MimeInfo
    *mime_info;

  EndianType
    endian;

  long
    value;

  register const MimeInfo
    *p;

  register const unsigned char
    *q;

  register long
    i;

  unsigned long
    lsb_first;

  assert(exception != (ExceptionInfo *) NULL);
  if ((mime_list == (LinkedListInfo *) NULL) ||
      (instantiate_mime == WizardFalse))
    if (InitializeMimeList(exception) == WizardFalse)
      return((const MimeInfo *) NULL);
  if ((mime_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(mime_list) != WizardFalse))
    return((const MimeInfo *) NULL);
  if ((magic == (const unsigned char *) NULL) || (length == 0))
    return((const MimeInfo *) GetValueFromLinkedList(mime_list,0));
  if (length == 0)
    return((const MimeInfo *) NULL);
  /*
    Search for requested mime type.
  */
  mime_info=(const MimeInfo *) NULL;
  lsb_first=1;
  AcquireSemaphoreInfo(&mime_semaphore);
  ResetLinkedListIterator(mime_list);
  p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
  while (p != (const MimeInfo *) NULL)
  {
    assert(p->offset >= 0);
    if (mime_info != (const MimeInfo *) NULL)
      if (p->priority > mime_info->priority)
        {
          p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
          continue;
        }
    if ((p->pattern != (char *) NULL) && (filename != (char *) NULL))
      {
        if (GlobExpression(filename,p->pattern,WizardFalse) != WizardFalse)
          mime_info=p;
        p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
        continue;
      }
    switch (p->data_type)
    {
      case ByteData:
      {
        if ((size_t) (p->offset+4) > length)
          break;
        q=magic+p->offset;
        value=(*q++);
        if (p->mask == 0)
          {
            if (p->value == value)
              mime_info=p;
          }
        else
          {
            if ((p->value & p->mask) == value)
              mime_info=p;
          }
        break;
      }
      case ShortData:
      {
        if ((size_t) (p->offset+4) > length)
          break;
        q=magic+p->offset;
        endian=p->endian;
        if (p->endian == UndefinedEndian)
          endian=(*(char *) &lsb_first) == 1 ? LSBEndian : MSBEndian;
        if (endian == LSBEndian)
          {
            value=(*q++);
            value|=(*q++) << 8;
          }
        else
          {
            value=(*q++) << 8;
            value|=(*q++);
          }
        if (p->mask == 0)
          {
            if (p->value == value)
              mime_info=p;
          }
        else
          {
            if ((p->value & p->mask) == value)
              mime_info=p;
          }
        break;
      }
      case LongData:
      {
        if ((size_t) (p->offset+4) > length)
          break;
        q=magic+p->offset;
        endian=p->endian;
        if (p->endian == UndefinedEndian)
          endian=(*(char *) &lsb_first) == 1 ? LSBEndian : MSBEndian;
        if (endian == LSBEndian)
          {
            value=(*q++);
            value|=(*q++) << 8;
            value|=(*q++) << 16;
            value|=(*q++) << 24;
          }
        else
          {
            value=(*q++) << 24;
            value|=(*q++) << 16;
            value|=(*q++) << 8;
            value|=(*q++);
          }
        if (p->mask == 0)
          {
            if (p->value == value)
              mime_info=p;
          }
        else
          {
            if ((p->value & p->mask) == value)
              mime_info=p;
          }
        break;
      }
      case StringData:
      default:
      {
        for (i=0; i <= (long) p->extent; i++)
        {
          if ((size_t) (p->offset+i+p->length) > length)
            break;
          if (memcmp(magic+p->offset+i,p->magic,p->length) == 0)
            {
              mime_info=p;
              break;
            }
        }
        break;
      }
    }
    p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
  }
  if (p != (const MimeInfo *) NULL)
    (void) InsertValueInLinkedList(mime_list,0,
      RemoveElementByValueFromLinkedList(mime_list,p));
  RelinquishSemaphoreInfo(mime_semaphore);
  return(mime_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M i m e I n f o L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeInfoList() returns any image aliases that match the specified
%  pattern.
%
%  The magic of the GetMimeInfoList function is:
%
%      const MimeInfo **GetMimeInfoList(const char *pattern,
%        unsigned long *number_aliases,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of magics in the
%      list.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MimeInfoCompare(const void *x,const void *y)
{
  const MimeInfo
    **p,
    **q;

  p=(const MimeInfo **) x,
  q=(const MimeInfo **) y;
  if (strcasecmp((*p)->path,(*q)->path) == 0)
    return(strcasecmp((*p)->type,(*q)->type));
  return(strcasecmp((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

WizardExport const MimeInfo **GetMimeInfoList(const char *pattern,
  unsigned long *number_aliases,ExceptionInfo *exception)
{
  const MimeInfo
    **aliases;

  register const MimeInfo
    *p;

  register long
    i;

  /*
    Allocate mime list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_aliases != (unsigned long *) NULL);
  *number_aliases=0;
  p=GetMimeInfo((char *) NULL,(unsigned char *) "*",0,exception);
  if (p == (const MimeInfo *) NULL)
    return((const MimeInfo **) NULL);
  aliases=(const MimeInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(mime_list)+1UL,sizeof(*aliases));
  if (aliases == (const MimeInfo **) NULL)
    return((const MimeInfo **) NULL);
  /*
    Generate mime list.
  */
  AcquireSemaphoreInfo(&mime_semaphore);
  ResetLinkedListIterator(mime_list);
  p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
  for (i=0; p != (const MimeInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->type,pattern,WizardFalse) != WizardFalse))
      aliases[i++]=p;
    p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
  }
  RelinquishSemaphoreInfo(mime_semaphore);
  qsort((void *) aliases,(size_t) i,sizeof(*aliases),MimeInfoCompare);
  aliases[i]=(MimeInfo *) NULL;
  *number_aliases=(unsigned long) i;
  return(aliases);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M i m e L i s t                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeList() returns any image format aliases that match the specified
%  pattern.
%
%  The format of the GetMimeList function is:
%
%      char **GetMimeList(const char *pattern,unsigned long *number_aliases,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_aliases:  This integer returns the number of image format aliases
%      in the list.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int MimeCompare(const void *x,const void *y)
{
  register char
    *p,
    *q;

  p=(char *) x;
  q=(char *) y;
  return(strcasecmp(p,q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

WizardExport char **GetMimeList(const char *pattern,
  unsigned long *number_aliases,ExceptionInfo *exception)
{
  char
    **aliases;

  register const MimeInfo
    *p;

  register long
    i;

  /*
    Allocate configure list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_aliases != (unsigned long *) NULL);
  *number_aliases=0;
  p=GetMimeInfo((char *) NULL,(unsigned char *) "*",0,exception);
  if (p == (const MimeInfo *) NULL)
    return((char **) NULL);
  aliases=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(mime_list)+1UL,sizeof(*aliases));
  if (aliases == (char **) NULL)
    return((char **) NULL);
  AcquireSemaphoreInfo(&mime_semaphore);
  ResetLinkedListIterator(mime_list);
  p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
  for (i=0; p != (const MimeInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->type,pattern,WizardFalse) != WizardFalse))
      aliases[i++]=ConstantString(p->type);
    p=(const MimeInfo *) GetNextValueInLinkedList(mime_list);
  }
  RelinquishSemaphoreInfo(mime_semaphore);
  qsort((void *) aliases,(size_t) i,sizeof(*aliases),MimeCompare);
  aliases[i]=(char *) NULL;
  *number_aliases=(unsigned long) i;
  return(aliases);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M i m e D e s c r i p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeDescription() returns the mime type description.
%
%  The format of the GetMimeDescription method is:
%
%      const char *GetMimeDescription(const MimeInfo *mime_info)
%
%  A description of each parameter follows:
%
%    o mime_info:  The magic info.
%
*/
WizardExport const char *GetMimeDescription(const MimeInfo *mime_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(mime_info != (MimeInfo *) NULL);
  assert(mime_info->signature == WizardSignature);
  return(mime_info->description);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M i m e T y p e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMimeType() returns the mime type.
%
%  The format of the GetMimeType method is:
%
%      const char *GetMimeType(const MimeInfo *mime_info)
%
%  A description of each parameter follows:
%
%    o mime_info:  The magic info.
%
*/
WizardExport const char *GetMimeType(const MimeInfo *mime_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(mime_info != (MimeInfo *) NULL);
  assert(mime_info->signature == WizardSignature);
  return(mime_info->type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e M i m e L i s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeMimeList() initializes the mime list.
%
%  The format of the InitializeMimeList method is:
%
%      WizardBooleanType InitializeMimeList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType InitializeMimeList(ExceptionInfo *exception)
{
  if ((mime_list == (LinkedListInfo *) NULL) &&
      (instantiate_mime == WizardFalse))
    {
      AcquireSemaphoreInfo(&mime_semaphore);
      if ((mime_list == (LinkedListInfo *) NULL) &&
          (instantiate_mime == WizardFalse))
        {
          (void) LoadMimeLists(MimeFilename,exception);
          instantiate_mime=WizardTrue;
        }
      RelinquishSemaphoreInfo(mime_semaphore);
    }
  return(mime_list != (LinkedListInfo *) NULL ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n s t a n t i a t e M i m e F a c i l i t y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InstantiateMimeFacility() instantiates the mime facility.
%
%  The format of the InstantiateMimeFacility method is:
%
%      WizardBooleanType InstantiateMimeFacility(void)
%
*/
WizardExport WizardBooleanType InstantiateMimeFacility(void)
{
  AcquireSemaphoreInfo(&mime_semaphore);
  RelinquishSemaphoreInfo(mime_semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t M i m e I n f o                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListMimeInfo() lists the magic info to a file.
%
%  The format of the ListMimeInfo method is:
%
%      WizardBooleanType ListMimeInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ListMimeInfo(FILE *file,ExceptionInfo *exception)
{
  const char
    *path;

  const MimeInfo
    **mime_info;

  long
    j;

  register long
    i;

  unsigned long
    number_aliases;

  if (file == (const FILE *) NULL)
    file=stdout;
  mime_info=GetMimeInfoList("*",&number_aliases,exception);
  if (mime_info == (const MimeInfo **) NULL)
    return(WizardFalse);
  j=0;
  path=(const char *) NULL;
  for (i=0; i < (long) number_aliases; i++)
  {
    if (mime_info[i]->stealth != WizardFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (strcasecmp(path,mime_info[i]->path) != 0))
      {
        if (mime_info[i]->path != (char *) NULL)
          (void) fprintf(file,"\nPath: %s\n\n",mime_info[i]->path);
        (void) fprintf(file,"Type                   Description\n");
        (void) fprintf(file,"-------------------------------------------------"
          "------------------------------\n");
      }
    path=mime_info[i]->path;
    (void) fprintf(file,"%s",mime_info[i]->type);
    if (strlen(mime_info[i]->type) <= 25)
      {
        for (j=(long) strlen(mime_info[i]->type); j <= 27; j++)
          (void) fprintf(file," ");
      }
    else
      {
        (void) fprintf(file,"\n");
        for (j=0; j <= 27; j++)
          (void) fprintf(file," ");
      }
    if (mime_info[i]->description != (char *) NULL)
      (void) fprintf(file,"%s",mime_info[i]->description);
    (void) fprintf(file,"\n");
  }
  (void) fflush(file);
  mime_info=(const MimeInfo **) RelinquishWizardMemory((void *) mime_info);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d M i m e L i s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadMimeList() loads the magic configuration file which provides a mapping
%  between magic attributes and a magic name.
%
%  The format of the LoadMimeList method is:
%
%      WizardBooleanType LoadMimeList(const char *xml,const char *filename,
%        const unsigned long depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The mime list in XML format.
%
%    o filename:  The mime list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType LoadMimeList(const char *xml,const char *filename,
  const unsigned long depth,ExceptionInfo *exception)
{
  const char
    *attribute;

  MimeInfo
    *mime_info = (MimeInfo *) NULL;

  WizardBooleanType
    status;

  XMLTreeInfo
    *mime,
    *mime_map,
    *include;

  /*
    Load the mime map file.
  */
  (void) LogWizardEvent(ConfigureEvent,GetWizardModule(),
    "Loading mime map \"%s\" ...",filename);
  if (xml == (const char *) NULL)
    return(WizardFalse);
  if (mime_list == (LinkedListInfo *) NULL)
    {
      mime_list=NewLinkedList(0);
      if (mime_list == (LinkedListInfo *) NULL)
        ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
    }
  mime_map=NewXMLTree(xml,exception);
  if (mime_map == (XMLTreeInfo *) NULL)
    return(WizardFalse);
  status=WizardTrue;
  include=GetXMLTreeChild(mime_map,"include");
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
              path[MaxTextExtent],
              *xml;

            GetPathComponent(filename,HeadPath,path);
            if (*path != '\0')
              (void) ConcatenateWizardString(path,DirectorySeparator,
                MaxTextExtent);
            if (*attribute == *DirectorySeparator)
              (void) CopyWizardString(path,attribute,MaxTextExtent);
            else
              (void) ConcatenateWizardString(path,attribute,MaxTextExtent);
            xml=FileToString(path,~0,exception);
            if (xml != (char *) NULL)
              {
                status=LoadMimeList(xml,path,depth+1,exception);
                xml=DestroyString(xml);
              }
          }
      }
    include=GetNextXMLTreeTag(include);
  }
  mime=GetXMLTreeChild(mime_map,"mime");
  while (mime != (XMLTreeInfo *) NULL)
  {
    const char
      *attribute;

    /*
      Process mime element.
    */
    mime_info=(MimeInfo *) AcquireWizardMemory(sizeof(*mime_info));
    if (mime_info == (MimeInfo *) NULL)
      ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
    (void) ResetWizardMemory(mime_info,0,sizeof(*mime_info));
    mime_info->path=ConstantString(filename);
    mime_info->signature=WizardSignature;
    attribute=GetXMLTreeAttribute(mime,"data-type");
    if (attribute != (const char *) NULL)
      mime_info->data_type=(DataType) ParseWizardOption(WizardDataTypeOptions,
        WizardTrue,attribute);
    attribute=GetXMLTreeAttribute(mime,"description");
    if (attribute != (const char *) NULL)
      mime_info->description=ConstantString(attribute);
    attribute=GetXMLTreeAttribute(mime,"endian");
    if (attribute != (const char *) NULL)
      mime_info->endian=(EndianType) ParseWizardOption(WizardEndianOptions,
        WizardTrue,attribute);
    attribute=GetXMLTreeAttribute(mime,"magic");
    if (attribute != (const char *) NULL)
      {
        char
          *token;

        const char
          *p;

        register unsigned char
          *q;

        token=AcquireString(attribute);
        (void) SubstituteString((char **) &token,"&lt;","<");
        (void) SubstituteString((char **) &token,"&amp;","&");
        (void) SubstituteString((char **) &token,"&quot;","\"");
        mime_info->magic=(unsigned char *) AcquireString(token);
        q=mime_info->magic;
        for (p=token; *p != '\0'; )
        {
          if (*p == '\\')
            {
              p++;
              if (isdigit((int) ((unsigned char) *p)) != 0)
                {
                  char
                    *end;

                  *q++=(unsigned char) strtol(p,&end,8);
                  p+=(end-p);
                  mime_info->length++;
                  continue;
                }
              switch (*p)
              {
                case 'b': *q='\b'; break;
                case 'f': *q='\f'; break;
                case 'n': *q='\n'; break;
                case 'r': *q='\r'; break;
                case 't': *q='\t'; break;
                case 'v': *q='\v'; break;
                case 'a': *q='a'; break;
                case '?': *q='\?'; break;
                default: *q=(unsigned char) (*p); break;
              }
              p++;
              q++;
              mime_info->length++;
              continue;
            }
          *q++=(unsigned char) (*p++);
          mime_info->length++;
        }
        token=DestroyString(token);
        if (mime_info->data_type != StringData)
          mime_info->value=strtol((char *) mime_info->magic,(char **) NULL,0);
      }
    attribute=GetXMLTreeAttribute(mime,"mask");
    if (attribute != (const char *) NULL)
      mime_info->mask=strtol(attribute,(char **) NULL,0);
    attribute=GetXMLTreeAttribute(mime,"offset");
    if (attribute != (const char *) NULL)
      {
        char
          *c;

        mime_info->offset=(WizardOffsetType) strtol(attribute,&c,0);
        if (*c == ':')
          mime_info->extent=(size_t) strtol(c+1,(char **) NULL,0);
      }
    attribute=GetXMLTreeAttribute(mime,"pattern");
    if (attribute != (const char *) NULL)
      mime_info->pattern=ConstantString(attribute);
    attribute=GetXMLTreeAttribute(mime,"priority");
    if (attribute != (const char *) NULL)
      mime_info->priority=strtol(attribute,(char **) NULL,0);
    attribute=GetXMLTreeAttribute(mime,"stealth");
    if (attribute != (const char *) NULL)
      mime_info->stealth=IsWizardTrue(attribute);
    attribute=GetXMLTreeAttribute(mime,"type");
    if (attribute != (const char *) NULL)
      mime_info->type=ConstantString(attribute);
    status=AppendValueToLinkedList(mime_list,mime_info);
    if (status == WizardFalse)
      (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
        "memory allocation failed `%s'",filename);
    mime=GetNextXMLTreeTag(mime);
  }
  mime_map=DestroyXMLTree(mime_map);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L o a d M i m e L i s t s                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadMimeList() loads one or more magic configuration file which provides a
%  mapping between magic attributes and a magic name.
%
%  The format of the LoadMimeLists method is:
%
%      WizardBooleanType LoadMimeLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The font file name.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType LoadMimeLists(const char *filename,
  ExceptionInfo *exception)
{
#if defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
  return(LoadMimeList(MimeMap,"built-in",0,exception));
#else
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  WizardStatusType
    status;

  status=WizardFalse;
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status|=LoadMimeList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  options=DestroyConfigureOptions(options);
  if ((mime_list == (LinkedListInfo *) NULL) || 
      (IsLinkedListEmpty(mime_list) != WizardFalse))
    status|=LoadMimeList(MimeMap,"built-in",0,exception);
  else
    ClearWizardException(exception);
  return(status != 0 ? WizardTrue : WizardFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W i z a r d T o M i m e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardToMime() returns the officially registered (or de facto) MIME
%  media-type corresponding to a wizard string.  If there is no registered
%  media-type, then the string "image/x-wizard" (all lower case) is returned.
%  The returned string must be deallocated by the user.
%
%  The format of the WizardToMime method is:
%
%      char *WizardToMime(const char *wizard)
%
%  A description of each parameter follows.
%
%   o  wizard:  ImageMagick format specification "wizard" tag.
%
*/
WizardExport char *WizardToMime(const char *wizard)
{
  char
    filename[MaxTextExtent],
    media[MaxTextExtent];

  const MimeInfo
    *mime_info;

  ExceptionInfo
    *exception;

  (void) FormatWizardString(filename,MaxTextExtent,"file.%s",wizard);
  LocaleLower(filename);
  exception=AcquireExceptionInfo();
  mime_info=GetMimeInfo(filename,(unsigned char *) " ",1,exception);
  exception=DestroyExceptionInfo(exception);
  if (mime_info != (const MimeInfo *) NULL)
    return(ConstantString(GetMimeType(mime_info)));
  (void) FormatWizardString(media,MaxTextExtent,"image/x-%s",wizard);
  LocaleLower(media+8);
  return(ConstantString(media));
}
