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
	
	if((strcmp(s->s_name, "major") == 0)||(strcmp(s->s_name, "ionian") == 0)){
		memcpy(x->curscale, ionian, sizeof(x->ionian));
		post("major scale");
	}
	else if(strcmp(s->s_name, "dorian") == 0){
		memcpy(x->curscale, dorian, sizeof(x->dorian));
		post("dorian scale");
	}
	else if(strcmp(s->s_name, "phrygian") == 0){
		memcpy(x->curscale, phrygian, sizeof(x->phrygian));
		post("phrygian scale");
	}
	else if(strcmp(s->s_name, "lydian") == 0){
		memcpy(x->curscale, lydian, sizeof(x->lydian));
		post("lydian scale");
	}
	else if(strcmp(s->s_name, "mixo") == 0){
		memcpy(x->curscale, mixo, sizeof(x->mixo));
		post("mixo scale");
	}
	else if((strcmp(s->s_name, "aeolian") == 0)||(strcmp(s->s_name, "minor") == 0)){
		memcpy(x->curscale, aeolian, sizeof(x->aeolian));
		post("aeolian scale");
	}
	else if(strcmp(s->s_name, "locrian") == 0){
		memcpy(x->curscale, locrian, sizeof(x->locrian));
		post("locrian scale");
	}
	else if(strcmp(s->s_name, "harminor") == 0){
		memcpy(x->curscale, harminor, sizeof(x->harminor));
		post("harmonic minor scale");
	}
	else if(strcmp(s->s_name, "melminor") == 0){
		memcpy(x->curscale, melminor, sizeof(x->melminor));
		post("melodic minor scale");
	}
	else if(strcmp(s->s_name, "bartok") == 0){
		memcpy(x->curscale, bartok, sizeof(x->bartok));
		post("bartok scale");
	}
	else if(strcmp(s->s_name, "neapminor") == 0){
		memcpy(x->curscale, neapminor, sizeof(x->neapminor));
		post("neapolitan minor scale");
	}
	else if(strcmp(s->s_name, "neapmajor") == 0){
		memcpy(x->curscale, neapmajor, sizeof(x->neapmajor));
		post("neapolitan major scale");
	}
	else if(strcmp(s->s_name, "rominor") == 0){
		memcpy(x->curscale, rominor, sizeof(x->rominor));
		post("romanian minor scale");
	}
	else if(strcmp(s->s_name, "superlocrian") == 0){
		memcpy(x->curscale, superloc, sizeof(x->superloc));
		post("superlocrian scale");
	}
	else if(strcmp(s->s_name, "spanish") == 0){
		memcpy(x->curscale, spanish, sizeof(x->spanish));
		post("spanish scale");
	}
	else if(strcmp(s->s_name, "enigmatic") == 0){
		memcpy(x->curscale, enigmatic, sizeof(x->enigmatic));
		post("enigmatic scale");
	}
	else if(strcmp(s->s_name, "todi") == 0){
		memcpy(x->curscale, todi, sizeof(x->todi));
		post("todi scale");
	}
	else if(strcmp(s->s_name, "purvi") == 0){
		memcpy(x->curscale, purvi, sizeof(x->purvi));
		post("purvi scale");
	}
	else if(strcmp(s->s_name, "marva") == 0){
		memcpy(x->curscale, marva, sizeof(x->marva));
		post("marva scale");
	}
	else if(strcmp(s->s_name, "bhairav") == 0){
		memcpy(x->curscale, bhairav, sizeof(x->bhairav));
		post("bhairav scale");
	}
	else if(strcmp(s->s_name, "ahirbhairav") == 0){
		memcpy(x->curscale, ahirbhairav, sizeof(x->ahirbhairav));
		post("ahirbhairav scale");
	}
	else if(strcmp(s->s_name, "leadingwhole") == 0){
		memcpy(x->curscale, leadwhole, sizeof(x->leadwhole));
		post("leading wholenote scale");
	}
	else if(strcmp(s->s_name, "lydianminor") == 0){
		memcpy(x->curscale, lydianminor, sizeof(x->lydianminor));
		post("lydian minor scale");
	}
	else if(strcmp(s->s_name, "locrianmajor") == 0){
		memcpy(x->curscale, locrianmajor, sizeof(x->locrianmajor));
		post("locrian major scale");
	}
	else if(strcmp(s->s_name, "dim1") == 0){
		memcpy(x->curscale, dim1, sizeof(x->dim1));
		post("diminished 1 scale");
	}
	else if(strcmp(s->s_name, "dim2") == 0){
		memcpy(x->curscale, dim2, sizeof(x->dim2));
		post("diminished 2 scale");
	}
	else if(strcmp(s->s_name, "whole") == 0){
		memcpy(x->curscale, whole, sizeof(x->whole));
		post("whole tone scale");
	}
	else if(strcmp(s->s_name, "hexmajor7") == 0){
		memcpy(x->curscale, hexmajor7, sizeof(x->hexmajor7));
		post("hex major 7 scale");
	}
	else if(strcmp(s->s_name, "hexdorian") == 0){
		memcpy(x->curscale, hexdorian, sizeof(x->hexdorian));
		post("hex dorian scale");
	}
	else if(strcmp(s->s_name, "hexphrygian") == 0){
		memcpy(x->curscale, hexphrygian, sizeof(x->hexphrygian));
		post("hex phrygian scale");
	}
	else if(strcmp(s->s_name, "hexsus") == 0){
		memcpy(x->curscale, hexsus, sizeof(x->hexsus));
		post("hex sus scale");
	}
	else if(strcmp(s->s_name, "hexmajor") == 0){
		memcpy(x->curscale, hexmajor, sizeof(x->hexmajor));
		post("hex major scale");
	}
	else if(strcmp(s->s_name, "hexaeolian") == 0){
		memcpy(x->curscale, hexaeolian, sizeof(x->hexaeolian));
		post("hex aeolian scale");
	}
	else if(strcmp(s->s_name, "yu") == 0){
		memcpy(x->curscale, yu, sizeof(x->yu));
		post("yu scale");
	}
	else if(strcmp(s->s_name, "zhi") == 0){
		memcpy(x->curscale, zhi, sizeof(x->zhi));
		post("zhi scale");
	}
	else if(strcmp(s->s_name, "jiao") == 0){
		memcpy(x->curscale, jiao, sizeof(x->jiao));
		post("jiao scale");
	}
	else if(strcmp(s->s_name, "shang") == 0){
		memcpy(x->curscale, shang, sizeof(x->shang));
		post("shang scale");
	}
	else if(strcmp(s->s_name, "gong") == 0){
		memcpy(x->curscale, gong, sizeof(x->gong));
		post("gong scale");
	}
	else if(strcmp(s->s_name, "minorpent") == 0){
		memcpy(x->curscale, minorpent, sizeof(x->minorpent));
		post("minorpent scale");
	}
	else if(strcmp(s->s_name, "majorpent") == 0){
		memcpy(x->curscale, majorpent, sizeof(x->majorpent));
		post("majorpent scale");
	}
	else if(strcmp(s->s_name, "ritusen") == 0){
		memcpy(x->curscale, ritusen, sizeof(x->ritusen));
		post("ritusen scale");
	}
	else if(strcmp(s->s_name, "egyptian") == 0){
		memcpy(x->curscale, egyptian, sizeof(x->egyptian));
		post("egyptian scale");
	}
	else if(strcmp(s->s_name, "kumoi") == 0){
		memcpy(x->curscale, kumoi, sizeof(x->kumoi));
		post("kumoi scale");
	}
	else if(strcmp(s->s_name, "hirajoshi") == 0){
		memcpy(x->curscale, hirajoshi, sizeof(x->hirajoshi));
		post("hirajoshi scale");
	}
	else if(strcmp(s->s_name, "iwato") == 0){
		memcpy(x->curscale, iwato, sizeof(x->iwato));
		post("iwato scale");
	}
	else if(strcmp(s->s_name, "chinese") == 0){
		memcpy(x->curscale, chinese, sizeof(x->chinese));
		post("chinese scale");
	}
	else if(strcmp(s->s_name, "indian") == 0){
		memcpy(x->curscale, indian, sizeof(x->indian));
		post("indian scale");
	}
	else if(strcmp(s->s_name, "pelog") == 0){
		memcpy(x->curscale, pelog, sizeof(x->pelog));
		post("pelog scale");
	}
	else if(strcmp(s->s_name, "prometheus") == 0){
		memcpy(x->curscale, prometheus, sizeof(x->prometheus));
		post("prometheus scale");
	}
	else if(strcmp(s->s_name, "scriabin") == 0){
		memcpy(x->curscale, scriabin, sizeof(x->kumoi));
		post("scriabin scale");
	}
	else if(strcmp(s->s_name, "partch_o1") == 0){
		memcpy(x->curscale, partch_o1, sizeof(x->partch_o1));
		post("partch_o1 scale");
	}
	else if(strcmp(s->s_name, "partch_o2") == 0){
		memcpy(x->curscale, partch_o2, sizeof(x->partch_o2));
		post("partch_o2 scale");
	}
	else if(strcmp(s->s_name, "partch_o3") == 0){
		memcpy(x->curscale, partch_o3, sizeof(x->partch_o3));
		post("partch_o3 scale");
	}
	else if(strcmp(s->s_name, "partch_o4") == 0){
		memcpy(x->curscale, partch_o4, sizeof(x->partch_o4));
		post("partch_o4 scale");
	}
	else if(strcmp(s->s_name, "partch_o5") == 0){
		memcpy(x->curscale, partch_o5, sizeof(x->partch_o5));
		post("partch_o5 scale");
	}
	else if(strcmp(s->s_name, "partch_o6") == 0){
		memcpy(x->curscale, partch_o6, sizeof(x->partch_o6));
		post("partch_o6 scale");
	}
	else if(strcmp(s->s_name, "partch_u1") == 0){
		memcpy(x->curscale, partch_u1, sizeof(x->partch_u1));
		post("partch_u1 scale");
	}
	else if(strcmp(s->s_name, "partch_u2") == 0){
		memcpy(x->curscale, partch_u2, sizeof(x->partch_u2));
		post("partch_u2 scale");
	}
	else if(strcmp(s->s_name, "partch_u3") == 0){
		memcpy(x->curscale, partch_u3, sizeof(x->partch_u3));
		post("partch_u3 scale");
	}
	else if(strcmp(s->s_name, "partch_u4") == 0){
		memcpy(x->curscale, partch_u4, sizeof(x->partch_u4));
		post("partch_u4 scale");
	}
	else if(strcmp(s->s_name, "partch_u5") == 0){
		memcpy(x->curscale, partch_u5, sizeof(x->partch_u5));
		post("partch_u5 scale");
	}
	else if(strcmp(s->s_name, "partch_u6") == 0){
		memcpy(x->curscale, partch_u6, sizeof(x->partch_u6));
		post("partch_u6 scale");
	}
	else if(strcmp(s->s_name, "ajam") == 0){
		memcpy(x->curscale, ajam, sizeof(x->ajam));
		post("ajam scale");
	}
	else if(strcmp(s->s_name, "jiharkah") == 0){
		memcpy(x->curscale, jiharkah, sizeof(x->jiharkah));
		post("jiharkah scale");
	}
	else if(strcmp(s->s_name, "shawqAfza") == 0){
		memcpy(x->curscale, shawqAfza, sizeof(x->shawqAfza));
		post("shawqAfza scale");
	}
	else if(strcmp(s->s_name, "sikah") == 0){
		memcpy(x->curscale, sikah, sizeof(x->sikah));
		post("sikah scale");
	}
	else if(strcmp(s->s_name, "sikahDesc") == 0){
		memcpy(x->curscale, sikahDesc, sizeof(x->sikahDesc));
		post("sikahDesc scale");
	}
	else if(strcmp(s->s_name, "huzam") == 0){
		memcpy(x->curscale, huzam, sizeof(x->huzam));
		post("huzam scale");
	}
	else if(strcmp(s->s_name, "iraq") == 0){
		memcpy(x->curscale, iraq, sizeof(x->iraq));
		post("iraq scale");
	}
	else if(strcmp(s->s_name, "partch_u6") == 0){
		memcpy(x->curscale, partch_u6, sizeof(x->partch_u6));
		post("partch_u6 scale");
	}
	else if(strcmp(s->s_name, "bastanikar") == 0){
		memcpy(x->curscale, bastanikar, sizeof(x->bastanikar));
		post("bastanikar scale");
	}
	else if(strcmp(s->s_name, "mustar") == 0){
		memcpy(x->curscale, mustar, sizeof(x->mustar));
		post("mustar scale");
	}
	else if(strcmp(s->s_name, "bayati") == 0){
		memcpy(x->curscale, bayati, sizeof(x->bayati));
		post("bayati scale");
	}
	else if(strcmp(s->s_name, "karjighar") == 0){
		memcpy(x->curscale, karjighar, sizeof(x->karjighar));
		post("karjighar scale");
	}
	else if(strcmp(s->s_name, "husseini") == 0){
		memcpy(x->curscale, husseini, sizeof(x->husseini));
		post("husseini scale");
	}
	else if(strcmp(s->s_name, "nahawand") == 0){
		memcpy(x->curscale, nahawand, sizeof(x->nahawand));
		post("nahawand scale");
	}
	else if(strcmp(s->s_name, "nahawandDesc") == 0){
		memcpy(x->curscale, nahawandDesc, sizeof(x->nahawandDesc));
		post("nahawandDesc scale");
	}
	else if(strcmp(s->s_name, "farahfaza") == 0){
		memcpy(x->curscale, farahfaza, sizeof(x->farahfaza));
		post("farahfaza scale");
	}
	else if(strcmp(s->s_name, "murassah") == 0){
		memcpy(x->curscale, murassah, sizeof(x->murassah));
		post("murassah scale");
	}
	else if(strcmp(s->s_name, "ushaqMashri") == 0){
		memcpy(x->curscale, ushaqMashri, sizeof(x->ushaqMashri));
		post("ushaqMashri scale");
	}
	else if(strcmp(s->s_name, "rast") == 0){
		memcpy(x->curscale, rast, sizeof(x->rast));
		post("rast scale");
	}
	else if(strcmp(s->s_name, "rastDesc") == 0){
		memcpy(x->curscale, rastDesc, sizeof(x->rastDesc));
		post("rastDesc scale");
	}
	else if(strcmp(s->s_name, "suznak") == 0){
		memcpy(x->curscale, suznak, sizeof(x->suznak));
		post("suznak scale");
	}
	else if(strcmp(s->s_name, "nairuz") == 0){
		memcpy(x->curscale, nairuz, sizeof(x->nairuz));
		post("nairuz scale");
	}
	else if(strcmp(s->s_name, "yakah") == 0){
		memcpy(x->curscale, yakah, sizeof(x->yakah));
		post("yakah scale");
	}
	else if(strcmp(s->s_name, "yakahDesc") == 0){
		memcpy(x->curscale, yakahDesc, sizeof(x->yakahDesc));
		post("yakahDesc scale");
	}
	else if(strcmp(s->s_name, "mahur") == 0){
		memcpy(x->curscale, mahur, sizeof(x->mahur));
		post("mahur scale");
	}
	else if(strcmp(s->s_name, "hijaz") == 0){
		memcpy(x->curscale, hijaz, sizeof(x->hijaz));
		post("hijaz scale");
	}
	else if(strcmp(s->s_name, "hijazDesc") == 0){
		memcpy(x->curscale, hijazDesc, sizeof(x->hijazDesc));
		post("hijazDesc scale");
	}
	else if(strcmp(s->s_name, "zanjaran") == 0){
		memcpy(x->curscale, zanjaran, sizeof(x->zanjaran));
		post("zanjaran scale");
	}
	else if(strcmp(s->s_name, "zanjaran2") == 0){
		memcpy(x->curscale, zanjaran2, sizeof(x->zanjaran2));
		post("zanjaran2 scale");
	}
	else if(strcmp(s->s_name, "saba") == 0){
		memcpy(x->curscale, saba, sizeof(x->saba));
		post("saba scale");
	}
	else if(strcmp(s->s_name, "zamzam") == 0){
		memcpy(x->curscale, zamzam, sizeof(x->zamzam));
		post("zamzam scale");
	}
	else if(strcmp(s->s_name, "kurd") == 0){
		memcpy(x->curscale, kurd, sizeof(x->kurd));
		post("kurd scale");
	}
	else if(strcmp(s->s_name, "kijazKarKurd") == 0){
		memcpy(x->curscale, kijazKarKurd, sizeof(x->kijazKarKurd));
		post("kijazKarKurd scale");
	}
	else if(strcmp(s->s_name, "nawaAthar") == 0){
		memcpy(x->curscale, nawaAthar, sizeof(x->nawaAthar));
		post("nawaAthar scale");
	}
	else if(strcmp(s->s_name, "nikriz") == 0){
		memcpy(x->curscale, nikriz, sizeof(x->nikriz));
		post("nikriz scale");
	}
	else if(strcmp(s->s_name, "atharKurd") == 0){
		memcpy(x->curscale, atharKurd, sizeof(x->atharKurd));
		post("atharKurd scale");
	}
	else if(strcmp(s->s_name, "aug1") == 0){
		memcpy(x->curscale, aug1, sizeof(x->aug1));
		post("aug1 scale");
	}
	else if(strcmp(s->s_name, "aug2") == 0){
		memcpy(x->curscale, aug2, sizeof(x->aug2));
		post("aug2 scale");
	};

}

static void *scaledeg_new(t_symbol *s, t_floatarg f){
	t_scaledeg *x = (t_scaledeg *)pd_new(scaledeg_class);
	scaledeg_pickscale(x, s);
	x->offset = f;
	//symbolinlet_new(&x->x_obj, &x->scalesym);
	floatinlet_new(&x->x_obj, &x->offset);
	outlet_new(&x->x_obj, &s_float);
	return (x);
}

void scaledeg_setup(void){
	scaledeg_class = class_new(gensym("scaledeg"), (t_newmethod)scaledeg_new, 0,
			sizeof(t_scaledeg), 0, A_DEFSYMBOL, A_DEFFLOAT, 0);
	class_addfloat(scaledeg_class, (t_method)scaledeg_float);
	class_addmethod(scaledeg_class, (t_method)scaledeg_pickscale, gensym("scale"), A_SYMBOL, 0);
}
