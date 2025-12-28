
#define NULL_CHAR '\0'
#define CRACK_DONE_FLAG NULL_CHAR


/* LIBASW */
extern char *connasw();
extern char *disconnasw();
/* LIBFRM */
extern int exprerr;
float expr();
char *polish();
/* LIBDGL */
char crack();
extern int arg_index;
extern char *arg_option;
extern float sfexpr();
extern long summation();
/* LIBPROCOM */
struct proplist {
	char 	*propname, 
		*propval;
	struct proplist 
		*nextprop, 
		*lastprop;
};
typedef struct proplist PROP;
extern PROP *getheader();
extern PROP *getpaddr();
extern PROP *mkprop();
extern char *getprop();
extern int fgetfbuf();
extern int fputfbuf();
extern int fgetsbuf();
extern int fputsbuf();
extern int fgetfloat();
extern int fputfloat();
extern int fgetshort();
extern int fputshort();
extern int fgetlong();
extern int fputlong();
extern void fflushfloat();

extern void trans();
extern void f1init(double sequence) ;
extern double corrand(double lb, double ub) ;
extern void set_sample_size(short size) ;
extern int finitbuf(FILE *iop) ;
extern int pinit(FILE *iop) ;
extern int cpioheader(FILE *ip, FILE *op) ;
extern int putheader(FILE *iop) ;
extern int wprop(struct proplist *p, FILE *iop) ;
extern int parseprop(FILE *iop) ;
extern int putprop(FILE *iop, char *name, char *value) ;
extern void bump(FILE *iop, short samplesize) ; 
extern int putfc(char ch, FILE *iop) ; 
extern int cpoheader(PROP *pl, FILE *op) ; 

  

#define getfloat(fptr)fgetfloat(fptr,stdin)
#define getshort(sptr)fgetshort(sptr,stdin)
#define getlong(fptr)fgetlong(fptr,stdin)
#define putfloat(fptr)fputfloat(fptr,stdout)
#define putshort(fptr)fputshort(fptr,stdout)
#define putlong(fptr)fputlong(fptr,stdin)
#define flushfloat()fflushfloat(stdout)
#define flushshort()fflushshort(stdout)
#define flushlong()fflushlong(stdout)
#define getsample(fptr)fgetsample(fptr,stdin)

struct func {
	char *ftype;	/* function type: MONO_IN_X, or XY_PAIRS */
	char *fname;	/* function name */
	long flen;	/* function length */
	float *fxval;	/* x function values */
	float *fyval;	/* y function values */
}; 
typedef struct func FUNCTION;
extern FUNCTION *read_func_fid();
extern FUNCTION *read_func_file();

/* libran */
extern double frand();
extern double onefrand();
extern double corrand();
extern double getf();
extern double arcsin();
extern double beta();
extern double cauchy();
extern double expn();
extern double gamma();
extern double gauss();
extern double hyper();
extern double lin();
extern double logist();
extern double plapla();
extern double randfc();
extern double randfi();
