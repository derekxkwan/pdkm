#include "m_pd.h"
#include <stdlib.h>
#include <string.h>

//ported from SuperCollider's Scale
static t_class *scaledeg_class;

typedef struct _scaledeg {
	t_object x_obj;
	char scalename[10];
	int curscale[10];
	t_float offset;
	int ionian[9];
	int dorian[9];
	int phrygian[9];
	int lydian[9];
	int mixo[9];
	int aeolian[9];
	int locrian[9];
	int harminor[9];
	int melminor[9];
	int bartok[9];
	int neapminor[9];
	int neapmajor[9];
	int rominor[9];
	int superloc[9];
	int spanish[9];
	int enigmatic[9];
	int todi[9];
	int purvi[9];
	int marva[9];
	int bhairav[9];
	int ahirbhairav[9];
	int leadwhole[9];
	int lydianminor[9];
	int locrianmajor[9];
	int dim1[10];
	int dim2[10];
	int whole[8];
	int hexmajor7[8];
	int hexdorian[8];
	int hexphrygian[8];
	int hexsus[8];
	int hexmajor[8];
	int hexaeolian[8];
	int yu[7];
	int zhi[7];
	int jiao[7];
	int shang[7];
	int gong[7];
	int minorpent[7];
	int majorpent[7];
	int ritusen[7];
	int egyptian[7];
	int kumoi[7];
	int hirajoshi[7];
	int iwato[7];
	int chinese[7];
	int indian[7];
	int pelog[7];
	int prometheus[7];
	int partch_o1[8];
	int partch_o2[8];
	int partch_o3[8];
	int partch_o4[8];
	int partch_o5[8];
	int partch_o6[8];
	int partch_u1[8];
	int partch_u2[8];
	int partch_u3[8];
	int partch_u4[8];
	int partch_u5[8];
	int partch_u6[8];
	int scriabin[7];
	int ajam[9];
	int jiharkah[9];
	int shawqAfza[9];
	int sikah[9];
	int sikahDesc[9];
	int huzam[9];
	int iraq[9];
	int bastanikar[9];
	int mustar[9];
	int bayati[9];
	int karjighar[9];
	int husseini[9];
	int nahawand[9];
	int nahawandDesc[9];
	int farahfaza[9];
	int murassah[9];
	int ushaqMashri[9];
	int rast[9];
	int rastDesc[9];
	int suznak[9];
	int nairuz[9];
	int yakah[9];
	int yakahDesc[9];
	int mahur[9];
	int hijaz[9];
	int hijazDesc[9];
	int zanjaran[9];
	int zanjaran2[9];
	int saba[9];
	int zamzam[9];
	int kurd[9];
	int kijazKarKurd[9];
	int nawaAthar[9];
	int nikriz[9];
	int atharKurd[9];
	int aug1[8];
	int aug2[8];
} t_scaledeg;

static void scaledeg_float(t_scaledeg *x, t_float f){
	int returnval;
	int iptoctave;
	int input = (int) f;
	int curscale[10];
	memcpy(curscale, x->curscale, sizeof(x->curscale));
	int offset = (int) x->offset;
	offset = (offset >= 0)?offset:0;
	int numnotes = curscale[0];
	int octscale = curscale[1];
	iptoctave = (int)(input/numnotes);
	input %= numnotes;
	iptoctave *= octscale;
	input += 2;
	returnval = curscale[input] + offset + iptoctave;
	
	outlet_float(x->x_obj.ob_outlet, returnval);



}


