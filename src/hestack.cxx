//
// hestack.cxx
//
// Hestack functions for subdividing faces according to the path edge
//
// Copyright (c) 1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>
#include <assert.h>
#include "smd.h"
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "veclib.h"

#include "hestack.h"

//
// face subdivision algorithm (new)
//
// (一つの面に乗っている分割エッジは 4 本までという制限をつける)
// タイルを一つずつ作成し，その都度面をアップデートする
// アップデート:
// - ed->sp_fc (only if ed->type = EDGE_ONFACE )
//
void subdivide_ppdface( SubdivFace *sfc, Sppd *ppd,
			int* n_newfc, SubdivFace* newfcall,
			int* n_newed, SubdivEdge* newedall )
{
  Spfc *fc = sfc->fc;

  if ( fc == NULL ) return;
  //if ( fc->hn != TRIANGLE ) return;

  //
  // Hestack の初期化
  // Hestack, Hevt へ格納
  // - he->vt も Hevt (の先頭) に格納
  // - val によってソート
  // ◎ Hevt からエッジへの双方向巡回リストを作る
  //    (その過程で必要なエッジも生成する)
  // newedall への格納も行う
  //
  Hestack *hestack = (Hestack *) malloc( fc->hen * sizeof( Hestack ) );
  initialize_hestack( sfc, hestack, ppd, n_newed, newedall );
  
  //
  // Hestack を使って面を生成
  // 生成するもの (しなくてはならないもの)
  // - face
  // - halfedge
  // - fc->nrm
  // newfcall への格納も行う
  subdivide_ppdface_by_hestack( fc, hestack, ppd, n_newfc, newfcall );

  // 
  // お役ごめん
  //
  for ( int i = 0; i < fc->hen; ++i ) {
    for ( int j = 0; j < hestack[i].hevtn; ++j ) {
      free_halfedgevertexedge( &(hestack[i].hevt[j]) );
    }
    free( hestack[i].hevt );
  }
  free( hestack );
}

/////////////////////////////////////////////////////////////////////
//
// Hestack Functions
//

