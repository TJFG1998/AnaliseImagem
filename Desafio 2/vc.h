//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define _CRT_SECURE_NO_WARNINGS	

#define VC_DEBUG

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b5)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;//unsigned char para começar em 0 ate 125
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

//FUNÇÃO PARA CALCULO DO NEGATIVO DE UMA IMAGEM GRAY E DE UMA IMAGEM RGB
int vc_gray_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);

//função que converte de rgb para hsv
int vc_rgb_to_hsv(IVC *src, IVC *dst);

//função que converte de rgb para gray
int vc_rgb_to_gray(IVC *src, IVC *dst);

//converte uma imagem com escala de intensidades em cinzento para uma em escala de cores no espaço rgb
int vc_scale_gray_to_rgb(IVC *src, IVC *dst);//binarizar uma imagem em tons de cinzento por thresholdingint vc_gray_to_binary(IVC *src, IVC *dst, int threshold);//binarizar uma imagem em tons de cinzento por thresholding da media globalint vc_gray_to_binary_global_mean(IVC *src, IVC* dst);//Binarização automatica atraves do ponto do meioint vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel);//dilataçãoint vc_binary_dilate(IVC *src, IVC *dst, int kernel);//erosão bináriosint vc_binary_erode(IVC *src, IVC *dst, int kernel);//int vc_binary_open(IVC *src, IVC *dst, int kernel);//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);
int vc_binary_blob_info(IVC *src, OVC * blobs, int nblobs);

/*funções para o trabalho*/

int vc_verifica_dados_brancos(int nlabelsB, int nlabelsBP, OVC *blobs, OVC *blobsP, int numeroDadosBrancos, int numeroPintasPretas);
int vc_verifica_dados_pretos(int nlabelsP, int nlabelsPB, OVC *blobs, OVC *blobsP, int numeroDadosPretos, int numeroPintasBrancas);
int vc_desenha_box(IVC *src, OVC *blobs, int nBlobs);
int vc_pinta_centroMassa(IVC *src, OVC *blobs, int nBlobs);