static void scaledeg_pickscale(t_scaledeg *x, t_symbol *s){
	char sname[10];
	memcpy(sname, s->s_name, strlen(s->s_name) + 1);
	memcpy(x->scalename, sname, strlen(sname)+1);
	if((strcmp(sname, "major") == 0)||(strcmp(sname, "ionian") == 0)){
		memcpy(x->curscale, x->ionian, sizeof(x->ionian));
		post("major scale");
	}
	else if(strcmp(sname, "dorian") == 0){
		memcpy(x->curscale, x->dorian, sizeof(x->dorian));
		post("dorian scale");
	}
	else if(strcmp(sname, "phrygian") == 0){
		memcpy(x->curscale, x->phrygian, sizeof(x->phrygian));
		post("phrygian scale");
	}
	else if(strcmp(sname, "lydian") == 0){
		memcpy(x->curscale, x->lydian, sizeof(x->lydian));
		post("lydian scale");
	}
	else if(strcmp(sname, "mixo") == 0){
		memcpy(x->curscale, x->mixo, sizeof(x->mixo));
		post("mixo scale");
	}
	else if((strcmp(sname, "aeolian") == 0)||(strcmp(sname, "minor") == 0)){
		memcpy(x->curscale, x->aeolian, sizeof(x->aeolian));
		post("aeolian scale");
	}
	else if(strcmp(sname, "locrian") == 0){
		memcpy(x->curscale, x->locrian, sizeof(x->locrian));
		post("locrian scale");
	}
	else if(strcmp(sname, "harminor") == 0){
		memcpy(x->curscale, x->harminor, sizeof(x->harminor));
		post("harmonic minor scale");
	}
	else if(strcmp(sname, "melminor") == 0){
		memcpy(x->curscale, x->melminor, sizeof(x->melminor));
		post("melodic minor scale");
	}
	else if(strcmp(sname, "bartok") == 0){
		memcpy(x->curscale, x->bartok, sizeof(x->bartok));
		post("bartok scale");
	}
	else if(strcmp(sname, "neapminor") == 0){
		memcpy(x->curscale, x->neapminor, sizeof(x->neapminor));
		post("neapolitan minor scale");
	}
	else if(strcmp(sname, "neapmajor") == 0){
		memcpy(x->curscale, x->neapmajor, sizeof(x->neapmajor));
		post("neapolitan major scale");
	}
	else if(strcmp(sname, "rominor") == 0){
		memcpy(x->curscale, x->rominor, sizeof(x->rominor));
		post("romanian minor scale");
	}
	else if(strcmp(sname, "superlocrian") == 0){
		memcpy(x->curscale, x->superloc, sizeof(x->superloc));
		post("superlocrian scale");
	}
	else if(strcmp(sname, "spanish") == 0){
		memcpy(x->curscale, x->spanish, sizeof(x->spanish));
		post("spanish scale");
	}
	else if(strcmp(sname, "enigmatic") == 0){
		memcpy(x->curscale, x->enigmatic, sizeof(x->enigmatic));
		post("enigmatic scale");
	}
	else if(strcmp(sname, "todi") == 0){
		memcpy(x->curscale, x->todi, sizeof(x->todi));
		post("todi scale");
	}
	else if(strcmp(sname, "purvi") == 0){
		memcpy(x->curscale, x->purvi, sizeof(x->purvi));
		post("purvi scale");
	}
	else if(strcmp(sname, "marva") == 0){
		memcpy(x->curscale, x->marva, sizeof(x->marva));
		post("marva scale");
	}
	else if(strcmp(sname, "bhairav") == 0){
		memcpy(x->curscale, x->bhairav, sizeof(x->bhairav));
		post("bhairav scale");
	}
	else if(strcmp(sname, "ahirbhairav") == 0){
		memcpy(x->curscale, x->ahirbhairav, sizeof(x->ahirbhairav));
		post("ahirbhairav scale");
	}
	else if(strcmp(sname, "leadingwhole") == 0){
		memcpy(x->curscale, x->leadwhole, sizeof(x->leadwhole));
		post("leading whole-note scale");
	}
	else if(strcmp(sname, "lydianminor") == 0){
		memcpy(x->curscale, x->lydianminor, sizeof(x->lydianminor));
		post("lydian minor scale");
	}
	else if(strcmp(sname, "locrianmajor") == 0){
		memcpy(x->curscale, x->locrianmajor, sizeof(x->locrianmajor));
		post("locrian major scale");
	}
	else if(strcmp(sname, "dim1") == 0){
		memcpy(x->curscale, x->dim1, sizeof(x->dim1));
		post("diminished 1 scale");
	}
	else if(strcmp(sname, "dim2") == 0){
		memcpy(x->curscale, x->dim2, sizeof(x->dim2));
		post("diminished 2 scale");
	}
	else if(strcmp(sname, "whole") == 0){
		memcpy(x->curscale, x->whole, sizeof(x->whole));
		post("whole tone scale");
	}
	else if(strcmp(sname, "hexmajor7") == 0){
		memcpy(x->curscale, x->hexmajor7, sizeof(x->hexmajor7));
		post("hex major 7 scale");
	}
	else if(strcmp(sname, "hexdorian") == 0){
		memcpy(x->curscale, x->hexdorian, sizeof(x->hexdorian));
		post("hex dorian scale");
	}
	else if(strcmp(sname, "hexphrygian") == 0){
		memcpy(x->curscale, x->hexphrygian, sizeof(x->hexphrygian));
		post("hex phrygian scale");
	}
	else if(strcmp(sname, "hexsus") == 0){
		memcpy(x->curscale, x->hexsus, sizeof(x->hexsus));
		post("hex sus scale");
	}
	else if(strcmp(sname, "hexmajor") == 0){
		memcpy(x->curscale, x->hexmajor, sizeof(x->hexmajor));
		post("hex major scale");
	}
	else if(strcmp(sname, "hexaeolian") == 0){
		memcpy(x->curscale, x->hexaeolian, sizeof(x->hexaeolian));
		post("hex aeolian scale");
	}
	else if(strcmp(sname, "yu") == 0){
		memcpy(x->curscale, x->yu, sizeof(x->yu));
		post("yu scale");
	}
	else if(strcmp(sname, "zhi") == 0){
		memcpy(x->curscale, x->zhi, sizeof(x->zhi));
		post("zhi scale");
	}
	else if(strcmp(sname, "jiao") == 0){
		memcpy(x->curscale, x->jiao, sizeof(x->jiao));
		post("jiao scale");
	}
	else if(strcmp(sname, "shang") == 0){
		memcpy(x->curscale, x->shang, sizeof(x->shang));
		post("shang scale");
	}
	else if(strcmp(sname, "gong") == 0){
		memcpy(x->curscale, x->gong, sizeof(x->gong));
		post("gong scale");
	}
	else if(strcmp(sname, "minorpent") == 0){
		memcpy(x->curscale, x->minorpent, sizeof(x->minorpent));
		post("minorpent scale");
	}
	else if(strcmp(sname, "majorpent") == 0){
		memcpy(x->curscale, x->majorpent, sizeof(x->majorpent));
		post("majorpent scale");
	}
	else if(strcmp(sname, "ritusen") == 0){
		memcpy(x->curscale, x->ritusen, sizeof(x->ritusen));
		post("ritusen scale");
	}
	else if(strcmp(sname, "egyptian") == 0){
		memcpy(x->curscale, x->egyptian, sizeof(x->egyptian));
		post("egyptian scale");
	}
	else if(strcmp(sname, "kumoi") == 0){
		memcpy(x->curscale, x->kumoi, sizeof(x->kumoi));
		post("kumoi scale");
	}
	else if(strcmp(sname, "hirajoshi") == 0){
		memcpy(x->curscale, x->hirajoshi, sizeof(x->hirajoshi));
		post("hirajoshi scale");
	}
	else if(strcmp(sname, "iwato") == 0){
		memcpy(x->curscale, x->iwato, sizeof(x->iwato));
		post("iwato scale");
	}
	else if(strcmp(sname, "chinese") == 0){
		memcpy(x->curscale, x->chinese, sizeof(x->chinese));
		post("chinese scale");
	}
	else if(strcmp(sname, "indian") == 0){
		memcpy(x->curscale, x->indian, sizeof(x->indian));
		post("indian scale");
	}
	else if(strcmp(sname, "pelog") == 0){
		memcpy(x->curscale, x->pelog, sizeof(x->pelog));
		post("pelog scale");
	}
	else if(strcmp(sname, "prometheus") == 0){
		memcpy(x->curscale, x->prometheus, sizeof(x->prometheus));
		post("prometheus scale");
	}
	else if(strcmp(sname, "kumoi") == 0){
		memcpy(x->curscale, x->kumoi, sizeof(x->kumoi));
		post("kumoi scale");
	}
	else if(strcmp(sname, "partch_o1") == 0){
		memcpy(x->curscale, x->partch_o1, sizeof(x->partch_o1));
		post("partch_o1 scale");
	}
	else if(strcmp(sname, "partch_o2") == 0){
		memcpy(x->curscale, x->partch_o2, sizeof(x->partch_o2));
		post("partch_o2 scale");
	}
	else if(strcmp(sname, "partch_o3") == 0){
		memcpy(x->curscale, x->partch_o3, sizeof(x->partch_o3));
		post("partch_o3 scale");
	}
	else if(strcmp(sname, "partch_o4") == 0){
		memcpy(x->curscale, x->partch_o4, sizeof(x->partch_o4));
		post("partch_o4 scale");
	}
	else if(strcmp(sname, "partch_o5") == 0){
		memcpy(x->curscale, x->partch_o5, sizeof(x->partch_o5));
		post("partch_o5 scale");
	}
	else if(strcmp(sname, "partch_o6") == 0){
		memcpy(x->curscale, x->partch_o6, sizeof(x->partch_o6));
		post("partch_o6 scale");
	}
	else if(strcmp(sname, "partch_u1") == 0){
		memcpy(x->curscale, x->partch_u1, sizeof(x->partch_u1));
		post("partch_u1 scale");
	}
	else if(strcmp(sname, "partch_u2") == 0){
		memcpy(x->curscale, x->partch_u2, sizeof(x->partch_u2));
		post("partch_u2 scale");
	}
	else if(strcmp(sname, "partch_u3") == 0){
		memcpy(x->curscale, x->partch_u3, sizeof(x->partch_u3));
		post("partch_u3 scale");
	}
	else if(strcmp(sname, "partch_u4") == 0){
		memcpy(x->curscale, x->partch_u4, sizeof(x->partch_u4));
		post("partch_u4 scale");
	}
	else if(strcmp(sname, "partch_u5") == 0){
		memcpy(x->curscale, x->partch_u5, sizeof(x->partch_u5));
		post("partch_u5 scale");
	}
	else if(strcmp(sname, "partch_u6") == 0){
		memcpy(x->curscale, x->partch_u6, sizeof(x->partch_u6));
		post("partch_u6 scale");
	}
	else if(strcmp(sname, "ajam") == 0){
		memcpy(x->curscale, x->ajam, sizeof(x->ajam));
		post("ajam scale");
	}
	else if(strcmp(sname, "jiharkah") == 0){
		memcpy(x->curscale, x->jiharkah, sizeof(x->jiharkah));
		post("jiharkah scale");
	}
	else if(strcmp(sname, "shawqAfza") == 0){
		memcpy(x->curscale, x->shawqAfza, sizeof(x->shawqAfza));
		post("shawqAfza scale");
	}
	else if(strcmp(sname, "sikah") == 0){
		memcpy(x->curscale, x->sikah, sizeof(x->sikah));
		post("sikah scale");
	}
	else if(strcmp(sname, "sikahDesc") == 0){
		memcpy(x->curscale, x->sikahDesc, sizeof(x->sikahDesc));
		post("sikahDesc scale");
	}
	else if(strcmp(sname, "huzam") == 0){
		memcpy(x->curscale, x->huzam, sizeof(x->huzam));
		post("huzam scale");
	}
	else if(strcmp(sname, "iraq") == 0){
		memcpy(x->curscale, x->iraq, sizeof(x->iraq));
		post("iraq scale");
	}
	else if(strcmp(sname, "partch_u6") == 0){
		memcpy(x->curscale, x->partch_u6, sizeof(x->partch_u6));
		post("partch_u6 scale");
	}
	else if(strcmp(sname, "bastanikar") == 0){
		memcpy(x->curscale, x->bastanikar, sizeof(x->bastanikar));
		post("bastanikar scale");
	}
	else if(strcmp(sname, "mustar") == 0){
		memcpy(x->curscale, x->mustar, sizeof(x->mustar));
		post("mustar scale");
	}
	else if(strcmp(sname, "bayati") == 0){
		memcpy(x->curscale, x->bayati, sizeof(x->bayati));
		post("bayati scale");
	}
	else if(strcmp(sname, "karjighar") == 0){
		memcpy(x->curscale, x->karjighar, sizeof(x->karjighar));
		post("karjighar scale");
	}
	else if(strcmp(sname, "husseini") == 0){
		memcpy(x->curscale, x->husseini, sizeof(x->husseini));
		post("husseini scale");
	}
	else if(strcmp(sname, "nahawand") == 0){
		memcpy(x->curscale, x->nahawand, sizeof(x->nahawand));
		post("nahawand scale");
	}
	else if(strcmp(sname, "nahawandDesc") == 0){
		memcpy(x->curscale, x->nahawandDesc, sizeof(x->nahawandDesc));
		post("nahawandDesc scale");
	}
	else if(strcmp(sname, "farahfaza") == 0){
		memcpy(x->curscale, x->farahfaza, sizeof(x->farahfaza));
		post("farahfaza scale");
	}
	else if(strcmp(sname, "murassah") == 0){
		memcpy(x->curscale, x->murassah, sizeof(x->murassah));
		post("murassah scale");
	}
	else if(strcmp(sname, "ushaqMashri") == 0){
		memcpy(x->curscale, x->ushaqMashri, sizeof(x->ushaqMashri));
		post("ushaqMashri scale");
	}
	else if(strcmp(sname, "rast") == 0){
		memcpy(x->curscale, x->rast, sizeof(x->rast));
		post("rast scale");
	}
	else if(strcmp(sname, "rastDesc") == 0){
		memcpy(x->curscale, x->rastDesc, sizeof(x->rastDesc));
		post("rastDesc scale");
	}
	else if(strcmp(sname, "suznak") == 0){
		memcpy(x->curscale, x->suznak, sizeof(x->suznak));
		post("suznak scale");
	}
	else if(strcmp(sname, "nairuz") == 0){
		memcpy(x->curscale, x->nairuz, sizeof(x->nairuz));
		post("nairuz scale");
	}
	else if(strcmp(sname, "yakah") == 0){
		memcpy(x->curscale, x->yakah, sizeof(x->yakah));
		post("yakah scale");
	}
	else if(strcmp(sname, "yakahDesc") == 0){
		memcpy(x->curscale, x->yakahDesc, sizeof(x->yakahDesc));
		post("yakahDesc scale");
	}
	else if(strcmp(sname, "mahur") == 0){
		memcpy(x->curscale, x->mahur, sizeof(x->mahur));
		post("mahur scale");
	}
	else if(strcmp(sname, "hijaz") == 0){
		memcpy(x->curscale, x->hijaz, sizeof(x->hijaz));
		post("hijaz scale");
	}
	else if(strcmp(sname, "hijazDesc") == 0){
		memcpy(x->curscale, x->hijazDesc, sizeof(x->hijazDesc));
		post("hijazDesc scale");
	}
	else if(strcmp(sname, "zanjaran") == 0){
		memcpy(x->curscale, x->zanjaran, sizeof(x->zanjaran));
		post("zanjaran scale");
	}
	else if(strcmp(sname, "zanjaran2") == 0){
		memcpy(x->curscale, x->zanjaran2, sizeof(x->zanjaran2));
		post("zanjaran2 scale");
	}
	else if(strcmp(sname, "saba") == 0){
		memcpy(x->curscale, x->saba, sizeof(x->saba));
		post("saba scale");
	}
	else if(strcmp(sname, "zamzam") == 0){
		memcpy(x->curscale, x->zamzam, sizeof(x->zamzam));
		post("zamzam scale");
	}
	else if(strcmp(sname, "kurd") == 0){
		memcpy(x->curscale, x->kurd, sizeof(x->kurd));
		post("kurd scale");
	}
	else if(strcmp(sname, "kijazKarKurd") == 0){
		memcpy(x->curscale, x->kijazKarKurd, sizeof(x->kijazKarKurd));
		post("kijazKarKurd scale");
	}
	else if(strcmp(sname, "nawaAthar") == 0){
		memcpy(x->curscale, x->nawaAthar, sizeof(x->nawaAthar));
		post("nawaAthar scale");
	}
	else if(strcmp(sname, "nikriz") == 0){
		memcpy(x->curscale, x->nikriz, sizeof(x->nikriz));
		post("nikriz scale");
	}
	else if(strcmp(sname, "atharKurd") == 0){
		memcpy(x->curscale, x->atharKurd, sizeof(x->atharKurd));
		post("atharKurd scale");
	}
	else if(strcmp(sname, "aug1") == 0){
		memcpy(x->curscale, x->aug1, sizeof(x->aug1));
		post("aug1 scale");
	}
	else if(strcmp(sname, "aug2") == 0){
		memcpy(x->curscale, x->aug2, sizeof(x->aug2));
		post("aug2 scale");
	};

}

