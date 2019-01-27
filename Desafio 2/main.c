#define _CRT_SECURE_NO_WARNINGS	

#include <stdlib.h>
#include <stdio.h>
#include "vc.h"


int main(void)
{
	//declaração de variaveis
	IVC * image[10];
	OVC *blobs;
	OVC *blobsP;
	int nlabelsB, nlabelsP, op, i, numeroDadosBrancos = 0;
	int j, numeroPintasBrancas = 0;
	int numeroDadosPretos = 0, numeroPintasPretas = 0;
	int k, l;
	int nlabelsBP, nlabelsPB;

	image[0] = vc_read_image("Dados.ppm"); //ler a imagem normal

	if (image[0] == NULL)
	{
		printf("ERROR -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	image[1] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels); //criar uma nova imagem com o mesmo tamanho da primeira, mas com apenas 1 canal para a conversao para gray

	if (image[1] == NULL) {
		printf("ERROR -> vc_image_new():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	//converter a imagem original para gray
	vc_rgb_to_gray(image[0], image[1]);

	image[2] = vc_image_new(image[1]->width, image[1]->height, 1, image[1]->levels);//para utilizar na passagem da imagem em tons de cinza para binario

	if (image[2] == NULL) {
		printf("ERROR -> vc_image_new():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	//passagem da imagem em tons de cinza para binario
	vc_gray_to_binary(image[1], image[2], 194);
	vc_write_image("graybinary.pgm", image[2]);

	//inverter a imagem obtida nos tons de cinza
	image[5] = vc_image_new(image[1]->width, image[1]->height, 1, image[1]->levels);

	vc_gray_negative(image[1]);
	vc_write_image("graynegative.pgm", image[1]);

	vc_gray_to_binary(image[1], image[5], 198);
	vc_write_image("graynegativeBinary.pgm", image[5]);

	//dado branco
	image[3] = vc_image_new(image[2]->width, image[2]->height, 1, image[2]->levels);//graybinary

	image[6] = vc_image_new(image[5]->width, image[5]->height, 1, image[5]->levels);//graynegativebinary


	if (image[3] == NULL) {
		printf("ERROR -> vc_image_new():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	if (image[6] == NULL) {
		printf("ERROR -> vc_image_new():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	//graybinary
	blobs = vc_binary_blob_labelling(image[2], image[3], &nlabelsB);
	vc_binary_blob_info(image[3], blobs, nlabelsB);

	

	//graynegativebinary
	blobsP = vc_binary_blob_labelling(image[5], image[6], &nlabelsBP);
	vc_binary_blob_info(image[6], blobsP, nlabelsBP);

	//verificacao
	vc_verifica_dados_brancos(nlabelsB, nlabelsBP, blobs, blobsP, numeroDadosBrancos, numeroPintasPretas);
	
	//desenhar nos dados brancos
	vc_desenha_box(image[0], blobs, nlabelsB);
	vc_pinta_centroMassa(image[0], blobs, nlabelsB);
	vc_write_image("boundingbox.pgm", image[0]);


	//dado preto
	image[4] = vc_image_new(image[2]->width, image[2]->height, 1, image[2]->levels);//graybinary
	

	image[7] = vc_image_new(image[5]->width, image[5]->height, 1, image[5]->levels);//graynegativebinary

	if (image[4] == NULL) {
		printf("ERROR -> vc_image_new():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	//binarygray
	blobs = vc_binary_blob_labelling(image[2], image[4], &nlabelsP);
	vc_binary_blob_info(image[4], blobs, nlabelsP);

	//dilatebinarygraynegative
	vc_binary_dilate(image[5], image[7], 7);
	vc_write_image("dilategraynegative.pgm", image[7]);

	image[8] = vc_image_new(image[7]->width, image[7]->height, 1, image[7]->levels);//graynegativebinary


	blobsP = vc_binary_blob_labelling(image[7], image[8], &nlabelsPB);
	vc_binary_blob_info(image[8], blobsP, nlabelsPB);

	//desenhar nos dados pretos
	vc_desenha_box(image[0], blobsP, nlabelsPB);
	vc_pinta_centroMassa(image[0], blobsP, nlabelsPB);
	vc_write_image("boundingbox.pgm", image[0]);

	vc_verifica_dados_pretos(nlabelsP, nlabelsPB, blobs, blobsP, numeroDadosPretos, numeroPintasBrancas);



	//free das imagens
	for (i = 1; i <= 8 ; i++)
	{
		vc_image_free(image[i]);
	}
	

	//filtergear
	system("cmd /c start filtergear boundingbox.pgm ");
	

	getchar();
	return 1;
}