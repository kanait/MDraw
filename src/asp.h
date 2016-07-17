//
// asp.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _ASP_H
#define _ASP_H

// functions
extern Splp *asp( SGraph *, Sppd *, int );
extern void add_vertices_edges_sgraph(SGraph *, int, double, int );
extern BOOL isOnDifferentEdges(SGvt *, SGvt *);
extern int sged_num_midpoint( SGed *, SGraph * );
extern SGlp *ShortestPath_Dijkstra_SGraph(SGraph *, SGvt *, SGvt *);
extern double sged_length( SGed * );
extern int sg_stepnumber_to_src(SGVtList *, SGvt *, SGvt *);
extern SGlp *sgvtlist_to_sglp(SGVtList *, SGvt *, SGvt *);
extern SGraph *sglp_to_sgraph(SGlp *, SGraph *);
extern Splp *sglp_to_splp( SGlp *, Sppd * );
extern Splp *sglp_to_splp_new( SGlp *, Sppd * );
// extern SGlpvt *next_sglv( SGlpvt * );
// extern SGed *next_sged( SGlpvt * );

#endif // _ASP_H
