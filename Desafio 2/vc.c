//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//desabilita (no msvc++) warnings de funções nao seguras (fopen,sscanf,etc..)
#define _CRT_SECURE_NO_WARNINGS	

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
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
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
	}
	
	*t = 0;
	
	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
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
			if((countbits > 8) || (x == width - 1))
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


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
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
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;
	
	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
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
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
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


int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
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
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
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

int vc_gray_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificação de erros

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;

	if (channels != 1) return 0;

	//inverte a imagem

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y*bytesperline + x * channels;

			data[pos] = 255 - data[pos];
		}
	}

	return 1;
}

int vc_rgb_negative(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width*srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificação de erros

	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;

	if (channels != 3) return 0;

	//inverte a imagem

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y*bytesperline + x * channels;

			data[pos] = 255 - data[pos];//R
			data[pos + 1] = 255 - data[pos + 1];//G 
			data[pos + 2] = 255 - data[pos + 2];//B
		}
	}

	return 1;
}

int vc_rgb_to_hsv(IVC *src, IVC *dst) {

	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf, value, saturation, hue, min;

	// Verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			value = 0; if (rf > value) { value = rf; } if (gf > value) { value = gf; } if (bf > value) { value = bf; }
			min = 255; if (rf < min) { min = rf; } if (gf < min) { min = gf; } if (bf < min) { min = bf; }

			if (value == 0 || value == min) {
				saturation = 0; hue = 0;
			}
			else
			{
				saturation = ((value - min) * 255) / value;

				if (value == rf && gf >= bf) { hue = 60.0 * (gf - bf) / (value - min); }
				if (value == rf && bf > gf) { hue = 360.0 + 60.0 * (gf - bf) / (value - min); }
				if (value == gf) { hue = 120.0 + 60.0 * (bf - rf) / (value - min); }
				if (value == bf) { hue = 240.0 + 60.0 * (rf - gf) / (value - min); }
			}

			datadst[pos_dst] = (unsigned char)(hue * 255 / 360);
			datadst[pos_dst + 1] = (unsigned char)saturation;
			datadst[pos_dst + 2] = (unsigned char)(value);
		}
	}
	return 1;


}

int vc_rgb_to_gray(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//verificar erros

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}

	}

	return 1;
}

int vc_scale_gray_to_rgb(IVC *src, IVC *dst) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 3))return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y*bytesperline_src + x*channels_src;
			pos_dst = y*bytesperline_dst + x*channels_dst;


			if (datasrc[pos_src] <= 64) {
				datadst[pos_dst] = 0;
				datadst[pos_dst + 1] = datasrc[pos_src] * (255 / 64);
				datadst[pos_dst + 2] = 255;
			}
			else if (datasrc[pos_src] <= 128) {
				datadst[pos_dst] = 0;
				datadst[pos_dst + 1] = 255;
				datadst[pos_dst + 2] = 255 - (datasrc[pos_src] - 64)*(255 / 64);
			}
			else if (datasrc[pos_src] <= 192) {
				datadst[pos_dst] = (datasrc[pos_src] - 128) * (255 / 64);
				datadst[pos_dst + 1] = 255;
				datadst[pos_dst + 2] = 0;
			}
			else {
				datadst[pos_dst] = 255;
				datadst[pos_dst + 1] = 255 - (datasrc[pos_src] - 192)*(255 / 64);
				datadst[pos_dst + 2] = 0;
			}

		}
	}
	return 1;
}


int vc_gray_to_binary(IVC *src, IVC *dst, int threshold)
{
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float color;

	//verificar erros

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			color = datasrc[pos_src];

			/*1*/	if (color > threshold) { datadst[pos_dst] = 255; }
			/*0*/	if (color <= threshold) { datadst[pos_dst] = 0; }
		}
	}
}

int vc_gray_to_binary_global_mean(IVC *src, IVC* dst)
{
	unsigned char *datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float color, threshold = 0;

	//verificar erros

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			threshold += datasrc[pos_src];
		}
	}

	threshold = threshold / (width * height);

		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				pos_src = y * bytesperline_src + x * channels_src;
				pos_dst = y * bytesperline_dst + x * channels_dst;
				color = datasrc[pos_src];

				if (color > threshold) { datadst[pos_dst] = 255; }
				if (color <= threshold) { datadst[pos_dst] = 0; }

			}
	}
}

