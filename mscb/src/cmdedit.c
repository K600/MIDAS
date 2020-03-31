/********************************************************************\

  Name:         cmdedit.c
  Created by:   Stefan Ritt

  Contents:     Command-line editor for msc

\********************************************************************/

#include <stdio.h>
#include <string.h>

#define MAX_HISTORY 50
#define LINE_LENGTH 256

#define TRUE 1
#define FALSE 0

char history[MAX_HISTORY][LINE_LENGTH];
int his_index = 0;

extern int ss_getchar(int reset);
extern char *ss_gets(char *string, int size);
extern unsigned int ss_millitime();

/* special characters */
#define CH_BS             8
#define CH_TAB            9
#define CH_CR            13

#define CH_EXT 0x100

#define CH_HOME   (CH_EXT+0)
#define CH_INSERT (CH_EXT+1)
#define CH_DELETE (CH_EXT+2)
#define CH_END    (CH_EXT+3)
#define CH_PUP    (CH_EXT+4)
#define CH_PDOWN  (CH_EXT+5)
#define CH_UP     (CH_EXT+6)
#define CH_DOWN   (CH_EXT+7)
#define CH_RIGHT  (CH_EXT+8)
#define CH_LEFT   (CH_EXT+9)

/*------------------------------------------------------------------*/

