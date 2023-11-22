/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    TTTTT  IIIII  M   M  EEEEE  RRRR                         %
%                      T      I    MM MM  E      R   R                        %
%                      T      I    M M M  EEE    RRRR                         %
%                      T      I    M   M  E      R R                          %
%                      T    IIIII  M   M  EEEEE  R  R                         %
%                                                                             %
%                                                                             %
%                     Wizard's Toolkit Timing Methods                         %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                              January 1993                                   %
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
%  Contributed by Bill Radcliffe and Bob Friesenhahn.
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/log.h"
#include "wizard/memory_.h"
#include "wizard/timer.h"

/*
  Typedef declarations.
*/
struct _TimerInfo
{
  Timer
    user,
    elapsed;

  TimerState
    state;

  size_t
    signature;
};

/*
  Define declarations.
*/
#if !defined(CLOCKS_PER_SEC)
#define CLOCKS_PER_SEC  100
#endif

/*
  Forward declarations.
*/
static double
  UserTime(void);

static void
  StopTimer(TimerInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e T i m e r I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireTimerInfo() initializes the TimerInfo structure.  It effectively
%  creates a stopwatch and starts it.
%
%  The format of the AcquireTimerInfo method is:
%
%      TimerInfo *AcquireTimerInfo(void)
%
*/
WizardExport TimerInfo *AcquireTimerInfo(void)
{
  TimerInfo
    *timer_info;

  timer_info=(TimerInfo *) AcquireWizardMemory(sizeof(*timer_info));
  if (timer_info == (TimerInfo *) NULL)
    ThrowWizardFatalError(CipherDomain,MemoryError);
  (void) memset(timer_info,0,sizeof(*timer_info));
  timer_info->signature=WizardSignature;
  GetTimerInfo(timer_info);
  return(timer_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n t i n u e T i m e r                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ContinueTimer() resumes a stopped stopwatch. The stopwatch continues
%  counting from the last StartTimer() onwards.
%
%  The format of the ContinueTimer method is:
%
%      WizardBooleanType ContinueTimer(TimerInfo *timer_info)
%
%  A description of each parameter follows.
%
%    o  timer_info: Time statistics structure.
%
*/
WizardExport WizardBooleanType ContinueTimer(TimerInfo *timer_info)
{
  assert(timer_info != (TimerInfo *) NULL);
  assert(timer_info->signature == WizardSignature);
  if (timer_info->state == UndefinedTimerState)
    return(WizardFalse);
  if (timer_info->state == StoppedTimerState)
    {
      timer_info->user.total-=timer_info->user.stop-timer_info->user.start;
      timer_info->elapsed.total-=
        timer_info->elapsed.stop-timer_info->elapsed.start;
    }
  timer_info->state=RunningTimerState;
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y T i m e r I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyTimerInfo() zeros memory associated with the TimerInfo structure.
%
%  The format of the DestroyTimerInfo method is:
%
%      TimerInfo *DestroyTimerInfo(TimerInfo *timer_info)
%
%  A description of each parameter follows:
%
%    o timer_info: The cipher context.
%
*/
WizardExport TimerInfo *DestroyTimerInfo(TimerInfo *timer_info)
{
  WizardAssert(CipherDomain,timer_info != (TimerInfo *) NULL);
	WizardAssert(CipherDomain,timer_info->signature == WizardSignature);
  timer_info->signature=(~WizardSignature);
  timer_info=(TimerInfo *) RelinquishWizardMemory(timer_info);
  return(timer_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E l a p s e d T i m e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ElapsedTime() returns the elapsed time (in seconds) since the last call to
%  StartTimer().
%
%  The format of the ElapsedTime method is:
%
%      double ElapsedTime()
%
*/
static double ElapsedTime(void)
{
#if defined(WIZARDSTOOLKIT_HAVE_TIMES) && defined(WIZARDSTOOLKIT_HAVE_SYSCONF)
  struct tms
    timer;

  return((double) times(&timer)/sysconf(_SC_CLK_TCK));
#else
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  return(NTElapsedTime());
#else
  return((double) clock()/CLOCKS_PER_SEC);
#endif
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E l a p s e d T i m e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetElapsedTime() returns the elapsed time (in seconds) passed between the
%  start and stop events. If the stopwatch is still running, it is stopped
%  first.
%
%  The format of the GetElapsedTime method is:
%
%      double GetElapsedTime(TimerInfo *timer_info)
%
%  A description of each parameter follows.
%
%    o  timer_info: Timer statistics structure.
%
*/
WizardExport double GetElapsedTime(TimerInfo *timer_info)
{
  assert(timer_info != (TimerInfo *) NULL);
  assert(timer_info->signature == WizardSignature);
  if (timer_info->state == UndefinedTimerState)
    return(0.0);
  if (timer_info->state == RunningTimerState)
    StopTimer(timer_info);
  return(timer_info->elapsed.total);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t T i m e r I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetTimerInfo() initializes the TimerInfo structure.  It effectively creates
%  a stopwatch and starts it.
%
%  The format of the GetTimerInfo method is:
%
%      void GetTimerInfo(TimerInfo *timer_info)
%
%  A description of each parameter follows.
%
%    o  timer_info: Timer statistics structure.
%
*/
WizardExport void GetTimerInfo(TimerInfo *timer_info)
{
  /*
    Create a stopwatch and start it.
  */
  assert(timer_info != (TimerInfo *) NULL);
  timer_info->state=UndefinedTimerState;
  StartTimer(timer_info,WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t U s e r T i m e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetUserTime() returns the User time (user and system) by the operating
%  system (in seconds) between the start and stop events. If the stopwatch is
%  still running, it is stopped first.
%
%  The format of the GetUserTime method is:
%
%      double GetUserTime(TimerInfo *timer_info)
%
%  A description of each parameter follows.
%
%    o  timer_info: Timer statistics structure.
%
*/
WizardExport double GetUserTime(TimerInfo *timer_info)
{
  assert(timer_info != (TimerInfo *) NULL);
  assert(timer_info->signature == WizardSignature);
  if (timer_info->state == UndefinedTimerState)
    return(0.0);
  if (timer_info->state == RunningTimerState)
    StopTimer(timer_info);
  return(timer_info->user.total);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t T i m e r                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetTimer() resets the stopwatch.
%
%  The format of the ResetTimer method is:
%
%      void ResetTimer(TimerInfo *timer_info)
%
%  A description of each parameter follows.
%
%    o  timer_info: Timer statistics structure.
%
*/
WizardExport void ResetTimer(TimerInfo *timer_info)
{
  assert(timer_info != (TimerInfo *) NULL);
  assert(timer_info->signature == WizardSignature);
  StopTimer(timer_info);
  timer_info->elapsed.stop=0.0;
  timer_info->user.stop=0.0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t a r t T i m e r                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StartTimer() starts the stopwatch.
%
%  The format of the StartTimer method is:
%
%      void StartTimer(TimerInfo *timer_info,const WizardBooleanType reset)
%
%  A description of each parameter follows.
%
%    o  timer_info: Timer statistics structure.
%
%    o  reset: If reset is WizardTrue, then the stopwatch is reset prior to
%       starting.  If reset is WizardFalse, then timing is continued without
%       resetting the stopwatch.
%
*/
WizardExport void StartTimer(TimerInfo *timer_info,
  const WizardBooleanType reset)
{
  assert(timer_info != (TimerInfo *) NULL);
  assert(timer_info->signature == WizardSignature);
  if (reset != WizardFalse)
    {
      /*
        Reset the stopwatch before starting it.
      */
      timer_info->user.total=0.0;
      timer_info->elapsed.total=0.0;
    }
  if (timer_info->state != RunningTimerState)
    {
      timer_info->elapsed.start=ElapsedTime();
      timer_info->user.start=UserTime();
    }
  timer_info->state=RunningTimerState;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S t o p T i m e r                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StopTimer() stops the stopwatch.
%
%  The format of the StopTimer method is:
%
%      void StopTimer(TimerInfo *timer_info)
%
%  A description of each parameter follows.
%
%    o  timer_info: Timer statistics structure.
%
*/
static void StopTimer(TimerInfo *timer_info)
{
  assert(timer_info != (TimerInfo *) NULL);
  assert(timer_info->signature == WizardSignature);
  timer_info->elapsed.stop=ElapsedTime();
  timer_info->user.stop=UserTime();
  if (timer_info->state == RunningTimerState)
    {
      timer_info->user.total+=timer_info->user.stop-
        timer_info->user.start+WizardEpsilon;
      timer_info->elapsed.total+=timer_info->elapsed.stop-
        timer_info->elapsed.start+WizardEpsilon;
    }
  timer_info->state=StoppedTimerState;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   U s e r T i m e                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UserTime() returns the total time the process has been scheduled (in
%  seconds) since the last call to StartTimer().
%
%  The format of the UserTime method is:
%
%      double UserTime()
%
*/
static double UserTime(void)
{
#if defined(WIZARDSTOOLKIT_HAVE_TIMES) && defined(WIZARDSTOOLKIT_HAVE_SYSCONF)
  struct tms
    timer;

  (void) times(&timer);
  return((double) (timer.tms_utime+timer.tms_stime)/sysconf(_SC_CLK_TCK));
#else
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  return(NTUserTime());
#else
  return((double) clock()/CLOCKS_PER_SEC);
#endif
#endif
}
