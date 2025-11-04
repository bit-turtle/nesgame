// define a 2x2 metasprite and store a normal and flipped version
#define DEF_METASPRITE(name,code,pal)\
const byte name[2][] = {\
	{\
		{0,0,(code)+0,pal}\
                {0,8,(code)+1,pal}\
                {8,0,(code)+2,pal}\
                {8,8,(code)+3,pal}\
	}\
        {\
		{0,0,(code)+0,(pal)|OAM_FLIP_H}\
                {0,8,(code)+1,(pal)|OAM_FLIP_H}\
                {8,0,(code)+2,(pal)|OAM_FLIP_H}\
                {8,8,(code)+3,(pal)|OAM_FLIP_H}\
	}\
};