int cmd_edit(const char *prompt, char *cmd, int(*dir) (char *, int *), int(*idle) ())
{
   char line[LINE_LENGTH];
   int i, j, k, c, hi;
   int status;
   unsigned int last_time = 0;
   int escape_flag = 0;

   if (ss_getchar(0) == -1) {
      /* normal input if ss_getchar not supported */
      fputs(prompt, stdout);
      ss_gets(cmd, 256);
      return strlen(cmd);
   }

   fputs(prompt, stdout);
   fflush(stdout);

   hi = his_index;
   memset(line, 0, LINE_LENGTH);
   memset(history[hi], 0, LINE_LENGTH);
   strcpy(line, cmd);
   fputs(line, stdout);
   i = strlen(cmd);
   fflush(stdout);

   do {
      c = ss_getchar(0);

      if (c == 27)
         escape_flag = TRUE;

      if (c >= ' ' && c < CH_EXT && escape_flag) {
         escape_flag = FALSE;
         if (c == 'p')
            c = 6;
      }

      /* normal input */
      if (c >= ' ' && c < CH_EXT) {
         if (strlen(line) < LINE_LENGTH - 1) {
            for (j = strlen(line); j >= i; j--)
               line[j + 1] = line[j];
            if (i < LINE_LENGTH - 1) {
               line[i++] = c;
               fputc(c, stdout);
            }
            for (j = i; j < (int) strlen(line); j++)
               fputc(line[j], stdout);
            for (j = i; j < (int) strlen(line); j++)
               fputc('\b', stdout);
         }
      }

      /* BS */
      if (c == CH_BS && i > 0) {
         i--;
         fputc('\b', stdout);
         for (j = i; j <= (int) strlen(line); j++) {
            line[j] = line[j + 1];
            if (line[j])
               fputc(line[j], stdout);
            else
               fputc(' ', stdout);
         }
         for (k = 0; k < j - i; k++)
            fputc('\b', stdout);
      }

      /* DELETE/Ctrl-D */
      if (c == CH_DELETE || c == 4) {
         for (j = i; j <= (int) strlen(line); j++) {
            line[j] = line[j + 1];
            if (line[j])
               fputc(line[j], stdout);
            else
               fputc(' ', stdout);
         }
         for (k = 0; k < j - i; k++)
            fputc('\b', stdout);
      }

      /* Erase line: CTRL-W, CTRL-U */
      if (c == 23 || c == 21) {
         i = strlen(line);
         memset(line, 0, sizeof(line));
         printf("\r%s", prompt);
         for (j = 0; j < i; j++)
            fputc(' ', stdout);
         for (j = 0; j < i; j++)
            fputc('\b', stdout);
         i = 0;
      }

      /* Erase line from cursor: CTRL-K */
      if (c == 11) {
         for (j = i; j < (int) strlen(line); j++)
            fputc(' ', stdout);
         for (j = i; j < (int) strlen(line); j++)
            fputc('\b', stdout);
         for (j = strlen(line); j >= i; j--)
            line[j] = 0;
      }

      /* left arrow, CTRL-B */
      if ((c == CH_LEFT || c == 2) && i > 0) {
         i--;
         fputc('\b', stdout);
      }

      /* right arrow, CTRL-F */
      if ((c == CH_RIGHT || c == 6) && i < (int) strlen(line))
         fputc(line[i++], stdout);

      /* HOME, CTRL-A */
      if ((c == CH_HOME || c == 1) && i > 0) {
         for (j = 0; j < i; j++)
            fputc('\b', stdout);
         i = 0;
      }

      /* END, CTRL-E */
      if ((c == CH_END || c == 5) && i < (int) strlen(line)) {
         for (j = i; j < (int) strlen(line); j++)
            fputc(line[i++], stdout);
         i = strlen(line);
      }

      /* up arrow / CTRL-P */
      if (c == CH_UP || c == 16) {
         if (history[(hi + MAX_HISTORY - 1) % MAX_HISTORY][0]) {
            hi = (hi + MAX_HISTORY - 1) % MAX_HISTORY;
            i = strlen(line);
            fputc('\r', stdout);
            fputs(prompt, stdout);
            for (j = 0; j < i; j++)
               fputc(' ', stdout);
            for (j = 0; j < i; j++)
               fputc('\b', stdout);
            memcpy(line, history[hi], 256);
            i = strlen(line);
            for (j = 0; j < i; j++)
               fputc(line[j], stdout);
         }
      }

      /* down arrow / CTRL-N */
      if (c == CH_DOWN || c == 14) {
         if (history[hi][0]) {
            hi = (hi + 1) % MAX_HISTORY;
            i = strlen(line);
            fputc('\r', stdout);
            fputs(prompt, stdout);
            for (j = 0; j < i; j++)
               fputc(' ', stdout);
            for (j = 0; j < i; j++)
               fputc('\b', stdout);
            memcpy(line, history[hi], 256);
            i = strlen(line);
            for (j = 0; j < i; j++)
               fputc(line[j], stdout);
         }
      }

      /* CTRL-F */
      if (c == 6) {
         for (j = (hi + MAX_HISTORY - 1) % MAX_HISTORY; j != hi;
              j = (j + MAX_HISTORY - 1) % MAX_HISTORY)
            if (history[j][0] && strncmp(line, history[j], i) == 0) {
               memcpy(line, history[j], 256);
               fputs(line + i, stdout);
               i = strlen(line);
               break;
            }
         if (j == hi)
            fputc(7, stdout);
      }

      /* tab */
      if (c == 9 && dir != NULL) {
         status = dir(line, &i);

         /* redraw line */
         fputc('\r', stdout);
         fputs(prompt, stdout);
         fputs(line, stdout);

         for (j = 0; j < (int) strlen(line) - i; j++)
            fputc('\b', stdout);
      }

      if (c != 0) {
         last_time = ss_millitime();
         fflush(stdout);
      }

      if ((ss_millitime() - last_time > 300) && idle != NULL) {
         status = idle();

         if (status) {
            fputc('\r', stdout);
            fputs(prompt, stdout);
            fputs(line, stdout);

            for (j = 0; j < (int) strlen(line) - i; j++)
               fputc('\b', stdout);

            fflush(stdout);
         }
      }

   } while (c != CH_CR);

   strcpy(cmd, line);

   if (dir != NULL)
      if (strcmp(cmd, history[(his_index + MAX_HISTORY - 1) % MAX_HISTORY]) != 0 &&
          cmd[0]) {
         strcpy(history[his_index], cmd);
         his_index = (his_index + 1) % MAX_HISTORY;
      }

   /* reset terminal */
   ss_getchar(1);

   fputc('\n', stdout);

   return strlen(line);
}
