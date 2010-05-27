/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           RRRR    EEEEE   SSSSS   OOO   U   U  RRRR    CCCC  EEEEE          %
%           R   R   E       SS     O   O  U   U  R   R  C      E              %
%           RRRR    EEE      SSS   O   O  U   U  RRRR   C      EEE            %
%           R R     E          SS  O   O  U   U  R R    C      E              %
%           R  R    EEEEE   SSSSS   OOO    UUU   R  R    CCCC  EEEEE          %
%                                                                             %
%                                                                             %
%                        Get/Set ImageMagick Resources.                       %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               September 2002                                %
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
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hashmap.h"
#include "wizard/log.h"
#include "wizard/memory_.h"
#include "wizard/option.h"
#include "wizard/random_.h"
#include "wizard/resource_.h"
#include "wizard/semaphore.h"
#include "wizard/signature.h"
#include "wizard/splay-tree.h"
#include "wizard/string_.h"
#include "wizard/string-private.h"
#include "wizard/token.h"
#include "wizard/utility.h"

/*
  Define  declarations.
*/
#define WizardResourceInfinity  (~0UL)

/*
  Typedef declarations.
*/
typedef struct _ResourceInfo
{
  WizardOffsetType
    area,
    memory,
    map,
    disk,
    file;

  WizardSizeType
    area_limit,
    memory_limit,
    map_limit,
    disk_limit,
    file_limit;
} ResourceInfo;

/*
  Global declarations.
*/
static RandomInfo
  *random_info = (RandomInfo *) NULL;

static ResourceInfo
  resource_info =
  {
    WizardULLConstant(0),
    WizardULLConstant(0),
    WizardULLConstant(0),
    WizardULLConstant(0),
    WizardULLConstant(0),
    WizardULLConstant(2048)*1024*1024,
    WizardULLConstant(1536)*1024*1024,
    WizardULLConstant(8192)*1024*1024,
    WizardResourceInfinity,
    WizardULLConstant(768)
  };

static SemaphoreInfo
  *resource_semaphore = (SemaphoreInfo *) NULL;

static SplayTreeInfo
  *temporary_resources = (SplayTreeInfo *) NULL;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e U n i q u e F i l e n a m e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireUniqueFilename() replaces the contents of path by a unique path name.
%
%  The format of the AcquireUniqueFilename method is:
%
%      WizardBooleanType AcquireUniqueFilename(char *path,Exception *exception)
%
   A description of each parameter follows.
