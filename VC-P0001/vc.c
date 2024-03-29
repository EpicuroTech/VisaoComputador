//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VIS�O POR COMPUTADOR
//
//             [  BRUNO OLIVEIRA - boliveira@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <math.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
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


// Libertar mem�ria de uma imagem
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
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
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

			// Aloca mem�ria para imagem
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

			// Aloca mem�ria para imagem
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

//=================================================================================

// FUN��O AULA ESPA�OS DE COR  ---GRAY---
// VC04_15
int vc_gray_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	//inverter imagem Gray

	for (y = 0; y < height; y ++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
		}
	}
	return 1;
}

// FUN��O AULA ESPA�OS DE COR ---RGB---
// VC04_16
int vc_rgb_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
			data[pos+1] = 255 - data[pos+1];
			data[pos+2] = 255 - data[pos+2];
		}
	}
	return 1;
}
//Matem RED e o resto com o mesmo valor de RED ---RGB_RED_GRAY---
//VC04_20
int vc_rgb_get_red_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			//data[pos] = 255 - data[pos];
			data[pos + 1] = data[pos];
			data[pos + 2] = data[pos];
		}
	}
	return 1;
}


//Matem Green e o resto com o mesmo valor de GREEN ---RGB_GREEN_GRAY---
//VC04_20
int vc_rgb_get_green_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos+1];
			//data[pos + 1] = 0;
			data[pos + 2] = data[pos+1];
		}
	}
	return 1;
}

//Matem Blue e o resto o com o mesmo valor de blue ---RGB_BLUE_GRAY---
//VC04_20
int vc_rgb_get_blue_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos+2];
			data[pos + 1] = data[pos+2];
			//data[pos + 2] = 0;
		}
	}
	return 1;
}
//Matem RED e o resto a 0
//VC04_20
int vc_rgb_get_red(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			//data[pos] = data[pos];
			data[pos + 1] = 0;
			data[pos + 2] = 0;
		}
	}
	return 1;
}
//Matem GREEN e o resto a 0
//VC04_20
int vc_rgb_get_green(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 0;
			//data[pos + 1] = data[pos + 1];
			data[pos + 2] = 0;
		}
	}
	return 1;
}
//Matem BLUE e o resto a 0
//VC04_20
int vc_rgb_get_blue(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 0;
			data[pos + 1] = 0;
			//data[pos + 2] = data[pos + 2];
		}
	}
	return 1;
}
//RGB TO GRAY
//
//VC04_23
int vc_rgb_to_gray(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
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

//RGB TO HSV
//VC04_33
int vc_rgb_to_hsv(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//auxiliares hsv H = hue; S=(auxmax-auxmin)/auxmax; V= auxmax;
	float auxmax, auxmin, hue;

	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src+1];
			bf = (float)datasrc[pos_src+2];

			auxmax = rf;//inicializar igual a rf posicao i=0
			auxmin = rf;
			for (int i = 0; i < 2;i++)
			{
				if (auxmax < (float)datasrc[pos_src+i+1])
				{
					auxmax = (float)datasrc[pos_src+i+1];
				}
				if (auxmin > (float)datasrc[pos_src+i+1])
				{
					auxmin = (float)datasrc[pos_src+i+1];
				}
			}

			if ((rf == gf) && (gf == bf)) hue = 0;

			else if((rf==auxmax) && (gf >= bf))//R e blue min
			{
				hue = 60 * (gf - bf) / (auxmax - auxmin);
			}
			else if ((rf == auxmax) && (bf >= gf))//R green min
			{
				hue = 360 + 60 * (gf - bf) / (auxmax - auxmin);
			}
			else if (gf == auxmax) //green max
			{
				hue = 120 + 60 * (bf-rf) / (auxmax - auxmin);
			}
			else if (bf == auxmax) //blue max
			{
				hue = 240 + 60 * (rf-gf) / (auxmax - auxmin);
			}

			//hue
			datadst[pos_dst] = (unsigned char)((hue*255)/360);//H

			//Satura��o
			if((auxmax-auxmin) == 0 || auxmax == 0) datadst[pos_dst+1] = (unsigned char)0;//S
			else datadst[pos_dst+1] = (unsigned char)(((auxmax-auxmin)/auxmax)*255);

			//value
			datadst[pos_dst+2] = (unsigned char)auxmax;//V

		}
	}
	return 1;
}
//VC04_37 HSV SEGMENTA��O
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y;
	long int pos_src, pos_dst;
	float hf, sf, vf;

	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			hf = (float)datasrc[pos_src];
			sf = (float)datasrc[pos_src + 1];
			vf = (float)datasrc[pos_src + 2];

			//h vem de 0 a 360 enquanto S e V vem de 0 a 100
			//colocar imagem a preto e branco
			if ((hf > (float)hmin*255/360 && hf < (float)hmax * 255 / 360) && (sf > (float)smin * 255 / 100 && sf < (float)smax * 255 / 100) && (vf > (float)vmin * 255 / 100 && vf < (float)vmax * 255 / 100))
			{
				
				datadst[pos_dst] = (unsigned char)255;
			}
			else datadst[pos_dst] = (unsigned char)0;
		}
	}
	return 1;
}
//VC04_42
int vc_scale_gray_to_rgb(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y;
	long int pos_src, pos_dst;
	float gray;


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 3)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			gray = (float)datasrc[pos_src];

			if (gray < 64)
			{
				datadst[pos_dst] = (unsigned char)0;
				datadst[pos_dst + 1] = (unsigned char)(gray * 4);
				datadst[pos_dst + 2] = (unsigned char)255;
			}
			else if (gray < 128)
			{
				datadst[pos_dst] = (unsigned char)0;
				datadst[pos_dst + 1] = (unsigned char)255;
				datadst[pos_dst + 2] = (unsigned char)(255 - (gray-64) * 4);
			}
			else if (gray < 192)
			{
				datadst[pos_dst] = (unsigned char)((gray-128) * 4);
				datadst[pos_dst + 1] = (unsigned char)255;
				datadst[pos_dst + 2] = (unsigned char)0;
			}
			else if (gray <= 255)
			{
				datadst[pos_dst] = (unsigned char)255;
				datadst[pos_dst + 1] = (unsigned char)(255 - (gray-192) * 4);
				datadst[pos_dst + 2] = (unsigned char)0;
			}
		}
	}
	return 1;
}

