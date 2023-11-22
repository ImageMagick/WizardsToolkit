/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    TTTTT   OOO   K   K  EEEEE  N   N                        %
%                      T    O   O  K  K   E      NN  N                        %
%                      T    O   O  KKK    EEE    N N N                        %
%                      T    O   O  K  K   E      N  NN                        %
%                      T     OOO   K   K  EEEEE  N   N                        %
%                                                                             %
%                                                                             %
%                      Wizard's Toolkit Token Methods                         %
%                                                                             %
%                               Software Design                               %
%                                   Cristy                                    %
%                                 March 2003                                  %
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
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/locale_.h"
#include "wizard/string_.h"
#include "wizard/string-private.h"
#include "wizard/token.h"
#include "wizard/utility.h"

/*
  Typedef declarations.
*/
struct _TokenInfo
{
  int
    state;

  WizardStatusType
    flag;

  ssize_t
    offset;

  char
    quote;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t N e x t T o k e n                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextToken() gets a token from the token stream.  A token is defined as
%  a sequence of characters delimited by whitespace (e.g. clip-path), a
%  sequence delimited with quotes (.e.g "Quote me"), or a sequence enclosed in
%  parenthesis (e.g. rgb(0,0,0)).  GetNextToken() also recognizes these
%  separator characters: ':', '=', ',', and ';'.
%
%  The format of the GetNextToken method is:
%
%      void GetNextToken(const char *start,const char **end,
%        const size_t extent,char *token)
%
%  A description of each parameter follows:
%
%    o start: the start of the token sequence.
%
%    o end: point to the end of the token sequence.
%
%    o extent: maximum extent of the token.
%
%    o token: copy the token to this buffer.
%
*/
WizardExport void GetNextToken(const char *start,const char **end,
  const size_t extent,char *token)
{
  double
    value;

   const char
    *p;

   ssize_t
    i;

  assert(start != (const char *) NULL);
  assert(token != (char *) NULL);
  i=0;
  p=start;
  while ((isspace((int) ((unsigned char) *p)) != 0) && (*p != '\0'))
    p++;
  switch (*p)
  {
    case '\0':
      break;
    case '"':
    case '\'':
    case '`':
    case '{':
    {
       char
        escape;

      switch (*p)
      {
        case '"': escape='"'; break;
        case '\'': escape='\''; break;
        case '`': escape='\''; break;
        case '{': escape='}'; break;
        default: escape=(*p); break;
      }
      for (p++; *p != '\0'; p++)
      {
        if ((*p == '\\') && ((*(p+1) == escape) || (*(p+1) == '\\')))
          p++;
        else
          if (*p == escape)
            {
              p++;
              break;
            }
        if (i < (ssize_t) (extent-1))
          token[i++]=(*p);
      }
      break;
    }
    case '/':
    {
      if (i < (ssize_t) (extent-1))
        token[i++]=(*p++);
      if ((*p == '>') || (*p == '/'))
        if (i < (ssize_t) (extent-1))
          token[i++]=(*p++);
      break;
    }
    default:
    {
      char
        *q;

      value=StringToDouble(p,&q);
      (void) value;
      if ((p != q) && (*p != ','))
        {
          for ( ; (p < q) && (*p != ','); p++)
            if (i < (ssize_t) (extent-1))
              token[i++]=(*p);
          if (*p == '%')
            if (i < (ssize_t) (extent-1))
              token[i++]=(*p++);
          break;
        }
      if ((*p != '\0') && (isalpha((int) ((unsigned char) *p)) == 0) &&
          (*p != *DirectorySeparator) && (*p != '#') && (*p != '<'))
        {
          if (i < (ssize_t) (extent-1))
            token[i++]=(*p++);
          break;
        }
      for ( ; *p != '\0'; p++)
      {
        if (((isspace((int) ((unsigned char) *p)) != 0) || (*p == '=') ||
            (*p == ',') || (*p == ':') || (*p == ';')) && (*(p-1) != '\\'))
          break;
        if ((i > 0) && (*p == '<'))
          break;
        if (i < (ssize_t) (extent-1))
          token[i++]=(*p);
        if (*p == '>')
          break;
        if (*p == '(')
          for (p++; *p != '\0'; p++)
          {
            if (i < (ssize_t) (extent-1))
              token[i++]=(*p);
            if ((*p == ')') && (*(p-1) != '\\'))
              break;
          }
      }
      break;
    }
  }
  token[i]='\0';
  if (LocaleNCompare(token,"url(",4) == 0)
    {
      ssize_t
        offset;

      offset=4;
      if (token[offset] == '#')
        offset++;
      i=(ssize_t) strlen(token);
      (void) CopyWizardString(token,token+offset,WizardPathExtent);
      token[i-offset-1]='\0';
    }
  while (isspace((int) ((unsigned char) *p)) != 0)
    p++;
  if (end != (const char **) NULL)
    *end=(const char *) p;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G l o b E x p r e s s i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GlobExpression() returns WizardTrue if the expression matches the pattern.
%
%  The format of the GlobExpression function is:
%
%      WizardBooleanType GlobExpression(const char *expression,
%        const char *pattern,const WizardBooleanType case_insensitive)
%
%  A description of each parameter follows:
%
%    o expression: Specifies a pointer to a text string containing a file name.
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o case_insensitive: set to WizardTrue to ignore the case when matching
%      an expression.
%
*/
WizardExport WizardBooleanType GlobExpression(const char *expression,
  const char *pattern,const WizardBooleanType case_insensitive)
{
  WizardBooleanType
    done,
    match;

  /*
    Return on empty pattern or '*'.
  */
  if (pattern == (char *) NULL)
    return(WizardTrue);
  if (*pattern == '\0')
    return(WizardTrue);
  if (LocaleCompare(pattern,"*") == 0)
    return(WizardTrue);
  /*
    Evaluate glob expression.
  */
  done=WizardFalse;
  while ((*pattern != '\0') && (done == WizardFalse))
  {
    if (*expression == '\0')
      if ((*pattern != '{') && (*pattern != '*'))
        break;
    switch (*pattern)
    {
      case '*':
      {
        WizardBooleanType
          status;

        while (*pattern == '*') 
          pattern++;
        status=WizardFalse;
        while ((*expression != '\0') && (status == WizardFalse))
          status=GlobExpression(expression++,pattern,case_insensitive);
        if (status != WizardFalse)
          {
            while (*expression != '\0')
              expression++;
            while (*pattern != '\0')
              pattern++;
          }
        break;
      }
      case '[':
      {
        char
          c;

        pattern++;
        for ( ; ; )
        {
          if ((*pattern == '\0') || (*pattern == ']'))
            {
              done=WizardTrue;
              break;
            }
          if (*pattern == '\\')
            {
              pattern++;
              if (*pattern == '\0')
                {
                  done=WizardTrue;
                  break;
                }
             }
          if (*(pattern+1) == '-')
            {
              c=(*pattern);
              pattern+=2;
              if (*pattern == ']')
                {
                  done=WizardTrue;
                  break;
                }
              if (*pattern == '\\')
                {
                  pattern++;
                  if (*pattern == '\0')
                    {
                      done=WizardTrue;
                      break;
                    }
                }
              if ((*expression < c) || (*expression > *pattern))
                {
                  pattern++;
                  continue;
                }
            }
          else
            if (*pattern != *expression)
              {
                pattern++;
                continue;
              }
          pattern++;
          while ((*pattern != ']') && (*pattern != '\0'))
          {
            if ((*pattern == '\\') && (*(pattern+1) != '\0'))
              pattern++;
            pattern++;
          }
          if (*pattern != '\0')
            {
              pattern++;
              expression++;
            }
          break;
        }
        break;
      }
      case '?':
      {
        pattern++;
        expression++;
        break;
      }
      case '{':
      {
         const char
          *p;

        pattern++;
        while ((*pattern != '}') && (*pattern != '\0'))
        {
          p=expression;
          match=WizardTrue;
          while ((*p != '\0') && (*pattern != '\0') &&
                 (*pattern != ',') && (*pattern != '}') &&
                 (match != WizardFalse))
          {
            if (*pattern == '\\')
              pattern++;
            match=(*pattern == *p) ? WizardTrue : WizardFalse;
            p++;
            pattern++;
          }
          if (*pattern == '\0')
            {
              match=WizardFalse;
              done=WizardTrue;
              break;
            }
          if (match != WizardFalse)
            {
              expression=p;
               while ((*pattern != '}') && (*pattern != '\0'))
               {
                 pattern++;
                 if (*pattern == '\\')
                   {
                     pattern++;
                     if (*pattern == '}')
                       pattern++;
                   }
               }
             }
           else
             {
               while ((*pattern != '}') && (*pattern != ',') &&
                      (*pattern != '\0'))
               {
                 pattern++;
                 if (*pattern == '\\')
                   {
                     pattern++;
                     if ((*pattern == '}') || (*pattern == ','))
                       pattern++;
                   }
               }
             }
            if (*pattern != '\0')
              pattern++;
          }
        break;
      }
      case '\\':
      {
        pattern++;
        if (*pattern == '\0')
          break;
      }
      default:
      {
        if (case_insensitive != WizardFalse)
          {
            if (tolower((int) ((unsigned char) *expression)) !=
                tolower((int) ((unsigned char) *pattern)))
              {
                done=WizardTrue;
                break;
              }
          }
        else
          if (*expression != *pattern)
            {
              done=WizardTrue;
              break;
            }
        expression++;
        pattern++;
      }
    }
  }
  while (*pattern == '*') 
    pattern++;
  match=(*expression == '\0') && (*pattern == '\0') ? WizardTrue : WizardFalse;
  return(match);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
+     I s G l o b                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsGlob() returns WizardTrue if the path specification contains a globbing
%  pattern.
%
%  The format of the IsGlob method is:
%
%      WizardBooleanType IsGlob(const char *geometry)
%
%  A description of each parameter follows:
%
%    o status: IsGlob() returns WizardTrue if the path specification contains
%      a globbing patten.
%
%    o path: The path.
%
*/
WizardExport WizardBooleanType IsGlob(const char *path)
{
  WizardBooleanType
    status = WizardFalse;

   const char
    *p;

  if (IsPathAcessible(path) != WizardFalse)
    return(WizardFalse);
  for (p=path; *p != '\0'; p++)
  {
    switch (*p)
    {
      case '*':
      case '?':
      case '{':
      case '}':
      case '[':
      case ']':
      {
        status=WizardTrue;
        break;
      }
      default:
        break;
    }
  }
  return(status);
}
