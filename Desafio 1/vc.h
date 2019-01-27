//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG
#define MAX(a,b)(a<b?a:b)
#define MAX3(a,b,c) (a>b ? (a>c ? a:c):(b>c ?b:c))
#define MIN3(a,b,c) (a<b ? (a<c ? a:c):(b<c ?b:c))
#define PI 3.14
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// area
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perimetro
	int label;					// Etiqueta
	int cor;					//1 blue;2 red;3 yellow;4 beje
	unsigned char *mask;		// Not Used
	unsigned char *data;		// Not Used
	int channels;				// Not Used
	int levels;					// Not Used
} OVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);

IVC *vc_image_free(IVC *image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);

int vc_write_image(char *filename, IVC *image);

int vc_gray_to_binary(IVC *srcdst, int threshold);

int vc_binary_dilate(IVC *src, IVC *dst, int kernel);

int vc_binary_erode(IVC *src, IVC *dst, int kernel);

OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);

int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs,IVC *print);

int vc_pinta_box(IVC *imagem, int xmin, int ymin, int width, int height);

int vc_catch_form(int width,int height,int area,int perimeter);

int vc_catch_yellow(IVC *srcdst);

int vc_catch_blue(IVC *srcdst);

int vc_catch_red(IVC *srcdst);

int vc_catch_beje(IVC *srcdst);

int vc_count_yellow(IVC *print);

int vc_count_blue(IVC *print);

int vc_count_red(IVC *print);

int vc_count_beje(IVC *print);

void vc_count_colors();
