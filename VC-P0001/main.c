#include <stdio.h>
#include "vc.h"




//Abrir imagem, alterar e gravar em novo ficheiro
//VC03_19 INVERTER CORES ---image->data[i] = 255 - image->data[i]---;
int main_VC03_19(void)
{
	IVC* image;
	int i;

	image = vc_read_image("images/FLIR/flir-05.pgm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	for (i = 0; i < image->bytesperline * image->height; i += image->channels)
	{
		image->data[i] = 255 - image->data[i];
	}

	vc_write_image("vc-0001.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}
//CRIA IMAGEM
//WHITE-BLACK
//BLACK-WHITE
//VC03_24 
int main_VC03_24(void)
{
	IVC* image;
	int x, y;
	long int pos;

	image = vc_image_new(320, 280, 1, 1);
	if (image == NULL)
	{
		printf("ERRO -> vc_image_new():\n\tOut of memory!\n");
		getchar();
		return 0;
	}

	for (x = 0; x < image->width; x++)
	{
		for (y = 0; y < image->height; y++)
		{
			pos = y * image->bytesperline + x * image->channels;

			if ((x <= image->width / 2) && (y <= image->height / 2)) image->data[pos] = 1;
			else if ((x > image->width / 2) && (y > image->height / 2)) image->data[pos] = 1;
			else image->data[pos] = 0;
		}
	}

	vc_write_image("vc-0001.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//ABRE IMAGEM BINARIA E TROCA PRETO POR BRANCO, GRAVA
//VC03_26
int main_VC03_26(void)
{
	IVC* image;
	int i;

	image = vc_read_image("vc-0001.pgm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	for (i = 0; i < image->bytesperline * image->height; i += image->channels)
	{
		if (image->data[i] == 1) image->data[i] = 0;
		else image->data[i] = 1;
	}

	vc_write_image("vc-0002.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}

//ABRE IMAGEM, GERA NEGATIVO GRAY, GRAVA
//ABRE IMAGEM, GERA NEGATIVO RGB, GRAVA
//VC_04_15/16
int main(void)
{
	IVC* image;
	int i;

	image = vc_read_image("images/Additional/houses.pgm");
	if (image == NULL)
	{
		printf("ERRO -> vc_read_image():\n\tFile not found!\n");
		getchar();
		return 0;
	}

	vc_gray_negative(image); // PARA GRAY
	//vc_rgb_negative(image); // PARA RGB

	vc_write_image("vc-0415.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}
