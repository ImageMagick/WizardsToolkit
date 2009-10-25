/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                         FFFFF  IIIII  L      EEEEE                          %
%                         F        I    L      E                              %
%                         FFF      I    L      EEE                            %
%                         F        I    L      E                              %
%                         F      IIIII  LLLLL  EEEEE                          %
%                                                                             %
%                                                                             %
%                         Wizard's Toolkit File Methods                       %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
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
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/blob.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/file.h"
#include "wizard/memory_.h"
#include "wizard/semaphore.h"
#include "wizard/string_.h"
#include "wizard/utility.h"
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
#include <pthread.h>
#endif
#if defined(__WINDOWS__)
#include <windows.h>
#endif

/*
  Forward declarations.
*/
static WizardBooleanType
  RelinquishFileLock(FileInfo *,ExceptionInfo *);

/*
  Typedef declarations.
*/
struct _FileInfo
{
  char
    *path;

  int
    file;

  struct stat
    properties;

  SemaphoreInfo
    *semaphore;

  time_t
    timestamp;

  unsigned long
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e F i l e L o c k                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireFileLock() acquires a lock for a file.
%
%  The format of the AcquireFileLock method is:
%
%      WizardBooleanType AcquireFileLock(FileInfo *file_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static unsigned long GetWizardThreadId(void)
{
#if defined(WIZARDSTOOLKIT_HAVE_PTHREAD)
  return((unsigned long) pthread_self());
#endif
#if defined(__WINDOWS__)
  return((unsigned long) GetCurrentThreadId());
#endif
  return((unsigned long) getpid());
}

static WizardBooleanType AcquireFileLock(FileInfo *file_info,
  ExceptionInfo *exception)
{
  char
    *path;

  long
    pid;

  register long
    i;

  unsigned long
    tid;

  WizardStatusType
    status;

  /*
    Engage primitive atomic lock.
  */
  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",file_info->path);
  LockSemaphoreInfo(file_info->semaphore);
  path=AcquireString(file_info->path);
  AppendFileExtension("lck",path);
  for (i=0; i < 10; i++)
  {
    file_info->file=open(path,O_WRONLY | O_CREAT | O_EXCL,S_MODE);
    if (file_info->file == -1)
      {
        if (errno != EEXIST)
          break;
        file_info->file=open(path,O_RDONLY);
        if (file_info->file == -1)
          break;
        pid=(-1);
        tid=(~0UL);
        status=ReadFileChunk(file_info,&pid,sizeof(pid));
        status|=ReadFileChunk(file_info,&tid,sizeof(tid));
        if (close(file_info->file) == -1)
          (void) ThrowWizardException(exception,GetWizardModule(),FileError,
            "unable to close file `%s': %s",path,strerror(errno));
        file_info->file=(-1);
        if (status != WizardFalse)
          {
            WizardBooleanType
              active_process;

            if ((pid == (long) getpid()) && (tid == GetWizardThreadId()))
              {
                path=DestroyString(path);
                UnlockSemaphoreInfo(file_info->semaphore);
                return(WizardTrue);
              }
#if defined(__WINDOWS__)
            {
              HANDLE
                handle;

              handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD) pid);
              active_process=handle == (HANDLE) NULL ? WizardFalse : WizardTrue;
              if (handle != (HANDLE) NULL)
                CloseHandle(handle);
            }
#else
            active_process=kill((pid_t) pid,0) == -1 ? WizardFalse : WizardTrue;
#endif
            if (active_process == WizardFalse)
              {
                if (errno != ESRCH)
                  break;
                i--;
                if (remove(path) == -1)
                  (void) ThrowWizardException(exception,GetWizardModule(),
                    FileError,"unable to remove file `%s': %s",path,
                    strerror(errno));
                break;
              }
          }
        (void) sleep(1);
        continue;
      }
    pid=(long) getpid();
    status=WriteFileChunk(file_info,&pid,sizeof(pid));
    tid=GetWizardThreadId();
    status=WriteFileChunk(file_info,&tid,sizeof(tid));
    if (close(file_info->file) == -1)
      (void) ThrowWizardException(exception,GetWizardModule(),FileError,
        "unable to close file `%s': %s",path,strerror(errno));
    file_info->file=(-1);
    if (status == WizardFalse)
      break;
    i--;
  }
  path=DestroyString(path);
  UnlockSemaphoreInfo(file_info->semaphore);
  return(WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e F i l e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireFileInfo() opens the file for reading or writing and returns a
%  FileInfo structure.
%
%  The format of the AcquireFileInfo method is:
%
%      FileInfo *AcquireFileInfo(const char *path,const char *relative_path,
%        const FileMode mode,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o path: The file path.
%
%    o relative_path: The path relative to the default path.
%
%    o mode: The file I/O mode.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport FileInfo *AcquireFileInfo(const char *path,
  const char *relative_path,const FileMode mode,ExceptionInfo *exception)
{
  char
    *home;

  FileInfo
    *file_info;

  /*
    Acquire file info.
  */
  assert(relative_path != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",relative_path);
  assert(exception != (ExceptionInfo *) NULL);
  file_info=(FileInfo *) AcquireWizardMemory(sizeof(*file_info));
  if (file_info == (FileInfo *) NULL)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),FileError,
        "memory allocation failed `%s'",strerror(errno));
      return((FileInfo *) NULL);
    }
  (void) ResetWizardMemory(file_info,0,sizeof(*file_info));
  file_info->path=AcquireString((char *) NULL);
  file_info->file=(-1);
  file_info->semaphore=AllocateSemaphoreInfo();
  file_info->timestamp=time((time_t *) NULL);
  file_info->signature=WizardSignature;
  if (path != (char *) NULL)
    (void) CopyWizardString(file_info->path,path,MaxTextExtent);
  else
    {
      /*
        Default location for key ring, random state, etc.
      */
      *file_info->path='\0';
      home=GetEnvironmentValue("WIZARD_HOME");
      if (home != (char *) NULL)
        {
          (void) CopyWizardString(file_info->path,home,MaxTextExtent);
          (void) ConcatenateWizardString(file_info->path,DirectorySeparator,
            MaxTextExtent);
          home=(char *) RelinquishWizardMemory(home);
        }
      else
        {
          home=GetEnvironmentValue("HOME");
          if (home == (char *) NULL)
            home=GetEnvironmentValue("USERPROFILE");
          if (home != (char *) NULL)
            {
              (void) CopyWizardString(file_info->path,home,MaxTextExtent);
              (void) ConcatenateWizardString(file_info->path,
                DirectorySeparator,MaxTextExtent);
              (void) ConcatenateWizardString(file_info->path,".wizard",
                MaxTextExtent);
#if defined(__WINDOWS__)
              (void) mkdir(file_info->path);
#else
              (void) mkdir(file_info->path,0700);
#endif
              (void) ConcatenateWizardString(file_info->path,
                DirectorySeparator,MaxTextExtent);
              home=(char *) RelinquishWizardMemory(home);
            }
        }
      (void) ConcatenateWizardString(file_info->path,relative_path,
        MaxTextExtent);
    }
  if (AcquireFileLock(file_info,exception) == WizardFalse)
    {
      file_info=DestroyFileInfo(file_info,exception);
      return((FileInfo *) NULL);
    }
  /*
    Open file.
  */
  switch (mode)
  {
    case ReadFileMode:
    {
      file_info->file=open(file_info->path,O_RDONLY | O_BINARY);
      break;
    }
    case WriteFileMode:
    {
      file_info->file=open(file_info->path,O_RDWR | O_CREAT | O_BINARY,
        S_MODE);
      break;
    }
    default:
    {
      (void) ThrowWizardException(exception,GetWizardModule(),FileError,
        "Invalid file mode `%s'",path);
      file_info=DestroyFileInfo(file_info,exception);
      return((FileInfo *) NULL);
    }
  }
  if (file_info->file < 0)
    {
      file_info=DestroyFileInfo(file_info,exception);
      return((FileInfo *) NULL);
    }
  (void) fstat(file_info->file,&file_info->properties);
  return(file_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y F i l e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyFile() destroys the file on disk.
%
%  The format of the DestroyFile method is:
%
%      WizardBooleanType DestroyFile(FileInfo *file_info,
%        Exceptioninfo *exception)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType DestroyFile(FileInfo *file_info,
  ExceptionInfo *exception)
{
  if (file_info->file >= 0)
    if (close(file_info->file) == -1)
      {
        (void) ThrowWizardException(exception,GetWizardModule(),FileError,
          "unable to close file `%s': %s",file_info->path,strerror(errno));
        return(WizardFalse);
      }
  file_info->file=(-1);
  if (remove(file_info->path) == -1)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),FileError,
        "unable to close file `%s': %s",file_info->path,strerror(errno));
      return(WizardFalse);
    }
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y F i l e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyFileInfo() destroys the file info structure.
%
%  The format of the DestroyFileInfo method is:
%
%      FileInfo *DestroyFileInfo(FileInfo *file_info)
%
%  A description of each parameter follows:
%
%    o path: The file name.
%
*/
WizardExport FileInfo *DestroyFileInfo(FileInfo *file_info,
  ExceptionInfo *exception)
{
  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",file_info->path);
  LockSemaphoreInfo(file_info->semaphore);
  if (file_info->file >= 0)
    if (close(file_info->file) == -1)
      (void) ThrowWizardException(exception,GetWizardModule(),FileError,
        "unable to close file `%s': %s",file_info->path,strerror(errno));
  file_info->file=(-1);
  (void) RelinquishFileLock(file_info,exception);
  if (file_info->path != (char *) NULL)
    file_info->path=DestroyString(file_info->path);
  file_info->signature=(~WizardSignature);
  UnlockSemaphoreInfo(file_info->semaphore);
  DestroySemaphoreInfo(&file_info->semaphore);
  file_info=(FileInfo *) RelinquishWizardMemory(file_info);
  return(file_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t F i l e D e s c r i p t o r                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetFileDescriptor() returns the file descriptor.
%
%  The format of the GetFileDescriptor method is:
%
%      int GetFileDescriptor(const FileInfo *file_info)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
*/
WizardExport int GetFileDescriptor(const FileInfo *file_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  return(file_info->file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t F i l e P a t h                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetFilePath() returns the file path.
%
%  The format of the GetFilePath method is:
%
%      const const char *GetFilePath(const FileInfo *file_info)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
*/
WizardExport const char *GetFilePath(const FileInfo *file_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  return(file_info->path);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t F i l e P r o p e r t i e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetFileproperties() returns the file properties.
%
%  The format of the GetFileProperties method is:
%
%      const struct stat *GetFileProperties(const FileInfo *file_info)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
*/
WizardExport const struct stat *GetFileProperties(const FileInfo *file_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  return(&file_info->properties);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d F i l e B y t e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFileByte() returns the character read as an unsigned char cast to an
%  integer or EOF on end of file or error.
%
%  The format of the ReadFileByte method is:
%
%      int ReadFileByte(FileInfo *file_info)
%
%  A description of each parameter follows.
%
%    o file_info: the blob info.
%
*/
WizardExport int ReadFileByte(FileInfo *file_info)
{
  unsigned char
    buffer[1];

  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  if (ReadFileChunk(file_info,buffer,1) == WizardFalse)
    return(EOF);
  return((int) (*buffer));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F i l e C h u n k                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFileChunk() reliably reads a chunk of data from a file.  It returns
%  WizardTrue if all the data requested is read otherwise WizardFalse.
%
%  The format of the ReadFileChunk method is:
%
%      WizardBooleanType ReadFileChunk(FileInfo *file_info,const void *data,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
%    o data: The data.
%
%    o length: The data length in bytes.
%
*/
WizardExport WizardBooleanType ReadFileChunk(FileInfo *file_info,void *data,
  const size_t length)
{
  register ssize_t
    i;

  ssize_t
    count;

  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",file_info->path);
  for (i=0; i < (ssize_t) length; i+=count)
  {
    count=read(file_info->file,(unsigned char *) data+i,Min(length-i,(size_t)
      WizardMaxBufferExtent));
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  if (i < (ssize_t) length)
    return(WizardFalse);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d F i l e 1 6 B i t s                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFile16Bits() reads a 16-bit quantity in least-significant byte first
%  order and returns it.
%
%  The format of the ReadFile16Bits method is:
%
%      WizardBooleanType ReadFile16Bits(FileInfo *file_info,
%        unsigned short *value)
%
%  A description of each parameter follows.
%
%    o file_info: The image.
%
%    o value: The value.
%
*/
WizardExport WizardBooleanType ReadFile16Bits(FileInfo *file_info,
  unsigned short *value)
{
  WizardBooleanType
    status;

  unsigned char
    buffer[2];

  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  *buffer=(unsigned char) '\0';
  status=ReadFileChunk(file_info,buffer,sizeof(buffer));
  if (status == WizardFalse)
    return(status);
  *value=(unsigned short) (buffer[1] << 8);
  *value|=(unsigned short) buffer[0];
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d F i l e 3 2 B i t s                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFile32Bits() reads a 32-bit quantity in least-significant byte first
%  order and returns it.
%
%  The format of the ReadFile32Bits method is:
%
%      WizardBooleanType ReadFile32Bits(FileInfo *file_info,
%        unsigned long *value)
%
%  A description of each parameter follows.
%
%    o file_info: The image.
%
%    o value: The value.
%
*/
WizardExport WizardBooleanType ReadFile32Bits(FileInfo *file_info,
  unsigned long *value)
{
  WizardBooleanType
    status;

  unsigned char
    buffer[4];

  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  *buffer=(unsigned char) '\0';
  status=ReadFileChunk(file_info,buffer,sizeof(buffer));
  if (status == WizardFalse)
    return(status);
  *value=((unsigned long) buffer[3]) << 24;
  *value|=((unsigned long) buffer[2]) << 16;
  *value|=((unsigned long) buffer[1]) << 8;
  *value|=((unsigned long) buffer[0]);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d F i l e 6 4 B i t s                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFile64Bits() reads a 64-bit quantity in least-significant byte first
%  order and returns it.
%
%  The format of the ReadFile64Bits method is:
%
%      WizardBooleanType ReadFile64Bits(FileInfo *file_info,
%        WizardSizeType *value)
%
%  A description of each parameter follows.
%
%    o file_info: The image.
%
%    o value: The value.
%
*/
WizardExport WizardBooleanType ReadFile64Bits(FileInfo *file_info,
  WizardSizeType *value)
{
  WizardBooleanType
    status;

  unsigned char
    buffer[8];

  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  *buffer=(unsigned char) '\0';
  status=ReadFileChunk(file_info,buffer,sizeof(buffer));
  if (status == WizardFalse)
    return(status);
  *value=((WizardSizeType) buffer[7]) << 56;
  *value|=((WizardSizeType) buffer[6]) << 48;
  *value|=((WizardSizeType) buffer[5]) << 40;
  *value|=((WizardSizeType) buffer[4]) << 32;
  *value|=((WizardSizeType) buffer[3]) << 24;
  *value|=((WizardSizeType) buffer[2]) << 16;
  *value|=((WizardSizeType) buffer[1]) << 8;
  *value|=(WizardSizeType) buffer[0];
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h F i l e L o c k                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishFileLock() relinquishes a file lock.
%
%  The format of the RelinquishFileLock method is:
%
%      WizardBooleanType RelinquishFileLock(FileInfo *file_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType RelinquishFileLock(FileInfo *file_info,
  ExceptionInfo *exception)
{
  char
    *path;

  path=AcquireString(file_info->path);
  AppendFileExtension("lck",path);
  if (remove(path) == -1)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),FileError,
        "unable to remove file `%s': %s",path,strerror(errno));
      path=DestroyString(path);
      return(WizardFalse);
    }
  path=DestroyString(path);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F i l e C h u n k                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFileChunk() reliably writes data to a file.
%
%  The format of the WriteFileChunk method is:
%
%      WizardBooleanType WriteFileChunk(FileInfo *file_info,const void *data,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o file_info: The file info.
%
%    o data: The data.
%
%    o lenth: The data length in bytes.
%
*/
WizardExport WizardBooleanType WriteFileChunk(FileInfo *file_info,
  const void *data,const size_t length)
{
  register ssize_t
    i;

  ssize_t
    count;

  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",file_info->path);
  for (i=0; i < (ssize_t) length; i+=count)
  {
    count=write(file_info->file,(unsigned char *) data+i,length-i);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  if (i < (ssize_t) length)
    return(WizardFalse);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e F i l e 1 6 B i t s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFile16Bits() writes a long value as a 16-bit quantity in
%  least-significant byte first order.
%
%  The format of the WriteFile16Bits method is:
%
%      WizardBooleanType WriteFile16Bits(FileInfo *file_info,
%        const unsigned short value)
%
%  A description of each parameter follows.
%
%    o value:  Specifies the value to write.
%
%    o file_info: The image.
%
*/
WizardExport WizardBooleanType WriteFile16Bits(FileInfo *file_info,
  const unsigned short value)
{
  unsigned char
    buffer[2];

  assert(file_info != (FileInfo *) NULL);
  assert(file_info->signature == WizardSignature);
  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  return(WriteFileChunk(file_info,buffer,sizeof(buffer)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e F i l e 3 2 B i t s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFile32Bits() writes a long value as a 32-bit quantity in
%  least-significant byte first order.
%
%  The format of the WriteFile32Bits method is:
%
%      WizardBooleanType WriteFile32Bits(FileInfo *file_info,
%        const unsigned long value)
%
%  A description of each parameter follows.
%
%    o value:  Specifies the value to write.
%
%    o file_info: The image.
%
*/
WizardExport WizardBooleanType WriteFile32Bits(FileInfo *file_info,
  const unsigned long value)
{
  unsigned char
    buffer[4];

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(file_info != (FileInfo *) NULL);
  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  buffer[2]=(unsigned char) (value >> 16);
  buffer[3]=(unsigned char) (value >> 24);
  return(WriteFileChunk(file_info,buffer,sizeof(buffer)));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e F i l e 6 4 B i t s                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFile64Bits() writes a long value as a 64-bit quantity in
%  least-significant byte first order.
%
%  The format of the WriteFile64Bits method is:
%
%      WizardBooleanType WriteFile64Bits(FileInfo *file_info,
%        const WizardSizeType value)
%
%  A description of each parameter follows.
%
%    o value:  Specifies the value to write.
%
%    o file_info: The image.
%
*/
WizardExport WizardBooleanType WriteFile64Bits(FileInfo *file_info,
  const WizardSizeType value)
{
  unsigned char
    buffer[8];

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(file_info != (FileInfo *) NULL);
  buffer[0]=(unsigned char) value;
  buffer[1]=(unsigned char) (value >> 8);
  buffer[2]=(unsigned char) (value >> 16);
  buffer[3]=(unsigned char) (value >> 24);
  buffer[4]=(unsigned char) (value >> 32);
  buffer[5]=(unsigned char) (value >> 40);
  buffer[6]=(unsigned char) (value >> 48);
  buffer[7]=(unsigned char) (value >> 56);
  return(WriteFileChunk(file_info,buffer,sizeof(buffer)));
}