int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel) 
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	float threshold = 0;
	int max, min;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;
	kernel /= 2;

	for (y = kernel; y < height - kernel; y++)
	{
		for (x = kernel; x < width - kernel; x++)
		{
			pos_dst = y*bytesperline_dst + x*channels_dst;

			for (max = 0, min = 255, x2 = x - kernel; x2 <= x + kernel; x2++)
			{
				for (y2 = y - kernel; y2 <= y + kernel; y2++)
				{
					pos_src = y2*bytesperline_src + x2*channels_src;
					if (datasrc[pos_src] > max) { max = datasrc[pos_src]; }
					else if (datadst[pos_src] < min) { min = datasrc[pos_src]; }
				}
			}
			threshold = 0.5*(max + min);

			pos_src = y*bytesperline_src + x*channels_src;
			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	return 1;
}

//sempre impar o kernel
int vc_binary_dilate(IVC *src, IVC *dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width*dst->channels;
	int channels_dst = dst->channels;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	float threshold = 0;
	int veri;

	//verificação de erros

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;
	

	//por a imagem a preto
	
	int j;

	int pixeis = width * height; //obter o numero total de pixeis 

	for (j = 0; j < pixeis; j++)
	{
		dst->data[j] = 0;
	}
	

	kernel /= 2;


	for (y = kernel; y < height - kernel; y++)
	{
		for (x = kernel; x < width - kernel; x++)
		{
			pos_dst = y*bytesperline_dst + x*channels_dst;
			veri = 0;
			for (y2 = y - kernel; y2 <= y + kernel; y2++)
			{
				for  (x2 = x - kernel; x2 <= x + kernel; x2++)
				{
					pos_src = y2*bytesperline_src + x2*channels_src;
					if (datasrc[pos_src] == 255) { veri = 1; }
				}

			}
			if (veri == 1) { datadst[pos_dst] = 255; }
			else datadst[pos_dst] = 0;
		}
	}

	return 1;

}

//sempre impar o kernel
int vc_binary_erode(IVC *src, IVC *dst, int kernel)
{
unsigned char* datasrc = (unsigned char*)src->data;
int bytesperline_src = src->width*src->channels;
int channels_src = src->channels;
unsigned char* datadst = (unsigned char*)dst->data;
int width = src->width;
int height = src->height;
int bytesperline_dst = dst->width*dst->channels;
int channels_dst = dst->channels;
int x, y, x2, y2;
long int pos_src, pos_dst;
int verifica;
kernel *= 0.5;

if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))return 0;
if ((src->width != dst->width) || (src->height != dst->height))return 0;
if ((src->channels != 1) || (dst->channels != 1))return 0;


for (y = kernel; y < height - kernel; y++)
{
	for (x = kernel; x < width - kernel; x++)
	{
		pos_dst = y*bytesperline_dst + x*channels_dst;

		verifica = 0;

		for (y2 = y - kernel; y2 <= y + kernel; y2++)
		{
			for (x2 = x - kernel; x2 <= x + kernel; x2++)
			{
				pos_src = y2*bytesperline_src + x2*channels_src;
				if (datasrc[pos_src] == 0) { verifica = 1; }
			}
		}

		if (verifica == 1) { datadst[pos_dst] = 0; }
		else { datadst[pos_dst] = 255; }

	}
}


return 1;

}

// Aplicar fecho binário
int vc_binary_close(IVC *src , IVC *dst, int erodeSize, int dilateSize)
{
	// Criar imagem temporária
	IVC *imagemTemporaria;
	imagemTemporaria = vc_image_new(src->width, src->height, src->channels, src->levels);


	// Validar imagem original
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (src->channels != 1)
		return 0;


	// Validar imagem de destino
	if ((src->width <= 0) || (dst->height <= 0) || (dst->data == NULL))
		return 0;
	if (dst->channels != 1)
		return 0;


	// Executar função de dilatação
	if (vc_binary_dilate(dst, imagemTemporaria, dilateSize) == 0)
		return 0;


	// Executar função de erosão
	if (vc_binary_erode(imagemTemporaria, dst, erodeSize) == 0)
		return 0;


	// Libertar imagem temporária
	vc_image_free(imagemTemporaria);


	// Devolver que operação foi executada com sucesso
	return 1;
}

