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
%                               John Cristy                                   %
%                               March 2003                                    %
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
#include "wizard/exception-private.h"
#include "utility_.h"

/*
  Forward declaration.
*/
static char
   **ListFiles(const char *,const char *,size_t *);

static int
  IsDirectory(const char *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x p a n d F i l e n a m e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandFilename() expands '~' in a path.
%
%  The format of the ExpandFilename function is:
%
%      ExpandFilename(char *path)
%
%  A description of each parameter follows:
%
%    o path: Specifies a pointer to a character array that contains the
%      path.
%
*/
static void ExpandFilename(char *path)
{
  char
    expand_path[MaxTextExtent];

  if (path == (char *) NULL)
    return;
  if (*path != '~')
    return;
  (void) CopyWizardString(expand_path,path,MaxTextExtent);
  if ((*(path+1) == *DirectorySeparator) || (*(path+1) == '\0'))
    {
      char
        *home;

      /*
        Substitute ~ with $HOME.
      */
      (void) CopyWizardString(expand_path,".",MaxTextExtent);
      (void) ConcatenateWizardString(expand_path,path+1,MaxTextExtent);
      home=GetEnvironmentValue("HOME");
      if (home == (char *) NULL)
        home=GetEnvironmentValue("USERPROFILE");
      if (home != (char *) NULL)
        {
          (void) CopyWizardString(expand_path,home,MaxTextExtent);
          (void) ConcatenateWizardString(expand_path,path+1,MaxTextExtent);
          home=DestroyString(home);
        }
    }
  else
    {
#if defined(WIZARDSTOOLKIT_POSIX_SUPPORT)
      char
        username[MaxTextExtent];

      register char
        *p;

      struct passwd
        *entry;

      /*
        Substitute ~ with home directory from password file.
      */
      (void) CopyWizardString(username,path+1,MaxTextExtent);
      p=strchr(username,'/');
      if (p != (char *) NULL)
        *p='\0';
      entry=getpwnam(username);
      if (entry == (struct passwd *) NULL)
        return;
      (void) CopyWizardString(expand_path,entry->pw_dir,MaxTextExtent);
      if (p != (char *) NULL)
        {
          (void) ConcatenateWizardString(expand_path,"/",MaxTextExtent);
          (void) ConcatenateWizardString(expand_path,p+1,MaxTextExtent);
        }
#endif
    }
  (void) CopyWizardString(path,expand_path,MaxTextExtent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p a n d F i l e n a m e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandFilenames() checks each argument of the command line vector and
%  expands it if they have a wildcard character.  For example, *.jpg might
%  expand to:  bird.jpg rose.jpg tiki.jpg.
%
%  The format of the ExpandFilenames function is:
%
%      WizardBooleanType ExpandFilenames(int *argc,char ***argv)
%
%  A description of each parameter follows:
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
*/
WizardExport WizardBooleanType ExpandFilenames(int *argc,char ***argv)
{
  char
    **filelist,
    filename[MaxTextExtent],
    home_directory[MaxTextExtent],
    *option,
    path[MaxTextExtent],
    **vector;

  ssize_t
    count,
    parameters;

  register ssize_t
    i,
    j;

  size_t
    number_files;

  /*
    Allocate argument vector.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(argc != (int *) NULL);
  assert(argv != (char ***) NULL);
  vector=(char **) AcquireQuantumMemory(*argc+1UL,sizeof(*vector));
  if (vector == (char **) NULL)
    return(WizardFalse);
  /*
    Expand any wildcard filenames.
  */
  if (getcwd(home_directory,MaxTextExtent) == (char *) NULL)
    return(WizardFalse);
  count=0;
  for (i=0; i < (ssize_t) *argc; i++)
  {
    option=(*argv)[i];
    vector[count++]=ConstantString(option);
    parameters=ParseWizardOption(WizardCommandOptions,WizardFalse,option);
    if (parameters > 0)
      {
        /*
          Do not expand command option parameters.
        */
        for (j=0; j < parameters; j++)
        {
          i++;
          if (i == (ssize_t) *argc)
            break;
          option=(*argv)[i];
          vector[count++]=ConstantString(option);
        }
        continue;
      }
    if ((*option == '"') || (*option == '\''))
      continue;
    GetPathComponent(option,TailPath,filename);
    if (IsGlob(filename) == WizardFalse)
      continue;
    GetPathComponent(option,HeadPath,path);
    ExpandFilename(path);
    filelist=ListFiles(*path == '\0' ? home_directory : path,filename,
      &number_files);
    if (filelist == (char **) NULL)
      continue;
    for (j=0; j < (ssize_t) number_files; j++)
      if (IsDirectory(filelist[j]) <= 0)
        break;
    if (j == (ssize_t) number_files)
      {
        for (j=0; j < (ssize_t) number_files; j++)
          filelist[j]=DestroyString(filelist[j]);
        filelist=(char **) RelinquishWizardMemory(filelist);
        continue;
      }
    /*
      Transfer file list to argument vector.
    */
    vector=(char **) ResizeQuantumMemory(vector,(size_t) *argc+count+
      number_files+1,sizeof(*vector));
    if (vector == (char **) NULL)
      return(WizardFalse);
    count--;
    for (j=0; j < (ssize_t) number_files; j++)
    {
      (void) CopyWizardString(filename,path,MaxTextExtent);
      if (*path != '\0')
        (void) ConcatenateWizardString(filename,DirectorySeparator,
          MaxTextExtent);
      (void) ConcatenateWizardString(filename,filelist[j],MaxTextExtent);
      filelist[j]=DestroyString(filelist[j]);
      if (IsAccessible(filename) != WizardFalse)
        {
          char
            path[MaxTextExtent];

          *path='\0';
          (void) ConcatenateWizardString(path,filename,MaxTextExtent);
          vector[count++]=ConstantString(path);
        }
    }
    filelist=(char **) RelinquishWizardMemory(filelist);
  }
  vector[count]=(char *) NULL;
  if (IsEventLogging() != WizardFalse)
    {
      char
        *command_line;

      command_line=AcquireString(vector[0]);
      for (i=1; i < count; i++)
      {
        (void) ConcatenateString(&command_line," {");
        (void) ConcatenateString(&command_line,vector[i]);
        (void) ConcatenateString(&command_line,"}");
      }
      (void) LogWizardEvent(ConfigureEvent,GetWizardModule(),
        "Command line: %s",command_line);
      command_line=DestroyString(command_line);
    }
  *argc=(int) count;
  *argv=vector;
  return(WizardTrue);
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
  status=stat(path,&file_info);
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
+   L i s t F i l e s                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListFiles() reads the directory specified and returns a list of filenames
%  contained in the directory sorted in ascending alphabetic order.
%
%  The format of the ListFiles function is:
%
%      char **ListFiles(const char *directory,const char *pattern,
%        ssize_t *number_entries)
%
%  A description of each parameter follows:
%
%    o filelist: Method ListFiles returns a list of filenames contained
%      in the directory.  If the directory specified cannot be read or it is
%      a file a NULL list is returned.
%
%    o directory: Specifies a pointer to a text string containing a directory
%      name.
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_entries:  This integer returns the number of filenames in the
%      list.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int FileCompare(const void *x,const void *y)
{
  register const char
    **p,
    **q;

  p=(const char **) x;
  q=(const char **) y;
  return(LocaleCompare(*p,*q));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static inline int WizardReadDirectory(DIR *directory,struct dirent *entry,
  struct dirent **result)
{
#if defined(WIZARDSTOOLKIT_HAVE_READDIR_R)
  return(readdir_r(directory,entry,result));
#else
  (void) entry;
  errno=0;
  *result=readdir(directory);
  return(errno);
#endif
}

static char **ListFiles(const char *directory,const char *pattern,
  size_t *number_entries)
{
  char
    **filelist;

  DIR
    *current_directory;

  struct dirent
    *buffer,
    *entry;

  size_t
    max_entries;

  /*
    Open directory.
  */
  assert(directory != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",directory);
  assert(pattern != (const char *) NULL);
  assert(number_entries != (size_t *) NULL);
  *number_entries=0;
  current_directory=opendir(directory);
  if (current_directory == (DIR *) NULL)
    return((char **) NULL);
  /*
    Allocate filelist.
  */
  max_entries=2048;
  filelist=(char **) AcquireQuantumMemory((size_t) max_entries,
    sizeof(*filelist));
  if (filelist == (char **) NULL)
    {
      (void) closedir(current_directory);
      return((char **) NULL);
    }
  /*
    Save the current and change to the new directory.
  */
  buffer=(struct dirent *) AcquireAlignedMemory(1,sizeof(*buffer)+FILENAME_MAX+
    MaxTextExtent);
  if (buffer == (struct dirent *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  while ((WizardReadDirectory(current_directory,buffer,&entry) == 0) &&
         (entry != (struct dirent *) NULL))
  {
    if (*entry->d_name == '.')
      continue;
    if ((IsDirectory(entry->d_name) > 0) ||
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
        (GlobExpression(entry->d_name,pattern,WizardTrue) != WizardFalse))
#else
        (GlobExpression(entry->d_name,pattern,WizardFalse) != WizardFalse))
#endif
      {
        if (*number_entries >= max_entries)
          {
            /*
              Extend the file list.
            */
            max_entries<<=1;
            filelist=(char **) ResizeQuantumMemory(filelist,(size_t)
              max_entries,sizeof(*filelist));
            if (filelist == (char **) NULL)
              break;
          }
#if defined(vms)
        {
          register char
            *p;

          p=strchr(entry->d_name,';');
          if (p)
            *p='\0';
          if (*number_entries > 0)
            if (LocaleCompare(entry->d_name,filelist[*number_entries-1]) == 0)
              continue;
        }
#endif
        filelist[*number_entries]=(char *) AcquireString(entry->d_name);
        if (IsDirectory(entry->d_name) > 0)
          (void) ConcatenateWizardString(filelist[*number_entries],
            DirectorySeparator,MaxTextExtent);
        (*number_entries)++;
      }
  }
  buffer=(struct dirent *) RelinquishWizardMemory(buffer);
  (void) closedir(current_directory);
  if (filelist == (char **) NULL)
    return((char **) NULL);
  /*
    Sort filelist in ascending order.
  */
  qsort((void *) filelist,(size_t) *number_entries,sizeof(*filelist),
    FileCompare);
  return(filelist);
}