//VC05_10 GRAY PARA BINARIO COM THRESHOLD
int vc_gray_to_binary(IVC* srcdst, int threshold)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verifica�ao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	//inverter imagem Gray

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			if (data[pos] < threshold) data[pos] = 0;
			else data[pos] = 1;
		}
	}
	//alterar imagem para 1 nivel!!! binaria!!!
	srcdst->levels = 1;
	return 1;
}

//VC05_12 GRAY PARA BINARIO COM MEDIA GLOBAL
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y;
	long int pos_src, pos_dst;
	int soma=0;
	float media;


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source

			soma += datasrc[pos_src];

		}
	}
	media = soma / (height * width);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino
			if (datasrc[pos_src] < media) datadst[pos_dst] = 0;
			else datadst[pos_dst] = 1;
		}
	}
	return 1;
}

//VC05_12midpoint  GRAY PARA BINARIO COM KERNEL
int vc_gray_to_binary_kernel_midpoint(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, x2, y2, ksize;
	long int pos_src, pos_dst, pos_src2;
	float treshold, min, max;


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	ksize = (kernel - 1) / 2;
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			min = 255;
			max = 0;
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino


			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
			{
				for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
				{
					if ((y2 >= 0) && (y2 < height) && (x2 >= 0) && (x2 < width))
					{

						pos_src2 = y2 * bytesperline_src + x2 * channels_src;//posicao da source
						//pos_dst = y2 * bytesperline_dst + x2 * channels_dst;//posicao destino
						if (max <= (float)datasrc[pos_src2])
						{
							max = (float)datasrc[pos_src2];
						}
						if (min >= (float)datasrc[pos_src2])
						{
							min = (float)datasrc[pos_src2];
						}
					}
					else datadst[pos_dst] = 0;
				}
			}
			treshold = (min + max)*(0.5);//mid point 
			if ((float)datasrc[pos_src] < treshold) datadst[pos_dst] = 0;
			else datadst[pos_dst] = 1;
			
		}
	}
	return 1;
}

//VC06_18 dilatacao
int vc_binary_dilate(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, x2, y2, ksize;
	long int pos_src, pos_dst, pos_src2;
	int max;


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	ksize = (kernel - 1) / 2;
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			max = 0;
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			////limites da imagem com o kernel
			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
			{
				if ((y2 >= 0) && (y2 < height))
				{
					pos_src2 = y2 * bytesperline_src + x * channels_src;//posicao da source
					if (max <= datasrc[pos_src2])
					{
						max = datasrc[pos_src2];
					}
				}
				
			}
			////limites da imagem com o kernel
			for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
			{
				if ((x2 >= 0) && (x2 < width))
				{
					pos_src2 = y * bytesperline_src + x2 * channels_src;//posicao da source
					if (max <= datasrc[pos_src2])
					{
						max = datasrc[pos_src2];
					}
				}
				
			}
			datadst[pos_dst] = max;
		}
	}
	return 1;
}

