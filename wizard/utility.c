/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%             U   U  TTTTT  IIIII  L      IIIII  TTTTT  Y   Y                 %
%             U   U    T      I    L        I      T     Y Y                  %
%             U   U    T      I    L        I      T      Y                   %
%             U   U    T      I    L        I      T      Y                   %
%              UUU     T    IIIII  LLLLL  IIIII    T      Y                   %
%                                                                             %
%                                                                             %
%                     Wizard's Toolkit Utility Methods                        %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March 2003                                    %
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
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/locale_.h"
#include "wizard/memory_.h"
#include "wizard/resource_.h"
#include "wizard/string-private.h"
#include "wizard/utility.h"
#include "wizard/utility-private.h"
#if defined(WIZARDSTOOLKIT_HAVE_MACH_O_DYLD_H)
#include <mach-o/dyld.h>
#endif

/*
  Static declarations.
*/
static const char
  Base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
  Forward declaration.
*/
static int
  IsDirectory(const char *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A p p e n d F i l e E x t e n s i o n                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AppendFileExtension() appends a file extension type to the filename.  If an
%  extension to the file already exists, it is first removed.
%
%  The extension of the AppendFileExtension method is:
%
%      void AppendFileExtension(const char *extension,char *filename)
%
%  A description of each parameter follows.
%
%   o  extension:  Specifies a pointer to an array of characters.  This is the
%      extension of the image.
%
%   o  filename:  Specifies a pointer to an array of characters.  The unique
%      file name is returned in this array.
%
*/
WizardExport void AppendFileExtension(const char *extension,char *filename)
{
  char
    root[WizardPathExtent];

  assert(extension != (char *) NULL);
  assert(filename != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",filename);
  if ((*extension == '\0') || (*filename == '\0'))
    return;
  GetPathComponent(filename,RootPath,root);
  (void) CopyWizardString(filename,root,WizardPathExtent);
  (void) ConcatenateWizardString(filename,".",WizardPathExtent);
  (void) ConcatenateWizardString(filename,extension,WizardPathExtent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B a s e 6 4 D e c o d e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Base64Decode() decodes Base64-encoded text and returns its binary
%  equivalent.  NULL is returned if the text is not valid Base64 data, or a
%  memory allocation failure occurs.
%
%  The format of the Base64Decode method is:
%
%      unsigned char *Base64Decode(const char *source,length_t *length)
%
%  A description of each parameter follows:
%
%    o source:  A pointer to a Base64-encoded string.
%
%    o length: The number of bytes decoded.
%
*/
WizardExport unsigned char *Base64Decode(const char *source,size_t *length)
{
  int
    state;

   const char
    *p,
    *q;

   size_t
    i;

  unsigned char
    *decode;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(source != (char *) NULL);
  assert(length != (size_t *) NULL);
  *length=0;
  decode=(unsigned char *) AcquireQuantumMemory((strlen(source)+3)/4,
    3*sizeof(*decode));
  if (decode == (unsigned char *) NULL)
    return((unsigned char *) NULL);
  i=0;
  state=0;
  for (p=source; *p != '\0'; p++)
  {
    if (isspace((int) ((unsigned char) *p)) != 0)
      continue;
    if (*p == '=')
      break;
    q=strchr(Base64,*p);
    if (q == (char *) NULL)
      {
        decode=(unsigned char *) RelinquishWizardMemory(decode);
        return((unsigned char *) NULL);  /* non-Base64 character */
      }
    switch (state)
    {
      case 0:
      {
        decode[i]=(q-Base64) << 2;
        state++;
        break;
      }
      case 1:
      {
        decode[i++]|=(q-Base64) >> 4;
        decode[i]=((q-Base64) & 0x0f) << 4;
        state++;
        break;
      }
      case 2:
      {
        decode[i++]|=(q-Base64) >> 2;
        decode[i]=((q-Base64) & 0x03) << 6;
        state++;
        break;
      }
      case 3:
      {
        decode[i++]|=(q-Base64);
        state=0;
        break;
      }
    }
  }
  /*
    Verify Base-64 string has proper terminal characters.
  */
  if (*p != '=')
    {
      if (state != 0)
        {
          decode=(unsigned char *) RelinquishWizardMemory(decode);
          return((unsigned char *) NULL);
        }
    }
  else
    {
      p++;
      switch (state)
      {
        case 0:
        case 1:
        {
          /*
            Unrecognized '=' character.
          */
          decode=(unsigned char *) RelinquishWizardMemory(decode);
          return((unsigned char *) NULL);
        }
        case 2:
        {
          for ( ; *p != '\0'; p++)
            if (isspace((int) ((unsigned char) *p)) == 0)
              break;
          if (*p != '=')
            {
              decode=(unsigned char *) RelinquishWizardMemory(decode);
              return((unsigned char *) NULL);
            }
          p++;
        }
        case 3:
        {
          for ( ; *p != '\0'; p++)
            if (isspace((int) ((unsigned char) *p)) == 0)
              {
                decode=(unsigned char *) RelinquishWizardMemory(decode);
                return((unsigned char *) NULL);
              }
          if ((int) decode[i] != 0)
            {
              decode=(unsigned char *) RelinquishWizardMemory(decode);
              return((unsigned char *) NULL);
            }
        }
        default:
          break;
      }
    }
  *length=i;
  return(decode);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B a s e 6 4 E n c o d e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Base64Encode() encodes arbitrary binary data to Base64 encoded format as
%  described by the "Base64 Content-Transfer-Encoding" section of RFC 2045 and
%  returns the result as a null-terminated ASCII string.  NULL is returned if
%  a memory allocation failure occurs.
%
%  The format of the Base64Encode method is:
%
%      char *Base64Encode(const unsigned char *blob,const size_t blob_length,
%        size_t *encode_length)
%
%  A description of each parameter follows:
%
%    o blob:  A pointer to binary data to encode.
%
%    o blob_length: The number of bytes to encode.
%
%    o encode_length:  The number of bytes encoded.
%
*/
WizardExport char *Base64Encode(const unsigned char *blob,
  const size_t blob_length,size_t *encode_length)
{
  char
    *encode;

   const unsigned char
    *p;

   size_t
    i;

  size_t
    quantum,
    remainder;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(blob != (const unsigned char *) NULL);
  assert(blob_length != 0);
  assert(encode_length != (size_t *) NULL);
  *encode_length=0;
  quantum=4*blob_length/3+4;
  encode=(char *) AcquireQuantumMemory(quantum,sizeof(*encode));
  if (encode == (char *) NULL)
    return((char *) NULL);
  i=0;
  for (p=blob; p < (blob+blob_length-2); p+=3)
  {
    encode[i++]=Base64[(int) (*p >> 2)];
    encode[i++]=Base64[(int) (((*p & 0x03) << 4)+(*(p+1) >> 4))];
    encode[i++]=Base64[(int) (((*(p+1) & 0x0f) << 2)+(*(p+2) >> 6))];
    encode[i++]=Base64[(int) (*(p+2) & 0x3f)];
  }
  remainder=blob_length % 3;
  if (remainder != 0)
    {
      ssize_t
        j;

      unsigned char
        code[3];

      code[0]='\0';
      code[1]='\0';
      code[2]='\0';
      for (j=0; j < (ssize_t) remainder; j++)
        code[j]=(*p++);
      encode[i++]=Base64[(int) (code[0] >> 2)];
      encode[i++]=Base64[(int) (((code[0] & 0x03) << 4)+(code[1] >> 4))];
      if (remainder == 1)
        encode[i++]='=';
      else
        encode[i++]=Base64[(int) (((code[1] & 0x0f) << 2)+(code[2] >> 6))];
      encode[i++]='=';
    }
  *encode_length=i;
  encode[i++]='\0';
  assert(i <= quantum);
  return(encode);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C h o p P a t h C o m p o n e n t s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ChopPathComponents() removes the number of specified file components from a
%  path.
%
%  The format of the ChopPathComponents method is:
%
%      ChopPathComponents(char *path,size_t components)
%
%  A description of each parameter follows:
%
%    o path:  The path.
%
%    o components:  The number of components to chop.
%
*/
WizardExport void ChopPathComponents(char *path,const size_t components)
{
   ssize_t
    i;

  for (i=0; i < (ssize_t) components; i++)
    GetPathComponent(path,HeadPath,path);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t P a t h C o m p o n e n t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPathComponent() returns the parent directory name, filename, basename,
%  or extension of a file path.
%
%  The format of the GetPathComponent function is:
%
%      GetPathComponent(const char *path,PathType type,char *component)
%
%  A description of each parameter follows:
%
%    o path: Specifies a pointer to a character array that contains the
%      file path.
%
%    o type: Specififies which file path component to return.
%
%    o component: The selected file path component is returned here.
%
*/
WizardExport void GetPathComponent(const char *path,PathType type,
  char *component)
{
  char
    filesystem[WizardPathExtent],
    subnode[WizardPathExtent],
    *q;

   char
    *p;

  /*
    Get basename of client.
  */
  assert(path != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",path);
  assert(component != (const char *) NULL);
  if (*path == '\0')
    {
      *component='\0';
      return;
    }
  (void) CopyWizardString(component,path,WizardPathExtent);
  *filesystem='\0';
  for (p=component; *p != '\0'; p++)
    if ((*p == ':') && (IsDirectory(path) < 0) &&
        (IsPathAcessible(path) == WizardFalse))
      {     
        /*
          Look for filesystem specification (e.g. c:/documents).
        */
        (void) CopyWizardString(filesystem,component,(size_t)
          (p-component+1));
        if (IsDirectory(filesystem) < 0)
          *filesystem='\0';
        else         
          for (q=component; *q != '\0'; q++)
            *q=(*++p);
        break;
      }
  *subnode='\0';
  p=component;
  if (*p != '\0')
    p=component+strlen(component)-1;
  if ((*p == ']') && (strchr(component,'[') != (char *) NULL))
    {
      /*
        Look for subnode specification (e.g. book[2]).
      */
      for (q=p-1; q > component; q--)
        if (*q == '[')
          break;
      if (*q == '[')
        {
          (void) CopyWizardString(subnode,q+1,WizardPathExtent);
          subnode[p-q-1]='\0';
          *q='\0';
        }
    }
  p=component;
  if (*p != '\0')
    for (p=component+(strlen(component)-1); p > component; p--)
      if (IsBasenameSeparator(*p))
        break;
  switch (type)
  {
    case FilesystemPath:
    {
      (void) CopyWizardString(component,filesystem,WizardPathExtent);
      break;
    }
    case RootPath:
    {
      for (p=component+(strlen(component)-1); p > component; p--)
      {
        if (IsBasenameSeparator(*p) != WizardFalse)
          break;
        if (*p == '.')
          break;
      }
      if (*p == '.')
        *p='\0';
      break;
    }
    case HeadPath:
    {
      *p='\0';
      break;
    }
    case TailPath:
    {
      if (IsBasenameSeparator(*p) != WizardFalse)
        (void) CopyWizardMemory((unsigned char *) component,
          (const unsigned char *) (p+1),strlen(p+1)+1);
      break;
    }
    case BasePath:
    {
      if (IsBasenameSeparator(*p) != WizardFalse)
        (void) CopyWizardString(component,p+1,WizardPathExtent);
      for (p=component+(strlen(component)-1); p > component; p--)
        if (*p == '.')
          {
            *p='\0';
            break;
          }
      break;
    }
    case ExtensionPath:
    {
      if (IsBasenameSeparator(*p) != WizardFalse)
        (void) CopyWizardString(component,p+1,WizardPathExtent);
      p=component;
      if (*p != '\0')
        for (p=component+strlen(component)-1; p > component; p--)
          if (*p == '.')
            break;
      *component='\0';
      if (*p == '.')
        (void) CopyWizardString(component,p+1,WizardPathExtent);
      p=strchr(component,'?');
      if (p != (char *) NULL)
        *p='\0';
      break;
    }
    case SubnodePath:
    {
      (void) CopyWizardString(component,subnode,WizardPathExtent);
      break;
    }
    case CanonicalPath:
    case UndefinedPath:
      break;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t P a t h C o m p o n e n t s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPathComponents() returns a list of path components.
%
%  The format of the GetPathComponents method is:
%
%      char **GetPathComponents(const char *path,
%        size_t *number_componenets)
%
%  A description of each parameter follows:
%
%    o path:  Specifies the string to segment into a list.
%
%    o number_components:  return the number of components in the list
%
*/
WizardExport char **GetPathComponents(const char *path,
  size_t *number_components)
{
  char
    **components;

   char
    *q;

   const char
    *p;

   ssize_t
    i;

  if (path == (char *) NULL)
    return((char **) NULL);
  *number_components=1;
  for (p=path; *p != '\0'; p++)
    if (IsBasenameSeparator(*p))
      (*number_components)++;
  components=(char **) AcquireQuantumMemory((size_t) (*number_components+1),
    sizeof(*components));
  if (components == (char **) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  p=path;
  for (i=0; i < (ssize_t) *number_components; i++)
  {
    for (q=(char *) p; *q != '\0'; q++)
      if (IsBasenameSeparator(*q))
        break;
    components[i]=(char *) AcquireQuantumMemory((size_t) (q-p)+WizardPathExtent,
      sizeof(**components));
    if (components[i] == (char *) NULL)
      ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
    (void) CopyWizardString(components[i],p,(size_t) (q-p+1));
    p=q+1;
  }
  components[i]=(char *) NULL;
  return(components);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E x e c u t i o n P a t h                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetExecutionPath() returns the pathname of the executable that started
%  the process.  On success WizardTrue is returned, otherwise WizardFalse.
%
%  The format of the GetExecutionPath method is:
%
%      WizardBooleanType GetExecutionPath(char *path,const size_t extent)
%
%  A description of each parameter follows:
%
%    o path: The pathname of the executable that started the process.
%
%    o extent: the maximum extent of the path.
%
*/
WizardExport WizardBooleanType GetExecutionPath(char *path,const size_t extent)
{
  char
    *directory;

  *path='\0';
  directory=getcwd(path,(unsigned long) extent);
  (void) directory;
#if defined(WIZARDSTOOLKIT_HAVE_GETPID) && defined(WIZARDSTOOLKIT_HAVE_READLINK) && defined(PATH_MAX)
  {
    char
      link_path[WizardPathExtent],
      real_path[PATH_MAX+1];

    int
      length;

    (void) FormatLocaleString(link_path,WizardPathExtent,"/proc/%.20g/exe",
      (double) getpid());
    length=readlink(link_path,real_path,PATH_MAX);
    if (length == -1)
      {
        (void) FormatLocaleString(link_path,WizardPathExtent,"/proc/%.20g/file",
          (double) getpid());
        length=readlink(link_path,real_path,PATH_MAX);
      }
    if ((length > 0) && ((size_t) length <= PATH_MAX))
      {
        real_path[length]='\0';
        (void) CopyWizardString(path,real_path,extent);
      }
  }
#endif
#if defined(WIZARDSTOOLKIT_HAVE__NSGETEXECUTABLEPATH)
  {

    char
      executable_path[PATH_MAX << 1],
      real_path[PATH_MAX+1];
   
    size_t
      length;

    length=sizeof(executable_path);
    if ((_NSGetExecutablePath(executable_path,&length) == 0) &&
        (realpath(executable_path,real_path) != (char *) NULL))
      (void) CopyWizardString(path,real_path,extent);
  }
#endif
#if defined(WIZARDSTOOLKIT_HAVE_GETEXECNAME)
  {
    const char
      *execution_path;

    execution_path=(const char *) getexecname();
    if (execution_path != (const char *) NULL)
      {
        if (*execution_path != *DirectorySeparator)
          (void) ConcatenateWizardString(path,DirectorySeparator,extent);
        (void) ConcatenateWizardString(path,execution_path,extent);
      }
  }
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  NTGetExecutionPath(path,extent);
#endif
#if defined(__GNU__)
  {
    char
      *program_name,
      *execution_path;

    ssize_t
      count;

    count=0;
    execution_path=(char *) NULL;
    program_name=program_invocation_name;
    if (*program_invocation_name != '/')
      {
        size_t
          extent;

        extent=strlen(directory)+strlen(program_name)+2;
        program_name=AcquireQuantumMemory(extent,sizeof(*program_name));
        if (program_name == (char *) NULL)
          program_name=program_invocation_name;
        else
          count=FormatLocaleString(program_name,extent,"%s/%s",directory,
            program_invocation_name);
      }
    if (count != -1)
      {
        execution_path=realpath(program_name,NULL);
        if (execution_path != (char *) NULL)
          (void) CopyWizardString(path,execution_path,extent);
      }
    if (program_name != program_invocation_name)
      program_name=(char *) RelinquishWizardMemory(program_name);
    execution_path=(char *) RelinquishWizardMemory(execution_path);
  }
#endif
#if defined(__OpenBSD__)
  {
    extern char
      *__progname;

    (void) CopyMagickString(path,__progname,extent);
  }
#endif
  return(IsPathAcessible(path));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s P a t h A c c e s s i b l e                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPathAcessible() returns WizardTrue if the file as defined by the path is
%  accessible.
%
%  The format of the IsPathAcessible method is:
%
%      WizardBooleanType IsPathAcessible(const char *filename)
%
%  A description of each parameter follows.
%
%    o path:  Specifies a path to a file.
%
*/
WizardExport WizardBooleanType IsPathAcessible(const char *path)
{
  int
    status;

  struct stat
    file_info;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(WizardFalse);
  status=stat_utf8(path,&file_info);
  if (status != 0)
    return(WizardFalse);
  if (S_ISREG(file_info.st_mode) == 0)
    return(WizardFalse);
  if (access(path,F_OK) != 0)
    return(WizardFalse);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a d d P a g e S i z e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardPageSize() returns the memory page size.
%
%  The format of the GetWizardPageSize method is:
%
%      ssize_t GetWizardPageSize()
%
*/
WizardPrivate ssize_t GetWizardPageSize(void)
{
  static ssize_t
    page_size = -1;

  if (page_size > 0)
    return(page_size);
#if defined(WIZARDSTOOLKIT_HAVE_SYSCONF) && defined(_SC_PAGE_SIZE)
  page_size=(ssize_t) sysconf(_SC_PAGE_SIZE);
#else
#if defined(WIZARDSTOOLKIT_HAVE_GETPAGESIZE)
  page_size=(ssize_t) getpagesize();
#endif
#endif
  if (page_size <= 0)
    page_size=16384;
  return(page_size);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  I s D i r e c t o r y                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDirectory() returns -1 if the directory does not exist,  1 is returned
%  if the path represents a directory otherwise 0.
%
%  The format of the IsDirectory method is:
%
%      int IsDirectory(const char *path)
%
%  A description of each parameter follows.
%
%   o  path:  The directory path.
%
%
*/
static int IsDirectory(const char *path)
{
#if !defined(X_OK)
#define X_OK  1
#endif

  int
    status;

  struct stat
    file_info;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(WizardFalse);
  status=stat_utf8(path,&file_info);
  if (status != 0)
    return(-1);
  if (S_ISDIR(file_info.st_mode) == 0)
    return(0);
  if (access(path,X_OK) != 0)
    return(0);
  return(1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s W i z a r d T r u e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWizardTrue() returns WizardTrue if the value is "true", "on", "yes" or
%  "1".
%
%  The format of the IsWizardTrue method is:
%
%      WizardBooleanType IsWizardTrue(const char *value)
%
%  A description of each parameter follows:
%
%    o option: either WizardTrue or WizardFalse depending on the value
%      parameter.
%
%    o value: Specifies a pointer to a character array.
%
*/
WizardExport WizardBooleanType IsWizardTrue(const char *value)
{
  if (value == (char *) NULL)
    return(WizardFalse);
  if (LocaleCompare(value,"true") == 0)
    return(WizardTrue);
  if (LocaleCompare(value,"on") == 0)
    return(WizardTrue);
  if (LocaleCompare(value,"yes") == 0)
    return(WizardTrue);
  if (LocaleCompare(value,"1") == 0)
    return(WizardTrue);
  return(WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  P a r s e W i z a r d T i m e                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseWizardTime() parses the specified time in the Internet date/time
%  format and the returns the results.
%
%  The format of the ParseWizardTime method is:
%
%      const char *ParseWizardTime(const char *timestamp,time_t *target)
%
%  A description of each parameter follows.
%
%   o timestamp:  the Internet date/time here.
%
%   o target:  return the time since the Epoch (00:00:00 UTC, January 1, 1970),
%     measured in seconds.
%
*/
WizardExport const char *ParseWizardTime(const char *timestamp,time_t *target)
{
  char
    *q;

  double
    value;

  ssize_t
    timezone;

   const char
    *p;

   ssize_t
    i;

  static char
    separators[] = "--T::-:";

  struct tm
    gm_time,
    local_time,
    target_time;

  *target=time((time_t *) NULL);
#if defined(WIZARDSTOOLKIT_HAVE_LOCALTIME_R)
  (void) localtime_r(target,&local_time);
#else
  (void) memcpy(&local_time,localtime(target),sizeof(local_time));
#endif
#if defined(WIZARDSTOOLKIT_HAVE_GMTIME_R)
  (void) gmtime_r(target,&gm_time);
#else
  (void) memcpy(&gm_time,gmtime(target),sizeof(gm_time));
#endif
  timezone=(ssize_t) ((local_time.tm_min-gm_time.tm_min)/60+local_time.tm_hour-
    gm_time.tm_hour+24*((local_time.tm_year-gm_time.tm_year) != 0 ?
    (local_time.tm_year-gm_time.tm_year) : (local_time.tm_yday-
    gm_time.tm_yday)));
  (void) memset(&target_time,0,sizeof(target_time));
  p=timestamp;
  for (i=0; ; i++)
  {
    value=StringToDouble(p,&q);
    if (*q != separators[i])
      break;
    switch (i)
    {
      case 0: target_time.tm_year=(int) (value+0.5)-1900; break;
      case 1: target_time.tm_mon=(int) (value+0.5)-1; break;
      case 2: target_time.tm_mday=(int) (value+0.5); break;
      case 3: target_time.tm_hour=(int) (value+0.5); break;
      case 4: target_time.tm_min=(int) (value+0.5); break;
      case 5: target_time.tm_sec=(int) (value+0.5); break;
      case 6: timezone+=(int) (value+0.5); break;
      default: break;
    }
    p=q;
    if (*q == '\0')
      break;
    p++;
  }
  *target=mktime(&target_time)-3600*timezone;
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W i z a d d D e l a y                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WizardDelay() suspends program execution for the number of milliseconds
%  specified.
%
%  The format of the Delay method is:
%
%      void WizardDelay(const WizardSizeType milliseconds)
%
%  A description of each parameter follows:
%
%    o milliseconds: Specifies the number of milliseconds to delay before
%      returning.
%
*/
WizardExport void WizardDelay(const WizardSizeType milliseconds)
{
  if (milliseconds == 0)
    return;
#if defined(WIZARDSTOOLKIT_HAVE_NANOSLEEP)
  {
    struct timespec
      timer;

    timer.tv_sec=(time_t) (milliseconds/1000);
    timer.tv_nsec=(milliseconds % 1000)*1000*1000;
    (void) nanosleep(&timer,(struct timespec *) NULL);
  }
#elif defined(WIZARDSTOOLKIT_HAVE_USLEEP)
  usleep(1000*milliseconds);
#elif defined(WIZARDSTOOLKIT_HAVE_SELECT)
  {
    struct timeval
      timer;

    timer.tv_sec=(long) milliseconds/1000;
    timer.tv_usec=(long) (milliseconds % 1000)*1000;
    (void) select(0,(XFD_SET *) NULL,(XFD_SET *) NULL,(XFD_SET *) NULL,&timer);
  }
#elif defined(WIZARDSTOOLKIT_HAVE_POLL)
  (void) poll((struct pollfd *) NULL,0,(int) milliseconds);
#elif defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  Sleep((long) milliseconds);
#elif defined(vms)
  {
    float
      timer;

    timer=milliseconds/1000.0;
    lib$wait(&timer);
  }
#elif defined(__BEOS__)
  snooze(1000*milliseconds);
#else
# error "Time delay method not defined."
#endif
}
