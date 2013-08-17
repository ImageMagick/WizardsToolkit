/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             L       OOO    GGGG                             %
%                             L      O   O  G                                 %
%                             L      O   O  G GG                              %
%                             L      O   O  G   G                             %
%                             LLLLL   OOO    GGG                              %
%                                                                             %
%                                                                             %
%                           Log ImageMagick Events                            %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                September 2002                               %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/option.h"
#include "wizard/semaphore.h"
#include "wizard/string_.h"
#include "wizard/string-private.h"
#include "wizard/thread_.h"
#include "wizard/thread-private.h"
#include "wizard/timer.h"
#include "wizard/token.h"
#include "wizard/utility.h"
#include "wizard/utility-private.h"
#include "wizard/version.h"
#include "wizard/xml-tree.h"

/*
  Define declarations.
*/
#define LogFilename  "log.xml"

/*
  Typedef declarations.
*/
typedef enum
{
  UndefinedHandler = 0x0000,
  NoHandler = 0x0000,
  ConsoleHandler = 0x0001,
  StdoutHandler = 0x0002,
  StderrHandler = 0x0004,
  FileHandler = 0x0008,
  DebugHandler = 0x0010,
  EventHandler = 0x0020
} LogHandlerType;

typedef struct _EventInfo
{
  char
    *name;

  LogEventType
    event;
} EventInfo;

typedef struct _HandlerInfo
{
  const char
    *name;

  LogHandlerType
    handler;
} HandlerInfo;

struct _LogInfo
{
  LogEventType
    event_mask;

  LogHandlerType
    handler_mask;

  char
    *path,
    *name,
    *filename,
    *format;

  size_t
    generations,
    limit;

  FILE
    *file;

  size_t
    generation;

  WizardBooleanType
    append,
    stealth;

  TimerInfo
    *timer;

  size_t
    signature;
};

/*
  Declare log map.
*/
static const HandlerInfo
  LogHandlers[] =
  {
    { "console", ConsoleHandler },
    { "debug", DebugHandler },
    { "event", EventHandler },
    { "file", FileHandler },
    { "none", NoHandler },
    { "stderr", StderrHandler },
    { "stdout", StdoutHandler },
    { (char *) NULL, UndefinedHandler }
  };

typedef struct _LogMapInfo
{
  const LogEventType
    event_mask;

  const LogHandlerType
    handler_mask;

  const char
    *filename,
    *format;
} LogMapInfo;

/*
  Static declarations.
*/
static const LogMapInfo
  LogMap[] =
  {
    { NoEvents, ConsoleHandler, "Magick-%g.log",
      "%t %r %u %v %d %c[%p]: %m/%f/%l/%d\\n  %e" }
  };

static char
  log_name[MaxTextExtent] = "Wizard";

static LinkedListInfo
  *log_list = (LinkedListInfo *) NULL;

static SemaphoreInfo
  *log_semaphore = (SemaphoreInfo *) NULL;

static volatile WizardBooleanType
  instantiate_log = WizardFalse;

/*
  Forward declarations.
*/
static LogHandlerType
  ParseLogHandlers(const char *);