//VC06_18 erosao
//int vc_binary_erode(IVC* src, IVC* dst, int kernel);

//VC06 ambas.....

//VC08_22 Histogramas GRAY
int vc_gray_histogram_show(IVC* src, IVC* dst)
{	
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, hist[256] = {0}, max = 0;
	long int pos_src, pos_dst;
	int total = height * width;
	float pdf[256];


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			//conta ocorrencias de cada valor de pixel
			hist[datasrc[pos_src]]++;
			for (int i = 0; i < 256; i++)
			{
				if (hist[i] >= max) max = hist[i];
			}
		}
	}
	for (int i = 0; i < 256; i++)
	{
		pdf[i] = (float)hist[i] / max;//total para equali...
	}
	////testes print ocorrencias
	for (int i = 0; i < 256; i++)
	{
		printf("Valor: %d, tem %d ocorrencias.\n", i, hist[i]);
		printf("Valor: %d, tem %f ocorrencias.\n", i, pdf[i]);
	}
	//--------------------------------------------------------
	// Gera o grafico com o histograma
	for (int i = 0, x = (width - 256) / 2; i < 256; i++, x++)
	{
		for (y = height - 1; y > height - 1 - pdf[i] * height; y--)
		{
			datadst[y * bytesperline_dst + x * channels_dst] = 255;
		}
	}
	
	// Desenha linhas de inicio (itensidade = 0) e fim (intensidade = 255)
	for (y = 0; y < height - 1; y++)
	{
		datadst[y * bytesperline_dst + ((width - 256) / 2 - 1) * channels_dst] = 127;
		datadst[y * bytesperline_dst + ((width + 256) / 2 + 1) * channels_dst] = 127;
	}
	
	//---------------------------------------------------------
	return 1;
}


//VC08_23 Histogramas EQUALIZER GRAY
int vc_gray_histogram_equalization(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, hist[256] = { 0 }, max = 0;
	long int pos_src, pos_dst;
	int total = height * width;//maximo de pixeis
	float pdf[256], min = 256, cdf[256];


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			//conta ocorrencias de cada valor de pixel
			hist[datasrc[pos_src]]++;
			//for (int i = 0; i < 256; i++)
			//{
			//	if (hist[i] >= max) max = hist[i];//saber qual � o valor maximo
			//}
		}
	}
	for (int i = 0; i < 256; i++)
	{
		pdf[i] = (float)hist[i] / total;//total para equali...
		if ((pdf[i] < min) && (pdf[i] != 0)) min = pdf[i];//saber minimo do pdf
	}

	for (int i = 0; i < 256; i++)
	{
		if (i != 0) cdf[i] = cdf[i - 1] + pdf[i];//acumulada
		else cdf[0] = pdf[0];
	}
	////testes print ocorrencias
	for (int i = 0; i < 256; i++)
	{
		printf("Valor: %d, tem %d ocorrencias.\n", i, hist[i]);
		printf("Valor: %d, tem %f ocorrencias.\n", i, pdf[i]);
		printf("Valor: %d, tem %f ocorrencias.\n", i, cdf[i]);
	}
	//--------------------------------------------------------
	//GERAR IMAGEM
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			datadst[y * bytesperline_dst + x * channels_dst] = ((cdf[datasrc[pos_src]]-min)/1-min)*(255-1);
		}
	}
	//---------------------------------------------------------
	return 1;
}

//VC07_11 blob Labelling 
int vc_binary_blob_labelling(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, min, label=1;
	long int pos_src, pos_dst, posA, posB, posC, posD, posX;


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			posX = y * bytesperline_dst + x * channels_dst;//posicao destino

			//colocar destino a 255
			datadst[posX] = 255;
		}
	}
	for (y = 1;y < height-1;y++)
	{
		for (x = 1;x < width-1; x++)
		{
			//POSI�OES DO KERNEL / VIZINHOS
			posX = y * bytesperline_src + x * channels_src;//posicao da source X
			posA = (y-1) * bytesperline_src + (x-1) * channels_src;
			posB = (y - 1) * bytesperline_src + x * channels_src;
			posC = (y - 1) * bytesperline_src + (x + 1) * channels_src;
			posD = y * bytesperline_src + (x - 1) * channels_src;

			if (datadst[posX] != 0)
			{

				if (datadst[posA] == 0 && datadst[posB] == 0 && datadst[posC] == 0 && datadst[posD] == 0)
				{
					datadst[posX] = label;
					label++;
				}

				else 
				{
					min = 255;
					//encontrar minimo
					if (datadst[posA] <= min && datadst[posA] != 0) min = datadst[posA];
					if (datadst[posB] <= min && datadst[posB] != 0) min = datadst[posB];
					if (datadst[posC] <= min && datadst[posC] != 0) min = datadst[posC];
					if (datadst[posD] <= min && datadst[posD] != 0) min = datadst[posD];

					datadst[posX] = min;
				}
			}
		}
	}
	return 1;
}

