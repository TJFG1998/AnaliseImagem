#include <stdio.h>
#include "vc.h"


int main(void) {
	IVC *image[11];
	OVC *blobsDomino, *blobsPintas;
	int numeroObjDomino, numeroObjPintas;
	int i;


	image[0] = vc_read_image("Domino.ppm");
	if (image[0] == NULL) {
		printf("ERROR -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	for (i = 1; i < 11; i++)
	{
		image[i] = vc_image_new(image[0]->width, image[0]->height, 1, image[0]->levels);
	}

	for (i = 1; i < 11; i++)
	{
		if (image[i] == NULL) {
			printf("ERROR -> vc_read_image():\n\tFile not found!\n");
			getchar();
			return 0;
		}
	}

	///
	vc_rgb_to_gray(image[0], image[1]);
	vc_gray_to_binary(image[1], image[2],160);
	vc_binary_erode(image[2], image[3], 5);
	vc_binary_dilate(image[3], image[4],9);
	///

	///
	vc_gray_negative(image[1]);
	vc_gray_to_binary(image[1], image[6],200);
	vc_binary_erode(image[6], image[7], 5);
	vc_binary_dilate(image[7], image[8], 11);
	///

	/// Imagem 1
	blobsDomino = vc_binary_blob_labelling(image[4], image[9], &numeroObjDomino);
	vc_binary_blob_info(image[9], blobsDomino, numeroObjDomino);
	///Imagem 2
	blobsPintas = vc_binary_blob_labelling(image[8], image[10], &numeroObjPintas);
	vc_binary_blob_info(image[10], blobsPintas, numeroObjPintas);
	///

	vc_reconhece_domino(blobsDomino, blobsPintas, numeroObjPintas);

	vc_desenha_box_centromassa(image[0], blobsDomino, numeroObjDomino);

	vc_write_image("DominosFinal.ppm", image[0]);

	for (i = 0; i < 11; i++)
	{
		vc_image_free(image[i]);
	}

	getchar();

	return 0;
}