//
// Hestack の初期化
// Hestack, Hevt へ格納
// - he->vt も Hevt (の先頭) に格納
// - val によってソート
// ◎ Hevt からエッジへの双方向巡回リストを作る
//
void initialize_hestack( SubdivFace *sfc, Hestack *hestack, Sppd *ppd,
			 int* n_newed, SubdivEdge* newedall )
{ 
  Spfc *fc = sfc->fc;
  Sphe *he = fc->sphe;
  // temp
//    display("fc %d\n", fc->no );
//    do {
//      display("he vt %d ed %d\n", he->vt->no, he->ed->no );
//    } while ( (he = he->nxt) != fc->sphe );
  //
  // Hestack の初期化
  //
  int i, j, n;
  //Sphe *he = fc->sphe;
  for ( i = 0; i < fc->hen; ++i ) {

    hestack[i].he = he;
    hestack[i].hevtn = 0;

    // 
    // hevt の初期化
    // 最大で (sfc->en + 1) の数の hevt を持つのでそれだけ確保
    // sfc->en: その面を通るエッジの数 + 1 (そのエッジ (ハーフエッジ) の始点)
    //
    hestack[i].hevt = (Hevt *) malloc( (sfc->en +1) * sizeof(Hevt) );
    for ( j = 0; j < (sfc->en + 1); ++j ) initialize_hevt( &(hestack[i].hevt[j]) );
    
    he = he->nxt;
    
  }

  //
  // Hestack, Hevt へ格納
  // - he->vt も Hevt (の先頭) に格納
  // - val によってソート
  //

  // he->vt を Hevt (の先頭) に格納
  he = fc->sphe;
  i = 0;
  do {
    hestack[i].hevt[0].vt = he->vt;
    hestack[i].hevt[0].val = 0.0;
    //hestack[i].hevt[0].isUsed = FALSE;
    ++(hestack[i].hevtn);
    ++i;
  } while ( (he = he->nxt) != fc->sphe );

  
  // 分割エッジの頂点の挿入
  Sped *ed;
  for ( i = 0; i < sfc->en; ++i ) {
    ed = sfc->ed[i];
    if ( ed->sv->type != VERTEX_ORIGINAL ) {
      n = find_henum_hestack( ed->sv->sp_ed, fc->hen, hestack );
//        display("n %d\n", n );
      assert( n != SMDNULL );
      insert_hevt_in_hestack( ed->sv, ed, &(hestack[n]) );
    }
    if ( ed->ev->type != VERTEX_ORIGINAL ) {
      n = find_henum_hestack( ed->ev->sp_ed, fc->hen, hestack );
//        display("n %d\n", n );
      assert( n != SMDNULL );
      insert_hevt_in_hestack( ed->ev, ed, &(hestack[n]) );
    }
  }

  // i と j を hevt の中に入れる (後で使うので)
  for ( i = 0; i < fc->hen; ++i ) {
    for ( j = 0; j < hestack[i].hevtn; ++j ) {
      hestack[i].hevt[j].hsid = i;
      hestack[i].hevt[j].hvid = j;
    }
  }

  // hevted (双方向巡回リスト) を作成する
  //
  // 1. 三角形ループ上のエッジから hevted を作成する時
  //   その頂点に入るエッジを val=1, 出るエッジを val =0 とする
  //
  // 2. 分割エッジから hevted を作成する時
  //   2.1 分割頂点に入る時 ( vt->type != ORIGINAL )
  //     val=0.5 として挿入
  //     ( 注意: 同じ分割頂点に 2 本以上のエッジが入る
  //     ことはない -> 境界エッジの入力時にチェックする )
  //   2.2 三角形の頂点の時
  //     反対側の hestack にある対応した hevt->val の値を用いる

  // 三角形ループ上のエッジから hevted を作成する
  Hevt *hevt = &(hestack[0].hevt[0]);
  Hevt *nhevt = NULL;
  Spvt *vt, *nvt;
  do {
    nhevt = next_hevt_triloop( hevt, fc->hen, hestack );

    vt  = hevt->vt;

//      if ( vt->no == 10323 ) display("vt %d fc %d \n", vt->no, fc->no );
    
    nvt = nhevt->vt;
    if ( ( ed = find_ppdedge( vt, nvt ) ) == NULL ) {
      ed = create_ppdedge( ppd );
      ed->sv = vt;
      ed->ev = nvt;
      ed->length = V3DistanceBetween2Points( &(ed->sv->vec), &(ed->ev->vec) );
      (void) create_vtxed( vt, ed );
      (void) create_vtxed( nvt, ed );
      if ( ed->no == 37391 ) {
	display("@@@@ ed %d sv %d %g %g %g ev %d %g %g %g\n",
		ed->no,
		ed->sv->no, ed->sv->vec.x, ed->sv->vec.y, ed->sv->vec.z, 
		ed->ev->no, ed->ev->vec.x, ed->ev->vec.y, ed->ev->vec.z );
      }

      // newedall への格納
      if ( n_newed != NULL )
	{
	  // 新しく生成されたエッジの格納
	  newedall[*n_newed].ed = ed; ++(*n_newed);
	}
    }
    (void) insert_hevted( 0.0, ed, hevt );
    (void) insert_hevted( 1.0, ed, nhevt );
    
    hevt = nhevt;
  } while ( hevt != &(hestack[0].hevt[0]) );
  
  // 分割エッジから hevted を作成する
  for ( i = 0; i < sfc->en; ++i ) {
    ed = sfc->ed[i];
    for ( j = 0; j < 2; ++j ) {
      vt = (!j) ? ed->sv : ed->ev;
      hevt = find_hevt_from_hestack( vt, fc->hen, hestack );
      if ( vt->type == VERTEX_ADDED ) {
	// val=0.5 として挿入
	(void) insert_hevted( 0.5, ed, hevt );
      } else {
	Spvt *pvt = another_vt( ed, vt );
	Hevt *phevt = find_hevt_from_hestack( pvt, fc->hen, hestack );
	(void) insert_hevted( phevt->val, ed, hevt );
      }
    }
  }

  // hevted の mate を作成する
  hevted_makemate( fc->hen, hestack );

  // temp
  for ( i = 0; i < fc->hen; ++i ) {

    display("hestack no.%d ed %d hevtn %d\n",
	    i,
	    hestack[i].he->ed->no,
	    hestack[i].hevtn );
    for ( j = 0; j < hestack[i].hevtn; ++j ) {
      display("\thevt hsid %d hvid %d vt %d val %g\n",
	      hestack[i].hevt[j].hsid,
	      hestack[i].hevt[j].hvid,
	      hestack[i].hevt[j].vt->no,
	      hestack[i].hevt[j].val );
      for ( Hevted *hve = hestack[i].hevt[j].svted; hve != NULL; hve = hve->nxt ) {
	display("\t hevted ed %d val %g mate ed %d vt %d\n",
		hve->ed->no,
		hve->val,
		( hve->mate != NULL ) ? hve->mate->ed->no : -1,
		( hve->mate != NULL ) ? hve->mate->hevt->vt->no : -1
		);
      }
    }
  }

}