%
%   o  path:  Specifies a pointer to an array of characters.  The unique path
%      name is returned in this array.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType AcquireUniqueFilename(char *path,
  ExceptionInfo *exception)
{
  int
    file;

  file=AcquireUniqueFileResource("",path,exception);
  if (file == -1)
    return(WizardFalse);
  if (close(file) == -1)
    (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
      "unable to close file `%s': %s",path,strerror(errno));
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e U n i q u e F i l e R e s o u r c e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireUniqueFileResource() returns a unique file name, and returns a file
%  descriptor for the file open for reading and writing.
%
%  The format of the AcquireUniqueFileResource() method is:
%
%      int AcquireUniqueFileResource(const char *path,char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%   o  path:  Open the file in this directory path.
%
%   o  filename:  The unique filename is returned in this array.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static void *DestroyTemporaryResources(void *temporary_resource)
{
  remove((char *) temporary_resource);
  return((void *) NULL);
}

static WizardBooleanType GetPathTemplate(const char *path,char *filename)
{
  char
    *directory;

  int
    status;

  register char
    *p;

  struct stat
    file_info;

  (void) CopyWizardString(filename,"wizard-XXXXXXXX",MaxTextExtent);
  if (*path != '\0')
    directory=ConstantString(path);
  else
    {
      directory=GetEnvironmentValue("WIZARD_TMPDIR");
      if (directory == (char *) NULL)
        directory=GetEnvironmentValue("TMPDIR");
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
      if (directory == (char *) NULL)
        directory=GetEnvironmentValue("TMP");
      if (directory == (char *) NULL)
        directory=GetEnvironmentValue("TEMP");
#endif
#if defined(P_tmpdir)
      if (directory == (char *) NULL)
        directory=ConstantString(P_tmpdir);
#endif
      if (directory == (char *) NULL)
        return(WizardTrue);
    }
  if (strlen(directory) > (MaxTextExtent-15))
    {
      directory=(char *) RelinquishWizardMemory(directory);
      return(WizardTrue);
    }
  status=stat(directory,&file_info);
  if ((status != 0) || !S_ISDIR(file_info.st_mode))
    {
      directory=(char *) RelinquishWizardMemory(directory);
      return(WizardTrue);
    }
  if (directory[strlen(directory)-1] == *DirectorySeparator)
    (void) FormatWizardString(filename,MaxTextExtent,"%swizard-XXXXXXXX",
      directory);
  else
    (void) FormatWizardString(filename,MaxTextExtent,"%s%swizard-XXXXXXXX",
      directory,DirectorySeparator);
  directory=(char *) RelinquishWizardMemory(directory);
  if (*DirectorySeparator != '/')
    for (p=filename; *p != '\0'; p++)
      if (*p == *DirectorySeparator)
        *p='/';
  return(WizardTrue);
}

WizardExport int AcquireUniqueFileResource(const char *path,char *filename,
  ExceptionInfo *exception)
{
#if !defined(O_NOFOLLOW)
#define O_NOFOLLOW 0
#endif
#if !defined(TMP_MAX)
# define TMP_MAX  238328
#endif

  char
    *resource;

  int
    c,
    file;

  register char
    *p;

  register ssize_t
    i;

  static const char
    portable_filename[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

  StringInfo
    *key;

  unsigned char
    *datum;

  WizardAssert(ResourceDomain,path != (const char *) NULL);
  WizardAssert(ResourceDomain,filename != (char *) NULL);
  (void) LogWizardEvent(ResourceEvent,GetWizardModule(),"%s",path);
  if (random_info == (RandomInfo *) NULL)
    random_info=AcquireRandomInfo(SHA256Hash);
  file=(-1);
  for (i=0; i < (ssize_t) TMP_MAX; i++)
  {
    /*
      Get temporary pathname.
    */
    (void) GetPathTemplate(path,filename);
#if defined(WIZARDSTOOLKIT_HAVE_MKSTEMP)
    file=mkstemp(filename);
    if (file != -1)
      break;
#endif
    key=GetRandomKey(random_info,8);
    p=filename+strlen(filename)-8;
    datum=GetStringInfoDatum(key);
    for (i=0; i < (ssize_t) GetStringInfoLength(key); i++)
    {
      c=(int) (datum[i] & 0x3f);
      *p++=portable_filename[c];
    }
    key=DestroyStringInfo(key);
    file=open(filename,O_RDWR | O_CREAT | O_EXCL | O_BINARY | O_NOFOLLOW,
      S_MODE);
    if ((file > 0) || (errno != EEXIST))
      break;
  }
  (void) LogWizardEvent(ResourceEvent,GetWizardModule(),"%s",filename);
  if (file == -1)
    return(file);
  if (AcquireWizardResource(FileResource,1) == WizardFalse)
    {
      if (close(file) == -1)
        (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
          "unable to close file `%s': %s",filename,strerror(errno));
      if (remove(path) == -1)
        (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
          "unable to remove file `%s': %s",filename,strerror(errno));
      return(-1);
    }
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  if (temporary_resources == (SplayTreeInfo *) NULL)
    temporary_resources=NewSplayTree(CompareSplayTreeString,
      RelinquishWizardMemory,DestroyTemporaryResources);
  UnlockSemaphoreInfo(resource_semaphore);
  resource=ConstantString(filename);
  (void) AddValueToSplayTree(temporary_resources,resource,resource);
  return(file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e W i z a r d R e s o u r c e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireWizardResource() acquires resources of the specified type.
%  WizardFalse is returned if the specified resource is exhausted otherwise
%  WizardTrue.
%
%  The format of the AcquireWizardResource() method is:
%
%      WizardBooleanType AcquireWizardResource(const ResourceType type,
%        const WizardSizeType size)
%
%  A description of each parameter follows:
%
%    o type: The type of resource.
%
%    o size: The number of bytes needed from for this resource.
%
*/
WizardExport WizardBooleanType AcquireWizardResource(const ResourceType type,
  const WizardSizeType size)
{
  char
    resource_current[MaxTextExtent],
    resource_limit[MaxTextExtent],
    resource_request[MaxTextExtent];

  WizardBooleanType
    status;

  WizardSizeType
    limit;

  status=WizardFalse;
  (void) FormatWizardSize(size,WizardFalse,resource_request);
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area=(WizardOffsetType) size;
      limit=resource_info.area_limit;
      status=(resource_info.area_limit == WizardResourceInfinity) ||
        (size < limit) ? WizardTrue : WizardFalse;
      (void) FormatWizardSize((WizardSizeType) resource_info.area,WizardFalse,
        resource_current);
      (void) FormatWizardSize(resource_info.area_limit,WizardFalse,
        resource_limit);
      break;
    }
    case MemoryResource:
    {
      resource_info.memory+=size;
      limit=resource_info.memory_limit;
      status=(resource_info.memory_limit == WizardResourceInfinity) ||
        ((WizardSizeType) resource_info.memory < limit) ?
        WizardTrue : WizardFalse;
      (void) FormatWizardSize((WizardSizeType) resource_info.memory,WizardTrue,
        resource_current);
      (void) FormatWizardSize(resource_info.memory_limit,WizardTrue,
        resource_limit);
      break;
    }
    case MapResource:
    {
      resource_info.map+=size;
      limit=resource_info.map_limit;
      status=(resource_info.map_limit == WizardResourceInfinity) ||
        ((WizardSizeType) resource_info.map < limit) ?
        WizardTrue : WizardFalse;
      (void) FormatWizardSize((WizardSizeType) resource_info.map,WizardTrue,
        resource_current);
      (void) FormatWizardSize(resource_info.map_limit,WizardTrue,
        resource_limit);
      break;
    }
    case DiskResource:
    {
      resource_info.disk+=size;
      limit=resource_info.disk_limit;
      status=(resource_info.disk_limit == WizardResourceInfinity) ||
        ((WizardSizeType) resource_info.disk < limit) ?
        WizardTrue : WizardFalse;
      (void) FormatWizardSize((WizardSizeType) resource_info.disk,WizardFalse,
        resource_current);
      (void) FormatWizardSize(resource_info.disk_limit,WizardFalse,
        resource_limit);
      break;
    }
    case FileResource:
    {
      resource_info.file+=size;
      limit=resource_info.file_limit;
      status=(resource_info.file_limit == WizardResourceInfinity) ||
        ((WizardSizeType) resource_info.file < limit) ?
        WizardTrue : WizardFalse;
      (void) FormatWizardSize((WizardSizeType) resource_info.file,WizardFalse,
        resource_current);
      (void) FormatWizardSize((WizardSizeType) resource_info.file_limit,
        WizardFalse,resource_limit);
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  (void) LogWizardEvent(ResourceEvent,GetWizardModule(),"%s: %sB/%sB/%sB",
    WizardOptionToMnemonic(WizardResourceOptions,(ssize_t) type),resource_request,
    resource_current,resource_limit);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A s y n c h r o n o u s R e s o u r c e C o m p o n e n t T e r m i n u s %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AsynchronousResourceComponentTerminus() destroys the resource environment.
%  It differs from ResourceComponentTerminus() in that it can be called from a
%  asynchronous signal handler.
%
%  The format of the ResourceComponentTerminus() method is:
%
%      ResourceComponentTerminus(void)
%
*/
WizardExport void AsynchronousResourceComponentTerminus(void)
{
  const char
    *path;

  if (temporary_resources == (SplayTreeInfo *) NULL)
    return;
  /*
    Remove any lingering temporary files.
  */
  ResetSplayTreeIterator(temporary_resources);
  path=(const char *) GetNextKeyInSplayTree(temporary_resources);
  while (path != (const char *) NULL)
  {
    (void) remove(path);
    path=(const char *) GetNextKeyInSplayTree(temporary_resources);
  }
  if (random_info != (RandomInfo *) NULL)
    random_info=DestroyRandomInfo(random_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a r d R e s o u r c e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardResource() returns the the specified resource in megabytes.
%
%  The format of the GetWizardResource() method is:
%
%      WizardSizeType GetWizardResource(const ResourceType type)
%
%  A description of each parameter follows:
%
%    o type: The type of resource.
%
*/
WizardExport WizardSizeType GetWizardResource(const ResourceType type)
{
  WizardSizeType
    resource;

  resource=0;
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource=(WizardSizeType) resource_info.area;
      break;
    }
    case MemoryResource:
    {
      resource=(WizardSizeType) resource_info.memory;
      break;
    }
    case MapResource:
    {
      resource=(WizardSizeType) resource_info.map;
      break;
    }
    case DiskResource:
    {
      resource=(WizardSizeType) resource_info.disk;
      break;
    }
    case FileResource:
    {
      resource=(WizardSizeType) resource_info.file;
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  return(resource);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a r d R e s o u r c e L i m i t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardResource() returns the the specified resource limit in megabytes.
%
%  The format of the GetWizardResourceLimit() method is:
%
%      size_t GetWizardResourceLimit(const ResourceType type)
%
%  A description of each parameter follows:
%
%    o type: The type of resource.
%
*/
WizardExport WizardSizeType GetWizardResourceLimit(const ResourceType type)
{
  WizardSizeType
    resource;

  resource=0;
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource=(WizardSizeType) resource_info.area_limit;
      break;
    }
    case MemoryResource:
    {
      resource=(WizardSizeType) resource_info.memory_limit;
      break;
    }
    case MapResource:
    {
      resource=(WizardSizeType) resource_info.map_limit;
      break;
    }
    case DiskResource:
    {
      resource=(WizardSizeType) resource_info.disk_limit;
      break;
    }
    case FileResource:
    {
      resource=(WizardSizeType) resource_info.file_limit;
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  return(resource);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t W i z a r d R e s o u r c e I n f o                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListWizardResourceInfo() lists the resource info to a file.
%
%  The format of the ListWizardResourceInfo method is:
%
%      WizardBooleanType ListWizardResourceInfo(FILE *file,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ListWizardResourceInfo(FILE *file,
  ExceptionInfo *wizard_unused(exception))
{
  char
    area_limit[MaxTextExtent],
    disk_limit[MaxTextExtent],
    map_limit[MaxTextExtent],
    memory_limit[MaxTextExtent];

  if (file == (const FILE *) NULL)
    file=stdout;
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  (void) FormatWizardSize(resource_info.area_limit,WizardFalse,area_limit);
  (void) FormatWizardSize(resource_info.map_limit,WizardTrue,map_limit);
  (void) FormatWizardSize(resource_info.memory_limit,WizardTrue,memory_limit);
  (void) FormatWizardSize(resource_info.disk_limit,WizardFalse,disk_limit);
  (void) fprintf(file,"File        Area      Memory         Map        Disk\n");
  (void) fprintf(file,"----------------------------------------------------\n");
  (void) fprintf(file,"%4lu  %9sB  %9sB  %9sB  %9sB\n",(size_t)
    resource_info.file_limit,area_limit,memory_limit,map_limit,disk_limit);
  (void) fflush(file);
  UnlockSemaphoreInfo(resource_semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h W i z a r d R e s o u r c e                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishWizardResource() relinquishes resources of the specified type.
%
%  The format of the RelinquishWizardResource() method is:
%
%      void RelinquishWizardResource(const ResourceType type,
%        const WizardSizeType size)
%
%  A description of each parameter follows:
%
%    o type: The type of resource.
%
%    o size: The size of the resource.
%
*/
WizardExport void RelinquishWizardResource(const ResourceType type,
  const WizardSizeType size)
{
  char
    resource_current[MaxTextExtent],
    resource_limit[MaxTextExtent],
    resource_request[MaxTextExtent];

  (void) FormatWizardSize(size,WizardFalse,resource_request);
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area=(WizardOffsetType) size;
      (void) FormatWizardSize((WizardSizeType) resource_info.area,
        WizardFalse,resource_current);
      (void) FormatWizardSize(resource_info.area_limit,
        WizardFalse,resource_limit);
      break;
    }
    case MemoryResource:
    {
      resource_info.memory-=size;
      (void) FormatWizardSize((WizardSizeType) resource_info.memory,WizardTrue,
        resource_current);
      (void) FormatWizardSize(resource_info.memory_limit,WizardTrue,
        resource_limit);
      break;
    }
    case MapResource:
    {
      resource_info.map-=size;
      (void) FormatWizardSize((WizardSizeType) resource_info.map,WizardTrue,
        resource_current);
      (void) FormatWizardSize(resource_info.map_limit,WizardTrue,
        resource_limit);
      break;
    }
    case DiskResource:
    {
      resource_info.disk-=size;
      (void) FormatWizardSize((WizardSizeType) resource_info.disk,WizardFalse,
        resource_current);
      (void) FormatWizardSize(resource_info.disk_limit,WizardFalse,
        resource_limit);
      break;
    }
    case FileResource:
    {
      resource_info.file-=size;
      (void) FormatWizardSize((WizardSizeType) resource_info.file,WizardFalse,
        resource_current);
      (void) FormatWizardSize((WizardSizeType) resource_info.file_limit,
        WizardFalse,resource_limit);
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  (void) LogWizardEvent(ResourceEvent,GetWizardModule(),"%s: %sB/%sB/%sB",
    WizardOptionToMnemonic(WizardResourceOptions,(ssize_t) type),resource_request,
    resource_current,resource_limit);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    R e l i n q u i s h U n i q u e F i l e R e s o u r c e                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishUniqueFileResource() relinquishes a unique file resource.
%
%  The format of the RelinquishUniqueFileResource() method is:
%
%      WizardBooleanType RelinquishUniqueFileResource(const char *path,
%        const WizardBooleanType trash))
%
%  A description of each parameter follows:
%
%    o name: the name of the temporary resource.
%
%    o trash: a value other than 0 trashs the file at the specified path.
%
*/
WizardExport WizardBooleanType RelinquishUniqueFileResource(const char *path,
  const WizardBooleanType trash)
{
  WizardAssert(ResourceDomain,path != (const char *) NULL);
  (void) LogWizardEvent(ResourceEvent,GetWizardModule(),"%s",path);
  if (temporary_resources != (SplayTreeInfo *) NULL)
    {
      register char
        *p;

      ResetSplayTreeIterator(temporary_resources);
      p=(char *) GetNextKeyInSplayTree(temporary_resources);
      while (p != (char *) NULL)
      {
        if (LocaleCompare(p,path) == 0)
          break;
        p=(char *) GetNextKeyInSplayTree(temporary_resources);
      }
      if (p != (char *) NULL)
        (void) DeleteNodeFromSplayTree(temporary_resources,p);
    }
  if (trash == WizardFalse)
    return(WizardTrue);
  return(remove(path) == 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s o u r c e C o m p o n e n t G e n e s i s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResourceComponentGenesis() instantiates the resource component.
%
%  The format of the ResourceComponentGenesis method is:
%
%      WizardBooleanType ResourceComponentGenesis(void)
%
*/

static inline ssize_t WizardMax(const ssize_t x,const ssize_t y)
{
  if (x > y)
    return(x);
  return(y);
}

WizardExport WizardBooleanType ResourceComponentGenesis(void)
{
  char
    *limit;

  ssize_t
    files,
    pages,
    pagesize;

  size_t
    memory;

  /*
    Set Wizard resource limits.
  */
  AcquireSemaphoreInfo(&resource_semaphore);
  pagesize=(-1);
#if defined(WIZARDSTOOLKIT_HAVE_SYSCONF) && defined(_SC_PAGESIZE)
  pagesize=sysconf(_SC_PAGESIZE);
#elif defined(WIZARDSTOOLKIT_HAVE_GETPAGESIZE) && defined(POSIX)
  pagesize=getpagesize();
#endif
  pages=(-1);
#if defined(WIZARDSTOOLKIT_HAVE_SYSCONF) && defined(_SC_PHYS_PAGES)
  pages=sysconf(_SC_PHYS_PAGES);
#endif
  memory=(size_t) ((pages+512)/1024)*((pagesize+512)/1024);
  if ((pagesize <= 0) || (pages <= 0))
    memory=2048UL;
#if defined(PixelCacheThreshold)
  memory=PixelCacheThreshold;
#endif
  (void) SetWizardResourceLimit(AreaResource,2*memory);
  (void) SetWizardResourceLimit(MemoryResource,3*memory/2);
  (void) SetWizardResourceLimit(MapResource,4*memory);
  limit=GetEnvironmentValue("WIZARD_AREA_LIMIT");
  if (limit != (char *) NULL)
    {
      (void) SetWizardResourceLimit(AreaResource,StringToUnsignedLong(limit));
      limit=DestroyString(limit);
    }
  limit=GetEnvironmentValue("WIZARD_MEMORY_LIMIT");
  if (limit != (char *) NULL)
    {
      (void) SetWizardResourceLimit(MemoryResource,StringToUnsignedLong(limit));
      limit=DestroyString(limit);
    }
  limit=GetEnvironmentValue("WIZARD_MAP_LIMIT");
  if (limit != (char *) NULL)
    {
      (void) SetWizardResourceLimit(MapResource,StringToUnsignedLong(limit));
      limit=DestroyString(limit);
    }
  limit=GetEnvironmentValue("WIZARD_DISK_LIMIT");
  if (limit != (char *) NULL)
    {
      (void) SetWizardResourceLimit(DiskResource,StringToUnsignedLong(limit));
      limit=DestroyString(limit);
    }
  files=(-1);
#if defined(WIZARDSTOOLKIT_HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
  files=sysconf(_SC_OPEN_MAX);
#elif defined(WIZARDSTOOLKIT_HAVE_GETDTABLESIZE) && defined(POSIX)
  files=getdtablesize();
#endif
  (void) SetWizardResourceLimit(FileResource,(size_t)
    WizardMax(3L*files/4L,64L));
  limit=GetEnvironmentValue("WIZARD_FILE_LIMIT");
  if (limit != (char *) NULL)
    {
      (void) SetWizardResourceLimit(FileResource,StringToUnsignedLong(limit));
      limit=DestroyString(limit);
    }
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R e s o u r c e C o m p o n e n t G e n e s i s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResourceComponentTerminus() destroys the resource component.
%
%  The format of the ResourceComponentTerminus() method is:
%
%      ResourceComponentTerminus(void)
%
*/
WizardExport void ResourceComponentTerminus(void)
{
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  if (temporary_resources != (SplayTreeInfo *) NULL)
    temporary_resources=DestroySplayTree(temporary_resources);
  UnlockSemaphoreInfo(resource_semaphore);
  DestroySemaphoreInfo(&resource_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t W i z a r d R e s o u r c e L i m i t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetWizardResourceLimit() sets the limit for a particular resource in bytes.
%
%  The format of the SetWizardResourceLimit() method is:
%
%      WizardBooleanType SetWizardResourceLimit(const ResourceType type,
%        const WizardResourceType limit)
%
%  A description of each parameter follows:
%
%    o type: The type of resource.
%
%    o limit: The maximum limit for the resource.
%
*/
WizardExport WizardBooleanType SetWizardResourceLimit(const ResourceType type,
  const WizardSizeType limit)
{
  if (resource_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&resource_semaphore);
  LockSemaphoreInfo(resource_semaphore);
  switch (type)
  {
    case AreaResource:
    {
      resource_info.area_limit=limit;
      break;
    }
    case MemoryResource:
    {
      resource_info.memory_limit=limit;
      break;
    }
    case MapResource:
    {
      resource_info.map_limit=limit;
      break;
    }
    case DiskResource:
    {
      resource_info.disk_limit=limit;
      break;
    }
    case FileResource:
    {
      resource_info.file_limit=limit;
      break;
    }
    default:
      break;
  }
  UnlockSemaphoreInfo(resource_semaphore);
  return(WizardTrue);
}
