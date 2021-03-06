//
// sgprique.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _SGPRIQUE_H
#define _SGPRIQUE_H

extern SGPQHeap *create_sgpqheap(int);
extern void swap_sgpqcont(SGPQCont *, SGPQCont *);
extern void insert_sgpqcont(double, SGPQHeap *, SGvt *, SGvt *);
extern void sg_adjust_to_parent(int, SGPQHeap *);
extern void sg_adjust_to_child(int, SGPQHeap *);
extern void adjust_sgpqheap(Id, SGPQHeap *);
extern void deletemin_sgpqcont(SGPQHeap *);
extern void free_sgpqheap(SGPQHeap *);

#endif // _SGPRIQUE_H
