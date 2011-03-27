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
%                                 John Cristy                                 %
%                                 March 2003                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/string_.h"
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
+   G e t W i z a r d T o k e n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardToken() gets a token from the token stream.  A token is defined as a
%  sequence of characters delimited by whitespace (e.g. clip-path), a sequence
%  delimited with quotes (.e.g "Quote me"), or a sequence enclosed in
%  parenthesis (e.g. rgb(0,0,0)).
%
%  The format of the GetWizardToken method is:
%
%      void GetWizardToken(const char *start,const char **end,char *token)
%
%  A description of each parameter follows:
%
%    o start: the start of the token sequence.
%
%    o end: point to the end of the token sequence.
%
%    o token: copy the token to this buffer.
%
*/
WizardExport void GetWizardToken(const char *start,const char **end,char *token)
{
  register const char
    *p;

  register ssize_t
    i;

  i=0;
  for (p=start; *p != '\0'; )
  {
    while ((isspace((int) ((unsigned char) *p)) != 0) && (*p != '\0'))
      p++;
    if (*p == '\0')
      break;
    switch (*p)
    {
      case '"':
      case '\'':
      case '`':
      case '{':
      {
        register char
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
          token[i++]=(*p);
        }
        break;
      }
      case '/':
      {
        token[i++]=(*p++);
        if ((*p == '>') || (*p == '/'))
          token[i++]=(*p++);
        break;
      }
      default:
      {
        char
          *q;

        double
          value;

        value=strtod(p,&q);
        (void) value;
        if ((p != q) && (*p != ','))
          {
            for ( ; (p < q) && (*p != ','); p++)
              token[i++]=(*p);
            if (*p == '%')
              token[i++]=(*p++);
            break;
          }
        if ((isalpha((int) ((unsigned char) *p)) == 0) &&
            (*p != *DirectorySeparator) && (*p != '#') && (*p != '<'))
          {
            token[i++]=(*p++);
            break;
          }
        for ( ; *p != '\0'; p++)
        {
          if (((isspace((int) ((unsigned char) *p)) != 0) || (*p == '=') ||
              (*p == ',') || (*p == ':')) && (*(p-1) != '\\'))
            break;
          if ((i > 0) && (*p == '<'))
            break;
          token[i++]=(*p);
          if (*p == '>')
            break;
          if (*p == '(')
            for (p++; *p != '\0'; p++)
            {
              token[i++]=(*p);
              if ((*p == ')') && (*(p-1) != '\\'))
                break;
            }
        }
        break;
      }
    }
    break;
  }
  token[i]='\0';
  if (LocaleNCompare(token,"url(#",5) == 0)
    {
      i=(ssize_t) strlen(token);
      (void) CopyWizardString(token,token+5,MaxTextExtent);
      token[i-6]='\0';
    }
  if (LocaleNCompare(token,"url(",4) == 0)
    {
      i=(ssize_t) strlen(token);
      (void) CopyWizardString(token,token+4,MaxTextExtent);
      token[i-5]='\0';
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
        register const char
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
          else
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
    status;

  if (IsAccessible(path) != WizardFalse)
    return(WizardFalse);
  status=(strchr(path,'*') != (char *) NULL) ||
    (strchr(path,'?') != (char *) NULL) ||
    (strchr(path,'{') != (char *) NULL) ||
    (strchr(path,'}') != (char *) NULL) ||
    (strchr(path,'[') != (char *) NULL) ||
    (strchr(path,']') != (char *) NULL) ? WizardTrue : WizardFalse;
  return(status);
}
