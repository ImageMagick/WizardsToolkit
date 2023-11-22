/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit timer methods.
*/
#ifndef _WIZARDSTOOLKIT_TIMER_H
#define _WIZARDSTOOLKIT_TIMER_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define WizardEpsilon  1.0e-15

typedef enum
{
  UndefinedTimerState,
  StoppedTimerState,
  RunningTimerState
} TimerState;

typedef struct _Timer
{
  double
    start,
    stop,
    total;
} Timer;

typedef struct _TimerInfo
  TimerInfo;

extern WizardExport double
  GetElapsedTime(TimerInfo *),
  GetUserTime(TimerInfo *);

extern WizardExport TimerInfo
  *AcquireTimerInfo(void),
  *DestroyTimerInfo(TimerInfo *);

extern WizardExport WizardBooleanType
  ContinueTimer(TimerInfo *);

extern WizardExport void
  GetTimerInfo(TimerInfo *),
  ResetTimer(TimerInfo *),
  StartTimer(TimerInfo *,const WizardBooleanType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