//
// he->ed ==ed となる hestack の番号をとってくる
//
int find_henum_hestack( Sped *ed, int hen, Hestack *hs )
{
  for ( int j = 0; j < hen; ++j ) {
    if ( hs[j].he->ed == ed ) {
      return j;
    }
  }
  return SMDNULL;
}

//
// 三角形の辺ループ上の次の hevt を取ってくる
//
Hevt *next_hevt_triloop( Hevt *hevt, int hn, Hestack *hs )
{
  // hestack の次の hevt を返す
  int i = hevt->hsid;
  int j = hevt->hvid;
  Hestack *chs = &(hs[i]);
  if ( (i == hn - 1) && (j == chs->hevtn - 1) ) {
    return &(hs[0].hevt[0]);
  } else if ( j == chs->hevtn - 1 ) {
    return &(hs[i+1].hevt[0]);
  } else {
    return &(hs[i].hevt[j+1]);
  }
}

//
// hestack を使って面を分割する
//
void subdivide_ppdface_by_hestack( Spfc *fc, Hestack *hestack, Sppd *ppd,
				   int* n_newfc, SubdivFace* newfcall )
{
  for (int i = 0; i < fc->hen; ++i ) {
    for ( int j = 0; j < hestack[i].hevtn; ++j ) {
      Hevt *hevt = &(hestack[i].hevt[j]);
      for ( Hevted *hve = hevt->svted; hve != NULL; hve = hve->nxt ) {
	
	if ( isvalid_create_ppdface( hve, hevt ) == TRUE ) {
	  // create face
	  Spfc *newfc = create_ppdface( ppd );
	  newfc->bpso = ppd->spso;
	  newfc->bppt = ppd->sppt;
	  display("create fc %d\n", newfc->no );
	  
	  // newfcall への格納
	  if ( n_newfc != NULL )
	    {
	      newfcall[*n_newfc].fc = fc; ++(*n_newfc);
	    }

	  Hevted *chve = hve;
	  Hevt *chevt = hevt;
	  do {
	    chve->isVisited = TRUE;
	    
	    { // create halfedge begin

	      Sphe *newhe = create_ppdhalfedge( newfc );
	      newhe->vt = chevt->vt;
	      newhe->ed = chve->ed;

	      // mate の付けかえ
	      Sped *ed = newhe->ed;
	      if ( ed->sv == newhe->vt ) {
		ed->lhe = newhe;
		if ( ed->rhe != NULL ) {
		  newhe->mate = ed->rhe;
		  ed->rhe->mate = newhe;
		}
	      } else {
		ed->rhe = newhe;
		if ( ed->lhe != NULL ) {
		  newhe->mate = ed->lhe;
		  ed->lhe->mate = newhe;
		}
	      }

	      // vertex -> halfedge link
	      newhe->vt->sphe = newhe;

	      // edge の属性の修正 (EDGE_ONFACE -> EDGE_ORIGINAL)
	      if ( newhe->ed->type == EDGE_ONFACE )
		{
		  newhe->ed->type = EDGE_ORIGINAL;
		  newhe->ed->sp_fc = NULL;
		}
	    
	      display("\tcreate halfedge vt %d ed %d\n", 
		      newhe->vt->no, newhe->ed->no );
	      
	    } // create halfedge end
	    
	    Hevted *mate = chve->mate;
	    chevt = mate->hevt;
	    chve = mate->prv;
	  } while ( chevt != hevt );

	  // normal
	  calc_fnorm( newfc );
	}
      }
    }
  }
}

BOOL isvalid_create_ppdface( Hevted *hve, Hevt *hevt )
{
  Hevted *chve = hve;
  Hevt *chevt = hevt;
  do {
    if ( chve->isVisited == TRUE ) return FALSE;
    Hevted *mate = chve->mate;
    chevt = mate->hevt;
    if ( mate->prv == NULL ) return FALSE;
    chve = mate->prv;
  } while ( chevt != hevt );

  return TRUE;
}
  
/////////////////////////////////////////////////////////////////////
//
// Hevt Functions
//

void initialize_hevt( Hevt *hevt )
{
  hevt->vt = NULL;
  hevt->hsid = SMDNULL;
  hevt->hvid = SMDNULL;
//    hevt->isUsed = FALSE;
  hevt->nhve = 0;
  hevt->svted = NULL;
  hevt->evted = NULL;
  
}
  