static void *scaledeg_new(t_symbol *s, t_floatarg f){
	t_scaledeg *x = (t_scaledeg *)pd_new(scaledeg_class);
	char sname[10];
	int	ionian[9] = {7,12,0,2,4,5,7,9,11};
	int	dorian[9] = {7,12,0,2,3,5,7,9,10};
	int	phrygian[9] = {7,12,0,1,3,5,7,8,10};
	int	lydian[9] = {7,12,0,2,4,6,7,9,11};
	int	mixo[9] = {7,12,0,2,4,5,7,9,10};
	int	aeolian[9] = {7,12,0,2,3,5,7,8,10};
	int	locrian[9] = {7,12,0,1,3,5,6,8,10};
	int	harminor[9] = {7,12,0,2,3,5,7,8,11};
	int	melminor[9] = {7,12,0,2,4,5,7,8,10};
	int	bartok[9] = {7,12,0,2,4,5,7,8,10};
	int	neapminor[9] = {7,12,0,1,3,5,7,8,11};
	int	neapmajor[9] = {7,12,0,1,3,5,7,9,11};
	int	rominor[9] = {7,12,0,2,3,6,7,9,10};
	int	superloc[9] = {7,12,0,1,3,4,6,8,10};
	int	spanish[9] = {7,12,0,1,4,5,7,8,10};
	int	enigmatic[9] = {7,12,0,1,4,6,8,10,11};
	int	todi[9] = {7,12,0,1,3,6,7,8,11};
	int	purvi[9] = {7,12,0,1,4,6,7,8,11};
	int	marva[9] = {7,12,0,1,4,6,7,9,11};
	int	bhairav[9] = {7,12,0,1,4,5,7,8,11};
	int	ahirbhairav[9] = {7,12,0,1,4,5,7,8,11};
	int	leadwhole[9] = {7,12,0,2,4,6,8,10,11};
	int	lydianminor[9] = {7,12,0,2,4,6,7,8,10};
	int	locrianmajor[9] = {7,12,0,2,4,5,6,8,10};
	int	dim1[10] = {8,12,0,1,3,4,6,7,9,10};
	int	dim2[10] = {8,12,0,2,3,5,6,8,9,11};
	int	whole[8] = {6,12,0,2,4,6,8,10};
	int	hexmajor7[8] = {6,12,0,2,4,7,9,11};
	int	hexdorian[8] = {6,12,0,2,3,5,7,10};
	int	hexphrygian[8] = {6,12,0,1,3,5,8,10};
	int	hexsus[8] = {6,12,0,2,5,7,9,10};
	int	hexmajor[8] = {6,12,0,2,4,5,7,9};
	int	hexaeolian[8] = {6,12,0,3,5,7,8,10};
	int yu[7] = {5,12,0,3,5,7,10};
	int zhi[7] = {5,12,0,2,5,7,9};
	int jiao[7] = {5,12,0,3,5,8,10};
	int shang[7] = {5,12,0,2,5,7,10};
	int gong[7] = {5,12,0,2,4,7,9};
	int minorpent[7] = {5,12,0,3,5,7,10};
	int majorpent[7] = {5,12,0,2,4,7,9};
	int ritusen[7] = {5,12,0,2,5,7,9};
	int egyptian[7] = {5,12,0,2,5,7,10};
	int kumoi[7] = {5,12,0,2,3,7,9};
	int hirajoshi[7] = {5,12,0,2,3,7,8};
	int iwato[7] = {5,12,0,1,5,6,10};
	int chinese[7] = {5,12,0,4,6,7,11};
	int indian[7] = {5,12,0,4,5,7,10};
	int pelog[7] = {5,12,0,1,3,7,8};
	int prometheus[7] = {5,12,0,2,4,6,11};
	int scriabin[7] = {5,12,0,1,4,7,9};
	int partch_o1[8] = {6,43,0,8,14,20,25,34};
	int partch_o2[8] = {6,43,0,7,13,18,27,35};
	int partch_o3[8] = {6,43,0,6,12,21,29,36};
	int partch_o4[8] = {6,43,0,5,15,23,30,37};
	int partch_o5[8] = {6,43,0,10,18,25,31,38};
	int partch_o6[8] = {6,43,0,9,16,22,28,33};
	int partch_u1[8] = {6,43,0,9,18,23,29,35};
	int partch_u2[8] = {6,43,0,8,16,25,30,36};
	int partch_u3[8] = {6,43,0,7,14,22,31,37};
	int partch_u4[8] = {6,43,0,6,13,20,28,38};
	int partch_u5[8] = {6,43,0,5,12,18,25,33};
	int partch_u6[8] = {6,43,0,10,15,21,27,34};
	int ajam[9] = {7,24,0,4,8,10,14,18,22};
	int jiharkah[9] = {7,24,0,4,8,10,14,18,21};
	int shawqAfza[9] = {7,24,0,4,8,10,14,16,22};
	int sikah[9] = {7,24,0,3,7,11,14,17,21};
	int sikahDesc[9] = {7,24,0,3,7,11,13,17,21};
	int huzam[9] = {7,24,0,3,7,9,15,17,21};
	int iraq[9] = {7,24,0,3,7,10,13,17,21};
	int bastanikar[9] = {7,24,0,3,7,10,13,15,21};
	int mustar[9] = {7,24,0,5,7,11,13,17,21};
	int bayati[9] = {7,24,0,3,6,10,14,16,20};
	int karjighar[9] = {7,24,0,3,6,10,12,18,20};
	int husseini[9] = {7,24,0,3,6,10,14,17,21};
	int nahawand[9] = {7,24,0,4,6,10,14,16,22};
	int nahawandDesc[9] = {7,24,0,4,6,10,14,16,20};
	int farahfaza[9] = {7,24,0,4,6,10,14,16,20};
	int murassah[9] = {7,24,0,4,6,10,12,18,20};
	int ushaqMashri[9] = {7,24,0,4,6,10,14,17,21};
	int rast[9] = {7,24,0,4,7,10,14,18,21};
	int rastDesc[9] = {7,24,0,4,7,10,14,18,20};
	int suznak[9] = {7,24,0,4,7,10,14,16,22};
	int nairuz[9] = {7,24,0,4,7,10,14,17,20};
	int yakah[9] = {7,24,0,4,7,10,14,18,21};
	int yakahDesc[9] = {7,24,0,4,7,10,14,18,20};
	int mahur[9] = {7,24,0,4,7,10,14,18,22};
	int hijaz[9] = {7,24,0,2,8,10,14,17,20};
	int hijazDesc[9] = {7,24,0,2,8,10,14,16,20};
	int zanjaran[9] = {7,24,0,2,8,10,14,18,20};
	int zanjaran2[9] = {7,24,0,2,8,10,14,16,22};
	int saba[9] = {7,24,0,3,6,8,12,16,20};
	int zamzam[9] = {7,24,0,2,6,8,14,16,20};
	int kurd[9] = {7,24,0,2,6,10,14,16,20};
	int kijazKarKurd[9] = {7,24,0,2,8,10,14,16,22};
	int nawaAthar[9] = {7,24,0,4,6,12,14,16,22};
	int nikriz[9] = {7,24,0,4,6,12,14,18,20};
	int atharKurd[9] = {7,24,0,2,6,12,14,16,22};
	int aug1[8] = {6,12,0,3,4,7,8,11};
	int aug2[8] = {6,12,0,1,4,5,8,9};
	memcpy(x->ionian,ionian,sizeof(ionian));
	memcpy(x->dorian,dorian,sizeof(dorian));
	memcpy(x->phrygian,phrygian,sizeof(phrygian));
	memcpy(x->lydian,lydian,sizeof(lydian));
	memcpy(x->mixo,mixo,sizeof(mixo));
	memcpy(x->aeolian,aeolian,sizeof(aeolian));
	memcpy(x->locrian,locrian,sizeof(locrian));
	memcpy(x->harminor,harminor,sizeof(harminor));
	memcpy(x->melminor,melminor,sizeof(melminor));
	memcpy(x->bartok,bartok,sizeof(bartok));
	memcpy(x->neapminor,neapminor,sizeof(neapminor));
	memcpy(x->neapmajor,neapmajor,sizeof(neapmajor));
	memcpy(x->rominor,rominor,sizeof(rominor));
	memcpy(x->superloc,superloc,sizeof(superloc));
	memcpy(x->spanish,spanish,sizeof(spanish));
	memcpy(x->enigmatic,enigmatic,sizeof(enigmatic));
	memcpy(x->todi,todi,sizeof(todi));
	memcpy(x->purvi,purvi,sizeof(purvi));
	memcpy(x->marva,marva,sizeof(marva));
	memcpy(x->bhairav,bhairav,sizeof(bhairav));
	memcpy(x->ahirbhairav,ahirbhairav,sizeof(ahirbhairav));
	memcpy(x->leadwhole,leadwhole,sizeof(leadwhole));
	memcpy(x->lydianminor,lydianminor,sizeof(lydianminor));
	memcpy(x->locrianmajor,locrianmajor,sizeof(locrianmajor));
	memcpy(x->dim1,dim1,sizeof(dim1));
	memcpy(x->dim2,dim2,sizeof(dim2));
	memcpy(x->whole,whole,sizeof(whole));
	memcpy(x->hexmajor7,hexmajor7,sizeof(hexmajor7));
	memcpy(x->hexdorian,hexdorian,sizeof(hexdorian));
	memcpy(x->hexphrygian,hexphrygian,sizeof(hexphrygian));
	memcpy(x->hexsus,hexsus,sizeof(hexsus));
	memcpy(x->hexmajor,hexmajor,sizeof(hexmajor));
	memcpy(x->hexaeolian,hexaeolian,sizeof(hexaeolian));
	memcpy(x->yu,yu,sizeof(yu));
	memcpy(x->zhi,zhi,sizeof(zhi));
	memcpy(x->jiao,jiao,sizeof(jiao));
	memcpy(x->shang,shang,sizeof(shang));
	memcpy(x->gong,gong,sizeof(gong));
	memcpy(x->minorpent,minorpent,sizeof(minorpent));
	memcpy(x->majorpent,majorpent,sizeof(majorpent));
	memcpy(x->ritusen,ritusen,sizeof(ritusen));
	memcpy(x->egyptian,egyptian,sizeof(egyptian));
	memcpy(x->kumoi,kumoi,sizeof(kumoi));
	memcpy(x->hirajoshi,hirajoshi,sizeof(hirajoshi));
	memcpy(x->iwato,iwato,sizeof(iwato));
	memcpy(x->chinese,chinese,sizeof(chinese));
	memcpy(x->indian,indian,sizeof(indian));
	memcpy(x->pelog,pelog,sizeof(pelog));
	memcpy(x->prometheus,prometheus,sizeof(prometheus));
	memcpy(x->scriabin,scriabin,sizeof(scriabin));
	memcpy(x->partch_o1,partch_o1,sizeof(partch_o1));
	memcpy(x->partch_o2,partch_o2,sizeof(partch_o2));
	memcpy(x->partch_o3,partch_o3,sizeof(partch_o3));
	memcpy(x->partch_o4,partch_o4,sizeof(partch_o4));
	memcpy(x->partch_o5,partch_o5,sizeof(partch_o5));
	memcpy(x->partch_o6,partch_o6,sizeof(partch_o6));
	memcpy(x->partch_u1,partch_u1,sizeof(partch_u1));
	memcpy(x->partch_u2,partch_u2,sizeof(partch_u2));
	memcpy(x->partch_u3,partch_u3,sizeof(partch_u3));
	memcpy(x->partch_u4,partch_u4,sizeof(partch_u4));
	memcpy(x->partch_u5,partch_u5,sizeof(partch_u5));
	memcpy(x->partch_u6,partch_u6,sizeof(partch_u6));
	memcpy(x->ajam,ajam,sizeof(ajam));
	memcpy(x->jiharkah,jiharkah,sizeof(jiharkah));
	memcpy(x->shawqAfza,shawqAfza,sizeof(shawqAfza));
	memcpy(x->sikah,sikah,sizeof(sikah));
	memcpy(x->sikahDesc,sikahDesc,sizeof(sikahDesc));
	memcpy(x->huzam,huzam,sizeof(huzam));
	memcpy(x->iraq,iraq,sizeof(iraq));
	memcpy(x->bastanikar,bastanikar,sizeof(bastanikar));
	memcpy(x->mustar,mustar,sizeof(mustar));
	memcpy(x->bayati,bayati,sizeof(bayati));
	memcpy(x->karjighar,karjighar,sizeof(karjighar));
	memcpy(x->husseini,husseini,sizeof(husseini));
	memcpy(x->nahawand,nahawand,sizeof(nahawand));
	memcpy(x->nahawandDesc,nahawandDesc,sizeof(nahawandDesc));
	memcpy(x->farahfaza,farahfaza,sizeof(farahfaza));
	memcpy(x->murassah,murassah,sizeof(murassah));
	memcpy(x->ushaqMashri,ushaqMashri,sizeof(ushaqMashri));
	memcpy(x->rast,rast,sizeof(rast));
	memcpy(x->rastDesc,rastDesc,sizeof(rastDesc));
	memcpy(x->suznak,suznak,sizeof(suznak));
	memcpy(x->nairuz,nairuz,sizeof(nairuz));
	memcpy(x->yakah,yakah,sizeof(yakah));
	memcpy(x->yakahDesc,yakahDesc,sizeof(yakahDesc));
	memcpy(x->mahur,mahur,sizeof(mahur));
	memcpy(x->hijaz,hijaz,sizeof(hijaz));
	memcpy(x->hijazDesc,hijazDesc,sizeof(hijazDesc));
	memcpy(x->zanjaran,zanjaran,sizeof(zanjaran));
	memcpy(x->zanjaran2,zanjaran2,sizeof(zanjaran2));
	memcpy(x->saba,saba,sizeof(saba));
	memcpy(x->zamzam,zamzam,sizeof(zamzam));
	memcpy(x->kurd,kurd,sizeof(kurd));
	memcpy(x->kijazKarKurd,kijazKarKurd,sizeof(kijazKarKurd));
	memcpy(x->nawaAthar,nawaAthar,sizeof(nawaAthar));
	memcpy(x->nikriz,nikriz,sizeof(nikriz));
	memcpy(x->atharKurd,atharKurd,sizeof(atharKurd));
	memcpy(x->aug1,aug1,sizeof(aug1));
	memcpy(x->aug2,aug2,sizeof(aug2));
	

	memcpy(sname, s->s_name, strlen(s->s_name)+1);
	memcpy(x->scalename, s->s_name, sizeof(sname));
	scaledeg_pickscale(x, s);
	x->offset = f;
	//symbolinlet_new(&x->x_obj, &x->scalesym);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_symbol, gensym("pickscale"));
	floatinlet_new(&x->x_obj, &x->offset);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

void scaledeg_setup(void){
	scaledeg_class = class_new(gensym("scaledeg"), (t_newmethod)scaledeg_new, 0,
			sizeof(t_scaledeg), 0, A_DEFSYMBOL, A_DEFFLOAT, 0);
	class_addfloat(scaledeg_class, (t_method)scaledeg_float);
	class_addmethod(scaledeg_class, (t_method)scaledeg_pickscale, gensym("pickscale"), A_SYMBOL, 0);
}