//VC09_36 Detecao de contornos prewitt
int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, min, fdX, fdY;
	long int pos_src, pos_dst, posA, posB, posC, posD, posE, posF, posG,posH, posX;
	float magn;

				/*Posi�oes			SOBEL X				PREWITT X
				* [A	B	C]		[-1		0	1]		[-1		0	1]
				* [D	X	E]		[-2		0	2]		[-1		0	1]
				* [F	G	H]		[-1		0	1]		[-1		0	1]
				*					SOBEL Y				PREWITT Y
				*					[-1		-2	1]		[-1		-1	-1]
				*					[0		0	0]		[0		0	 0]
				*					[1		2	1]		[1		1	 1]
				*/

	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 1;y < height - 1;y++)
	{
		for (x = 1;x < width - 1; x++)
		{
			//POSI�OES DO KERNEL / VIZINHOS
			posA = (y - 1) * bytesperline_src + (x - 1) * channels_src;
			posB = (y - 1) * bytesperline_src + x * channels_src;
			posC = (y - 1) * bytesperline_src + (x + 1) * channels_src;
			posD = y * bytesperline_src + (x - 1) * channels_src;
			posX = y * bytesperline_src + x * channels_src;	//POSICAO X
			posE = y * bytesperline_src + (x + 1) * channels_src;
			posF = (y + 1) * bytesperline_src + (x - 1) * channels_src;
			posG = (y + 1) * bytesperline_src + x * channels_src;
			posH = (y + 1) * bytesperline_src + (x + 1) * channels_src;

			// Calcular derivadas
			fdX = datasrc[posA] * -1 + datasrc[posC] * 1 + datasrc[posD] * -1 + datasrc[posE] * 1 + datasrc[posF] * -1 + datasrc[posH] * 1;
			fdY = datasrc[posA] * -1 + datasrc[posB] * -1 + datasrc[posC] * -1 + datasrc[posF] * 1 + datasrc[posG] * 1 + datasrc[posH] * 1;

			// Calcular magnitude
			magn = sqrt(fdX*fdX+fdY*fdY);
			if (magn > th)
			{
				datadst[posX] = 255;//(unsigned char)magn;
			}
			else datadst[posX] = 0;
		}
	}
	return 1;
}

//VC09_36 Detecao de contornos sobel
int vc_gray_edge_sobel(IVC* src, IVC* dst, float th)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, min, fdX, fdY;
	long int pos_src, pos_dst, posA, posB, posC, posD, posE, posF, posG, posH, posX;
	float magn;

	/*Posi�oes			SOBEL X				PREWITT X
	* [A	B	C]		[-1		0	1]		[-1		0	1]
	* [D	X	E]		[-2		0	2]		[-1		0	1]
	* [F	G	H]		[-1		0	1]		[-1		0	1]
	*					SOBEL Y				PREWITT Y
	*					[-1		-2	1]		[-1		-1	-1]
	*					[0		0	0]		[0		0	 0]
	*					[1		2	1]		[1		1	 1]
	*/

	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 1;y < height - 1;y++)
	{
		for (x = 1;x < width - 1; x++)
		{
			//POSI�OES DO KERNEL / VIZINHOS
			posA = (y - 1) * bytesperline_src + (x - 1) * channels_src;
			posB = (y - 1) * bytesperline_src + x * channels_src;
			posC = (y - 1) * bytesperline_src + (x + 1) * channels_src;
			posD = y * bytesperline_src + (x - 1) * channels_src;
			posX = y * bytesperline_src + x * channels_src;	//POSICAO X
			posE = y * bytesperline_src + (x + 1) * channels_src;
			posF = (y + 1) * bytesperline_src + (x - 1) * channels_src;
			posG = (y + 1) * bytesperline_src + x * channels_src;
			posH = (y + 1) * bytesperline_src + (x + 1) * channels_src;

			// Calcular derivadas
			fdX = datasrc[posA] * -1 + datasrc[posC] * 1 + datasrc[posD] * -2 + datasrc[posE] * 2 + datasrc[posF] * -1 + datasrc[posH] * 1;
			fdY = datasrc[posA] * -1 + datasrc[posB] * -2 + datasrc[posC] * -1 + datasrc[posF] * 1 + datasrc[posG] * 2 + datasrc[posH] * 1;

			// Calcular magnitude
			magn = sqrt(fdX * fdX + fdY * fdY);
			if (magn > th)
			{
				datadst[posX] = 255; //(unsigned char)magn;
			}
			else datadst[posX] = 0;
		}
	}
	return 1;
}