//
// ソートして Hestack に入れる ( vt->type == VERTEX_ADDED のみ)
//
void insert_hevt_in_hestack( Spvt *vt, // 挿入する頂点
			     Sped *ed, // 切ったエッジ (切られたエッジではない)
			     Hestack *hs )
{
  double val = ( vt->sp_ed->sv == hs->he->vt ) ? vt->sp_val : 1.0 - vt->sp_val;
  int n = SMDNULL;
  for ( int i = 1; i < hs->hevtn; ++i ) {
    if ( hs->hevt[i].vt == NULL ) {
      // 追加
      n = i;
      break;
    } else {
      if ( hs->hevt[i].val > val ) {
	// 挿入
	n = i;
	// 一個ずつずらす
	for ( int j = hs->hevtn; j > i; --j ) {
	  //if ( j - 1 == 0 ) continue; // he->vt は動かさない
	  hs->hevt[j].vt = hs->hevt[j-1].vt;
	  hs->hevt[j].val = hs->hevt[j-1].val;
//	  hs->hevt[j].isUsed = hs->hevt[j-1].isUsed;
	}
	break;
      }
    }
  }
  if ( n == SMDNULL ) n = hs->hevtn;
//    display("----- n %d\n", n );
  assert( n != SMDNULL );
  hs->hevt[n].vt = vt;
  hs->hevt[n].val = val;
//  hs->hevt[n].isUsed = FALSE;

  ++(hs->hevtn);

//    display("(tmp)hevtn %d\n", hs->hevtn );
//    for ( i = 0; i < hs->hevtn; ++i ) {
//      display("\thevt hsid %d hvid %d vt %d val %g\n", 
//  	    hs->hevt[i].hsid,
//  	    hs->hevt[i].hvid,
//  	    hs->hevt[i].vt->no,
//  	    hs->hevt[i].val );
//    }
    
}

//
// 次の hevt を取ってくる
//
//  Hevt *next_hevt( Hevt *hevt, int hn, Hestack *hs )
//  {
//    // hevt->ed を持つ場合は，エッジのもう一方の頂点が，まだ
//    // なぞられていない (isUsed = FALSE) 場合にのみ，次の頂点となる
//    if ( hevt->ed != NULL ) {
//      Spvt *vt = (hevt->ed->sv != hevt->vt) ? hevt->ed->sv : hevt->ed->ev;
//      // vt を持つ hevt を見つける
//      Hevt *nhv = find_hevt_from_hestack( vt, hn, hs );
//      assert( nhv != NULL );
//      if ( nhv->isUsed == FALSE ) return nhv;
//    }

//    // hestack の次の hevt を返す
//    int i = hevt->hsid;
//    int j = hevt->hvid;
//    Hestack *chs = &(hs[i]);
//    if ((i == hn - 1) && (j == chs->hevtn - 1)) {
//      return &(hs[0].hevt[0]);
//    } else if (j == chs->hevtn - 1) {
//      return &(hs[i+1].hevt[0]);
//    } else {
//      return &(hs[i+1].hevt[j+1]);
//    }
//  }

//
// vt を持つ hevt を返す
//
Hevt *find_hevt_from_hestack( Spvt *vt, int hn, Hestack *hs )
{
  for ( int i = 0; i < hn; ++i ) {
    for ( int j = 0; j < hs[i].hevtn; ++j ) {
      if ( hs[i].hevt[j].vt == vt ) return &(hs[i].hevt[j]);
    }
  }
  return NULL;
}

//
// hestack の中でまだ isUsed = TRUE になっていない頂点を取ってくる
//
//  Hevt *hestack_unused_hevt( int hn, Hestack *hs )
//  {
//    for ( int i = 0; i < hn; ++i ) {
//      for ( int j = 0; j < hs[i].hevtn; ++j ) {
//        if ( hs[i].hevt[j].isUsed == FALSE ) return &(hs[i].hevt[j]);
//      }
//    }
//    return NULL;
//  }

/////////////////////////////////////////////////////////////////////
//
// Hevted Functions (双方向巡回リスト)
//

//
// Create Hevted
Hevted *create_hevted( Sped *ed, Hevt *hevt )
{
  Hevted *hve;

  if (ed == NULL) return NULL;
  
  hve = (Hevted *) malloc( sizeof(Hevted) );

  hve->nxt = NULL;
  if ( hevt->svted == NULL) { // first
    hve->prv    = NULL;
    hevt->evted = hevt->svted = hve;
  } else {
    hve->prv    = hevt->evted;
    hevt->evted = hevt->evted->nxt = hve;
  }
  
  hve->ed = ed;
  hve->hevt = hevt;
  hve->mate = NULL;
  hve->isVisited = FALSE;
  hve->val = 0.0;

  ++( hevt->nhve );
  
  return hve;
}

