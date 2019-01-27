//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT√âCNICO DO C√ÅVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORM√ÅTICOS
//                    VIS√ÉO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN√á√ïES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem√≥ria para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar mem√≥ria de uma imagem
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN√á√ïES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len){
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height){
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height){
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y<height; y++)
	{
		for (x = 0; x<width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

IVC *vc_read_image(char *filename){
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem√≥ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			//printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem√≥ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			//printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

int vc_write_image(char *filename, IVC *image){
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

int vc_rgb_to_gray(IVC *src, IVC *dst){
	unsigned char *datasrc = (unsigned char *)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;

	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	if ((width <= 0) || (height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != src->height)) return 0;
	if ((channels_src != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf*0.587) + (bf*0.114));
		}
	}
	return 1;
}

int vc_gray_to_binary(IVC *srcdst, int threshold) {
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	if ((width <= 0) || (height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			if (data[pos] > threshold) {
				data[pos] = 255;
			}

			else {
				data[pos] = 0;
			}
		}
	}
	return 1;
}

int vc_binary_dilate(IVC *src, IVC *dst, int kernel){
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xx, yy;
	int xxyymax = (kernel - 1) / 2;
	int xxyymin = -xxyymax;
	int max, min;
	long int pos, posk;
	unsigned char threshold;
	int aux = 0;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			max = datasrc[pos];
			min = datasrc[pos];
			aux = 0;
			// NxM Vizinhos
			for (yy = xxyymin; yy <= xxyymax; yy++)
			{
				for (xx = xxyymin; xx <= xxyymax; xx++)
				{
					if ((y + yy >= 0) && (y + yy < height) && (x + xx >= 0) && (x + xx < width))
					{
						posk = (y + yy) * bytesperline + (x + xx) * channels;

						if (datasrc[posk] == 255) aux = 255;
					}
				}
			}
			datadst[pos] = aux;
		}
	}

	return 1;
}

int vc_binary_erode(IVC *src, IVC *dst, int kernel){
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xx, yy;
	int xxyymax = (kernel - 1) / 2;
	int xxyymin = -xxyymax;
	long int pos, posk;
	unsigned char threshold;
	int aux = 0;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			aux = 255;
			// NxM Vizinhos
			for (yy = xxyymin; yy <= xxyymax; yy++)
			{
				for (xx = xxyymin; xx <= xxyymax; xx++)
				{
					if ((y + yy >= 0) && (y + yy < height) && (x + xx >= 0) && (x + xx < width))
					{
						posk = (y + yy) * bytesperline + (x + xx) * channels;

						if (datasrc[posk] == 0) aux = 0;
					}
				}
			}
			datadst[pos] = aux;
		}
	}

	return 1;
}