static WizardBooleanType
  InitializeLogList(ExceptionInfo *),
  LoadLogLists(const char *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o s e W i z a r d L o g                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloseWizardLog() closes the Wizard log.
%
%  The format of the CloseWizardLog method is:
%
%      CloseWizardLog(void)
%
*/
WizardExport void CloseWizardLog(void)
{
  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  if (IsEventLogging() == WizardFalse)
    return;
  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  LockSemaphoreInfo(log_semaphore);
  if (log_info->file != (FILE *) NULL)
    {
      (void) fprintf(log_info->file,"</log>\n");
      (void) fclose(log_info->file);
      log_info->file=(FILE *) NULL;
    }
  UnlockSemaphoreInfo(log_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t L o g I n f o                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogInfo() searches the log list for the specified name and if found
%  returns attributes for that log.
%
%  The format of the GetLogInfo method is:
%
%      const LogInfo *GetLogInfo(const char *name,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o name: The log name.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport const LogInfo *GetLogInfo(const char *name,
  ExceptionInfo *exception)
{
  register const LogInfo
    *p;

  assert(exception != (ExceptionInfo *) NULL);
  if ((log_list == (LinkedListInfo *) NULL) || (instantiate_log == WizardFalse))
    if (InitializeLogList(exception) == WizardFalse)
      return((const LogInfo *) NULL);
  if ((log_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(log_list) != WizardFalse))
    return((const LogInfo *) NULL);
  if ((name == (const char *) NULL) || (LocaleCompare(name,"*") == 0))
    return((const LogInfo *) GetValueFromLinkedList(log_list,0));
  /*
    Search for named log.
  */
  LockSemaphoreInfo(log_semaphore);
  ResetLinkedListIterator(log_list);
  p=(const LogInfo *) GetNextValueInLinkedList(log_list);
  while (p != (const LogInfo *) NULL)
  {
    if (LocaleCompare(name,p->name) == 0)
      break;
    p=(const LogInfo *) GetNextValueInLinkedList(log_list);
  }
  if (p == (LogInfo *) NULL)
    (void) ThrowWizardException(exception,GetWizardModule(),OptionWarning,
      "no such element `%s'",name);
  else
    (void) InsertValueInLinkedList(log_list,0,
      RemoveElementByValueFromLinkedList(log_list,p));
  UnlockSemaphoreInfo(log_semaphore);
  return(p);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o g I n f o L i s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogInfoList() returns any logs that match the specified pattern.
%
%  The format of the GetLogInfoList function is:
%
%      const LogInfo **GetLogInfoList(const char *pattern,
%        size_t *number_preferences,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_preferences:  This integer returns the number of logs in the list.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LogInfoCompare(const void *x,const void *y)
{
  const LogInfo
    **p,
    **q;

  p=(const LogInfo **) x,
  q=(const LogInfo **) y;
  if (LocaleCompare((*p)->path,(*q)->path) == 0)
    return(LocaleCompare((*p)->name,(*q)->name));
  return(LocaleCompare((*p)->path,(*q)->path));
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

WizardExport const LogInfo **GetLogInfoList(const char *pattern,
  size_t *number_preferences,ExceptionInfo *exception)
{
  const LogInfo
    **preferences;

  register const LogInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate log list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_preferences != (size_t *) NULL);
  *number_preferences=0;
  p=GetLogInfo("*",exception);
  if (p == (const LogInfo *) NULL)
    return((const LogInfo **) NULL);
  preferences=(const LogInfo **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(log_list)+1UL,sizeof(*preferences));
  if (preferences == (const LogInfo **) NULL)
    return((const LogInfo **) NULL);
  /*
    Generate log list.
  */
  LockSemaphoreInfo(log_semaphore);
  ResetLinkedListIterator(log_list);
  p=(const LogInfo *) GetNextValueInLinkedList(log_list);
  for (i=0; p != (const LogInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->name,pattern,WizardFalse) != WizardFalse))
      preferences[i++]=p;
    p=(const LogInfo *) GetNextValueInLinkedList(log_list);
  }
  UnlockSemaphoreInfo(log_semaphore);
  qsort((void *) preferences,(size_t) i,sizeof(*preferences),LogInfoCompare);
  preferences[i]=(LogInfo *) NULL;
  *number_preferences=(size_t) i;
  return(preferences);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o g L i s t                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogList() returns any logs that match the specified pattern.
%
%  The format of the GetLogList function is:
%
%      char **GetLogList(const char *pattern,size_t *number_preferences,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_preferences:  This integer returns the number of logs in the list.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int LogCompare(const void *x,const void *y)
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

WizardExport char **GetLogList(const char *pattern,
  size_t *number_preferences,ExceptionInfo *exception)
{
  char
    **preferences;

  register const LogInfo
    *p;

  register ssize_t
    i;

  /*
    Allocate log list.
  */
  assert(pattern != (char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",pattern);
  assert(number_preferences != (size_t *) NULL);
  *number_preferences=0;
  p=GetLogInfo("*",exception);
  if (p == (const LogInfo *) NULL)
    return((char **) NULL);
  preferences=(char **) AcquireQuantumMemory((size_t)
    GetNumberOfElementsInLinkedList(log_list)+1UL,sizeof(*preferences));
  if (preferences == (char **) NULL)
    return((char **) NULL);
  /*
    Generate log list.
  */
  LockSemaphoreInfo(log_semaphore);
  ResetLinkedListIterator(log_list);
  p=(const LogInfo *) GetNextValueInLinkedList(log_list);
  for (i=0; p != (const LogInfo *) NULL; )
  {
    if ((p->stealth == WizardFalse) &&
        (GlobExpression(p->name,pattern,WizardFalse) != WizardFalse))
      preferences[i++]=ConstantString(p->name);
    p=(const LogInfo *) GetNextValueInLinkedList(log_list);
  }
  UnlockSemaphoreInfo(log_semaphore);
  qsort((void *) preferences,(size_t) i,sizeof(*preferences),LogCompare);
  preferences[i]=(char *) NULL;
  *number_preferences=(size_t) i;
  return(preferences);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L o g N a m e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLogName() returns the current log name.
%
%  The format of the GetLogName method is:
%
%      const char *GetLogName(void)
%
*/
WizardExport const char *GetLogName(void)
{
  return(log_name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n i t i a l i z e L o g L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InitializeLogList() initialize the log list.
%
%  The format of the InitializeLogList method is:
%
%      WizardBooleanType InitializeLogList(ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType InitializeLogList(ExceptionInfo *exception)
{
  if ((log_list == (LinkedListInfo *) NULL) && (instantiate_log == WizardFalse))
    {
      if (log_semaphore == (SemaphoreInfo *) NULL)
        AcquireSemaphoreInfo(&log_semaphore);
      LockSemaphoreInfo(log_semaphore);
      if ((log_list == (LinkedListInfo *) NULL) &&
          (instantiate_log == WizardFalse))
        {
          (void) LoadLogLists(LogFilename,exception);
          instantiate_log=WizardTrue;
        }
      UnlockSemaphoreInfo(log_semaphore);
    }
  return(log_list != (LinkedListInfo *) NULL ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s E v e n t L o g g i n g                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsEventLogging() returns WizardTrue if debug of events is enabled otherwise
%  WizardFalse.
%
%  The format of the IsEventLogging method is:
%
%      WizardBooleanType IsEventLogging(void)
%
*/
WizardExport WizardBooleanType IsEventLogging(void)
{
  const LogInfo
    *log_info;

  ExceptionInfo
    *exception;

  if ((log_list == (LinkedListInfo *) NULL) ||
      (IsLinkedListEmpty(log_list) != WizardFalse))
    return(WizardFalse);
  exception=AcquireExceptionInfo();
  log_info=GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  return(log_info->event_mask != NoEvents ? WizardTrue : WizardFalse);
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L i s t L o g I n f o                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ListLogInfo() lists the log info to a file.
%
%  The format of the ListLogInfo method is:
%
%      WizardBooleanType ListLogInfo(FILE *file,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o file:  An pointer to a FILE.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType ListLogInfo(FILE *file,ExceptionInfo *exception)
{
#define MegabytesToBytes(value) ((WizardSizeType) (value)*1024*1024)

  const char
    *path;

  const LogInfo
    **log_info;

  ssize_t
    j;

  register ssize_t
    i;

  size_t
    number_aliases;

  if (file == (const FILE *) NULL)
    file=stdout;
  log_info=GetLogInfoList("*",&number_aliases,exception);
  if (log_info == (const LogInfo **) NULL)
    return(WizardFalse);
  j=0;
  path=(const char *) NULL;
  for (i=0; i < (ssize_t) number_aliases; i++)
  {
    if (log_info[i]->stealth != WizardFalse)
      continue;
    if ((path == (const char *) NULL) ||
        (LocaleCompare(path,log_info[i]->path) != 0))
      {
        size_t
          length;

        if (log_info[i]->path != (char *) NULL)
          (void) fprintf(file,"\nPath: %s\n\n",log_info[i]->path);
        length=0;
        for (j=0; j < (ssize_t) (8*sizeof(LogHandlerType)); j++)
        {
          size_t
            mask;

          mask=1;
          mask<<=j;
          if ((log_info[i]->handler_mask & mask) != 0)
            {
              (void) fprintf(file,"%s ",LogHandlers[j].name);
              length+=strlen(LogHandlers[j].name);
            }
        }
        for (j=(ssize_t) length; j <= 12; j++)
          (void) FormatLocaleFile(file," ");
        (void) fprintf(file," Generations     Limit  Format\n");
        (void) fprintf(file,"-----------------------------------------"
          "--------------------------------------\n");
      }
    path=log_info[i]->path;
    if (log_info[i]->filename != (char *) NULL)
      {
        (void) fprintf(file,"%s",log_info[i]->filename);
        for (j=(ssize_t) strlen(log_info[i]->filename); j <= 16; j++)
          (void) fprintf(file," ");
      }
    (void) fprintf(file,"%9g  ",(double) log_info[i]->generations);
    (void) fprintf(file,"%8g   ",(double) log_info[i]->limit);
    if (log_info[i]->format != (char *) NULL)
      (void) fprintf(file,"%s",log_info[i]->format);
    (void) fprintf(file,"\n");
  }
  (void) fflush(file);
  log_info=(const LogInfo **) RelinquishWizardMemory((void *) log_info);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o g C o m p o n e n t G e n e s i s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LogComponentGenesis() instantiates the log component.
%
%  The format of the LogComponentGenesis method is:
%
%      WizardBooleanType LogComponentGenesis(void)
%
*/
WizardExport WizardBooleanType LogComponentGenesis(void)
{
  ExceptionInfo
    *exception;

  AcquireSemaphoreInfo(&log_semaphore);
  exception=AcquireExceptionInfo();
  (void) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o g C o m p o n e n t T e r m i n u s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LogComponentTerminus() destroys the log component.
%
%  The format of the LogComponentTerminus method is:
%
%      LogComponentTerminus(void)
%
*/

static void *DestroyLogElement(void *log_info)
{
  register LogInfo
    *p;

  p=(LogInfo *) log_info;
  if (p->file != (FILE *) NULL)
    {
      (void) fprintf(p->file,"</log>\n");
      (void) fclose(p->file);
      p->file=(FILE *) NULL;
    }
  if (p->format != (char *) NULL)
    p->format=DestroyString(p->format);
  if (p->filename != (char *) NULL)
    p->filename=DestroyString(p->filename);
  if (p->path != (char *) NULL)
    p->path=DestroyString(p->path);
  if (p->timer != (TimerInfo *) NULL)
    p->timer=DestroyTimerInfo(p->timer);
  p=(LogInfo *) RelinquishWizardMemory(p);
  return((void *) NULL);
}

WizardExport void LogComponentTerminus(void)
{
  if (log_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&log_semaphore);
  LockSemaphoreInfo(log_semaphore);
  if (log_list != (LinkedListInfo *) NULL)
    log_list=DestroyLinkedList(log_list,DestroyLogElement);
  instantiate_log=WizardFalse;
  UnlockSemaphoreInfo(log_semaphore);
  DestroySemaphoreInfo(&log_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L o g W i z a r d E v e n t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LogWizardEvent() logs an event as determined by the log configuration file.
%  If an error occurs, WizardFalse is returned otherwise WizardTrue.
%
%  The format of the LogWizardEvent method is:
%
%      WizardBooleanType LogWizardEvent(const LogEventType type,
%        const char *module,const char *function,const size_t line,
%        const char *format,...)
%
%  A description of each parameter follows:
%
%    o type: The event type.
%
%    o filename: The source module filename.
%
%    o function: The function name.
%
%    o line: The line number of the source module.
%
%    o format: The output format.
%
*/
static char *TranslateEvent(const LogEventType wizard_unused(type),
  const char *module,const char *function,const size_t line,
  const char *domain,const char *event)
{
  char
    *text;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  register char
    *q;

  register const char
    *p;

  size_t
    extent;

  time_t
    seconds;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  seconds=time((time_t *) NULL);
  elapsed_time=GetElapsedTime(log_info->timer);
  user_time=GetUserTime(log_info->timer);
  text=AcquireString(event);
  if (log_info->format == (char *) NULL)
    return(text);
  extent=strlen(event)+MaxTextExtent;
  if (LocaleCompare(log_info->format,"xml") == 0)
    {
      char
        timestamp[MaxTextExtent];

      /*
        Translate event in "XML" format.
      */
      (void) FormatWizardTime(seconds,extent,timestamp);
      (void) FormatLocaleString(text,extent,
        "<entry>\n"
        "  <timestamp>%s</timestamp>\n"
        "  <elapsed-time>%lu:%02lu.%03lu</elapsed-time>\n"
        "  <user-time>%0.3f</user-time>\n"
        "  <process-id>%.20g</process-id>\n"
        "  <thread-id>%.20g</thread-id>\n"
        "  <module>%s</module>\n"
        "  <function>%s</function>\n"
        "  <line>%.20g</line>\n"
        "  <domain>%s</domain>\n"
        "  <event>%s</event>\n"
        "</entry>",timestamp,(unsigned long) (elapsed_time/60.0),(unsigned long)
        floor(fmod(elapsed_time,60.0)),(unsigned long) (1000.0*(elapsed_time-
        floor(elapsed_time))+0.5),user_time,(double) getpid(),(double)
        GetWizardThreadSignature(),module,function,(double) line,domain,event);
      return(text);
    }
  /*
    Translate event in "human readable" format.
  */
  q=text;
  for (p=log_info->format; *p != '\0'; p++)
  {
    *q='\0';
    if ((size_t) (q-text+MaxTextExtent) >= extent)
      {
        extent+=MaxTextExtent;
        text=(char *) ResizeQuantumMemory(text,extent+MaxTextExtent,
          sizeof(*text));
        if (text == (char *) NULL)
          return((char *) NULL);
        q=text+strlen(text);
      }
    /*
      The format of the log is defined by embedding special format characters:

        %c   client name
        %d   domain
        %e   event
        %f   function
        %g   generation
        %l   line
        %m   module
        %n   log name
        %p   process id
        %r   real CPU time
        %t   wall clock time
        %u   user CPU time
        %v   version
        %%   percent sign
        \n   newline
        \r   carriage return
    */
    if ((*p == '\\') && (*(p+1) == 'r'))
      {
        *q++='\r';
        p++;
        continue;
      }
    if ((*p == '\\') && (*(p+1) == 'n'))
      {
        *q++='\n';
        p++;
        continue;
      }
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    switch (*p)
    {
      case 'c':
      {
        q+=CopyWizardString(q,GetClientName(),extent);
        break;
      }
      case 'd':
      {
        q+=CopyWizardString(q,domain,extent);
        break;
      }
      case 'e':
      {
        q+=CopyWizardString(q,event,extent);
        break;
      }
      case 'f':
      {
        q+=CopyWizardString(q,function,extent);
        break;
      }
      case 'g':
      {
        if (log_info->generations == 0)
          {
            (void) CopyWizardString(q,"0",extent);
            q++;
            break;
          }
        q+=FormatLocaleString(q,extent,"%.20g",(double) (log_info->generation %
          log_info->generations));
        break;
      }
      case 'l':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) line);
        break;
      }
      case 'm':
      {
        register const char
          *p;

        for (p=module+strlen(module)-1; p > module; p--)
          if (*p == *DirectorySeparator)
            {
              p++;
              break;
            }
        q+=CopyWizardString(q,p,extent);
        break;
      }
      case 'n':
      {
        q+=CopyWizardString(q,GetLogName(),extent);
        break;
      }
      case 'p':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) getpid());
        break;
      }
      case 'r':
      {
        q+=FormatLocaleString(q,extent,"%lu:%02lu.%03lu",(unsigned long)
          (elapsed_time/60.0),(unsigned long) floor(fmod(elapsed_time,60.0)),
          (unsigned long) (1000.0*(elapsed_time-floor(elapsed_time))+0.5));
        break;
      }
      case 't':
      {
        q+=FormatWizardTime(seconds,extent,q);
        break;
      }
      case 'u':
      {
        q+=FormatLocaleString(q,extent,"%0.3fu",user_time);
        break;
      }
      case 'v':
      {
        q+=CopyWizardString(q,WizardLibVersionText,extent);
        break;
      }
      case '%':
      {
        *q++=(*p);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  *q='\0';
  return(text);
}

static char *TranslateFilename(const LogInfo *log_info)
{
  char
    *filename;

  register char
    *q;

  register const char
    *p;

  size_t
    extent;

  /*
    Translate event in "human readable" format.
  */
  assert(log_info != (LogInfo *) NULL);
  assert(log_info->filename != (char *) NULL);
  filename=AcquireString((char *) NULL);
  extent=MaxTextExtent;
  q=filename;
  for (p=log_info->filename; *p != '\0'; p++)
  {
    *q='\0';
    if ((size_t) (q-filename+MaxTextExtent) >= extent)
      {
        extent+=MaxTextExtent;
        filename=(char *) ResizeQuantumMemory(filename,extent+MaxTextExtent,
          sizeof(*filename));
        if (filename == (char *) NULL)
          return((char *) NULL);
        q=filename+strlen(filename);
      }
    /*
      The format of the filename is defined by embedding special format
      characters:

        %c   client name
        %n   log name
        %p   process id
        %v   version
        %%   percent sign
    */
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    switch (*p)
    {
      case 'c':
      {
        q+=CopyWizardString(q,GetClientName(),extent);
        break;
      }
      case 'g':
      {
        if (log_info->generations == 0)
          {
            (void) CopyWizardString(q,"0",extent);
            q++;
            break;
          }
        q+=FormatLocaleString(q,extent,"%.20g",(double) (log_info->generation %
          log_info->generations));
        break;
      }
      case 'n':
      {
        q+=CopyWizardString(q,GetLogName(),extent);
        break;
      }
      case 'p':
      {
        q+=FormatLocaleString(q,extent,"%.20g",(double) getpid());
        break;
      }
      case 'v':
      {
        q+=CopyWizardString(q,WizardLibVersionText,extent);
        break;
      }
      case '%':
      {
        *q++=(*p);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  *q='\0';
  return(filename);
}

WizardBooleanType LogWizardEventList(const LogEventType type,const char *module,
  const char *function,const size_t line,const char *format,
  va_list operands)
{
  char
    event[MaxTextExtent],
    *text;

  const char
    *domain;

  ExceptionInfo
    *exception;

  int
    n;

  LogInfo
    *log_info;

  if (IsEventLogging() == WizardFalse)
    return(WizardFalse);
  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  LockSemaphoreInfo(log_semaphore);
  if ((log_info->event_mask & type) == 0)
    {
      UnlockSemaphoreInfo(log_semaphore);
      return(WizardTrue);
    }
  domain=WizardOptionToMnemonic(WizardLogEventOptions,type);
#if defined(WIZARDSTOOLKIT_HAVE_VSNPRINTF)
  n=vsnprintf(event,MaxTextExtent,format,operands);
#else
  n=vsprintf(event,format,operands);
#endif
  if (n < 0)
    event[MaxTextExtent-1]='\0';
  text=TranslateEvent(type,module,function,line,domain,event);
  if (text == (char *) NULL)
    {
      (void) ContinueTimer(log_info->timer);
      UnlockSemaphoreInfo(log_semaphore);
      return(WizardFalse);
    }
  if ((log_info->handler_mask & ConsoleHandler) != 0)
    {
      (void) fprintf(stderr,"%s\n",text);
      (void) fflush(stderr);
    }
  if ((log_info->handler_mask & DebugHandler) != 0)
    {
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
      OutputDebugString(text);
      OutputDebugString("\n");
#endif
    }
  if ((log_info->handler_mask & EventHandler) != 0)
    {
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
      (void) NTReportEvent(text,WizardFalse);
#endif
    }
  if ((log_info->handler_mask & FileHandler) != 0)
    {
      struct stat
        file_info;

      file_info.st_size=0;
      if (log_info->file != (FILE *) NULL)
        (void) fstat(fileno(log_info->file),&file_info);
      if (file_info.st_size > (ssize_t) (1024*1024*log_info->limit))
        {
          (void) fprintf(log_info->file,"</log>\n");
          (void) fclose(log_info->file);
          log_info->file=(FILE *) NULL;
        }
      if (log_info->file == (FILE *) NULL)
        {
          char
            *filename;

          filename=TranslateFilename(log_info);
          if (filename == (char *) NULL)
            {
              (void) ContinueTimer(log_info->timer);
              UnlockSemaphoreInfo(log_semaphore);
              return(WizardFalse);
            }
          log_info->append=IsPathAcessible(filename);
          log_info->file=fopen_utf8(filename,"ab");
          filename=(char  *) RelinquishWizardMemory(filename);
          if (log_info->file == (FILE *) NULL)
            {
              UnlockSemaphoreInfo(log_semaphore);
              return(WizardFalse);
            }
          log_info->generation++;
          if (log_info->append == WizardFalse)
            (void) fprintf(log_info->file,"<?xml version=\"1.0\" "
              "encoding=\"UTF-8\" standalone=\"yes\"?>\n");
          (void) fprintf(log_info->file,"<log>\n");
        }
      (void) fprintf(log_info->file,"  <event>%s</event>\n",text);
      (void) fflush(log_info->file);
    }
  if ((log_info->handler_mask & StdoutHandler) != 0)
    {
      (void) fprintf(stdout,"%s\n",text);
      (void) fflush(stdout);
    }
  if ((log_info->handler_mask & StderrHandler) != 0)
    {
      (void) fprintf(stderr,"%s\n",text);
      (void) fflush(stderr);
    }
  text=(char  *) RelinquishWizardMemory(text);
  (void) ContinueTimer(log_info->timer);
  UnlockSemaphoreInfo(log_semaphore);
  return(WizardTrue);
}

WizardBooleanType LogWizardEvent(const LogEventType type,const char *module,
  const char *function,const size_t line,const char *format,...)
{
  va_list
    operands;

  WizardBooleanType
    status;

  va_start(operands,format);
  status=LogWizardEventList(type,module,function,line,format,operands);
  va_end(operands);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   L o a d L o g L i s t                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadLogList() loads the log configuration file which provides a
%  mapping between log attributes and log name.
%
%  The format of the LoadLogList method is:
%
%      WizardBooleanType LoadLogList(const char *xml,const char *filename,
%        const size_t depth,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o xml:  The log list in XML format.
%
%    o filename:  The log list filename.
%
%    o depth: depth of <include /> statements.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType LoadLogList(const char *xml,const char *filename,
  const size_t depth,ExceptionInfo *exception)
{
  char
    keyword[MaxTextExtent],
    *token;

  const char
    *q;

  LogInfo
    *log_info = (LogInfo *) NULL;

  WizardStatusType
    status;

  /*
    Load the log map file.
  */
  if (xml == (const char *) NULL)
    return(WizardFalse);
  if (log_list == (LinkedListInfo *) NULL)
    {
      log_list=NewLinkedList(0);
      if (log_list == (LinkedListInfo *) NULL)
        {
          (void) ThrowWizardException(exception,GetWizardModule(),
            ConfigureError,"memory allocation failed: `%s'",filename);
          return(WizardFalse);
        }
    }
  status=WizardTrue;
  token=AcquireString((char *) xml);
  for (q=(char *) xml; *q != '\0'; )
  {
    /*
      Interpret XML.
    */
    GetWizardToken(q,&q,token);
    if (*token == '\0')
      break;
    (void) CopyWizardString(keyword,token,MaxTextExtent);
    if (LocaleNCompare(keyword,"<!DOCTYPE",9) == 0)
      {
        /*
          Doctype element.
        */
        while ((LocaleNCompare(q,"]>",2) != 0) && (*q != '\0'))
          GetWizardToken(q,&q,token);
        continue;
      }
    if (LocaleNCompare(keyword,"<!--",4) == 0)
      {
        /*
          Comment element.
        */
        while ((LocaleNCompare(q,"->",2) != 0) && (*q != '\0'))
          GetWizardToken(q,&q,token);
        continue;
      }
    if (LocaleCompare(keyword,"<include") == 0)
      {
        /*
          Include element.
        */
        while (((*token != '/') && (*(token+1) != '>')) && (*q != '\0'))
        {
          (void) CopyWizardString(keyword,token,MaxTextExtent);
          GetWizardToken(q,&q,token);
          if (*token != '=')
            continue;
          GetWizardToken(q,&q,token);
          if (LocaleCompare(keyword,"file") == 0)
            {
              if (depth > 200)
                (void) ThrowWizardException(exception,GetWizardModule(),
                  ConfigureError,"include element nested too deeply: `%s'",
                  token);
              else
                {
                  char
                    path[MaxTextExtent],
                    *xml;

                  GetPathComponent(filename,HeadPath,path);
                  if (*path != '\0')
                    (void) ConcatenateWizardString(path,DirectorySeparator,
                      MaxTextExtent);
                  if (*token == *DirectorySeparator)
                    (void) CopyWizardString(path,token,MaxTextExtent);
                  else
                    (void) ConcatenateWizardString(path,token,MaxTextExtent);
                  xml=FileToString(path,~0,exception);
                  if (xml != (char *) NULL)
                    {
                      status&=LoadLogList(xml,path,depth+1,exception);
                      xml=DestroyString(xml);
                    }
                }
            }
        }
        continue;
      }
    if (LocaleCompare(keyword,"<logmap>") == 0)
      {
        /*
          Allocate memory for the log list.
        */
        log_info=(LogInfo *) AcquireWizardMemory(sizeof(*log_info));
        if (log_info == (LogInfo *) NULL)
          ThrowFatalException(ResourceFatalError,
            "memory allocation failed: `%s'");
        (void) ResetWizardMemory(log_info,0,sizeof(*log_info));
        log_info->path=ConstantString(filename);
        log_info->timer=AcquireTimerInfo();
        log_info->signature=WizardSignature;
        continue;
      }
    if (log_info == (LogInfo *) NULL)
      continue;
    if (LocaleCompare(keyword,"</logmap>") == 0)
      {
        status=AppendValueToLinkedList(log_list,log_info);
        if (status == WizardFalse)
          (void) ThrowWizardException(exception,GetWizardModule(),
            ResourceError,"memory allocation failed: `%s'",filename);
        log_info=(LogInfo *) NULL;
      }
    GetWizardToken(q,(const char **) NULL,token);
    if (*token != '=')
      continue;
    GetWizardToken(q,&q,token);
    GetWizardToken(q,&q,token);
    switch (*keyword)
    {
      case 'E':
      case 'e':
      {
        if (LocaleCompare((char *) keyword,"events") == 0)
          {
            log_info->event_mask=(LogEventType) (log_info->event_mask |
              ParseWizardOption(WizardLogEventOptions,WizardTrue,token));
            break;
          }
        break;
      }
      case 'F':
      case 'f':
      {
        if (LocaleCompare((char *) keyword,"filename") == 0)
          {
            if (log_info->filename != (char *) NULL)
              log_info->filename=(char *)
                RelinquishWizardMemory(log_info->filename);
            log_info->filename=ConstantString(token);
            break;
          }
        if (LocaleCompare((char *) keyword,"format") == 0)
          {
            if (log_info->format != (char *) NULL)
              log_info->format=(char *)
                RelinquishWizardMemory(log_info->format);
            log_info->format=ConstantString(token);
            break;
          }
        break;
      }
      case 'G':
      case 'g':
      {
        if (LocaleCompare((char *) keyword,"generations") == 0)
          {
            if (LocaleCompare(token,"unlimited") == 0)
              {
                log_info->generations=(~0UL);
                break;
              }
            log_info->generations=StringToUnsignedLong(token);
            break;
          }
        break;
      }
      case 'L':
      case 'l':
      {
        if (LocaleCompare((char *) keyword,"limit") == 0)
          {
            if (LocaleCompare(token,"unlimited") == 0)
              {
                log_info->limit=(~0UL);
                break;
              }
            log_info->limit=StringToUnsignedLong(token);
            break;
          }
        break;
      }
      case 'O':
      case 'o':
      {
        if (LocaleCompare((char *) keyword,"output") == 0)
          {
            log_info->handler_mask=(LogHandlerType)
              (log_info->handler_mask | ParseLogHandlers(token));
            break;
          }
        break;
      }
      default:
        break;
    }
  }
  token=DestroyString(token);
  if (log_list == (LinkedListInfo *) NULL)
    return(WizardFalse);
  return(status != 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L o a d L o g L i s t s                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LoadLogLists() loads one or more log configuration file which provides a
%  mapping between log attributes and log name.
%
%  The format of the LoadLogLists method is:
%
%      WizardBooleanType LoadLogLists(const char *filename,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: The log configuration filename.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType LoadLogLists(const char *filename,
  ExceptionInfo *exception)
{
  const StringInfo
    *option;

  LinkedListInfo
    *options;

  WizardStatusType
    status;

  register ssize_t
    i;

  /*
    Load built-in log map.
  */
  status=WizardFalse;
  if (log_list == (LinkedListInfo *) NULL)
    {
      log_list=NewLinkedList(0);
      if (log_list == (LinkedListInfo *) NULL)
        {
          ThrowFileException(exception,FileError,filename);
          return(WizardFalse);
        }
    }
  /*
    Load external log map.
  */
  status=WizardTrue;
  options=GetConfigureOptions(filename,exception);
  option=(const StringInfo *) GetNextValueInLinkedList(options);
  while (option != (const StringInfo *) NULL)
  {
    status&=LoadLogList((const char *) GetStringInfoDatum(option),
      GetStringInfoPath(option),0,exception);
    option=(const StringInfo *) GetNextValueInLinkedList(options);
  }
  /*
    Load built-in log map.
  */
  options=DestroyConfigureOptions(options);
  for (i=0; i < (ssize_t) (sizeof(LogMap)/sizeof(*LogMap)); i++)
  {
    LogInfo
      *log_info;

    register const LogMapInfo
      *p;

    p=LogMap+i;
    log_info=(LogInfo *) AcquireWizardMemory(sizeof(*log_info));
    if (log_info == (LogInfo *) NULL)
      {
        (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
          "memory allocation failed `%s'",log_info->name);
        continue;
      }
    (void) ResetWizardMemory(log_info,0,sizeof(*log_info));
    log_info->path=ConstantString("[built-in]");
    log_info->timer=AcquireTimerInfo();
    log_info->event_mask=p->event_mask;
    log_info->handler_mask=p->handler_mask;
    log_info->filename=ConstantString(p->filename);
    log_info->format=ConstantString(p->format);
    log_info->signature=WizardSignature;
    status&=AppendValueToLinkedList(log_list,log_info);
    if (status == WizardFalse)
      (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
        "memory allocation failed `%s'",log_info->name);
  }
  return(status != 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   P a r s e L o g H a n d l e r s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ParseLogHandlers() parses a string defining which handlers takes a log
%  message and exports them.
%
%  The format of the ParseLogHandlers method is:
%
%      LogHandlerType ParseLogHandlers(const char *handlers)
%
%  A description of each parameter follows:
%
%    o handlers: one or more handlers separated by commas.
%
*/
static LogHandlerType ParseLogHandlers(const char *handlers)
{
  LogHandlerType
    handler_mask;

  register const char
    *p;

  register ssize_t
    i;

  size_t
    length;

  handler_mask=NoHandler;
  for (p=handlers; p != (char *) NULL; p=strchr(p,','))
  {
    while ((*p != '\0') && ((isspace((int) ((unsigned char) *p)) != 0) ||
           (*p == ',')))
      p++;
    for (i=0; LogHandlers[i].name != (char *) NULL; i++)
    {
      length=strlen(LogHandlers[i].name);
      if (LocaleNCompare(p,LogHandlers[i].name,length) == 0)
        {
          handler_mask=(LogHandlerType) (handler_mask | LogHandlers[i].handler);
          break;
        }
    }
    if (LogHandlers[i].name == (char *) NULL)
      return(UndefinedHandler);
  }
  return(handler_mask);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t L o g E v e n t M a s k                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetLogEventMask() accepts a list that determines which events to log.  All
%  other events are ignored.  By default, no debug is enabled.  This method
%  returns the previous log event mask.
%
%  The format of the SetLogEventMask method is:
%
%      LogEventType SetLogEventMask(const char *events)
%
%  A description of each parameter follows:
%
%    o events: log these events.
%
*/
WizardExport LogEventType SetLogEventMask(const char *events)
{
  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  ssize_t
    option;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  option=ParseWizardOption(WizardLogEventOptions,WizardTrue,events);
  LockSemaphoreInfo(log_semaphore);
  log_info=(LogInfo *) GetValueFromLinkedList(log_list,0);
  log_info->event_mask=(LogEventType) option;
  if (option == -1)
    log_info->event_mask=UndefinedEvents;
  UnlockSemaphoreInfo(log_semaphore);
  return(log_info->event_mask);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t L o g F o r m a t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetLogFormat() sets the format for the "human readable" log record.
%
%  The format of the LogWizardFormat method is:
%
%      SetLogFormat(const char *format)
%
%  A description of each parameter follows:
%
%    o format: The log record format.
%
*/
WizardExport void SetLogFormat(const char *format)
{
  ExceptionInfo
    *exception;

  LogInfo
    *log_info;

  exception=AcquireExceptionInfo();
  log_info=(LogInfo *) GetLogInfo("*",exception);
  exception=DestroyExceptionInfo(exception);
  LockSemaphoreInfo(log_semaphore);
  if (log_info->format != (char *) NULL)
    log_info->format=DestroyString(log_info->format);
  log_info->format=ConstantString(format);
  UnlockSemaphoreInfo(log_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t L o g N a m e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetLogName() sets the log name and returns it.
%
%  The format of the SetLogName method is:
%
%      const char *SetLogName(const char *name)
%
%  A description of each parameter follows:
%
%    o log_name: SetLogName() returns the current client name.
%
%    o name: Specifies the new client name.
%
*/
WizardExport const char *SetLogName(const char *name)
{
  if ((name != (char *) NULL) && (*name != '\0'))
    (void) CopyWizardString(log_name,name,MaxTextExtent);
  return(log_name);
}