int vc_binary_open(IVC *src, IVC *dst, int kernel) {

	IVC *dstTemp = vc_image_new(src->width, src->height, 1, src->levels);

	int ret = 1;

	ret &= vc_binary_erode(src, dstTemp, kernel);

	ret &= vc_binary_dilate(dstTemp, dst, kernel); // ret para verificar erro de 0 ou 1

	vc_image_free(dstTemp);

	return ret;

}

// Etiquetagem de blobs
// src		: Imagem binária de entrada
// dst		: Imagem grayscale (irá conter as etiquetas)
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. É necessário libertar posteriormente esta memória.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
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
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

				// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
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

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
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

	//printf("\nMax Label = %d\n", label);

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
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


int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta área de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
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
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);

		//printf("Area: %d", blobs->area);
		//printf("Centro de Gravidade: %d %d", blobs->xc, blobs->yc);
	}

	return 1;
}


//funções trabalho 1, Desafio 2

int vc_verifica_dados_brancos(int nlabelsB, int nlabelsBP, OVC *blobs, OVC *blobsP, int numeroDadosBrancos, int numeroPintasPretas) {
	int i, k;


	//Para os dados
	for (i = 0; i < nlabelsB; i++)
	{
		//no caso de a are ser superior  a 4000 mil é um dado
		if (blobs[i].area > 4000)
		{
			numeroDadosBrancos++;

			printf("\n Dado Branco %d \n ", numeroDadosBrancos);

			printf(" Centro de Massa:  [%d %d]\n", blobs[i].xc, blobs[i].yc);

			//Para associar as pintas Pretas aos dados brancos
			for (k = 0; k < nlabelsBP; k++)
			{
				//condição que verifica se as pintas estao dentro do dado
				if (((blobsP[k].xc > blobs[i].x) && (blobsP[k].xc < (blobs[i].x + blobs[i].width))) && ((blobsP[k].yc > blobs[i].y) && (blobsP[k].yc < (blobs[i].y + blobs[i].height))))
				{
					numeroPintasPretas++;
				}
			}

			printf(" Numero de Pintas: %d \n", numeroPintasPretas);

			numeroPintasPretas = 0;

			

		}

	}



	return 1;
}

int vc_verifica_dados_pretos(int nlabelsP, int nlabelsPB, OVC *blobs, OVC *blobsP, int numeroDadosPretos, int numeroPintasBrancas) {

	int j, k;

	for (j = 0; j < nlabelsP; j++)
	{
		//no caso da area ser inferior a 100 é uma pinta
		if (blobs[j].area < 100)

			numeroPintasBrancas++;


	}

	for (k = 0; k < nlabelsPB; k++)
	{
		if (blobsP[k].area > 4000)
		{
			numeroDadosPretos++;
			printf("\n Dado Preto %d \n ", numeroDadosPretos);
			printf(" Centro de Massa: [%d %d]\n", blobsP[k].xc, blobsP[k].yc);
			printf(" Numero de Pintas : %d", numeroPintasBrancas);
		}

	
	}

	return 1;
}

int vc_desenha_box(IVC *src, OVC *blobs, int nBlobs) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	int i, xx, yy,pos = 0;

	for (i = 0; i < nBlobs; i++)
	{
		for (yy = blobs[i].y; yy <= blobs[i].y + blobs[i].height; yy++)
		{
			for(xx = blobs[i].x; xx <= blobs[i].x + blobs[i].width; xx++)
			{
				pos = yy * bytesperline_src + xx * channels_src;

				if ((yy == blobs[i].y || yy == blobs[i].y + blobs[i].height || xx == blobs[i].x || xx == blobs[i].x + blobs[i].width) && blobs[i].area > 4000)
				{
					datasrc[pos] = 252;
					datasrc[pos + 1] = 2;
					datasrc[pos + 2] = 139;
		
				}
			}
		}
	}	
}

int vc_pinta_centroMassa(IVC *src, OVC *blobs, int nBlobs)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width*src->channels;
	int channels_src = src->channels;
	int i = 0, xx, yy, pos = 0;

	for ( i = 0; i < nBlobs; i++)
	{
		yy = blobs[i].yc;
		xx = blobs[i].xc;

		pos = yy * bytesperline_src + xx * channels_src;

		if (blobs[i].area > 4000)
		{
			datasrc[pos] = 252;
			datasrc[pos + 1] = 2;
			datasrc[pos + 2] = 139;

			
			yy++;
			xx++;
			
		}
		
	}
}