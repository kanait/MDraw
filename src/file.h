//
// file.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _FILE_H
#define _FILE_H

// defines
#define comment(c) (c == '\n' || c == '!' || c == '%' || c == '#' || c == '*')

// functions
extern void find_fileheader(char *, char *);
extern void find_filename(char *, char *);
extern void filehead(char *, char *);
extern char *open_filename(char *, char *, char *);
extern char *cut(char *, char *);
extern void display(char *, ...);
extern void display_file_open(char *);
extern void display_file_close(void);

#endif // _FILE_H