Hevted *find_hevted( Sped *ed, Hevt *hevt )
{
  for ( Hevted *hve = hevt->svted; hve != NULL; hve = hve->nxt ) {
    if ( hve->ed == ed ) return hve;
  }
  return NULL;
}

//
// 値 val を評価にして nhve を挿入
//
Hevted *insert_hevted( double val, Sped *ed, Hevt *hevt )
{
//    display("insert.\n");
  Hevted *nhve;
  if ( hevt->svted == NULL ) { // first
    nhve = create_hevted( ed, hevt );
    nhve->val = val;
    return nhve;
  }

  //
  // hve->val の値が val よりもおおきい hve を探す
  //
  Hevted *hve = hevt->svted;
  while ( (hve != NULL) && (val > hve->val) ) {
//      display("--- hve ed %d val %g\n", hve->ed->no, hve->val );
    hve = hve->nxt;
  }

  if ( hve == NULL ) { // last
    nhve = create_hevted( ed, hevt );
    nhve->val = val;
    return nhve;
  } 

  //
  // hve の直前に nhve を挿入する
  //
//    display("hve ed %d val %g\n", hve->ed->no, hve->val );
  nhve = (Hevted *) malloc( sizeof(Hevted) );
  nhve->prv = hve->prv;
  nhve->nxt = hve;
  if ( hve->prv != NULL ) hve->prv->nxt = nhve;
  hve->prv = nhve;
  if ( hevt->svted == hve ) hevt->svted = nhve;

  nhve->ed = ed;
  nhve->hevt = hevt;
  nhve->mate = NULL;
  nhve->val = val;
  nhve->isVisited = FALSE;
  
  ++( hevt->nhve );

  return nhve;
}

void free_halfedgevertexedge( Hevt *hevt )
{
  Hevted *hve, *nhve;
  for ( hve = hevt->svted; hve != NULL; hve = nhve ) {
    nhve = hve->nxt;
    free_hevted( hve, hevt );
  }
  hevt->nhve = 0;
  hevt->svted = hevt->evted = NULL;
}

void free_hevted( Hevted *hve, Hevt *hevt )
{
  if ( hve == NULL ) return;

  hve->ed = NULL;
  hve->hevt = NULL;

  // mate のリンクをはずす
  if ( hve->mate != NULL ) {
    hve->mate->mate = NULL;
    hve->mate = NULL;
  }
  
  if ( hevt->svted == hve ) {
    if ( (hevt->svted = hve->nxt) != NULL )
      hve->nxt->prv = NULL;
    else {
      hevt->evted = NULL;
    }
  } else if ( hevt->evted == hve ) {
    hve->prv->nxt = NULL;
    hevt->evted = hve->prv;
  } else {
    hve->prv->nxt = hve->nxt;
    hve->nxt->prv = hve->prv;
  }
  free( hve );
  --( hevt->nhve );
}

void hevted_makemate( int hen, Hestack *hs )
{
  for ( int i = 0; i < hen; ++i ) {
    for ( int j = 0; j < hs[i].hevtn; ++j ) {

      Hevt *hevt = &(hs[i].hevt[j]);
      for ( Hevted *hve = hevt->svted; hve != NULL; hve = hve->nxt ) {
	// もうすでに mate がある
	Sped *ed = hve->ed;
	if ( hve->mate == NULL ) {
	  // mate を探す
	  Spvt *pvt = another_vt( ed, hevt->vt );
	  Hevt *phevt = find_hevt_from_hestack( pvt, hen, hs );
	  Hevted *phve = find_hevted( ed, phevt );
	  assert( phve != NULL );
	  hve->mate = phve;
	  phve->mate = hve;
	}
      }
    }
  }
}
	
// SubdivFace 関数

void init_subdivface( SubdivFace* sfc )
{
  // オリジナル面
  sfc->fc = NULL;

  // 分割するためのエッジ
  sfc->en = 0;

  // 分割された面
//    sfc->fn = 0;
}
  
SubdivFace* find_subdivface( int fn, SubdivFace* sfcall, Spfc* fc )
{
  for ( int i = 0; i < fn; ++i )
    {
      if ( sfcall[i].fc == fc ) return &(sfcall[i]);
    }
  return NULL;
}

// SubdivEdge 関数

void init_subdivedge( SubdivEdge* sed )
{
  // オリジナルエッジ
  sed->ed = NULL;

  // 分割するための頂点
  sed->vn = 0;

  // 分割されたエッジ
//    sed->en = 0;
}
  
SubdivEdge *find_subdivedge( int en, SubdivEdge* sedall, Sped* ed )
{
  for ( int i = 0; i < en; ++i )
    {
      if ( sedall[i].ed == ed ) return &(sedall[i]);
    }
  return NULL;
}