//VC10_14 Dominio Espacial media
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, x2, y2, ksize;
	long int pos_src, pos_dst, pos_src2, soma=0;
	float media;


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	/*Posi�oes
	* [A	B	C]
	* [D	X	E]
	* [F	G	H]
	*/
	ksize = (kernel - 1) / 2;
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino


			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
			{
				for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
				{
					if ((y2 >= 0) && (y2 < height) && (x2 >= 0) && (x2 < width))
					{

						pos_src2 = y2 * bytesperline_src + x2 * channels_src;//posicao da source
						soma += datasrc[pos_src2];

					}
					else datadst[pos_dst] = datasrc[pos_src];//se o kernel estiver fora da imagem
				}
			}
			media = soma / (float)(kernel*kernel);//exemplo 5 por 5 = kernel x kernel
			datadst[pos_dst] = media;
			soma = 0;
		}
	}
	return 1;
}

//VC10_14 Dominio Espacial mediana
int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, x2, y2, ksize, npixkernel, i;
	npixkernel = kernel * kernel;
	long int pos_src, pos_dst, pos_src2, mediana[1000];


	//verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	/*Posi�oes
	* [A	B	C]
	* [D	X	E]
	* [F	G	H]
	*/
	ksize = (kernel - 1) / 2;
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			i = 0;
			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
			{
				for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
				{
					if ((y2 >= 0) && (y2 < height) && (x2 >= 0) && (x2 < width))
					{

						pos_src2 = y2 * bytesperline_src + x2 * channels_src;//posicao da source
						mediana[i] = datasrc[pos_src2];
						i++;

					}
					//else datadst[pos_dst] = datasrc[pos_src];//se o kernel estiver fora da imagem
				}
			}
			//buble sort
			for (i = 0; i < npixkernel - 1; i++)
			{
				for (int j = 0; j < npixkernel - i - 1; j++) {
					if (mediana[j] > mediana[j + 1]) 
					{
						int temp = mediana[j];
						mediana[j] = mediana[j + 1];
						mediana[j + 1] = temp;
					}
				}
			}
			datadst[pos_dst] = mediana[(npixkernel-1)/2];
		}
	}
	return 1;
}

//VC10_20 filtro gaussian INCOMPLETO
//int vc_gray_lowpass_gaussian_filter(IVC* src, IVC* dst)
//{
//	// info source
//	unsigned char* datasrc = (unsigned char*)src->data;
//	int bytesperline_src = src->width * src->channels;
//	int channels_src = src->channels;
//
//	//info destino
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int bytesperline_dst = dst->width * dst->channels;
//	int channels_dst = dst->channels;
//
//	// medidas
//	int width = src->width;
//	int height = src->height;
//
//	//auxiliares gerais
//	int x, y, x2, y2, ksize, npixkernel, i;
//	npixkernel = kernel * kernel;
//	long int pos_src, pos_dst, pos_src2, mediana[1000];
//
//
//	//verifica��o de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
//	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
//	if ((src->channels != 1) || (dst->channels != 1)) return 0;
//
//	/*Posi�oes
//	* [A	B	C]
//	* [D	X	E]
//	* [F	G	H]
//	*/
//	ksize = (kernel - 1) / 2;
//	for (y = 0;y < height;y++)
//	{
//		for (x = 0;x < width; x++)
//		{
//			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
//			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino
//
//			i = 0;
//			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
//			{
//				for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
//				{
//					if ((y2 >= 0) && (y2 < height) && (x2 >= 0) && (x2 < width))
//					{
//
//						pos_src2 = y2 * bytesperline_src + x2 * channels_src;//posicao da source
//						mediana[i] = datasrc[pos_src2];
//						i++;
//
//					}
//					//else datadst[pos_dst] = datasrc[pos_src];//se o kernel estiver fora da imagem
//				}
//			}
//			//buble sort
//			for (i = 0; i < npixkernel - 1; i++)
//			{
//				for (int j = 0; j < npixkernel - i - 1; j++) {
//					if (mediana[j] > mediana[j + 1])
//					{
//						int temp = mediana[j];
//						mediana[j] = mediana[j + 1];
//						mediana[j + 1] = temp;
//					}
//				}
//			}
//			datadst[pos_dst] = mediana[(npixkernel - 1) / 2];
//		}
//	}
//	return 1;
//}