// Etiquetagem de blobs
// src		: Imagem binria
// dst		: Imagem grayscale (ir conter as etiquetas)
// nlabels	: Endereo de memria de uma varivel inteira. Recebe o nmero de etiquetas encontradas.
// OVC*		: Retorna lista de estruturas de blobs (objectos), com respectivas etiquetas.  necessrio libertar posteriormente esta memria.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels){
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num;
	OVC *blobs; // Apontador para lista de blobs (objectos) que ser retornada desta funo.

				// Verificao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixis de plano de fundo devem obrigatriamente ter valor 0
	// Todos os pixis de primeiro plano devem obrigatriamente ter valor 255
	// Sero atribudas etiquetas no intervalo [1,254]
	// Este algoritmo est assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binria
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

													// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est marcado, j tem etiqueta (j no  255), e  menor que a etiqueta "num"
					if ((datadst[posA] != 0) && (datadst[posA] != 255) && (datadst[posA] < num))
					{
						num = datadst[posA];
					}
					// Se B est marcado, j tem etiqueta (j no  255), e  menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (datadst[posB] != 255) && (datadst[posB] < num))
					{
						num = datadst[posB];
					}
					// Se C est marcado, j tem etiqueta (j no 255), e  menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (datadst[posC] != 255) && (datadst[posC] < num))
					{
						num = datadst[posC];
					}
					// Se D est marcado, j tem etiqueta (j no  255), e  menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (datadst[posD] != 255) && (datadst[posD] < num))
					{
						num = datadst[posD];
					}

					// Actualiza a tabela de etiquetas
					if ((datadst[posA] != 0) && (datadst[posA] != 255))
					{
						if (labeltable[datadst[posA]] != labeltable[num])
						{
							for (a = 1; a<label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posA]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					if ((datadst[posB] != 0) && (datadst[posB] != 255))
					{
						if (labeltable[datadst[posB]] != labeltable[num])
						{
							for (a = 1; a<label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posB]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					if ((datadst[posC] != 0) && (datadst[posC] != 255))
					{
						if (labeltable[datadst[posC]] != labeltable[num])
						{
							for (a = 1; a<label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posC]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					if ((datadst[posD] != 0) && (datadst[posD] != 255))
					{
						if (labeltable[datadst[posD]] != labeltable[num])
						{
							for (a = 1; a<label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posD]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					labeltable[datadst[posX]] = num;

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	// Contagem do nmero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que no hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se no h blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;
	
	return blobs;
}

//area e centro de massa
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs,IVC* print){
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax,box,discount=0;
	long int sumx, sumy;	

	// VerificaÔøΩÔøΩo de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta ÔøΩrea de cada blob
	for (i = 0; i<nblobs; i++){
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		for (y = 1; y<height - 1; y++){
			for (x = 1; x<width - 1; x++){
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// ÔøΩrea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// PerÔøΩmetro
					// Se pelo menos um dos quatro vizinhos nÔøΩo pertence ao mesmo label, entÔøΩo ÔøΩ um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}
		
		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;
		
		// Centro de Gravidade
		blobs[i].xc = (xmin+xmax)/2;
		blobs[i].yc = (ymin+ymax)/2;
		
		if(blobs[i].area > 1500){//para eliminar a peÁa que n„o era possivel ler na totalidade
		
			printf("\nCentro Massa: X = %d Y = %d  Area: %d  ",blobs[i].xc,blobs[i].yc,blobs[i].area);
			vc_desenha_box(print,blobs[i]);
			vc_catch_form(blobs[i].width,blobs[i].height,blobs[i].area,blobs[i].perimeter);
		}
		else{
			discount++;
		}
	}
	
	return discount;
}

int vc_catch_blue(IVC *srcdst){
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	float max = 0, min = 255;
	float s = 0.0, h = 0.0;
	float r = 0.0, g = 0.0, b = 0.0;

	if ((width <= 0) || (height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			r = (float)data[pos];
			g = (float)data[pos + 1];
			b = (float)data[pos + 2];

			max = MAX3(r, g, b);
			if (max == 0.0 || max == r || max == g || max < r+10 || max < g+10)//primeiro restriÁ„o de cores
			{
				data[pos] = 0;
				data[pos + 1] = 0;
				data[pos + 2] = 0;
				s = 0.0;
				h = 0.0;
			}
			else
			{
				min = MIN3(r, g, b);

				s = (max - min) / max;

				s = s * 100;
				if ((max == r) && (g >= b)) h = 60.0f * (g - b) / (max - min);
				else if ((max == g) && (b > g)) h = 360.0f + 60.0f * (g - b) / (max - min);
				else if (max == g) h = 120.0f + 60.0f * (b - r) / (max - min);
				else h = 240.0f + 60.0f * (r - g) / (max - min);

				//h = (h / 360.0) * 255.0;

			}
			max = (max / 255.0) * 100;

			if ((h >= 210) && (s> 65) && (max >= 30) && (max <= 65)){//segunda restriÁ„o
				data[pos] = 255;
				data[pos + 1] = 255;
				data[pos + 2] = 255;
			}
			else{
					data[pos] = 0;
				data[pos + 1] = 0;
				data[pos + 2] = 0;
			}
		}
	}
	
	return 1;
}

int vc_catch_red(IVC *srcdst){
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	float max = 0, min = 255;
	float s = 0.0, h = 0.0;
	float r = 0.0, g = 0.0, b = 0.0;

	if ((width <= 0) || (height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			r = (float)data[pos];
			g = (float)data[pos + 1];
			b = (float)data[pos + 2];

			max = MAX3(r, g, b);
			if (max == b || max ==g || max < g+50)//primeiro filtro
			{
				data[pos] = 0;
				data[pos + 1] = 0;
				data[pos + 2] = 0;
				s = 0.0;
				h = 0.0;
			}
			else
			{
				min = MIN3(r, g, b);

				s = (max - min) / max;

				s = s * 100;
				if ((max == r) && (g >= b)) h = 60.0f * (g - b) / (max - min);
				else if ((max == g) && (b > g)) h = 360.0f + 60.0f * (g - b) / (max - min);
				else if (max == g) h = 120.0f + 60.0f * (b - r) / (max - min);
				else h = 240.0f + 60.0f * (r - g) / (max - min);

				//h = (h / 360.0) * 255.0;

			}
			max = (max / 255.0) * 100;
			
			if (r>g+b+20){//segundo filtro
				data[pos] = 255;
				data[pos + 1] = 255;
				data[pos + 2] = 255;
			}
		}
	}
	return 1;
}

int vc_catch_yellow(IVC *srcdst){
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	float max = 0, min = 255;
	float s = 0.0, h = 0.0;
	float r = 0.0, g = 0.0, b = 0.0;

	if ((width <= 0) || (height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			r = (float)data[pos];
			g = (float)data[pos + 1];
			b = (float)data[pos + 2];

			max = MAX3(r, g, b);
			if (max == 0.0)
			{
				s = 0.0;
				h = 0.0;
			}
			else
			{
				min = MIN3(r, g, b);

				s = (max - min) / max;

				s = s * 100;
				if ((max == r) && (g >= b)) h = 60.0f * (g - b) / (max - min);
				else if ((max == g) && (b > g)) h = 360.0f + 60.0f * (g - b) / (max - min);
				else if (max == g) h = 120.0f + 60.0f * (b - r) / (max - min);
				else h = 240.0f + 60.0f * (r - g) / (max - min);

				//h = (h / 360.0) * 255.0;

			}
			max = (max / 255.0) * 100;
			
			if ((h >= 50) && (h <= 60) && (s >= 50) && (max >= 45) && (max <= 70) && (b < 60)){//unica restriÁ„o
				data[pos] = 255; 
				data[pos + 1] = 255;
				data[pos + 2] = 255;
			}
			else{
				data[pos] = 0;
				data[pos + 1] = 0;
				data[pos + 2] = 0;
			}
		}
	}
	return 1;
}

int vc_catch_beje(IVC *srcdst){
	
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	float max = 0, min = 255;
	float s = 0.0, h = 0.0;
	float r = 0.0, g = 0.0, b = 0.0;

	if ((width <= 0) || (height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			r = (float)data[pos];
			g = (float)data[pos + 1];
			b = (float)data[pos + 2];

			max = MAX3(r, g, b);
			if (max == 0.0 || g < 90 || b < 50 || r > 150)//primeiro filtro
			{
				data[pos] = 0;
				data[pos + 1] = 0;
				data[pos + 2] = 0;
				s = 0.0;
				h = 0.0;
			}
			else
			{
				min = MIN3(r, g, b);

				s = (max - min) / max;

				s = s * 100;
				if ((max == r) && (g >= b)) h = 60.0f * (g - b) / (max - min);
				else if ((max == g) && (b > g)) h = 360.0f + 60.0f * (g - b) / (max - min);
				else if (max == g) h = 120.0f + 60.0f * (b - r) / (max - min);
				else h = 240.0f + 60.0f * (r - g) / (max - min);

				//h = (h / 360.0) * 255.0;

			}
			max = (max / 255.0) * 100;

			if ((h >= 24) && (h <= 56) && (s >= 20) && (s <= 50) && (max >= 40) && (max <= 60)){//segunda restriÁ„o
				data[pos] = 255;
				data[pos + 1] = 255;
				data[pos + 2] = 255;
			}
			else{
				data[pos] = 0;
				data[pos + 1] = 0;
				data[pos + 2] = 0;
			}
		}
	}
	return 1;
	
}

int vc_desenha_box(IVC *src, OVC*blob) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	int i, xx, yy, pos = 0, posCentroMassa;
		for (yy = blob->y; yy <= blob->y + blob->height; yy++)
		{
			for (xx = blob->x; xx <= blob->x + blob->width; xx++)
			{
				if ((yy == blob->y||yy == blob->y + blob->height || xx == blob->x||xx == blob->x + blob->width))
				{
					pos = yy * bytesperline_src + xx * channels_src;
					posCentroMassa = blob->yc * bytesperline_src + blob->xc * channels_src;

					datasrc[pos] = 0;
					datasrc[pos + 1] = 255;
					datasrc[pos + 2] = 0;

					datasrc[posCentroMassa] = 0;
					datasrc[posCentroMassa + 1] = 255;
					datasrc[posCentroMassa + 2] = 0;
				}
			}
		}
}

int vc_catch_form(int width,int height,int area,int perimeter){
	int area_estimada,raio,i,pos,erro,dif,base,altura,area1;
	raio=width/2;raio=raio*raio;
	area_estimada=PI*raio;
	erro=area-area_estimada;
	base=floor(perimeter/5);altura=floor(height/2);area1=(base*altura);area1=area1/2;area1=area1*5;		

	if(erro<0){erro=erro*-1;}//torna o erro positivo sem perder o seu valor

	dif=width-height;

	if(dif<10 && dif>-10){
		if(erro<150 && erro>50){
			printf("Circulo\n");
			return 0;
		}
		erro=area1-area;
		if(erro>150){
			printf("Pentagno\n");
			return 0;
		}
		else{
			printf("Quadrado\n");
			return 0;
		}
	}	
	else{
		area_estimada=(width*height)/2;
		erro=area-area_estimada;
		if(erro<0){
			erro=erro*-1;
		}
		if(erro<500 && erro>300){
			printf("Rectangulo\n");
			return 0;
		}
		else{
			printf("Triangulo\n");
			return 0;
		}
	}
	return 0;
}

int vc_count_blue(IVC *print){
	
	IVC* image[4];
	OVC* labelling;
	int nrObjetos=0,i,j;
	image[0]=vc_read_image("PecasDeMadeira.ppm");
	
	if(image[0]==NULL){
		printf("ERRO na leitura da imagem");
	}
	
	image[1]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[2]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[3]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[4]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
		
	vc_catch_blue(image[0]);//filtro de catura do azul
	vc_rgb_to_gray(image[0],image[1]);
	vc_gray_to_binary(image[1],3);
	vc_binary_dilate(image[1],image[2],5);
	vc_binary_erode(image[2],image[3],1);
	
	labelling=vc_binary_blob_labelling(image[3],image[4],&nrObjetos);
	
	printf("\n-------------------------------------------------------\nObjetos Azuis Identificados :\n");
	
	i=vc_binary_blob_info(image[4],labelling,nrObjetos,print);
	
	nrObjetos-=i;
	
	printf("\nNumero Total de Objetos Azuis : %d\n",nrObjetos);

	vc_image_free(image[0]);
	vc_image_free(image[1]);
	vc_image_free(image[2]);
	vc_image_free(image[3]);
	
	return nrObjetos;
}

int vc_count_red(IVC *print){
	
	IVC* image[4];
	OVC* labelling;
	int nrObjetos=0,i;
	image[0]=vc_read_image("PecasDeMadeira.ppm");
	
	if(image[0]==NULL){
		printf("ERRO na leitura da imagem");
	}
	
	image[1]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[2]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[3]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[4]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
		
	vc_catch_red(image[0]);//filtro de catura do vermelho
	vc_rgb_to_gray(image[0],image[1]);
	vc_gray_to_binary(image[1],3);
	vc_binary_dilate(image[1],image[2],5);
	vc_binary_erode(image[2],image[3],1);
	
	labelling=vc_binary_blob_labelling(image[3],image[4],&nrObjetos);
	
	printf("\n-------------------------------------------------------\nObjetos Vermelhos Identificados :\n");
	
	i=vc_binary_blob_info(image[4],labelling,nrObjetos,print);
	
	nrObjetos-=i;
	
	printf("\nNumero Total de Objetos Vermelhos: %d\n",nrObjetos);
	
	vc_image_free(image[0]);
	vc_image_free(image[1]);
	vc_image_free(image[2]);
	vc_image_free(image[3]);
	
	return nrObjetos;
}

int vc_count_yellow(IVC *print){
	
	IVC* image[4];
	OVC* labelling;
	int nrObjetos=0,i;
	image[0]=vc_read_image("PecasDeMadeira.ppm");
	
	if(image[0]==NULL){
		printf("ERRO na leitura da imagem");
	}
	
	image[1]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[2]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[3]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[4]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	
	vc_catch_yellow(image[0]);//filtro de catura do amarelo
	vc_rgb_to_gray(image[0],image[1]);
	vc_gray_to_binary(image[1],3);
	vc_binary_dilate(image[1],image[2],6);
	vc_binary_erode(image[2],image[3],2);
	
	labelling=vc_binary_blob_labelling(image[3],image[4],&nrObjetos);
	
	printf("\n-------------------------------------------------------\nObjetos Amarelos Identificados :\n");
	
	i=vc_binary_blob_info(image[4],labelling,nrObjetos,print);
	
	nrObjetos-=i;
	
	printf("\nNumero Total de Objetos Amarelos : %d\n",nrObjetos);
		
	vc_image_free(image[0]);
	vc_image_free(image[1]);
	vc_image_free(image[2]);
	vc_image_free(image[3]);
	
	return nrObjetos;	
}

int vc_count_beje(IVC*print){
	
	IVC* image[4];
	OVC* labelling;
	int nrObjetos=0,i;
	image[0]=vc_read_image("PecasDeMadeira.ppm");
	
	if(image[0]==NULL){
		printf("ERRO na leitura da imagem");
	}
	
	image[1]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[2]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[3]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);
	image[4]=vc_image_new(image[0]->width, image[0]->height,1,image[0]->levels);	
		
	vc_catch_beje(image[0]);//filtro de captura do beje
	vc_rgb_to_gray(image[0],image[1]);
	vc_gray_to_binary(image[1],3);
	vc_binary_dilate(image[1],image[2],10);
	vc_binary_erode(image[2],image[3],10);
	
	labelling=vc_binary_blob_labelling(image[3],image[4],&nrObjetos);
	
	printf("\n-------------------------------------------------------\nObjetos Beje Identificados :\n");
	
	i=vc_binary_blob_info(image[4],labelling,nrObjetos,print);
	
	nrObjetos-=i;
	
	printf("\nNumero Total de Objetos Beje: %d\n",nrObjetos);
	
	vc_image_free(image[0]);
	vc_image_free(image[1]);
	vc_image_free(image[2]);
	vc_image_free(image[3]);
	
	return nrObjetos;
}

//o inicio do programa dever· ser por aqui
void vc_count_colors(){

	int nrTotalObjetos=0;	
	IVC *print;
	print=vc_read_image("PecasDeMadeira.ppm");
	printf("\nObjetos segmantados por cores\n");
	
	nrTotalObjetos+=vc_count_blue(print);
	
	nrTotalObjetos+=vc_count_red(print);
	
	nrTotalObjetos+=vc_count_yellow(print);
	
	nrTotalObjetos+=vc_count_beje(print);
	
	printf("\n#####################################\n\nNumero total de objetos: %d",nrTotalObjetos);
	
	vc_write_image("bounding.ppm",print);
	
}
